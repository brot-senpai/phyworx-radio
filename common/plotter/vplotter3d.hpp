#pragma once

#include "vdevice.hpp"
#include "vinfo.hpp"
#include "vutils.hpp"
#include "vlime.hpp"
#include "vplotter.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vvertex.hpp"


namespace pworx{
  class VPlotter3D {
    public:
    VPlotter3D();
    ~VPlotter3D();

    struct UBOVS{
      glm::mat4 proj;      
      glm::mat4 view;
      glm::mat4 model;
    } uboVS;

    VDevice*                              vdevice = nullptr;
    VkQueue*                              graphicsQueue = nullptr;
    VLime*                                sdr = nullptr; 
    VkRenderPass*                         renderPass = nullptr;
    VkPipelineCache*                      pipelineCache = nullptr;
    VkDescriptorPool                     descriptorPool;
    VkPipelineLayout                      pipelineLayout;
    std::vector<VkShaderModule>           shaderModules;
    VkPipeline                            pipeline;
    pworx::Buffer                         vertexBufferPlotter3D;
    pworx::Buffer                         stagingBuffer;
    VkDescriptorSetLayout                 descriptorSetLayout;
    VkDescriptorSet                       descriptorSet;
    VkDeviceSize                          bufferSize;
    bool                                  firstPlot = true; 

    void                                  setupPlotter3D(VDevice* device, VkQueue* graphicsQueue, VkPipelineCache* cache, VkRenderPass* renderpass, VLime* sdrdevice);
    void                                  preparePlotter3DPipelines(VkPipelineLayout);
    VkPipelineShaderStageCreateInfo       loadShader(std::string fileName, VkShaderStageFlagBits stage);
    void                                  updateUniformBuffers();
    void                                  createVertexBuffer();
  
    void                                  drawPlotter3D(VkCommandBuffer commandBuffer);
    void                                  setDescriptorSet();
    void                                  setDescriptorSetLayout();
    
  };
};