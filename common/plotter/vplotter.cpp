#include "vplotter.hpp"

namespace pworx{
  VPlotter::VPlotter(){
    
  }

  void VPlotter::showDemo(bool* show){
    //ImPlot::ShowDemoWindow(show);
  }

  void VPlotter::setPlotter(VLime* device){
    this->sdr = device;

    sdr->setDevice();
    if(sdr->deviceCount >= 1){
      haveDevice = true;
      getDeviceInfo();
      setSDRParams();
      if(!sdr->initDevice()){
        std::cout<<"Failed to init sdr-> "<<std::endl;
      }
      sdr->initDevice();
      sdr->setupStream();
    }
    
    ImPlot::CreateContext();
  }

  void VPlotter::showScatterPlot(){

    if(oldFreq != sdrParams.freq){
      setSDRParams();
      oldFreq = sdrParams.freq;
    }
    sdr->getBuffer();
    
    if (ImPlot::BeginPlot(sdr->streamBuffer.title.c_str(), "I", "Q")) {
      ImPlot::PlotScatter("", sdr->streamBuffer.vertices2D.data()[0].data(), sdr->streamBuffer.vertices2D.data()[1].data(), sdr->streamBuffer.numSamples);    
      ImPlot::EndPlot();
    }
    
  }

  void VPlotter::getDeviceInfo(){
    sdrInfo.deviceName = sdr->deviceInfo.deviceName;
  }

  void VPlotter::setSDRParams(){
    sdr->rxParams.freq = sdrParams.freq;
  }

  void VPlotter::getSDRParams(){
    sdrParams.min = sdr->rxRange.min;
    sdrParams.max = sdr->rxRange.max;
    sdrParams.step = sdr->rxRange.step;
  }

  VPlotter::~VPlotter(){
/*     if(haveDevice){
      sdr->endStream();
    }    */ 
    ImPlot::DestroyContext();
  }
};