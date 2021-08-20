#pragma once

#include "grid.hpp"
#include "imgui.h"

class VGridUI : public pworx::Grid{
  public:
    VGridUI();
    ~VGridUI();

    
    void                              showGraphSettings();
    void                              initVTopo();    
    
};