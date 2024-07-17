#include "PersonalGL.h"

GLuint gTextureID = 0;
GLuint gNormalMapID = 1;
GLuint gVertexShader;
GLuint gFragmentShader;
bool gUseShaders = true;

LoadPNG png;
extern unsigned int glTextureIndexId;

void checkGLError(const std::string& context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << context << ": " << gluErrorString(err) << std::endl;
    }
}

void PersonalGL::glScale(float scale) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(scale, scale, 1);
    glMatrixMode(GL_MODELVIEW);
}

void PersonalGL::glMultiply(float multiple) {
    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, multiple);
}

unsigned int PersonalGL::glCreateTex(const char* filename) {
    id++;
    unsigned int colorMode;
    std::unique_ptr<unsigned char[]> data(png.loadFile(filename));

    if (!data) {
        throw std::runtime_error("Failed to load PNG file: " + std::string(filename));
    }

    if (png.getColorType() == 6) {
        colorMode = GL_RGBA;
    }
    else {
        colorMode = GL_RGB;
    }

    glGenTextures(1, &glTextureIndexId);
    checkGLError("glGenTextures");
    glBindTexture(GL_TEXTURE_2D, glTextureIndexId);
    checkGLError("glBindTexture");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkGLError("glTexParameteri MAG_FILTER");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGLError("glTexParameteri MIN_FILTER");

    if (png.getHeight() == png.getWidth()) {
        glTexImage2D(GL_TEXTURE_2D, 0, colorMode, png.getWidth(), png.getHeight(), 0, colorMode, GL_UNSIGNED_BYTE, data.get());
        checkGLError("glTexImage2D");
    }
    else {
        gluBuild2DMipmaps(GL_TEXTURE_2D, colorMode, png.getWidth(), png.getHeight(), colorMode, GL_UNSIGNED_BYTE, data.get());
        checkGLError("gluBuild2DMipmaps");
    }

    return glTextureIndexId;
}

unsigned char* PersonalGL::readShaderFile(const char* fileName) {
    std::ifstream file(fileName, std::ios::binary);

    // Try to open the specified shader file
    if (!file) {
        std::cerr << "Cannot open shader file: " << fileName << ". Loading fallback shader." << std::endl;
        file.open("shaders/basic_vshader.glsl", std::ios::binary);

        // If the fallback shader file cannot be opened, throw an exception
        if (!file) {
            throw std::runtime_error("Cannot open fallback shader file: shaders/basic_shader.glsl");
        }
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    unsigned char* buffer = new unsigned char[fileSize + 1];
    file.read(reinterpret_cast<char*>(buffer), fileSize);
    buffer[fileSize] = '\0';

    return buffer;
}

GLuint PersonalGL::compileShader(const char* filePath, GLenum shaderType) {
    unsigned char* shaderSource = readShaderFile(filePath);
    if (!shaderSource) {
        throw std::runtime_error("Failed to read shader file: " + std::string(filePath));
    }

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, reinterpret_cast<const GLchar* const*>(&shaderSource), nullptr);
    glCompileShader(shader);
    delete[] shaderSource;

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        throw std::runtime_error("Shader compilation error (" + std::string(filePath) + "): " + std::string(infoLog));
    }

    return shader;
}

GLuint PersonalGL::createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader linking error: " << infoLog << std::endl;
        throw std::runtime_error("Shader linking failed");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int PersonalGL::loadOpenGLFunctions() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return 0;
    }

    int numTextureUnits = 0;
    std::cout << "Getting Max Texture Units..." << std::endl;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &numTextureUnits);

    std::cout << "Getting GL Extensions...." << std::endl;
    const char* ext = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

    std::cout << "Checking if Extensions are NULL.." << std::endl;
    if (strstr(ext, "GL_ARB_multitexture") == NULL) {
        MessageBoxA(NULL, "GL_ARB_multitexture extension was not found", "ERROR", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    // No need to manually load functions with GLEW
    return 1;
}

void PersonalGL::checkGLError(const std::string& location) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << location << ": " << err << std::endl;
    }
}