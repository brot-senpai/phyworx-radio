#include "vbase.hpp"


VBase::VBase(){
  viewUpdated = false;
}

bool VBase::initVk(){  
  getEnabledFeatures();
  VkResult res = vdevice.createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
  if (res != VK_SUCCESS) {
		pworx::Utils::exitFatal("Could not create Vulkan device: \n" + pworx::Utils::errorString(res), res);
		return false;
	}
  vdevice._logicalDevice;

  vkGetDeviceQueue(vdevice._logicalDevice, vdevice.queueIndices.graphicsFamily.value(), 0, &_graphicsQueue);
  if(_graphicsQueue == VK_NULL_HANDLE){
    throw std::runtime_error("Failed to get graphics queue. ");
  }
  else std::cout<<"Graphics queue created. "<<std::endl;

  VkBool32 validDepthFormat = pworx::Utils::getSupportedDepthFormat(vdevice._physicalDevice, &depthFormat);
  assert(depthFormat);

  VkSemaphoreCreateInfo semaphoreCreateInfo = pworx::VInfo::semaphoreCreateInfo();
  if(vkCreateSemaphore(vdevice._logicalDevice, &semaphoreCreateInfo, vdevice._allocator, &semaphores.presentComplete) != VK_SUCCESS){
    throw std::runtime_error("Failed to create present complete semaphore. ");
  }
  if(vkCreateSemaphore(vdevice._logicalDevice, &semaphoreCreateInfo, vdevice._allocator, &semaphores.renderComplete) != VK_SUCCESS){
    throw std::runtime_error("Failed to create render complete semaphore. ");
  }

  submitInfo = pworx::VInfo::submitInfo();
  submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphores.presentComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphores.renderComplete;
  return true;
}

VkResult VBase::createCommandPool(){
  VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vdevice.queueIndices.graphicsFamily.value();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	return vkCreateCommandPool(vdevice._logicalDevice, &cmdPoolInfo, nullptr, &cmdPool);
}

void VBase::setupSwapChain(){  

  vdevice.createSwapChain(&width, &height, settings.vsync);
}

void VBase::createCommandBuffers(){
  drawCmdBuffers.resize(vdevice.imageCount);
  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		pworx::VInfo::commandBufferAllocateInfo(
			cmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			static_cast<uint32_t>(drawCmdBuffers.size()));
  if(vkAllocateCommandBuffers(vdevice._logicalDevice, &cmdBufAllocateInfo, drawCmdBuffers.data()) != VK_SUCCESS){
    throw std::runtime_error("Failed to create command buffer. ");
  }
  else{
    if(enableValidationLayers){
      std::cout<<"Command buffers created, size:  "<<drawCmdBuffers.size()<<std::endl;
    }
  }
}

void VBase::getEnabledFeatures(){}

void VBase::createSynchronizationPrimitives(){
  VkFenceCreateInfo fenceCreateInfo = pworx::VInfo::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	waitFences.resize(drawCmdBuffers.size());
	for (auto& fence : waitFences) {
		if(vkCreateFence(vdevice._logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS){
      throw std::runtime_error("Failed to create fences. ");
    }
    else{
      if(enableValidationLayers){
        std::cout<<"Fences created. "<<std::endl;
      }      
    }
	}
}

void VBase::setupDepthStencil(){
  VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = depthFormat;
	imageCI.extent = { width, height, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if(vkCreateImage(vdevice._logicalDevice, &imageCI, nullptr, &depthStencil.image) != VK_SUCCESS){
    throw std::runtime_error("Failed to create image. ");
  }
	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(vdevice._logicalDevice, depthStencil.image, &memReqs);

	VkMemoryAllocateInfo memAllloc{};
	memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllloc.allocationSize = memReqs.size;
	memAllloc.memoryTypeIndex = vdevice.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if(vkAllocateMemory(vdevice._logicalDevice, &memAllloc, nullptr, &depthStencil.mem) != VK_SUCCESS){
    throw std::runtime_error("Failed to allocate depth stencil memory.");
  }
	if(vkBindImageMemory(vdevice._logicalDevice, depthStencil.image, depthStencil.mem, 0) != VK_SUCCESS){
    throw std::runtime_error("Failed to bind image memory.");
  }

	VkImageViewCreateInfo imageViewCI{};
	imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.image = depthStencil.image;
	imageViewCI.format = depthFormat;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.layerCount = 1;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	if(vkCreateImageView(vdevice._logicalDevice, &imageViewCI, nullptr, &depthStencil.view) != VK_SUCCESS){
    throw std::runtime_error("Failed to create depth image view. ");
  }
  else{
    if(enableValidationLayers){
      std::cout<<"Depth stencil image view created. "<<std::endl;
    }    
  }
}

void VBase::setupRenderPass(){
  std::array<VkAttachmentDescription, 2> attachments = {};
  
	attachments[0].format = vdevice.colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

  if(vkCreateRenderPass(vdevice._logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS){
    throw std::runtime_error("Failed to create render pass. ");
  }
  else{
    if(enableValidationLayers){
      std::cout<<"Renderpass created. "<<std::endl;
    }
  }
}

void VBase::createPipelineCache(){
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if(vkCreatePipelineCache(vdevice._logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS){
    throw std::runtime_error("Failed to create pipeline cache. ");
  }
  else{
    if(enableValidationLayers){
      std::cout<<"Pipeline cache created. "<<std::endl;
    }
  }
}

void VBase::setupFrameBuffer(){
  VkImageView attachments[2];

	attachments[1] = depthStencil.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	frameBuffers.resize(vdevice.imageCount);
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		attachments[0] = vdevice.buffers[i].view;
		if(vkCreateFramebuffer(vdevice._logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS){
      throw std::runtime_error("Failed to create frame buffers. ");
    }
    else{
      if(enableValidationLayers){
        std::cout<<"Frame buffers created. "<<std::endl;
      }
    }
	}
}

void VBase::submitFrame(){

	VkResult result = vdevice.queuePresent(_graphicsQueue, currentBuffer, semaphores.renderComplete);
	if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			windowResize();
			return;
		}
    else{
      if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to present queue. ");
      }
    }
  }
  if(vkQueueWaitIdle(_graphicsQueue) != VK_SUCCESS){
    throw std::runtime_error("Queue failed to wait idle. ");
  }
}
void VBase::prepareFrame(){
  VkResult result = vdevice.acquireNextImage(semaphores.presentComplete, &currentBuffer);
	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		windowResize();
	}
	else {
		if(result != VK_SUCCESS){
      throw std::runtime_error("Failed to acquire next image. ");
    }
	}
}
void VBase::windowResize(){
  if (!prepared)
	{
		return;
	}
	prepared = false;
	resized = true;

	vkDeviceWaitIdle(vdevice._logicalDevice);

	width = destWidth;
	height = destHeight;
  vdevice.SCR_WIDTH = width;
  vdevice.SCR_HEIGHT = height;
	setupSwapChain();

	vkDestroyImageView(vdevice._logicalDevice, depthStencil.view, nullptr);
	vkDestroyImage(vdevice._logicalDevice, depthStencil.image, nullptr);
	vkFreeMemory(vdevice._logicalDevice, depthStencil.mem, nullptr);
	setupDepthStencil();
	for (uint32_t i = 0; i < frameBuffers.size(); i++) {
		vkDestroyFramebuffer(vdevice._logicalDevice, frameBuffers[i], nullptr);
	}
	setupFrameBuffer();

	if ((width > 0.0f) && (height > 0.0f)) {
		if (settings.overlay) {
			vimgui.resize(width, height);
		}
	}

	destroyCommandBuffers();
	createCommandBuffers();
	buildCommandBuffers();

	//vkDeviceWaitIdle(vdevice._logicalDevice);

	windowResized();
	viewChanged();

	prepared = true;
}
void VBase::windowResized() {}
void VBase::buildCommandBuffers() {}

void VBase::renderLoop(){
  if (benchmark.active) {
		benchmark.run([=] { render(); }, vdevice.properties);
		//vkDeviceWaitIdle(vdevice._logicalDevice);
		if (benchmark.filename != "") {
			benchmark.saveResults();
		}
		return;
	}

	destWidth = width;
	destHeight = height;
	lastTimestamp = std::chrono::high_resolution_clock::now();
  

  while (!glfwWindowShouldClose(vdevice._window))
	{
		auto tStart = std::chrono::high_resolution_clock::now();
		    
		double xpos, ypos;
    glfwGetCursorPos(vdevice._window, &xpos, &ypos);
    mousePos.x = xpos;
		mousePos.y = ypos;
		
    int mouseState = glfwGetMouseButton(vdevice._window, GLFW_MOUSE_BUTTON_LEFT);
    int rightMouseState = glfwGetMouseButton(vdevice._window, GLFW_MOUSE_BUTTON_RIGHT);
    if(rightMouseState == GLFW_PRESS){
			if(io.MouseDelta.x < 0){
				camera.ProcessKeyboard(LEFT, frameTimer);
				vimgui.updated = true;
    		viewUpdated = true;
			}
			if(io.MouseDelta.x > 0){
				camera.ProcessKeyboard(RIGHT, frameTimer);
				vimgui.updated = true;
    		viewUpdated = true;
			}
			if(io.MouseDelta.y < 0){
				camera.ProcessKeyboard(FORWARD, frameTimer);
				vimgui.updated = true;
    		viewUpdated = true;
			}
			if(io.MouseDelta.y > 0){
				camera.ProcessKeyboard(BACKWARD, frameTimer);
				vimgui.updated = true;
    		viewUpdated = true;
			}
		}	
    if(mouseState == GLFW_PRESS){
      mouseCallback(vdevice._window, xpos, ypos);
      vimgui.updated = true;
      if(!io.WantCaptureMouse){
        viewUpdated = true;
      }
      
    }else firstMouse = true;
    if(io.MouseWheel != 0){
      camera.ProcessMouseScroll(io.MouseWheel, frameTimer);
			vimgui.updated = true;
      if(!io.WantCaptureMouse){
        viewUpdated = true;
      }
    }
    processInput(vdevice._window);
		render();
		frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		frameTimer = tDiff / 1000.0f;    

    if(mouseState == GLFW_PRESS){
      mouseButtons.left = true;
    }else{
      mouseButtons.left = false;
    }
   
    if (viewUpdated)
		{
			viewUpdated = false;
			viewChanged();
		}
		if (!paused)
		{
			timer += timerSpeed * frameTimer;
			if (timer > 1.0)
			{
				timer -= 1.0f;
			}
		}
		float fpsTimer = std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count();
		if (fpsTimer > 1000.0f)
		{
			if (!settings.overlay)
			{
				std::string windowTitle = getWindowTitle();
				
			}
			lastFPS = (float)frameCounter * (1000.0f / fpsTimer);
			frameCounter = 0;
			lastTimestamp = tEnd;
		}
    
		updateOverlay();
    glfwPollEvents();
    
	}
  vkDeviceWaitIdle(vdevice._logicalDevice);
}

void VBase::viewChanged() {}

std::string VBase::getWindowTitle()
{
	std::string device(vdevice.properties.deviceName);
	std::string windowTitle;
	windowTitle = vdevice._title + " - " + device;
	if (!settings.overlay) {
		windowTitle += " - " + std::to_string(frameCounter) + " fps";
	}
	return windowTitle;
}

void VBase::processInput(GLFWwindow* window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if(ImGui::IsKeyDown(0x53) | ImGui::IsKeyDown(0x109)){
    camera.ProcessKeyboard(FORWARD, frameTimer);
		vimgui.updated = true;
    viewUpdated = true;
  }
  if(ImGui::IsKeyDown(0x57) | ImGui::IsKeyDown(0x108)){
    camera.ProcessKeyboard(BACKWARD, frameTimer);
		vimgui.updated = true;
    viewUpdated = true;
  }
  if(ImGui::IsKeyDown(0x41) | ImGui::IsKeyDown(0x107)){
    camera.ProcessKeyboard(LEFT, frameTimer);
		vimgui.updated = true;
    viewUpdated = true;
  }
  if(ImGui::IsKeyDown(0x44) | ImGui::IsKeyDown(0x106)){
    camera.ProcessKeyboard(RIGHT, frameTimer);
		vimgui.updated = true;
    viewUpdated = true;
  }
}

void VBase::mouseMoved(double x, double y, bool & handled) {}


void VBase::mouseCallback(GLFWwindow* window, double xpos, double ypos){
  if(firstMouse){
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void VBase::updateOverlay(){
}

void VBase::showVImGui(){
  
}

void VBase::drawImGui(const VkCommandBuffer commandBuffer){
  if(settings.overlay){
    const VkViewport viewport = pworx::VInfo::viewport((float)width, (float)height, 0.0f, 1.0f);
    const VkRect2D scissor = pworx::VInfo::rect2D(width, height, 0, 0);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vimgui.draw(commandBuffer);
  }
}

VkPipelineShaderStageCreateInfo VBase::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = pworx::Utils::loadShader(fileName.c_str(), vdevice._logicalDevice);

	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

void VBase::OnUpdateVImGui(pworx::VImGui *overlay) {}

void VBase::destroyCommandBuffers(){
  vkFreeCommandBuffers(vdevice._logicalDevice, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}

bool VBase::prepare(){
  //vdevice = new pworx::VDevice;
  
  if(vdevice.initDevice(width, height) != true){
    std::cout<<"Failed to init device. "<<std::endl;
    return false;
  }
  if(initVk() != true){
    std::cout<<"Failed to init vk. "<<std::endl;
    return false;
  }
  ImGui_ImplGlfw_InitForVulkan(vdevice._window, true);
  glfwSetScrollCallback(vdevice._window, ImGui_ImplGlfw_ScrollCallback);
  setupSwapChain();
  if(createCommandPool() != VK_SUCCESS){
    std::cout<<"Failed to create command pool."<<std::endl;
    return false;
  }  
  
  createCommandBuffers();
  createSynchronizationPrimitives();
  setupDepthStencil();
  setupRenderPass();
  createPipelineCache();
  setupFrameBuffer();
/*   if(settings.grid){
    grid.setGrid(&vdevice, &_graphicsQueue, &pipelineCache, &renderPass);
    
  } */
  
  settings.overlay = settings.overlay && (!benchmark.active);
  if(settings.overlay){
    std::cout<<"settings.overlay true"<<std::endl;
    vimgui.vdevice = &vdevice;
    vimgui.queue = _graphicsQueue;
    vimgui.shaders = {
      loadShader("../shaders/ui/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      loadShader("../shaders/ui/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    vimgui.prepareResources();
    vimgui.preparePipeline(pipelineCache, renderPass);

  }
  

  return true;
}

VBase::~VBase(){
  vdevice.swapCleanUp();
  
  for(auto modules : shaderModules){
    vkDestroyShaderModule(vdevice._logicalDevice, modules, vdevice._allocator);
  }
  if (descriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(vdevice._logicalDevice, descriptorPool, vdevice._allocator);
	}
  destroyCommandBuffers();
  vkDestroyRenderPass(vdevice._logicalDevice, renderPass, vdevice._allocator);
  for(auto framebuffer : frameBuffers){
    vkDestroyFramebuffer(vdevice._logicalDevice, framebuffer, vdevice._allocator);
  }

  vkDestroyImageView(vdevice._logicalDevice, depthStencil.view, vdevice._allocator);
  vkDestroyImage(vdevice._logicalDevice, depthStencil.image, vdevice._allocator);
  vkFreeMemory(vdevice._logicalDevice, depthStencil.mem, vdevice._allocator);
  vkDestroyPipelineCache(vdevice._logicalDevice, pipelineCache, vdevice._allocator);
  vkDestroyCommandPool(vdevice._logicalDevice, cmdPool, vdevice._allocator);
  vkDestroySemaphore(vdevice._logicalDevice, semaphores.renderComplete, vdevice._allocator);
  vkDestroySemaphore(vdevice._logicalDevice, semaphores.presentComplete, vdevice._allocator);
  
  for(auto fence : waitFences){
    vkDestroyFence(vdevice._logicalDevice, fence, vdevice._allocator);
  }

  vimgui.freeResources();
  
}
