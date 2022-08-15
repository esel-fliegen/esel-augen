#pragma once

#include "vimgui.hpp"
#include "vaugen.hpp"


class VBase : public VImgui
{
  public:
    VBase();

    std::string path = "/dev/video0";
    void run();
    void mainLoop();
    void initVulkan();    
    void cleanup();




  protected:
    
};

