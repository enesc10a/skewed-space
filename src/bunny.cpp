#include "Angel.h"
#include "bunny.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Constructor: Loads the OFF file and initializes the bunny model
Bunny::Bunny(const char* filename) {
    loadOFF(filename);
}

// Loads the 3D model from an OFF file
void Bunny::loadOFF(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read OFF file header and vertex/face counts
    std::string header;
    int numVerts, numTriangles, edges;
    file >> header >> numVerts >> numTriangles >> edges;

    // Read vertex positions
    std::vector<vec4> vertices(numVerts);
    for (int i = 0; i < numVerts; ++i) {
        float x, y, z;
        file >> x >> y >> z;
        vertices[i] = vec4(x, y, z, 1.0f); // Store in homogeneous coordinates
    }

    // Read face indices and construct triangle list
    std::vector<vec4> bunnyVertices;
    for (int i = 0; i < numTriangles; ++i) {
        int vertexCount, idx0, idx1, idx2;
        file >> vertexCount >> idx0 >> idx1 >> idx2;
        bunnyVertices.push_back(vertices[idx0]);
        bunnyVertices.push_back(vertices[idx1]);
        bunnyVertices.push_back(vertices[idx2]); // Convert to triangle list
    }

    numVertices = bunnyVertices.size(); // Store number of triangles

    // Setup OpenGL VAO & VBO for rendering the bunny model
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bunnyVertices.size() * sizeof(vec4), &bunnyVertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glBindVertexArray(0); // Unbind VAO
}

// Draws the bunny model using the loaded vertex data
void Bunny::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

// Destructor: Cleans up allocated OpenGL buffers
Bunny::~Bunny() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
