#ifndef GLTF_SKELETON_H
#define GLTF_SKELETON_H
#define GLM_ENABLE_EXPERIMENTAL

// Define constants for type checking
#define TYPE_SCALAR "SCALAR"
#define TYPE_VEC2 "VEC2"
#define TYPE_VEC3 "VEC3"
#define TYPE_VEC4 "VEC4"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <unordered_map>

#include "GLTFNode.h"
#include "GLTFMesh.h"
#include "GLTFAccessor.h"
#include "GLTFBuffer.h"
#include "Vertex.h"

class GLTFSkeleton {
public:

    struct Bone {
        int nodeIndex;
        int parentIndex; //parent bone index (not node)
        std::string name;
        glm::mat4 inverseBindMatrix;
        glm::mat4 globalTransform;
        glm::mat4 transform;
        std::vector<int> children;
    };

    GLTFSkeleton(const GLTFMesh& meshManager, GLTFNode& nodeManager, const GLTFAccessor& accessorManager, GLTFBuffer& bufferManager);

    const Bone& getBone(int index) const;
    const std::vector<Bone>& getBones() const;
    glm::mat4 getGlobalBoneTransform(int boneIndex) const;
    const std::vector<glm::mat4>& getJointMatrices() const;

    void updateSkeleton(const std::vector<GLTFNode::Node>& nodes);
   // void calculateJointMatrices();

    void printSkeleton() const;
    void loadVertices();
    const std::unordered_map<int, std::vector<Vertex>>& getVertices() const;
    void applySkinning();
    void validateJointIndices();
    void initializeSkeleton();

    void parseSkin(const auto skin);

    std::vector<glm::mat4> jointMatrices;

private:
    const GLTFMesh& meshManager;
    GLTFNode& nodeManager;
    const GLTFAccessor& accessorManager;
    GLTFBuffer& bufferManager;
    std::vector<Bone> bones;
    std::vector<glm::vec4> weights;

    std::vector<glm::mat4> inverseBindMatrices;
    void addBone(int nodeIndex, int parentIndex, const glm::mat4& inverseBindMatrix, const std::string& name);
    void loadInverseBindMatrices();
    void checkInverseBindMatrices();
    void calculateBoneTransforms();
    void addRootBones(const auto skin);
    void addChildBones(const auto skin);
    void normalizeWeights();
    const std::vector<glm::vec4>& getWeights() const;


    int boneIndex = 0;
    std::unordered_map<int, int> nodeToBoneIndex;
    std::vector<Vertex> vertices;
    std::unordered_map<int, std::vector<Vertex>> verticesPerMesh;
    const float tolerance = 1e-4f;
};

#endif // GLTF_SKELETON_H