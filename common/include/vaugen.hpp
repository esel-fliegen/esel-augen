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
    int width, height;
    
    void initVAugen(VkDevice*, VkQueue*);
    void setupCamera(std::vector<std::string>);
    void captureFrame();
    void cleanupCameras();

    bool createFrameTexture(VkCommandBuffer, cv::Mat);
    void destroyFrameObjects();
    void destroyFrameViewObjects();
    void renderLoop(ImGui_ImplVulkanH_Window*);
    VkResult vkResult(VkResult, std::string);
};
