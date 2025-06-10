#version 410 core
precision highp float;

uniform vec2  uResolution;
uniform vec3  uCamPos;
uniform vec3  uCamRight;
uniform vec3  uCamUp;
uniform vec3  uCamForward;
uniform float uFovRad;
uniform float uAspect;

uniform vec3  uSpherePos[1];     // only Sun
uniform float uSphereRad[1];
uniform sampler2D uSphereTex[1];

out vec4 fragColor;

vec4 sphereColor(int id, vec3 P)
{
    vec3 n = normalize(P - uSpherePos[id]);
    float u = 0.5 + atan(n.z,n.x)/(2.0*3.14159265);
    float v = 0.5 - asin(n.y)   / 3.14159265;
    return texture(uSphereTex[id], vec2(u,v));
}

void main()
{
    // build simple straight ray
    vec2 ndc   = (gl_FragCoord.xy / uResolution) * 2.0 - 1.0;
    float pin  = -1.0 / tan(uFovRad * 0.5);
    vec3 dir   = normalize(ndc.x * uAspect * uCamRight +
                           ndc.y *           uCamUp    +
                           pin   *           uCamForward);

    // analytic ray–sphere intersection with the Sun
    vec3  ro = uCamPos;
    vec3  rc = uSpherePos[0];
    vec3  q  = ro - rc;
    float b  = dot(dir,q);
    float c  = dot(q,q) - uSphereRad[0]*uSphereRad[0];
    float d  = b*b - c;

    if(d < 0.0){
        fragColor = vec4(0.0,0.0,0.0,1.0);   // miss → black
        return;
    }

    float t = -b - sqrt(d);
    if(t < 0.0){                             // behind camera
        fragColor = vec4(0.0,0.0,0.0,1.0);
        return;
    }

    vec3 hitP = ro + t * dir;
    fragColor = sphereColor(0, hitP);        // texel from Sun only
}

//
//  diag.glsl
//  410HW_00
//
//  Created by Enes Faruk Çona on 10.06.2025.
//

