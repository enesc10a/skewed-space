#version 410 core
precision highp float;

const int   MAX_SPHERES      = 8;
const int   MAX_STEPS        = 900;
const float STEP_FACTOR      = 0.015;   // dt ≈ r²·STEP_FACTOR
const float HIT_EPS          = 0.00000001;    // “close enough” threshold

// ───────── uniforms pushed each frame ──────────────────────────────
uniform vec2  uResolution;              // window size
uniform vec3  uCamPos;                  // world
uniform mat4  uCamInvView;              // inverse view matrix
uniform float uFovRad;                  // vertical FOV
uniform float uWorldLimit;              // far escape radius
uniform float uLightSpeed;              // “massless” speed   (1.0 default)
uniform float uGravityScale;            // scales masses      (1.0 default)

uniform int   uSphereCount;
uniform vec3  uSpherePos [MAX_SPHERES];
uniform float uSphereRad [MAX_SPHERES];
uniform float uSphereMass[MAX_SPHERES];
uniform sampler2D uSphereTex[MAX_SPHERES];

out vec4 fragColor;

// ───────── helpers ─────────────────────────────────────────────────
vec3 gravitationalAccel(vec3 p) {
    vec3 a = vec3(0.0);
    for(int i=0;i<uSphereCount;i++){
        vec3 r = uSpherePos[i]-p;
        float d2 = dot(r,r)+1e-6;                  // avoid /0
        a += uGravityScale*uSphereMass[i]*r / (d2*sqrt(d2));
    }
    return a;
}

bool sphereHit(vec3 p, out int idx){
    for(int i=0;i<uSphereCount;i++){
        if(length(p-uSpherePos[i]) < uSphereRad[i]+HIT_EPS){
            idx=i; return true;
        }
    }
    return false;
}

vec4 sampleSphereColor(int i, vec3 hitPos){
    // local → sphere-local normal
    vec3 n = normalize(hitPos - uSpherePos[i]);
    float u = 0.5 + atan(n.z,n.x)/(2.0*3.14159265);
    float v = 0.5 - asin(n.y)/3.14159265;
    return texture(uSphereTex[i], vec2(u,v));
}

// ───────── main ────────────────────────────────────────────────────
void main(){
    // 1. primary ray (NDC → world)
    vec2 ndc = (gl_FragCoord.xy / uResolution) * 2.0 - 1.0;
    float z = -1.0 / tan(uFovRad*0.5);                  // pinhole dist
    vec3 dirView = normalize(vec3(ndc.x, -ndc.y, z));
    vec3 dirWorld = normalize((uCamInvView * vec4(dirView,0.0)).xyz);

    vec3 pos = uCamPos;
    vec3 vel = dirWorld * uLightSpeed;

    float s = 0.0;                                      // arc length
    int   hitIdx = -1;

    // 2. integrate geodesic
    for(int step=0; step<MAX_STEPS; ++step){
        float r = length(pos);
        if(r>uWorldLimit) break;
        if(sphereHit(pos,hitIdx)) break;

        // adaptive dt ~ r² to keep large steps far away
        float dt = r*r*STEP_FACTOR;

        // RK4 -------------------------------------------------------
        vec3 k1v = gravitationalAccel(pos) * dt;
        vec3 k1x = vel * dt;

        vec3 k2v = gravitationalAccel(pos + 0.5*k1x) * dt;
        vec3 k2x = (vel + 0.5*k1v) * dt;

        vec3 k3v = gravitationalAccel(pos + 0.5*k2x) * dt;
        vec3 k3x = (vel + 0.5*k2v) * dt;

        vec3 k4v = gravitationalAccel(pos + k3x) * dt;
        vec3 k4x = (vel + k3v) * dt;

        pos += (k1x + 2.0*(k2x+k3x) + k4x) / 6.0;
        vel += (k1v + 2.0*(k2v+k3v) + k4v) / 6.0;
        s   += length(k1x);                              // good enough
    }

    // 3. colouring
    if(hitIdx >= 0){
        vec4 col = sampleSphereColor(hitIdx, pos);
        gl_FragDepth = 1.0 - exp(-s * 0.01);   // (or your linear depth)
        fragColor    = col;
    } else {
        fragColor    = vec4(0.0, 0.0, 0.0, 1.0);   // ← solid black background
        gl_FragDepth = 1.0;
    }
}

//
//  raytracer.glsl
//  410HW_00
//
//  Created by Enes Faruk Çona on 10.06.2025.
//

