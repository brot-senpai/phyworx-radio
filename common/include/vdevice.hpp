#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "vbuffer.hpp"
#include "vinfo.hpp"
#include <algorithm>
#include <assert.h>
#include <exception>
#include <string>
#include <iostream>
#include <optional>
#include <set>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}



namespace pworx{
  struct VDevice{
    std::string                             _title = "";
    VkPhysicalDevice                        _physicalDevice = VK_NULL_HANDLE;
    VkDevice                                _logicalDevice = VK_NULL_HANDLE;
    VkInstance                              _instance = VK_NULL_HANDLE;
    GLFWwindow*                             _window = nullptr;
    VkSurfaceKHR                            _surface = VK_NULL_HANDLE;
    VkAllocationCallbacks*                  _allocator = nullptr;
    uint32_t                                SCR_WIDTH;
    uint32_t                                SCR_HEIGHT;

    VkPhysicalDeviceProperties              properties;
    VkPhysicalDeviceFeatures                features;
    VkPhysicalDeviceFeatures                enabledFeatures;
    VkPhysicalDeviceMemoryProperties        memoryProperties;
    std::vector<VkQueueFamilyProperties>    queueFamilyProperties;
    std::vector<std::string>                supportedExtension;
    std::vector<const char*>                enabledExtensions;
    void*                                   deviceCreatepNextChain = nullptr;
    VkCommandPool                           commandPool = VK_NULL_HANDLE;
    std::string                             deviceName;    
    bool                                    enableDebugMarkers = false;      
    VkDebugUtilsMessengerEXT                debugMessenger = VK_NULL_HANDLE;
    VkSwapchainKHR                          swapChainz = VK_NULL_HANDLE;
    uint32_t                                imageCount;
    VkFormat                                colorFormat;
    bool                                    firstSwap = true;

    struct QueueFamilyIndices{
      std::optional<uint32_t> graphicsFamily;
      std::optional<uint32_t> presentFamily;
      std::optional<uint32_t> transferFamily;
      std::optional<uint32_t> computeFamily;

      bool isComplete(){
        return graphicsFamily.has_value()
          && presentFamily.has_value();
      }
    };
    QueueFamilyIndices queueIndices;
    
    struct SwapChainSupportDetails{
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
      uint32_t formatCount;
      uint32_t presentModeCount;
    };

    typedef struct _SwapChainBuffers {
      VkImage image;
      VkImageView view;
    } SwapChainBuffer;


    std::vector<VkImage>                      images;
    std::vector<SwapChainBuffer>              buffers;

    SwapChainSupportDetails swapChainSupportDetails;

    operator VkDevice() const{
      return _logicalDevice;
    }
    explicit                    VDevice();
    bool                        initDevice(uint32_t, uint32_t);
    ~VDevice();

    void                        createSurface();
    uint32_t                    getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
    VkResult                    createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions,void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkResult                    createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlas, VkDeviceSize size,
                                  VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
    VkResult                    createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, pworx::Buffer *buffer,
                                  VkDeviceSize size, void* data = nullptr);
    void                        copyBuffer(pworx::Buffer *src, pworx::Buffer *dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
    VkCommandBuffer             beginSingleTimeCommands();
    void                        endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsQueue);
    void                        copyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkQueue graphicsQueue);
    VkCommandPool               createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer             createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
    VkCommandBuffer             createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
    void                        flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
    void                        flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
    bool                        extensionsSupported(std::string extension);
    VkFormat                    getSupportedDepthFormat(bool checkSamplingSupport);
    bool                        createWindow();
    void                        createInstance();
    void                        setupDebugMessenger();
    std::vector<const char*>    getRequiredExtensions();
    void                        populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void                        pickPhysicalDevice();
    bool                        isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices          findQueueFamilies(VkPhysicalDevice device);
    void                        querySwapChainSupport(VkPhysicalDevice device);
    bool                        checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool                        checkValidationLayerSupport();
    VkResult                    CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger); 
    void                        DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    VkExtent2D                  chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkPresentModeKHR            chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkSurfaceFormatKHR          chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    void                        createSwapChain(uint32_t *width, uint32_t *height, bool vsync = false);
    VkResult                    queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
    VkResult                    acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex);
    
    void                        swapCleanUp();
  };
}