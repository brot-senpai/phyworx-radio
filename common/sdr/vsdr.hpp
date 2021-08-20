#pragma once
#include "vlime.hpp"

class VSDR{
  public:
    VSDR();
    ~VSDR();

    VLime sdrDevice;
};