#include "Scene.h"

#include "Engine.h"

#define SIZE 32
#define OFFSET 16.0f

void Scene::CallGameObjectsModules()
{
	renderCallCount = 0;

	// Iterate all object/modules
	do
	{
		restartGameObjectIterationPoint:

		if (Engine::done == true)
			return;

		// Ensures restart wont loop by mistake
		restartGameObjectIteration = false;

		// Each object
		for (gameObjectIterationIndex = 0;; ++gameObjectIterationIndex)
		{
			if (restartGameObjectIteration == true)
				goto restartGameObjectIterationPoint;// Stop and restart
			else if (gameObjectIterationIndex == gameObjects.size())
				break;

			// Each module
			for (gameObjectModuleIterationIndex = 0;; ++gameObjectModuleIterationIndex)
			{
				if (restartGameObjectIteration == true)
					goto restartGameObjectIterationPoint;// Stop and restart
				else if (gameObjectModuleIterationIndex == gameObjects[gameObjectIterationIndex].gameObjectModules.size())
					break;

				// Each Function
				for (gameObjectModuleFunctionsIterationIndex = 0;; ++gameObjectModuleFunctionsIterationIndex)
				{
					if (restartGameObjectIteration == true)
						goto restartGameObjectIterationPoint;// Stop and restart
					else if (gameObjectModuleFunctionsIterationIndex == gameObjects[gameObjectIterationIndex].gameObjectModules[gameObjectModuleIterationIndex].functions.size())
						break;

					// Call Function
					((*this).*gameObjects[gameObjectIterationIndex].gameObjectModules[gameObjectModuleIterationIndex].functions[gameObjectModuleFunctionsIterationIndex])(gameObjects[gameObjectIterationIndex].gameObjectModules[gameObjectModuleIterationIndex].data);
				}
			}
		}
	} while (restartGameObjectIteration); // Restart if requested
}
void Scene::Clear()
{
	for (size_t i = 0; i != renderCalls.size(); ++i)
		if(renderCalls[i].vertexPushConstantData != nullptr)
			delete renderCalls[i].vertexPushConstantData;
	renderCalls.clear();

	for (size_t i = 0; i != gameObjects.size(); ++i)
		for (size_t j = 0; j != gameObjects[i].gameObjectModules.size(); ++j)
			if (gameObjects[i].gameObjectModules[j].data != nullptr)
				((*this).*gameObjects[gameObjectIterationIndex].gameObjectModules[gameObjectModuleIterationIndex].destructor)(gameObjects[gameObjectIterationIndex].gameObjectModules[gameObjectModuleIterationIndex].data);
	gameObjects.clear();

	restartGameObjectIteration = true;
}

void Scene::StartIntro(void* _data)
{
	// Cleans game state
	Clear();

	// Start next game state
	gameObjects.push_back(Scene::GameObject());
	gameObjects[gameObjects.size() - 1].gameObjectModules.push_back(Scene::GameObject::GameObjectModule::GetGameObjectModule({ &Scene::StartVoxelEditor }, nullptr, nullptr));
}

void Scene::StartVoxelEditor(void* _data)
{
	// Cleans game state
	Clear();

	renderCalls.resize(1 + SIZE * SIZE * SIZE);

	gameObjects.push_back(Scene::GameObject());
	gameObjects[0].gameObjectModules.push_back(Scene::GameObject::GameObjectModule::GetGameObjectModule({ &Scene::UpdateVoxelEditor, &Scene::DrawVoxelEditor }, &Scene::DestroyVoxelEditor, new EditableVoxelObject));
	EditableVoxelObject* editableVoxelObject = (EditableVoxelObject*)gameObjects[0].gameObjectModules[0].data;
	editableVoxelObject->editData.x = SIZE/2;
	editableVoxelObject->editData.y = SIZE/2;
	editableVoxelObject->editData.z = SIZE/2;
	editableVoxelObject->editData.r = 1.0f;
	editableVoxelObject->editData.g = 1.0f;
	editableVoxelObject->editData.b = 1.0f;

	editableVoxelObject->voxels.data.resize(SIZE);
	for (size_t i = 0; i != editableVoxelObject->voxels.data.size(); ++i)
	{
		editableVoxelObject->voxels.data[i].resize(SIZE);
		for (size_t j = 0; j != editableVoxelObject->voxels.data[i].size(); ++j)
		{
			editableVoxelObject->voxels.data[i][j].resize(SIZE);
			for (size_t k = 0; k != editableVoxelObject->voxels.data[i][j].size(); ++k)
			{
				editableVoxelObject->voxels.data[i][j][k].visible = false;
				editableVoxelObject->voxels.data[i][j][k].r = 0.0f;
				editableVoxelObject->voxels.data[i][j][k].g = 0.0f;
				editableVoxelObject->voxels.data[i][j][k].b = 0.0f;
			}
		}
	}

	for (size_t i = 0; i < editableVoxelObject->voxels.data.size(); i += editableVoxelObject->voxels.data.size() -1)
	{
		for (size_t j = 0; j < editableVoxelObject->voxels.data[0].size(); j += editableVoxelObject->voxels.data[0].size() -1)
		{
			for (size_t k = 0; k < editableVoxelObject->voxels.data[0][0].size(); k += editableVoxelObject->voxels.data[0][0].size() -1)
			{
				editableVoxelObject->voxels.data[i][j][k].visible = true;
				editableVoxelObject->voxels.data[i][j][k].r = 1.0f;
				editableVoxelObject->voxels.data[i][j][k].g = 1.0f;
				editableVoxelObject->voxels.data[i][j][k].b = 1.0f;
			}
		}
	}

	//editableVoxelObject->voxels.data[0][0][0].visible = true;
	//editableVoxelObject->voxels.data[0][0][editableVoxelObject->voxels.data[0][0].size() - 1].visible = true;
	//editableVoxelObject->voxels.data[0][editableVoxelObject->voxels.data[0].size() - 1][0].visible = true;
	//editableVoxelObject->voxels.data[0][editableVoxelObject->voxels.data[0].size() - 1][editableVoxelObject->voxels.data[0][0].size() - 1].visible = true;
	//
	//editableVoxelObject->voxels.data[editableVoxelObject->voxels.data.size() - 1][0][0].visible = true;
	//editableVoxelObject->voxels.data[editableVoxelObject->voxels.data.size() - 1][0][editableVoxelObject->voxels.data[0][0].size() - 1].visible = true;
	//editableVoxelObject->voxels.data[editableVoxelObject->voxels.data.size() - 1][editableVoxelObject->voxels.data[0].size() - 1][0].visible = true;
	//editableVoxelObject->voxels.data[editableVoxelObject->voxels.data.size() - 1][editableVoxelObject->voxels.data[0].size() - 1][editableVoxelObject->voxels.data[0][0].size() - 1].visible = true;
}
void Scene::UpdateVoxelEditor(void* _data)
{
	if (Engine::GetInputState(Engine::IC_ESCAPE))
	{
		// Cleans up itself
		Clear();

		// Terminates app
		Engine::done = true;

		return;
	}

	EditableVoxelObject* editableVoxelObject = (EditableVoxelObject*)_data;

	bool controlled = Engine::GetInputState(Engine::IC_CONTROL) == Engine::INPUT_STATE::IS_DOWN;
	bool shifted = Engine::GetInputState(Engine::IC_SHIFT) == Engine::INPUT_STATE::IS_DOWN;

	// Move Selection
	if (Engine::GetInputState(Engine::IC_D) == Engine::INPUT_STATE::IS_PRESSED)
		++editableVoxelObject->editData.x;
	if (Engine::GetInputState(Engine::IC_A) == Engine::INPUT_STATE::IS_PRESSED)
		--editableVoxelObject->editData.x;

	if (Engine::GetInputState(Engine::IC_E) == Engine::INPUT_STATE::IS_PRESSED)
		++editableVoxelObject->editData.y;
	if (Engine::GetInputState(Engine::IC_Q) == Engine::INPUT_STATE::IS_PRESSED)
		--editableVoxelObject->editData.y;

	if (Engine::GetInputState(Engine::IC_W) == Engine::INPUT_STATE::IS_PRESSED)
		++editableVoxelObject->editData.z;
	if (Engine::GetInputState(Engine::IC_S) == Engine::INPUT_STATE::IS_PRESSED)
		--editableVoxelObject->editData.z;

	// Change selection's color
	Engine::INPUT_STATE inputCheckType = Engine::INPUT_STATE::IS_PRESSED;
	if (shifted)
		inputCheckType = Engine::INPUT_STATE::IS_DOWN;

	float colorChangeAmmount = 1.0f / 255.0f;
	if (controlled)
		colorChangeAmmount *= 16.0f;

	if(shifted)
		colorChangeAmmount = (float)Engine::deltaTime;

	// R
	if (Engine::GetInputState(Engine::IC_U) == inputCheckType)
	{
		editableVoxelObject->editData.r += colorChangeAmmount;
		if (editableVoxelObject->editData.r > 1.0f)
			editableVoxelObject->editData.r = 1.0f;
	}
	if (Engine::GetInputState(Engine::IC_J) == inputCheckType)
	{
		editableVoxelObject->editData.r -= colorChangeAmmount;
		if (editableVoxelObject->editData.r < 0.0f)
			editableVoxelObject->editData.r = 0.0f;
	}

	// G
	if (Engine::GetInputState(Engine::IC_I) == inputCheckType)
	{
		editableVoxelObject->editData.g += colorChangeAmmount;
		if (editableVoxelObject->editData.g > 1.0f)
			editableVoxelObject->editData.g = 1.0f;
	}
	if (Engine::GetInputState(Engine::IC_K) == inputCheckType)
	{
		editableVoxelObject->editData.g -= colorChangeAmmount;
		if (editableVoxelObject->editData.g < 0.0f)
			editableVoxelObject->editData.g = 0.0f;
	}

	// B
	if (Engine::GetInputState(Engine::IC_O) == inputCheckType)
	{
		editableVoxelObject->editData.b += colorChangeAmmount;
		if (editableVoxelObject->editData.b > 1.0f)
			editableVoxelObject->editData.b = 1.0f;
	}
	if (Engine::GetInputState(Engine::IC_L) == inputCheckType)
	{
		editableVoxelObject->editData.b -= colorChangeAmmount;
		if (editableVoxelObject->editData.b < 0.0f)
			editableVoxelObject->editData.b = 0.0f;
	}

	// Copy
	if (Engine::GetInputState(Engine::IC_C) == Engine::INPUT_STATE::IS_PRESSED)
	{
		editableVoxelObject->editData.r = editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].r;
		editableVoxelObject->editData.g = editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].g;
		editableVoxelObject->editData.b = editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].b;
	}

	// Confirm
	if (Engine::GetInputState(Engine::IC_ENTER) == Engine::INPUT_STATE::IS_PRESSED)
	{
		if (editableVoxelObject->editData.x >= 0 && editableVoxelObject->editData.x < editableVoxelObject->voxels.data.size())
		{
			if (editableVoxelObject->editData.y >= 0 && editableVoxelObject->editData.y < editableVoxelObject->voxels.data[0].size())
			{
				if (editableVoxelObject->editData.z >= 0 && editableVoxelObject->editData.z < editableVoxelObject->voxels.data[0][0].size())
				{
					editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].visible = true;
					editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].r = editableVoxelObject->editData.r;
					editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].g = editableVoxelObject->editData.g;
					editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].b = editableVoxelObject->editData.b;
				}
			}
		}
	}

	// Remove
	if (Engine::GetInputState(Engine::IC_DELETE) == Engine::INPUT_STATE::IS_PRESSED ||
		Engine::GetInputState(Engine::IC_BACKSPACE) == Engine::INPUT_STATE::IS_PRESSED ||
		Engine::GetInputState(Engine::IC_PAD_DOT) == Engine::INPUT_STATE::IS_PRESSED)
	{
		editableVoxelObject->voxels.data[editableVoxelObject->editData.x][editableVoxelObject->editData.y][editableVoxelObject->editData.z].visible = false;
	}

	// Set to predefined
	if (Engine::GetInputState(Engine::IC_F9) == Engine::INPUT_STATE::IS_PRESSED)
	{
		// Reset voxels
		for (size_t i = 0; i != editableVoxelObject->voxels.data.size(); ++i)
		{
			editableVoxelObject->voxels.data[i].resize(SIZE);
			for (size_t j = 0; j != editableVoxelObject->voxels.data[i].size(); ++j)
			{
				editableVoxelObject->voxels.data[i][j].resize(SIZE);
				for (size_t k = 0; k != editableVoxelObject->voxels.data[i][j].size(); ++k)
				{
					editableVoxelObject->voxels.data[i][j][k].visible = false;
					editableVoxelObject->voxels.data[i][j][k].r = 0.0f;
					editableVoxelObject->voxels.data[i][j][k].g = 0.0f;
					editableVoxelObject->voxels.data[i][j][k].b = 0.0f;
				}
			}
		}

		for (size_t i = 0; i < editableVoxelObject->voxels.data.size(); i += editableVoxelObject->voxels.data.size() - 1)
		{
			for (size_t j = 0; j < editableVoxelObject->voxels.data[0].size(); j += editableVoxelObject->voxels.data[0].size() - 1)
			{
				for (size_t k = 0; k < editableVoxelObject->voxels.data[0][0].size(); k += editableVoxelObject->voxels.data[0][0].size() - 1)
				{
					editableVoxelObject->voxels.data[i][j][k].visible = true;
					editableVoxelObject->voxels.data[i][j][k].r = 1.0f;
					editableVoxelObject->voxels.data[i][j][k].g = 1.0f;
					editableVoxelObject->voxels.data[i][j][k].b = 1.0f;
				}
			}
		}
	}

	// Convert Into Mesh
	if (Engine::GetInputState(Engine::IC_SPACE) == Engine::INPUT_STATE::IS_PRESSED && false)
	{
		struct VertexData
		{
			float x, y, z, r, g, b, nx, ny, nz;
		};
		struct FaceData
		{
			uint32_t i1, i2, i3;
		};
		std::vector<VertexData> vertexData;
		std::vector<FaceData> faceData;

		uint32_t currentIndex = 0;
		for (size_t x = 0; x != editableVoxelObject->voxels.data.size(); ++x)
		{
			for (size_t y = 0; y != editableVoxelObject->voxels.data[x].size(); ++y)
			{
				for (size_t z = 0; z != editableVoxelObject->voxels.data[x][y].size(); ++z)
				{
					// if visible
					if (editableVoxelObject->voxels.data[x][y][z].visible == true)
					{
						float r = editableVoxelObject->voxels.data[x][y][z].r;
						float g = editableVoxelObject->voxels.data[x][y][z].g;
						float b = editableVoxelObject->voxels.data[x][y][z].b;

						// left face 0~5	x-1
						if (true)
						{
							VertexData v0 = { -0.5f, -0.5f, -0.5f,		r,g,b,		-1.0f, 0.0f, 0.0f };
							VertexData v1 = { -0.5f, -0.5f, +0.5f,		r,g,b,		-1.0f, 0.0f, 0.0f };
							VertexData v2 = { -0.5f, +0.5f, +0.5f,		r,g,b,		-1.0f, 0.0f, 0.0f };
							VertexData v3 = { -0.5f, +0.5f, -0.5f,		r,g,b,		-1.0f, 0.0f, 0.0f };
							FaceData f0 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f1 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}

						// top face 6~11	y+1
						if (true)
						{
							VertexData v4 = { -0.5f, +0.5f, -0.5f,		r,g,b,		0.0f, +1.0f, 0.0f };
							VertexData v5 = { -0.5f, +0.5f, +0.5f,		r,g,b,		0.0f, +1.0f, 0.0f };
							VertexData v6 = { +0.5f, +0.5f, +0.5f,		r,g,b,		0.0f, +1.0f, 0.0f };
							VertexData v7 = { +0.5f, +0.5f, -0.5f,		r,g,b,		0.0f, +1.0f, 0.0f };
							FaceData f2 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f3 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}

						// right face 12~17		x+1
						if (true)
						{
							VertexData v8 = { +0.5f, +0.5f, -0.5f,		r,g,b,		+1.0f, 0.0f, 0.0f };
							VertexData v9 = { +0.5f, +0.5f, +0.5f,		r,g,b,		+1.0f, 0.0f, 0.0f };
							VertexData v10 = { +0.5f, -0.5f, +0.5f,		r,g,b,		+1.0f, 0.0f, 0.0f };
							VertexData v11 = { +0.5f, -0.5f, -0.5f,		r,g,b,		+1.0f, 0.0f, 0.0f };
							FaceData f4 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f5 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}

						// bottom face 18~23	y-1
						if (true)
						{
							VertexData v12 = { +0.5f, -0.5f, -0.5f,		r,g,b,		0.0f, -1.0f, 0.0f };
							VertexData v13 = { +0.5f, -0.5f, +0.5f,		r,g,b,		0.0f, -1.0f, 0.0f };
							VertexData v14 = { -0.5f, -0.5f, +0.5f,		r,g,b,		0.0f, -1.0f, 0.0f };
							VertexData v15 = { -0.5f, -0.5f, -0.5f,		r,g,b,		0.0f, -1.0f, 0.0f };
							FaceData f6 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f7 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}

						// front face 24~29		z-1
						if (true)
						{
							VertexData v16 = { -0.5f, +0.5f, -0.5f,		r,g,b,		0.0f, 0.0f, -1.0f };
							VertexData v17 = { +0.5f, +0.5f, -0.5f,		r,g,b,		0.0f, 0.0f, -1.0f };
							VertexData v18 = { +0.5f, -0.5f, -0.5f,		r,g,b,		0.0f, 0.0f, -1.0f };
							VertexData v19 = { -0.5f, -0.5f, -0.5f,		r,g,b,		0.0f, 0.0f, -1.0f };
							FaceData f8 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f9 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}

						// back face 30~35		z+1
						if (true)
						{
							VertexData v20 = { +0.5f, +0.5f, +0.5f,		r,g,b,		0.0f, 0.0f, +1.0f };
							VertexData v21 = { -0.5f, +0.5f, +0.5f,		r,g,b,		0.0f, 0.0f, +1.0f };
							VertexData v22 = { -0.5f, -0.5f, +0.5f,		r,g,b,		0.0f, 0.0f, +1.0f };
							VertexData v23 = { +0.5f, -0.5f, +0.5f,		r,g,b,		0.0f, 0.0f, +1.0f };
							FaceData f10 = { currentIndex + 0, currentIndex + 1, currentIndex + 2 };
							FaceData f11 = { currentIndex + 0, currentIndex + 2, currentIndex + 3 };
							currentIndex += 4;
						}
					}
				}
			}
		}

	}
}
void Scene::DrawVoxelEditor(void * _data)
{
	EditableVoxelObject* editableVoxelObject = (EditableVoxelObject*)_data;

	// Render selection
	if (renderCalls[renderCallCount].vertexPushConstantData != nullptr)
		delete[] renderCalls[renderCallCount].vertexPushConstantData;
	renderCalls[renderCallCount] = Scene::RenderCall::GetRenderCall(glm::translate(glm::mat4(1), glm::vec3(editableVoxelObject->editData.x - OFFSET, editableVoxelObject->editData.y - OFFSET, editableVoxelObject->editData.z - OFFSET)), 0, sizeof(float) * 4, new float[4], 0, 0);
	((float*)renderCalls[renderCallCount].vertexPushConstantData)[1] = editableVoxelObject->editData.r;
	((float*)renderCalls[renderCallCount].vertexPushConstantData)[2] = editableVoxelObject->editData.g;
	((float*)renderCalls[renderCallCount].vertexPushConstantData)[3] = editableVoxelObject->editData.b;
	
	++renderCallCount;

	for (size_t i = 0; i != editableVoxelObject->voxels.data.size(); ++i)
	{
		for (size_t j = 0; j != editableVoxelObject->voxels.data[i].size(); ++j)
		{
			for (size_t k = 0; k != editableVoxelObject->voxels.data[i][j].size(); ++k)
			{
				if (editableVoxelObject->voxels.data[i][j][k].visible == true)
				{
					if(renderCalls[renderCallCount].vertexPushConstantData != nullptr)
						delete[] renderCalls[renderCallCount].vertexPushConstantData;
					renderCalls[renderCallCount] = Scene::RenderCall::GetRenderCall(glm::translate(glm::mat4(), glm::vec3((float)(i - OFFSET), (float)(j - OFFSET), (float)(k - OFFSET))), 0, sizeof(float) * 4, new float[4], 0, 0);
					// [0] is reserved by the implementation of modelMatrices
					((float*)renderCalls[renderCallCount].vertexPushConstantData)[1] = editableVoxelObject->voxels.data[i][j][k].r;
					((float*)renderCalls[renderCallCount].vertexPushConstantData)[2] = editableVoxelObject->voxels.data[i][j][k].g;
					((float*)renderCalls[renderCallCount].vertexPushConstantData)[3] = editableVoxelObject->voxels.data[i][j][k].b;
					
					++renderCallCount;
				}
			}
		}
	}
}
void Scene::DestroyVoxelEditor(void * _data)
{
	delete (EditableVoxelObject*)_data;
}
