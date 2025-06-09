#ifndef BUNNY_H
#define BUNNY_H

#include "Angel.h"
#include <vector>

class Bunny {
public:
    Bunny(const char* filename);
    ~Bunny();
    void draw();

private:
    GLuint vao, vbo;
    int numVertices;
    void loadOFF(const char* filename);
};

#endif

