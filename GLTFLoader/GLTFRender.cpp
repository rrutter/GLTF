#include "GLTF2.h"
#include "Loadpng.h"
#include <unordered_set>

extern PersonalGL pGL;
extern CCamera Camera;
extern LoadPNG png;

bool showJoints = true;

void GLTFLoader::initialize() {
    initBuffers();
    initializeTextures();
    initializeShaders();
}

void GLTFLoader::initializeShaders() {
    shaderProgram = pGL.createShaderProgram("shaders/gltf_vshader.glsl", "shaders/gltf_fshader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
    }
    else {
        std::cout << "Shader program created successfully: " << shaderProgram << std::endl;
    }
}

glm::mat4 GLTFLoader::getNodeTransform(const GLTFNode::Node& node) const {
    glm::mat4 transform = glm::mat4(1.0f);

    // Validate and set translation
    transform = glm::translate(transform, node.translation);

    // Validate and set rotation
    transform *= glm::mat4_cast(node.rotation);

    // Validate and set scale
    glm::vec3 validScale = node.scale;
    if (validScale.x == 0.0f) validScale.x = 1.0f;
    if (validScale.y == 0.0f) validScale.y = 1.0f;
    if (validScale.z == 0.0f) validScale.z = 1.0f;
    transform = glm::scale(transform, validScale);

    return transform;
}

glm::mat4 GLTFLoader::getNodeHierarchyTransform(int nodeIndex) const {
    glm::mat4 transform = glm::mat4(1.0f);
    auto& nodes = nodeManager.getNodes();
    int currentIndex = nodeIndex;
    std::unordered_set<int> visitedNodes;

    while (currentIndex >= 0) {
        if (visitedNodes.find(currentIndex) != visitedNodes.end()) {
            std::cerr << "Cycle or repeated node detected at index: " << currentIndex << std::endl;
            break;
        }

        visitedNodes.insert(currentIndex);  // Mark this node as visited

        const auto& node = nodes[currentIndex];
        transform = getNodeTransform(node) * transform; //model disappears

        int parentIndex = -1;
        for (size_t i = 0; i < nodes.size(); ++i) {
            for (int child : nodes[i].children) {
                if (child == currentIndex) {
                    parentIndex = static_cast<int>(i);
                    break;
                }
            }
            if (parentIndex >= 0) {
                break;
            }
        }


        if (parentIndex >= 0) {
        }
        else {
            std::cout << "Root Node Found: " << currentIndex << std::endl;
            std::cout << "Root Node Name: " << node.name << std::endl;
        }

        currentIndex = parentIndex;
    }

    return transform;
}

void GLTFLoader::initializeTextures() {
    const auto& images = materialManager.getImages();
    const auto& textures = materialManager.getTextures();

    for (const auto& texture : textures) {
        if (texture.source >= 0 && texture.source < images.size()) {
            const auto& image = images[texture.source];

            if (!image.uri.empty()) {
                try {
                    GLuint textureID = pGL.glCreateTex(image.uri.c_str());
                    textureIDMap[texture.source] = textureID;
                    std::cout << "Loaded texture from file: " << image.uri << " as texture ID: " << textureID << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Error loading texture: " << e.what() << std::endl;
                }
            }
            else if (!image.data.empty()) {
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexImage2D(GL_TEXTURE_2D, 0, image.colorType == PNG_COLOR_TYPE_RGB ? GL_RGB : GL_RGBA, image.width, image.height, 0, image.colorType == PNG_COLOR_TYPE_RGB ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image.data.data());

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glBindTexture(GL_TEXTURE_2D, 0);
                textureIDMap[texture.source] = textureID;

                std::cout << "Loaded embedded texture as texture ID: " << textureID << std::endl;
                std::cout << "Texture details - Width: " << image.width << ", Height: " << image.height << ", Color Type: " << image.colorType << std::endl;
            }
            else {
                std::cerr << "No texture data available for texture source: " << texture.source << std::endl;
            }
        }
    }
}


void GLTFLoader::initBuffers() {
    const auto& nodes = nodeManager.getNodes();
    const auto& meshes = meshManager.getMeshes();

    if (nodes.empty() || meshes.empty()) {
        std::cerr << "Nodes or meshes are empty. Initialization failed." << std::endl;
        return;
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        if (node.meshIndex >= 0 && node.meshIndex < meshes.size()) {
            const auto& mesh = meshes[node.meshIndex];
            glm::mat4 nodeTransform = getNodeHierarchyTransform(static_cast<int>(i));

            for (const auto& primitive : mesh.primitives) {
                if (primitive.positionAccessor >= 0) {
                    PrimitiveBuffers buffers;
                    setupVertexArrayObject(buffers, primitive, node.meshIndex);
                    buffers.transform = nodeTransform;
                    buffers.materialIndex = primitive.materialIndex;
                    primitiveBuffers.push_back(buffers);

                    // Debug output to verify buffers are correctly set up
                    std::cout << "Initialized buffers for node index " << i << " with mesh index " << node.meshIndex << std::endl;
                }
                else {
                    std::cerr << "Primitive with invalid position accessor at node index " << i << std::endl;
                }
            }
        }
    }

    if (primitiveBuffers.empty()) {
        std::cerr << "No buffers were initialized. Check mesh and node data." << std::endl;
    }
}


void GLTFLoader::render() {
    glUseProgram(shaderProgram);

    glm::mat4 viewMatrix = Camera.getViewMatrix();
    glm::mat4 projectionMatrix = Camera.getProjectionMatrix();

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    //GLuint boneTransformsLoc = glGetUniformLocation(shaderProgram, "boneTransforms");
    GLuint jointMatricesLoc = glGetUniformLocation(shaderProgram, "jointMatrices");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);


    const auto jointMatrices = skeleton.getJointMatrices();

    if (!jointMatrices.empty()) {
        //glUniformMatrix4fv(boneTransformsLoc, boneTransforms.size(), GL_FALSE, &boneTransforms[0][0][0]);
        glUniformMatrix4fv(jointMatricesLoc, jointMatrices.size(), GL_FALSE, &jointMatrices[0][0][0]);
    }

    for (const auto& buffers : primitiveBuffers) {
        glBindVertexArray(buffers.vao);

        glm::mat4 modelMatrix = buffers.transform;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);

        // Uncomment the line below to render in wireframe mode for better visualization
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, buffers.indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (showJoints) {
        // Render joints
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&projectionMatrix[0][0]);

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&viewMatrix[0][0]);

        glEnable(GL_POINT_SMOOTH);
        glPointSize(10.0f);

        glBegin(GL_POINTS);
        for (const auto& transform : jointMatrices) {
            glm::vec3 bonePosition = glm::vec3(transform[3]); // Extract the translation part of the bone transform
            glVertex3f(bonePosition.x, bonePosition.y, bonePosition.z);
        }
        glEnd();
    }

    glUseProgram(0);
}





void GLTFLoader::setupVertexArrayObject(PrimitiveBuffers& buffers, const GLTFMesh::Primitive& primitive, int meshIndex) {
    const auto& verticesMap = skeleton.getVertices();
    if (verticesMap.find(meshIndex) == verticesMap.end()) {
        std::cerr << "Mesh index " << meshIndex << " not found in vertices map." << std::endl;
        return;
    }
    checkVerts(primitive, meshIndex);
    const auto& vertices = verticesMap.at(meshIndex);

    glGenVertexArrays(1, &buffers.vao);
    glBindVertexArray(buffers.vao);

    glGenBuffers(1, &buffers.vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.vboPositions);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joints));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(4);

    if (primitive.indicesAccessor >= 0) {
        const auto& indices = bufferManager.getIndices(accessorManager.getAccessors()[primitive.indicesAccessor]);
        glGenBuffers(1, &buffers.eboIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.eboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        buffers.indexCount = static_cast<GLsizei>(indices.size());
    }

    glBindVertexArray(0);
    PersonalGL::checkGLError("setupVertexArrayObject");
}



void GLTFLoader::checkVerts(const GLTFMesh::Primitive& primitive, int meshIndex) {
    const auto& verticesMap = skeleton.getVertices();

    if (verticesMap.find(meshIndex) == verticesMap.end()) {
        std::cerr << "Mesh index " << meshIndex << " not found in vertices map." << std::endl;
        return;
    }

    const auto& verticesFromSkeleton = verticesMap.at(meshIndex);

    // Print Buffer Manager Vertices
    const auto& positions = bufferManager.getPositions(accessorManager.getAccessors()[primitive.positionAccessor]);
    const auto& normals = bufferManager.getNormals(accessorManager.getAccessors()[primitive.normalAccessor]);
    const auto& texcoords = bufferManager.getTexcoords(accessorManager.getAccessors()[primitive.texcoordAccessor]);
    const auto& joints = bufferManager.getJoints(accessorManager.getAccessors()[primitive.jointsAccessor]);
    const auto& weights = bufferManager.getWeights(accessorManager.getAccessors()[primitive.weightsAccessor]);

    std::cout << "checking verts..." << std::endl;
    std::cout << "Skeleton vert size " << verticesFromSkeleton.size() << std::endl;
    std::cout << "Buffer vert size " << positions.size() << std::endl;

    // Check for differences
    //for (size_t i = 0; i < std::min<size_t>(verticesFromSkeleton.size(), positions.size()); ++i) {
    for (size_t i = 0; i < 0; ++i) {
        const auto& vertex = verticesFromSkeleton[i];
        bool difference = false;

        if (vertex.position != positions[i]) {
            std::cout << "Difference in Position at index " << i << std::endl;
            difference = true;
        }
        if (vertex.normal != normals[i]) {
            std::cout << "Difference in Normal at index " << i << std::endl;
            difference = true;
        }
        if (vertex.texCoord != texcoords[i]) {
            std::cout << "Difference in TexCoord at index " << i << std::endl;
            difference = true;
        }
        if (vertex.weights != weights[i]) {
            std::cout << "Difference in Weights at index " << i << std::endl;
            difference = true;
        }
        if (vertex.joints != glm::ivec4(joints[i])) {
            std::cout << "Difference in Joints at index " << i << std::endl;
            difference = true;
        }

        if (difference) {
            std::cout << "Vertex " << i << ": " << std::endl;
            std::cout << "  Skeleton Vertex - Position: " << glm::to_string(vertex.position)
                << ", Normal: " << glm::to_string(vertex.normal)
                << ", TexCoord: " << glm::to_string(vertex.texCoord)
                << ", Weights: " << glm::to_string(vertex.weights)
                << ", Joints: " << glm::to_string(vertex.joints) << std::endl;
            std::cout << "  Buffer Vertex - Position: " << glm::to_string(positions[i])
                << ", Normal: " << glm::to_string(normals[i])
                << ", TexCoord: " << glm::to_string(texcoords[i])
                << ", Weights: " << glm::to_string(weights[i])
                << ", Joints: " << glm::to_string(joints[i]) << std::endl;
        }
    }
}




void GLTFLoader::setAnimation(const std::string& animationName) {
    animationManager.setAnimation(animationName);
}

void GLTFLoader::updateAnimation(float deltaTime) {
    animationManager.updateAnimation(deltaTime, nodeManager, skeleton, meshManager);
}