#pragma once

#include "vimgui.hpp"
#include "vaugen.hpp"
#include <memory>


class VBase : public VImgui
{
  public:
    VBase();

    std::vector<std::string> paths ;
    void run();
    void mainLoop();
    void initVulkan();    
    void cleanup();

 
    VAugen augen;

  protected:
    
};

