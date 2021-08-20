#pragma once

#include "vbuffer.hpp"
#include "vvertex.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>


class VFunc{
  public:
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float zmin;
    float zmax;
    float precision;
    float steps;
    uint32_t tCount = 0;
    uint32_t xCount = 0;
      

    std::vector<pworx::VVertex2>        vertices;
    pworx::Buffer                       buffer;
    pworx::TopologyType                 topoType;
    float func(float x, float t){
      return 1 - pow(tanh(x-(-t)), 2);
    }

    void generateVertices(){
      glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
      steps = (uint32_t)(1/precision);
      const int xmin_int = (int)(xmin * steps);
      const int xmax_int = (int)(xmax * steps);
      const int ymin_int = (int)(ymin * steps);
      const int ymax_int = (int)(ymax * steps);
      for(int t = ymin_int; t < ymax_int; t++){
        tCount += 1;
        int x_counter = 0;
        float t_float = (float)t/steps;
        for(int x = xmin_int; x < xmax_int; x++){
          float x_float = (float)x/steps;
          x_counter += 1; 
          float u = func(x_float, t_float);
          vertices.push_back({glm::vec3(x_float, t_float, u), color});
        }
        xCount = x_counter;
      }
  }
};

