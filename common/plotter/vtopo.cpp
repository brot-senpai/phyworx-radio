#include "vtopo.hpp"

pworx::VTopo::VTopo(){}

void pworx::VTopo::initVTopo(VDevice* device, VkQueue* graphicsQueue, VkPipelineCache* cache, VkRenderPass* renderpass){
  this->vdevice = device;
  this->queue = graphicsQueue;
  this->pipelineCache = cache;
  this->renderPass = renderpass;

}

void pworx::VTopo::createVertexBuffer(std::vector<VVertex2>& vert, pworx::Buffer& buffer){
  VkDeviceSize bufferSize = sizeof(vert[0]) * (vert.size()); 

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, &stagingBuffer, &stagingBufferMemory );

  void* data;
  vkMapMemory(vdevice->_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vert.data(), (size_t) bufferSize);
  vkUnmapMemory(vdevice->_logicalDevice, stagingBufferMemory);

  vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize, &buffer.buffer, &buffer.memory);
  
  vdevice->copyBuffer(&stagingBuffer, &buffer.buffer, bufferSize, *queue);

  vkDestroyBuffer(vdevice->_logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(vdevice->_logicalDevice, stagingBufferMemory, nullptr);

  //if(enableValidationLayers) std::cout<<"Vertex Buffer created. "<<std::endl; 
}

void pworx::VTopo::prepareLinePipeline(VkPipelineLayout layout){
  auto bindingDescription = VVertex2::getBindingDescription();
  auto attributeDescription = VVertex2::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = pworx::VInfo::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, VK_FALSE);  
  VkPipelineRasterizationStateCreateInfo rasterizationStateCI = pworx::VInfo::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_LINE, VK_CULL_MODE_FRONT_AND_BACK, VK_FRONT_FACE_CLOCKWISE, 0); 
  VkPipelineColorBlendAttachmentState blendAttachmentState = pworx::VInfo::pipelineColorBlendAttachmentState(0xf, VK_TRUE);
  VkPipelineColorBlendStateCreateInfo colorBlendState = pworx::VInfo::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
  VkPipelineDepthStencilStateCreateInfo depthStencilState = pworx::VInfo::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
  VkPipelineViewportStateCreateInfo viewportState = pworx::VInfo::pipelineViewportStateCreateInfo(1, 1, 0);
  VkPipelineMultisampleStateCreateInfo multisampleState = pworx::VInfo::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
  std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState = pworx::VInfo::pipelineDynamicStateCreateInfo(dynamicStateEnables);
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
  shaderStages[0] = loadShader("../shaders/plotter/grid3d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  shaderStages[1] = loadShader("../shaders/plotter/grid3d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  VkGraphicsPipelineCreateInfo pipelineCI = pworx::VInfo::pipelineCreateInfo(layout, *renderPass, 0);
  pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
  pipelineCI.pRasterizationState = &rasterizationStateCI;
  pipelineCI.pColorBlendState = &colorBlendState;
  pipelineCI.pMultisampleState = &multisampleState;
  pipelineCI.pViewportState = &viewportState;
  pipelineCI.pDepthStencilState = &depthStencilState;
  pipelineCI.pDynamicState = &dynamicState;

  pipelineCI.pVertexInputState = &vertexInputInfo;
  pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
  pipelineCI.pStages = shaderStages.data();


  if(vkCreateGraphicsPipelines(*vdevice, *pipelineCache, 1, &pipelineCI, nullptr, &linePipeline) != VK_SUCCESS){
    throw std::runtime_error("Failed to create graphics pipeline. ");
  }  
}

void pworx::VTopo::drawVTopo(VkCommandBuffer commandBuffer){
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vfunc->buffer.buffer, offsets);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipeline);
  int idx=0;
  for(int i = 0; i<vfunc->tCount; i++){
    vkCmdDraw(commandBuffer, vfunc->xCount, 1, idx, 0);
    idx += vfunc->xCount;
  }
  
}

/* void pworx::VTopo::generateVertices(){
  glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  int t_counter = 0;
  
  for(float t = 0; t < 5; t+=0.1){
    t_counter += 1;
    int x_counter = 0;
    for(float x = -5; x <5; x+=0.1){
      x_counter += 1; 
      float u = 1 - pow(tanh(x-t), 2);
      lineVertices.push_back({glm::vec3(x, t, u), color});
    }
  }
} */

VkPipelineShaderStageCreateInfo pworx::VTopo::loadShader(std::string fileName, VkShaderStageFlagBits stage){
  VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = pworx::Utils::loadShader(fileName.c_str(), vdevice->_logicalDevice);

	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	shaderModules.push_back(shaderStage.module);
	return shaderStage;

}

pworx::VTopo::~VTopo(){
  for(auto modules : shaderModules){
    vkDestroyShaderModule(vdevice->_logicalDevice, modules, vdevice->_allocator);
  }

  vkFreeMemory(vdevice->_logicalDevice, vfunc->buffer.memory, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, vfunc->buffer.buffer, vdevice->_allocator);
  vkDestroyPipeline(vdevice->_logicalDevice, linePipeline, vdevice->_allocator);
}