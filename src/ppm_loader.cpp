#include "ppm_loader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <cctype>

GLuint loadPPMTexture(const std::string& path, bool buildMipmaps)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[PPM] Cannot open " << path << '\n';
        return 0;
    }

    // ---------- 1. Read magic number ----------
    std::string token;
    file >> token;
    bool isP3 = (token == "P3");
    bool isP6 = (token == "P6");

    if (!isP3 && !isP6) {
        std::cerr << "[PPM] " << path << " is not a valid PPM (P3 or P6) format!\n";
        return 0;
    }

    // ---------- 2. Skip comments ----------
    auto skipComments = [&file]() {
        char c;
        while ((c = file.peek()) == '#') {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    };

    skipComments();
    int width, height, maxVal;
    file >> width >> height;
    skipComments();
    file >> maxVal;
    file.get();  // consume the newline after maxVal

    if (width <= 0 || height <= 0 || maxVal <= 0) {
        std::cerr << "[PPM] Invalid header in " << path << '\n';
        return 0;
    }

    // ---------- 3. Read pixel data ----------
    std::vector<GLubyte> pixels(width * height * 3);
    if (isP3) {
        for (size_t i = 0; i < pixels.size(); ++i) {
            int val;
            file >> val;
            pixels[i] = static_cast<GLubyte>((maxVal == 255) ? val : (val * 255 / maxVal));
        }
    } else if (isP6) {
        file.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
        if (maxVal != 255) {
            for (auto& px : pixels) {
                px = static_cast<GLubyte>(px * 255 / maxVal);
            }
        }
    }

    // ---------- 4. Upload to GPU ----------
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE,
                 pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (buildMipmaps) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    std::cout << "[PPM] Loaded " << path << " (" << width << "×" << height
              << ", " << (isP3 ? "P3 ASCII" : "P6 Binary") << ") as tex #" << texID << '\n';

    return texID;
}
