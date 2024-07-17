#ifndef GLTF_NODE_H
#define GLTF_NODE_H
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "yyjson.h"

class GLTFNode {
public:
    struct Node {
        glm::vec3 translation;
        glm::quat rotation;
        glm::vec3 scale;
        int meshIndex;
        int index;
        std::vector<int> children;
        std::string name;
        bool isRoot;
        bool isBone;
        glm::mat4 transformation = glm::mat4(1.0f);
        yyjson_val* extensions = nullptr;
        yyjson_val* extras = nullptr;
    };

    void parseNodes(yyjson_val* nodesArray);
    void identifyRootNodes();
    void identifyBones(const std::vector<int>& joints);
    std::vector<GLTFNode::Node>& getNodes();
    const std::vector<GLTFNode::Node>& getNodes() const;
    glm::mat4 getNodeTransform(const Node& node) const;
    int findParentNodeIndex(int nodeIndex) const;
    std::vector<int> getRootNodes() const;
    std::vector<int> getChildNodes(int nodeIndex) const;
    glm::mat4 getGlobalTransform(int nodeIndex) const;
    void updateNodeTransformation(Node& node);

    // New getter and setter methods
    glm::vec3 getNodeTranslation(int nodeIndex) const;
    void setNodeTranslation(int nodeIndex, const glm::vec3& translation);
    glm::quat getNodeRotation(int nodeIndex) const;
    void setNodeRotation(int nodeIndex, const glm::quat& rotation);
    glm::vec3 getNodeScale(int nodeIndex) const;
    void setNodeScale(int nodeIndex, const glm::vec3& scale);

private:
    std::vector<Node> nodes;
    void printNodeInfo(const Node& node, size_t index) const;
    bool showDebug = false;
};

#endif // GLTF_NODE_H