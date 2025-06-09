#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "cube.h"

Cube::Cube() { setupCube(); }

Cube::~Cube() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Cube::setupCube() {
    GLfloat vertices[] = {
        // Front face
        -1.0f, -1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,

        // Back face
        -1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,

        // Left face
        -1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,

        // Right face
         1.0f, -1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,

        // Top face
        -1.0f,  1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,

        // Bottom face
        -1.0f, -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Cube::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
