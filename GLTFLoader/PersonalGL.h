#pragma once
#ifndef PERSONAL_GL_H
#define PERSONAL_GL_H

#include <GL/glew.h>

#include <windows.h>
#include "Loadpng.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>



class PersonalGL
{
public:
    unsigned int id = 0; // Initialize id to 0
    void glScale(float scale);
    void glMultiply(float multiple);
    unsigned int glCreateTex(const char* filename);
    int loadOpenGLFunctions();
    static void checkGLError(const std::string& location);
    GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

private:
    unsigned char* readShaderFile(const char* fileName);
    GLuint compileShader(const char* filePath, GLenum shaderType);
};

#endif // PERSONAL_GL_H