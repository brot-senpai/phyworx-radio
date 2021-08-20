#pragma once

#include "vdevice.hpp"
#include "vvertex.hpp"
#include "vinfo.hpp"
#include "vutils.hpp"
#include "vfunc.hpp"
#include <math.h>

namespace pworx{
  class VTopo{
    public:
      VTopo();
      ~VTopo();
      void                                          initVTopo(VDevice*, VkQueue*, VkPipelineCache*, VkRenderPass*);
      void                                          prepareLinePipeline(VkPipelineLayout);
      void                                          drawVTopo(VkCommandBuffer);
      void                                          createVertexBuffer(std::vector<VVertex2>& vert, pworx::Buffer& buffer);
      VFunc*                                        vfunc = nullptr;
    protected:
      VDevice*                                      vdevice = nullptr;
      VkQueue*                                      queue = nullptr;
      VkRenderPass*                                 renderPass = nullptr;
      VkPipelineCache*                              pipelineCache = nullptr;
      VkPipeline                                    linePipeline;
      // pworx::Buffer                                 lineVertexBuffer;
      // std::vector<VVertex2>                         lineVertices;

      std::vector<VkShaderModule>                   shaderModules;
      std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

      
      
      void                                          generateVertices();      
    
      VkPipelineShaderStageCreateInfo               loadShader(std::string fileName, VkShaderStageFlagBits stage);
  };
};