#include "Renderer.h"

#include "Engine.h"

#if _DEBUG
#define VK_CHECK_RESULT(call, variable, callName) vkResult = call;																				\
logger << "Result: " << vkResult << "	Value: " << variable << "	Time: " << Engine::globalTimer.GetTime() << "	Call: " << callName << '\n'
#define VK_CHECK_CLEANUP(call, variable, callName) call;																						\
cleanUpLogger << "Value: " << variable << "	Time: " << Engine::globalTimer.GetTime() << "	Call: " << callName << '\n';						\
variable = VK_NULL_HANDLE
#else
#define VK_CHECK_RESULT(call, variable, callName) vkResult = call
#define VK_CHECK_CLEANUP(call, variable, callName) call
#endif

#define GRAPHICS_PRESENT_QUEUE_INDEX 0
#define GRAPHICS_QUEUE physicalDevices[device.physicalDeviceIndex].queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0]
#define P_GRAPHICS_QUEUE _physicalDevice.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0]

#define SHADER_VIEW_PROJECTION_UNIFORM_BINDING 0
#define SHADER_MODEL_MATRICES_UNIFORM_BINDING 1
#define SHADER_TEXTURE_UNIFORM_BINDING 2

Renderer* Renderer::currentRenderer;

void Renderer::InitInstance(const char * _appName, uint32_t _appVersion, const char * _engineName, uint32_t _engineVersion, std::vector<const char*> _targetInstanceLayerNames, std::vector<const char*> _targetInstanceExtensionNames)
{
	// Get available instance layers
	uint32_t propertyCount = 0;
	std::vector<VkLayerProperties> availableInstanceLayerNames;
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr), "????????????????", "vkEnumerateInstanceLayerProperties");
	availableInstanceLayerNames.resize(propertyCount);
	VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&propertyCount, availableInstanceLayerNames.data()), "????????????????", "vkEnumerateInstanceLayerProperties");

	// Get available instance extensions
	propertyCount = 0;
	std::vector<VkExtensionProperties> availableInstanceExtensionNames;
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr), "????????????????", "vkEnumerateInstanceExtensionProperties");
	availableInstanceExtensionNames.resize(propertyCount);
	VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, availableInstanceExtensionNames.data()), "????????????????", "vkEnumerateInstanceExtensionProperties");

	// Get available target instance layers
	std::vector<const char*> finalInstanceLayerNames;
	for (size_t i = 0; i != _targetInstanceLayerNames.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j != availableInstanceLayerNames.size(); ++j)
		{
			if (strcmp(_targetInstanceLayerNames[i], availableInstanceLayerNames[j].layerName) == 0)
			{
				found = true;
				finalInstanceLayerNames.push_back(availableInstanceLayerNames[j].layerName);
				break;
			}
		}

#if _DEBUG
		if (found == false)
			logger << "ERROR: Instance Layer" << _targetInstanceLayerNames[i] << " is not available.\n";
#endif
	}

	// Get available target instance extensions
	std::vector<const char*> finalInstanceExtensionNames;
	for (size_t i = 0; i != _targetInstanceExtensionNames.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j != availableInstanceExtensionNames.size(); ++j)
		{
			if (strcmp(_targetInstanceExtensionNames[i], availableInstanceExtensionNames[j].extensionName) == 0)
			{
				found = true;
				finalInstanceExtensionNames.push_back(availableInstanceExtensionNames[j].extensionName);
				break;
			}
		}

#if _DEBUG
		if (found == false)
			logger << "ERROR: Instance Extension" << _targetInstanceExtensionNames[i] << " is not available.\n";
#endif
	}

	// Fill instance creation structures
	VkApplicationInfo applicationInfo;
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = _appName;
	applicationInfo.applicationVersion = _appVersion;
	applicationInfo.pEngineName = _engineName;
	applicationInfo.engineVersion = _engineVersion;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = (uint32_t)finalInstanceLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = finalInstanceLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)finalInstanceExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = finalInstanceExtensionNames.data();

	// Create instance
	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), instance, "vkCreateInstance");
}
#if _DEBUG
void Renderer::InitDebug(VkDebugReportFlagsEXT _debugFlags, VKAPI_ATTR VkBool32(*_debugReportCallback)(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*))
{
	// Creates debug report callback using custom report callback function
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pNext = nullptr;
	debugReportCallbackCreateInfo.flags = _debugFlags;
	debugReportCallbackCreateInfo.pfnCallback = _debugReportCallback;
	debugReportCallbackCreateInfo.pUserData = nullptr;

	PFN_vkCreateDebugReportCallbackEXT FP_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	VK_CHECK_RESULT(FP_vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReportCallback), debugReportCallback, "FP_vkCreateDebugReportCallbackEXT");
}
#endif
void Renderer::InitPhysicalDevices(std::vector<VkFormat>* _preferedAllDepthFormats)
{
	// Get available physical devices
	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &propertyCount, nullptr), "????????????????", "vkEnumeratePhysicalDevices");
	std::vector<VkPhysicalDevice> physicalDevicesHandles(propertyCount);
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &propertyCount, physicalDevicesHandles.data()), "????????????????", "vkEnumeratePhysicalDevices");

	// Get physical devices properties
	physicalDevices.resize(physicalDevicesHandles.size());
	for (size_t i = 0; i != physicalDevicesHandles.size(); ++i)
	{
		physicalDevices[i].Init(physicalDevicesHandles[i], _preferedAllDepthFormats);
	}
}
void Renderer::InitWindow(const char * _windowTitle, const char * _windowName, WNDPROC _wndProc, int _width, int _height)
{
	// Sets up window data
	window.hInstance = GetModuleHandle(NULL);
	window.name = _windowName;
	window.hWnd = NULL;
	window.width = _width;
	window.height = _height;

	if (_wndProc == nullptr)
		_wndProc = VkU::WndProc;

	// Registers window
	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = _wndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = GetModuleHandle(NULL);
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = window.name;
	wndClassEx.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClassEx))
	{
#if _DEBUG
		logger << "Failed to register window " << window.name << " titled " << _windowTitle << ".\n";
#endif
		return;
	}

	// Gets resilution
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Defines window design
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	// Adjusts window size to fit the required client draw space on the window design
	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = (long)_width;
	windowRect.bottom = (long)_height;
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	// Creates window
	currentRenderer = this;
	window.hWnd = CreateWindowEx(
		0,
		window.name,
		_windowTitle,
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	if (window.hWnd == NULL)
	{
#if _DEBUG
		logger << "Failed to register window " << window.name << " titled " << _windowTitle << ".\n";
#endif
		return;
	}

	// Centers window
	uint32_t x = (screenWidth - windowRect.right) / 2;
	uint32_t y = (screenHeight - windowRect.bottom) / 2;
	SetWindowPos(window.hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// Makes window visible and focused
	ShowWindow(window.hWnd, SW_SHOW);
	SetForegroundWindow(window.hWnd);
	SetFocus(window.hWnd);

	// Creates Vulkan window abstraction
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo;
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.pNext = nullptr;
	win32SurfaceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	win32SurfaceCreateInfo.hinstance = window.hInstance;
	win32SurfaceCreateInfo.hwnd = window.hWnd;

	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, nullptr, &surface.handle), surface.handle, "vkCreateWin32SurfaceKHR");
}
void Renderer::PickPhysicalDeviceAndQueue(std::vector<VkU::Queue> _queues, VkPhysicalDeviceFeatures _features)
{
	// Looks at each physical device
	for (size_t i = 0; i != physicalDevices.size(); ++i)
	{
		bool featureCompatible = true;
		bool queueCompatible = false;

		// Loops through each feature value to validate
		VkBool32* features1 = (VkBool32*)&_features;
		VkBool32* features2 = (VkBool32*)&physicalDevices[i].features;
		for (size_t j = 0; j != sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++j)
		{
			if (features1[j] == VK_TRUE && features2[j] == VK_FALSE)
			{
				featureCompatible = false;
				break;
			}
		}
		if (!featureCompatible)
			break; // aborts device early

				   // Tree searches for compatible queues
		physicalDevices[i].queues = _queues;
		physicalDevices[i].queues = VkU::Queue::PickDeviceQueuesIndices(physicalDevices[i].queues, physicalDevices[i], { surface }, &queueCompatible);
		if (!queueCompatible)
			physicalDevices[i].queues.clear();
	}

	// pick from available devices
	for (size_t i = 0; i != physicalDevices.size(); ++i)
	{
		if (physicalDevices[i].queues.size() > 0)
		{
			device.physicalDeviceIndex = (uint32_t)i;
			break;
		}
	}
}
void Renderer::InitDevice(std::vector<const char*> _targetDeviceLayerNames, std::vector<const char*> _targetDeviceExtensionNames, VkPhysicalDeviceFeatures _features, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas, std::vector<VkPresentModeKHR>* _preferedPresentModes)
{
	// List queues for the device
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(physicalDevices[device.physicalDeviceIndex].queues.size());
	for (size_t i = 0; i != deviceQueueCreateInfos.size(); ++i)
	{
		deviceQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[i].pNext = nullptr;
		deviceQueueCreateInfos[i].flags = VK_RESERVED_FOR_FUTURE_USE;
		deviceQueueCreateInfos[i].queueFamilyIndex = physicalDevices[device.physicalDeviceIndex].queues[i].queueFamilyIndex;
		deviceQueueCreateInfos[i].queueCount = physicalDevices[device.physicalDeviceIndex].queues[i].count;
		deviceQueueCreateInfos[i].pQueuePriorities = &physicalDevices[device.physicalDeviceIndex].queues[i].priority;
	}

	// Get available device layers
	uint32_t propertyCount = 0;
	std::vector<VkLayerProperties> availableDeviceLayerProperties;
	VK_CHECK_RESULT(vkEnumerateDeviceLayerProperties(physicalDevices[device.physicalDeviceIndex].handle, &propertyCount, nullptr), "????????????????", "vkEnumerateDeviceLayerProperties");
	availableDeviceLayerProperties.resize(propertyCount);
	VK_CHECK_RESULT(vkEnumerateDeviceLayerProperties(physicalDevices[device.physicalDeviceIndex].handle, &propertyCount, availableDeviceLayerProperties.data()), "????????????????", "vkEnumerateDeviceLayerProperties");

	// Get available device extensions
	propertyCount = 0;
	std::vector<VkExtensionProperties> availableDeviceExtensionProperties;
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevices[device.physicalDeviceIndex].handle, nullptr, &propertyCount, nullptr), "????????????????", "vkEnumerateDeviceExtensionProperties");
	availableDeviceExtensionProperties.resize(propertyCount);
	VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevices[device.physicalDeviceIndex].handle, nullptr, &propertyCount, availableDeviceExtensionProperties.data()), "????????????????", "vkEnumerateDeviceExtensionProperties");

	// Get available target device layers
	std::vector<const char*> finalDeviceLayerNames;
	for (size_t i = 0; i != _targetDeviceLayerNames.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j != availableDeviceLayerProperties.size(); ++j)
		{
			if (strcmp(_targetDeviceLayerNames[i], availableDeviceLayerProperties[j].layerName) == 0)
			{
				found = true;
				finalDeviceLayerNames.push_back(availableDeviceLayerProperties[j].layerName);
				break;
			}
		}

#if _DEBUG
		if (found == false)
			logger << "Device named " << physicalDevices[device.physicalDeviceIndex].properties.deviceName << " is missing requested layer named " << _targetDeviceLayerNames[i] << ".\n";
#endif
	}

	// Get available target device extensions
	std::vector<const char*> finalDeviceExtensionNames;
	for (size_t i = 0; i != _targetDeviceExtensionNames.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j != availableDeviceExtensionProperties.size(); ++j)
		{
			if (strcmp(_targetDeviceExtensionNames[i], availableDeviceExtensionProperties[j].extensionName) == 0)
			{
				found = true;
				finalDeviceExtensionNames.push_back(availableDeviceExtensionProperties[j].extensionName);
				break;
			}
		}

#if _DEBUG
		if (found == false)
			logger << "Device named " << physicalDevices[device.physicalDeviceIndex].properties.deviceName << " is missing requested extension named " << _targetDeviceExtensionNames[i] << ".\n";
#endif
	}

	// Creates device
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.enabledLayerCount = (uint32_t)finalDeviceLayerNames.size();
	deviceCreateInfo.ppEnabledLayerNames = finalDeviceLayerNames.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)finalDeviceExtensionNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = finalDeviceExtensionNames.data();
	deviceCreateInfo.pEnabledFeatures = &_features;

	VK_CHECK_RESULT(vkCreateDevice(physicalDevices[device.physicalDeviceIndex].handle, &deviceCreateInfo, nullptr, &device.handle), device.handle, "vkCreateDevice");

	// Gets device queues
	for (size_t i = 0; i != physicalDevices[device.physicalDeviceIndex].queues.size(); ++i)
	{
		physicalDevices[device.physicalDeviceIndex].queues[i].handles.resize(physicalDevices[device.physicalDeviceIndex].queues[i].count);
		for (size_t j = 0; j != physicalDevices[device.physicalDeviceIndex].queues[i].handles.size(); ++j)
		{
			vkGetDeviceQueue(device.handle, physicalDevices[device.physicalDeviceIndex].queues[i].queueFamilyIndex, physicalDevices[device.physicalDeviceIndex].queues[i].queueIndex + (uint32_t)j, &physicalDevices[device.physicalDeviceIndex].queues[i].handles[j]);
		}
	}

	// Gets surface properties on this device
	surface.colorFormat = VkU::Surface::GetVkSurfaceFormatKHR(physicalDevices[device.physicalDeviceIndex].handle, surface, nullptr);
	surface.compositeAlpha = VkU::Surface::GetVkCompositeAlphaFlagBitsKHR(VkU::Surface::GetVkSurfaceCapabilitiesKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle), _preferedCompositeAlphas);
	surface.presentMode = VkU::Surface::GetVkPresentModeKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle, _preferedPresentModes);
}
void Renderer::InitCommandPool()
{
	// Pool where command buffers are allocated from
	VkCommandPoolCreateInfo commandPoolCreateInfo;
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = physicalDevices[device.physicalDeviceIndex].queues[GRAPHICS_PRESENT_QUEUE_INDEX].queueFamilyIndex;

	VK_CHECK_RESULT(vkCreateCommandPool(device.handle, &commandPoolCreateInfo, nullptr, &commandPool), commandPool, "vkCreateCommandPool");
}
void Renderer::InitCommandBuffers()
{
	// General purpose command buffer
	VkCommandBufferAllocateInfo generalCommandBufferAllocateInfo;
	generalCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	generalCommandBufferAllocateInfo.pNext = nullptr;
	generalCommandBufferAllocateInfo.commandPool = commandPool;
	generalCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	generalCommandBufferAllocateInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device.handle, &generalCommandBufferAllocateInfo, &setupCommandBuffer), setupCommandBuffer, "vkAllocateCommandBuffers");
}
void Renderer::InitFences()
{
	// General purpose fence
	VkFenceCreateInfo generalFenceCreateInfo;
	generalFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	generalFenceCreateInfo.pNext = nullptr;
	generalFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK_RESULT(vkCreateFence(device.handle, &generalFenceCreateInfo, nullptr, &setupFence), setupFence, "vkCreateFence");
}
void Renderer::InitSemaphores()
{
	// Semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

	VK_CHECK_RESULT(vkCreateSemaphore(device.handle, &semaphoreCreateInfo, nullptr, &semaphoreRenderDone), semaphoreRenderDone, "vkCreateSemaphore");
}
void Renderer::InitRenderPass()
{
	// What is done to each attachment during each step
	std::vector<VkAttachmentDescription> attachmentDescriptions(2);
	// Color attachment description
	attachmentDescriptions[0].flags = 0;
	attachmentDescriptions[0].format = surface.colorFormat.format;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment description
	attachmentDescriptions[1].flags = 0;
	attachmentDescriptions[1].format = physicalDevices[device.physicalDeviceIndex].depthFormat;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Reference of attachment descriptions to the subpass description
	VkAttachmentReference colorAttachmentReference;
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Description of each subpass
	std::vector<VkSubpassDescription> subpassDescription(1);
	subpassDescription[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	subpassDescription[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription[0].inputAttachmentCount = 0;
	subpassDescription[0].pInputAttachments = nullptr;
	subpassDescription[0].colorAttachmentCount = 1;
	subpassDescription[0].pColorAttachments = &colorAttachmentReference;
	subpassDescription[0].pResolveAttachments = nullptr;
	subpassDescription[0].pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription[0].preserveAttachmentCount = 0;
	subpassDescription[0].pPreserveAttachments = nullptr;

	// List of dependencies of the sum of subpasses of a specific renderPass
	std::vector<VkSubpassDependency> subpassDependency(1);
	subpassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency[0].dstSubpass = 0;
	subpassDependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency[0].srcAccessMask = 0;
	subpassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency[0].dependencyFlags = 0;

	// RenderPass creation structure
	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescriptions.size();
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = (uint32_t)subpassDescription.size();
	renderPassCreateInfo.pSubpasses = subpassDescription.data();
	renderPassCreateInfo.dependencyCount = (uint32_t)subpassDependency.size();
	renderPassCreateInfo.pDependencies = subpassDependency.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device.handle, &renderPassCreateInfo, nullptr, &renderPass), renderPass, "vkCreateRenderPass");
}
void Renderer::InitSwapchain(uint32_t _targetSwapchainImageCount)
{
	// Get window's surface's capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle, &surfaceCapabilities), "????????????????", "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	// Set Swapchain size to be the same as the surface
	swapchain.extent.width = surfaceCapabilities.currentExtent.width;
	swapchain.extent.height = surfaceCapabilities.currentExtent.height;

	// Cap the target image count to surface's min/max
	if (_targetSwapchainImageCount > surfaceCapabilities.maxImageCount)
		_targetSwapchainImageCount = surfaceCapabilities.maxImageCount;
	else if (_targetSwapchainImageCount < surfaceCapabilities.minImageCount)
		_targetSwapchainImageCount = surfaceCapabilities.minImageCount;

	// Cap the swapchain width & haight to surface's min/max
	if (swapchain.extent.width > surfaceCapabilities.maxImageExtent.width)
		swapchain.extent.width = surfaceCapabilities.maxImageExtent.width;
	else if (swapchain.extent.width < surfaceCapabilities.minImageExtent.width)
		swapchain.extent.width = surfaceCapabilities.minImageExtent.width;
	if (swapchain.extent.height > surfaceCapabilities.maxImageExtent.height)
		swapchain.extent.height = surfaceCapabilities.maxImageExtent.height;
	else if (swapchain.extent.height < surfaceCapabilities.minImageExtent.height)
		swapchain.extent.height = surfaceCapabilities.minImageExtent.height;

	// Swapchain creation structure
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.pNext = nullptr;
	swapchainCreateInfoKHR.flags = VK_RESERVED_FOR_FUTURE_USE;
	swapchainCreateInfoKHR.surface = surface.handle;
	swapchainCreateInfoKHR.minImageCount = _targetSwapchainImageCount;
	swapchainCreateInfoKHR.imageFormat = surface.colorFormat.format;
	swapchainCreateInfoKHR.imageColorSpace = surface.colorFormat.colorSpace;
	swapchainCreateInfoKHR.imageExtent = swapchain.extent;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
	swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = surface.compositeAlpha;
	swapchainCreateInfoKHR.presentMode = surface.presentMode;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device.handle, &swapchainCreateInfoKHR, nullptr, &swapchain.handle), swapchain.handle, "vkCreateSwapchainKHR");

	// Getting swapchain's image's handles
	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, nullptr), "????????????????", "vkGetSwapchainImagesKHR");
	swapchain.images.resize(propertyCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, swapchain.images.data()), "????????????????", "vkGetSwapchainImagesKHR");

	// Swapchain's depth image creation
	swapchain.depthImage.InitHandle(physicalDevices[device.physicalDeviceIndex].depthFormat, { swapchain.extent.width, swapchain.extent.height, 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device.handle);
	swapchain.depthImage.InitMemory(physicalDevices[device.physicalDeviceIndex], device.handle);
	swapchain.depthImage.InitView(physicalDevices[device.physicalDeviceIndex].depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, device.handle);

	// Transition layout
	VkU::BeginCommandBuffer(setupCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = swapchain.depthImage.handle;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), "????????????????", "vkEndCommandBuffer");

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	// wait for fence to be signaled
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkResetFences");
	// resets fence signaling
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
	// calls submit, fence gets signaled once submit is done
	VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, setupFence), "????????????????", "vkQueueSubmit");

	// Swapchain's image's view creation structure
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	imageViewCreateInfo.image = VK_NULL_HANDLE;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = surface.colorFormat.format;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	// Creating views for swapchain's images
	swapchain.views.resize(swapchain.images.size());
	for (size_t i = 0; i != swapchain.views.size(); ++i)
	{
		imageViewCreateInfo.image = swapchain.images[i];
		VK_CHECK_RESULT(vkCreateImageView(device.handle, &imageViewCreateInfo, nullptr, &swapchain.views[i]), swapchain.views[i], "vkCreateImageView");
	}

	// Swapchain's framebuffers creation structure
	VkFramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = nullptr;
	framebufferCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 0;
	framebufferCreateInfo.pAttachments = nullptr;
	framebufferCreateInfo.width = swapchain.extent.width;
	framebufferCreateInfo.height = swapchain.extent.height;
	framebufferCreateInfo.layers = 1;

	swapchain.framebuffers.resize(swapchain.images.size());
	for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
	{
		// One color and one depth attachment, like the renderPass describes
		std::vector<VkImageView> attachments;
		attachments.push_back(swapchain.views[i]);
		attachments.push_back(swapchain.depthImage.view);

		framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();

		VK_CHECK_RESULT(vkCreateFramebuffer(device.handle, &framebufferCreateInfo, nullptr, &swapchain.framebuffers[i]), swapchain.framebuffers[i], "vkCreateFramebuffer");
	}

	// Rendering command buffers
	swapchain.commandBuffers.resize(swapchain.images.size());

	VkCommandBufferAllocateInfo renderCommandBufferAllocateInfo;
	renderCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	renderCommandBufferAllocateInfo.pNext = nullptr;
	renderCommandBufferAllocateInfo.commandPool = commandPool;
	renderCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	renderCommandBufferAllocateInfo.commandBufferCount = (uint32_t)swapchain.commandBuffers.size();

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device.handle, &renderCommandBufferAllocateInfo, swapchain.commandBuffers.data()), "????????????????", "vkAllocateCommandBuffers");

	// Rendering fences
	swapchain.fences.resize(swapchain.images.size());

	VkFenceCreateInfo renderFenceCreateInfo;
	renderFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	renderFenceCreateInfo.pNext = nullptr;
	renderFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i != swapchain.fences.size(); ++i)
	{
		VK_CHECK_RESULT(vkCreateFence(device.handle, &renderFenceCreateInfo, nullptr, &swapchain.fences[i]), swapchain.fences[i], "vkCreateFence");
	}

	// Semaphore to synchronize next swapchain image availability
	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

	VK_CHECK_RESULT(vkCreateSemaphore(device.handle, &semaphoreCreateInfo, nullptr, &swapchain.semaphoreImageAvailable), swapchain.semaphoreImageAvailable, "vkCreateSemaphore");

	// Set Projection to the new size
	viewProjection[1] = glm::perspective(fovProjection, (float)swapchain.extent.width / (float)swapchain.extent.height, nearProjection, farProjection);
}

void Renderer::InitUniformBuffers(uint32_t _maxModelMatrixCount)
{
	// Buffer and staging buffers for view and projection matrices
	size_t viewProjectionBufferSize = sizeof(glm::mat4) * 2;
	viewProjectionBuffer.Init(viewProjectionBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	viewProjectionStaging.Init(viewProjectionBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Buffer and staging buffers for model matrices
	maxModelMatrixCount = _maxModelMatrixCount;
	size_t modelMatricesBufferSize = sizeof(glm::mat4) * _maxModelMatrixCount;
	modelMatrices.resize(maxModelMatrixCount);
	modelMatricesBuffer.Init(modelMatricesBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	modelMatricesStaging.Init(modelMatricesBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}
void Renderer::LoadModels(std::vector<const char*>* _modelNames)
{
	struct LoadedMeshData
	{
		VkU::VERTEX_TYPE	vertexType;
		uint64_t			vertexSize;
		uint8_t*			vertexData;

		uint32_t			indexCount;
		uint64_t			indexSize;
		uint8_t*			indexData;
	};

	// Reads files referenced in _modelNames, and stores the data into meshData
	std::vector<LoadedMeshData> loadedMeshData(_modelNames->size());
	for (size_t i = 0; i != _modelNames->size(); ++i)
	{
		loadedMeshData[i].vertexType = VkU::VERTEX_TYPE::UNKNOWN;
		loadedMeshData[i].vertexSize = 0;
		loadedMeshData[i].vertexData = nullptr;

		loadedMeshData[i].indexCount = 0;
		loadedMeshData[i].indexSize = 0;
		loadedMeshData[i].indexData = nullptr;

		// Assimp related structures and settings
		Assimp::Importer Importer;
		const aiScene* pScene;
		int processSteps = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate; // join all identical vertices and force mesh triangularization

		// If necessary, generate normals, tangets and bitangents. Originals are not replaced. aiProcess_GenSmoothNormals for smooth normals
		bool generateNormals = true;
		if (generateNormals == true)
			processSteps = processSteps | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

		// Read fila using processSteps settings
		pScene = Importer.ReadFile((*_modelNames)[i], processSteps);

		// File validity checks
#if _DEBUG
		if (pScene == nullptr)
			logger << "Model file named " << (*_modelNames)[i] << " is missing/corrupted/not a loadable file/etc.\n";

		if(pScene->mNumMeshes > 1)
			logger << "Model file named " << (*_modelNames)[i] << " has more than one mesh, only one per file is supported.\n";
		else if (pScene->mNumMeshes < 0)
			logger << "Model file named " << (*_modelNames)[i] << " has 0 meshes.\n";

		if(pScene->mMeshes[0]->HasPositions() == false)
			logger << "Model file named " << (*_modelNames)[i] << " mesh[0] has no positions.\n";
		if(pScene->mMeshes[0]->HasFaces() == false)
			logger << "Model file named " << (*_modelNames)[i] << " mesh[0] has no faces.\n";

		if ((pScene->mMeshes[0]->mPrimitiveTypes & aiPrimitiveType_POINT) == aiPrimitiveType_POINT)
			logger << "Model file named " << (*_modelNames)[i] << " mesh[0] has points.\n";
		if ((pScene->mMeshes[0]->mPrimitiveTypes & aiPrimitiveType_LINE) == aiPrimitiveType_LINE)
			logger << "Model file named " << (*_modelNames)[i] << " mesh[0] has lines.\n";
		if ((pScene->mMeshes[0]->mPrimitiveTypes & aiPrimitiveType_POLYGON) == aiPrimitiveType_POLYGON)
			logger << "Model file named " << (*_modelNames)[i] << " mesh[0] has polygons other than triangles.\n";
#endif

		// Get vertex attributes
		if (pScene->mMeshes[0]->HasPositions())
			loadedMeshData[i].vertexType = (VkU::VERTEX_TYPE)(loadedMeshData[i].vertexType | VkU::VERTEX_TYPE::XYZ);
		if (pScene->mMeshes[0]->HasVertexColors(0))
			loadedMeshData[i].vertexType = (VkU::VERTEX_TYPE)(loadedMeshData[i].vertexType | VkU::VERTEX_TYPE::RGB);
		if (pScene->mMeshes[0]->HasTextureCoords(0))
			loadedMeshData[i].vertexType = (VkU::VERTEX_TYPE)(loadedMeshData[i].vertexType | VkU::VERTEX_TYPE::UV);
		if (pScene->mMeshes[0]->HasNormals())
			loadedMeshData[i].vertexType = (VkU::VERTEX_TYPE)(loadedMeshData[i].vertexType | VkU::VERTEX_TYPE::N);
		if (pScene->mMeshes[0]->HasTangentsAndBitangents())
			loadedMeshData[i].vertexType = (VkU::VERTEX_TYPE)(loadedMeshData[i].vertexType | VkU::VERTEX_TYPE::TB);

		uint64_t vertexCount = pScene->mMeshes[0]->mNumVertices;
		loadedMeshData[i].vertexSize = vertexCount * VkU::GetVertexTypeStride(loadedMeshData[i].vertexType);

		// Get index attributes
		loadedMeshData[i].indexCount = pScene->mMeshes[0]->mNumFaces * 3;

		loadedMeshData[i].indexSize = loadedMeshData[i].indexCount * sizeof(uint32_t);

		// Allocate data
		loadedMeshData[i].vertexData = new uint8_t[loadedMeshData[i].vertexSize];
		loadedMeshData[i].indexData = new uint8_t[loadedMeshData[i].indexSize];
		ZeroMemory(loadedMeshData[i].vertexData, loadedMeshData[i].vertexSize);
		ZeroMemory(loadedMeshData[i].indexData, loadedMeshData[i].indexSize);

		// Get data
		uint64_t dataPos = 0;

		// Fill indices
		for (unsigned int j = 0; j != pScene->mMeshes[0]->mNumFaces; ++j)
		{
			const aiFace& face = pScene->mMeshes[0]->mFaces[j];

#if _DEBUG
			if (face.mNumIndices != 3)
				logger << "Model file named " << (*_modelNames)[i] << " mesh[0].face[" << j << "] has " << face.mNumIndices << " indices.\n";
#endif

			memcpy(&loadedMeshData[i].indexData[dataPos], face.mIndices, sizeof(uint32_t) * 3);
			dataPos += sizeof(uint32_t) * 3;
		}

		// Fill vertices
		dataPos = 0;
		for (auto j = 0; j != pScene->mMeshes[0]->mNumVertices; ++j)
		{
			// XY
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::XY)
			{
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mVertices[j].x, sizeof(float) * 2);
				dataPos += sizeof(float) * 2;
			}

			// Z
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::Z)
			{
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mVertices[j].z, sizeof(float) * 1);
				dataPos += sizeof(float) * 1;
			}

			// RGB
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::RGB)
			{
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mColors[j]->r, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
			}

			// UV
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::UV)
			{
				float uv[2] = { pScene->mMeshes[0]->mTextureCoords[0][j].x, -pScene->mMeshes[0]->mTextureCoords[0][j].y };
				memcpy(&loadedMeshData[i].vertexData[dataPos], &uv, sizeof(float) * 2);
				dataPos += sizeof(float) * 2;
			}

			// N
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::N)
			{
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mNormals[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
			}

			// TB
			if (loadedMeshData[i].vertexType & VkU::VERTEX_TYPE::TB)
			{
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mTangents[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
				memcpy(&loadedMeshData[i].vertexData[dataPos], &pScene->mMeshes[0]->mBitangents[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
			}
		}
	}

	// Get vertex and index sizes
	size_t vertexBufferSize = 0;
	size_t indexBufferSize = 0;
	meshOffsets.resize(loadedMeshData.size());
	for (size_t i = 0; i != loadedMeshData.size(); ++i)
	{
		// Stores offsets
		meshOffsets[i].vertexOffset = vertexBufferSize;
		meshOffsets[i].indexOffset = indexBufferSize;
		meshOffsets[i].indexCount = (uint32_t)loadedMeshData[i].indexCount;

		vertexBufferSize += loadedMeshData[i].vertexSize;
		indexBufferSize += loadedMeshData[i].indexSize;
	}

	// Creates the vertex and index buffer
	vertexBuffer.Init(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	indexBuffer.Init(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Fill vertex and index staging buffers
	VkU::Buffer vertexStaging;
	VkU::Buffer indexStaging;
	vertexStaging.Init(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	indexStaging.Init(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, device.handle, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	void* data = nullptr;
	VK_CHECK_RESULT(vkMapMemory(device.handle, vertexStaging.memory, 0, vertexBufferSize, 0, &data), data, "vkMapMemory");
	VkDeviceSize dataOffset = 0;
	for (size_t i = 0; i != loadedMeshData.size(); ++i)
	{
		dataOffset += meshOffsets[i].vertexOffset;
		memcpy((void*)((size_t)data + dataOffset), loadedMeshData[i].vertexData, loadedMeshData[i].vertexSize);
	}
	vkUnmapMemory(device.handle, vertexStaging.memory);

	data = nullptr;
	VK_CHECK_RESULT(vkMapMemory(device.handle, indexStaging.memory, 0, indexBufferSize, 0, &data), data, "vkMapMemory");
	dataOffset = 0;
	for (size_t i = 0; i != loadedMeshData.size(); ++i)
	{
		dataOffset += meshOffsets[i].indexOffset;
		memcpy((void*)((size_t)data + dataOffset), loadedMeshData[i].indexData, loadedMeshData[i].indexSize);
	}
	vkUnmapMemory(device.handle, indexStaging.memory);

	// Transfers staging buffers into the buffers
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
	VkU::BeginCommandBuffer(setupCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copyVertexRegion;
	copyVertexRegion.srcOffset = 0;
	copyVertexRegion.dstOffset = 0;
	copyVertexRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(setupCommandBuffer, vertexStaging.handle, vertexBuffer.handle, 1, &copyVertexRegion);

	VkBufferCopy copyIndexRegion;
	copyIndexRegion.srcOffset = 0;
	copyIndexRegion.dstOffset = 0;
	copyIndexRegion.size = indexBufferSize;
	vkCmdCopyBuffer(setupCommandBuffer, indexStaging.handle, indexBuffer.handle, 1, &copyIndexRegion);

	VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), setupCommandBuffer, "vkEndCommandBuffer");

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, setupFence), 0, "vkQueueSubmit");

	// Delete meshes on ram
	for (size_t i = 0; i != loadedMeshData.size(); ++i)
	{
		delete[] loadedMeshData[i].vertexData;
		delete[] loadedMeshData[i].indexData;
	}

	// Deletes staging buffers
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkWaitForFences");

	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, vertexStaging.handle, nullptr), vertexStaging.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, vertexStaging.memory, nullptr), vertexStaging.memory, "vkFreeMemory");

	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, indexStaging.handle, nullptr), indexStaging.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, indexStaging.memory, nullptr), indexStaging.memory, "vkFreeMemory");
}
void Renderer::LoadImages(std::vector<const char*>* _imageNamesTGA)
{
	struct ImageData
	{
		uint32_t width, height;
		uint8_t channelCount, bytesPerChannel;
		VkDeviceSize size;
		VkFormat format;
		void* data;
	};

	// Reads files referenced in _imageNames, and stores the data into imageData
	std::vector<ImageData> imageData(_imageNamesTGA->size());
	for (size_t i = 0; i != _imageNamesTGA->size(); ++i)
	{
		imageData[i].format = VK_FORMAT_UNDEFINED;
		imageData[i].channelCount = 0;
		imageData[i].bytesPerChannel = 0;
		imageData[i].width = 0;
		imageData[i].height = 0;
		imageData[i].size = 0;
		imageData[i].data = nullptr;

		uint8_t header[12];

		FILE* fTGA;
		fTGA = fopen((*_imageNamesTGA)[i], "rb");

		uint8_t uncompressedTGAcompare[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };

		// File validity checks
		if (fTGA == NULL)
		{
#if _DEBUG
			logger << "TGA image file named " << (*_imageNamesTGA)[i] << " is missing.\n";
#endif
			goto TGAerror;
		}

		if (fread(&header, sizeof(header), 1, fTGA) == 0)
		{
#if _DEBUG
			logger << "TGA image file named " << (*_imageNamesTGA)[i] << " header could not be read.\n";
#endif
			goto TGAerror;
		}

#if _DEBUG
		uint8_t compressedTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };
		if (memcmp(compressedTGAcompare, &header, sizeof(header)) == 0)
		{
			logger << "TGA image file named " << (*_imageNamesTGA)[i] << " compressed TGA is not supported.\n";
			goto TGAerror;
		}

		if (!memcmp(uncompressedTGAcompare, &header, sizeof(header)) == 0)
		{
			logger << "TGA image file named " << (*_imageNamesTGA)[i] << " invalid header.\n";
			goto TGAerror;
		}
#endif

		// Load TGA file
		if (memcmp(uncompressedTGAcompare, &header, sizeof(header)) == 0)
		{
			uint8_t header[6];

			// read inner header
			if (fread(header, sizeof(header), 1, fTGA) == 0)
			{
#if _DEBUG
				logger << "TGA image file named " << (*_imageNamesTGA)[i] << " header2 could not be read.\n";
#endif
				goto TGAerror;
			}

			// Get values from header
			imageData[i].width = header[1] * 256 + header[0];
			imageData[i].height = header[3] * 256 + header[2];
			uint8_t bpp = header[4];

			// Chack values validity
			if (imageData[i].width == 0 || imageData[i].height == 0 || (bpp != 24 && bpp != 32))
			{
#if _DEBUG
				logger << "TGA image file named " << (*_imageNamesTGA)[i] << " width or height is 0, or bits per pixel is not 24 ro 32. Width = " << imageData[i].width << " Height = " << imageData[i].height << " BPP = " << (uint32_t)bpp << "\n";
#endif
				goto TGAerror;
			}

			// Extrapolate cannelCount and BytesPerChannel from bpp
			if (bpp == 24)
			{
				imageData[i].channelCount = 3;
				imageData[i].bytesPerChannel = 1;
				imageData[i].format = VK_FORMAT_B8G8R8_UNORM;
			}
			else if (bpp == 32)
			{
				imageData[i].channelCount = 4;
				imageData[i].bytesPerChannel = 1;
				imageData[i].format = VK_FORMAT_B8G8R8A8_UNORM;
			}

			// Calculate size
			imageData[i].size = imageData[i].channelCount * imageData[i].bytesPerChannel * imageData[i].width * imageData[i].height;

			// Allocate and read data
			imageData[i].data = new uint8_t[imageData[i].size];
			if (fread(imageData[i].data, 1, imageData[i].size, fTGA) != imageData[i].size)
			{
#if _DEBUG
				logger << "TGA image file named " << (*_imageNamesTGA)[i] << " could not read file's data properly, size mismatch.\n";
#endif
				goto TGAerror;
			}
		}
		continue;

		// Clean up in case of failure
	TGAerror:
		imageData[i].format = VK_FORMAT_UNDEFINED;
		imageData[i].channelCount = 0;
		imageData[i].bytesPerChannel = 0;
		imageData[i].width = 0;
		imageData[i].height = 0;
		imageData[i].size = 0;
		delete[] imageData[i].data;
		imageData[i].data = nullptr;
	}

	textures.resize(imageData.size());
	std::vector<VkU::StagingImage> stagingImages(imageData.size());
	for (size_t i = 0; i != imageData.size(); ++i)
	{
		// Transfers data to the GPU
		VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), 0, "vkWaitForFences");
		VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
		VkU::BeginCommandBuffer(setupCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// Image
		textures[i].InitHandle(imageData[i].format, { imageData[i].width, imageData[i].height, 1 }, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, device.handle);
		textures[i].InitMemory(physicalDevices[device.physicalDeviceIndex], device.handle);
		// View
		textures[i].InitView(imageData[i].format, VK_IMAGE_ASPECT_COLOR_BIT, device.handle);

		// Staging
		stagingImages[i].InitHandle(imageData[i].format, { imageData[i].width, imageData[i].height, 1 }, device.handle);
		stagingImages[i].InitMemory(physicalDevices[device.physicalDeviceIndex], device.handle);
		stagingImages[i].FillImageMemory(imageData[i].width, imageData[i].height, imageData[i].size, imageData[i].data, device.handle);

		delete[] imageData[i].data;

		// Transition
		VkImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = textures[i].handle;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		VkImageMemoryBarrier stagingImageMemoryBarrier;
		stagingImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		stagingImageMemoryBarrier.pNext = nullptr;
		stagingImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		stagingImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		stagingImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		stagingImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		stagingImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		stagingImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		stagingImageMemoryBarrier.image = stagingImages[i].handle;
		stagingImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		stagingImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		stagingImageMemoryBarrier.subresourceRange.levelCount = 1;
		stagingImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		stagingImageMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingImageMemoryBarrier);

		VkImageSubresourceLayers imageSubresourceLayers;
		imageSubresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresourceLayers.mipLevel = 0;
		imageSubresourceLayers.baseArrayLayer = 0;
		imageSubresourceLayers.layerCount = 1;

		VkImageCopy imageCopy;
		imageCopy.srcSubresource = imageSubresourceLayers;
		imageCopy.srcOffset = { 0, 0, 0 };
		imageCopy.dstSubresource = imageSubresourceLayers;
		imageCopy.dstOffset = { 0, 0, 0 };
		imageCopy.extent.width = imageData[i].width;
		imageCopy.extent.height = imageData[i].height;
		imageCopy.extent.depth = 1;
		vkCmdCopyImage(setupCommandBuffer, stagingImages[i].handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textures[i].handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

		VkImageMemoryBarrier finalMemoryBarrier;
		finalMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		finalMemoryBarrier.pNext = nullptr;
		finalMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		finalMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		finalMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		finalMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		finalMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		finalMemoryBarrier.image = textures[i].handle;
		finalMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		finalMemoryBarrier.subresourceRange.baseMipLevel = 0;
		finalMemoryBarrier.subresourceRange.levelCount = 1;
		finalMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		finalMemoryBarrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalMemoryBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), 0, "vkEndCommandBuffer");

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &setupCommandBuffer;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, setupFence), GRAPHICS_QUEUE, "vkQueueSubmit");
		// Wait for textures to be transfered
		VK_CHECK_RESULT(vkQueueWaitIdle(GRAPHICS_QUEUE), GRAPHICS_QUEUE, "vkQueueWaitIdle");
	}
	
	// Cleanup
	for (size_t i = 0; i != stagingImages.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkFreeMemory(device.handle, stagingImages[i].memory, nullptr), stagingImages[i].memory, "vkFreeMemory");
		VK_CHECK_CLEANUP(vkDestroyImage(device.handle, stagingImages[i].handle, nullptr), stagingImages[i].handle, "vkDestroyImage");
	}
}
void Renderer::InitSamplers()
{
	// Linear filtering sampler
	VkSamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;//VK_FILTER_NEAREST
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VK_CHECK_RESULT(vkCreateSampler(device.handle, &samplerCreateInfo, nullptr, &sampler), sampler, "vkCreateSampler");
}

void Renderer::InitDescriptorPool(uint32_t _combinedImageSamplerCount, uint32_t _uniformBufferCount, uint32_t _storageBufferCount)
{
	// Pool needs to allocate space for each descriptor type (image, uniform, and storage)
	std::vector<VkDescriptorPoolSize> descriptorPoolSize(3);
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize[0].descriptorCount = _combinedImageSamplerCount;

	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[1].descriptorCount = _uniformBufferCount;

	descriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[2].descriptorCount = _storageBufferCount;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.maxSets = 4;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)descriptorPoolSize.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize.data();

	VK_CHECK_RESULT(vkCreateDescriptorPool(device.handle, &descriptorPoolCreateInfo, nullptr, &descriptorPool), descriptorPool, "vkCreateDescriptorPool");
}
void Renderer::InitDescriptorSetLayouts()
{
	// Camera descriptor
	VkDescriptorSetLayoutBinding cameraDescriptorSetLayoutBinding;
	cameraDescriptorSetLayoutBinding.binding = 0;
	cameraDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraDescriptorSetLayoutBinding.descriptorCount = 1;
	cameraDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	cameraDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	// Model matrices descriptor
	VkDescriptorSetLayoutBinding modelMatricesDescriptorSetLayoutBinding;
	modelMatricesDescriptorSetLayoutBinding.binding = 1;
	modelMatricesDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelMatricesDescriptorSetLayoutBinding.descriptorCount = 1;
	modelMatricesDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelMatricesDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	// Texture descriptor
	VkDescriptorSetLayoutBinding textureDescriptorSetLayoutBinding;
	textureDescriptorSetLayoutBinding.binding = 2;
	textureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureDescriptorSetLayoutBinding.descriptorCount = 1;
	textureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	textureDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;
	
	// Descriptor set layout creation structure
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = nullptr;
	descriptorSetLayoutCreateInfo.flags = VK_NULL_HANDLE;

	// MVP layout
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingPipeline[] = { cameraDescriptorSetLayoutBinding, modelMatricesDescriptorSetLayoutBinding };
	descriptorSetLayoutCreateInfo.bindingCount = sizeof(descriptorSetLayoutBindingPipeline) / sizeof(VkDescriptorSetLayoutBinding);
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindingPipeline;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.handle, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayoutMVP), descriptorSetLayoutMVP, "vkCreateDescriptorSetLayout");

	// Texture layout
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingTexture[] = { textureDescriptorSetLayoutBinding };
	descriptorSetLayoutCreateInfo.bindingCount = sizeof(descriptorSetLayoutBindingTexture) / sizeof(VkDescriptorSetLayoutBinding);
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindingTexture;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.handle, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayoutTexture), descriptorSetLayoutTexture, "vkCreateDescriptorSetLayout");
}
void Renderer::InitDescriptorSets()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = nullptr;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;

	// MVP
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayoutMVP;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device.handle, &descriptorSetAllocateInfo, &descriptorSetMVP), descriptorSetMVP, "vkAllocateDescriptorSets");

	// One texture descriptorSet per texture
	descriptorSetsTexture.resize(textures.size());
	for (size_t i = 0; i != descriptorSetsTexture.size(); ++i)
	{
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayoutTexture;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device.handle, &descriptorSetAllocateInfo, &descriptorSetsTexture[i]), descriptorSetsTexture[i], "vkAllocateDescriptorSets");
	}
}
void Renderer::UpdateDescriptorSets()
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets(2 + descriptorSetsTexture.size());

	// Write for view projection matrices
	VkDescriptorBufferInfo viewProjectionDescriptorBufferInfo;
	viewProjectionDescriptorBufferInfo.buffer = viewProjectionBuffer.handle;
	viewProjectionDescriptorBufferInfo.offset = 0;
	viewProjectionDescriptorBufferInfo.range = sizeof(viewProjection);

	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].pNext = nullptr;
	writeDescriptorSets[0].dstSet = descriptorSetMVP;
	writeDescriptorSets[0].dstBinding = SHADER_VIEW_PROJECTION_UNIFORM_BINDING;
	writeDescriptorSets[0].dstArrayElement = 0;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[0].pImageInfo = nullptr;
	writeDescriptorSets[0].pBufferInfo = &viewProjectionDescriptorBufferInfo;
	writeDescriptorSets[0].pTexelBufferView = nullptr;

	// Model matrices
	VkDescriptorBufferInfo modelMatricesDescriptorBufferInfo;
	modelMatricesDescriptorBufferInfo.buffer = modelMatricesBuffer.handle;
	modelMatricesDescriptorBufferInfo.offset = 0;
	modelMatricesDescriptorBufferInfo.range = sizeof(glm::mat4) * modelMatrices.size();

	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].pNext = nullptr;
	writeDescriptorSets[1].dstSet = descriptorSetMVP;
	writeDescriptorSets[1].dstBinding = SHADER_MODEL_MATRICES_UNIFORM_BINDING;
	writeDescriptorSets[1].dstArrayElement = 0;
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[1].pImageInfo = nullptr;
	writeDescriptorSets[1].pBufferInfo = &modelMatricesDescriptorBufferInfo;
	writeDescriptorSets[1].pTexelBufferView = nullptr;

	// "Binds" each descriptor to a texture
	std::vector<VkDescriptorImageInfo> descriptorImageInfos(descriptorSetsTexture.size());
	for (size_t i = 0; i != descriptorSetsTexture.size(); ++i)
	{
		descriptorImageInfos[i].sampler = sampler;
		descriptorImageInfos[i].imageView = textures[i].view;
		descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		writeDescriptorSets[i + 2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[i + 2].pNext = nullptr;
		writeDescriptorSets[i + 2].dstSet = descriptorSetsTexture[i];
		writeDescriptorSets[i + 2].dstBinding = SHADER_TEXTURE_UNIFORM_BINDING;
		writeDescriptorSets[i + 2].dstArrayElement = 0;
		writeDescriptorSets[i + 2].descriptorCount = 1;
		writeDescriptorSets[i + 2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[i + 2].pImageInfo = &descriptorImageInfos[i];
		writeDescriptorSets[i + 2].pBufferInfo = nullptr;
		writeDescriptorSets[i + 2].pTexelBufferView = nullptr;
	}

	vkUpdateDescriptorSets(device.handle, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void Renderer::RecreateSwapchain(int _width, int _height)
{
	if (swapchain.handle == VK_NULL_HANDLE)
		return;

	uint32_t targetSwapchainImageCount = (uint32_t)swapchain.images.size();

	// Get window's surface's capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle, &surfaceCapabilities), "????????????????", "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	if (_width == 0 || _height == 0)
	{
		swapchain.extent.width = surfaceCapabilities.currentExtent.width;
		swapchain.extent.height = surfaceCapabilities.currentExtent.height;
	}
	else
	{
		swapchain.extent.width = _width;
		swapchain.extent.height = _height;
	}

	if (swapchain.extent.width == 0 || swapchain.extent.height == 0)
		return;

	// Cap the target image count to surface's min/max
	if (targetSwapchainImageCount > surfaceCapabilities.maxImageCount)
		targetSwapchainImageCount = surfaceCapabilities.maxImageCount;
	else if (targetSwapchainImageCount < surfaceCapabilities.minImageCount)
		targetSwapchainImageCount = surfaceCapabilities.minImageCount;

	// Cap the swapchain width & haight to surface's min/max
	if (swapchain.extent.width > surfaceCapabilities.maxImageExtent.width)
		swapchain.extent.width = surfaceCapabilities.maxImageExtent.width;
	else if (swapchain.extent.width < surfaceCapabilities.minImageExtent.width)
		swapchain.extent.width = surfaceCapabilities.minImageExtent.width;
	if (swapchain.extent.height > surfaceCapabilities.maxImageExtent.height)
		swapchain.extent.height = surfaceCapabilities.maxImageExtent.height;
	else if (swapchain.extent.height < surfaceCapabilities.minImageExtent.height)
		swapchain.extent.height = surfaceCapabilities.minImageExtent.height;

	VkSwapchainKHR oldSwapchain = swapchain.handle;

	// Swapchain creation structure
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
	swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.pNext = nullptr;
	swapchainCreateInfoKHR.flags = VK_RESERVED_FOR_FUTURE_USE;
	swapchainCreateInfoKHR.surface = surface.handle;
	swapchainCreateInfoKHR.minImageCount = targetSwapchainImageCount;
	swapchainCreateInfoKHR.imageFormat = surface.colorFormat.format;
	swapchainCreateInfoKHR.imageColorSpace = surface.colorFormat.colorSpace;
	swapchainCreateInfoKHR.imageExtent = swapchain.extent;
	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
	swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;
	swapchainCreateInfoKHR.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = surface.compositeAlpha;
	swapchainCreateInfoKHR.presentMode = surface.presentMode;
	swapchainCreateInfoKHR.clipped = VK_TRUE;
	swapchainCreateInfoKHR.oldSwapchain = oldSwapchain;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device.handle, &swapchainCreateInfoKHR, nullptr, &swapchain.handle), swapchain.handle, "vkCreateSwapchainKHR");

	// Destroy obsolete old swapchain structures
	// Fence
	VK_CHECK_RESULT(vkWaitForFences(device.handle, (uint32_t)swapchain.fences.size(), swapchain.fences.data(), VK_TRUE, -1), "????????????????", "vkResetFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, (uint32_t)swapchain.fences.size(), swapchain.fences.data()), "????????????????", "vkResetFences");
	for (size_t i = 0; i != swapchain.fences.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFence(device.handle, swapchain.fences[i], nullptr), swapchain.fences[i], "vkDestroyFence");
	}
	swapchain.fences.clear();
	// CommandBuffer
	vkFreeCommandBuffers(device.handle, commandPool, (uint32_t)swapchain.commandBuffers.size(), swapchain.commandBuffers.data());
	swapchain.commandBuffers.clear();
	// FrameBuffer
	for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFramebuffer(device.handle, swapchain.framebuffers[i], nullptr), swapchain.framebuffers[i], "vkDestroyFramebuffer");
	}
	swapchain.framebuffers.clear();
	// Views
	for (size_t i = 0; i != swapchain.views.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, swapchain.views[i], nullptr), swapchain.views[i], "vkDestroyImageView");
	}
	swapchain.views.clear();
	// Depth
	VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, swapchain.depthImage.view, nullptr), swapchain.depthImage.view, "vkDestroyImageView");
	VK_CHECK_CLEANUP(vkDestroyImage(device.handle, swapchain.depthImage.handle, nullptr), swapchain.depthImage.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, swapchain.depthImage.memory, nullptr), swapchain.depthImage.memory, "vkFreeMemory");
	// OldSwapChain
	VK_CHECK_CLEANUP(vkDestroySwapchainKHR(device.handle, oldSwapchain, nullptr), oldSwapchain, "vkDestroySwapchainKHR");

	// Getting swapchain's image's handles
	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, nullptr), "????????????????", "vkGetSwapchainImagesKHR");
	swapchain.images.resize(propertyCount);
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, swapchain.images.data()), "????????????????", "vkGetSwapchainImagesKHR");

	// Swapchain's depth image creation
	swapchain.depthImage.InitHandle(physicalDevices[device.physicalDeviceIndex].depthFormat, { swapchain.extent.width, swapchain.extent.height, 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device.handle);
	swapchain.depthImage.InitMemory(physicalDevices[device.physicalDeviceIndex], device.handle);
	swapchain.depthImage.InitView(physicalDevices[device.physicalDeviceIndex].depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, device.handle);

	// Transition layout
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
	VkU::BeginCommandBuffer(setupCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = swapchain.depthImage.handle;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), "????????????????", "vkEndCommandBuffer");

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, setupFence), "????????????????", "vkQueueSubmit");

	// Swapchain's image's view creation structure
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	imageViewCreateInfo.image = VK_NULL_HANDLE;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = surface.colorFormat.format;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	// Creating views for swapchain's images
	swapchain.views.resize(swapchain.images.size());
	for (size_t i = 0; i != swapchain.views.size(); ++i)
	{
		imageViewCreateInfo.image = swapchain.images[i];
		VK_CHECK_RESULT(vkCreateImageView(device.handle, &imageViewCreateInfo, nullptr, &swapchain.views[i]), swapchain.views[i], "vkCreateImageView");
	}

	// Swapchain's framebuffers creation structure
	VkFramebufferCreateInfo framebufferCreateInfo;
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = nullptr;
	framebufferCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 0;
	framebufferCreateInfo.pAttachments = nullptr;
	framebufferCreateInfo.width = swapchain.extent.width;
	framebufferCreateInfo.height = swapchain.extent.height;
	framebufferCreateInfo.layers = 1;

	swapchain.framebuffers.resize(swapchain.images.size());
	for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
	{
		// One color and one depth attachment, like the renderPass describes
		std::vector<VkImageView> attachments;
		attachments.push_back(swapchain.views[i]);
		attachments.push_back(swapchain.depthImage.view);

		framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();

		VK_CHECK_RESULT(vkCreateFramebuffer(device.handle, &framebufferCreateInfo, nullptr, &swapchain.framebuffers[i]), swapchain.framebuffers[i], "vkCreateFramebuffer");
	}

	// Rendering command buffers
	swapchain.commandBuffers.resize(swapchain.images.size());

	VkCommandBufferAllocateInfo renderCommandBufferAllocateInfo;
	renderCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	renderCommandBufferAllocateInfo.pNext = nullptr;
	renderCommandBufferAllocateInfo.commandPool = commandPool;
	renderCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	renderCommandBufferAllocateInfo.commandBufferCount = (uint32_t)swapchain.commandBuffers.size();

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device.handle, &renderCommandBufferAllocateInfo, swapchain.commandBuffers.data()), "????????????????", "vkAllocateCommandBuffers");

	// Rendering fences
	swapchain.fences.resize(swapchain.images.size());

	VkFenceCreateInfo renderFenceCreateInfo;
	renderFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	renderFenceCreateInfo.pNext = nullptr;
	renderFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i != swapchain.fences.size(); ++i)
	{
		VK_CHECK_RESULT(vkCreateFence(device.handle, &renderFenceCreateInfo, nullptr, &swapchain.fences[i]), swapchain.fences[i], "vkCreateFence");
	}

	// Update projection to the new size
	viewProjection[1] = glm::perspective(fovProjection, (float)swapchain.extent.width / (float)swapchain.extent.height, nearProjection, farProjection);

	// Update Pipeline Viewport and Scissor
	viewports[0].width = (float)swapchain.extent.width;
	viewports[0].height = (float)swapchain.extent.height;

	scissors[0].extent = swapchain.extent;

	// Destroy old Graphics Pipelines
	for (size_t i = 0; i != graphicsPipelines.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyPipeline(device.handle, graphicsPipelines[i], nullptr), graphicsPipelines[i], "vkDestroyPipeline");
	}
	graphicsPipelines.clear();

	// Creates graphics pipelines with the updated data
	graphicsPipelines.resize(graphicsPipelineCreateInfos.size());
	if (graphicsPipelines.size() > 0)
	{
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, (uint32_t)graphicsPipelineCreateInfos.size(), graphicsPipelineCreateInfos.data(), nullptr, graphicsPipelines.data()), "????????????????", " - vkCreateGraphicsPipelines");
	}
}

void Renderer::InitPipelineLayout()
{
	// Push constant with model matrix index and 3 extra floats of space
	VkPushConstantRange vertexShaderPushConstantRange;
	vertexShaderPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderPushConstantRange.offset = 0;
	vertexShaderPushConstantRange.size = sizeof(float) * 4;

	// 0 set with MVP, 1 with texture
	VkDescriptorSetLayout descriptorSetLayouts[] = { descriptorSetLayoutMVP, descriptorSetLayoutTexture };

	// Pipeline Layout creation structure
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineLayoutCreateInfo.setLayoutCount = sizeof(descriptorSetLayouts) / sizeof(VkDescriptorSetLayout);
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &vertexShaderPushConstantRange;

	VK_CHECK_RESULT(vkCreatePipelineLayout(device.handle, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), pipelineLayout, "vkCreatePipelineLayout");
}
void Renderer::LoadShaderModules(std::vector<VkU::ShaderData> _shaderModulesData)
{
	// Load spv and creates shader module
	shaderModules.resize(_shaderModulesData.size());
	for (size_t i = 0; i != _shaderModulesData.size(); ++i)
	{
		// Load SPV
		size_t fileSize = 0;
		char* buffer = nullptr;

		VkU::ShaderModule::LoadSpvCode(_shaderModulesData[i].filename, fileSize, &buffer);

		// Shader module creation structure
		VkShaderModuleCreateInfo shaderModuleCreateInfo;
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pNext = nullptr;
		shaderModuleCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		shaderModuleCreateInfo.codeSize = fileSize;
		shaderModuleCreateInfo.pCode = (uint32_t*)buffer;
		VK_CHECK_RESULT(vkCreateShaderModule(device.handle, &shaderModuleCreateInfo, nullptr, &shaderModules[i].handle), shaderModules[i].handle, "vkCreateShaderModule");

		shaderModules[i].stage = _shaderModulesData[i].stage;
		shaderModules[i].entryPointName = _shaderModulesData[i].entryPointName;

		// Cleanup
		delete[] buffer;
	}
}
void Renderer::InitPipelineData()
{
	// Agregates shader modules
	pipelineShaderStagesCreateInfos.resize(1);

	pipelineShaderStagesCreateInfos[0].resize(2);

	pipelineShaderStagesCreateInfos[0][0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStagesCreateInfos[0][0].pNext = nullptr;
	pipelineShaderStagesCreateInfos[0][0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineShaderStagesCreateInfos[0][0].stage = shaderModules[0].stage;
	pipelineShaderStagesCreateInfos[0][0].module = shaderModules[0].handle;
	pipelineShaderStagesCreateInfos[0][0].pName = shaderModules[0].entryPointName;
	pipelineShaderStagesCreateInfos[0][0].pSpecializationInfo = nullptr;

	pipelineShaderStagesCreateInfos[0][1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStagesCreateInfos[0][1].pNext = nullptr;
	pipelineShaderStagesCreateInfos[0][1].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineShaderStagesCreateInfos[0][1].stage = shaderModules[1].stage;
	pipelineShaderStagesCreateInfos[0][1].module = shaderModules[1].handle;
	pipelineShaderStagesCreateInfos[0][1].pName = shaderModules[1].entryPointName;
	pipelineShaderStagesCreateInfos[0][1].pSpecializationInfo = nullptr;

	// Vertex input rate abd stride
	vertexInputBindingDescriptions.resize(1);

	vertexInputBindingDescriptions[0].binding = 0;
	vertexInputBindingDescriptions[0].stride = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::XYZ_UV_NTB);
	vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Vertex attributes format & offset
	vertexInputAttributeDescriptions.resize(1);

	vertexInputAttributeDescriptions[0].resize(5);

	// XYZ
	vertexInputAttributeDescriptions[0][0].location = 0;
	vertexInputAttributeDescriptions[0][0].binding = 0;
	vertexInputAttributeDescriptions[0][0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0][0].offset = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::UNKNOWN);

	// UV
	vertexInputAttributeDescriptions[0][1].location = 1;
	vertexInputAttributeDescriptions[0][1].binding = 0;
	vertexInputAttributeDescriptions[0][1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescriptions[0][1].offset = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::XYZ);

	// N
	vertexInputAttributeDescriptions[0][2].location = 2;
	vertexInputAttributeDescriptions[0][2].binding = 0;
	vertexInputAttributeDescriptions[0][2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0][2].offset = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::XYZ_UV);

	// T
	vertexInputAttributeDescriptions[0][3].location = 3;
	vertexInputAttributeDescriptions[0][3].binding = 0;
	vertexInputAttributeDescriptions[0][3].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0][3].offset = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::XYZ_UV_N);

	// B
	vertexInputAttributeDescriptions[0][4].location = 4;
	vertexInputAttributeDescriptions[0][4].binding = 0;
	vertexInputAttributeDescriptions[0][4].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0][4].offset = VkU::GetVertexTypeStride(VkU::VERTEX_TYPE::XYZ_UV_NT);

	// Vertex binding description
	pipelineVertexInputStateCreateInfos.resize(1);

	pipelineVertexInputStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfos[0].pNext = nullptr;
	pipelineVertexInputStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineVertexInputStateCreateInfos[0].vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfos[0].pVertexBindingDescriptions = &vertexInputBindingDescriptions[0];
	pipelineVertexInputStateCreateInfos[0].vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions[0].size();
	pipelineVertexInputStateCreateInfos[0].pVertexAttributeDescriptions = vertexInputAttributeDescriptions[0].data();

	// Topology and restart
	pipelineInputAssemblyStateCreateInfos.resize(1);
	pipelineInputAssemblyStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfos[0].pNext = nullptr;
	pipelineInputAssemblyStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineInputAssemblyStateCreateInfos[0].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfos[0].primitiveRestartEnable = VK_FALSE;

	// ?
	pipelineTessellationStateCreateInfos;

	// Render area
	viewports.resize(1);

	viewports[0].x = 0.0f;
	viewports[0].y = 0.0f;
	viewports[0].width = (float)swapchain.extent.width;
	viewports[0].height = (float)swapchain.extent.height;
	viewports[0].minDepth = 0.0f;
	viewports[0].maxDepth = 1.0f;

	// Cutout
	scissors.resize(1);

	scissors[0].offset = { 0, 0 };
	scissors[0].extent = swapchain.extent;

	// Sum render areas
	pipelineViewportStateCreateInfos.resize(1);

	pipelineViewportStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfos[0].pNext = nullptr;
	pipelineViewportStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineViewportStateCreateInfos[0].viewportCount = 1;
	pipelineViewportStateCreateInfos[0].pViewports = &viewports[0];
	pipelineViewportStateCreateInfos[0].scissorCount = 1;
	pipelineViewportStateCreateInfos[0].pScissors = &scissors[0];

	// Polygon fill, Culling, Front face
	pipelineRasterizationStateCreateInfos.resize(1);

	pipelineRasterizationStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfos[0].pNext = nullptr;
	pipelineRasterizationStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineRasterizationStateCreateInfos[0].depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfos[0].rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfos[0].polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfos[0].cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfos[0].frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfos[0].depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfos[0].depthBiasConstantFactor = 0.0f;
	pipelineRasterizationStateCreateInfos[0].depthBiasClamp = 0.0f;
	pipelineRasterizationStateCreateInfos[0].depthBiasSlopeFactor = 0.0f;
	pipelineRasterizationStateCreateInfos[0].lineWidth = 1.0f;

	// ?
	pipelineMultisampleStateCreateInfos.resize(1);

	pipelineMultisampleStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfos[0].pNext = nullptr;
	pipelineMultisampleStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineMultisampleStateCreateInfos[0].rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfos[0].sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfos[0].minSampleShading = 0.0f;
	pipelineMultisampleStateCreateInfos[0].pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfos[0].alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfos[0].alphaToOneEnable = VK_FALSE;

	// Depth test
	pipelineDepthStencilStateCreateInfos.resize(1);

	pipelineDepthStencilStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfos[0].pNext = nullptr;
	pipelineDepthStencilStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineDepthStencilStateCreateInfos[0].depthTestEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfos[0].depthWriteEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfos[0].depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfos[0].depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfos[0].stencilTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfos[0].front = {};
	pipelineDepthStencilStateCreateInfos[0].back = {};
	pipelineDepthStencilStateCreateInfos[0].minDepthBounds = 0.0f;
	pipelineDepthStencilStateCreateInfos[0].maxDepthBounds = 1.0f;

	// Will it blend? if so how?
	pipelineColorBlendAttachmentState.resize(1);

	pipelineColorBlendAttachmentState[0].blendEnable = VK_TRUE;
	pipelineColorBlendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineColorBlendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	// ?
	pipelineColorBlendStateCreateInfos.resize(1);

	pipelineColorBlendStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfos[0].pNext = nullptr;
	pipelineColorBlendStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	pipelineColorBlendStateCreateInfos[0].logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfos[0].logicOp = VK_LOGIC_OP_CLEAR;
	pipelineColorBlendStateCreateInfos[0].attachmentCount = 1;
	pipelineColorBlendStateCreateInfos[0].pAttachments = &pipelineColorBlendAttachmentState[0];
	pipelineColorBlendStateCreateInfos[0].blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfos[0].blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfos[0].blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfos[0].blendConstants[3] = 0.0f;

	// What can be changed without recreating the pipeline
	//pipelineDynamicStateCreateInfo.resize(1);
	//
	//pipelineDynamicStateCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//pipelineDynamicStateCreateInfo[0].pNext = nullptr;
	//pipelineDynamicStateCreateInfo[0].flags = VK_RESERVED_FOR_FUTURE_USE;
	//pipelineDynamicStateCreateInfo[0].dynamicStateCount = 1;
	//pipelineDynamicStateCreateInfo[0].pDynamicStates;

	// Graphics pipeline creation structure
	graphicsPipelineCreateInfos.resize(1);

	graphicsPipelineCreateInfos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfos[0].pNext = nullptr;
	graphicsPipelineCreateInfos[0].flags = 0;
	graphicsPipelineCreateInfos[0].stageCount = (uint32_t)pipelineShaderStagesCreateInfos[0].size();
	graphicsPipelineCreateInfos[0].pStages = pipelineShaderStagesCreateInfos[0].data();
	graphicsPipelineCreateInfos[0].pVertexInputState = &pipelineVertexInputStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pInputAssemblyState = &pipelineInputAssemblyStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pTessellationState = nullptr;
	graphicsPipelineCreateInfos[0].pViewportState = &pipelineViewportStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pRasterizationState = &pipelineRasterizationStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pMultisampleState = &pipelineMultisampleStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pDepthStencilState = &pipelineDepthStencilStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pColorBlendState = &pipelineColorBlendStateCreateInfos[0];
	graphicsPipelineCreateInfos[0].pDynamicState = nullptr;
	graphicsPipelineCreateInfos[0].layout = pipelineLayout;
	graphicsPipelineCreateInfos[0].renderPass = renderPass;
	graphicsPipelineCreateInfos[0].subpass = 0;
	graphicsPipelineCreateInfos[0].basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfos[0].basePipelineIndex = 0;

	// Compute pipeline creation structure
	computePipelineCreateInfos;
}
void Renderer::InitPipeline()
{
	// Creates graphics pipelines from the preset data
	graphicsPipelines.resize(graphicsPipelineCreateInfos.size());
	if (graphicsPipelines.size() > 0)
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, (uint32_t)graphicsPipelineCreateInfos.size(), graphicsPipelineCreateInfos.data(), nullptr, graphicsPipelines.data()), "????????????????", " - vkCreateGraphicsPipelines");

	// Creates compute pipelines from the preset data
	computePipelines.resize(computePipelineCreateInfos.size());
	if(computePipelines.size() > 0)
		VK_CHECK_RESULT(vkCreateComputePipelines(device.handle, VK_NULL_HANDLE, (uint32_t)computePipelineCreateInfos.size(), computePipelineCreateInfos.data(), nullptr, computePipelines.data()), "????????????????", " - vkCreateComputePipelines");
}

void Renderer::Init(uint32_t _maxModelMatrixCount, std::vector<const char*>* _modelNames, std::vector<const char*>* _imageNamesTGA)
{
#if _DEBUG
	logger.Start("_RendererLogger.txt");
	debugReportCallbackLogger.Start("_DebugReportCallbackLogger.txt");
	cleanUpLogger.Start("_RendererCleanUpLogger.txt");
#endif

	nearProjection = 0.1f;
	farProjection = 1000.0f;
	fovProjection = glm::radians(45.0f);

	/// Instance
	InitInstance(
		"App Name", 0,
		"Engine Name", 0,
		{
			"VK_LAYER_LUNARG_swapchain",
#if _DEBUG
			//"VK_LAYER_RENDERDOC_Capture",
			"VK_LAYER_LUNARG_standard_validation",
#endif
		},
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
#if _DEBUG
			"VK_EXT_debug_report",
#endif
		});
	/// Debug
#if _DEBUG
	InitDebug(
		VK_DEBUG_REPORT_ERROR_BIT_EXT
		| VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
		//| VK_DEBUG_REPORT_DEBUG_BIT_EXT
		//| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
		, VkU::DebugReportCallback
	);
#endif
	/// Physical Device
	InitPhysicalDevices(&VkU::preferedDepthFormats);

	/// Window / Surface
	InitWindow("Title", "Name", nullptr, 800, 600);

	/// Pick Physical Device
	VkPhysicalDeviceFeatures features = {};
	features.samplerAnisotropy = true;
	features.logicOp = true;
	features.fillModeNonSolid = true;
	PickPhysicalDeviceAndQueue({ VkU::Queue::GetQueue(VK_TRUE, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 1.0f, 1) }, features);

	/// Device / Queue / Surface Properties
	InitDevice(
	{
#if _DEBUG
		"VK_LAYER_LUNARG_standard_validation",
#endif
	},
	{ "VK_KHR_swapchain" }
	, features, &VkU::preferedCompositeAlphas, &VkU::preferedPresentModes);

	/// Command Buffers / Fences / Semaphores
	InitCommandPool();
	InitCommandBuffers();
	InitFences();
	InitSemaphores();

	/// RenderPass
	InitRenderPass();

	/// Swapchain
	InitSwapchain(3);

	/// Buffers & models
	InitUniformBuffers(_maxModelMatrixCount);
	LoadModels(_modelNames);

	/// Images & samplers
	LoadImages(_imageNamesTGA);
	InitSamplers();

	/// Descriptor
	InitDescriptorPool((uint32_t)textures.size(), 2, 0);
	InitDescriptorSetLayouts();
	InitDescriptorSets();
	UpdateDescriptorSets();

	/// Pipeline
	InitPipelineLayout();
	LoadShaderModules({
			{ "Shaders/vert.spv",	VK_SHADER_STAGE_VERTEX_BIT,		"main" },
			{ "Shaders/frag.spv",	VK_SHADER_STAGE_FRAGMENT_BIT,	"main" },
		 });
	InitPipelineData();
	InitPipeline();

}
void Renderer::Draw(Scene* _scene)
{
	static float x = 9.5f;
	static float y = -0.0f;
	static float z = 60.0f;
	if (GetAsyncKeyState(Engine::IC_ARROW_LEFT))
		x += (float)Engine::deltaTime * 2;
	if (GetAsyncKeyState(Engine::IC_ARROW_RIGHT))
		x -= (float)Engine::deltaTime * 2;

	if (GetAsyncKeyState(Engine::IC_ARROW_UP))
		z -= (float)Engine::deltaTime * 20;
	if (GetAsyncKeyState(Engine::IC_ARROW_DOWN))
		z += (float)Engine::deltaTime * 20;
	//y += 0.01f * (float)deltaTime;
	viewProjection[0] = glm::lookAt(glm::vec3(sin(x/3)*z, y, cos(x/3)*z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	// Get swapchain image to be used
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
	VK_CHECK_RESULT(vkAcquireNextImageKHR(device.handle, swapchain.handle, -1, swapchain.semaphoreImageAvailable, setupFence, &swapchain.imageIndex), swapchain.imageIndex, "vkAcquireNextImageKHR");

	// Begin recording for that image
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &swapchain.fences[swapchain.imageIndex], VK_TRUE, -1), swapchain.fences[swapchain.imageIndex], "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &swapchain.fences[swapchain.imageIndex]), swapchain.fences[swapchain.imageIndex], "vkResetFences");
	VkU::BeginCommandBuffer(swapchain.commandBuffers[swapchain.imageIndex], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	// Color to clear attachments with
	VkClearValue clearColor[2];
	clearColor[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearColor[1].depthStencil = { 1.0f, 0 };

	// Begin renderPass structure
	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = swapchain.framebuffers[swapchain.imageIndex];
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = { swapchain.extent.width, swapchain.extent.height };
	renderPassBeginInfo.clearValueCount = sizeof(clearColor) / sizeof(VkClearValue);
	renderPassBeginInfo.pClearValues = clearColor;
	vkCmdBeginRenderPass(swapchain.commandBuffers[swapchain.imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkDeviceSize offset = 0;

	// Structure representing vertex shader's push constant
	struct VertexShaderPushConstantData
	{
		uint32_t modelIndex;
		float red;
		float green;
		float blue;

	} vertexShaderPushConstantData = { 0, 0.0f, 0.0f, 0.0f };

	uint32_t lastRenderMode = -1;
	uint32_t lastTextureID = -1;
	uint32_t lastMeshID = -1;

	// binds MVP uniforms
	vkCmdBindDescriptorSets(swapchain.commandBuffers[swapchain.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetMVP, 0, nullptr);

	for (size_t i = 0; i != _scene->renderCallCount; ++i)
	{
		// Checks if pipeline, texture, or mesh needs to be updated
		uint32_t newRenderMode = (_scene->renderCalls)[i].renderMode;
		uint32_t newTextureID = (_scene->renderCalls)[i].textureID;
		uint32_t newMeshID = (_scene->renderCalls)[i].meshID;

		if (lastRenderMode != newRenderMode)
		{
			// Binds pipeline
			vkCmdBindPipeline(swapchain.commandBuffers[swapchain.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelines[newRenderMode]);
			lastRenderMode = newRenderMode;
		}

		if (lastTextureID != newTextureID)
		{
			// Binds texture
			vkCmdBindDescriptorSets(swapchain.commandBuffers[swapchain.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSetsTexture[newTextureID], 0, nullptr);
			lastTextureID = newRenderMode;
		}

		// Updates the model matrix
		modelMatrices[i] = _scene->renderCalls[i].transform;
		// Gives the index to the model matrix
		vertexShaderPushConstantData.modelIndex = (uint32_t)i;
		vertexShaderPushConstantData.red = ((float*)_scene->renderCalls[i].vertexPushConstantData)[1];
		vertexShaderPushConstantData.green = ((float*)_scene->renderCalls[i].vertexPushConstantData)[2];
		vertexShaderPushConstantData.blue = ((float*)_scene->renderCalls[i].vertexPushConstantData)[3];

		vkCmdPushConstants(swapchain.commandBuffers[swapchain.imageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertexShaderPushConstantData), &vertexShaderPushConstantData);

		if (lastMeshID != newMeshID)
		{
			// Binds mesh
			vkCmdBindVertexBuffers(swapchain.commandBuffers[swapchain.imageIndex], 0, 1, &vertexBuffer.handle, &meshOffsets[newMeshID].vertexOffset);
			vkCmdBindIndexBuffer(swapchain.commandBuffers[swapchain.imageIndex], indexBuffer.handle, meshOffsets[newMeshID].indexOffset, VK_INDEX_TYPE_UINT32);
			lastMeshID = newMeshID;
		}

		// Draw call
		vkCmdDrawIndexed(swapchain.commandBuffers[swapchain.imageIndex], meshOffsets[newMeshID].indexCount, 1, 0, 0, 0);
	}

	// Ends render pass
	vkCmdEndRenderPass(swapchain.commandBuffers[swapchain.imageIndex]);

	// Ends recording
	VK_CHECK_RESULT(vkEndCommandBuffer(swapchain.commandBuffers[swapchain.imageIndex]), swapchain.commandBuffers[swapchain.imageIndex], "vkEndCommandBuffer");
}
void Renderer::Render()
{
	// Update view projection buffer
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), setupFence, "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), setupFence, "vkResetFences");
	VkU::BeginCommandBuffer(setupCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	// Fill staging buffer
	viewProjectionStaging.FillStagingBuffer(sizeof(viewProjection), viewProjection, device.handle);
	viewProjectionBuffer.UpdateBuffer(sizeof(viewProjection), setupCommandBuffer, viewProjectionStaging);

	// Update model matrices
	modelMatricesStaging.FillStagingBuffer(sizeof(glm::mat4) * modelMatrices.size(), modelMatrices.data(), device.handle);
	modelMatricesBuffer.UpdateBuffer(sizeof(glm::mat4) * modelMatrices.size(), setupCommandBuffer, modelMatricesStaging);

	VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), setupCommandBuffer, "vkEndCommandBuffer");

	// Update
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, setupFence), GRAPHICS_QUEUE, "vkQueueSubmit");

	// Submit draw calls
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &swapchain.semaphoreImageAvailable;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchain.commandBuffers[swapchain.imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphoreRenderDone;
	VK_CHECK_RESULT(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submitInfo, swapchain.fences[swapchain.imageIndex]), GRAPHICS_QUEUE, "vkQueueSubmit");

	// Present
	VkPresentInfoKHR presentInfoKHR;
	presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfoKHR.pNext = nullptr;
	presentInfoKHR.waitSemaphoreCount = 1;
	presentInfoKHR.pWaitSemaphores = &semaphoreRenderDone;
	presentInfoKHR.swapchainCount = 1;
	presentInfoKHR.pSwapchains = &swapchain.handle;
	presentInfoKHR.pImageIndices = &swapchain.imageIndex;
	presentInfoKHR.pResults = nullptr;
	VK_CHECK_RESULT(vkQueuePresentKHR(GRAPHICS_QUEUE, &presentInfoKHR), GRAPHICS_QUEUE, "vkQueuePresentKHR");

	// Handle window
	static uint64_t currentSecond = 0;
	static uint32_t frameCount = 0;
	static uint32_t fps = 0;

	// Update window text
	if ((uint64_t)Engine::globalTimer.GetTime() != currentSecond)
	{
		currentSecond = (uint64_t)Engine::globalTimer.GetTime();

		fps = frameCount;
		frameCount = 0;
	}
	else
		++frameCount;

	std::string windowText = "Time: ";
	windowText += std::to_string(Engine::lastTime);
	windowText += " FPS: ";
	windowText += std::to_string(fps);

	SetWindowText(window.hWnd, windowText.c_str());

	// Handle window messages
	currentRenderer = this;
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
void Renderer::ShutDown()
{
	// Wait for fence and device to avoid trying to destroy handles in use
	VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), 0, "vkWaitForFences");
	VK_CHECK_RESULT(vkQueueWaitIdle(GRAPHICS_QUEUE), GRAPHICS_QUEUE, "vkQueueWaitIdle");

	/// Sampler
	VK_CHECK_CLEANUP(vkDestroySampler(device.handle, sampler, nullptr), sampler, "vkDestroySampler");

	/// Textures
	for (size_t i = 0; i != textures.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, textures[i].view, nullptr), textures[i].view, "vkDestroyImageView");
		VK_CHECK_CLEANUP(vkDestroyImage(device.handle, textures[i].handle, nullptr), textures[i].handle, "vkDestroyImage");
		VK_CHECK_CLEANUP(vkFreeMemory(device.handle, textures[i].memory, nullptr), textures[i].memory, "vkFreeMemory");
	}

	/// Buffers
	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, indexBuffer.handle, nullptr), indexBuffer.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, indexBuffer.memory, nullptr), indexBuffer.memory, "vkFreeMemory");

	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, vertexBuffer.handle, nullptr), vertexBuffer.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, vertexBuffer.memory, nullptr), vertexBuffer.memory, "vkFreeMemory");

	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, modelMatricesStaging.handle, nullptr), modelMatricesStaging.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, modelMatricesStaging.memory, nullptr), modelMatricesStaging.memory, "vkFreeMemory");
	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, modelMatricesBuffer.handle, nullptr), modelMatricesBuffer.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, modelMatricesBuffer.memory, nullptr), modelMatricesBuffer.memory, "vkFreeMemory");

	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, viewProjectionStaging.handle, nullptr), viewProjectionStaging.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, viewProjectionStaging.memory, nullptr), viewProjectionStaging.memory, "vkFreeMemory");
	VK_CHECK_CLEANUP(vkDestroyBuffer(device.handle, viewProjectionBuffer.handle, nullptr), viewProjectionBuffer.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, viewProjectionBuffer.memory, nullptr), viewProjectionBuffer.memory, "vkFreeMemory");

	/// Pipelines
	for (size_t i = 0; i != computePipelines.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyPipeline(device.handle, computePipelines[i], nullptr), computePipelines[i], "vkDestroyPipeline");
	}
	computePipelines.clear();

	for (size_t i = 0; i != graphicsPipelines.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyPipeline(device.handle, graphicsPipelines[i], nullptr), graphicsPipelines[i], "vkDestroyPipeline");
	}
	graphicsPipelines.clear();

	/// Shader Modules
	for (size_t i = 0; i != shaderModules.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyShaderModule(device.handle, shaderModules[i].handle, nullptr), shaderModules[i].handle, "vkDestroyShaderModule");
	}
	shaderModules.clear();

	/// Pipeline Layout
	VK_CHECK_CLEANUP(vkDestroyPipelineLayout(device.handle, pipelineLayout, nullptr), pipelineLayout, "vkDestroyPipelineLayout");

	/// RenderPass
	VK_CHECK_CLEANUP(vkDestroyRenderPass(device.handle, renderPass, nullptr), renderPass, "vkDestroyRenderPass");

	/// Descriptor
	VK_CHECK_CLEANUP(vkDestroyDescriptorSetLayout(device.handle, descriptorSetLayoutTexture, nullptr), descriptorSetLayoutTexture, "vkDestroyDescriptorSetLayout");
	VK_CHECK_CLEANUP(vkDestroyDescriptorSetLayout(device.handle, descriptorSetLayoutMVP, nullptr), descriptorSetLayoutMVP, "vkDestroyDescriptorSetLayout");
	VK_CHECK_CLEANUP(vkDestroyDescriptorPool(device.handle, descriptorPool, nullptr), descriptorPool, "vkDestroyDescriptorSetLayout");

	/// Command Buffers / Fences / Semaphores
	VK_CHECK_CLEANUP(vkDestroySemaphore(device.handle, semaphoreRenderDone, nullptr), semaphoreRenderDone, "vkDestroySemaphore");

	VK_CHECK_CLEANUP(vkDestroyFence(device.handle, setupFence, nullptr), setupFence, "vkDestroyFence");

	VK_CHECK_CLEANUP(vkDestroyCommandPool(device.handle, commandPool, nullptr), commandPool, "vkDestroyCommandPool");

	/// Swapchain
	VK_CHECK_CLEANUP(vkDestroySemaphore(device.handle, swapchain.semaphoreImageAvailable, nullptr), swapchain.semaphoreImageAvailable, "vkDestroySemaphore");
	for (size_t i = 0; i != swapchain.fences.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFence(device.handle, swapchain.fences[i], nullptr), swapchain.fences[i], "vkDestroyFence");
	}
	swapchain.fences.clear();
	for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFramebuffer(device.handle, swapchain.framebuffers[i], nullptr), swapchain.framebuffers[i], "vkDestroyFramebuffer");
	}
	swapchain.framebuffers.clear();
	for (size_t i = 0; i != swapchain.views.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, swapchain.views[i], nullptr), swapchain.views[i], "vkDestroyImageView");
	}
	swapchain.views.clear();
	VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, swapchain.depthImage.view, nullptr), swapchain.depthImage.view, "vkDestroyImageView");
	VK_CHECK_CLEANUP(vkDestroyImage(device.handle, swapchain.depthImage.handle, nullptr), swapchain.depthImage.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(device.handle, swapchain.depthImage.memory, nullptr), swapchain.depthImage.memory, "vkFreeMemory");
	VK_CHECK_CLEANUP(vkDestroySwapchainKHR(device.handle, swapchain.handle, nullptr), swapchain.handle, "vkDestroySwapchainKHR");

	/// Device
	VK_CHECK_CLEANUP(vkDestroyDevice(device.handle, nullptr), device.handle, "vkDestroyDevice");

	/// Window / Surface
	VK_CHECK_CLEANUP(vkDestroySurfaceKHR(instance, surface.handle, nullptr), surface.handle, "vkDestroySurfaceKHR");

	DestroyWindow(window.hWnd);
	UnregisterClass(window.name, GetModuleHandle(NULL));

	/// Debug
#if _DEBUG
	if (debugReportCallback != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT FP_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		VK_CHECK_CLEANUP(FP_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr), debugReportCallback, "FP_vkDestroyDebugReportCallbackEXT");
	}
#endif

	/// Instance
	VK_CHECK_CLEANUP(vkDestroyInstance(instance, nullptr), instance, "vkDestroyInstance");
}

LRESULT VkU::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
	case WM_CLOSE:
		Engine::done = true;
		break;
	case WM_PAINT:
		ValidateRect(_hWnd, NULL);
		break;
	case WM_SIZE:
		if (_wParam == SIZE_MINIMIZED)
			Renderer::currentRenderer->RecreateSwapchain(LOWORD(_lParam), HIWORD(_lParam));
		else if (_wParam == SIZE_MAXIMIZED)
			Renderer::currentRenderer->RecreateSwapchain(LOWORD(_lParam), HIWORD(_lParam));
		else if (_wParam == SIZE_RESTORED)
			Renderer::currentRenderer->RecreateSwapchain(0, 0);
		break;
	}

	return (DefWindowProc(_hWnd, _uMsg, _wParam, _lParam));
}

void VkU::BeginCommandBuffer(VkCommandBuffer _commandBuffer, VkCommandBufferUsageFlags _flags)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = _flags;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	VK_CHECK_RESULT(vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo), _commandBuffer, "vkBeginCommandBuffer");
}

VkSurfaceFormatKHR VkU::Surface::GetVkSurfaceFormatKHR(VkPhysicalDevice _physicalDevice, Surface _surface, std::vector<VkFormat>* _preferedColorFormats)
{
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface.handle, &propertyCount, nullptr), "????????????????", "vkGetPhysicalDeviceSurfaceFormatsKHR");
	surfaceFormats.resize(propertyCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface.handle, &propertyCount, surfaceFormats.data()), "????????????????", "vkGetPhysicalDeviceSurfaceFormatsKHR");

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		if (_preferedColorFormats != nullptr)
			return{ (*_preferedColorFormats)[0], surfaceFormats[0].colorSpace };
		else
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	}
	else if (_preferedColorFormats != nullptr)
	{
		for (uint32_t i = 0; i != (*_preferedColorFormats).size(); ++i)
		{
			for (uint32_t j = 0; j != surfaceFormats.size(); ++j)
			{
				if ((*_preferedColorFormats)[i] == surfaceFormats[j].format)
				{
					return{ surfaceFormats[i] };
					break;
				}
			}
		}
	}

	return surfaceFormats[0];
}
VkSurfaceCapabilitiesKHR VkU::Surface::GetVkSurfaceCapabilitiesKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities), "????????????????", "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	return surfaceCapabilities;
}
VkCompositeAlphaFlagBitsKHR VkU::Surface::GetVkCompositeAlphaFlagBitsKHR(VkSurfaceCapabilitiesKHR _surfaceCapabilities, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas)
{
	if (_preferedCompositeAlphas != nullptr)
	{
		for (size_t i = 0; i != _preferedCompositeAlphas->size(); ++i)
		{
			if (((*_preferedCompositeAlphas)[i] & _surfaceCapabilities.supportedCompositeAlpha) == (*_preferedCompositeAlphas)[i])
			{
				return (*_preferedCompositeAlphas)[i];
			}
		}
	}

	return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
}
VkPresentModeKHR VkU::Surface::GetVkPresentModeKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface, std::vector<VkPresentModeKHR>* _preferedPresentModes)
{
	if (_preferedPresentModes != nullptr)
	{
		std::vector<VkPresentModeKHR> presentModes;
		uint32_t propertyCount = 0;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &propertyCount, nullptr), "????????????????", "vkGetPhysicalDeviceSurfacePresentModesKHR");
		presentModes.resize(propertyCount);
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &propertyCount, presentModes.data()), "????????????????", "vkGetPhysicalDeviceSurfacePresentModesKHR");

		for (size_t i = 0; i != _preferedPresentModes->size(); ++i)
		{
			for (size_t j = 0; j != presentModes.size(); ++j)
			{
				if (presentModes[j] == (*_preferedPresentModes)[i])
				{
					return (*_preferedPresentModes)[i];
				}
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

void VkU::Buffer::Init(VkDeviceSize _size, VkBufferUsageFlags _usage, VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryProperties)
{
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = _size;
	bufferCreateInfo.usage = _usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &bufferCreateInfo, nullptr, &handle), handle, "vkCreateBuffer");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_vkDevice, handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = _physicalDevice.FindMemoryTypeIndex(memoryRequirements, _memoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &memory), memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, handle, memory, 0), "????????????????", "vkBindBufferMemory");
}
void VkU::Buffer::FillStagingBuffer(VkDeviceSize _size, void * _data, VkDevice _device)
{
	void* data;
	VK_CHECK_RESULT(vkMapMemory(_device, memory, 0, _size, 0, &data), data, "vkMapMemory");
	memcpy(data, _data, _size);
	vkUnmapMemory(_device, memory);
}

void VkU::Image::InitHandle(VkFormat _format, VkExtent3D _extent3D, VkImageUsageFlags _usage, VkDevice _device)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _format;
	imageCreateInfo.extent = _extent3D;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = _usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VK_CHECK_RESULT(vkCreateImage(_device, &imageCreateInfo, nullptr, &handle), handle, "vkCreateImage");
}
void VkU::Image::InitMemory(PhysicalDevice _physicalDevice, VkDevice _device)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = _physicalDevice.FindMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &memory), memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindImageMemory(_device, handle, memory, 0), "????????????????", "vkBindImageMemory");
}
void VkU::Image::InitView(VkFormat _format, VkImageAspectFlags _aspect, VkDevice _device)
{
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	imageViewCreateInfo.image = handle;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = _format;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = _aspect;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	VK_CHECK_RESULT(vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &view), view, "vkCreateImageView");
}

void VkU::StagingImage::InitHandle(VkFormat _format, VkExtent3D _extent3D, VkDevice _device)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _format;
	imageCreateInfo.extent = _extent3D;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VK_CHECK_RESULT(vkCreateImage(_device, &imageCreateInfo, nullptr, &handle), handle, "vkCreateImage");
}
void VkU::StagingImage::InitMemory(PhysicalDevice _physicalDevice, VkDevice _device)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = _physicalDevice.FindMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &memory), memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindImageMemory(_device, handle, memory, 0), "????????????????", "vkBindImageMemory");
}
void VkU::StagingImage::FillImageMemory(uint32_t _width, uint32_t _height, VkDeviceSize _size, void * _data, VkDevice device)
{
	VkImageSubresource imageSubresource;
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.mipLevel = 0;
	imageSubresource.arrayLayer = 0;

	VkSubresourceLayout subresourceLayout;
	vkGetImageSubresourceLayout(device, handle, &imageSubresource, &subresourceLayout);

	void* mappedData;
	VK_CHECK_RESULT(vkMapMemory(device, memory, 0, _size, 0, &mappedData), mappedData, "vkMapMemory");

	if (subresourceLayout.rowPitch == _width * 4)
		memcpy(mappedData, _data, _size);
	else
	{
		uint8_t* _data8b = (uint8_t*)_data;
		uint8_t* data8b = (uint8_t*)mappedData;

		for (uint32_t y = 0; y < _height; y++)
			memcpy(&data8b[y * subresourceLayout.rowPitch], &_data8b[y * _width * 4], _width * 4);
	}

	vkUnmapMemory(device, memory);
}

void VkU::MappedBuffer::Init(VkDeviceSize _size, VkBufferUsageFlags _usage, VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryProperties)
{
	size = _size;
	usage = _usage;

	subBufferMapHead = new MappedBuffer::Node;
	fragments = 1;
	subBufferMapTail = subBufferMapHead;

	subBufferMapHead->inUse = false;
	subBufferMapHead->offset = 0;
	subBufferMapHead->size = size;
	subBufferMapHead->prev = nullptr;
	subBufferMapHead->next = nullptr;

	// Create GPU buffer
	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &bufferCreateInfo, nullptr, &handle), handle, "vkCreateBuffer");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_vkDevice, handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = _physicalDevice.FindMemoryTypeIndex(memoryRequirements, _memoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &memory), memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, handle, memory, 0), "????????????????", "vkBindBufferMemory");
}
VkU::MappedBuffer::Node* VkU::MappedBuffer::AllocateSubBuffer(VkDeviceSize _size, void* _data, VkCommandBuffer _commandBuffer, VkFence _fence, VkDevice _vkDevice, PhysicalDevice _physicalDevice)
{
	// Find subBuffer to allocate from
	MappedBuffer::Node* currentBuffer = subBufferMapHead;
	for (;;)
	{
		if (currentBuffer == nullptr)
			return nullptr; // No space available
		if (currentBuffer->inUse == true || currentBuffer->size < _size)
		{
			currentBuffer = currentBuffer->next;
			continue; // Check if next is available
		}

		break; // Found space
	}

	// Update buffer map
	MappedBuffer::Node* newBuffer;

	// if fragment has the same size
	if (currentBuffer->size == _size)
	{
		newBuffer = currentBuffer;
		newBuffer->inUse = true;
	}
	// if fragment must be split
	else
	{
		newBuffer = new MappedBuffer::Node;
		++fragments;
		newBuffer->next = currentBuffer;
		newBuffer->prev = currentBuffer->prev;
		newBuffer->offset = currentBuffer->offset;
		newBuffer->size = _size;
		newBuffer->inUse = true;

		if (newBuffer->prev != nullptr)
			newBuffer->prev->next = newBuffer;
		else
			subBufferMapHead = newBuffer;
		if (newBuffer->next != nullptr)
			newBuffer->next->prev = newBuffer;

		currentBuffer->size -= newBuffer->size;
		currentBuffer->offset += newBuffer->size;
	}

	// Fill GPU bufer
	VkU::Buffer stagingBuffer;
	stagingBuffer.Init(newBuffer->size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, _vkDevice, _physicalDevice, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	stagingBuffer.FillStagingBuffer(newBuffer->size, _data, _vkDevice);
	
	// Write to buffer
	VK_CHECK_RESULT(vkWaitForFences(_vkDevice, 1, &_fence, VK_TRUE, -1), _fence, "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(_vkDevice, 1, &_fence), _fence, "vkResetFences");
	VkU::BeginCommandBuffer(_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = newBuffer->offset;
	copyRegion.size = newBuffer->size;

	vkCmdCopyBuffer(_commandBuffer, stagingBuffer.handle, handle, 1, &copyRegion);

	VK_CHECK_RESULT(vkEndCommandBuffer(_commandBuffer), _commandBuffer, "vkEndCommandBuffer");

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VK_CHECK_RESULT(vkQueueSubmit(P_GRAPHICS_QUEUE, 1, &submitInfo, _fence), P_GRAPHICS_QUEUE, "vkQueueSubmit");

	return newBuffer;
}
void VkU::MappedBuffer::FreeSubBuffer(Node* _subBuffer)
{
	_subBuffer->inUse = false;

	// Merge with next fragment if possible
	if (_subBuffer->next != nullptr)
	{
		if (_subBuffer->next->inUse == false)
		{// Join next
			MappedBuffer::Node* tempNode = _subBuffer->next;
			_subBuffer->size += tempNode->size;
			_subBuffer->next = tempNode->next;

			if (_subBuffer->next != nullptr)
				_subBuffer->next->prev = _subBuffer;

			delete tempNode;
			--fragments;
		}
	}

	// Merge with previous fragment if possible
	if (_subBuffer->prev != nullptr)
	{
		if (_subBuffer->prev->inUse == false)
		{// Join prev
			MappedBuffer::Node* tempNode = _subBuffer->prev;
			_subBuffer->size += tempNode->size;
			_subBuffer->offset = tempNode->offset;
			_subBuffer->prev = tempNode->prev;

			if (_subBuffer->prev != nullptr)
				_subBuffer->prev->next = _subBuffer;

			delete tempNode;
			--fragments;
		}
	}
}
void VkU::MappedBuffer::Clear(VkDevice _vkDevice)
{
	MappedBuffer::Node* currentBuffer = subBufferMapHead;

	for (;;)
	{
		if (subBufferMapHead == nullptr)
			break;

		currentBuffer = subBufferMapHead->next;

		delete subBufferMapHead;

		subBufferMapHead = currentBuffer;
	}

	subBufferMapTail = nullptr;
	fragments = 0;
	size = 0;

	// Empty GPU buffer
	VK_CHECK_CLEANUP(vkDestroyBuffer(_vkDevice, handle, nullptr), handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(_vkDevice, memory, nullptr), memory, "vkFreeMemory");
}

void VkU::MappedBuffer::Cout()
{
	system("cls");
	std::cout << "Size = " << this->size << "\nFragments = " << this->fragments << "\nHead = " << this->subBufferMapHead << "\nTail = " << this->subBufferMapTail << "\n\n";

	MappedBuffer::Node* currentBuffer = subBufferMapHead;
	uint64_t count = 0;
	uint64_t sum = 0;
	while (currentBuffer != nullptr)
	{
		sum += currentBuffer->size;

		std::cout << "	Index = " << count << "\nCurrentBuffer = " << currentBuffer << "\nInUse = ";
		if (currentBuffer->inUse == true)
			std::cout << "true";
		else
			std::cout << "false";
		std::cout << "\nOffset = " << currentBuffer->offset << "\nSize = " << currentBuffer->size << "\nPrev = " << currentBuffer->prev << "\nNext = " << currentBuffer->next << "\n\n";

		currentBuffer = currentBuffer->next;

		++count;
	}

	std::cout << "Count = " << count << "\nSum = " << sum;
}
