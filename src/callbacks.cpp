// callbacks.cpp
#include "callbacks.h"
#include "Angel.h"
#include <GLFW/glfw3.h>

// camera globals
vec3 cameraPos   = vec3(0.0f, 0.0f,  50.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp    = vec3(0.0f, 1.0f,  0.0f);
float cameraSpeed = 20.0f; // units per second

static double lastTime = 0.0;

void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // get current time
    double currentTime = glfwGetTime();

    // static so it persists across calls,
    // initialize to negative to detect “first time”
    static double lastTime = -1.0;
    float        delta;

    if (lastTime < 0.0) {
        // first key‐press: ignore the jump
        delta = 0.0f;
    } else {
        delta = float(currentTime - lastTime);
    }
    lastTime = currentTime;

    // now move camera with a sane delta
    vec3 right = normalize(cross(cameraFront, cameraUp));
    if (key == GLFW_KEY_W)
        cameraPos += cameraFront * cameraSpeed * delta;
    else if (key == GLFW_KEY_S)
        cameraPos -= cameraFront * cameraSpeed * delta;
    else if (key == GLFW_KEY_A)
        cameraPos -= right * cameraSpeed * delta;
    else if (key == GLFW_KEY_D)
        cameraPos += right * cameraSpeed * delta;
    else if (key == GLFW_KEY_Q)
        cameraPos += cameraUp * cameraSpeed * delta;
    else if (key == GLFW_KEY_E)
        cameraPos -= cameraUp * cameraSpeed * delta;
    else if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
