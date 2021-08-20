#include "voverlay.hpp"

VOverlay::VOverlay(){}

void VOverlay::updateOverlay(){
  if (!settings.overlay)
		return;

	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mousePos.x, mousePos.y);
	io.MouseDown[0] = mouseButtons.left;
	io.MouseDown[1] = mouseButtons.right;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(overlayWidth, height), ImGuiCond_FirstUseEver);
	ImGui::Begin("Control", nullptr, ImGuiWindowFlags_NoMove);
	
	ImGui::TextUnformatted(vdevice.properties.deviceName);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);
 
  ImGui::PushItemWidth(110.0f * vimgui.scale);
  
  if(ImGui::BeginTabBar("")){
    if(ImGui::BeginTabItem("SDR")){
      sdrTab();
      ImGui::EndTabItem();
    }      

    if(ImGui::BeginTabItem("Graphs")){
      graphTab();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }    
    
  	
  OnUpdateVImGui(&vimgui);  
  
  ImGui::PopItemWidth();
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::Render();

  if(vimgui.update() || vimgui.updated){
    buildCommandBuffers();
    vimgui.updated = false;
  }
}


void VOverlay::connectSDR(){
  if(SDRconnected){
    if(ImGui::CollapsingHeader("Settings")){
      showSDRSettings();
    }
    if(ImGui::CollapsingHeader("Plots")){
      showIQScatterPlot();
    }
  }  
}

void VOverlay::showSDRSettings(){
  ImGui::Text("Current Antenna: %s", sdr.sdrDevice.rxParams.currentAntenna.c_str());
  ImGui::Text("Current RX Freq: %.2e", (double)sdr.sdrDevice.rxParams.freq);
  ImGui::Text("RX Min: %.2e", (double)sdr.sdrDevice.rxParams.rxMin);
  ImGui::Text("RX Max: %.2e", (double)sdr.sdrDevice.rxParams.rxMax);
  ImGui::Text("Current Bandwidth: %.2e", (double)sdr.sdrDevice.rxParams.bandWidth);
  ImGui::Text("Min Bandwidth: %.2e", (double)sdr.sdrDevice.rxParams.bandWidthMin);
  ImGui::Text("Max Bandwidth: %.2e", (double)sdr.sdrDevice.rxParams.bandWidthMax);
  

  ImGui::Spacing();
  ImGui::InputFloat("Hz", &sdr.sdrDevice.tempRXFreq); ImGui::SameLine();
  if(ImGui::Button("Set RX Freq")){
    vplotter.sdrParams.freq = sdr.sdrDevice.tempRXFreq;
    sdr.sdrDevice.setRXFreq(sdr.sdrDevice.tempRXFreq);
  }
  ImGui::InputFloat(" ", &sdr.sdrDevice.tempBW); ImGui::SameLine();
  if(ImGui::Button("Set Bandwidth")){
    sdr.sdrDevice.setBandWidth(sdr.sdrDevice.tempBW);
  }

}

void VOverlay::showIQScatterPlot(){
  ImGui::InputFloat("Freq", &vplotter.sdrParams.freq, 100000.0f);
    if(ImGui::CollapsingHeader("IQ Plot")){
      vplotter.showScatterPlot(); 
      if(settings.first3DPlot){
        vplotter3D.createVertexBuffer();
        settings.first3DPlot = false;
      }   
      settings.draw3DPlot = true;    
    }
    else{
      settings.draw3DPlot = false;
    }
}

void VOverlay::sdrTab(){
  if(settings.implot && vplotter.haveDevice){
    ImGui::Spacing();
    ImGui::Spacing();    
    ImGui::Text("%s", vplotter.sdrInfo.deviceName.c_str()); 
    
    ImGui::Checkbox("Connect", &SDRconnected);
    connectSDR();
	}
  else{
    ImGui::Spacing();
    ImGui::Text("No SDR connected.");
  }
}

void VOverlay::graphTab(){
  if(ImGui::Checkbox("Show Graph", &settings.showTopo)){
    
  }
  if(ImGui::CollapsingHeader("3D Graph Settings")){
    grid.showGraphSettings();
  }
}

void VOverlay::prepare(){
  VBase::prepare();
  if(settings.implot){
    vplotter.setPlotter(&sdr.sdrDevice);
    vplotter3D.setupPlotter3D(&vdevice, &_graphicsQueue, &pipelineCache, &renderPass, &sdr.sdrDevice);
  }
}


VOverlay::~VOverlay(){}