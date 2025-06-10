#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <GLFW/glfw3.h>

// ---------------- existing ----------------
enum ShapeType { CUBE, SPHERE, BUNNY, NUM_SHAPES };
enum DrawMode  { SOLID, WIREFRAME };

extern ShapeType currentShape;
extern DrawMode  currentDrawMode;
extern bool      colorToggle;

// ---------------- new for HW-3 ------------

// rendering “view” (wireframe / shaded / textured)

enum ShadingMode { GOURAUD = 0, PHONG = 1 };
enum Material    { PLASTIC  = 0, METAL  = 1 };
enum DisplayMode { DISP_WIREFRAME = 0, DISP_SHADED = 1, DISP_TEXTURED = 2 };


extern DisplayMode currentDisplay;
extern ShadingMode currentShading;
extern Material    currentMaterial;

extern bool useAmbient, useDiffuse, useSpecular;
extern bool lightFollowsObject;

extern float zoom;            // perspective zoom factor (1.0 = default)
extern int   currentTexture;  // index into texture ID array

// ------------------------------------------

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPosCallback(GLFWwindow*, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

#endif
