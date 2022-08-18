#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



class VWindow{
  public:
    GLFWwindow* glfwWindow;
    uint32_t WINDOW_WIDTH = 1280;
    uint32_t WINDOW_HEIGHT = 480;
    VWindow();
    void initGLFWwindow();
    void processInput(GLFWwindow*);
    void cleanupGLFWwindow();

};