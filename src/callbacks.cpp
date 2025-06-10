// callbacks.cpp
#include "callbacks.h"
#include <GLFW/glfw3.h>

// Camera globals (defined in callbacks.h)

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Only handle ESC here; movement is polled per-frame in main.cpp
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Prevent division by zero on minimize/restore
    if (width  == 0) width  = 1;
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
}
