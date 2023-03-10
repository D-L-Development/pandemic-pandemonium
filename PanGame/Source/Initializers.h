#pragma once

#include "Vector2D.h"
#include "InputDevice.h"
#include <memory>
#include <string>
#include "Definitions.h"


const float PI = 3.14159f;
const float fPRV = 10.0f;

inline float PW2RW(float x) { return x * fPRV; }
inline float RW2PW(float x) { return x / fPRV; }
inline float RW2PWAngle(float x) { return((float)x * (2.0f * PI) / 360.0f); } //degrees to radians
inline float PW2RWAngle(float x) { return((float)x * 360.0f / (2.0f * PI)); }

//Basic Types
typedef unsigned int Uint32;
typedef Uint32 GAME_INT;



const int SCREEN_WIDTH{ 600 };
const int SCREEN_HEIGHT{ 900 };

inline int score{ 0 };
inline int highScore{ 0 };
inline bool isPlayerDead{ false };
inline int health{ 100 };  //player health

enum GAME_OBJECT_SHAPE { GAME_RECTANGLE, GAME_CIRCLE };
enum GAME_BODY_TYPE { GAME_STATIC, GAME_KINEMATIC, GAME_DYNAMIC };
enum class ObjectType { Player, Karen, Platform, Virus, Item, Floor, Any };
enum class Item { VIRUS, PAPERS, MASK, mRNA, SYRINGE, TESTTUBE, BUNDLE, NONE };

struct ItemNumbers {
	int Papers{ 0 };
	int Mask{ 0 };
	int mRNA{ 0 };
	int Syringe{ 0 };
	int TestTube{ 0 };
	int Bundle{ 0 };


	void incrementItemCount(Item itemType) {
		switch (itemType)
		{
		case Item::PAPERS:
			++Papers;
			break;
		case Item::MASK:
			++Mask;
			break;
		case Item::mRNA:
			++mRNA;
			break;
		case Item::SYRINGE:
			++Syringe;
			break;
		case Item::TESTTUBE:
			++TestTube;
			break;
		case Item::BUNDLE:
			break;
		default:
			break;
		}

		checkForFive();
	}
private:
	void checkForFive() {
		int count{ 0 };
		if (Papers > 0) {
			count++;
		}
		if (TestTube > 0) {
			count++;
		}
		if (mRNA > 0) {
			count++;
		}
		if (Mask > 0) {
			count++;
		}
		if (Syringe > 0) {
			count++;
		}

		if (count >= 5) {
			Bundle++;
			Papers--;
			TestTube--;
			mRNA--;
			Mask--;
			Syringe--;
		}
	}

};





struct PhysicsPresets {
	float spinSpeed{ 0.0f };
	GAME_BODY_TYPE physicsBodyType;
	GAME_OBJECT_SHAPE objectShape;
	float density;
	float friction;
	float restitution;
	float angularDamping;
	float linearDamping;
	float force;
	bool physicsOn{ false };
};

struct GamePosition {
	Vector2D position;
	float angle;
};


//Info for clipping array
struct ClipInfo {
	int spriteID{ 0 };
	int animLag{ 0 };
	int numSprites{ 0 };
	int height{ 0 };
	int width{ 0 };
	int col_num{ 0 };
	int row_num{ 0 };
};

struct SpritePresets {
	bool createSprite{ false };
	bool isAnimated{ false };   //flag for whether to read a sprite sheet
	std::shared_ptr<Texture> spriteTexture{ nullptr };
	std::shared_ptr<ClipInfo> clip{ nullptr };     //instance of clipInfo for array
};

struct BodyPresets {
	bool createBody{ false };
	GamePosition positionElements{ {0,0}, 0 };
	std::shared_ptr<SpriteComponent>sprite{ nullptr };
	PhysicsPresets physics;
	ObjectType objectType{ ObjectType::Any };
	Item itemType{ Item::NONE };
};



//structs to decide whether to create components or not
//more will be added here such as rotation speed, etc.
struct UserInputPresets {
	bool createUserInput{ false };
};

struct MovePresets {
	bool createMove{ false };
};

struct RotatePresets {
	bool createRotate{ false };
};

struct SlidePresets {
	bool createSlide{ false };
};

struct ChasePresets {
	bool createChase{ false };
};

struct ObjectFactoryPresets {
	SpritePresets spriteInitializers;
	BodyPresets bodyInitializers;
	UserInputPresets inputInitializers;
	MovePresets moveInitializers;
	RotatePresets rotateInitializers;
	SlidePresets slideInitializers;
	ChasePresets chaseInitializers;
};