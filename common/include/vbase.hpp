#pragma once

#include "vdevice.hpp"
#include "vutils.hpp"
#include "vbenchmark.hpp"
#include "vimgui.hpp"
#include "imgui_impl_glfw.h"

#include "camera.hpp"
#include "imgui.h"
#include "vbuffer.hpp"
#include "vgridui.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class VBase {
  public:

    struct Settings {
      bool grid = false;
      bool fullscreen = false;
      bool vsync = false;
      bool overlay = false;
      bool implot = false;
      bool draw3DPlot = false;
      bool first3DPlot = true;
      bool showTopo = false;
    } settings;

    struct {
      VkImage image;
      VkDeviceMemory mem;
      VkImageView view;
    } depthStencil;

    struct {
      bool left = false;
      bool right = false;
      bool middle = false;
    } mouseButtons;

    VBase();
    pworx::VDevice                              vdevice;
    
    glm::vec2                                   mousePos;

    float lastX = (float)width/ 2.0f;
    float lastY = (float)height / 2.0f;
    bool firstMouse = true;
    
    pworx::VImGui                               vimgui;
    pworx::Benchmark                            benchmark;
    bool                                        prepare();
    bool                                        prepared = false;
	  bool                                        resized = false;
	  uint32_t                                    width = 1280;
	  uint32_t                                    height = 720;
    float                                       frameTimer = 1.0f;
    float                                       timer = 0.0f;
	  float                                       timerSpeed = 0.25f;
	  bool                                        paused = false;
    float                                       tDiff;
    VkClearColorValue                           defaultClearColor = { { 0.09f, 0.09f, 0.09f, 1.0f } };
    virtual void                                setupDepthStencil();
    virtual void                                setupRenderPass();
    virtual void                                setupFrameBuffer();
    virtual void                                buildCommandBuffers();
    virtual void                                windowResized();
    virtual void                                viewChanged();
    virtual void                                getEnabledFeatures();
    virtual void                                OnUpdateVImGui(pworx::VImGui *overlay);
    virtual void                                mouseMoved(double x, double y, bool &handled);
    virtual void                                updateOverlay();
    VkPipelineShaderStageCreateInfo             loadShader(std::string fileName, VkShaderStageFlagBits stage);
    void                                        renderLoop();
    virtual void                                render() = 0;
    void                                        prepareFrame();    
    void                                        drawImGui(const VkCommandBuffer commandBuffer);
    void                                        submitFrame();
    void                                        createDepthResources();

    bool                                        viewUpdated;
    
    Camera                                      camera;
    VGridUI                                     grid;
    
    
    ~VBase();

  private:
    
    uint32_t                                    destWidth;
	  uint32_t                                    destHeight;
    
    bool                                        initVk();
    VkResult                                    createCommandPool();         
    void                                        setupSwapChain();
    void                                        createCommandBuffers();
    void                                        createSynchronizationPrimitives();
    void                                        createPipelineCache();
    std::string                                 getWindowTitle();    
    void                                        destroyCommandBuffers();
    void                                        windowResize();
    void                                        mouseCallback(GLFWwindow* window, double xpos, double ypos);
    void                                        mouseScroll();
    void                                        showVImGui();
    
  protected:
  
    struct {
      VkSemaphore presentComplete;
      VkSemaphore renderComplete;
    } semaphores;

    
    VkQueue                                     copyQueue;
    uint32_t                                    currentBuffer = 0;
    std::vector<VkFence>                        waitFences;
    uint32_t                                    frameCounter = 0;
	  uint32_t                                    lastFPS = 0;
    VkQueue                                     _graphicsQueue;
    VkFormat                                    depthFormat;
    VkSubmitInfo                                submitInfo;
    VkPipelineStageFlags                        submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkCommandPool                               cmdPool;
    std::vector<VkCommandBuffer>                drawCmdBuffers;
    VkRenderPass                                renderPass;
    VkPipelineCache                             pipelineCache;
    std::vector<VkFramebuffer>                  frameBuffers;
    std::vector<VkShaderModule>                 shaderModules;
    VkDescriptorPool                            descriptorPool = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures                    enabledFeatures{};
    std::vector<const char*>                    enabledDeviceExtensions;
    void*                                       deviceCreatepNextChain = nullptr;
    
    ImGuiIO&                                    io = ImGui::GetIO();
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
    

    void                                        processInput(GLFWwindow* window);
};
