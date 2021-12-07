#include "StaticHandler.h"
#include "ObjectFactory.h"
#include "PhysicsDevice.h"
#include "RandomPosition.h"
#include <string>
#include <random>

//struct so we can pass multiple things into sdl timer
struct KarenCallBackParam {
	std::vector<std::shared_ptr<GameObject>>& newObjects;
	bool& createKaren;
	KarenCallBackParam(std::vector<std::shared_ptr<GameObject>>& vec, bool& makeKaren) : newObjects(vec), createKaren(makeKaren) {}
};

// callback for deleting karen from the game vector
Uint32 RemoveKarenCallback(Uint32 interval, void* param) {

	//cast back to type we want
	KarenCallBackParam* parameters = static_cast<KarenCallBackParam*>(param);

	parameters->newObjects.erase(std::remove_if(
		parameters->newObjects.begin(), parameters->newObjects.end(), //searches beginning of vector to end
		[](std::shared_ptr<GameObject> object) {

			bool shouldDelete = object->GetComponent<BodyComponent>()->getObjectType() == ObjectType::Karen;

			// if object is has a slide component and is below screen, then delete the body and object
			if (shouldDelete) { object->GetComponent<BodyComponent>()->getPDevice()->removeObject(object.get()); }
			return (shouldDelete);
		}),
		parameters->newObjects.end());

	//setting flag back so she can be created again
	parameters->createKaren = true;

	return 0;
}

// call back for stoping karen's chase behavior and changing velocity
Uint32 GetKarenOffScreen(Uint32 interval, void* param) {
	//cast back to type we want
	KarenCallBackParam* parameters = static_cast<KarenCallBackParam*>(param);

	for (auto& object : parameters->newObjects) {
		if (auto karenBody = object->GetComponent<BodyComponent>(); karenBody->getObjectType() == ObjectType::Karen) {
			// disable chase behavior
			object->GetComponent<ChaseComponent>()->disabled = true;
			// increase karen's velocity
			karenBody->getPDevice()->setLinearVelocity(object.get(), { 400, 50 });
			// delete karen after 3 seconds (time in ms, callback function name, parameter being passed)
			SDL_TimerID timerID = SDL_AddTimer(3000, RemoveKarenCallback, param);
		}
	}


	return 0;
}

StaticHandler::StaticHandler(ObjectFactory* factory, std::shared_ptr<BodyComponent> playerBodyComponent, std::shared_ptr<UserInputComponent> playerInputComponent) :
	playerBodyComponent(playerBodyComponent), playerInputComponent(playerInputComponent), factory(factory)
{
	//Configuring Future Assets
	if (futureDoc.LoadFile("./Assets/Config/FutureObjects.xml") != tinyxml2::XML_SUCCESS)
	{
		printf("Bad Library File Path");
		exit(1);
	}

	tinyxml2::XMLElement* FutureRoot = futureDoc.FirstChildElement("Objects");  //Root

	//Grabbing Karen info and platform info from xml to use to pass into factory create method later
	tinyxml2::XMLElement* enemyElement = FutureRoot->FirstChildElement("Enemy");
	tinyxml2::XMLElement* platformsElement = enemyElement->NextSiblingElement("Platforms");

	//searching for karen in xml
	for (karenElement = enemyElement->FirstChildElement(); karenElement; karenElement = karenElement->NextSiblingElement()) {
		std::string componentName = karenElement->Attribute("name");
		if (componentName == "Karen") {
			break;
		}
	}

	if (!karenElement) {
		printf("Failed to grab Karen from xml!");
		exit(1);
	}

	//searching for platform in xml
	for (platformElement = platformsElement->FirstChildElement(); platformElement; platformElement = platformElement->NextSiblingElement()) {
		std::string componentName = platformElement->Attribute("name");
		if (componentName == "Platform") {
			break;
		}
	}

	if (!platformElement) {
		printf("Failed to grab Platform from xml!");
		exit(1);
	}

	randomHandler = std::make_unique<RandomPosition>();

}

StaticHandler::~StaticHandler() {

}

std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> StaticHandler::update(std::vector<std::shared_ptr<GameObject>>& gameObjects) {

	// kill karen by invoking a callback after 5 seconds
	if (killKaren) {
		KarenCallBackParam* paramObject = new KarenCallBackParam(gameObjects, createKaren);
		SDL_TimerID timerID = SDL_AddTimer(5000, GetKarenOffScreen, static_cast<void*>(paramObject)); //cast to void*
		killKaren = false;  //reset flag
	}


	// decide to delete
	deleteObjects(gameObjects);
	//decide whether to enable physics on static objects
	physicsEnabled(gameObjects);

	// check player's y postion, decide to scroll platfroms
	if (playerBodyComponent->getPosition().y <= SCREEN_HEIGHT / 2) {
		slideObjects(gameObjects);
	}

	return createObjects();
}

void StaticHandler::slideObjects(std::vector<std::shared_ptr<GameObject>>& gameObjects) {

	for (auto& object : gameObjects) {
		if (auto platformComponent = object->GetComponent<SlideComponent>()) {
			platformComponent->setSlide(playerInputComponent);
		}
	}

}

std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> StaticHandler::createObjects() {

	// make an empty vector of objects
	std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> newObjects = std::make_shared<std::vector<std::shared_ptr<GameObject>>>();

	// create karen one time and wait for her to be deleted before making a new one
	if ((score + 1) % 15 == 0 && !killKaren && createKaren) {
		score++;
		newObjects->push_back(std::shared_ptr<GameObject>(factory->create(karenElement)));
		newObjects->back()->GetComponent<ChaseComponent>()->targetBody = playerBodyComponent;
		newObjects->back()->GetComponent<BodyComponent>()->getPDevice()->setFixedRotation(newObjects->back().get(), true);
		killKaren = true;
		createKaren = false;
	}

	// if last platform created is on screen, make some more
	if ((firstRound) || (lastPlatformBody->getPosition().y > 0)) {
		firstRound = false;
		// getting positions from randomHandler
		std::vector<Vector2D> positions{ randomHandler->getRandomPositions() };

		// create a platform object for each position
		for (auto& position : positions) {
			// create new vector with platforms
			newObjects->push_back(std::shared_ptr<GameObject>(factory->create(platformElement)));

			//find the body for the object and give it a random position
			std::shared_ptr<BodyComponent> tempBody = newObjects->back()->GetComponent<BodyComponent>();
			tempBody->getPDevice()->setTransform(newObjects->back().get(), { position, 0.0f });
			// ignoring the collsion with new platfroms
			tempBody->getPDevice()->setEnabled(newObjects->back().get(), false);
		}

		// store the last platform's body position
		// to decide when to create more platfroms
		lastPlatformBody = newObjects->back()->GetComponent<BodyComponent>();


	}

	return newObjects;
}

void StaticHandler::deleteObjects(std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	//erase objects if they drop below the screen
	int initialSize{ (int)gameObjects.size() };
	gameObjects.erase(std::remove_if(
		gameObjects.begin(), gameObjects.end(), //searches beginning of vector to end
		[](std::shared_ptr<GameObject> object) {

			bool shouldDelete = object->GetComponent<SlideComponent>() && object->GetComponent<BodyComponent>()->getPosition().y > SCREEN_HEIGHT;
			// if object is has a slide component and is below screen, then delete the body and object
			if (shouldDelete) { object->GetComponent<BodyComponent>()->getPDevice()->removeObject(object.get()); }
			return (shouldDelete);
		}),
		gameObjects.end());

	score += (initialSize - gameObjects.size());
	if (highScore < score) {
		highScore = score;
	}
}

void StaticHandler::physicsEnabled(std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	for (auto& object : gameObjects) {

		/*if (auto platformComponent = object->GetComponent<SlideComponent>(); platformComponent) {
			if (playerBodyComponent->getVelocity().y > 0) {
				platformComponent->objectBody->getPDevice()->setEnabled(object.get(), true);
			}
			else {
				platformComponent->objectBody->getPDevice()->setEnabled(object.get(), false);
			}
		}*/
		if (auto platformComponent = object->GetComponent<SlideComponent>(); platformComponent) {
			if (platformComponent->objectBody->getPosition().y > playerBodyComponent->getPosition().y
				+ playerBodyComponent->getOwner()->GetComponent<SpriteComponent>()->getTexture()->getHeight() - 3) {
				platformComponent->objectBody->getPDevice()->setEnabled(object.get(), true);
			}
			else {
				platformComponent->objectBody->getPDevice()->setEnabled(object.get(), false);
			}
		}

	}
}
