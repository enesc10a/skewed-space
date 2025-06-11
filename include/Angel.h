#ifndef ANGEL_H
#define ANGEL_H

#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>


// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))

namespace Angel {

    GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile);

    const GLfloat DegreesToRadians = M_PI / 180.0;
    const GLfloat DivideByZeroTolerance = 1.0e-07f;

} // namespace Angel

#include "vec.h"
#include "mat.h"

using namespace Angel;

#endif // ANGEL_H
