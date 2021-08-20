#pragma once
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "implot.h"
#include "vlime.hpp"


namespace pworx{
  class VPlotter{
    public:
      VLime* sdr = nullptr;
      VPlotter();
      ~VPlotter();

      struct{
        std::string deviceName;
        std::string serial;
        std::string address;
      } sdrInfo;


      struct{
        float freq = 96.1e6;
        float min;
        float max;
        float step;
      } sdrParams;
      bool oldFreq = sdrParams.freq;
      void showDemo(bool*);
      void setPlotter(VLime* device);
      void showScatterPlot();

      bool firstPlot = true;      
      bool haveDevice = false;
      void getDeviceInfo();
      void setSDRParams();
      void getSDRParams();

  };
};



