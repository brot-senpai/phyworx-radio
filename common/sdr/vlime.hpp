#pragma once

#include "lime/LimeSuite.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string.h>
#include <glm/glm.hpp>
#include <vvertex.hpp>



class VLime{
  public:
    VLime();
    ~VLime();

    struct{
      float freq;
      float rxMin;
      float rxMax;
      float rxStep;
      float bandWidthMin;
      float bandWidthMax;
      float bandWidth;
      float normalizedGain;
      float sampleRate;
      std::vector<std::string> antennaList;
      std::string currentAntenna;
    }rxParams;

    float                             oldFreq;

    struct{
      std::string deviceName;
      std::string serial;
    }deviceInfo;

    

    lms_device_t*                     device = nullptr;
    
    bool                              setDevice();
    
    static const int                  sampleCnt = 1000; 
    int                               samplesRead = 0;
    int16_t                           buffer[sampleCnt * 2];
    uint16_t                          deviceCount;
    float                             tempRXFreq;
    float                             tempBW;

    struct{
      std::string title;
      std::vector<pworx::VVertex> vertices3D;
      std::vector<std::array<float, 2>> vertices2D;
      uint16_t numSamples = sampleCnt;
    } streamBuffer; 

    std::array<float, 4>              scatterColor = {1.0f, 0.0f, 0.0f, 1.0f};
    lms_info_str_t                    deviceList[8];
    lms_range_t                       rxRange;
    void                              getBuffer();
    void                              setupStream();
    void                              endStream();
    bool                              initDevice();

    bool                              getRXRange();    
    bool                              getParams();
    bool                              getAntenna();
    bool                              getBandWidth();
    bool                              getGain();
    float                             getRXFreq();

    bool                              setRXFreq(float);
    bool                              setBandWidth(float);

  private:
    lms_stream_t streamId;
    int error();
  
};
