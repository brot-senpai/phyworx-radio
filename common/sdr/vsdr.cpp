#include "vsdr.hpp"

VSDR::VSDR(){}

VSDR::~VSDR(){
  sdrDevice.endStream();
}