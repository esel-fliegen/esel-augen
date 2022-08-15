#pragma once

#include "opencv2/opencv.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include <string>
#include <iostream>
#include <vector>



class VAugen 
{
  public:
    VAugen();

    std::vector<cv::VideoCapture> cameras;
    std::vector<std::string> cameraPaths;
    cv::Mat frame1;
    ImTextureID frameTexture;

    void setupCamera(std::vector<std::string>);
    void captureFrame();

    bool createFrameTexture(VkCommandBuffer, cv::Mat);
    VkResult vkResult(VkResult, std::string);
};