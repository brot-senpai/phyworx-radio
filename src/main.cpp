#include <vulkan/vulkan.h>
#include "phyworx.hpp"

int main(){

  Phyworx* app = new Phyworx;
  app->prepare();
  app->renderLoop();
  delete app;
  
  return 0;
}
