#include "vdevice.hpp"
#include <unordered_set>

namespace pworx{
  VDevice::VDevice(){    
  }

  bool VDevice::initDevice(uint32_t width, uint32_t height){
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    pworx::VDevice::createWindow();
    pworx::VDevice::createInstance();
    pworx::VDevice::setupDebugMessenger();
    pworx::VDevice::createSurface();
    pworx::VDevice::pickPhysicalDevice();
    

    vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
    std::cout<<"Device name: "<<properties.deviceName<<std::endl;
    vkGetPhysicalDeviceFeatures(_physicalDevice, &features);
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, nullptr);
    if(extCount > 0){
      std::vector<VkExtensionProperties> extension(extCount);
      if(vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extCount, &extension.front()) == VK_SUCCESS){
        for(auto ext: extension){
          supportedExtension.push_back(ext.extensionName);
        }
      }
    }

    
    return true;
  }

  bool VDevice::createWindow(){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, _title.c_str(), nullptr, nullptr);
    if(_window == NULL){
      return false;
    }
    else return true;
  }

  void VDevice::createSurface(){
      if(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS){
    throw std::runtime_error("Failed to create window surface!");
  }
  }

  uint32_t VDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const{
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
      if((typeBits & 1) == 1){
        if((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties){
          if(memTypeFound){
            *memTypeFound = true;
          }
          return i;
        }
      }
      typeBits >>=1;
    }
    if(memTypeFound){
      *memTypeFound = false;
      return 0;
    }
    else{
      throw std::runtime_error("Could not find matching memory type");
    }
  }

  void VDevice::pickPhysicalDevice(){
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    if(deviceCount == 0) throw std::runtime_error("Failed to vulkan GPU.");
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for(const auto& device : devices){
      
      if(isDeviceSuitable(device)){
        _physicalDevice = device;
        std::cout<<"Physical device created. "<<std::endl;
        break;
      }
    }
    if(_physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("Failed to find suitable GPU");
    devices.clear();
  }

  bool VDevice::checkDeviceExtensionSupport(VkPhysicalDevice device){
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension : availableExtensions){
      requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
  }

  bool VDevice::isDeviceSuitable(VkPhysicalDevice device){
    queueIndices = pworx::VDevice::findQueueFamilies(device);

    bool extensionsSupported = pworx::VDevice::checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    querySwapChainSupport(device);
    if(extensionsSupported){
      SwapChainSupportDetails swapChainSupport = swapChainSupportDetails;
      swapChainAdequate = !swapChainSupport.formats.empty() &&
        !swapChainSupport.presentModes.empty();
    }

    return queueIndices.isComplete() && extensionsSupported && swapChainAdequate;
  }

  void VDevice::querySwapChainSupport(VkPhysicalDevice device){
    

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &swapChainSupportDetails.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
    swapChainSupportDetails.formatCount = formatCount;

    if(formatCount != 0){
      swapChainSupportDetails.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, swapChainSupportDetails.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
    swapChainSupportDetails.presentModeCount = presentModeCount;
    if(presentModeCount != 0){
      swapChainSupportDetails.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, swapChainSupportDetails.presentModes.data());
    }
    
  }

  pworx::VDevice::QueueFamilyIndices VDevice::findQueueFamilies(VkPhysicalDevice device){

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    int j = 0;
    for(const auto& queueFamily:queueFamilies){
      if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) queueIndices.graphicsFamily = i;
      //if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) j++;
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

      if(presentSupport) queueIndices.presentFamily = i;
      if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) queueIndices.computeFamily = i;
      if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) queueIndices.transferFamily = i;

      if(queueIndices.isComplete()) break;
      i++;
    }

    return queueIndices;
  }

  VkResult VDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions,void* pNextChain, bool useSwapChain, VkQueueFlags requestedQueueTypes){

		const float queuePriority(1.0f);
		
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueIndices.graphicsFamily.value(),
    queueIndices.presentFamily.value(), queueIndices.computeFamily.value(), queueIndices.transferFamily.value()};

    for(uint32_t queueFamily: uniqueQueueFamilies){
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain){
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain){
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		if (extensionsSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)){
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			enableDebugMarkers = true;
		}

		if (deviceExtensions.size() > 0){
			for (const char* enabledExtension : deviceExtensions)	{
				if (!extensionsSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->enabledFeatures = enabledFeatures;

		VkResult result = vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_logicalDevice);
		if (result != VK_SUCCESS){
			return result;
		}
    else{
      /* if(enableValidationLayers)  */std::cout<<"Logical device created. "<<std::endl;
    }

		// Create a default command pool for graphics command buffers
		commandPool = createCommandPool(queueIndices.graphicsFamily.value());

		return result;
  }

  VkResult VDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data){
    VkBufferCreateInfo bufferCreateInfo = pworx::VInfo::bufferCreateInfo(usageFlags, size);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if(vkCreateBuffer(_logicalDevice, &bufferCreateInfo, nullptr, buffer) != VK_SUCCESS){
      throw std::runtime_error("Failed to create buffer device buffer. ");
    }


		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = pworx::VInfo::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(_logicalDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;

		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		if(vkAllocateMemory(_logicalDevice, &memAlloc, nullptr, memory) != VK_SUCCESS){
      throw std::runtime_error("Failed to allocate device memory.");
    }
			
		if (data != nullptr){
			void *mapped;
			if(vkMapMemory(_logicalDevice, *memory, 0, size, 0, &mapped) != VK_SUCCESS){
        throw std::runtime_error("Failed to map device memory. ");
      }
			memcpy(mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0){
				VkMappedMemoryRange mappedRange = pworx::VInfo::mappedMemoryRange();
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(_logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(_logicalDevice, *memory);
		}

		if(vkBindBufferMemory(_logicalDevice, *buffer, *memory, 0) != VK_SUCCESS){
      throw std::runtime_error("Failed to bind buffer memory. ");
    }

		return VK_SUCCESS;
  }

  VkResult VDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, pworx::Buffer *buffer, VkDeviceSize size, void *data){
    buffer->device = _logicalDevice;

		VkBufferCreateInfo bufferCreateInfo = pworx::VInfo::bufferCreateInfo(usageFlags, size);
		if(vkCreateBuffer(_logicalDevice, &bufferCreateInfo, nullptr, &buffer->buffer) != VK_SUCCESS){
      throw std::runtime_error("Failed to create device buffer. ");
    }

	
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = pworx::VInfo::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(_logicalDevice, buffer->buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		if(vkAllocateMemory(_logicalDevice, &memAlloc, nullptr, &buffer->memory) != VK_SUCCESS){
      throw std::runtime_error("Failed to allocate device memory. ");
    }

		buffer->alignment = memReqs.alignment;
		buffer->size = size;
		buffer->usageFlags = usageFlags;
		buffer->memoryPropertyFlags = memoryPropertyFlags;

		if (data != nullptr){
			if(buffer->map() != VK_SUCCESS){
        throw std::runtime_error("Failed to map buffer. ");
      }
			memcpy(buffer->mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
				buffer->flush();

			buffer->unmap();
		}

		buffer->setupDescriptor();

		return buffer->bind();
  }

  
  void VDevice::copyBuffer(pworx::Buffer *src, pworx::Buffer *dst, VkQueue queue, VkBufferCopy* copyRegion){
    assert(dst->size <= src->size);
		assert(src->buffer);
		VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy bufferCopy{};
		if (copyRegion == nullptr){
			bufferCopy.size = src->size;
		}
		else{
			bufferCopy = *copyRegion;
		}

		vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

		flushCommandBuffer(copyCmd, queue);
  }

  void VDevice::copyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkQueue graphicsQueue) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer, graphicsQueue);
}

VkCommandBuffer VDevice::beginSingleTimeCommands(){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }
  void VDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue graphicsQueue){
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    //vkDeviceWaitIdle(_logicalDevice);
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(_logicalDevice, commandPool, 1, &commandBuffer);
  }


  VkCommandPool VDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags){
    VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		if(vkCreateCommandPool(_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS){
      throw std::runtime_error("Failed to create command pool. ");
    }
		return cmdPool;
  }

  VkCommandBuffer VDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin){
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = pworx::VInfo::commandBufferAllocateInfo(pool, level, 1);
		VkCommandBuffer cmdBuffer;
		if(vkAllocateCommandBuffers(_logicalDevice, &cmdBufAllocateInfo, &cmdBuffer) != VK_SUCCESS){
      throw std::runtime_error("Failed to allocate command buffer");
    }
		// If requested, also start recording for the new command buffer
		if (begin){
			VkCommandBufferBeginInfo cmdBufInfo = pworx::VInfo::commandBufferBeginInfo();
			if(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin command buffer. ");
      }
		}
		return cmdBuffer;
  }

  VkCommandBuffer VDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin){
    return createCommandBuffer(level, commandPool, begin);
  }

  void VDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free){
    if (commandBuffer == VK_NULL_HANDLE){
			return;
		}

		if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
      throw std::runtime_error("Failed to end command buffer. ");
    }

		VkSubmitInfo submitInfo = pworx::VInfo::submitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		VkFenceCreateInfo fenceInfo = pworx::VInfo::fenceCreateInfo(VK_FLAGS_NONE);
		VkFence fence;
		if(vkCreateFence(_logicalDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS){
      throw std::runtime_error("Failed to create fence. ");
    }
		if(vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS){
      throw std::runtime_error("Failed to submit queue. ");
    }
		if(vkWaitForFences(_logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS){
      throw std::runtime_error("Failed to wait for device fence. ");
    }
		vkDestroyFence(_logicalDevice, fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(_logicalDevice, pool, 1, &commandBuffer);
		}
  }

  void VDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free){
		return flushCommandBuffer(commandBuffer, queue, commandPool, free);
	}

  bool VDevice::extensionsSupported(std::string extension){
		return (std::find(supportedExtension.begin(), supportedExtension.end(), extension) != supportedExtension.end());
	}

  VkFormat VDevice::getSupportedDepthFormat(bool checkSamplingSupport){
		std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
		for (auto& format : depthFormats)		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
				if (checkSamplingSupport) {
					if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
						continue;
					}
				}
				return format;
			}
		}
		throw std::runtime_error("Could not find a matching depth format");
	}

  std::vector<const char*> VDevice::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }
  void VDevice::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = pworx::VDevice::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        pworx::VDevice::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }
  }

  void VDevice::createSwapChain(uint32_t *width, uint32_t *height, bool vsync){
  
    VkSwapchainKHR oldSwapchain = swapChainz;
    /* if(firstSwap){
      oldSwapchain = 0;
      firstSwap = false;
    }else{
      oldSwapchain = swapChainz;
    } */

     
    querySwapChainSupport(_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
    colorFormat = surfaceFormat.format;
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupportDetails.capabilities);

    VkExtent2D swapchainExtent = {};
    if (swapChainSupportDetails.capabilities.currentExtent.width == (uint32_t)-1){

      swapchainExtent.width = *width;
      swapchainExtent.height = *height;
    }
    else{
      swapchainExtent = swapChainSupportDetails.capabilities.currentExtent;
      *width = swapChainSupportDetails.capabilities.currentExtent.width;
      *height = swapChainSupportDetails.capabilities.currentExtent.height;
    }

    	uint32_t desiredNumberOfSwapchainImages = swapChainSupportDetails.capabilities.minImageCount + 1;
      if ((swapChainSupportDetails.capabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > swapChainSupportDetails.capabilities.maxImageCount))
      {
        desiredNumberOfSwapchainImages = swapChainSupportDetails.capabilities.maxImageCount;
      }

      // Find the transformation of the surface
      VkSurfaceTransformFlagsKHR preTransform;
      if (swapChainSupportDetails.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
      {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
      }
      else
      {
        preTransform = swapChainSupportDetails.capabilities.currentTransform;
      }

      VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

      std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      };
      for (auto& compositeAlphaFlag : compositeAlphaFlags) {
        if (swapChainSupportDetails.capabilities.supportedCompositeAlpha & compositeAlphaFlag) {
          compositeAlpha = compositeAlphaFlag;
          break;
        };
      }

      VkSwapchainCreateInfoKHR swapchainCI = {};
      swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      swapchainCI.surface = _surface;
      swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
      swapchainCI.imageFormat = surfaceFormat.format;
      swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
      swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
      swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
      swapchainCI.imageArrayLayers = 1;
      swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapchainCI.queueFamilyIndexCount = 0;
      swapchainCI.presentMode = presentMode;
      // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
      swapchainCI.oldSwapchain = oldSwapchain;
      // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
      swapchainCI.clipped = VK_TRUE;
      swapchainCI.compositeAlpha = compositeAlpha;

      // Enable transfer source on swap chain images if supported
      if (swapChainSupportDetails.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      }

      // Enable transfer destination on swap chain images if supported
      if (swapChainSupportDetails.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      }

      if(vkCreateSwapchainKHR(_logicalDevice, &swapchainCI, nullptr, &swapChainz) != VK_SUCCESS){
        throw std::runtime_error("Failed to create swap chain. ");
      }
      else{
        if(enableValidationLayers){
          std::cout<<"Swapcahin created"<<std::endl;
        }
        
      }

      if (oldSwapchain != VK_NULL_HANDLE) 
      { 
        for (uint32_t i = 0; i < imageCount; i++)
        {
          vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
        }
        vkDestroySwapchainKHR(_logicalDevice, oldSwapchain, nullptr);
      }
      if(vkGetSwapchainImagesKHR(_logicalDevice, swapChainz, &imageCount, NULL) != VK_SUCCESS){
        throw std::runtime_error("Failed to get swap chain images.");
      }
      images.resize(imageCount);
      if(vkGetSwapchainImagesKHR(_logicalDevice, swapChainz, &imageCount, images.data()));

      // Get the swap chain buffers containing the image and imageview
      buffers.resize(imageCount);
      for (uint32_t i = 0; i < imageCount; i++)
      {
        VkImageViewCreateInfo colorAttachmentView = {};
        colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorAttachmentView.pNext = NULL;
        colorAttachmentView.format = surfaceFormat.format;
        colorAttachmentView.components = {
          VK_COMPONENT_SWIZZLE_R,
          VK_COMPONENT_SWIZZLE_G,
          VK_COMPONENT_SWIZZLE_B,
          VK_COMPONENT_SWIZZLE_A
        };
        colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentView.subresourceRange.baseMipLevel = 0;
        colorAttachmentView.subresourceRange.levelCount = 1;
        colorAttachmentView.subresourceRange.baseArrayLayer = 0;
        colorAttachmentView.subresourceRange.layerCount = 1;
        colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorAttachmentView.flags = 0;

        buffers[i].image = images[i];

        colorAttachmentView.image = buffers[i].image;

        if(vkCreateImageView(_logicalDevice, &colorAttachmentView, nullptr, &buffers[i].view) != VK_SUCCESS){
          throw std::runtime_error("Failed to create image view. ");
        }
      }
  }

  VkResult VDevice::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore){
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChainz;
    presentInfo.pImageIndices = &imageIndex;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (waitSemaphore != VK_NULL_HANDLE)
    {
      presentInfo.pWaitSemaphores = &waitSemaphore;
      presentInfo.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &presentInfo);
  }

  VkResult VDevice::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex){
    return vkAcquireNextImageKHR(_logicalDevice, swapChainz, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
  }


  VkSurfaceFormatKHR VDevice::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
    for(const auto& availableFormat : availableFormats){
      if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM){
       
        return availableFormat;
      }
      else if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
        return availableFormat;
      }
    }
    return availableFormats[0];
  }


  VkPresentModeKHR VDevice::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
    for(const auto& availablePresentMode : availablePresentModes){
      if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR){
        return availablePresentMode;
      }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D VDevice::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
    if(swapChainSupportDetails.capabilities.currentExtent.width != UINT32_MAX){
      return swapChainSupportDetails.capabilities.currentExtent;
    }
    else{
      int width, height;
      glfwGetFramebufferSize(_window, &width, &height);

      VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
      };
      actualExtent.width = std::clamp(actualExtent.width, swapChainSupportDetails.capabilities.minImageExtent.width,
        swapChainSupportDetails.capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height, swapChainSupportDetails.capabilities.minImageExtent.height,
        swapChainSupportDetails.capabilities.maxImageExtent.height);
      
      return actualExtent;
    }
  }


  void VDevice::setupDebugMessenger(){
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    pworx::VDevice::populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
  }
  
  void VDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  bool VDevice::checkValidationLayerSupport(){
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

  VkResult VDevice::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void VDevice::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
  }

  void VDevice::swapCleanUp(){
    if (swapChainz != VK_NULL_HANDLE)
    {
      for (uint32_t i = 0; i < imageCount; i++)
      {
        vkDestroyImageView(_logicalDevice, buffers[i].view, nullptr);
      }
    }
    if (_surface != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(_logicalDevice, swapChainz, nullptr);
      vkDestroySurfaceKHR(_instance, _surface, nullptr);
    }
    _surface = VK_NULL_HANDLE;
    swapChainz = VK_NULL_HANDLE;
  }
  VDevice::~VDevice(){
    vkDeviceWaitIdle(_logicalDevice);
    if(commandPool != VK_NULL_HANDLE){
      vkDestroyCommandPool(_logicalDevice, commandPool, _allocator);
    }
    vkDestroyDevice(_logicalDevice, _allocator);
    if(enableValidationLayers){
      DestroyDebugUtilsMessengerEXT(_instance, debugMessenger, _allocator);
    }
    vkDestroySurfaceKHR(_instance, _surface, _allocator);
    vkDestroyInstance(_instance, _allocator);
    glfwDestroyWindow(_window);
    glfwTerminate();
  }
}

