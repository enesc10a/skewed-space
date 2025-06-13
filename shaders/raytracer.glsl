// shaders/raytracer.glsl  —  Modified for self-rotation
#version 410 core
precision highp float;

const int   MAX_SPHERES = 8;
const int   MAX_STEPS   = 30;
const float STEP_FACTOR = 0.15;
const float MAX_DT      = 1.0;

uniform vec2  uResolution;
uniform vec3  uCamPos, uCamRight, uCamUp, uCamForward;
uniform float uFovRad, uAspect;
uniform float uWorldLimit, uLightSpeed, uGravityScale, uHitEps, uDepthFar;

uniform int   uSunCount;
uniform int   uSunIndices[MAX_SPHERES];
uniform vec3  uKd, uKs;
uniform float uShininess;

uniform int   uSphereCount;
uniform vec3  uSpherePos[MAX_SPHERES];
uniform float uSphereRad[MAX_SPHERES];
uniform float uSphereMass[MAX_SPHERES];
uniform sampler2D uSphereTex[MAX_SPHERES];

// self-rotation:
uniform float uTime;
uniform vec3  uRotationAxis[MAX_SPHERES];
uniform float uRotationSpeed[MAX_SPHERES];

out vec4 fragColor;

float computeTimeStep(vec3 pos) {
    float dtRaw = length(pos) * length(pos) * STEP_FACTOR;
    return clamp(dtRaw, 0.2, MAX_DT); // lower cap: 0.0001, upper cap: MAX_DT
}

mat3 axisAngleMat(vec3 axis,float a){
    float c=cos(a), s=sin(a);
    vec3 u=normalize(axis), omc=(1.0-c)*u;
    return mat3(
      vec3(c+omc.x*u.x,    omc.x*u.y - u.z*s, omc.x*u.z + u.y*s),
      vec3(omc.y*u.x + u.z*s, c+omc.y*u.y,    omc.y*u.z - u.x*s),
      vec3(omc.z*u.x - u.y*s, omc.z*u.y + u.x*s, c+omc.z*u.z)
    );
}

vec3 gravAccel(vec3 p){
    vec3 a=vec3(0);
    for(int i=0;i<uSphereCount;++i){
        vec3 r=uSpherePos[i]-p; float d2=dot(r,r)+1e-6;
        a+=uGravityScale*uSphereMass[i]*r/(d2*sqrt(d2));
    }
    return a;
}

bool segSphere(vec3 p0,vec3 p1,out int idx,out float tHit){
    vec3 d=p1-p0; float A=dot(d,d);
    for(int i=0;i<uSphereCount;++i){
        float R=uSphereRad[i]+uHitEps;
        vec3 m=p0-uSpherePos[i];
        float B=dot(m,d), C=dot(m,m)-R*R;
        float disc=B*B-A*C; if(disc<0) continue;
        float t=(-B-sqrt(disc))/A;
        if(t>=0.0 && t<=1.0){ idx=i; tHit=t; return true; }
    }
    return false;
}

vec4 texSphere(int id,vec3 P){
    vec3 n0=normalize(P-uSpherePos[id]);
    float ang=uRotationSpeed[id]*uTime;
    mat3 R=axisAngleMat(uRotationAxis[id],ang);
    vec3 n=normalize(R*n0);
    float u=0.5+atan(n.z,n.x)/(2.0*3.14159265);
    float v=0.5-asin(n.y)/3.14159265;
    return texture(uSphereTex[id],clamp(vec2(u,v),0.0,1.0));
}

void main(){
    vec2 ndc=(gl_FragCoord.xy/uResolution)*2.0-1.0;
    float pin=-1.0/tan(uFovRad*0.5);
    vec3 dir=normalize(ndc.x*uAspect*uCamRight+ndc.y*uCamUp+pin*uCamForward);
    vec3 pos=uCamPos, vel=dir*uLightSpeed;
    float s=0.0; int hit=-1; float tHit;

    for(int i=0;i<MAX_STEPS;++i){
        if(length(pos)>uWorldLimit) break;
        float dt = computeTimeStep(pos);
        vec3 next=pos+vel*dt;
        if(segSphere(pos,next,hit,tHit)){
            pos=mix(pos,next,tHit);
            s+=length(vel)*dt*tHit;
            break;
        }
        vec3 k1v=gravAccel(pos)*dt, k1x=vel*dt;
        vec3 k2v=gravAccel(pos+0.5*k1x)*dt, k2x=(vel+0.5*k1v)*dt;
        vec3 k3v=gravAccel(pos+0.5*k2x)*dt, k3x=(vel+0.5*k2v)*dt;
        vec3 k4v=gravAccel(pos+k3x)*dt, k4x=(vel+k3v)*dt;
        pos+= (k1x+2.0*(k2x+k3x)+k4x)/6.0;
        vel+= (k1v+2.0*(k2v+k3v)+k4v)/6.0;
        s+=length(k1x);
    }

    if(hit>=0){
        vec4 base=texSphere(hit,pos);
        vec3 col;
        bool isSun=false;
        for(int i=0;i<uSunCount;++i) if(hit==uSunIndices[i]){isSun=true;break;}
        if(isSun) col=base.rgb;
        else{
            col=vec3(0);
            vec3 N=normalize(pos-uSpherePos[hit]);
            for(int i=0;i<uSunCount;++i){
                int si=uSunIndices[i];
                vec3 L=normalize(uSpherePos[si]-uSpherePos[hit]);
                float diff=max(dot(N,L),0.0);
                if(diff>0.0){
                    vec3 V=normalize(uCamPos-pos);
                    vec3 Rreflect=reflect(-L,N);
                    float spec=pow(max(dot(Rreflect,V),0.0),uShininess);
                    col+=uKd*diff*base.rgb + uKs*spec;
                }
            }
        }
        gl_FragDepth=clamp(s/uDepthFar,0.0,1.0);
        fragColor=vec4(col,base.a);
    } else {
        fragColor=vec4(0,0,0,1);
        gl_FragDepth=1.0;
    }
}
