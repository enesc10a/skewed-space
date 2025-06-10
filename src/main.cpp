//=============================================================================
//  main.cpp  —  Free-Roam Spheres (GPU Ray-traced Version)
//=============================================================================
#include "Angel.h"
#include "sphere.h"
#include "callbacks.h"
#include "ppm_loader.h"
#include <vector>
#include "globals.h"

// ─────────────────────────────────────────────────────────────────────────────
//  GLOBALS
// ─────────────────────────────────────────────────────────────────────────────
extern vec3  cameraPos, cameraFront, cameraUp;     // provided by callbacks
extern float cameraSpeed;

static std::vector<Sphere*> spheres;               // physics objects
static GLuint textures[3] = {0,0,0};               // sun / earth / moon

static GLuint rayShader   = 0;                     // raytracer.glsl program
static GLuint fsVAO       = 0;                     // full-screen triangle
static float  lightSpeed  = 5.1f;                  // tweak with keys
static float  gravityK    = 0.01f;
static float uHitEps=0.1;
static float uDepthFar=50;

GLuint shaderProgram=0;

// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS — build full-screen triangle & simple VS
// ─────────────────────────────────────────────────────────────────────────────
static void createFullScreenQuad()
{
    float tri[6] = { -1,-1,  3,-1,  -1, 3 };
    glGenVertexArrays(1,&fsVAO);
    glBindVertexArray(fsVAO);
    GLuint vbo;  glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(tri),tri,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
}

static GLuint compileRayProgram()
{
    return InitShader("shaders/screen.vert", "shaders/raytracer.glsl");
   // the fragment shader you wrote
}

// ─────────────────────────────────────────────────────────────────────────────
//  INITIALISATION
// ─────────────────────────────────────────────────────────────────────────────
static void initScene()
{
    rayShader = compileRayProgram();
    createFullScreenQuad();

    textures[0] = loadPPMTexture("resources/sun.ppm");
    textures[1] = loadPPMTexture("resources/earth.ppm");
    textures[2] = loadPPMTexture("resources/moon.ppm");

    spheres.reserve(3);

    spheres.push_back(new Sphere(20,{ 2.0f, { 0,0,0 }, {0}, {0,1,0}, 5.0f, 10000.0f }, &spheres));
    spheres.push_back(new Sphere(20,{ 1.0f, {20,0,0}, {0,0,15}, {0,1,0}, 30.0f, 1.0f }, &spheres));
    spheres.push_back(new Sphere(20,{ 0.5f, {25,0,0}, {0,0,18}, {0,1,0}, 60.0f, 0.01f}, &spheres));

    glEnable(GL_DEPTH_TEST);
}

// ─────────────────────────────────────────────────────────────────────────────
//  PER-FRAME UNIFORM UPLOAD  (for the ray shader)
// ─────────────────────────────────────────────────────────────────────────────
static void uploadRayUniforms(int w,int h,const mat4& invView)
{
    glUseProgram(rayShader);
    glUniform2f (glGetUniformLocation(rayShader,"uResolution"), w, h);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamPos"),1,&cameraPos.x);
    glUniformMatrix4fv(glGetUniformLocation(rayShader,"uCamInvView"),1,GL_TRUE,invView);

    
    vec3 front = normalize(cameraFront);
    vec3 right = normalize(cross(front, cameraUp));
    vec3 up    = normalize(cameraUp);

    glUniform3fv(glGetUniformLocation(rayShader,"uCamRight"),   1, &right.x);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamUp"),      1, &up.x);
    glUniform3fv(glGetUniformLocation(rayShader,"uCamForward"), 1, &front.x);
    glUniform1f (glGetUniformLocation(rayShader,"uAspect"), float(w)/h);
    
    
    glUniform1f(glGetUniformLocation(rayShader,"uFovRad"),
                45.0f * Angel::DegreesToRadians);
    glUniform1f(glGetUniformLocation(rayShader,"uWorldLimit"), 200.0f);
    glUniform1f(glGetUniformLocation(rayShader,"uLightSpeed"), lightSpeed);
    glUniform1f(glGetUniformLocation(rayShader,"uGravityScale"), gravityK);

    glUniform1i(glGetUniformLocation(rayShader,"uSphereCount"), spheres.size());
    
    glUniform1f(glGetUniformLocation(rayShader,"uHitEps"),   0.02f);   // or tweak
    glUniform1f(glGetUniformLocation(rayShader,"uDepthFar"), 300.0f);  // map s→depth
    glUniform1f(glGetUniformLocation(rayShader, "uAspect"), float(w) / h);


    for(int i=0;i<spheres.size();++i){
        const auto& P = spheres[i]->getProperties();
        std::string base = "uSpherePos["  + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(rayShader, base.c_str()),1,&P.position.x);

        base = "uSphereRad["  + std::to_string(i) + "]";
        glUniform1f(glGetUniformLocation(rayShader, base.c_str()), P.radius);

        base = "uSphereMass[" + std::to_string(i) + "]";
        glUniform1f(glGetUniformLocation(rayShader, base.c_str()), P.mass);

        base = "uSphereTex["  + std::to_string(i) + "]";
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(glGetUniformLocation(rayShader, base.c_str()), i);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DISPLAY
// ─────────────────────────────────────────────────────────────────────────────
static void display(float dt)
{
    // ── physics (unchanged) ────────────────────────────────────────────────
    const float Gs = 1.0f;
    for(auto* s: spheres) s->computeNetForce();
    for(auto* s: spheres) s->integrate(dt, Gs);

    // ── camera matrices (for inverse-view) ────────────────────────────────
    int w, h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

    mat4 V = LookAt(vec4(cameraPos,1),
                    vec4(cameraPos + cameraFront,1),
                    vec4(cameraUp, 0));

    // ── G-buffer clear & ray pass ─────────────────────────────────────────
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    uploadRayUniforms(w,h, V);

    glBindVertexArray(fsVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    if(!glfwInit()) return EXIT_FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800,600,"Free-Roam Spheres",nullptr,nullptr);
    if(!win){ glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(win);

    glfwSetKeyCallback(win, keyCallback);
    glfwSetFramebufferSizeCallback(win, framebufferSizeCallback);

    initScene();

    double last = glfwGetTime();
    while(!glfwWindowShouldClose(win)){
        double now = glfwGetTime();
        float  dt  = float(now-last)*0.8f;  last = now;

        display(dt);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    for(auto* s: spheres) delete s;
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
