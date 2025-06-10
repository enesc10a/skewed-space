#version 410 core
precision highp float;

const int   MAX_SPHERES = 8;
const int   MAX_STEPS   = 900;
const float STEP_FACTOR = 0.015;
const float MAX_DT      = 1.0;          // clamp step size

/* -------- uniforms ------------------------------------------------------- */
uniform vec2  uResolution;
uniform vec3  uCamPos;
uniform vec3  uCamRight;
uniform vec3  uCamUp;
uniform vec3  uCamForward;
uniform float uFovRad;
uniform float uAspect;

uniform float uWorldLimit;
uniform float uLightSpeed;
uniform float uGravityScale;
uniform float uHitEps;
uniform float uDepthFar;

uniform int   uSphereCount;
uniform vec3  uSpherePos [MAX_SPHERES];
uniform float uSphereRad [MAX_SPHERES];
uniform float uSphereMass[MAX_SPHERES];
uniform sampler2D uSphereTex[MAX_SPHERES];

out vec4 fragColor;

/* -------- gravity -------------------------------------------------------- */
vec3 gravAccel(vec3 p)
{
    vec3 a = vec3(0.0);
    for(int i=0;i<uSphereCount;++i){
        vec3 r = uSpherePos[i]-p;
        float d2 = dot(r,r) + 1e-6;
        a += uGravityScale * uSphereMass[i] * r / (d2*sqrt(d2));
    }
    return a;
}

/* -------- exact segment–sphere test ------------------------------------- */
bool segmentSphere(vec3 p0, vec3 p1, out int idx, out float tHit)
{
    vec3 d = p1 - p0;                       // segment direction
    float a = dot(d,d);

    for(int i=0;i<uSphereCount;++i){
        float rTot = uSphereRad[i] + uHitEps;
        vec3  m = p0 - uSpherePos[i];
        float b = dot(m, d);
        float c = dot(m, m) - rTot*rTot;

        if(c <= 0.0){ idx = i; tHit = 0.0; return true; }   // already in

        float disc = b*b - a*c;
        if(disc < 0.0) continue;            // miss

        float t = (-b - sqrt(disc)) / a;    // first root
        if(t >= 0.0 && t <= 1.0){ idx=i; tHit=t; return true; }
    }
    return false;
}

/* -------- texture lookup ------------------------------------------------- */
vec4 sphereColor(int id, vec3 P)
{
    vec3 n = normalize(P - uSpherePos[id]);
    float u = 0.5 + atan(n.z,n.x)/(2.0*3.14159265);
    float v = 0.5 - asin(n.y)   / 3.14159265;
    vec2  uv = clamp(vec2(u,v), 0.0, 1.0);   // no wrap
    return texture(uSphereTex[id], uv);
}

/* -------- main ----------------------------------------------------------- */
void main()
{
    /* build initial ray dir from basis */
    vec2  ndc = (gl_FragCoord.xy / uResolution) * 2.0 - 1.0;
    float pin = -1.0 / tan(uFovRad * 0.5);
    vec3  dir = normalize(ndc.x * uAspect * uCamRight +
                          ndc.y *           uCamUp    +
                          pin   *           uCamForward);

    vec3 pos = uCamPos;
    vec3 vel = dir * uLightSpeed;
    float s  = 0.0;
    int   hit = -1;

    for(int step=0; step<MAX_STEPS; ++step)
    {
        float r = length(pos);
        if(r > uWorldLimit) break;

        float dt = min(r*r * STEP_FACTOR, MAX_DT);
        vec3  nextPos = pos + vel * dt;

        float tHit;
        if (segmentSphere(pos, nextPos, hit, tHit)) {
            pos = mix(pos, nextPos, tHit);  // exact intersection point
            s  += length(vel) * dt * tHit;  // add partial distance
            break;
        }

        /* RK4 update */
        vec3 k1v = gravAccel(pos) * dt;
        vec3 k1x = vel * dt;

        vec3 k2v = gravAccel(pos + 0.5*k1x) * dt;
        vec3 k2x = (vel + 0.5*k1v) * dt;

        vec3 k3v = gravAccel(pos + 0.5*k2x) * dt;
        vec3 k3x = (vel + 0.5*k2v) * dt;

        vec3 k4v = gravAccel(pos + k3x) * dt;
        vec3 k4x = (vel + k3v) * dt;

        pos += (k1x + 2.0*(k2x+k3x) + k4x) / 6.0;
        vel += (k1v + 2.0*(k2v+k3v) + k4v) / 6.0;
        s   += length(k1x);                 // accumulate distance
    }

    if(hit >= 0){
        vec4 col = sphereColor(hit, pos);
        gl_FragDepth = clamp(s / uDepthFar, 0.0, 1.0);
        fragColor    = col;
    } else {
        fragColor    = vec4(0.0, 0.0, 0.0, 1.0);
        gl_FragDepth = 1.0;
    }
}
