#include "Game.h"

Timer Game::globalTimer;
double Game::lastTime;
double Game::deltaTime;

bool Game::done = false;

bool Game::inputs[512];
bool* Game::lastInput;
bool* Game::currentInput;

Game::INPUT_STATE Game::GetInputState(uint8_t _input)
{
	if (lastInput[_input] == true)
		if (currentInput[_input] == true)
			return INPUT_STATE::IS_DOWN;
		else
			return INPUT_STATE::IS_RELEASED;
	else
		if (currentInput[_input] == true)
			return INPUT_STATE::IS_PRESSED;
		else
			return INPUT_STATE::IS_IDLE;

	return INPUT_STATE();
}

void Game::Init()
{
	// Init Input
	currentInput = &inputs[0];
	lastInput = &inputs[256];

	for (int i = 0; i != 256; ++i)
	{
		currentInput[i] = GetAsyncKeyState(i);
		lastInput[i] = false;
	}

	// Init Time
	lastTime = 0.0;
	deltaTime = 0.0;
	globalTimer.SetResolution(Timer::RESOLUTION_NANOSECONDS);
	globalTimer.Play();

	// Clears scene
	scene->Clear();

	// First Game State
	scene->gameObjects.push_back(Scene::GameObject());
	scene->gameObjects[scene->gameObjects.size() - 1].gameObjectModules.push_back(Scene::GameObject::GameObjectModule::GetGameObjectModule({ &Scene::StartIntro }, nullptr, nullptr));
}

void Game::Input()
{
	// Update Input
	bool* tempPtr = currentInput;
	currentInput = lastInput;
	lastInput = tempPtr;

	for (int i = 0; i != 256; ++i)
		currentInput[i] =  GetAsyncKeyState(i);
}
void Game::Update()
{
	deltaTime = globalTimer.GetTime() - lastTime;
	lastTime = globalTimer.GetTime();

	scene->CallGameObjectsModules();
}
