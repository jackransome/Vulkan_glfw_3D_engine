#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>



#include <vector>
#include <array>
#include <set>
#include <chrono>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cstdlib>

struct DescriptorInfo {
	VkDescriptorType type;
	std::vector<VkBuffer> buffers;  // For uniform buffers
	VkBuffer buffer;               // For storage buffer
	VkImageView imageView;
	VkSampler sampler;
	VkDeviceSize bufferOffset;
	VkDeviceSize bufferRange;
	VkImageLayout imageLayout;

	DescriptorInfo(VkDescriptorType t) : type(t), buffer(nullptr), imageView(nullptr), sampler(nullptr),
		bufferOffset(0), bufferRange(0), imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
	}
};

struct PipelineBundle {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;  // One per swap chain image
	VkDescriptorPool descriptorPool;
	std::vector<DescriptorInfo> descriptorInfos;  // New member to store descriptor information

	// Constructor to initialize members
	PipelineBundle(VkPipeline p, VkPipelineLayout pl, VkDescriptorSetLayout dsl,
		std::vector<VkDescriptorSet> ds, VkDescriptorPool dp,
		std::vector<DescriptorInfo> di)
		: pipeline(p), pipelineLayout(pl), descriptorSetLayout(dsl),
		descriptorSets(std::move(ds)), descriptorPool(dp), descriptorInfos(std::move(di)) {
	}

	// Destructor to clean up Vulkan resources
	~PipelineBundle() {
		// Note: Add device destruction calls here if ownership is meant to be here
		// Otherwise, ensure these are destroyed in your main Vulkan management class
	}

	// Prevent copying
	PipelineBundle(const PipelineBundle&) = delete;
	PipelineBundle& operator=(const PipelineBundle&) = delete;

	// Allow moving
	PipelineBundle(PipelineBundle&&) = default;
	PipelineBundle& operator=(PipelineBundle&&) = default;
};

struct Texture {
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	std::string name;
	VkImageView textureImageView;
	int width;
	int height;
	uint32_t mipLevels;
};

//model struct
struct Model {
	uint32_t offset;
	uint32_t size;
	VkImageView textureImageView;
	VkImageView normalMapImageView;
};

//Object struct
struct RenderInstance {
	Model *model;
	glm::mat4 transformData;
};

struct UniformBufferObject {
	//glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 diffuse;   // Kd
	glm::vec3 specular;  // Ks
	glm::vec3 ambient;   // Ka
	float shininess;     // Ns
	float opacity;       // d or Tr

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, diffuse);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, specular);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, ambient);

		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32_SFLOAT;
		attributeDescriptions[6].offset = offsetof(Vertex, shininess);

		attributeDescriptions[7].binding = 0;
		attributeDescriptions[7].location = 7;
		attributeDescriptions[7].format = VK_FORMAT_R32_SFLOAT;
		attributeDescriptions[7].offset = offsetof(Vertex, opacity);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord &&
			diffuse == other.diffuse && specular == other.specular && ambient == other.ambient &&
			shininess == other.shininess && opacity == other.opacity;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};



const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/normal.png";

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Graphics {
public:

	bool shouldClose = false;

	void init();

	void run();

	void cleanup();

	void changeCameraPos(float x, float y, float z);

	void setCameraAngle(glm::vec3 cameraAngle);

	void setCameraPos(glm::vec3 cameraPos);

	glm::vec3 getProperCameraVelocity(glm::vec3 cameraVel);

	GLFWwindow* getWindowPointer();

	void addRenderInstance(float x, float y, float z, int modelIndex);

	glm::vec3 getCameraPos();

private:

	std::vector<Texture> textures;

	std::vector<Model> models;

	std::vector<std::vector<RenderInstance>> renderInstances;

	std::vector<size_t> renderInstanceIndexes;

	size_t totalRenderInstances = 0;

	const int MAX_RENDER_INSTANCES = 50000;

	const int WIDTH = 800;
	const int HEIGHT = 600;
	const int MAX_FRAMES_IN_FLIGHT = 2;

	glm::vec3 cameraAngle;

	glm::vec3 cameraPosition;

	float FOV = 90;

	glm::vec3 direction;

	glm::vec3 right;

	glm::vec3 up;

	float cameraVelocity = 0.5;

	GLFWwindow* window;

	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer storageBuffer;
	VkDeviceMemory storageBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	std::vector<PipelineBundle> pipelineBundles;

	bool framebufferResized = false;

	void loadResources();

	//void loadModels();

	void loadObjects();

	void setUpCamera();

	void resetRenderInstances();

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}

	uint32_t loadTexture(const std::string& texturePath);

	void initWindow();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	void initVulkan();

	void mainLoop();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createInstance();

	void setupDebugCallback();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createRenderPass();

	void createDescriptorSetLayout();

	void createGraphicsPipeline();

	void createFramebuffers();

	void createCommandPool();

	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	void createTextureImage();

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void createTextureImageView();

	void createTextureSampler();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void loadModel(std::string path, glm::vec4 colour, float scale);

	void createVertexBuffer();

	void createIndexBuffer();

	void clearStorageBuffer();

	void createStorageBuffer();

	void updateStorageBuffer();

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createCommandBuffers();

	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);

	void drawFrame();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();

	bool checkValidationLayerSupport();

	static std::vector<char> readFile(const std::string& filename);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

	VkPipeline createGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath,
		VkDescriptorSetLayout descriptorSetLayout, VkExtent2D swapChainExtent,
		VkRenderPass renderPass, VkPipelineLayout& pipelineLayout);

	VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	
	std::vector<VkDescriptorSet> createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t count);

	void updateDescriptorSet(const PipelineBundle& bundle);

	PipelineBundle createCurrentPipelineBundle(VkExtent2D swapChainExtent, VkRenderPass renderPass, uint32_t swapChainImageCount);

	VkDescriptorPool createDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);

	void updatePipelineBundleResources(PipelineBundle& bundle, const std::vector<VkBuffer> uniformBuffers, VkBuffer storageBuffer, VkImageView textureImageView, VkSampler textureSampler);

};