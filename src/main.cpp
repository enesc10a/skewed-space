#include "Angel.h"
#include "sphere.h"
#include "callbacks.h"
#include "ppm_loader.h"
#include <vector>
#include "globals.h"
// camera globals
extern vec3 cameraPos, cameraFront, cameraUp;
extern float cameraSpeed;

// scene globals
static std::vector<Sphere*> spheres;
static GLuint textures[3]  = {0,0,0};
GLuint shaderProgram = 0;

// set up common uniforms (your existing code)
static void setCommonUniforms(const mat4& V, const mat4& P) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),       1, GL_TRUE, V);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_TRUE, P);
    glUniform1i  (glGetUniformLocation(shaderProgram, "uShadingMode"),    1);
    glUniform1i  (glGetUniformLocation(shaderProgram, "uTexturing"),      1);
    glUniform3f  (glGetUniformLocation(shaderProgram, "uKa"),   0.1f,0.1f,0.1f);
    glUniform3f  (glGetUniformLocation(shaderProgram, "uKd"),   0.8f,0.8f,0.8f);
    glUniform3f  (glGetUniformLocation(shaderProgram, "uKs"),   1.0f,1.0f,1.0f);
    glUniform1f  (glGetUniformLocation(shaderProgram, "uShininess"), 32.0f);
    glUniform1i  (glGetUniformLocation(shaderProgram, "uUseAmb"),  1);
    glUniform1i  (glGetUniformLocation(shaderProgram, "uUseDiff"), 1);
    glUniform1i  (glGetUniformLocation(shaderProgram, "uUseSpec"), 1);
}

static void initScene() {
    shaderProgram = InitShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    glUseProgram(shaderProgram);

    // load textures
    textures[0] = loadPPMTexture("resources/sun.ppm");
    textures[1] = loadPPMTexture("resources/earth.ppm");
    textures[2] = loadPPMTexture("resources/moon.ppm");

    spheres.reserve(3);

    // Sun: heavy, almost stationary at origin
    Sphere::Properties sunProps{
        2.0f,                   // radius
        vec3(0.0f, 0.0f, 0.0f), // position
        vec3(0.0f),             // velocity
        vec3(0.0f, 1.0f, 0.0f), // rotation axis
        5.0f,                   // rotation speed (deg/sec)
        10000.0f                // mass
    };

    // Earth: orbits Sun roughly in a circle
    Sphere::Properties earthProps{
        1.0f,                   // radius
        vec3(20.0f, 0.0f, 0.0f),// position 20 units on +X
        vec3(0.0f, 0.0f, 15.0f),// initial tangential velocity
        vec3(0.0f, 1.0f, 0.0f), // rotation axis
        30.0f,                  // rotation speed
        1.0f                    // mass
    };

    // Moon: small, orbits Earth
    Sphere::Properties moonProps{
        0.5f,                   // radius
        vec3(25.0f, 0.0f, 0.0f),// 5 units beyond Earth
        vec3(0.0f, 0.0f, 18.0f),// slightly faster tangential velocity
        vec3(0.0f, 1.0f, 0.0f), // rotation axis
        60.0f,                  // rotation speed
        0.01f                    // mass
    };

    // instantiate and pass neighbor list for force calculations
    spheres.push_back(new Sphere(20, sunProps,   &spheres));
    spheres.push_back(new Sphere(20, earthProps, &spheres));
    spheres.push_back(new Sphere(20, moonProps,  &spheres));

    glEnable(GL_DEPTH_TEST);
}

static void display(float deltaTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Physics update (unchanged)
    const float Gscale = 1.0f;
    for (auto* s : spheres) s->computeNetForce();
    for (auto* s : spheres) s->integrate(deltaTime, Gscale);

    // 2. Build camera matrices
    vec4 eye4 = vec4(cameraPos, 1.0f);
    vec4 at4  = vec4(cameraPos + cameraFront, 1.0f);
    vec4 up4  = vec4(cameraUp, 0.0f);
    mat4 V    = LookAt(eye4, at4, up4);

    int w, h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);
    mat4 P    = Perspective(45.0f, float(w)/float(h), 0.1f, 100.0f);

    setCommonUniforms(V, P);

    // 3. Compute Sun→eye‐space once
    //    (we’ll reuse to compute per-object directional)
    vec3 sunWorld = spheres[0]->getProperties().position;
    vec4 sunEye4  = V * vec4(sunWorld, 1.0f);
    vec3 sunEye   = vec3(sunEye4.x, sunEye4.y, sunEye4.z);

    // locate the uniform once
    GLint lightDirLoc = glGetUniformLocation(shaderProgram, "uLightDir");
    GLint modelLoc    = glGetUniformLocation(shaderProgram, "model");

    // 4. Draw each sphere with its own uLightDir
    for (size_t i = 0; i < spheres.size(); ++i) {
        auto& P = spheres[i]->getProperties();

        // compute world→Sun direction for *this* sphere
        vec3 dirWorld = sunWorld - P.position;
        // transform to eye space (w=0 so no translation)
        vec4 dirEye4 = V * vec4(dirWorld, 0.0f);
        vec3 dirEye  = normalize(vec3(dirEye4.x, dirEye4.y, dirEye4.z));

        // upload that as your single directional light
        glUniform3fv(lightDirLoc, 1, &dirEye.x);

        // model matrix
        mat4 M = Translate(P.position) * Scale(P.radius, P.radius, P.radius);
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, M);

        // bind texture + draw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        spheres[i]->draw();
    }
}

int main() {
    if (!glfwInit()) return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    // create & store the window pointer
    GLFWwindow* win = glfwCreateWindow(800, 600, "Free Roam Spheres", nullptr, nullptr);
    if (!win) { glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(win);

    // input callbacks
    glfwSetKeyCallback(win, keyCallback);
    glfwSetFramebufferSizeCallback(win, framebufferSizeCallback);

    initScene();

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(win)) {
        double now  = glfwGetTime();
        float  dt   = float(now - lastTime);
        lastTime    = now;
        dt=dt*0.8;
        display(dt);
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    for (auto* s : spheres) delete s;
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
