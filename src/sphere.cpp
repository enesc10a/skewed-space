// sphere.cpp
#include "sphere.h"
#include "Angel.h"    

Sphere::Sphere(const Properties& props,
               const std::vector<Sphere*>* neighbors)
    : props_(props), neighbors_(neighbors)
{
}

Sphere::~Sphere() {
}

void Sphere::computeNetForce() {
    netForce_ = vec3(0.0f);
    if (!neighbors_) return;
    const float G = 1.0f;
    for (auto* other : *neighbors_) {
        if (other==this) continue;
        vec3 dir = other->props_.position - props_.position;
        float d = length(dir);
        if (d<=0.0f) continue;
        vec3 fdir = normalize(dir);
        float mag = G * props_.mass * other->props_.mass / (d*d);
        netForce_ += fdir * mag;
    }
}

void Sphere::integrate(float dt, float forceScale) {
    // translation
    vec3 accel = (netForce_/props_.mass) * forceScale;
    props_.velocity += accel * dt;
    props_.position += props_.velocity * dt;
    // self-rotation
    orientation_ += props_.rotationSpeed * dt;
    if (orientation_ > 360.0f) orientation_ -= 360.0f;
}
