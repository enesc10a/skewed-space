// sphere.cpp
#include "sphere.h"
#include "Angel.h"    // for mat4, Translate, Scale
#include <cmath>
#include "globals.h"
// bring in the same shaderProgram as in main.cpp


// build an arbitrary‐axis rotation matrix (Rodrigues’ formula)
static Angel::mat4 AxisAngle(const Angel::vec3& axis, float angleDeg) {
    float θ = angleDeg * Angel::DegreesToRadians;
    Angel::vec3 u = normalize(axis);
    float c = cos(θ), s = sin(θ), oc = 1.0f - c;
    return Angel::mat4(
        Angel::vec4(oc*u.x*u.x +    c, oc*u.x*u.y - u.z*s, oc*u.x*u.z + u.y*s, 0.0f),
        Angel::vec4(oc*u.y*u.x + u.z*s, oc*u.y*u.y +    c, oc*u.y*u.z - u.x*s, 0.0f),
        Angel::vec4(oc*u.z*u.x - u.y*s, oc*u.z*u.y + u.x*s, oc*u.z*u.z +    c, 0.0f),
        Angel::vec4(0,0,0,1)
    );
}

Sphere::Sphere(int subdivisions,
               const Properties& props,
               const std::vector<Sphere*>* neighbors)
    : props_(props), neighbors_(neighbors)
{
    setupGeometry(subdivisions);
}

Sphere::~Sphere() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void Sphere::setupGeometry(int subdivisions) {
    struct Vertex { vec3 pos, normal; vec2 tex; };
    std::vector<Vertex> verts;
    int subAxis = subdivisions * 4;
    int subHeight = subdivisions * 2;

    // build vertex list
    for (int y = 0; y <= subHeight; ++y) {
        float θ = M_PI * y / subHeight;
        float sT = sin(θ), cT = cos(θ);
        for (int x = 0; x <= subAxis; ++x) {
            float φ = 2.0f * M_PI * x / subAxis;
            float sP = sin(φ), cP = cos(φ);
            vec3 pos = vec3(
                props_.radius * sT * cP,
                props_.radius * cT,
                props_.radius * sT * sP
            );
            vec3 norm = normalize(pos);
            vec2 st = vec2(
                atan2(pos.z,pos.x)/(2.0f*M_PI) + 0.5f,
                acos(pos.y/props_.radius)/M_PI
            );
            verts.push_back({pos,norm,st});
        }
    }

    // build indexed triangles
    std::vector<Vertex> sphereVerts;
    sphereVerts.reserve(6 * subAxis * subHeight);
    for (int y = 0; y < subHeight; ++y) {
        for (int x = 0; x < subAxis; ++x) {
            int i0 =  y      * (subAxis+1) + x;
            int i1 = (y+1)   * (subAxis+1) + x;
            int i2 = i0 + 1;
            int i3 = i1 + 1;
            sphereVerts.push_back(verts[i0]);
            sphereVerts.push_back(verts[i1]);
            sphereVerts.push_back(verts[i2]);
            sphereVerts.push_back(verts[i1]);
            sphereVerts.push_back(verts[i3]);
            sphereVerts.push_back(verts[i2]);
        }
    }

    numVertices_ = int(sphereVerts.size());
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 sphereVerts.size()*sizeof(Vertex),
                 sphereVerts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,tex));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
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

void Sphere::draw() const {
    // build model matrix: translate, rotate, scale
    Angel::mat4 T = Angel::Translate(props_.position);
    Angel::mat4 R = AxisAngle(props_.rotationAxis, orientation_);
    Angel::mat4 S = Angel::Scale(props_.radius, props_.radius, props_.radius);
    Angel::mat4 M = T * R * S;

    // upload to uniform
    GLint loc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(loc, 1, GL_TRUE, M);

    // draw
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, numVertices_);
    glBindVertexArray(0);
}
