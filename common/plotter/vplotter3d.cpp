#include "vplotter3d.hpp"

pworx::VPlotter3D::VPlotter3D(){}

void pworx::VPlotter3D::setupPlotter3D(VDevice* device, VkQueue* graphicsQueue, VkPipelineCache* cache, VkRenderPass* renderpass, VLime* sdrdevice){

  this->vdevice = device;
  this->graphicsQueue = graphicsQueue;
  this->renderPass = renderpass;
  this->pipelineCache = cache;
  this->sdr = sdrdevice;

}

void pworx::VPlotter3D::preparePlotter3DPipelines(VkPipelineLayout layout){
  
  auto bindingDescription = pworx::VVertex::getBindingDescription();
  auto attributeDescription = pworx::VVertex::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = pworx::VInfo::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, VK_FALSE);  
  VkPipelineRasterizationStateCreateInfo rasterizationStateCI = pworx::VInfo::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, 0); 
  VkPipelineColorBlendAttachmentState blendAttachmentState = pworx::VInfo::pipelineColorBlendAttachmentState(0xf, VK_TRUE);
  VkPipelineColorBlendStateCreateInfo colorBlendState = pworx::VInfo::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
  VkPipelineDepthStencilStateCreateInfo depthStencilState = pworx::VInfo::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
  VkPipelineViewportStateCreateInfo viewportState = pworx::VInfo::pipelineViewportStateCreateInfo(1, 1, 0);
  VkPipelineMultisampleStateCreateInfo multisampleState = pworx::VInfo::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
  std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState = pworx::VInfo::pipelineDynamicStateCreateInfo(dynamicStateEnables);
  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
  shaderStages[0] = loadShader("../shaders/plotter/plot3d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
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
  
  if(vkCreateGraphicsPipelines(*vdevice, *pipelineCache, 1, &pipelineCI, nullptr, &pipeline) != VK_SUCCESS){
    throw std::runtime_error("Failed to create graphics pipeline. ");
  }

}


void pworx::VPlotter3D::createVertexBuffer(){
  sdr->getBuffer();
  bufferSize = sizeof(sdr->streamBuffer.vertices3D[0]) * sdr->streamBuffer.vertices3D.size();  

  if(firstPlot){
    vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, &stagingBuffer.buffer, &stagingBuffer.memory );
  }

  void* data;
  vkMapMemory(vdevice->_logicalDevice, stagingBuffer.memory, 0, bufferSize, 0, &data);
  memcpy(data, sdr->streamBuffer.vertices3D.data(), (size_t) bufferSize);
  vkUnmapMemory(vdevice->_logicalDevice, stagingBuffer.memory);

  if(firstPlot){
    vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize, &vertexBufferPlotter3D.buffer, &vertexBufferPlotter3D.memory);
  }

  vdevice->copyBuffer(&stagingBuffer.buffer, &vertexBufferPlotter3D.buffer, bufferSize, *graphicsQueue);

  if(firstPlot){
    firstPlot = false;
  }
}



void pworx::VPlotter3D::drawPlotter3D(VkCommandBuffer commandBuffer){
  createVertexBuffer();

  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferPlotter3D.buffer, offsets);
  
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdDraw(commandBuffer, static_cast<uint32_t>(sdr->streamBuffer.vertices3D.size()), 1, 0, 0);
  
}

void pworx::VPlotter3D::updateUniformBuffers(){


}

VkPipelineShaderStageCreateInfo pworx::VPlotter3D::loadShader(std::string fileName, VkShaderStageFlagBits stage){
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
  
	shaderStage.module = pworx::Utils::loadShader(fileName.c_str(), vdevice->_logicalDevice);

	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

pworx::VPlotter3D::~VPlotter3D(){
  if(shaderModules.size() >= 1){
    for(auto modules : shaderModules){
      vkDestroyShaderModule(vdevice->_logicalDevice, modules, vdevice->_allocator);
    }
  }
  vkFreeMemory(vdevice->_logicalDevice, vertexBufferPlotter3D.memory, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, vertexBufferPlotter3D.buffer, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, stagingBuffer.buffer, nullptr);
  vkFreeMemory(vdevice->_logicalDevice, stagingBuffer.memory, nullptr);
  vkDestroyPipeline(vdevice->_logicalDevice, pipeline, vdevice->_allocator);
 

}

