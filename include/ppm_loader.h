//
//  ppm_loader.h
//  410HW_00
//
//  Created by Enes Faruk Çona on 3.06.2025.
//
#ifndef PPM_LOADER_H
#define PPM_LOADER_H

#include <string>
#include <OpenGL/gl3.h>   // or <OpenGL/gl3.h> on macOS; match the rest of your project

/**
 * Reads an ASCII P3 PPM file and returns an OpenGL texture ID.
 * @param path          Full or relative path to the .ppm file
 * @param buildMipmaps  If true (default) glGenerateMipmap is called
 * @return GLuint       0 on failure, otherwise valid texture name
 */
GLuint loadPPMTexture(const std::string& path, bool buildMipmaps = true);

#endif // PPM_LOADER_H

