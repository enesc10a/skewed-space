// main.cpp  —  Free-Roam Spheres (GPU Ray-traced Version) – Multi-World, Reset & Self‐Rotation
#include "Angel.h"
#include "sphere.h"
#include "callbacks.h"
#include "ppm_loader.h"
#include <vector>
#include <GLFW/glfw3.h>
#include <string>

// Camera globals
static Angel::vec3 cameraPosOrig(0.0f, 5.0f, -30.0f);
static Angel::vec3 cameraFrontOrig(0.0f, 0.0f, -1.0f);
static Angel::vec3 cameraUpOrig(0.0f, 1.0f, 0.0f);

Angel::vec3 cameraPos   = cameraPosOrig;
Angel::vec3 cameraFront = cameraFrontOrig;
Angel::vec3 cameraUp    = cameraUpOrig;
float cameraSpeed = 80.0f;

static std::vector<Sphere*> spheres;
static GLuint textures[8] = {0}, texSun = 0, texEarth = 0, texMoon = 0,texNeutron=0,texRedSun=0;
static GLuint rayShader = 0, fsVAO = 0;

// Simulation params
static float lightSpeed = 18.1f;
static float gravityK   = 0.05f;

// World selector
static int currentWorld = 0;
static const int WORLD_COUNT = 4;

// ─────────────────────────────────────────────────────────────────────────────
static void createFullScreenQuad() {
    float tri[6] = { -1,-1, 3,-1, -1,3 };
    glGenVertexArrays(1, &fsVAO);
    glBindVertexArray(fsVAO);
    GLuint vbo; glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
}

static GLuint compileRayProgram() {
    return InitShader("shaders/screen.vert", "shaders/raytracer.glsl");
}

static void loadWorld(int w) {
    spheres.clear();
    // textures
    switch(w) {
        case 0: textures[0]=texSun; textures[1]=texEarth; textures[2]=texMoon; break;
        case 1: textures[0]=texSun; textures[1]=texNeutron;   textures[2]=texEarth; break;
        case 2: textures[0]=texSun; for(int i=1;i<=4;++i) textures[i]=texEarth; break;
        case 3: textures[0]=texSun; textures[1]=texNeutron; textures[2]=texRedSun; break;
    }
    // spheres
    switch(w) {
        case 0:
            spheres.push_back(new Sphere(20,{2.0f,{0,0,0},{0},{0,1,0},5.0f,10000.0f},&spheres));
            spheres.push_back(new Sphere(20,{1.0f,{20,0,0},{0,0,15},{0,1,0},30.0f,1.0f},&spheres));
            spheres.push_back(new Sphere(20,{0.5f,{25,0,0},{0,0,18},{0,1,0},60.0f,0.01f},&spheres));
            break;
        case 1: {
            // two suns in a tighter binary, Earth in a stable circumbinary orbit
            const float a     = 5.0f;                             // half‐separation
            const float M     = 8000.0f;                          // each sun’s mass
            const float vs    = sqrt(M / (4.0f * a));             // binary orbital speed ≈20
            const float vdiag = vs / sqrt(2.0f);                  // components for diagonal motion

            // Sun A at (−a,−a,0), velocity perpendicular in XY plane
            spheres.push_back(new Sphere(
                20,
                { 2.5f, { -a, -a, 0 }, {  vdiag, 0.0f, -vdiag }, { 0,1,0 }, 5.0f, M },
                &spheres
            ));
            // Sun B at (+a,+a,0), opposite velocity
            spheres.push_back(new Sphere(
                20,
                { 2.5f, {  a,  a, 0 }, { -vdiag, 0.0f,  vdiag }, { 0,1,0 }, 5.0f, M },
                &spheres
            ));

            // Earth in a circumbinary orbit at ~4× the binary half‐separation
            const float Re = 4.0f * a;                           // =20
            const float ve = sqrt(2.0f * M / Re);                // ≈28.3
            spheres.push_back(new Sphere(
                20,
                { 1.0f, { Re, 0.0f, 0.0f }, { 0.0f, 0.0f, ve }, { 0,1,0 }, 30.0f, 1.0f },
                &spheres
            ));
        } break;
        case 2:
            spheres.push_back(new Sphere(20,{3.0f,{0,0,0},{0},{0,1,0},5.0f,10000.0f},&spheres));
            { const float R=20.0f,v=12.0f; float ang[4]={0,1.5708f,3.1416f,4.7124f};
              for(int i=0;i<4;++i){
                float x=R*cos(ang[i]), z=R*sin(ang[i]);
                spheres.push_back(new Sphere(20,{1.0f,{x,0,z},{ v*z/R,0,-v*x/R},{0,1,0},30.0f,1.0f},&spheres));
              }
            } break;
        case 3: {
            const float R=15.0f,v=22.0f;
            float posX[3]={R,-R*0.5f,-R*0.5f}, posZ[3]={0,R*0.866f,-R*0.866f};
            for(int i=0;i<3;++i){
                Angel::vec3 p={posX[i],0,posZ[i]};
                Angel::vec3 tang=normalize(cross({0,1,0},p));
                Angel::vec3 vel=tang*v;
                spheres.push_back(new Sphere(20,{2.5f,p,{vel.x,vel.y,vel.z},{0,1,0},5.0f,9000.0f},&spheres));
            }
        } break;
    }
}

static void initScene() {
    rayShader = compileRayProgram();
    createFullScreenQuad();
    texSun   = loadPPMTexture("resources/sun.ppm");
    texEarth = loadPPMTexture("resources/earth.ppm");
    texMoon  = loadPPMTexture("resources/moon.ppm");
    texNeutron = loadPPMTexture("resources/neutron_star.ppm");
    texRedSun = loadPPMTexture("resources/red_sun.ppm");
    glEnable(GL_DEPTH_TEST);
    loadWorld(currentWorld);
}

static void uploadRayUniforms(int w,int h,const Angel::mat4& invView){
    glUseProgram(rayShader);

    // time‐based orientation
    float t = static_cast<float>(glfwGetTime());
    glUniform1f(glGetUniformLocation(rayShader,"uTime"), t);

    glUniform2f(glGetUniformLocation(rayShader,"uResolution"),w,h);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamPos"),1,&cameraPos.x);
    glUniformMatrix4fv(glGetUniformLocation(rayShader,"uCamInvView"),1,GL_TRUE,invView);

    Angel::vec3 f=normalize(cameraFront), r=normalize(cross(f,cameraUp)), u=normalize(cameraUp);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamRight"),1,&r.x);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamUp"),1,&u.x);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamForward"),1,&f.x);

    glUniform1f(glGetUniformLocation(rayShader,"uAspect"),float(w)/float(h));
    glUniform1f(glGetUniformLocation(rayShader,"uFovRad"),45.0f*Angel::DegreesToRadians);
    glUniform1f(glGetUniformLocation(rayShader,"uWorldLimit"),300.0f);
    glUniform1f(glGetUniformLocation(rayShader,"uLightSpeed"),lightSpeed);
    glUniform1f(glGetUniformLocation(rayShader,"uGravityScale"),gravityK);

    glUniform1i(glGetUniformLocation(rayShader,"uSphereCount"),spheres.size());
    glUniform1f(glGetUniformLocation(rayShader,"uHitEps"),0.02f);
    glUniform1f(glGetUniformLocation(rayShader,"uDepthFar"),300.0f);

    int sunCount=0, sunIdx[8]={0};
    switch(currentWorld){
      case 0: sunCount=1; break;
      case 1: sunCount=2; sunIdx[1]=1; break;
      case 2: sunCount=1; break;
      case 3: sunCount=3; sunIdx[1]=1; sunIdx[2]=2; break;
    }
    glUniform1i(glGetUniformLocation(rayShader,"uSunCount"),sunCount);
    glUniform1iv(glGetUniformLocation(rayShader,"uSunIndices"),sunCount,sunIdx);

    glUniform3f(glGetUniformLocation(rayShader,"uKd"),0.8f,0.8f,0.8f);
    glUniform3f(glGetUniformLocation(rayShader,"uKs"),1.0f,1.0f,1.0f);
    glUniform1f(glGetUniformLocation(rayShader,"uShininess"),32.0f);

    for(int i=0;i<(int)spheres.size();++i){
        auto P=spheres[i]->getProperties();
        std::string s;
        s="uSpherePos["+std::to_string(i)+"]"; glUniform3fv(glGetUniformLocation(rayShader,s.c_str()),1,&P.position.x);
        s="uSphereRad["+std::to_string(i)+"]"; glUniform1f (glGetUniformLocation(rayShader,s.c_str()),P.radius);
        s="uSphereMass["+std::to_string(i)+"]";glUniform1f (glGetUniformLocation(rayShader,s.c_str()),P.mass);
        s="uSphereTex["+std::to_string(i)+"]";
        glActiveTexture(GL_TEXTURE0+i); glBindTexture(GL_TEXTURE_2D,textures[i]);
        glUniform1i(glGetUniformLocation(rayShader,s.c_str()),i);

        // minimal self-rotation uniforms
        std::string ax="uRotationAxis["+std::to_string(i)+"]";
        std::string sp="uRotationSpeed["+std::to_string(i)+"]";
        glUniform3fv(glGetUniformLocation(rayShader,ax.c_str()),1,&P.rotationAxis.x);
        glUniform1f (glGetUniformLocation(rayShader,sp.c_str()),P.rotationSpeed);
    }
}

static void display(const Angel::mat4& invView,int w,int h){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    uploadRayUniforms(w,h,invView);
    glBindVertexArray(fsVAO);
    glDrawArrays(GL_TRIANGLES,0,3);
}

int main(){
    if(!glfwInit())return EXIT_FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
    glfwWindowHint(GLFW_OPENGL_PROFILE,   GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win=glfwCreateWindow(800,600,"Free-Roam Spheres",nullptr,nullptr);
    if(!win){glfwTerminate();return EXIT_FAILURE;}
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win,framebufferSizeCallback);

    initScene();

    bool prevL=false, prevR=false, prevX=false;
    double last=glfwGetTime();
    while(!glfwWindowShouldClose(win)){
        double now=glfwGetTime();
        float dt=float(now-last);
        last=now;

        // camera
        Angel::vec3 f=normalize(cameraFront), r=normalize(cross(f,cameraUp));
        if(glfwGetKey(win,GLFW_KEY_W)==GLFW_PRESS)cameraPos+=f*cameraSpeed*dt;
        if(glfwGetKey(win,GLFW_KEY_S)==GLFW_PRESS)cameraPos-=f*cameraSpeed*dt;
        if(glfwGetKey(win,GLFW_KEY_A)==GLFW_PRESS)cameraPos-=r*cameraSpeed*dt;
        if(glfwGetKey(win,GLFW_KEY_D)==GLFW_PRESS)cameraPos+=r*cameraSpeed*dt;
        if(glfwGetKey(win,GLFW_KEY_Q)==GLFW_PRESS)cameraPos+=cameraUp*cameraSpeed*dt;
        if(glfwGetKey(win,GLFW_KEY_E)==GLFW_PRESS)cameraPos-=cameraUp*cameraSpeed*dt;

        // world switch + reset
        bool cl=glfwGetKey(win,GLFW_KEY_LEFT)==GLFW_PRESS;
        bool cr=glfwGetKey(win,GLFW_KEY_RIGHT)==GLFW_PRESS;
        bool cR=glfwGetKey(win,GLFW_KEY_R)==GLFW_PRESS;
        if(cl&&!prevL){currentWorld=(currentWorld+WORLD_COUNT-1)%WORLD_COUNT;loadWorld(currentWorld);}
        if(cr&&!prevR){currentWorld=(currentWorld+1)%WORLD_COUNT;loadWorld(currentWorld);}
        if(cR&&!prevX){cameraPos=cameraPosOrig;cameraFront=cameraFrontOrig;cameraUp=cameraUpOrig;loadWorld(currentWorld);}
        prevL=cl;prevR=cr;prevX=cR;

        int w,h;glfwGetFramebufferSize(win,&w,&h);if(!h)h=1;if(!w)w=1;glViewport(0,0,w,h);
        Angel::mat4 V=LookAt(vec4(cameraPos,1),vec4(cameraPos+cameraFront,1),vec4(cameraUp,0));
        Angel::mat4 invV;for(int i=0;i<3;i++)for(int j=0;j<3;j++)invV[i][j]=V[j][i];
        Angel::vec3 t(V[0][3],V[1][3],V[2][3]);
        invV[0][3]=-(invV[0][0]*t.x+invV[0][1]*t.y+invV[0][2]*t.z);
        invV[1][3]=-(invV[1][0]*t.x+invV[1][1]*t.y+invV[1][2]*t.z);
        invV[2][3]=-(invV[2][0]*t.x+invV[2][1]*t.y+invV[2][2]*t.z);
        invV[3][0]=invV[3][1]=invV[3][2]=0;invV[3][3]=1;

        for(auto*s:spheres)s->computeNetForce();
        for(auto*s:spheres)s->integrate(dt,1.0f);

        display(invV,w,h);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    for(auto*s:spheres)delete s;
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
