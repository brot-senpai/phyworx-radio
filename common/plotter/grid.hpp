#pragma once 

#include "vdevice.hpp"
#include "vinfo.hpp"
#include "vutils.hpp"
#include "vvertex.hpp"
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace pworx{
  class Grid{
    public:
      Grid();
      ~Grid();

    VDevice*                                      vdevice;
    VkQueue*                                      queue;
    VkRenderPass*                                 renderPass;

    pworx::Buffer                                 xyVertexBuffer;
    pworx::Buffer                                 xzVertexBuffer;
    pworx::Buffer                                 yzVertexBuffer;
    VkPipeline                                    pipeline;
    VkPipelineCache*                              pipelineCache = nullptr;

    glm::vec3 whiteColor = glm::vec3(1.0f, 1.0f, 1.0f);
    struct GridParams{
      float _xmin;
      float _xmax;
      float _ymin;
      float _ymax;
      float _zmin;
      float _zmax;
      float _precision;
      float _xyAlpha = 0.5;
      float _xzAlpha = 0.5;
      float _yzAlpha = 0.5;
      glm::vec3 _xyColor = glm::vec3(1.0f, 0.0f, 0.0f);
      glm::vec3 _xzColor = glm::vec3(0.0f, 1.0f, 0.0f);
      glm::vec3 _yzColor = glm::vec3(0.0f, 0.0f, 1.0f);
      bool  _showXY = true;
      bool  _showXZ = true;
      bool  _showYZ = true;
      bool  _whiteGrid = false;
      float deltaX() {return _xmax-_xmin;}
      float deltaY() {return _ymax-_ymin;}
      float deltaZ() {return _zmax-_zmin;}
    } gridParams;

    
    uint32_t                                      steps;
    std::vector<VVertex2>                           xyVertices;
    std::vector<VVertex2>                           xzVertices;
    std::vector<VVertex2>                           yzVertices;
   
    std::vector<VkShaderModule>                   shaderModules;
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    void                                          setParams(float, float, float, float, float, float, float);
    void                                          setXY();
    void                                          setXZ();
    void                                          setYZ();

    VkPipelineShaderStageCreateInfo               loadShader(std::string fileName, VkShaderStageFlagBits stage);
    void                                          setGrid(VDevice* device, VkQueue* graphicsQueue, VkPipelineCache* cache, VkRenderPass* renderpass);
    void                                          createVertexBuffer();
    void                                          createVertexBuffer(std::vector<VVertex2>&, pworx::Buffer&, bool);
    void                                          createGrid();
    void                                          createXY(bool);
    void                                          createXZ(bool);
    void                                          createYZ(bool);
    void                                          recreateXY();
    void                                          recreateXZ();
    void                                          recreateYZ();
    void                                          preparePipelines(VkPipelineLayout layout);
    void                                          drawGrid(VkCommandBuffer);  

  };
}

