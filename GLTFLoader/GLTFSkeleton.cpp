#include "GLTFSkeleton.h"

GLTFSkeleton::GLTFSkeleton(const GLTFMesh& meshManager, const GLTFNode& nodeManager, const GLTFAccessor& accessorManager, GLTFBuffer& bufferManager)
    : meshManager(meshManager), nodeManager(nodeManager), accessorManager(accessorManager), bufferManager(bufferManager) {
    initializeSkeleton();
}

void GLTFSkeleton::addBone(int nodeIndex, int parentIndex, const glm::mat4& inverseBindMatrix, const std::string& name) {
    Bone bone = { nodeIndex, parentIndex, name, inverseBindMatrix, glm::mat4(1.0f), {} };
    if (parentIndex >= 0) {
        if (parentIndex >= bones.size()) {
            std::cerr << "Invalid parent index: " << parentIndex << " for bone: " << nodeIndex << std::endl;
            return;
        }
        bones[parentIndex].children.push_back(static_cast<int>(bones.size()));
    }
    bone.transform = nodeManager.getNodes()[bone.nodeIndex].transformation; // local transform
    bone.globalTransform = nodeManager.getGlobalTransform(bone.nodeIndex); //global transform
    bones.push_back(bone);
    std::cout << "Adding bone #" << bones.size() - 1 << " - Bone Name: " << name << std::endl;
}

void GLTFSkeleton::loadInverseBindMatrices() {
    const auto& skins = meshManager.getSkins();
    for (const auto& skin : skins) {
        if (skin.inverseBindMatricesAccessor < 0) continue;

        const auto& accessor = accessorManager.getAccessors()[skin.inverseBindMatricesAccessor];
        const auto& bufferView = bufferManager.getBufferViews()[accessor.bufferView];
        const auto& buffer = bufferManager.getBuffers()[bufferView.buffer];

        const float* data = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

        std::cout << "Loading inverse bind matrices for skin..." << std::endl;
        std::cout << "Buffer buffer view: " << accessor.bufferView << std::endl;
        std::cout << "Buffer view buffer: " << bufferView.buffer << std::endl;
        std::cout << "Buffer size: " << buffer.data.size() << std::endl;

        for (size_t i = 0; i < accessor.count; ++i) {
            glm::mat4 matrix = glm::make_mat4(data + i * 16);
            inverseBindMatrices.push_back(matrix);
        }
    }
}

void GLTFSkeleton::initializeSkeleton() {
    const auto& skins = meshManager.getSkins();
    loadInverseBindMatrices();
    for (const auto& skin : skins) {
        parseSkin(skin);
    }
    checkInverseBindMatrices();
    loadVertices();
    jointMatrices.resize(bones.size());
    normalizeWeights();
    calculateBoneTransforms();
    calculateJointMatrices();
}

void GLTFSkeleton::parseSkin(const auto skin) {
    addRootBones(skin);
    addChildBones(skin);
}

void GLTFSkeleton::addRootBones(const auto skin) {
    const auto& nodes = nodeManager.getNodes();

    for (size_t i = 0; i < skin.joints.size(); ++i) {
        int nodeIndex = skin.joints[i];
        int parentIndex = nodeManager.findParentNodeIndex(nodeIndex);
        if (parentIndex == -1 || std::find(skin.joints.begin(), skin.joints.end(), parentIndex) == skin.joints.end()) {
            glm::mat4 inverseBindMatrix = (i < inverseBindMatrices.size()) ? inverseBindMatrices[bones.size()] : glm::mat4(1.0f);
            std::string name = nodes[nodeIndex].name;
            std::cout << "Root Bone - Node Index: " << nodeIndex << ", Parent Index: " << parentIndex << ", Bone Name: " << name << std::endl;
            addBone(nodeIndex, -1, inverseBindMatrix, name); // Add root bone
        }
    }
}

void GLTFSkeleton::addChildBones(const auto skin) {
    const auto& nodes = nodeManager.getNodes();

    for (size_t i = 0; i < skin.joints.size(); ++i) {
        int nodeIndex = skin.joints[i];
        int parentIndex = nodeManager.findParentNodeIndex(nodeIndex);
        if (parentIndex != -1 && std::find(skin.joints.begin(), skin.joints.end(), parentIndex) != skin.joints.end()) {
            auto parentIt = std::find_if(bones.begin(), bones.end(), [parentIndex](const Bone& bone) {
                return bone.nodeIndex == parentIndex;
                });
            if (parentIt != bones.end()) {
                int parentBoneIndex = std::distance(bones.begin(), parentIt);
                glm::mat4 inverseBindMatrix = (i < inverseBindMatrices.size()) ? inverseBindMatrices[bones.size()] : glm::mat4(1.0f);
                std::string name = nodes[nodeIndex].name;
                std::cout << "Child Bone - Node Index: " << nodeIndex << ", Parent Index: " << parentIndex << ", Bone Name: " << name << std::endl;
                addBone(nodeIndex, parentBoneIndex, inverseBindMatrix, name); // Add child bone
            }
            else {
                std::cerr << "Parent bone for node " << nodeIndex << " not found. Skipping..." << std::endl;
            }
        }
    }
}



void GLTFSkeleton::checkInverseBindMatrices() {
    for (size_t i = 0; i < bones.size(); ++i) {
        if (bones[i].inverseBindMatrix != inverseBindMatrices[i]){
            std::cout << "bones " << i << "doesnt match with it's inverse matrix" << std::endl;
        }
    }
}

void GLTFSkeleton::calculateBoneTransforms() {
    for (size_t i = 0; i < bones.size(); ++i) {
        Bone& bone = bones[i];
        bone.transform = nodeManager.getNodeTransform(nodeManager.getNodes()[bone.nodeIndex]);
        bone.globalTransform = bone.transform;

        int parentIndex = bone.parentIndex;
        if (parentIndex >= 0) {
            bone.globalTransform = bones[parentIndex].globalTransform * bone.transform;
        }
    }
}

void GLTFSkeleton::calculateJointMatrices() {
    for (size_t i = 0; i < bones.size(); ++i) {
        //jointMatrices[i] = bones[i].globalTransform * bones[i].inverseBindMatrix; //looks all messed up!
        jointMatrices[i] = bones[i].globalTransform;
    }
}

void GLTFSkeleton::updateSkeleton(const std::vector<GLTFNode::Node>& nodes) {
    calculateBoneTransforms();
    calculateJointMatrices();
}

const std::vector<glm::mat4>& GLTFSkeleton::getJointMatrices() const {
    return jointMatrices;
}

const GLTFSkeleton::Bone& GLTFSkeleton::getBone(int index) const {
    return bones[index];
}

const std::vector<GLTFSkeleton::Bone>& GLTFSkeleton::getBones() const {
    return bones;
}

glm::mat4 GLTFSkeleton::getBoneTransform(int boneIndex) const {
    const Bone& bone = bones[boneIndex];
    return nodeManager.getGlobalTransform(bone.nodeIndex);
}

const std::vector<glm::vec4>& GLTFSkeleton::getWeights() const {
    return weights;
}

void GLTFSkeleton::normalizeWeights() {
    for (auto& meshPair : verticesPerMesh) {
        auto& meshVertices = meshPair.second;
        for (auto& vertex : meshVertices) {
            float totalWeight = vertex.weights[0] + vertex.weights[1] + vertex.weights[2] + vertex.weights[3];

            if (totalWeight != 1.0f) {
                if (totalWeight > 0.0f) {
                    vertex.weights /= totalWeight;
                }
                else {
                    std::cout << "Vertex weights sum to zero. Assigning default weights." << std::endl;
                    vertex.weights = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f); // Default weight if sum is zero
                }

                // Recalculate totalWeight after normalization
                totalWeight = vertex.weights[0] + vertex.weights[1] + vertex.weights[2] + vertex.weights[3];
                if (totalWeight >= 1.1f) {
                    std::cout << "Normalized vertex weights still do not sum to 1: " << totalWeight << std::endl;
                }
            }
        }
    }
}

void GLTFSkeleton::printSkeleton() const {
    for (size_t i = 0; i < bones.size(); ++i) {
        const auto& bone = bones[i];
        std::cout << "Bone Index: " << i << ", Name: " << bone.name << ", Node Index: " << bone.nodeIndex
            << ", Parent Index: " << bone.parentIndex << std::endl;
        if (bone.parentIndex >= 0) {
            std::cout << "Parent Bone Name: " << bones[bone.parentIndex].name << std::endl;
        }
        std::cout << "Inverse Bind Matrix: " << glm::to_string(bone.inverseBindMatrix) << std::endl;
        std::cout << "Global Transform: " << glm::to_string(bone.globalTransform) << std::endl;
        for (int child : bone.children) {
            std::cout << "  Child Bone Index: " << child << " (Name: " << bones[child].name << ")" << std::endl;
        }
        std::cout << "Joint Matrix: " << glm::to_string(jointMatrices[i]) << std::endl;
    }
}

void GLTFSkeleton::printDebugBoneTransform() const {
    if (debugBoneIndex >= 0 && debugBoneIndex < bones.size()) {
        const auto& bone = bones[debugBoneIndex];
        std::cout << "Debug Bone Index: " << debugBoneIndex << std::endl;
        std::cout << "Bone Name: " << bone.name << std::endl;
        std::cout << "Global Transform: " << glm::to_string(bone.globalTransform) << std::endl;
        std::cout << "Joint Matrix: " << glm::to_string(jointMatrices[debugBoneIndex]) << std::endl;
    }
    else {
        std::cerr << "Invalid debug bone index: " << debugBoneIndex << std::endl;
    }
}

void GLTFSkeleton::loadVertices() {
    const auto& meshes = meshManager.getMeshes();
    for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex) {
        const auto& mesh = meshes[meshIndex];
        for (const auto& primitive : mesh.primitives) {
            if (primitive.positionAccessor >= 0) {
                const auto& positionAccessor = accessorManager.getAccessors()[primitive.positionAccessor];
                const auto& positionBufferView = bufferManager.getBufferViews()[positionAccessor.bufferView];
                const auto& positionBuffer = bufferManager.getBuffers()[positionBufferView.buffer];
                const float* positionData = reinterpret_cast<const float*>(&positionBuffer.data[positionBufferView.byteOffset + positionAccessor.byteOffset]);

                const auto& normalAccessor = accessorManager.getAccessors()[primitive.normalAccessor];
                const auto& normalBufferView = bufferManager.getBufferViews()[normalAccessor.bufferView];
                const auto& normalBuffer = bufferManager.getBuffers()[normalBufferView.buffer];
                const float* normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);

                const auto& texCoordAccessor = accessorManager.getAccessors()[primitive.texcoordAccessor];
                const auto& texCoordBufferView = bufferManager.getBufferViews()[texCoordAccessor.bufferView];
                const auto& texCoordBuffer = bufferManager.getBuffers()[texCoordBufferView.buffer];
                const float* texCoordData = reinterpret_cast<const float*>(&texCoordBuffer.data[texCoordBufferView.byteOffset + texCoordAccessor.byteOffset]);

                const auto& weightsAccessor = accessorManager.getAccessors()[primitive.weightsAccessor];
                const auto& weightsBufferView = bufferManager.getBufferViews()[weightsAccessor.bufferView];
                const auto& weightsBuffer = bufferManager.getBuffers()[weightsBufferView.buffer];
                const float* weightsData = reinterpret_cast<const float*>(&weightsBuffer.data[weightsBufferView.byteOffset + weightsAccessor.byteOffset]);

                const auto& jointsAccessor = accessorManager.getAccessors()[primitive.jointsAccessor];
                const auto& jointsBufferView = bufferManager.getBufferViews()[jointsAccessor.bufferView];
                const auto& jointsBuffer = bufferManager.getBuffers()[jointsBufferView.buffer];
                const int* jointsData = reinterpret_cast<const int*>(&jointsBuffer.data[jointsBufferView.byteOffset + jointsAccessor.byteOffset]);

                size_t vertexCount = positionAccessor.count;
                std::vector<Vertex> vertices(vertexCount);

                for (size_t i = 0; i < vertexCount; ++i) {
                    vertices[i].position = glm::make_vec3(&positionData[i * 3]);
                    vertices[i].normal = glm::make_vec3(&normalData[i * 3]);
                    vertices[i].texCoord = glm::make_vec2(&texCoordData[i * 2]);
                    vertices[i].weights = glm::make_vec4(&weightsData[i * 4]);
                    vertices[i].joints = glm::ivec4(jointsData[i * 4], jointsData[i * 4 + 1], jointsData[i * 4 + 2], jointsData[i * 4 + 3]);
                }

                verticesPerMesh[meshIndex].insert(verticesPerMesh[meshIndex].end(), vertices.begin(), vertices.end());
            }
        }
    }
}

const std::unordered_map<int, std::vector<Vertex>>& GLTFSkeleton::getVertices() const {
    return verticesPerMesh;
}
