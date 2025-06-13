#version 410 core
precision highp float;

const int   MAX_SPHERES = 8;
const int   MAX_STEPS   = 90;
const float STEP_FACTOR = 0.015;
const float MAX_DT      = 1.0;

/* ─── uniforms from C++ ─────────────────────────────────────────── */
uniform vec2  uResolution;
uniform vec3  uCamPos;
uniform vec3  uCamRight, uCamUp, uCamForward;
uniform float uFovRad,   uAspect;

uniform float uWorldLimit;
uniform float uLightSpeed;
uniform float uGravityScale;
uniform float uHitEps;
uniform float uDepthFar;

uniform int   uSunIndex;          // Sun = 0
uniform vec3  uKa, uKd, uKs;      // 0.1 / 0.8 / 1.0
uniform float uShininess;         // 32

uniform int   uSphereCount;
uniform vec3  uSpherePos [MAX_SPHERES];
uniform float uSphereRad [MAX_SPHERES];
uniform float uSphereMass[MAX_SPHERES];
uniform sampler2D uSphereTex[MAX_SPHERES];

out vec4 fragColor;

/* ─── helpers: gravity, segment hit, texture lookup ─────────────── */
vec3 gravA(vec3 p){
    vec3 a = vec3(0);
    for(int i=0;i<uSphereCount;++i){
        vec3 r=uSpherePos[i]-p; float d2=dot(r,r)+1e-6;
        a+=uGravityScale*uSphereMass[i]*r/(d2*sqrt(d2));
    }
    return a;
}

bool segSphere(vec3 p0,vec3 p1,out int idx,out float tHit){
    vec3 d=p1-p0; float a=dot(d,d);
    for(int i=0;i<uSphereCount;++i){
        float r=uSphereRad[i]+uHitEps;
        vec3  m=p0-uSpherePos[i];
        float b=dot(m,d), c=dot(m,m)-r*r;
        if(c<=0){idx=i;tHit=0;return true;}
        float disc=b*b-a*c; if(disc<0)continue;
        float t=(-b-sqrt(disc))/a;
        if(t>=0&&t<=1){idx=i;tHit=t;return true;}
    }return false;
}

vec4 texSphere(int id,vec3 P){
    vec3 n=normalize(P-uSpherePos[id]);
    float u=0.5+atan(n.z,n.x)/(2.0*3.14159265);
    float v=0.5-asin(n.y)/3.14159265;
    return texture(uSphereTex[id],clamp(vec2(u,v),0.0,1.0));
}

/* ─── main ──────────────────────────────────────────────────────── */
void main(){
    /* primary ray */
    vec2  ndc=(gl_FragCoord.xy/uResolution)*2.0-1.0;
    float pin=-1.0/tan(uFovRad*0.5);
    vec3  dir=normalize(ndc.x*uAspect*uCamRight+
                        ndc.y*        uCamUp   +
                        pin *         uCamForward);

    vec3 pos=uCamPos;
    vec3 vel=dir*uLightSpeed;
    float s =0.0;
    int   hit=-1;

    for(int st=0;st<MAX_STEPS;++st){
        if(length(pos)>uWorldLimit)break;

        float dt=min(length(pos)*length(pos)*STEP_FACTOR,MAX_DT);
        vec3  next=pos+vel*dt;
        float tHit;
        if(segSphere(pos,next,hit,tHit)){
            pos=mix(pos,next,tHit);
            s += length(vel)*dt*tHit;
            break;
        }
        /* RK4 */
        vec3 k1v=gravA(pos)*dt, k1x=vel*dt;
        vec3 k2v=gravA(pos+0.5*k1x)*dt, k2x=(vel+0.5*k1v)*dt;
        vec3 k3v=gravA(pos+0.5*k2x)*dt, k3x=(vel+0.5*k2v)*dt;
        vec3 k4v=gravA(pos+k3x)*dt   , k4x=(vel+k3v)*dt;
        pos+= (k1x+2.0*(k2x+k3x)+k4x)/6.0;
        vel+= (k1v+2.0*(k2v+k3v)+k4v)/6.0;
        s  += length(k1x);
    }

    if(hit>=0){
        vec4 base = texSphere(hit,pos);

        /* directional Phong from Sun-center → hit-sphere-center */
        vec3 finalRGB;
        if(hit==uSunIndex){
            finalRGB = base.rgb;                           // emissive Sun
        }else{
            vec3 L = normalize(uSpherePos[uSunIndex] - uSpherePos[hit]);
            vec3 N = normalize(pos - uSpherePos[hit]);
            vec3 V = normalize(uCamPos - pos);
            vec3 R = reflect(-L,N);

            float diff = max(dot(N,L),0.0);
            float spec = pow(max(dot(R,V),0.0), uShininess);

            finalRGB = uKa*base.rgb + uKd*diff*base.rgb + uKs*spec;
        }

        gl_FragDepth = clamp(s/uDepthFar,0.0,1.0);
        fragColor    = vec4(finalRGB, base.a);
    }else{
        fragColor    = vec4(0,0,0,1);
        gl_FragDepth = 1.0;
    }
}
