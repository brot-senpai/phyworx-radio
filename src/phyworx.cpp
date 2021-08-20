#include "phyworx.hpp"

void Phyworx::buildCommandBuffers(){
  VkCommandBufferBeginInfo cmdBufInfo = pworx::VInfo::commandBufferBeginInfo();

  VkClearValue clearValues[2];
  clearValues[0].color = defaultClearColor;
  clearValues[1].depthStencil = { 1.0f, 0 };

  VkRenderPassBeginInfo renderPassBeginInfo = pworx::VInfo::renderPassBeginInfo();
  renderPassBeginInfo.renderPass = renderPass;
  renderPassBeginInfo.renderArea.offset.x = 0;
  renderPassBeginInfo.renderArea.offset.y = 0;
  renderPassBeginInfo.renderArea.extent.width = width;
  renderPassBeginInfo.renderArea.extent.height = height;
  renderPassBeginInfo.clearValueCount = 2;
  renderPassBeginInfo.pClearValues = clearValues;

  for(int32_t i = 0; i < drawCmdBuffers.size(); ++i){
    renderPassBeginInfo.framebuffer = frameBuffers[i];
    if(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo) != VK_SUCCESS){
      throw std::runtime_error("Failed to begin command buffer. ");
    }
    vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = pworx::VInfo::viewport((float)width, (float)height, 0.0f, 1.0f);
    vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

    VkRect2D scissor = pworx::VInfo::rect2D(width,	height,	0,	0);
    vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
    vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

    grid.drawGrid(drawCmdBuffers[i]);   
    if(settings.showTopo){
      vtopo.drawVTopo(drawCmdBuffers[i]);
    }    
    if(settings.draw3DPlot){
      vplotter3D.drawPlotter3D(drawCmdBuffers[i]);
    }
    
    drawImGui(drawCmdBuffers[i]);    
    
    vkCmdEndRenderPass(drawCmdBuffers[i]);
    if(vkEndCommandBuffer(drawCmdBuffers[i]));

  }
}

void Phyworx::setupDescriptorSet(){
  VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = descriptorPool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &descriptorSetLayout;

  
  if(vkAllocateDescriptorSets(vdevice._logicalDevice, &allocateInfo, &descriptorSet) != VK_SUCCESS){
    throw std::runtime_error("Failed to allocate descriptor set. ");
  }
  std::array<VkWriteDescriptorSet,1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;  
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBuffer.descriptor;


    vkUpdateDescriptorSets(vdevice._logicalDevice, 1, descriptorWrites.data(), 0, nullptr);

}


void Phyworx::setupDescriptorPool(){
  VkDescriptorPoolSize poolSizes[] = {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
  };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
  poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
  poolInfo.pPoolSizes = poolSizes;
  if(vkCreateDescriptorPool(vdevice._logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS){
    throw std::runtime_error("Failed to create descriptor pool. ");
  }
}

void Phyworx::preparePipelines(){

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = pworx::VInfo::pipelineLayoutCreateInfo(&descriptorSetLayout);
  if(vkCreatePipelineLayout(vdevice._logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
    throw std::runtime_error("Failed to create pipeline layout. ");
  }
  grid.preparePipelines(pipelineLayout);
  vtopo.prepareLinePipeline(pipelineLayout);
  vplotter3D.preparePlotter3DPipelines(pipelineLayout);

}

void Phyworx::setupDescriptorSetLayout(){
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  

  std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};//, plotter3DLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = bindings.data();

  if(vkCreateDescriptorSetLayout(vdevice._logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS){
    throw std::runtime_error("Failed to create descriptor set layout. ");
  }
}


void Phyworx::prepareUniformBuffers(){
  if(vdevice.createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffer, sizeof(uboMatrices)) != VK_SUCCESS){
      throw std::runtime_error("Failed to create uniform buffer. ");
    }

  if(uniformBuffer.map() != VK_SUCCESS){
    throw std::runtime_error("Failed to map uniform buffer. ");
  }

  updateUniformBuffers();
}

void Phyworx::updateUniformBuffers(){

  uboMatrices.projection = glm::perspective(glm::radians(camera.Zoom), (float)width/(float)height, 0.1f, 500.0f);
  uboMatrices.view = camera.GetViewMatrix();
  uboMatrices.model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
  memcpy(uniformBuffer.mapped, &uboMatrices, sizeof(uboMatrices));  
}


void Phyworx::draw(){
  VBase::prepareFrame();

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

  if(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS){
    throw std::runtime_error("Failed to submit queue. ");
  }

  VBase::submitFrame();
}

void Phyworx::initVTopo(){
  if(settings.grid){
    grid.setGrid(&vdevice, &_graphicsQueue, &pipelineCache, &renderPass);
    vtopo.initVTopo(&vdevice, &_graphicsQueue, &pipelineCache, &renderPass);
    vFunc.xmax = 5;
    grid.gridParams._xmax = vFunc.xmax;  
    vFunc.xmin = -5;
    grid.gridParams._xmin = vFunc.xmin;
    vFunc.ymax = 5;
    grid.gridParams._ymax = vFunc.ymax;
    vFunc.ymin = -5;
    grid.gridParams._ymin = vFunc.ymin;
    vFunc.zmax = 5;
    grid.gridParams._zmax = vFunc.zmax;
    vFunc.zmin = -1;
    grid.gridParams._zmin = vFunc.zmin;
    vFunc.precision = 0.1;
    grid.gridParams._precision = 0.5;
    grid.createGrid();

    vFunc.generateVertices();
    vtopo.createVertexBuffer(vFunc.vertices, vFunc.buffer);
    vtopo.vfunc = &vFunc;
    
  } 
  
}

Phyworx::~Phyworx(){
  vkDestroyPipeline(vdevice._logicalDevice, pipeline, nullptr);

  vkDestroyPipelineLayout(vdevice._logicalDevice, pipelineLayout, nullptr);
  vkDestroyDescriptorSetLayout(vdevice._logicalDevice, descriptorSetLayout, nullptr);
  
  uniformBuffer.destroy();  
}

void Phyworx::prepare(){
  VOverlay::prepare();
  
  prepareUniformBuffers();
  setupDescriptorSetLayout();
  setupDescriptorPool();
  initVTopo();
  preparePipelines();  
  setupDescriptorSet();
  buildCommandBuffers();

  prepared = true;
}