#include "vgridui.hpp"

VGridUI::VGridUI(){
  setParams(-10, 10, -10, 10, -1, 5, 0.5);
}

void VGridUI::showGraphSettings(){
  ImGui::Checkbox("Show XY", &gridParams._showXY);
  ImGui::Checkbox("Show XZ", &gridParams._showXZ);
  ImGui::Checkbox("Show YZ", &gridParams._showYZ);
  if(ImGui::Checkbox("White Grid", &gridParams._whiteGrid)){
    recreateXY();
    recreateXZ();
    recreateYZ();
  }
  if(ImGui::InputFloat("XY Alpha", &gridParams._xyAlpha, 0.1)){
    recreateXY();
  }  
  if(ImGui::InputFloat("XZ Alpha", &gridParams._xzAlpha, 0.1)){
    recreateXZ();
  }  
  if(ImGui::InputFloat("YZ Alpha", &gridParams._yzAlpha, 0.1)){
    recreateYZ();
  }  
}


VGridUI::~VGridUI(){}