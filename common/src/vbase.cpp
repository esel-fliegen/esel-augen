#include "vbase.hpp"

VBase::VBase()
{

}

void VBase::run()
{
  initGLFWwindow();
  initVulkan();
  initImgui();
  augen.cameraPaths.push_back("/dev/video0");
  augen.initVAugen(&logicalDevice, &graphicsQueue, augen.cameraPaths);
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
  vkDestroyDevice(logicalDevice, VK_NULL_HANDLE);
  if(enableValidationLayers)
  {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, VK_NULL_HANDLE);
  }
  vkDestroyInstance(instance, VK_NULL_HANDLE);
  cleanupGLFWwindow();
}

