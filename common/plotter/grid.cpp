#include "grid.hpp"

pworx::Grid::Grid(){}


void pworx::Grid::setGrid(VDevice* device, VkQueue* graphicsQueue, VkPipelineCache* cache, VkRenderPass* renderpass){
  this->vdevice = device;
  this->queue = graphicsQueue;
  this->pipelineCache = cache;
  this->renderPass = renderpass; 
  

}

void pworx::Grid::createGrid(){
  createXY(true);
  createXZ(true);
  createYZ(true);
}


void pworx::Grid::createVertexBuffer(std::vector<VVertex2>& vert, pworx::Buffer& buffer, bool first){
  VkDeviceSize bufferSize = sizeof(vert[0]) * (vert.size()); 

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, &stagingBuffer, &stagingBufferMemory );

  void* data;
  vkMapMemory(vdevice->_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vert.data(), (size_t) bufferSize);
  vkUnmapMemory(vdevice->_logicalDevice, stagingBufferMemory);

  if(first){
    vdevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize, &buffer.buffer, &buffer.memory);
  }
  

  vdevice->copyBuffer(&stagingBuffer, &buffer.buffer, bufferSize, *queue);

  vkDestroyBuffer(vdevice->_logicalDevice, stagingBuffer, nullptr);
  vkFreeMemory(vdevice->_logicalDevice, stagingBufferMemory, nullptr);

  //if(enableValidationLayers) std::cout<<"Vertex Buffer created. "<<std::endl; 
}

void pworx::Grid::preparePipelines(VkPipelineLayout layout){

  auto bindingDescription = VVertex2::getBindingDescription();
  auto attributeDescription = VVertex2::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = pworx::VInfo::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, 0, VK_FALSE);  
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


  if(vkCreateGraphicsPipelines(*vdevice, *pipelineCache, 1, &pipelineCI, nullptr, &pipeline) != VK_SUCCESS){
    throw std::runtime_error("Failed to create graphics pipeline. ");
  }  
}

void pworx::Grid::createXY(bool first){
  setXY();
  createVertexBuffer(xyVertices, xyVertexBuffer, first);
}

void pworx::Grid::createXZ(bool first){
  setXZ();
  createVertexBuffer(xzVertices, xzVertexBuffer, first);
}

void pworx::Grid::createYZ(bool first){
  setYZ();
  createVertexBuffer(yzVertices, yzVertexBuffer, first);
}

void pworx::Grid::recreateXY(){
  setXY();
  createVertexBuffer(xyVertices, xyVertexBuffer, false);
}

void pworx::Grid::recreateXZ(){
  setXZ();
  createVertexBuffer(xzVertices, xzVertexBuffer, false);
}

void pworx::Grid::recreateYZ(){
  setYZ();
  createVertexBuffer(yzVertices, yzVertexBuffer, false);
}



void pworx::Grid::drawGrid(VkCommandBuffer commandBuffer){

  VkDeviceSize offsets[] = {0};
  if(gridParams._showXY){
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &xyVertexBuffer.buffer, offsets);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(xyVertices.size()), 1, 0, 0);
  }
  if(gridParams._showXZ){
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &xzVertexBuffer.buffer, offsets);  
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);  
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(xzVertices.size()), 1, 0, 0);
  }
  if(gridParams._showYZ){
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &yzVertexBuffer.buffer, offsets);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);  
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(yzVertices.size()), 1, 0, 0);
  }  
}


VkPipelineShaderStageCreateInfo pworx::Grid::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = pworx::Utils::loadShader(fileName.c_str(), vdevice->_logicalDevice);

	shaderStage.pName = "main";
	assert(shaderStage.module != VK_NULL_HANDLE);
	shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

void pworx::Grid::setParams(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax, float prec){
  gridParams._xmin = xmin;
  gridParams._xmax = xmax;
  gridParams._ymin = ymin;
  gridParams._ymax = ymax;
  gridParams._zmin = zmin;
  gridParams._zmax = zmax;
  gridParams._precision = prec;

  steps = (uint32_t)(1/gridParams._precision);  
}


void pworx::Grid::setXY(){
  xyVertices.clear();
  const int xmin_int = (int)(gridParams._xmin * steps);
  const int xmax_int = (int)(gridParams._xmax * steps);
  const int ymin_int = (int)(gridParams._ymin * steps);
  const int ymax_int = (int)(gridParams._ymax * steps);
  const float z = 0.0;

  glm::vec4 color;
  if(!gridParams._whiteGrid){
    color = glm::vec4(gridParams._xyColor, gridParams._xyAlpha);
  }
  else{
    color = glm::vec4(whiteColor, gridParams._xyAlpha);
  }
  

  for(int i = xmin_int; i <= xmax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 xy_start(glm::vec3(i_float, gridParams._ymin, z));
    glm::vec3 xy_end(glm::vec3(i_float, gridParams._ymax, z));
    xyVertices.push_back({xy_start,color});
    xyVertices.push_back({xy_end,color});
  }
  for(int i = ymin_int; i <= ymax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 xy_start(glm::vec3(gridParams._xmin, i_float, z));
    glm::vec3 xy_end(glm::vec3(gridParams._xmax, i_float, z));
    
    xyVertices.push_back({xy_start, color});
    xyVertices.push_back({xy_end, color});
  }
}

void pworx::Grid::setXZ(){
  xzVertices.clear();
  const int xmin_int = (int)(gridParams._xmin * steps);
  const int xmax_int = (int)(gridParams._xmax * steps);
  const int zmin_int = (int)(gridParams._zmin * steps);
  const int zmax_int = (int)(gridParams._zmax * steps);
  const float y = gridParams._ymin;

  glm::vec4 color;
  if(!gridParams._whiteGrid){
    color = glm::vec4(gridParams._xzColor, gridParams._xyAlpha);
  }
  else{
    color = glm::vec4(whiteColor, gridParams._xzAlpha);
  }

  for(int i = xmin_int; i <= xmax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 xz_start(glm::vec3(i_float, y, gridParams._zmin));
    glm::vec3 xz_end(glm::vec3(i_float, y, gridParams._zmax));
    xzVertices.push_back({xz_start, color});
    xzVertices.push_back({xz_end, color});
  }
  for(int i = zmin_int; i <= zmax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 xz_start(glm::vec3(gridParams._xmin, y, i_float));
    glm::vec3 xz_end(glm::vec3(gridParams._xmax, y, i_float));
    xzVertices.push_back({xz_start, color});
    xzVertices.push_back({xz_end, color});
  }
}

void pworx::Grid::setYZ(){
  yzVertices.clear();
  const int ymin_int = (int)(gridParams._ymin * steps);
  const int ymax_int = (int)(gridParams._ymax * steps);
  const int zmin_int = (int)(gridParams._zmin * steps);
  const int zmax_int = (int)(gridParams._zmax * steps);
  const float x = 0.0;

  glm::vec4 color;
  if(!gridParams._whiteGrid){
    color = glm::vec4(gridParams._yzColor, gridParams._yzAlpha);
  }
  else{
    color = glm::vec4(whiteColor, gridParams._yzAlpha);
  }

  for(int i = ymin_int; i <= ymax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 yz_start(glm::vec3(x, i_float, gridParams._zmin));
    glm::vec3 yz_end(glm::vec3(x, i_float, gridParams._zmax));
    yzVertices.push_back({yz_start, color});
    yzVertices.push_back({yz_end, color});
  }
  for(int i = zmin_int; i <= zmax_int; ++i){
    float i_float = (float)i/steps;
    glm::vec3 yz_start(glm::vec3(x, gridParams._ymin, i_float));
    glm::vec3 yz_end(glm::vec3(x, gridParams._ymax, i_float));
    yzVertices.push_back({yz_start, color});
    yzVertices.push_back({yz_end, color});
  }
} 


pworx::Grid::~Grid(){
  for(auto modules : shaderModules){
    vkDestroyShaderModule(vdevice->_logicalDevice, modules, vdevice->_allocator);
  }

  vkFreeMemory(vdevice->_logicalDevice, xyVertexBuffer.memory, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, xyVertexBuffer.buffer, vdevice->_allocator);
  vkFreeMemory(vdevice->_logicalDevice, xzVertexBuffer.memory, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, xzVertexBuffer.buffer, vdevice->_allocator);
  vkFreeMemory(vdevice->_logicalDevice, yzVertexBuffer.memory, vdevice->_allocator);
  vkDestroyBuffer(vdevice->_logicalDevice, yzVertexBuffer.buffer, vdevice->_allocator);
  vkDestroyPipeline(vdevice->_logicalDevice, pipeline, vdevice->_allocator);

}