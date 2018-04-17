#ifndef	RENDERER_H
#define RENDERER_H

#include <array>
#include <vector>
#include <string>

#include "Logger.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>
#define VK_RESERVED_FOR_FUTURE_USE 0
#define VK_FLAGS_NONE 0

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Scene.h"

// logger
static VkResult vkResult;
#if _DEBUG
static Logger logger;
static Logger debugReportCallbackLogger;
static Logger cleanUpLogger;
#endif

namespace VkU
{
	// declarations
	struct Queue;

	enum VERTEX_TYPE
	{
		UNKNOWN = 0,
		XY = 1,		// width & height
		Z = 2,		// depth
		RGB = 4,	// rgb color
		UV = 8,		// texture coordinate
		N = 16,		// normal
		T = 32,		// tangent
		B = 64,		// bitangent

		XYZ		= XY | Z,
		NT		= N | T,
		TB		= T | B,
		NTB		= N | T | B,

		XYZ_RGB		= XYZ | RGB,
		XYZ_UV		= XYZ | UV,
		XYZ_UV_N	= XYZ | UV | N,
		XYZ_UV_NT	= XYZ | UV | NT,
		XYZ_UV_NTB	= XYZ | UV | NTB,
	};
	static inline uint32_t GetVertexTypeStride(VERTEX_TYPE _type)
	{
		uint32_t stride = 0;

		if ((_type & XY) == XY)
			stride += sizeof(float) * 2;
		if ((_type & Z) == Z)
			stride += sizeof(float) * 1;
		if ((_type & RGB) == RGB)
			stride += sizeof(float) * 3;
		if ((_type & UV) == UV)
			stride += sizeof(float) * 2;
		if ((_type & N) == N)
			stride += sizeof(float) * 3;
		if ((_type & T) == T)
			stride += sizeof(float) * 3;
		if ((_type & B) == B)
			stride += sizeof(float) * 3;

		return stride;
	}

	static std::vector<VkFormat> preferedDepthFormats = {
		VK_FORMAT_D32_SFLOAT, // 100% availability 3/d1/2017
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_D16_UNORM, // 100% availability 3/d1/2017
		VK_FORMAT_D16_UNORM_S8_UINT,
	};
	static std::vector<VkCompositeAlphaFlagBitsKHR> preferedCompositeAlphas = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	static std::vector<VkPresentModeKHR> preferedPresentModes = {
		VK_PRESENT_MODE_MAILBOX_KHR,
	};

	// other
#if _DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT _flags, VkDebugReportObjectTypeEXT _objType, uint64_t _obj, size_t _location, int32_t _code, const char* _layerPrefix, const char* _msg, void* _userData)
	{
		if (_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			debugReportCallbackLogger << "	INFORMATION:";
		else if (_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			debugReportCallbackLogger << "WARNING:";
		else if (_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			debugReportCallbackLogger << "PERFORMANCE:";
		else if (_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			debugReportCallbackLogger << "ERROR:";
		else if (_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			debugReportCallbackLogger << "	DEBUG:";

		debugReportCallbackLogger << _msg << '\n';

		return VK_FALSE; // VK_FALSE wont't abort the function that made this call
	}
#endif
	static LRESULT CALLBACK WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);

	static void BeginCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandBufferUsageFlags _flags);

	// hardware
	struct PhysicalDevice
	{
		VkPhysicalDevice						handle;

		VkPhysicalDeviceFeatures				features;
		VkPhysicalDeviceMemoryProperties		memoryProperties;
		VkPhysicalDeviceProperties				properties;
		std::vector<VkQueueFamilyProperties>	queueFamilyProperties;
		std::vector<VkBool32>					queueFamilyPresentable;

		VkFormat depthFormat;

		std::vector<Queue> queues;

		void Init(VkPhysicalDevice _physicalDeviceHandle, std::vector<VkFormat>* _preferedDepthFormat)
		{
			// Get attributes
			handle = _physicalDeviceHandle;

			vkGetPhysicalDeviceProperties(handle, &properties);
			vkGetPhysicalDeviceFeatures(handle, &features);
			vkGetPhysicalDeviceMemoryProperties(handle, &memoryProperties);

			uint32_t propertyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(handle, &propertyCount, nullptr);
			queueFamilyProperties.resize(propertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(handle, &propertyCount, queueFamilyProperties.data());

			queueFamilyPresentable.resize(queueFamilyProperties.size());
			for (uint32_t j = 0; j != queueFamilyPresentable.size(); ++j)
				queueFamilyPresentable[j] = vkGetPhysicalDeviceWin32PresentationSupportKHR(handle, j);

			// Find depth
			depthFormat = VK_FORMAT_UNDEFINED;

			if (_preferedDepthFormat != nullptr)
			{
				for (uint32_t j = 0; j != _preferedDepthFormat->size(); ++j)
				{
					VkFormatProperties formatProperties;
					vkGetPhysicalDeviceFormatProperties(handle, (*_preferedDepthFormat)[j], &formatProperties);

					if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
					{
						depthFormat = (*_preferedDepthFormat)[j];
						break;
					}
				}
			}

#if _DEBUG
			if (depthFormat == VK_FORMAT_UNDEFINED)
				logger << "Physical Device " << _physicalDeviceHandle << " named " << properties.deviceName << " driver version " << properties.driverVersion << " has no available DEPTH FORMAT.\n";
#endif
		}

		uint32_t FindMemoryTypeIndex(VkMemoryRequirements _memoryRequirements, VkMemoryPropertyFlags _memoryPropertyFlags)
		{
			for (uint32_t i = 0; i != memoryProperties.memoryTypeCount; ++i)
			{
				if ((_memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & _memoryPropertyFlags) == _memoryPropertyFlags)
				{
					return i;
				}
			}

			return -1;
		}
	};
	struct Window
	{
		HWND		hWnd;
		HINSTANCE	hInstance;
		const char*	name;

		int width;
		int height;
	};
	struct Surface
	{
		VkSurfaceKHR				handle;
		VkSurfaceFormatKHR			colorFormat;
		VkCompositeAlphaFlagBitsKHR	compositeAlpha;
		VkPresentModeKHR			presentMode;

		static VkSurfaceFormatKHR GetVkSurfaceFormatKHR(VkPhysicalDevice _physicalDevice, Surface _surface, std::vector<VkFormat>* _preferedColorFormats);
		static VkSurfaceCapabilitiesKHR GetVkSurfaceCapabilitiesKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface);
		static VkCompositeAlphaFlagBitsKHR GetVkCompositeAlphaFlagBitsKHR(VkSurfaceCapabilitiesKHR _surfaceCapabilities, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas);
		static VkPresentModeKHR GetVkPresentModeKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface, std::vector<VkPresentModeKHR>* _preferedPresentModes);
	};
	struct Queue
	{
		std::vector<VkQueue>	handles;

		uint32_t				queueFamilyIndex;
		uint32_t				queueIndex;

		VkQueueFlags			flags;
		VkBool32				presentability;
		float					priority;
		uint32_t				count;

		static bool CheckQueueFamilyIndexSupport(uint32_t _familyIndex, PhysicalDevice _physicalDevice, VkSurfaceKHR _surface, VkQueueFlags _flags, VkBool32 _presentability, uint32_t _count)
		{
			VkBool32 surfaceSupported = VK_FALSE;
			if (_surface != VK_NULL_HANDLE)
			{
				vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice.handle, _familyIndex, _surface, &surfaceSupported);
				if (surfaceSupported == VK_FALSE)
					return false;
			}

			if ((_flags & _physicalDevice.queueFamilyProperties[_familyIndex].queueFlags) != _flags)
				return false;

			if (_presentability == VK_TRUE && _physicalDevice.queueFamilyPresentable[_familyIndex] == VK_FALSE)
				return false;

			if (_count > _physicalDevice.queueFamilyProperties[_familyIndex].queueCount)
				return false;

			return true;
		}
		static std::vector<uint32_t> GetQueueFamilyIndicesWithSupport(Queue _deviceQueue, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces)
		{
			std::vector<uint32_t> indices;

			for (uint32_t i = 0; i != (uint32_t)_physicalDevice.queueFamilyProperties.size(); ++i)
			{
				bool allSurfacesValid = true;
				for (size_t s = 0; s != _surfaces.size(); ++s)
				{
					if (!CheckQueueFamilyIndexSupport(i, _physicalDevice, _surfaces[s].handle, _deviceQueue.flags, _deviceQueue.presentability, _deviceQueue.count))
					{
						allSurfacesValid = false;
						break;
					}
				}

				if (allSurfacesValid)
					indices.push_back(i);
			}

			return indices;
		}
		static bool PickDeviceQueuesIndicesRecursively(std::vector<uint32_t>& _queueFamilyUseCount, std::vector<std::vector<uint32_t>> _deviceQueuesValidIndices, std::vector<VkQueueFamilyProperties> _queueFamilyProperties, std::vector<std::array<uint32_t, 3>>& _queueFamily_Indices_Count, size_t _depth)
		{
			if (_depth == _deviceQueuesValidIndices.size())
				return true; // nothing to left to assign, therefore we succeed

			for (size_t i = 0; i != _deviceQueuesValidIndices[_depth].size(); ++i)
			{
				if (_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] + _queueFamily_Indices_Count[_depth][2] <= _queueFamilyProperties[_deviceQueuesValidIndices[_depth][i]].queueCount)
				{

					uint32_t queueIndex = _queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]];
					uint32_t queueFamilyIndex = _deviceQueuesValidIndices[_depth][i];
					_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] += _queueFamily_Indices_Count[_depth][2];

					if (PickDeviceQueuesIndicesRecursively(_queueFamilyUseCount, _deviceQueuesValidIndices, _queueFamilyProperties, _queueFamily_Indices_Count, _depth + 1))
					{
						_queueFamily_Indices_Count[_depth][0] = queueFamilyIndex;
						_queueFamily_Indices_Count[_depth][1] = queueIndex;
						return true;
					}
					else
					{
						_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] -= _queueFamily_Indices_Count[_depth][2];
					}
				}
			}

			return false;
		}
		static std::vector<VkU::Queue> PickDeviceQueuesIndices(std::vector<Queue> _queues, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces, bool * _isCompatible)
		{
			std::vector<std::vector<uint32_t>> deviceQueuesValidIndices;

			// get possible indices
			for (size_t q = 0; q != _queues.size(); ++q)
			{
				std::vector<uint32_t> indices = GetQueueFamilyIndicesWithSupport(_queues[q], _physicalDevice, _surfaces);

				if (indices.size() == 0)
				{
					if (_isCompatible != nullptr)
						*_isCompatible = false;

					return _queues; // one queue cannot be represented
				}
				else
					deviceQueuesValidIndices.push_back(indices);
			}

			// assign indices
			std::vector<uint32_t> queueFamilyUseCount(_physicalDevice.queueFamilyProperties.size());
			std::vector<std::array<uint32_t, 3>> queueFamily_Indices_Count(_queues.size());
			for (size_t i = 0; i != queueFamily_Indices_Count.size(); ++i)
				queueFamily_Indices_Count[i][2] = _queues[i].count;

			*_isCompatible = PickDeviceQueuesIndicesRecursively(queueFamilyUseCount, deviceQueuesValidIndices, _physicalDevice.queueFamilyProperties, queueFamily_Indices_Count, 0);

			for (size_t q = 0; q != _queues.size(); ++q)
			{
				_queues[q].queueFamilyIndex = queueFamily_Indices_Count[q][0];
				_queues[q].queueIndex = queueFamily_Indices_Count[q][1];
			}

			return _queues;
		}

		static inline Queue GetQueue(VkBool32 _presentability, VkQueueFlags _flags, float _priority, uint32_t _count)
		{
			return{ {}, (uint32_t)-1, (uint32_t)-1, _flags, _presentability, _priority, _count };
		}
	};
	struct Device
	{
		VkDevice handle;
		uint32_t physicalDeviceIndex;
	};

	// Data Objects
	struct ShaderData
	{
		const char* filename;
		VkShaderStageFlagBits stage;
		const char* entryPointName;
	};
	struct MeshData
	{
		VkDeviceSize vertexOffset;
		VkDeviceSize indexOffset;
		uint32_t indexCount;
	};

	// render objects
	struct Buffer
	{
		VkBuffer handle;
		VkDeviceMemory memory;

		void Init(VkDeviceSize _size, VkBufferUsageFlags _usage, VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryProperties);
		void FillStagingBuffer(VkDeviceSize _size, void* _data, VkDevice _device);
		void UpdateBuffer(VkDeviceSize _size, VkCommandBuffer _commandBuffer, Buffer _stagingBuffer)
		{
			VkBufferCopy copyRegion;
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = _size;

			vkCmdCopyBuffer(_commandBuffer, _stagingBuffer.handle, handle, 1, &copyRegion);
		}
	};
	struct MappedBuffer
	{
		VkBuffer handle;
		VkDeviceMemory memory;

		VkDeviceSize size;
		VkBufferUsageFlags usage;
		uint64_t fragments;

		struct Node
		{
			Node* prev;
			Node* next;
			VkDeviceSize offset;
			VkDeviceSize size;
			bool inUse;
		};
		Node* subBufferMapHead;
		Node* subBufferMapTail;

		void Init(VkDeviceSize _size, VkBufferUsageFlags _usage, VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryProperties);
		Node* AllocateSubBuffer(VkDeviceSize _size, void* _data, VkCommandBuffer _commandBuffer, VkFence _fence, VkDevice _vkDevice, PhysicalDevice _physicalDevice);
		void FreeSubBuffer(Node* _subBuffer);
		void Clear(VkDevice _vkDevice);

		void Cout();
	};
	struct Image
	{
		VkImage handle;
		VkDeviceMemory memory;
		VkImageView view;

		void InitHandle(VkFormat _format, VkExtent3D _extent3D, VkImageUsageFlags _usage, VkDevice _device);
		void InitMemory(PhysicalDevice _physicalDevice, VkDevice _device);
		void InitView(VkFormat _format, VkImageAspectFlags _aspect, VkDevice _device);
	};
	struct StagingImage
	{
		VkImage handle;
		VkDeviceMemory memory;

		void InitHandle(VkFormat _format, VkExtent3D _extent3D, VkDevice _device);
		void InitMemory(PhysicalDevice _physicalDevice, VkDevice _device);
		void FillImageMemory(uint32_t _width, uint32_t _height, VkDeviceSize _size, void* _data, VkDevice device);
	};
	
	struct Swapchain
	{
		VkSwapchainKHR handle;

		VkExtent2D extent;

		std::vector<VkImage>			images;
		VkU::Image						depthImage;
		std::vector<VkImageView>		views;
		std::vector<VkFramebuffer>		framebuffers;

		std::vector<VkCommandBuffer>	commandBuffers;
		std::vector<VkFence>			fences;
		VkSemaphore						semaphoreImageAvailable;

		uint32_t						imageIndex;
	};
	struct ShaderModule
	{
		VkShaderModule			handle;
		VkShaderStageFlagBits	stage;
		const char*				entryPointName;
		
		static void LoadSpvCode(const char* _filename, size_t& _fileSize, char** _buffer)
		{
			std::ifstream file(_filename, std::ios::ate | std::ios::binary);

			if (file.is_open())
			{
				_fileSize = (size_t)file.tellg();
				file.seekg(0);

				if (file.good())
				{
					*_buffer = new char[_fileSize];
					file.read(*_buffer, _fileSize);
				}
				else
				{
					_fileSize = 0;
				}
			}

			file.close();
		}
	};
}

class Renderer
{
	// core
	VkInstance instance;
#if _DEBUG
	VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;
#endif

	// hardware
	std::vector<VkU::PhysicalDevice> physicalDevices;
	VkU::Window window;
	VkU::Surface surface;
	VkU::Device device;

	// gpu & cpu communication / synchonization
	VkCommandPool commandPool;
	VkCommandBuffer setupCommandBuffer;
	VkFence setupFence;
	VkSemaphore semaphoreRenderDone;

	// renderPass
	VkRenderPass renderPass;

	// swapchain
	VkU::Swapchain swapchain;

	// descriptors
	VkDescriptorSetLayout descriptorSetLayoutMVP;
	VkDescriptorSetLayout descriptorSetLayoutTexture;
	VkDescriptorPool descriptorPool;

	// buffers
	glm::mat4 viewProjection[2];
	VkU::Buffer viewProjectionBuffer;
	VkU::Buffer viewProjectionStaging;

	uint32_t maxModelMatrixCount;
	std::vector<glm::mat4> modelMatrices;
	VkU::Buffer modelMatricesBuffer;
	VkU::Buffer modelMatricesStaging;

	// models
	std::vector<VkU::MeshData> meshOffsets;
	std::vector<uint64_t> vertexOffsets;
	std::vector<uint64_t> vertexSizes;
	VkU::Buffer vertexBuffer;
	std::vector<uint64_t> indexOffsets;
	std::vector<uint64_t> indexSizes;
	VkU::Buffer indexBuffer;

	// samplers & textures
	std::vector<VkU::Image> textures;

	VkSampler sampler;

	VkDescriptorSet descriptorSetMVP;
	std::vector<VkDescriptorSet> descriptorSetsTexture;

	// pipeline
	VkPipelineLayout pipelineLayout;
	std::vector<VkU::ShaderModule> shaderModules;

	std::vector<std::vector<VkPipelineShaderStageCreateInfo>>	pipelineShaderStagesCreateInfos;
	std::vector<VkVertexInputBindingDescription>					vertexInputBindingDescriptions;
	std::vector<std::vector<VkVertexInputAttributeDescription>>		vertexInputAttributeDescriptions;
	std::vector<VkPipelineVertexInputStateCreateInfo>			pipelineVertexInputStateCreateInfos;
	std::vector<VkPipelineInputAssemblyStateCreateInfo>			pipelineInputAssemblyStateCreateInfos;
	std::vector<VkPipelineTessellationStateCreateInfo>			pipelineTessellationStateCreateInfos;
	std::vector<VkViewport>											viewports;
	std::vector<VkRect2D>											scissors;
	std::vector<VkPipelineViewportStateCreateInfo>				pipelineViewportStateCreateInfos;
	std::vector<VkPipelineRasterizationStateCreateInfo>			pipelineRasterizationStateCreateInfos;
	std::vector<VkPipelineMultisampleStateCreateInfo>			pipelineMultisampleStateCreateInfos;
	std::vector<VkPipelineDepthStencilStateCreateInfo>			pipelineDepthStencilStateCreateInfos;
	std::vector<VkPipelineColorBlendAttachmentState>				pipelineColorBlendAttachmentState;
	std::vector<VkPipelineColorBlendStateCreateInfo>			pipelineColorBlendStateCreateInfos;
	std::vector<VkPipelineDynamicStateCreateInfo>				pipelineDynamicStateCreateInfo;
	std::vector<VkGraphicsPipelineCreateInfo>				graphicsPipelineCreateInfos;
	std::vector<VkComputePipelineCreateInfo>				computePipelineCreateInfos;

	std::vector<VkPipeline> graphicsPipelines;
	std::vector<VkPipeline> computePipelines;

	float nearProjection;
	float farProjection;
	float fovProjection;

	/// Functions
	void InitInstance(const char* _appName, uint32_t _appVersion, const char* _engineName, uint32_t _engineVersion, std::vector<const char*> _enabledInstanceLayerNames, std::vector<const char*> _enabledInstanceExtensionNames);
#if _DEBUG
	void InitDebug(VkDebugReportFlagsEXT _debugFlags, VKAPI_ATTR VkBool32(*_debugReportCallback)(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*));
#endif
	void InitPhysicalDevices(std::vector<VkFormat>* _preferedAllDepthFormats);
	void InitWindow(const char* _windowTitle, const char* _windowName, WNDPROC _wndProc, int _width, int _height);
	void PickPhysicalDeviceAndQueue(std::vector<VkU::Queue> _queues, VkPhysicalDeviceFeatures _features);
	void InitDevice(std::vector<const char*> _targetDeviceLayerNames, std::vector<const char*> _targetDeviceExtensionNames, VkPhysicalDeviceFeatures _features, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas, std::vector<VkPresentModeKHR>* _preferedPresentModes);
	void InitRenderPass();
	void InitSwapchain(uint32_t _targetSwapchainImageCount);
	void InitCommandPool();
	void InitCommandBuffers();
	void InitFences();
	void InitSemaphores();
	void InitDescriptorPool(uint32_t _combinedImageSamplerCount, uint32_t _uniformBufferCount, uint32_t _storageBufferCount);
	void InitDescriptorSetLayouts();
	void InitPipelineLayout();
	void LoadShaderModules(std::vector<VkU::ShaderData> _shaderModulesData);
	void InitPipelineData();
	void InitPipeline();
	void InitUniformBuffers(uint32_t _maxModelMatrixCount);
	void LoadModels(std::vector<const char*>* _modelNames);
	void LoadImages(std::vector<const char*>* _imageNamesTGA);
	void InitSamplers();
	void InitDescriptorSets();
	void UpdateDescriptorSets();

public:
	void Init(uint32_t _maxModelMatrixCount, std::vector<const char*>* _modelNames, std::vector<const char*>* _imageNamesTGA);
	void Draw(Scene* _scene);
	void Render();
	void ShutDown();

	static Renderer* currentRenderer;
	void RecreateSwapchain(int _width, int _height);
};

#endif