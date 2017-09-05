#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <set>

#define NOMINMAX

#define STB_IMAGE_IMPLEMENTATION

#include <unordered_map>

#include <glm/gtx/hash.hpp>

#include "Vertex.h";

//Uniform buffer struct containing variables to pass to the vertex shader
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
};

//Template for the Vertex struct used to hold vertex data that is draw by the GPU
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.colour) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

//initial width and height of the window
const int initialWidth = 1920;
const int initialHeight = 1080;

//path for the texture
const std::string TEXTURE_PATH = "textures/texture.png";

const int TextureWidth = 1000;
const int TextureHeight = 1000;

//VK_LAYER_LUNARG_standard_validation  is a layer that enables a lot of other useful debugging layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
//list of device extensions
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
//
////Creating the debug callback for getting the error when something goes wrong, for debugging purposes
//VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
//	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
//	if (func != nullptr) {
//		return func(instance, pCreateInfo, pAllocator, pCallback);
//	}
//	else {
//		return VK_ERROR_EXTENSION_NOT_PRESENT;
//	}
//}
//
////cleaning upthe debug callback 
//void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
//	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
//	if (func != nullptr) {
//		func(instance, callback, pAllocator);
//	}
//}


struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

//struct containing details about how the graphics card supports the swapchain
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};



//The Workspace class, containing all code handling input, and all vulkan graphics setup
class Graphics {
public:

	void initWindow();

	void initVulkan();

	void handleTiming();

	void drawString(std::string string, glm::vec2 pos);

	void drawFrame();

	void drawRect(float x, float y, float width, float height, float r, float g, float b, float a);

	void drawFlatImage(float x, float y, float width, float height, glm::vec2 imageMin, glm::vec2 imageMax);

	void drawCharacter(int i, glm::vec2 pos, bool upperCase);

	GLFWwindow* window;

	bool drawLines = false;

private:
	

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

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	std::vector<Vertex> triangleVertices;
	std::vector<uint32_t> triangleIndices;

	std::vector<Vertex> lineVertices;
	std::vector<uint32_t> lineIndices;

	VkBuffer triangleVertexBuffer;
	VkDeviceMemory triangleVertexBufferMemory;
	VkBuffer triangleIndexBuffer;
	VkDeviceMemory triangleIndexBufferMemory;

	VkBuffer lineVertexBuffer;
	VkDeviceMemory lineVertexBufferMemory;
	VkBuffer lineIndexBuffer;
	VkDeviceMemory lineIndexBufferMemory;

	VkBuffer uniformStagingBuffer;
	VkDeviceMemory uniformStagingBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	std::vector<VkCommandBuffer> commandBuffers;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;

	VkImageView depthImageView;

	int windowWidth, windowHeight;

	bool EXIT = false;

	float time;

	glm::vec2 cameraPosition;

	//VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

	//void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

	static void onWindowResized(GLFWwindow* window, int width, int height);

	void createDepthResources();

	VkFormat findDepthFormat();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	
	void createTextureSampler();

	void createTextureImageView();

	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

	void createTextureImage();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

	bool hasStencilComponent(VkFormat format);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createDescriptorSet();

	void createDescriptorPool();

	void createUniformBuffer();

	void createDescriptorSetLayout();

	void createIndexBuffer(std::vector<uint32_t> _indices, VkBuffer& _indexBuffer, VkDeviceMemory& _indexBufferMemory);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void clearBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);

	void createVertexBuffer(std::vector<Vertex> _vertices, VkBuffer& _vertexBuffer, VkDeviceMemory& _vertexBufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void recreateSwapChain();

	void createSemaphores();

	void createCommandBuffers(VkBuffer vertexBuffer);

	void createCommandPool();

	void createFramebuffers();

	void createRenderPass();

	void createGraphicsPipeline(VkPrimitiveTopology primitiveTopology);

	void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);

	void updateUniformBuffer();

	int getIntFromString(char letter[]);

	void createInstance();

	//void setupDebugCallback();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

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
};