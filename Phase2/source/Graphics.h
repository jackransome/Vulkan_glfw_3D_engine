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
	DescriptorInfo() : type(VK_DESCRIPTOR_TYPE_MAX_ENUM), buffer(nullptr), imageView(nullptr), sampler(nullptr),
		bufferOffset(0), bufferRange(0), imageLayout(VK_IMAGE_LAYOUT_UNDEFINED) {
	}
};

struct descriptorResource {
	descriptorResource(std::vector<VkBuffer> uniformBuffers) : uniformBuffers(uniformBuffers) {}
	descriptorResource(VkBuffer storageBuffer) : storageBuffer(storageBuffer) {}
	descriptorResource(VkSampler textureSampler, VkImageView textureImageView) : textureImageView(textureImageView), textureSampler(textureSampler) {}
	std::vector<VkBuffer> uniformBuffers;
	VkBuffer storageBuffer;
	VkImageView textureImageView;
	VkSampler textureSampler;
};

struct descriptorSetObject {
	descriptorSetObject(std::string name, VkDescriptorType type, VkShaderStageFlags stageFlags, size_t size, int count = 1) :
		name(name),
		type(type),
		stageFlags(stageFlags),
		size(size),
		count(count) {
	}
	std::string name;
	VkDescriptorType type;
	int binding;
	DescriptorInfo info;
	int count;
	VkShaderStageFlags stageFlags;
	size_t size;
};

struct PipelineBundle {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSet> descriptorSets;  // One per swap chain image
	std::vector<DescriptorInfo> descriptorInfos;  // New member to store descriptor information
	std::vector<descriptorSetObject> descriptorSetObjects;
	// Constructor to initialize members
	PipelineBundle(VkPipeline p, VkPipelineLayout pl,
		std::vector<VkDescriptorSet> ds,
		std::vector<DescriptorInfo> di,
		std::vector<descriptorSetObject> dso)
		: pipeline(p), pipelineLayout(pl),
		descriptorSets(std::move(ds)), descriptorInfos(std::move(di)), descriptorSetObjects(std::move(dso)) {
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

struct StorageBufferObject {
	std::string name;
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
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
	glm::vec2 textureOffset;
	glm::vec2 textureSize;
	bool hasNormalMap;
	glm::vec2 normalTextureOffset;
	glm::vec2 normalTextureSize;
};

//Object struct
struct RenderInstance {
	Model *model;
	glm::mat4 transformData;
};

struct ImageInfo {
	std::string name;
	glm::vec2 coordinates;
	glm::vec2 dimensions;
};

//sprite stuff is for 2D
struct SpriteData {
	int textureId;
	glm::vec3 position;
	glm::vec2 size;
	glm::vec4 texOffsetSize; // xy = texOffset, zw = texSize
	float rotation;
	float opacity;
};

struct Sprite {
	SpriteData data;
	Texture* texture;
};

struct UniformBufferObject {
	//glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
};

struct PushConstants {
	int index;
	float textureOffsetX;
	float textureOffsetY;
	float textureWidth;
	float textureHeight;
	int hasNormalMap;
	float normalTextureOffsetX;
	float normalTextureOffsetY;
	float normalTextureWidth;
	float normalTextureHeight;
};
static_assert(sizeof(PushConstants) == 40, "PushConstants struct size must be 40 bytes");

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

struct LightData {
    glm::vec3 position;
	float padding;
    glm::vec3 color;
    float intensity;
};

struct PushConstantInfo {
	PushConstantInfo(uint32_t size, VkShaderStageFlags stageFlags) : size(size), stageFlags(stageFlags), offset(-1) {}
	uint32_t size;
	VkShaderStageFlags stageFlags;
	uint32_t offset;
};

const std::string MODEL_PATH = "resources/models/chalet.obj";
const std::string TEXTURE_PATH = "resources/textures/normal.png";

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

	void addLight(glm::vec3 position, glm::vec3 color, float intensity);

	glm::vec3 getCameraPos();

private:

	std::vector<descriptorSetObject> descriptorSetObjects;

	std::vector<PushConstantInfo> pushConstantInfos;

	std::vector<Texture> textures;

	int textureWidth, textureHeight;

	std::vector<Model> models;

	std::vector<std::vector<RenderInstance>> renderInstances;

	std::vector<size_t> renderInstanceIndexes;

	std::vector<std::vector<Sprite>> spriteInstances;

	std::vector<size_t> spriteInstanceIndexes;
	
	std::vector<LightData> lights;

	size_t totalRenderInstances = 0;

	const int MAX_RENDER_INSTANCES = 50000;

	const int WIDTH = 1920;
	const int HEIGHT = 1080;
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

	StorageBufferObject transformBuffer;
	StorageBufferObject lightBuffer;

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

	std::vector<ImageInfo> atlasOffsets;

	VkSampleCountFlagBits msaaSamples;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	void updatePushConstants(VkCommandBuffer commandBuffer,
							VkPipelineLayout pipelineLayout,
							const PushConstantInfo& pcInfo,
							const void* data);

	void printSampleCount();

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void createColorResources();

	void getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

	glm::vec2 getAtlasOffset(std::string textureName);

	glm::vec2 getAtlasSize(std::string textureName);

	void readImageInfoFromFile(const std::string& filePath);

	void loadResources();

	//void loadModels();

	void loadObjects();

	void setUpCamera();

	void resetRenderInstances();

	void addSpriteInstance(int textureId, glm::vec3 position, glm::vec2 size, glm::vec2 texOffset, glm::vec2 texSize, float rotation, float opacity);

	void resetSpriteInstances();

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

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void loadModel(std::string path, glm::vec4 colour, float scale);

	void createVertexBuffer();

	void createIndexBuffer();

	void clearStorageBuffer(StorageBufferObject& storageBuffer);
	StorageBufferObject createStorageBuffer(std::string name, VkDeviceSize size);

	template<typename T>
    void updateStorageBuffer(StorageBufferObject storageBuffer, const std::vector<T>& data);

	void createUniformBuffers();

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
		VkRenderPass renderPass, VkPipelineLayout& pipelineLayout, std::vector<PushConstantInfo> &pushConstantInfos);

	VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	
	std::vector<VkDescriptorSet> createDescriptorSets(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, uint32_t count);

	void updateDescriptorSet(const PipelineBundle& bundle, int index);

	PipelineBundle createCurrentPipelineBundle(std::vector<descriptorSetObject> descriptorSetObjects, VkExtent2D swapChainExtent, VkRenderPass renderPass, uint32_t swapChainImageCount, std::vector<PushConstantInfo> &pushConstantInfos);

	VkDescriptorPool createDescriptorPool(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);

	void updateDescriptorResource(PipelineBundle& bundle, std::string name, descriptorResource& resource);

	void createTextureAtlasArray(std::vector<std::string> texturePaths);

};