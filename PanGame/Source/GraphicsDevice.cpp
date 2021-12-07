#include <string>
#include "GraphicsDevice.h"


GraphicsDevice::GraphicsDevice(Uint32 width, Uint32 height, bool fullScreen = true) : SCREEN_WIDTH(width), SCREEN_HEIGHT(height)
{
	//initialize all SDL subsystems
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	//initialize SDL_image subsystems
	if (!IMG_Init(IMG_INIT_PNG))
	{
		printf("SDL_image could not initialize! SDL_Error: %s\n", IMG_GetError());
	}

	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		exit(1);
	}

	//Initialize Font----------File path-----------font size
	scoreFont = TTF_OpenFont("./Assets/Fonts/impact.ttf", 22);
	textFont = TTF_OpenFont("./Assets/Fonts/impact.ttf", 17);
	deathFont = TTF_OpenFont("./Assets/Fonts/impact.ttf", 100);

	if (scoreFont == nullptr || textFont == nullptr || deathFont == nullptr) {
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
	}

	if (!fullScreen)
	{
		//Construct and check window construction
		screen = SDL_CreateWindow("Pandemic Pandemonium",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		screen = SDL_CreateWindow("Pandemic Pandemonium",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	}
	if (screen == nullptr)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	}

	//Construct the renderer
	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
	}

	//set the background color (default blue)
	SDL_SetRenderDrawColor(renderer, 145, 206, 235, 235);
	//set the background color (default white)
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


	////Load background image
	//backImg = IMG_Load("Hospital.png");
	////Create background texture
	//backTexture = SDL_CreateTextureFromSurface(renderer, backImg);
	////Render to window
	//SDL_RenderCopy(renderer, backTexture, NULL, NULL);




	//create view
	Vector2D position{ 0, 0 };
	view = std::make_unique<View>(position);
}

GraphicsDevice::~GraphicsDevice()
{
	//Free the window
	SDL_DestroyWindow(screen);
	screen = nullptr;

	//Free renderer
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;

	//clean up text
	TTF_CloseFont(scoreFont);
	TTF_CloseFont(textFont);
	TTF_CloseFont(deathFont);

	////Clean up background
	//SDL_DestroyTexture(backTexture);
	//SDL_FreeSurface(backImg);

	//Quit SDL Subsystems
	IMG_Quit();
	SDL_Quit();
	TTF_Quit();
}

void GraphicsDevice::Begin()
{
	SDL_RenderClear(renderer);
}


void GraphicsDevice::Present()
{
	SDL_RenderPresent(renderer);
}

SDL_Renderer* GraphicsDevice::getRenderer()
{
	return(renderer);
}

View* GraphicsDevice::getView()
{
	return view.get();
}

void GraphicsDevice::setView(View* view)
{
	this->view = std::unique_ptr<View>(view);
}

void GraphicsDevice::update(int& highScore, int& score, bool& isDead) {

	//-------------------HIGHSCORE RENDERING------------------------------------------

	std::string hScore = std::to_string(highScore);
	std::string hScoreText = "Highscore";

	//Render Command to Text
	SDL_Color textColor = { 125, 92, 128 };

	SDL_Surface* hScoreSurface{ TTF_RenderText_Solid(scoreFont, hScore.c_str(), textColor) };
	SDL_Surface* hScoreTextSurface{ TTF_RenderText_Solid(textFont, hScoreText.c_str(), textColor) };

	SDL_Texture* hScoreTexture = SDL_CreateTextureFromSurface(renderer, hScoreSurface);
	SDL_Texture* hScoreTextTexture = SDL_CreateTextureFromSurface(renderer, hScoreTextSurface);

	//Set rendering space and render to screen
	int width = 0, height = 0;
	SDL_QueryTexture(hScoreTexture, nullptr, nullptr, &width, &height);
	SDL_Rect hScoreQuad = { SCREEN_WIDTH - width - 15, 20, width, height };
	SDL_QueryTexture(hScoreTextTexture, nullptr, nullptr, &width, &height);
	SDL_Rect hScoreTextQuad = { SCREEN_WIDTH - width - 15, 2, width, height };

	//Render to screen
	SDL_RenderCopy(renderer, hScoreTexture, nullptr, &hScoreQuad);
	SDL_RenderCopy(renderer, hScoreTextTexture, nullptr, &hScoreTextQuad);

	SDL_FreeSurface(hScoreSurface);
	SDL_FreeSurface(hScoreTextSurface);
	SDL_DestroyTexture(hScoreTexture);
	SDL_DestroyTexture(hScoreTextTexture);

	//-----------------CURRENT SCORE RENDERING----------------------------------------

	std::string cScore = std::to_string(score);
	std::string cScoreText = "Current Score";

	SDL_Surface* cScoreSurface{ TTF_RenderText_Solid(scoreFont, cScore.c_str(), textColor) };
	SDL_Surface* cScoreTextSurface{ TTF_RenderText_Solid(textFont, cScoreText.c_str(), textColor) };

	//Render Command to Text
	SDL_Texture* cScoreTexture = SDL_CreateTextureFromSurface(renderer, cScoreSurface);
	SDL_Texture* cScoreTextTexture = SDL_CreateTextureFromSurface(renderer, cScoreTextSurface);

	//Set rendering space and render to screen
	SDL_QueryTexture(cScoreTexture, nullptr, nullptr, &width, &height);
	SDL_Rect cScoreQuad = { SCREEN_WIDTH - width - 15, 63, width, height };
	SDL_QueryTexture(cScoreTextTexture, nullptr, nullptr, &width, &height);
	SDL_Rect cScoreTextQuad = { SCREEN_WIDTH - width - 15, 45, width, height };

	SDL_RenderCopy(renderer, cScoreTextTexture, nullptr, &cScoreTextQuad);
	SDL_RenderCopy(renderer, cScoreTexture, nullptr, &cScoreQuad);

	SDL_FreeSurface(cScoreSurface);
	SDL_FreeSurface(cScoreTextSurface);
	SDL_DestroyTexture(cScoreTexture);
	SDL_DestroyTexture(cScoreTextTexture);

	//--------------------DEATH RENDERING-------------------------------------------

	std::string death{ NULL };
	if (isDead) {
		death = "YOU LOSE!";
	}

	//Render Command to Death Text
	SDL_Color deathColor = { 255, 37, 0 };
	SDL_Surface* deathSheetSurface{ TTF_RenderText_Solid(deathFont, death.c_str(), deathColor) };
	SDL_Texture* deathSheetTexture = SDL_CreateTextureFromSurface(renderer, deathSheetSurface);

	//Set rendering space and render to screen
	SDL_QueryTexture(deathSheetTexture, nullptr, nullptr, &width, &height);
	SDL_Rect deathQuad = { SCREEN_WIDTH - 475, 425, width, height };


	SDL_RenderCopy(renderer, deathSheetTexture, nullptr, &deathQuad);

	SDL_FreeSurface(deathSheetSurface);
	SDL_DestroyTexture(deathSheetTexture);
}