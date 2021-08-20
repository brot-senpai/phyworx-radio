#include "vlime.hpp"
#include "voverlay.hpp"
#include "vtopo.hpp"
#include "vfunc.hpp"

class Phyworx : public VOverlay{
  public: 
    pworx::Buffer                         uniformBuffer;
    pworx::VTopo                          vtopo;
    VFunc                          vFunc;


    struct UBOMatrices{
      glm::mat4 projection;
      glm::mat4 model;
      glm::mat4 view;
    } uboMatrices; 



    VkPipeline                              pipeline;
    VkPipelineLayout                        pipelineLayout;
    VkDescriptorSet                         descriptorSet;
    VkDescriptorSetLayout                   descriptorSetLayout;

    Phyworx() : VOverlay(){
      vdevice._title = "Phyworx Radio";
      settings.overlay = true;
      settings.grid = true;
      settings.implot = true;
    }

    void                                    prepare();
    void                                    prepareUniformBuffers();
    void                                    updateUniformBuffers();
    void                                    setupDescriptorSetLayout();
    void                                    preparePipelines();
    void                                    setupDescriptorPool();
    void                                    setupDescriptorSet();
    void                                    buildCommandBuffers();
    void                                    initVTopo();
    
    void                                    draw();   


    virtual void render(){
      if(!prepared) return;
      draw();

      if(viewUpdated | settings.draw3DPlot){        
        updateUniformBuffers();
      }
    }

    virtual void getEnabledFeatures(){
      if(vdevice.features.fillModeNonSolid){
      enabledFeatures.fillModeNonSolid = VK_TRUE;
      }
      if(vdevice.features.largePoints){
        enabledFeatures.largePoints = VK_TRUE;
      }
    }

    virtual void OnUpdateVImGui(pworx::VImGui *overlay){      
    }

    ~Phyworx();
};