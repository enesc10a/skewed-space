#ifndef SPHERE_H
#define SPHERE_H

#include "Angel.h"
#include <vector>

using namespace Angel;  // so you can write vec3 instead of Angel::vec3

class Sphere {
public:
    // ─── INSERT THIS ───
    struct Properties {
        float   radius;
        vec3    position;
        vec3    velocity;
        vec3    rotationAxis;
        float   rotationSpeed;
        float   mass;
    };
    // ────────────────────

    Sphere(int subdivisions,
           const Properties& props,
           const std::vector<Sphere*>* neighbors);
    ~Sphere();

    void computeNetForce();
    void integrate(float deltaTime, float forceScale);
    void draw() const;

    const Properties& getProperties() const { return props_; }

private:
    void setupGeometry(int subdivisions);

    GLuint                      vao_          = 0;
    GLuint                      vbo_          = 0;
    int                         numVertices_  = 0;

    Properties                  props_;
    const std::vector<Sphere*>* neighbors_    = nullptr;
    vec3                        netForce_     = vec3(0.0f);
    float                       orientation_  = 5.0f;
};

#endif // SPHERE_H
