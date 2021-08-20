#include "vlime.hpp"

VLime::VLime(){}

bool VLime::setDevice(){

  if((deviceCount = LMS_GetDeviceList(deviceList)) < 0){
    LMS_Close(device);
    return false;
  }

  std::cout<<"SDR devices found: "<<deviceCount<<std::endl;
  if(deviceCount >= 1){
    std::cout<<deviceList[0]<<std::endl;
    deviceInfo.deviceName = std::string(deviceList[0], 11);
  
  }
  if(deviceCount < 1) return false;
  
  if (LMS_Open(&device, deviceList[0], NULL)){
    LMS_Close(device);
    return false;
  }

  if(LMS_Init(device) != 0){
    LMS_Close(device);
    return false;
  }

  return true;
}


bool VLime::initDevice(){
  if(LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0){
    LMS_Close(device);
    return false;
  }

  if(setRXFreq(rxParams.freq)){
    oldFreq = rxParams.freq;
  }
  else{
    return false;
  }  

  if(!getParams()){
    return false;
  }
  return true;
}


bool VLime::getParams(){
  if(!getRXRange()){
    std::cout<<"Failed to get RX range."<<std::endl;
    return false;
  }
  if(!getAntenna()){
    std::cout<<"Failed to get antenna info. "<<std::endl;
    return false;
  }
  if(!getBandWidth()){
    std::cout<<"Failed to get bandwidth info. "<<std::endl;
    return false;
  }
  return true;
}

bool VLime::getBandWidth(){
  lms_range_t range;
  if(LMS_GetLPFBWRange(device, LMS_CH_RX, &range) !=0){
    return false;
  }

  float_type bw;
  if(LMS_GetLPFBW(device, LMS_CH_RX, 0, &bw) !=0){
    std::cout<<"Failed to get bandwidth. "<<std::endl;
    return false;
  }
  rxParams.bandWidth = bw;
  rxParams.bandWidthMin = range.min;
  rxParams.bandWidthMax = range.max;
  
  return true;
}

bool VLime::getAntenna(){
  int n;
  lms_name_t antenna_list[10];

  if((n = LMS_GetAntennaList(device, LMS_CH_RX, 0, antenna_list)) < 0){
    return false;
  }

  for(int i = 0; i < n; i++){
    rxParams.antennaList.push_back(antenna_list[i]);
  }
  if((n = LMS_GetAntenna(device, LMS_CH_RX, 0))< 0){
    return false;
  }
  rxParams.currentAntenna = antenna_list[n];

  return true;
}

bool VLime::getRXRange(){
  
  if(LMS_GetLOFrequencyRange(device, LMS_CH_RX, &rxRange) == -1){
    return false;
  }
  
  rxParams.rxMin = rxRange.min;
  rxParams.rxMax = rxRange.max;
  rxParams.rxStep = rxRange.step;
  return true;
}

float VLime::getRXFreq(){
  return rxParams.freq;
}

bool VLime::setBandWidth(float bw){
  if(LMS_SetLPFBW(device, LMS_CH_RX, 0, bw) != 0){
    return false;
  }
  rxParams.bandWidth = bw;
  return true;
}

bool VLime::setRXFreq(float frequency){
  if(LMS_SetLOFrequency(device, LMS_CH_RX, 0, frequency) != 0){ //
    LMS_Close(device);
    return false;
  }
  this->rxParams.freq = frequency;
  return true;
}

void VLime::setupStream(){
  streamId.channel = 0; //channel number
  streamId.fifoSize = 1024 * 1024; //fifo size in samples
  streamId.throughputVsLatency = 1.0; //optimize for max throughput
  streamId.isTx = false; //RX channel
  streamId.dataFmt = lms_stream_t::LMS_FMT_I12; //12-bit integers
  if (LMS_SetupStream(device, &streamId) != 0)
      error();
  LMS_StartStream(&streamId);
}

void VLime::getBuffer(){
  char freq[32];
  sprintf(freq, "%.4e", rxParams.freq);  
  streamBuffer.title = "Freq: ";
  streamBuffer.title.append(freq);

  auto t1 = std::chrono::high_resolution_clock::now();
  if(oldFreq != rxParams.freq){
    streamBuffer.title.clear();
    sprintf(freq, "%.4e", rxParams.freq);  
    streamBuffer.title = "Freq: ";
    streamBuffer.title.append(freq);
    setRXFreq(rxParams.freq);
    oldFreq = rxParams.freq;
  }
  
  samplesRead = LMS_RecvStream(&streamId, buffer, sampleCnt, NULL, 100);
  streamBuffer.vertices3D.clear();
  streamBuffer.vertices2D.clear();

  for(int j = 0; j < sizeof(buffer); ++j){
    streamBuffer.vertices3D.push_back({{(float)buffer[2*j]/5, (float)buffer[2*j+1]/5, 0.0}, scatterColor});
    streamBuffer.vertices2D.push_back({(float)buffer[2*j], (float)buffer[2*j+1]});
  }
}

void VLime::endStream(){
  LMS_StopStream(&streamId); 
  LMS_DestroyStream(device, &streamId); 
}

int VLime::error(){
  if(device != nullptr);
  exit(-1);
}

VLime::~VLime(){
  if(deviceCount >= 1){
    LMS_Close(device);
  }  
}

