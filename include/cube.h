#ifndef CUBE_H
#define CUBE_H

#include <GLFW/glfw3.h>

class Cube {
public:
    Cube();
    ~Cube();

    void draw();

private:
    GLuint vao;
    GLuint vbo; 
    void setupCube();
};

#endif
