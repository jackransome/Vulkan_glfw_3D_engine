#include "Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define NOMINMAX
#include <windows.h>

#include <unordered_set>

	void Graphics::init() {
		initWindow();
		initVulkan();
	}

	void Graphics::initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Graphics::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Graphics*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void Graphics::initVulkan() {
		createInstance(); // create the vulkan instance (get extensions, set name and version etc)
		setupDebugCallback(); // for validation layers
		createSurface(); // get glfw surface to draw on
		pickPhysicalDevice(); // picks the gpu
		createLogicalDevice(); // creates an abstraciton of the gpu
		createSwapChain(); // creates swap chain + swap chain images
		createImageViews(); //creates image views for the swap chain images
		createRenderPass(); // creates a render pass and a sub pass
		//set up the descriptor sets
		descriptorSetObjects.emplace_back("Uniform Buffer", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1);
		descriptorSetObjects.emplace_back("Texture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Texture), 1);
		descriptorSetObjects.emplace_back("Storage Buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4) * MAX_RENDER_INSTANCES, 1);
		descriptorSetObjects.emplace_back("Light Buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(LightData) * MAX_RENDER_INSTANCES, 1);

		//fill push constant info vector
		pushConstantInfos.emplace_back(sizeof(PushConstants), VK_SHADER_STAGE_VERTEX_BIT);
		pushConstantInfos.emplace_back(sizeof(int), VK_SHADER_STAGE_FRAGMENT_BIT);
		//pushConstantInfos.emplace_back(sizeof(float), VK_SHADER_STAGE_FRAGMENT_BIT);
		pipelineBundles.push_back(createCurrentPipelineBundle(descriptorSetObjects, swapChainExtent, renderPass, swapChainImages.size(), pushConstantInfos));

		createTextureAtlasArray({});
		createCommandPool();
		createColorResources();
		createDepthResources();// important for depth testing, so shaders only overwrite things further away
		createFramebuffers();
	
		createTextureImage();
		createTextureImageView();
		readImageInfoFromFile("resources/textures/image_paths.txt");
		loadResources();
		createTextureSampler();
	
		loadObjects();
		createVertexBuffer();
		createIndexBuffer();
		
		transformBuffer = createStorageBuffer("transform buffer",sizeof(glm::mat4) * MAX_RENDER_INSTANCES);
		std::vector<glm::mat4> storageBufferData;
		for (int i = 0; i < renderInstances.size(); i++) {
			for (int j = 0; j < renderInstanceIndexes[i]; j++) {
				storageBufferData.push_back(renderInstances[i][j].transformData);
			}
		}

		updateStorageBuffer(transformBuffer, storageBufferData);

		addLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f);
		lightBuffer = createStorageBuffer("light buffer", sizeof(LightData) * MAX_RENDER_INSTANCES);
		updateStorageBuffer(lightBuffer, lights);
		
		createUniformBuffers();

		updateDescriptorResource(pipelineBundles[0], "Uniform Buffer", descriptorResource(uniformBuffers));
		updateDescriptorResource(pipelineBundles[0], "Texture", descriptorResource(textureSampler, textureImageView));
		updateDescriptorResource(pipelineBundles[0], "Storage Buffer", descriptorResource(transformBuffer.buffer));
		updateDescriptorResource(pipelineBundles[0], "Light Buffer", descriptorResource(lightBuffer.buffer));
		for (int i = 0; i < swapChainImages.size(); i++) {
			updateDescriptorSet(pipelineBundles[0], i);
		}
		
		createCommandBuffers();
		createSyncObjects();
		setUpCamera();

		printSampleCount();
	}

	void Graphics::run() {
		if (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		} else {
			shouldClose = true;
		}

		vkDeviceWaitIdle(device);
	}

	void Graphics::cleanupSwapChain() { 
		vkDestroyImageView(device, colorImageView, nullptr);
		vkDestroyImage(device, colorImage, nullptr);
		vkFreeMemory(device, colorImageMemory, nullptr);

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void Graphics::cleanup() {
		cleanupSwapChain();

		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);

		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugReportCallbackEXT(instance, callback, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void Graphics::recreateSwapChain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanupSwapChain(); //NEED TO FIX THIS

		createSwapChain();
		createImageViews();
		createRenderPass();
		pipelineBundles.clear();
		std::vector<descriptorSetObject> descriptorSetObjects;
		descriptorSetObjects.emplace_back("Uniform Buffer", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 1);
		descriptorSetObjects.emplace_back("Texture", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Texture), 1);
		descriptorSetObjects.emplace_back("Storage Buffer", VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4) * MAX_RENDER_INSTANCES, 1);
		pipelineBundles.push_back(createCurrentPipelineBundle(descriptorSetObjects, swapChainExtent, renderPass, swapChainImages.size(), pushConstantInfos));
		createColorResources();
		createDepthResources();
		createFramebuffers();

		updateDescriptorResource(pipelineBundles[0], "Uniform Buffer", descriptorResource(uniformBuffers));
		updateDescriptorResource(pipelineBundles[0], "Texture", descriptorResource(textureSampler, textureImageView));
		updateDescriptorResource(pipelineBundles[0], "Storage Buffer", descriptorResource(transformBuffer.buffer));
		for (int i = 0; i < swapChainImages.size(); i++) {
			updateDescriptorSet(pipelineBundles[0], i);
		}

		createCommandBuffers();
		
	}

	void Graphics::printSampleCount() {
		int sampleCount = 1; // Default to 1 sample if unrecognized
		switch (msaaSamples) {
		case VK_SAMPLE_COUNT_1_BIT:
			sampleCount = 1;
			break;
		case VK_SAMPLE_COUNT_2_BIT:
			sampleCount = 2;
			break;
		case VK_SAMPLE_COUNT_4_BIT:
			sampleCount = 4;
			break;
		case VK_SAMPLE_COUNT_8_BIT:
			sampleCount = 8;
			break;
		case VK_SAMPLE_COUNT_16_BIT:
			sampleCount = 16;
			break;
		case VK_SAMPLE_COUNT_32_BIT:
			sampleCount = 32;
			break;
		case VK_SAMPLE_COUNT_64_BIT:
			sampleCount = 64;
			break;
		default:
			// Handle unexpected values if necessary
			break;
		}

		std::cout << "MSAA Sample Count: " << sampleCount << std::endl;
	}

	void Graphics::createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	void Graphics::setupDebugCallback() {
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug callback!");
		}
	}

	void Graphics::createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void Graphics::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				getMaxUsableSampleCount(device);
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void Graphics::createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
	}

	void Graphics::createColorResources() {
		VkFormat colorFormat = swapChainImageFormat;

		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
	}

	void Graphics::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void Graphics::createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void Graphics::createRenderPass() {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void Graphics::createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void Graphics::createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	void Graphics::createDepthResources() {
		VkFormat depthFormat = findDepthFormat();

		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	VkFormat Graphics::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat Graphics::findDepthFormat() {
		return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool Graphics::hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void Graphics::createTextureImage() {
		int mainTextureIndex = loadTexture("resources/textures/atlas.png");

		// Set class members to maintain original functionality
		textureImage = textures[mainTextureIndex].textureImage;
		textureImageMemory = textures[mainTextureIndex].textureImageMemory;
		mipLevels = textures[mainTextureIndex].mipLevels;
		textureWidth = textures[mainTextureIndex].width;
		textureHeight = textures[mainTextureIndex].height;
	}

	uint32_t Graphics::loadTexture(const std::string& texturePath) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(pixels);

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);

		Texture newTexture = { textureImage, textureImageMemory, texturePath,  createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels), texWidth, texHeight, mipLevels };
		textures.push_back(newTexture);

		std::cout << "created texture " << texturePath << " with miplevels = " << mipLevels << std::endl;

		return static_cast<uint32_t>(textures.size() - 1);
	}

	void Graphics::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}

	void Graphics::createTextureImageView() {
		textureImageView = textures[0].textureImageView;
	}

	void Graphics::createTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = static_cast<float>(std::max_element(textures.begin(), textures.end(),
			[](const Texture& a, const Texture& b) {
				return a.mipLevels < b.mipLevels;
			})->mipLevels);
		std::cout << "mipLevels: " << mipLevels << std::endl;
		samplerInfo.mipLodBias = 0;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VkImageView Graphics::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	void Graphics::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}


	void Graphics::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format)) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}

	void Graphics::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}

	glm::vec2 Graphics::getAtlasOffset(std::string textureName) {
		for (int i = 0; i < atlasOffsets.size(); i++) {
			if (textureName == atlasOffsets[i].name) {
				return atlasOffsets[i].coordinates;
			}
		}
		std::cout << "texture " << textureName << " not found in atlas" << std::endl;
		return glm::vec2(0, 0);
	}

	glm::vec2 Graphics::getAtlasSize(std::string textureName) {
		for (int i = 0; i < atlasOffsets.size(); i++) {
			if (textureName == atlasOffsets[i].name) {
				return atlasOffsets[i].dimensions;
			}
		}
		std::cout << "texture " << textureName << " not found in atlas" << std::endl;
		return glm::vec2(0, 0);
	}

	void Graphics::loadModel(std::string path, glm::vec4 defaultColor, float scale) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err, warn;
		std::string directory = path.substr(0, path.find_last_of("/\\") + 1);

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), directory.c_str())) {
			throw std::runtime_error(warn + err);
		}

		uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());
		uint32_t indexOffset = static_cast<uint32_t>(indices.size());
		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

		VkImageView modelTextureView = VK_NULL_HANDLE;
		VkImageView normalMapView = VK_NULL_HANDLE;

		std::string textureName;
		std::string normalMapName;
		bool hasNormalMap = false;

		// Load textures if available
		if (!materials.empty()) {
			const auto& material = materials[0];
			if (!material.diffuse_texname.empty()) {
				textureName = material.diffuse_texname;
				//std::string texturePath = directory + material.diffuse_texname;
				//std::cout << "loading " << texturePath << std::endl;
				//uint32_t textureIndex = loadTexture(texturePath);
				//modelTextureView = textures[textureIndex].textureImageView;
			}
			if (!material.bump_texname.empty()) {
				normalMapName = material.bump_texname;
				hasNormalMap = true;
			}
		}


		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0] * scale,
					attrib.vertices[3 * index.vertex_index + 1] * scale,
					attrib.vertices[3 * index.vertex_index + 2] * scale
				};

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}

				// Get material for this face
				int materialId = shape.mesh.material_ids[index.vertex_index / 3];
				if (materialId >= 0 && materialId < materials.size()) {
					const auto& material = materials[materialId];
					vertex.diffuse = glm::vec3(0.5f);// glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
					vertex.specular = glm::vec3(0.1f); //glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
					vertex.ambient = glm::vec3(0.1f); //glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
					vertex.shininess = 1.0f; //material.shininess;
					vertex.opacity = 1.0f; //material.dissolve;
				}
				else {
					// Use default color if no material is specified
					vertex.diffuse = glm::vec3(defaultColor);
					vertex.specular = glm::vec3(0.5f);
					vertex.ambient = glm::vec3(0.1f);
					vertex.shininess = 32.0f;
					vertex.opacity = defaultColor.a;
				}

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}

		uint32_t vertexCount = static_cast<uint32_t>(vertices.size()) - vertexOffset;
		uint32_t indexCount = static_cast<uint32_t>(indices.size()) - indexOffset;

		Model newModel;
		newModel.offset = indexOffset;
		newModel.size = indexCount;
		newModel.textureOffset = getAtlasOffset(textureName);
		newModel.textureSize = getAtlasSize(textureName);
		if (hasNormalMap) {
			newModel.normalTextureOffset = getAtlasOffset(normalMapName);
			newModel.normalTextureSize = getAtlasSize(normalMapName);
			std::cout << "normal map is " << normalMapName << std::endl;
		}
		newModel.hasNormalMap = hasNormalMap;
		models.push_back(newModel);
	}


	void Graphics::createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void Graphics::createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	StorageBufferObject Graphics::createStorageBuffer(std::string name, VkDeviceSize size) {
		StorageBufferObject storageBuffer;
		storageBuffer.name = name;
		storageBuffer.size = size;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		VkDeviceMemory bufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, storageBuffer.buffer, bufferMemory);

		copyBuffer(stagingBuffer, storageBuffer.buffer, size);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		return storageBuffer;
	}

	template<typename T>
	void Graphics::updateStorageBuffer(StorageBufferObject storageBuffer, const std::vector<T>& data) {
		VkDeviceSize bufferSize = sizeof(T) * data.size();
		if (bufferSize > storageBuffer.size) {
			throw std::runtime_error("Data size exceeds maximum buffer size");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
					stagingBuffer, stagingBufferMemory);

		void* mappedMemory;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedMemory);
		memcpy(mappedMemory, data.data(), bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		copyBuffer(stagingBuffer, storageBuffer.buffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void Graphics::createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(swapChainImages.size());
		uniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		}
	}

	void Graphics::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer Graphics::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Graphics::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void Graphics::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	uint32_t Graphics::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void Graphics::createCommandBuffers() {
		commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f,0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBundles[0].pipeline);

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			// TODO:
			//
			// loop through each group of instances, setting a push constant for the starting index in the storage buffer, and doing an instances draw
			// also update the descriptor set with the correct texture
			// what happens if there is no texture or it's a 2D image draw:
			// lets not support no texture for now
			// we need a new pipeline for only 2D stuff.
			int startingIndex = 0;
			
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBundles[0].pipelineLayout, 0, 1, &(pipelineBundles[0].descriptorSets)[i], 0, nullptr); //HERE123

			for (int j = 0; j < renderInstances.size(); j++) {
				if (renderInstanceIndexes[j] > 0) {
					//updatePipelineBundleResources(pipelineBundles[0], uniformBuffers, storageBuffer, textureImageView, textureSampler);
					//updateDescriptorSet(pipelineBundles[0], i);
					//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineBundles[0].pipelineLayout, 0, 1, &(pipelineBundles[0].descriptorSets)[i], 0, nullptr); //HERE123
					//updatePipelineBundleResources(pipelineBundles[0], uniformBuffers, storageBuffer, renderInstances[j][0].model->textureImageView, textureSampler);
					
					PushConstants pushConstants = {
						startingIndex,
						static_cast<float>(renderInstances[j][0].model->textureOffset.x) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->textureOffset.y) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->textureSize.x) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->textureSize.y) / 4096.0f,
						renderInstances[j][0].model->hasNormalMap,
						static_cast<float>(renderInstances[j][0].model->normalTextureOffset.x) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->normalTextureOffset.y) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->normalTextureSize.x) / 4096.0f,
						static_cast<float>(renderInstances[j][0].model->normalTextureSize.y) / 4096.0f,
					};
					updatePushConstants(commandBuffers[i], pipelineBundles[0].pipelineLayout, pushConstantInfos[0], &pushConstants);
					int lightCount = lights.size();
					updatePushConstants(commandBuffers[i], pipelineBundles[0].pipelineLayout, pushConstantInfos[1], &lightCount);
					vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(models[j].size), renderInstanceIndexes[j], static_cast<uint32_t>(models[j].offset), 0, 0);
				}
				startingIndex += renderInstanceIndexes[j];
			}
			
			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void Graphics::createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void Graphics::updateUniformBuffer(uint32_t currentImage) {

		direction = glm::vec3(
			cos(cameraAngle.y) * sin(cameraAngle.x),
			sin(cameraAngle.y),
			cos(cameraAngle.y) * cos(cameraAngle.x)
		);

		right = glm::vec3(
			sin(cameraAngle.x - 3.14f / 2.0f),
			0,
			cos(cameraAngle.x - 3.14f / 2.0f)
		);

		up = glm::cross(right, direction);

		UniformBufferObject ubo = {};
		//ubo.model = glm::rotate(glm::mat4(), 0.0f, glm::vec3(0, 0, 1));
		ubo.view = glm::lookAt(cameraPosition, cameraPosition + direction, up);
		ubo.proj = glm::perspective(glm::radians(FOV), swapChainExtent.width / (float)swapChainExtent.height, 0.001f, 1000.0f);
		ubo.proj[1][1] *= -1;
		ubo.cameraPos = cameraPosition;
		//static auto startTime = std::chrono::high_resolution_clock::now();

		//auto currentTime = std::chrono::high_resolution_clock::now();
		//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		//UniformBufferObject ubo = {};
		//ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
		//ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
	}

	void Graphics::drawFrame() {
		if (totalRenderInstances < 100) {
			//addRenderInstance((rand() % 30), (rand() % 30), (rand() % 30), 3);
		}
		

		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		//renderInstances[0][0].transformData = glm::translate(glm::mat4(1.0f), cameraPosition);

		updateUniformBuffer(imageIndex);
		std::vector<glm::mat4> storageBufferData;
		for (int i = 0; i < renderInstances.size(); i++) {
			for (int j = 0; j < renderInstanceIndexes[i]; j++) {
				storageBufferData.push_back(renderInstances[i][j].transformData);
			}
		}

		VkDeviceSize maxBufferSize = sizeof(storageBufferData[0]) * MAX_RENDER_INSTANCES;
		updateStorageBuffer(transformBuffer, storageBufferData);
		updateStorageBuffer(lightBuffer, lights);
		createCommandBuffers();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VkShaderModule Graphics::createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	VkSurfaceFormatKHR Graphics::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Graphics::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D Graphics::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	SwapChainSupportDetails Graphics::querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool Graphics::isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);
		
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate  && supportedFeatures.samplerAnisotropy;
	}

	bool Graphics::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	std::vector<const char*> Graphics::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	bool Graphics::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<char> Graphics::readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Graphics::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

	void Graphics::loadResources()
	{
		loadModel("resources/models/texcube.obj", glm::vec4(0.9, 0.1, 0.1, 1), 1);
		loadModel("resources/models/test3.obj", glm::vec4(0.2, 0.4, 0.9, 1), 1);
		loadModel("resources/models/xyzOrigin.obj", glm::vec4(0.1, 0.9, 0.1, 1), 1);
		loadModel("resources/models/bep.obj", glm::vec4(0.7, 0.9, 0.1, 1), 1);
		renderInstances.resize(models.size());
		renderInstanceIndexes.resize(models.size());
		std::fill(renderInstanceIndexes.begin(), renderInstanceIndexes.end(), 0);
	}


	void Graphics::loadObjects() {
		addRenderInstance(0, 0, 0, 3);
		addRenderInstance(0, 0, 0, 1);
		addRenderInstance(-1, -1, -1, 1);
		addRenderInstance(3, 0.1, 1, 2);
		addRenderInstance(5, 0.1, 0, 1);
		addRenderInstance(7, 0.1, 0, 0);
		addRenderInstance(9, 0.1, 2, 1);
	}

	void Graphics::setUpCamera() {
		cameraAngle = glm::vec3(1, 1, 1);
		cameraPosition = glm::vec3(0, 0, 0);
	}

	void Graphics::changeCameraPos(float x, float y, float z) {
		glm::vec3 forward = glm::vec3(sin(cameraAngle.x), 0, cos(cameraAngle.x));
		cameraPosition += x * right * cameraVelocity;
		cameraPosition.y += y * cameraVelocity;
		cameraPosition += z * forward * cameraVelocity;
	}

	glm::vec3 Graphics::getProperCameraVelocity(glm::vec3 cameraVel) {
		glm::vec3 forward = glm::vec3(sin(cameraAngle.x), 0, cos(cameraAngle.x));
		glm::vec3 vel = cameraVel.x * right * cameraVelocity + glm::vec3(0,cameraVel.y * cameraVelocity,0) + cameraVel.z * forward * cameraVelocity;
		vel.y = cameraVel.y * cameraVelocity;
		return vel;
	}

	void Graphics::setCameraAngle(glm::vec3 _cameraAngle) {
		cameraAngle = _cameraAngle;
	}

	void Graphics::setCameraPos(glm::vec3 cameraPos)
	{
		cameraPosition = cameraPos;
	}

	GLFWwindow* Graphics::getWindowPointer() {
		return window;
	}

	void Graphics::addRenderInstance(float x, float y, float z, int modelIndex) {
		
		if (modelIndex >= models.size()) {
			std::cout << "Model index " << modelIndex << "out of range" << std::endl;
			return;
		}
		if (totalRenderInstances == MAX_RENDER_INSTANCES) {
			//std::cout << "Max render instances reached" << std::endl;
			return;
		}
		std::vector<RenderInstance>& modelRenderInstances = renderInstances[modelIndex];

		modelRenderInstances.emplace_back();

		RenderInstance& instance = modelRenderInstances[renderInstanceIndexes[modelIndex]];
		renderInstanceIndexes[modelIndex]++;
		instance.model = &models[modelIndex];
		instance.transformData = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
		totalRenderInstances++;
		if (modelRenderInstances.size() >= modelRenderInstances.capacity()) {
			modelRenderInstances.reserve(modelRenderInstances.size() + 5);  // Add margin of 5
		}
	}

	void Graphics::resetRenderInstances() {
		std::fill(renderInstanceIndexes.begin(), renderInstanceIndexes.end(), 0);
	}

	glm::vec3 Graphics::getCameraPos() {
		return cameraPosition;
	}

	void Graphics::clearStorageBuffer(StorageBufferObject& storageBuffer) {
		vkDestroyBuffer(device, storageBuffer.buffer, nullptr);
		vkFreeMemory(device, storageBuffer.memory, nullptr);
		storageBuffer.memory = VK_NULL_HANDLE;
		storageBuffer.buffer = VK_NULL_HANDLE;
	}

	VkPipeline Graphics::createGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath,
		VkDescriptorSetLayout descriptorSetLayout, VkExtent2D swapChainExtent,
		VkRenderPass renderPass, VkPipelineLayout& pipelineLayout, std::vector<PushConstantInfo> &pushConstantInfos) {
		auto vertShaderCode = readFile(vertShaderPath);
		auto fragShaderCode = readFile(fragShaderPath);

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = msaaSamples;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		std::vector<VkPushConstantRange> pushConstantRanges;
		uint32_t offset = 0;

		for (int i = 0; i < pushConstantInfos.size(); i++) {
			VkPushConstantRange range{};
			range.offset = offset;
			range.size = pushConstantInfos[i].size;
			range.stageFlags = pushConstantInfos[i].stageFlags;
			pushConstantRanges.push_back(range);
			pushConstantInfos[i].offset = offset;
			offset += pushConstantInfos[i].size;
		}

		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipeline graphicsPipeline;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
						
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);

		return graphicsPipeline;
	}

	void Graphics::updatePushConstants(VkCommandBuffer commandBuffer,
							VkPipelineLayout pipelineLayout,
							const PushConstantInfo& pcInfo,
							const void* data) {
		vkCmdPushConstants(commandBuffer, pipelineLayout, pcInfo.stageFlags, pcInfo.offset, pcInfo.size, data);
	}

	VkDescriptorSetLayout Graphics::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		VkDescriptorSetLayout descriptorSetLayout;
		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		return descriptorSetLayout;
	}

	std::vector<VkDescriptorSet> Graphics::createDescriptorSets(VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout, uint32_t count) {
		std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = count;
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptorSets(count);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		return descriptorSets;
	}


void Graphics::addLight(glm::vec3 position, glm::vec3 color, float intensity) {
	LightData light;
	light.position = position;
	light.color = color;
	light.intensity = intensity;
	lights.push_back(light);
}

void Graphics::updateDescriptorSet(const PipelineBundle& bundle, int index) {

		std::vector<VkWriteDescriptorSet> descriptorWrites;
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkDescriptorImageInfo> imageInfos;

		descriptorWrites.reserve(bundle.descriptorInfos.size());
		bufferInfos.reserve(bundle.descriptorInfos.size());
		imageInfos.reserve(bundle.descriptorInfos.size());

		for (size_t j = 0; j < bundle.descriptorInfos.size(); ++j) {
			const auto& info = bundle.descriptorInfos[j];

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = bundle.descriptorSets[index];
			descriptorWrite.dstBinding = static_cast<uint32_t>(j);
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = info.type;
			descriptorWrite.descriptorCount = 1;

			switch (info.type) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				if (info.buffers.empty() || j >= info.buffers.size()) {
					throw std::runtime_error("Invalid uniform buffer for descriptor " + std::to_string(j));
				}
				bufferInfos.push_back({
					info.buffers[index],
					info.bufferOffset,
					info.bufferRange
					});
				descriptorWrite.pBufferInfo = &bufferInfos.back();
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				if (info.buffer == nullptr) {
					throw std::runtime_error("Storage buffer is null for descriptor " + std::to_string(j));
				}
				bufferInfos.push_back({
					(info.buffer),
					info.bufferOffset,
					info.bufferRange
					});
				descriptorWrite.pBufferInfo = &bufferInfos.back();
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				if (info.imageView == nullptr || info.sampler == nullptr) {
					throw std::runtime_error("Image view or sampler is null for descriptor " + std::to_string(j));
				}
				imageInfos.push_back({
					(info.sampler),
					(info.imageView),
					info.imageLayout
					});
				descriptorWrite.pImageInfo = &imageInfos.back();
				break;
			default:
				throw std::runtime_error("Unsupported descriptor type");
			}

			descriptorWrites.push_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorPool Graphics::createDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets) {
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return descriptorPool;
}

PipelineBundle Graphics::createCurrentPipelineBundle(std::vector<descriptorSetObject> descriptorSetObjects, VkExtent2D swapChainExtent, VkRenderPass renderPass,
	uint32_t swapChainImageCount, std::vector<PushConstantInfo> &pushConstantInfos) {

	//to make a descriptor available in multiple stages, do something like this: VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<DescriptorInfo> descriptorInfos;

	for (int i = 0; i < descriptorSetObjects.size(); i++) {
		descriptorSetObjects[i].binding = i;
		VkDescriptorSetLayoutBinding binding = {descriptorSetObjects[i].binding, descriptorSetObjects[i].type, descriptorSetObjects[i].count, descriptorSetObjects[i].stageFlags, nullptr};
		bindings.push_back(binding);

		DescriptorInfo info(binding.descriptorType);
		switch (binding.descriptorType) {
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			info.bufferRange = descriptorSetObjects[i].size;
			break;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			info.bufferRange = descriptorSetObjects[i].size;
			break;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		}
		descriptorInfos.push_back(info);
		descriptorSetObjects[i].info = info;
	}

	VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(bindings);

	// Create descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& binding : bindings) {
		poolSizes.push_back({ binding.descriptorType, swapChainImageCount });
	}
	VkDescriptorPool descriptorPool = createDescriptorPool(poolSizes, swapChainImageCount);

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline = createGraphicsPipeline("resources/shaders/vert.spv", "resources/shaders/frag.spv",
		descriptorSetLayout, swapChainExtent, renderPass, pipelineLayout, pushConstantInfos);

	std::vector<VkDescriptorSet> descriptorSets = createDescriptorSets(descriptorPool, descriptorSetLayout, swapChainImageCount);

	return PipelineBundle(pipeline, pipelineLayout, std::move(descriptorSets), std::move(descriptorInfos), std::move(descriptorSetObjects));
}

void Graphics::updateDescriptorResource(PipelineBundle& bundle, std::string name, descriptorResource& resource) {
	for (int i = 0; i < bundle.descriptorSetObjects.size(); i++) {
		if (bundle.descriptorSetObjects[i].name == name) {
			switch (bundle.descriptorSetObjects[i].type) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				bundle.descriptorInfos[i].buffers = resource.uniformBuffers;
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				bundle.descriptorInfos[i].buffer = resource.storageBuffer;
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				bundle.descriptorInfos[i].imageView = resource.textureImageView;
				bundle.descriptorInfos[i].sampler = resource.textureSampler;
				break;
			}
		}
	}
}

void copyImageSection(unsigned char* src, int srcWidth, int srcHeight, int srcChannels,
	unsigned char* dest, int destWidth, int destHeight, int destChannels,
	int srcX, int srcY, int destX, int destY, int copyWidth, int copyHeight) {
	copyWidth = std::min(copyWidth, srcWidth - srcX);
	copyWidth = std::min(copyWidth, destWidth - destX);
	copyHeight = std::min(copyHeight, srcHeight - srcY);
	copyHeight = std::min(copyHeight, destHeight - destY);

	int minChannels = std::min(srcChannels, destChannels);

	for (int y = 0; y < copyHeight; ++y) {
		for (int x = 0; x < copyWidth; ++x) {
			for (int c = 0; c < minChannels; ++c) {
				int srcIndex = ((srcY + y) * srcWidth + (srcX + x)) * srcChannels + c;
				int destIndex = ((destY + y) * destWidth + (destX + x)) * destChannels + c;
				dest[destIndex] = src[srcIndex];
			}
		}
	}
}

std::string getFilenameFromPath(const std::string& path) {
	size_t lastSlash = path.find_last_of("\\/");
	if (lastSlash != std::string::npos) {
		return path.substr(lastSlash + 1);
	}
	return path;
}

void Graphics::readImageInfoFromFile(const std::string& filePath) {
	std::ifstream file(filePath);

	if (!file.is_open()) {
		std::cerr << "Unable to open file: " << filePath << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string fullPath, coordinatesStr;

		if (std::getline(iss, fullPath, '|') && std::getline(iss, coordinatesStr)) {
			// Trim whitespace from fullPath and coordinatesStr
			fullPath.erase(0, fullPath.find_first_not_of(" \t"));
			fullPath.erase(fullPath.find_last_not_of(" \t") + 1);
			coordinatesStr.erase(0, coordinatesStr.find_first_not_of(" \t"));
			coordinatesStr.erase(coordinatesStr.find_last_not_of(" \t") + 1);

			// Extract filename from path
			std::string filename = getFilenameFromPath(fullPath);

			// Parse coordinates
			std::istringstream coordStream(coordinatesStr);
			std::string xStr, yStr, wStr, hStr;
			int x = 0, y = 0, w = 0, h = 0;

			if (std::getline(coordStream, xStr, ',') && std::getline(coordStream, yStr, ',') &&
				std::getline(coordStream, wStr, ',') && std::getline(coordStream, hStr)) {
				// Convert the strings to integers
				x = std::stoi(xStr);
				y = std::stoi(yStr);
				w = std::stoi(wStr);
				h = std::stoi(hStr);
			}

			// Store the parsed data in atlasOffsets (assuming atlasOffsets stores name, coordinates, and size)
			atlasOffsets.push_back({ filename, glm::vec2(x, y), glm::vec2(w, h) });
		}
	}
	file.close();

	// Optional debug printing
	return;
	for (const auto& info : atlasOffsets) {
		std::cout << "Image: " << info.name
			<< ", Coordinates: (" << info.coordinates.x
			<< ", " << info.coordinates.y << ")"
			<< ", Size: (" << info.dimensions.x
			<< ", " << info.dimensions.y << ")" << std::endl;
	}
}


bool comparePaths(const std::vector<std::string>& paths, const std::string& filePath) {
	// Read file contents into a set
	std::unordered_set<std::string> fileContents;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Unable to open file: " << filePath << std::endl;
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		// Extract the path part before the '|' character
		std::istringstream iss(line);
		std::string path;
		if (std::getline(iss, path, '|')) {
			// Trim leading and trailing whitespace
			path.erase(0, path.find_first_not_of(" \t"));
			path.erase(path.find_last_not_of(" \t") + 1);

			if (!path.empty()) {
				fileContents.insert(path);
			}
		}
	}
	file.close();

	// Check if sizes match
	if (paths.size() != fileContents.size()) {
		return false;
	}

	// Check if all paths in the vector are in the file
	for (const auto& path : paths) {
		if (fileContents.find(path) == fileContents.end()) {
			return false;
		}
	}

	return true;
}

std::vector<std::string> getFileNamesInDirectory(const std::string& directory) {
	std::vector<std::string> fileNames;
	std::string search_path = directory + "\\*.png";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				std::string fileName = fd.cFileName;
				if (fileName != "atlas.png") {
					fileNames.push_back(directory + "\\" + fd.cFileName);
				}
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	return fileNames;
}

struct Image {
	unsigned char* data;
	int width;
	int height;
	int channels;
	std::string path;
	int x;
	int y;
};

struct Skyline {
	int x, y, width;
};

void Graphics::createTextureAtlasArray(std::vector<std::string> texturePaths) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	uint32_t maxTextureSize = properties.limits.maxImageDimension2D;
	uint32_t maxTextureArrayLayers = properties.limits.maxImageArrayLayers;

	VkPhysicalDeviceMaintenance3Properties maintenance3Props{};
	maintenance3Props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;

	VkPhysicalDeviceProperties2 deviceProps2{};
	deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProps2.pNext = &maintenance3Props;

	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

	VkDeviceSize maxResourceSize = maintenance3Props.maxMemoryAllocationSize;

	uint32_t maxDimensionMemConstraint = static_cast<uint32_t>(std::sqrt(maxResourceSize / 4));

	uint32_t maxDim = std::min(maxDimensionMemConstraint, maxTextureSize);

	std::cout << "maxTextureSize = " << maxTextureSize << std::endl;
	std::cout << "maxTextureArrayLayers = " << maxTextureArrayLayers << std::endl;
	std::cout << "maxResourceSize = " << maxResourceSize << std::endl;
	std::cout << "maxDimension = " << maxDimensionMemConstraint << std::endl;
	std::cout << "maxDim = " << maxDim << std::endl;

	const std::string textures_path = "resources/textures";
	const char* output_path = "resources/textures/atlas.png";
	std::vector<Image> images;

	std::vector<std::string> filePaths = getFileNamesInDirectory(textures_path);

	if (comparePaths(filePaths, "resources/textures/image_paths.txt")) {
		return;
	}

	for (const auto& path : filePaths) {
		Image img;
		img.path = path;
		img.data = stbi_load(img.path.c_str(), &img.width, &img.height, &img.channels, 0);
		if (img.data == nullptr) {
			std::cerr << "Error loading image: " << img.path << ": " << stbi_failure_reason() << std::endl;
			continue;
		}
		std::cout << "Loaded image: " << img.path << " " << img.width << "x" << img.height << " with " << img.channels << " channels" << std::endl;
		images.push_back(img);
	}

	if (images.empty()) {
		std::cerr << "No images loaded successfully" << std::endl;
	}

	std::cout << "Total images loaded: " << images.size() << std::endl;

	if (images.empty()) {
		std::cerr << "No images loaded successfully" << std::endl;
	}

	// Sort images by height (descending)
	std::sort(images.begin(), images.end(), [](const Image& a, const Image& b) {
		return a.height > b.height;
		});

	int atlasWidth = 4096;
	int atlasHeight = 0;
	std::vector<Skyline> skyline = { {0, 0, atlasWidth} };
	std::vector<std::pair<int, int>> positions(images.size());

	for (size_t i = 0; i < images.size(); ++i) {
		const Image& img = images[i];
		int bestY = INT_MAX;
		int bestIndex = -1;

		// Find best position
		for (size_t j = 0; j < skyline.size(); ++j) {
			if (skyline[j].width >= img.width) {
				int y = skyline[j].y;
				if (y + img.height < bestY) {
					bestY = y + img.height;
					bestIndex = j;
				}
			}
		}

		if (bestIndex == -1) {
			// If no suitable position found, add to the top
			bestIndex = skyline.size();
			skyline.push_back({ 0, atlasHeight, atlasWidth });
			bestY = atlasHeight + img.height;
		}

		// Place image
		positions[i] = { skyline[bestIndex].x, skyline[bestIndex].y };
		atlasHeight = std::max(atlasHeight, bestY);

		// Update skyline
		Skyline newSegment = { skyline[bestIndex].x, bestY, img.width };
		int rightRemainder = skyline[bestIndex].width - img.width;

		if (rightRemainder > 0) {
			skyline.insert(skyline.begin() + bestIndex + 1, { skyline[bestIndex].x + img.width, skyline[bestIndex].y, rightRemainder });
		}

		skyline[bestIndex] = newSegment;

		// Merge adjacent segments with the same height
		for (size_t j = 0; j < skyline.size() - 1; ++j) {
			if (skyline[j].y == skyline[j + 1].y) {
				skyline[j].width += skyline[j + 1].width;
				skyline.erase(skyline.begin() + j + 1);
				--j;
			}
		}
	}

	// Create atlas
	int atlasChannels = 4;  // RGBA CHANGE TO 3 FOR NORMAL MAPS
	std::vector<unsigned char> atlas(4096 * 4096 * atlasChannels, 0);  // Initialize to transparent

	// Copy images to atlas
	for (size_t i = 0; i < images.size(); ++i) {
		Image& img = images[i];
		std::cout << "placing " << images[i].path << " which has " << img.channels << " channels\n";
		copyImageSection(img.data, img.width, img.height, img.channels,
			atlas.data(), atlasWidth, atlasHeight, atlasChannels,
			0, 0, positions[i].first, positions[i].second, img.width, img.height);
		std::cout << "Placed image " << i << " at (" << positions[i].first << ", " << positions[i].second << ")" << std::endl;
		img.x = positions[i].first;
		img.y = positions[i].second;
	}

	// Save atlas
	// TODO: MAKE THIS HANDLE MORE TEXTURES BY USING MULTIPLE TEXTURE LAYERS
	int result = stbi_write_png(output_path, atlasWidth, 4096, atlasChannels, atlas.data(), atlasWidth * atlasChannels);
	//int result = stbi_write_png(output_path, images[0].width, images[0].height, images[0].channels, images[0].data, images[0].width* images[0].channels);
	std::cout << " w: " << images[0].width << " h: " << images[0].height << " h: " << images[0].channels << std::endl;
	std::cout << " w: " << atlasWidth << " h: " << atlasHeight << " h: " << atlasChannels << std::endl;
	if (result == 0) {
		std::cerr << "Error writing image" << std::endl;
	}
	else {
		std::cout << "Atlas saved successfully" << std::endl;
	}
	const char* path_file = "resources/textures/image_paths.txt";
	std::ofstream outFile(path_file);
	if (outFile.is_open()) {
		for (const auto& img : images) {
			outFile << img.path << " | " << img.x << " , " << img.y << " , " << img.width << " , " << img.height << std::endl;
		}
		outFile.close();
		std::cout << "Image paths written to " << path_file << std::endl;
	}
	else {
		std::cerr << "Unable to open file for writing image paths" << std::endl;
	}
	// Cleanup
	for (auto& img : images) {
		stbi_image_free(img.data);
	}
}

void Graphics::addSpriteInstance(int textureId, glm::vec3 position, glm::vec2 size,
	glm::vec2 texOffset, glm::vec2 texSize,
	float rotation, float opacity) {
	if (textureId >= textures.size()) {
		std::cout << "Texture index " << textureId << " out of range" << std::endl;
		return;
	}

	std::vector<Sprite>& textureSprites = spriteInstances[textureId];
	textureSprites.emplace_back();
	Sprite& sprite = textureSprites[spriteInstanceIndexes[textureId]];
	spriteInstanceIndexes[textureId]++;

	sprite.texture = &textures[textureId];
	sprite.data = { textureId, position, size, glm::vec4(texOffset, texSize), rotation, opacity };

	if (textureSprites.size() >= textureSprites.capacity()) {
		textureSprites.reserve(textureSprites.size() + 5);
	}
}

void Graphics::resetSpriteInstances() {
	for (auto& instances : spriteInstances) {
		instances.clear();
	}
	std::fill(spriteInstanceIndexes.begin(), spriteInstanceIndexes.end(), 0);
}

void Graphics::getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags colorSampleCounts = physicalDeviceProperties.limits.framebufferColorSampleCounts;
	VkSampleCountFlags depthSampleCounts = physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	// Find the sample counts supported by both color and depth buffers
	VkSampleCountFlags counts = colorSampleCounts & depthSampleCounts;

	// Set the highest supported sample count
	if (counts & VK_SAMPLE_COUNT_64_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	}
	else if (counts & VK_SAMPLE_COUNT_32_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	}
	else if (counts & VK_SAMPLE_COUNT_16_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	}
	else if (counts & VK_SAMPLE_COUNT_8_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	}
	else if (counts & VK_SAMPLE_COUNT_4_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	}
	else if (counts & VK_SAMPLE_COUNT_2_BIT) {
		msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	}
	else {
		msaaSamples = VK_SAMPLE_COUNT_1_BIT; // Fallback to no MSAA support
	}
}