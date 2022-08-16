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

    VkDevice* logicalDevice;
    VkQueue* graphicsQueue;

    std::vector<cv::VideoCapture> cameras;
    std::vector<std::string> cameraPaths;
    cv::Mat frame1;
    ImTextureID frameTexture;
    bool showCamera = true;
    
    void initVAugen(VkDevice*, VkQueue*, std::vector<std::string>);
    void setupCamera(std::vector<std::string>);
    void captureFrame();

    bool createFrameTexture(VkCommandBuffer, cv::Mat);
    void destroyFrameObjects();
    void destroyFrameViewObjects();
    void renderLoop(ImGui_ImplVulkanH_Window*);
    VkResult vkResult(VkResult, std::string);
};