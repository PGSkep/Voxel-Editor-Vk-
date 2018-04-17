#include "Engine.h"

#include <Windows.h>

// Global
bool Engine::done;
Scene Engine::scene;
Renderer Engine::renderer;

// Time
Timer Engine::globalTimer;
double Engine::lastTime;
double Engine::deltaTime;

// Input
bool Engine::inputs[512];
bool* Engine::lastInput;
bool* Engine::currentInput;

Engine::INPUT_STATE Engine::GetInputState(uint8_t _input)
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

// Engine
void Engine::Init()
{
	// Init input
	currentInput = &inputs[0];
	lastInput = &inputs[256];

	for (int i = 0; i != 256; ++i)
	{
		currentInput[i] = GetAsyncKeyState(i);
		lastInput[i] = false;
	}

	// Init time
	lastTime = 0.0;
	deltaTime = 0.0;
	globalTimer.SetResolution(Timer::RESOLUTION_NANOSECONDS);
	globalTimer.Play();

	// Init scene
	scene.gameObjects.push_back(Scene::GameObject());
	scene.gameObjects[scene.gameObjects.size() - 1].gameObjectModules.push_back(Scene::GameObject::GameObjectModule::GetGameObjectModule({ &Scene::StartIntro }, nullptr, nullptr));

	// Init renderer
	std::vector<const char*> modelNames = {
		"Models/Cube.fbx",
	};
	std::vector<const char*> imageNamesTGA = {
		"Images/CouchNormal.tga",
	};

	renderer.Init(1024, &modelNames, &imageNamesTGA);
}
void Engine::Loop()
{
	UpdateTime();
	UpdateInput();
	UpdateScene();
	while (Engine::done != true)
	{
		UpdateRenderer();

		UpdateTime();
		UpdateInput();
		UpdateScene();
	}
}

void Engine::UpdateTime()
{
	deltaTime = globalTimer.GetTime() - lastTime;
	lastTime = globalTimer.GetTime();
}
void Engine::UpdateInput()
{
	bool* tempPtr = currentInput;
	currentInput = lastInput;
	lastInput = tempPtr;

	for (int i = 0; i != 256; ++i)
		currentInput[i] = GetAsyncKeyState(i);
}
void Engine::UpdateScene()
{
	scene.CallGameObjectsModules();
}
void Engine::UpdateRenderer()
{
	renderer.Draw(&scene);
	renderer.Render();
}

void Engine::ShutDown()
{
	renderer.ShutDown();
}
