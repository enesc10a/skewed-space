#ifndef SPHERE_H
#define SPHERE_H

#include "Angel.h"
#include <vector>

using namespace Angel;

class Sphere {
public:
    struct Properties {
        float   radius;
        vec3    position;
        vec3    velocity;
        vec3    rotationAxis;
        float   rotationSpeed;
        float   mass;
    };


    Sphere(
           const Properties& props,
           const std::vector<Sphere*>* neighbors);
    ~Sphere();

    void computeNetForce();
    void integrate(float deltaTime, float forceScale);
    void draw() const;

    const Properties& getProperties() const { return props_; }

private:

    Properties                  props_;
    const std::vector<Sphere*>* neighbors_    = nullptr;
    vec3                        netForce_     = vec3(0.0f);
    float                       orientation_  = 5.0f;
};

#endif 
