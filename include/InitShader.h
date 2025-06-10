#ifndef INITSHADER_H
#define INITSHADER_H

#include <GLFW/glfw3.h> 

GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile);
GLuint InitShader2(const char* vertexShaderFile,
                   const char* fragmentShaderFile);

#endif

