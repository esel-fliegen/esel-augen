#include "vbase.hpp"

VBase::VBase()
{

}

void VBase::run()
{
  initGLFWwindow();
  initVulkan();
  initImgui();
  augen.cameraPaths.push_back("nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=640, height=480, format=(string)NV12, framerate=(fraction)20/1 ! nvvidconv flip-method=0 ! video/x-raw, width=640, height=480, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
  //augen.cameraPaths.push_back("nvarguscamerasrc sensor-id=1 ! video/x-raw(memory:NVMM), width=640, height=480, format=(string)NV12, framerate=(fraction)20/1 ! nvvidconv flip-method=0 ! video/x-raw, width=640, height=480, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");

  augen.initVAugen(&logicalDevice, &graphicsQueue);
  mainLoop();  
  cleanup();
}

void VBase::initVulkan()
{
  createInstance();
  setupDebugMessenger();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  

}


void VBase::mainLoop()
{
  while (!glfwWindowShouldClose(glfwWindow))
  {
    processInput(glfwWindow);
    glfwPollEvents();
    renderLoopBegin();
    augen.renderLoop(&mainWindowData);
    renderLoopEnd(&mainWindowData);
    augen.destroyFrameViewObjects();
  }
}

void VBase::cleanup()
{
  
  cleanImgui();
  augen.cleanupCameras();
  vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
  if(enableValidationLayers)
  {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, VK_NULL_HANDLE);
  }
  vkDestroyInstance(instance, VK_NULL_HANDLE);
  cleanupGLFWwindow();
}

