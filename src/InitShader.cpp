#include "Angel.h"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace Angel {

GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader

    // Lambda function to read shader source code from a file
    auto readShaderSource = [](const char* filename) -> std::string {
        std::ifstream shaderFile(filename);
        if (!shaderFile.is_open()) {
            std::cerr << "Cannot open shader file: " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
        std::stringstream buffer;
        buffer << shaderFile.rdbuf(); // Read file contents into a string
        return buffer.str();
    };

    // Read and store shader source code
    std::string vertexSourceStr = readShaderSource(vertexShaderFile);
    std::string fragmentSourceStr = readShaderSource(fragmentShaderFile);
    const char* vertexSource = vertexSourceStr.c_str();
    const char* fragmentSource = fragmentSourceStr.c_str();

    // Load and compile the vertex shader
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Check for compilation errors
    GLint compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Vertex shader failed to compile" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Load and compile the fragment shader
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check for compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Fragment shader failed to compile" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create a shader program and attach shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram); // Link the shader program

    // Check for linking errors
    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Shader program failed to link" << std::endl;
        exit(EXIT_FAILURE);
    }

    return shaderProgram; // Return the compiled and linked shader program
}
GLuint InitShader2(const char* vertexShaderFile,
                   const char* fragmentShaderFile)
{
    auto readFile = [](const char* path)->std::string {
        std::ifstream f(path, std::ios::in | std::ios::binary);
        if(!f) { std::cerr << "Cannot open shader: " << path << '\n'; exit(EXIT_FAILURE); }
        std::ostringstream ss; ss << f.rdbuf(); return ss.str();
    };

    const std::string vsrcStr = readFile(vertexShaderFile);
    const std::string fsrcStr = readFile(fragmentShaderFile);
    const char* vsrc = vsrcStr.c_str();
    const char* fsrc = fsrcStr.c_str();

    auto compile = [](GLenum type, const char* src, const char* name)->GLuint {
        GLuint id = glCreateShader(type);
        glShaderSource(id,1,&src,nullptr);
        glCompileShader(id);
        GLint ok; glGetShaderiv(id,GL_COMPILE_STATUS,&ok);
        if(!ok){
            GLint len; glGetShaderiv(id,GL_INFO_LOG_LENGTH,&len);
            std::vector<char> log(len);
            glGetShaderInfoLog(id,len,nullptr,log.data());
            std::cerr << "Error compiling "<< name <<":\n"<< log.data() <<'\n';
            exit(EXIT_FAILURE);
        }
        return id;
    };

    GLuint vs = compile(GL_VERTEX_SHADER  , vsrc, vertexShaderFile);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc, fragmentShaderFile);

    GLuint prog = glCreateProgram();
    glAttachShader(prog,vs); glAttachShader(prog,fs);
    glLinkProgram(prog);

    GLint linked; glGetProgramiv(prog,GL_LINK_STATUS,&linked);
    if(!linked){
        GLint len; glGetProgramiv(prog,GL_INFO_LOG_LENGTH,&len);
        std::vector<char> log(len);
        glGetProgramInfoLog(prog,len,nullptr,log.data());
        std::cerr << "Link error:\n"<< log.data() <<'\n';
        exit(EXIT_FAILURE);
    }
    return prog;
}
}
