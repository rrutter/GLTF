#include "GLTFNode.h"
#include <iostream>

void GLTFNode::parseNodes(yyjson_val* nodesArray) {
    size_t idx, max;
    yyjson_val* node_val;
    yyjson_arr_foreach(nodesArray, idx, max, node_val) {
        Node node;

        node.translation = glm::vec3(0.0f);
        node.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        node.scale = glm::vec3(1.0f);
        node.meshIndex = -1;
        node.isRoot = true; // Initialize as root, will be corrected later
        node.index = static_cast<int>(idx); // Assign index here

        yyjson_val* translation_val = yyjson_obj_get(node_val, "translation");
        if (translation_val && yyjson_is_arr(translation_val)) {
            node.translation.x = yyjson_get_real(yyjson_arr_get(translation_val, 0));
            node.translation.y = yyjson_get_real(yyjson_arr_get(translation_val, 1));
            node.translation.z = yyjson_get_real(yyjson_arr_get(translation_val, 2));
        }

        yyjson_val* rotation_val = yyjson_obj_get(node_val, "rotation");
        if (rotation_val && yyjson_is_arr(rotation_val)) {
            node.rotation.x = yyjson_get_real(yyjson_arr_get(rotation_val, 0));
            node.rotation.y = yyjson_get_real(yyjson_arr_get(rotation_val, 1));
            node.rotation.z = yyjson_get_real(yyjson_arr_get(rotation_val, 2));
            node.rotation.w = yyjson_get_real(yyjson_arr_get(rotation_val, 3));
        }

        yyjson_val* scale_val = yyjson_obj_get(node_val, "scale");
        if (scale_val && yyjson_is_arr(scale_val)) {
            node.scale.x = yyjson_get_real(yyjson_arr_get(scale_val, 0));
            node.scale.y = yyjson_get_real(yyjson_arr_get(scale_val, 1));
            node.scale.z = yyjson_get_real(yyjson_arr_get(scale_val, 2));

            // Correct zero scaling values
            if (node.scale.x == 0.0f) node.scale.x = 1.0f;
            if (node.scale.y == 0.0f) node.scale.y = 1.0f;
            if (node.scale.z == 0.0f) node.scale.z = 1.0f;
        }


        yyjson_val* mesh_val = yyjson_obj_get(node_val, "mesh");
        if (mesh_val) {
            node.meshIndex = yyjson_get_int(mesh_val);
        }

        yyjson_val* children_val = yyjson_obj_get(node_val, "children");
        if (children_val && yyjson_is_arr(children_val)) {
            size_t child_idx, child_max;
            yyjson_val* child_val;
            yyjson_arr_foreach(children_val, child_idx, child_max, child_val) {
                node.children.push_back(yyjson_get_int(child_val));
            }
        }

        yyjson_val* name_val = yyjson_obj_get(node_val, "name");
        if (name_val && yyjson_is_str(name_val)) {
            node.name = yyjson_get_str(name_val);
        }

        yyjson_val* extensions_val = yyjson_obj_get(node_val, "extensions");
        if (extensions_val) {
            node.extensions = extensions_val;
        }

        yyjson_val* extras_val = yyjson_obj_get(node_val, "extras");
        if (extras_val) {
            node.extras = extras_val;
        }

        if(showDebug)printNodeInfo(node, idx);
        nodes.push_back(node);
    }

    identifyRootNodes(); // Call to identify root nodes after parsing
}

void GLTFNode::identifyRootNodes() {
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (int child : nodes[i].children) {
            nodes[child].isRoot = false; // Mark children as non-root
        }
    }
}

void GLTFNode::identifyBones(const std::vector<int>& joints) {
    for (int joint : joints) {
        if (joint >= 0 && joint < nodes.size()) {
            nodes[joint].isBone = true; // Mark node as bone
        }
    }
}

glm::mat4 GLTFNode::getNodeTransform(const Node& node) const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), node.translation);
    glm::mat4 rotation = glm::mat4_cast(node.rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), node.scale);
    glm::mat4 transform = translation * rotation * scale;

   // std::cout << "Node Index: " << node.index << ", Transform: " << glm::to_string(transform) << std::endl; //index is undefined

    return transform;
}

int GLTFNode::findParentNodeIndex(int nodeIndex) const {
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (int child : nodes[i].children) {
            if (child == nodeIndex) {
                return static_cast<int>(i);
            }
        }
    }
    return -1; // Return -1 if no parent is found
}

std::vector<int> GLTFNode::getRootNodes() const {
    std::vector<int> rootNodes;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].isRoot) {
            rootNodes.push_back(static_cast<int>(i));
        }
    }
    return rootNodes;
}

std::vector<int> GLTFNode::getChildNodes(int nodeIndex) const {
    if (nodeIndex >= 0 && nodeIndex < nodes.size()) {
        return nodes[nodeIndex].children;
    }
    return std::vector<int>();
}

glm::mat4 GLTFNode::getGlobalTransform(int nodeIndex) const {
    glm::mat4 transform = glm::mat4(1.0f);
    int currentIndex = nodeIndex;
    while (currentIndex >= 0) {
        const auto& node = nodes[currentIndex];
        transform = getNodeTransform(node) * transform;
        currentIndex = findParentNodeIndex(currentIndex);
    }
    return transform;
}

void GLTFNode::updateNodeTransformation(Node& node) {
    node.transformation = glm::translate(glm::mat4(1.0f), node.translation) * glm::mat4_cast(node.rotation) * glm::scale(glm::mat4(1.0f), node.scale);

    for (int childIndex : node.children) {
        updateNodeTransformation(nodes[childIndex]);
    }
    // Debug output to verify node transformation
  //  std::cout << "Node: " << node.name << ", Transformation: " << glm::to_string(node.transformation) << std::endl;
}

void GLTFNode::printNodeInfo(const Node& node, size_t index) const {
    std::cout << "Node Info [" << index << "]:" << std::endl;
    std::cout << "Translation: " << glm::to_string(node.translation) << std::endl;
    std::cout << "Rotation: " << glm::to_string(node.rotation) << std::endl;
    std::cout << "Scale: " << glm::to_string(node.scale) << std::endl;
    std::cout << "Mesh Index: " << node.meshIndex << std::endl;
    std::cout << "Children: ";
    for (const auto& child : node.children) {
        std::cout << child << " ";
    }
    std::cout << std::endl;
    std::cout << "Name: " << (node.name.empty() ? "None" : node.name) << std::endl;
    std::cout << "Is Root: " << (node.isRoot ? "Yes" : "No") << std::endl;
}

const std::vector<GLTFNode::Node>& GLTFNode::getNodes() const {
    return nodes;
}

std::vector<GLTFNode::Node>& GLTFNode::getNodes() {
    return nodes;
}

glm::vec3 GLTFNode::getNodeTranslation(int nodeIndex) const {
    return nodes[nodeIndex].translation;
}

void GLTFNode::setNodeTranslation(int nodeIndex, const glm::vec3& translation) {
    nodes[nodeIndex].translation = translation;
}

glm::quat GLTFNode::getNodeRotation(int nodeIndex) const {
    return nodes[nodeIndex].rotation;
}

void GLTFNode::setNodeRotation(int nodeIndex, const glm::quat& rotation) {
    nodes[nodeIndex].rotation = rotation;
}

glm::vec3 GLTFNode::getNodeScale(int nodeIndex) const {
    return nodes[nodeIndex].scale;
}

void GLTFNode::setNodeScale(int nodeIndex, const glm::vec3& scale) {
    nodes[nodeIndex].scale = scale;
}