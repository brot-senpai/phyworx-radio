#pragma once

#include "vbase.hpp"
#include "vsdr.hpp"
#include "vplotter.hpp"
#include "vplotter3d.hpp"

class VOverlay : public VBase{
  public:
    VOverlay();
    ~VOverlay();

    void                                        prepare();

  protected:
    VSDR                                        sdr;
    
    pworx::VPlotter                             vplotter;
    pworx::VPlotter3D                           vplotter3D;
    uint32_t                                    overlayWidth = 400;

    bool                                        SDRconnected = false;
    
    virtual void                                updateOverlay();

    void                                        connectSDR();
    void                                        showIQScatterPlot();
    void                                        showSDRSettings();
    void                                        sdrTab();
    void                                        graphTab();


    
};