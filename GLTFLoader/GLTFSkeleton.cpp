#include "GLTFSkeleton.h"
#include "PersonalGL.h"

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
    bone.globalTransform = nodeManager.getGlobalTransform(bone.nodeIndex); // global transform
    bones.push_back(bone);
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
        std::cout << "Accessor byte offset: " << accessor.byteOffset << std::endl;
        std::cout << "Buffer view byte offset: " << bufferView.byteOffset << std::endl;
        std::cout << "Buffer view byte stride: " << bufferView.byteStride << std::endl;
        std::cout << "Buffer view byte length: " << bufferView.byteLength << std::endl;
        std::cout << "Buffer size: " << buffer.data.size() << std::endl;

        size_t componentSize = 4 * 4 * sizeof(float); // size of a 4x4 matrix
        size_t byteStride = bufferView.byteStride == 0 ? componentSize : bufferView.byteStride;

        for (size_t i = 0; i < accessor.count; ++i) {
            const float* matrixData = reinterpret_cast<const float*>(data + i * byteStride / sizeof(float));
            glm::mat4 matrix = glm::make_mat4(matrixData);
            inverseBindMatrices.push_back(matrix);

            // Debug output for matrix data
            std::cout << "Matrix " << i << " offset: " << (bufferView.byteOffset + accessor.byteOffset + i * byteStride) << std::endl;
            std::cout << "Matrix " << i << ":" << std::endl;
            for (int row = 0; row < 4; ++row) {
                for (int col = 0; col < 4; ++col) {
                    std::cout << matrix[row][col] << " ";
                }
                std::cout << std::endl;
            }
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
    validateJointIndices();
    applySkinning();
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
        if (bones[i].inverseBindMatrix != inverseBindMatrices[i]) {
            std::cout << "bones " << i << " doesn't match with its inverse matrix" << std::endl;
        }
    }
}

void GLTFSkeleton::calculateBoneTransforms() {
    for (size_t i = 0; i < bones.size(); ++i) {
        Bone& bone = bones[i];
        bone.transform = nodeManager.getNodeTransform(nodeManager.getNodes()[bone.nodeIndex]);

        int parentIndex = bone.parentIndex;
        if (parentIndex >= 0) {
            bone.globalTransform = bones[parentIndex].globalTransform * bone.transform;
        }
        else {
            bone.globalTransform = bone.transform; // Root node's global transform is its own transform
        }
    }
}

void GLTFSkeleton::calculateJointMatrices() {
    for (size_t i = 0; i < bones.size(); ++i) {
        jointMatrices[i] = bones[i].globalTransform * bones[i].inverseBindMatrix;
    }
}

void GLTFSkeleton::updateSkeleton(const std::vector<GLTFNode::Node>& nodes) {
    calculateBoneTransforms();
    calculateJointMatrices();
   // applySkinning();
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

glm::mat4 GLTFSkeleton::getGlobalBoneTransform(int boneIndex) const {
    const Bone& bone = bones[boneIndex];//debug purposes
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
            if (totalWeight > 0.0f) {
                vertex.weights /= totalWeight;
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

                size_t vertexCount = positionAccessor.count;
                std::vector<Vertex> vertices(vertexCount);

                // Load joints and weights using GLTFBuffer methods
                const auto& jointsAccessor = accessorManager.getAccessors()[primitive.jointsAccessor];
                std::vector<glm::vec4> joints = bufferManager.getJoints(jointsAccessor);

                const auto& weightsAccessor = accessorManager.getAccessors()[primitive.weightsAccessor];
                std::vector<glm::vec4> weights = bufferManager.getWeights(weightsAccessor);

                for (size_t i = 0; i < vertexCount; ++i) {
                    vertices[i].position = glm::make_vec3(&positionData[i * 3]);
                    vertices[i].normal = glm::make_vec3(&normalData[i * 3]);
                    vertices[i].texCoord = glm::make_vec2(&texCoordData[i * 2]);
                    vertices[i].joints = glm::ivec4(joints[i]);
                    vertices[i].weights = weights[i];
                }

                verticesPerMesh[meshIndex].insert(verticesPerMesh[meshIndex].end(), vertices.begin(), vertices.end());
            }
        }
    }
}



const std::unordered_map<int, std::vector<Vertex>>& GLTFSkeleton::getVertices() const {
    return verticesPerMesh;
}

void GLTFSkeleton::applySkinning() {
    for (auto& meshPair : verticesPerMesh) {
        auto& vertices = meshPair.second;

        std::cout << "applying skinning to mesh!" << std::endl;
        for (auto& vertex : vertices) {
            glm::vec4 skinnedPosition(0.0f);
            glm::vec4 skinnedNormal(0.0f);

            for (int i = 0; i < 4; ++i) {
                if (vertex.weights[i] > 0.0f) {
                    int jointIndex = vertex.joints[i];
                    if (jointIndex < 0 || jointIndex >= jointMatrices.size()) {
                        std::cerr << "Invalid joint index: " << jointIndex << std::endl;
                        continue;
                    }
                    const glm::mat4& jointMatrix = jointMatrices[jointIndex];

                    skinnedPosition += jointMatrix * glm::vec4(vertex.position, 1.0f) * vertex.weights[i];
                    skinnedNormal += jointMatrix * glm::vec4(vertex.normal, 0.0f) * vertex.weights[i];
                }
            }

            vertex.position = glm::vec3(skinnedPosition);
            vertex.normal = glm::normalize(glm::vec3(skinnedNormal));

            // Debug output for skinned positions and normals
            //std::cout << "Vertex skinned position: " << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << std::endl;
            //std::cout << "Vertex skinned normal: " << vertex.normal.x << ", " << vertex.normal.y << ", " << vertex.normal.z << std::endl;
        }
    }
}


void GLTFSkeleton::validateJointIndices() {
    for (const auto& meshPair : verticesPerMesh) {
        int totalErrors = 0;
        const auto& vertices = meshPair.second;

        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& vertex = vertices[i];
            float weightSum = vertex.weights[0] + vertex.weights[1] + vertex.weights[2] + vertex.weights[3];

            for (int j = 0; j < 4; ++j) {
                if (vertex.weights[j] > 0.0f) { // 4 influenced joints
                    int jointIndex = vertex.joints[j];
                    if (jointIndex >= bones.size()) {
                        totalErrors++;
                        //std::cerr << "Invalid joint index: " << jointIndex << " in vertex data at vertex " << i << ", joint " << j << std::endl;
                        //std::cerr << "Joints: " << vertex.joints[0] << ", " << vertex.joints[1] << ", " << vertex.joints[2] << ", " << vertex.joints[3] << std::endl;
                    }
                }
            }
        }
    }
}

void GLTFSkeleton::checkSkinJoints(const auto& skin) {
    const auto& nodes = nodeManager.getNodes();

    for (size_t i = 0; i < skin.joints.size(); ++i) {
        int jointIndex = skin.joints[i];
        if (jointIndex >= bones.size()) {
            std::cerr << "Invalid joint index in skin: " << jointIndex << ", exceeds bone size: " << bones.size() << std::endl;
        }
        else {
            std::cout << "Valid joint index in skin: " << jointIndex << ", bone name: " << bones[jointIndex].name << std::endl;
        }
    }
}

size_t GLTFSkeleton::getNumComponents(const std::string& accessorType) const {
    if (accessorType == "SCALAR") return 1;
    if (accessorType == "VEC2") return 2;
    if (accessorType == "VEC3") return 3;
    if (accessorType == "VEC4") return 4;
    throw std::runtime_error("Unsupported accessor type");
}