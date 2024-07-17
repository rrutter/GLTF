#ifndef GLTF2_H
#define GLTF2_H
#define GLM_ENABLE_EXPERIMENTAL
#define NOMINMAX

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "yyjson.h"
#include "GLTFAccessor.h"
#include "GLTFAnimation.h"
#include "GLTFNode.h"
#include "GLTFMesh.h"
#include "GLTFBuffer.h"
#include "GLTFMaterial.h"
#include <glm/gtx/string_cast.hpp>

#include "PersonalGL.h"
#include "Camera.h"
#include <unordered_map>
#include "GLTFSkeleton.h"

class GLTFLoader {
public:
    GLTFLoader();  // Default constructor
    ~GLTFLoader(); // Destructor

    void loadModel(const std::string& filepath);
    void printAnimationNames() const;
    void printMeshData();
    void printMaterialData();

    std::vector<glm::vec3> getPositions() const;
    std::vector<glm::vec3> getNormals() const;
    std::vector<glm::vec2> getTexcoords() const;
    std::vector<unsigned int> getIndices() const;
    // OpenGL rendering methods
    glm::mat4 getNodeTransform(const GLTFNode::Node& node) const;
    glm::mat4 getNodeHierarchyTransform(int nodeIndex) const;
    void initialize();
    void render();

    void setAnimation(const std::string& animationName);
    void updateAnimation(float deltaTime);

private:
    struct Buffer {
        std::vector<unsigned char> data;
    };

    struct GLBHeader {
        uint32_t magic;
        uint32_t version;
        uint32_t length;
    };

    struct PrimitiveBuffers {
        GLuint vao;
        GLuint vboPositions;
        GLuint vboNormals;
        GLuint vboTexCoords;
        GLuint eboIndices;
        GLuint vboColors;
        GLuint vboWeights;
        GLuint vboJoints;
        size_t indexCount;
        int materialIndex;
        glm::mat4 transform;
    };

    std::vector<Buffer> buffers;
    GLTFAccessor accessorManager;
    GLTFAnimation animationManager;
    GLTFNode nodeManager;
    GLTFMesh meshManager;
    GLTFBuffer bufferManager;
    GLTFMaterial materialManager;
    GLTFSkeleton skeleton;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
    std::unordered_map<int, GLuint> textureIDMap;

    std::string getFileExtension(const std::string& filepath);
    void loadGLBModel(const std::string& filepath);
    void loadGLTFModel(const std::string& filepath);
    void parseGLTF(yyjson_val* root, const std::string& basePath);
    void loadExternalBuffer(const std::string& uri, const std::string& basePath);
    void printGLBHeaderInfo(const GLBHeader& header);
    void printChunkInfo(uint32_t chunkLength, uint32_t chunkType, size_t chunkDataSize);

    // renderer private variables
    GLuint vao;
    GLuint vboPositions;
    GLuint vboNormals;
    GLuint vboTexCoords;
    GLuint eboIndices;
    GLuint shaderProgram;
    std::vector<PrimitiveBuffers> primitiveBuffers;

    void initBuffers();
    void setupVertexArrayObject(PrimitiveBuffers& buffers, const GLTFMesh::Primitive& primitive, int meshIndex);
    void checkVerts(const GLTFMesh::Primitive& primitive, int meshIndex);
    //void checkVerts(const GLTFMesh::Primitive& primitive);
    void initializeShaders();
    void initializeTextures();
};

#endif // GLTF2_H
