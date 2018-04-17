#ifndef SCENE_H
#define SCENE_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "Timer.h"

class Scene
{
public:
	// Time

	// Iteration
	size_t gameObjectIterationIndex;
	size_t gameObjectModuleIterationIndex;
	size_t gameObjectModuleFunctionsIterationIndex;
	bool restartGameObjectIteration = false;

	// Rendering
	size_t renderCallCount;
	struct RenderCall
	{
		glm::mat4 transform;
		uint32_t renderMode;	// picks a pipeline
		size_t vertexPushConstantDataSize;// pushConstant data
		void* vertexPushConstantData;
		uint32_t textureID;		// picks a descriptor
		uint32_t meshID;		// picks offsets for vertices and indices and count for indices

		static RenderCall GetRenderCall(glm::mat4 _transform, uint32_t _renderMode, size_t _vertexPushConstantDataSize, void* _vertexPushConstantData, uint32_t _textureID, uint32_t _meshID)
		{ return { _transform, _renderMode, _vertexPushConstantDataSize, _vertexPushConstantData, _textureID, _meshID }; }
	};
	std::vector<RenderCall> renderCalls;

	// All "Game" things
	struct GameObject
	{
		// 
		struct GameObjectModule
		{
			std::vector<void(Scene::*)(void* _data)> functions;
			void(Scene::*destructor)(void* _data);
			void* data;

			static inline GameObjectModule GetGameObjectModule(std::vector<void(Scene::*)(void* _data)> _functions, void(Scene::*_destructor)(void* _data), void* _data)
			{ return { _functions, _destructor, _data }; }
		};
		std::vector<GameObjectModule> gameObjectModules;
	};
	std::vector<GameObject> gameObjects;

	// Scene functions
	void CallGameObjectsModules();
	void Clear();

	// Game Module functions

	void StartIntro(void* _data);

	struct EditableVoxelObject
	{
		struct VoxelObj
		{
			struct Voxel
			{
				bool visible;
				float r, g, b;
			};
			std::vector<std::vector<std::vector<Voxel>>> data;
		} voxels;
		struct EditData
		{
			int32_t x, y, z;
			float r, g, b;
		} editData;
	};
	void StartVoxelEditor(void* _data);
	void UpdateVoxelEditor(void* _data);
	void DrawVoxelEditor(void* _data);
	void DestroyVoxelEditor(void* _data);

	//void StartGame();
};

#endif