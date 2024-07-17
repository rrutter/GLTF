#include "GLTF2.h"
#include "GLTFAccessor.h"
#include "GLTFAnimation.h"
#include "GLTFNode.h"
#include "GLTFMesh.h"
#include "GLTFBuffer.h"
#include "GLTFMaterial.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

GLTFLoader::GLTFLoader() : skeleton(meshManager, nodeManager, accessorManager, bufferManager) {
    eboIndices = 0;
    shaderProgram = 0;
    vao = 0;
    vboNormals = 0;
    vboPositions = 0;
    vboTexCoords = 0;
}

GLTFLoader::~GLTFLoader() {
    // Destructor implementation (if needed)
}

void GLTFLoader::loadModel(const std::string& filepath) {
    std::string ext = getFileExtension(filepath);

    if (ext == "glb") {
        loadGLBModel(filepath);
    }
    else if (ext == "gltf") {
        loadGLTFModel(filepath);
    }
    else {
        std::cerr << "Unsupported file format: " << ext << std::endl;
    }
}

std::string GLTFLoader::getFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of(".");
    if (dotPos == std::string::npos) return "";
    return filepath.substr(dotPos + 1);
}

void GLTFLoader::loadGLBModel(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Failed to read file: " << filepath << std::endl;
        return;
    }

    if (size < sizeof(GLBHeader)) {
        std::cerr << "Invalid GLB file." << std::endl;
        return;
    }

    // Parse GLB header
    GLBHeader header;
    std::memcpy(&header, buffer.data(), sizeof(GLBHeader));

    // Ensure magic number is correct
    if (header.magic != 0x46546C67) { // 'glTF' in hexadecimal
        std::cerr << "Invalid GLB magic number. Expected 'glTF', got: " << std::hex << header.magic << std::dec << std::endl;
        return;
    }

    // Ensure version is supported
    if (header.version != 2) {
        std::cerr << "Unsupported GLB version: " << header.version << std::endl;
        return;
    }

    // Output header information
    printGLBHeaderInfo(header);

    size_t pos = sizeof(GLBHeader);
    bool jsonChunkParsed = false;
    bool binChunkParsed = false;
    std::vector<unsigned char> binChunkData;
    std::vector<char> jsonChunkData;

    while (pos < size) {
        uint32_t chunkLength = *reinterpret_cast<uint32_t*>(&buffer[pos]);
        uint32_t chunkType = *reinterpret_cast<uint32_t*>(&buffer[pos + 4]);
        size_t chunkDataSize = chunkLength;

        // Print chunk information
        printChunkInfo(chunkLength, chunkType, chunkDataSize);

        pos += 8; // Move past chunk length and type

        // Calculate padded length
        size_t paddedLength = (chunkLength + 3) & ~3; // Align to 4 bytes

        if (chunkType == 0x4E4F534A) { // 'JSON'
            if (jsonChunkParsed) {
                std::cerr << "Error: Multiple JSON chunks found." << std::endl;
                return;
            }
            jsonChunkParsed = true;

            jsonChunkData.resize(chunkLength);
            std::memcpy(jsonChunkData.data(), &buffer[pos], chunkLength);
        }
        else if (chunkType == 0x004E4942) { // 'BIN'
            if (binChunkParsed) {
                std::cerr << "Error: Multiple BIN chunks found." << std::endl;
                return;
            }
            binChunkParsed = true;

            if (chunkLength > 0) {
                binChunkData.resize(chunkLength);
                std::memcpy(binChunkData.data(), &buffer[pos], chunkLength);
            }
        }
        else {
            std::cerr << "Unknown chunk type: " << std::hex << chunkType << std::dec << std::endl;
            return;
        }

        pos += paddedLength; // Move past chunk data including padding
    }

    if (!jsonChunkParsed) {
        std::cerr << "Error: No JSON chunk found." << std::endl;
        return;
    }

    if (!binChunkParsed) {
        std::cerr << "Error: No BIN chunk found." << std::endl;
        return;
    }

    std::cout << "BIN chunk data size: " << binChunkData.size() << " bytes" << std::endl;

    // Parse JSON chunk first
    yyjson_doc* doc = yyjson_read(jsonChunkData.data(), jsonChunkData.size(), 0);
    if (!doc) {
        std::cerr << "Failed to read JSON document." << std::endl;
        return;
    }

    std::cout << "Successfully read JSON document." << std::endl;
    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!root) {
        std::cerr << "Failed to get root from JSON document." << std::endl;
        yyjson_doc_free(doc);
        return;
    }

    std::string basePath = filepath.substr(0, filepath.find_last_of("/\\") + 1);

    // Parse buffer metadata from JSON chunk before loading BIN chunk
    yyjson_val* buffers_val = yyjson_obj_get(root, "buffers");
    if (buffers_val && yyjson_is_arr(buffers_val)) {
        bufferManager.parseBuffers(buffers_val, basePath);
    }

    // Load embedded buffer data after parsing buffers
    if (!binChunkData.empty()) {
        bufferManager.loadEmbeddedBufferData(binChunkData);

        // Print buffer information after loading embedded buffer data
        for (size_t i = 0; i < bufferManager.getBuffers().size(); ++i) {
            const auto& buffer = bufferManager.getBuffers()[i];
            std::cout << "Buffer [" << i << "]: URI: " << buffer.uri << ", Byte Length: " << buffer.byteLength << ", Data Size: " << buffer.data.size() << std::endl;
        }
    }

    // Now parse the rest of the GLTF structure
    std::cout << "Successfully obtained root. Calling parseGLTF..." << std::endl;
    parseGLTF(root, basePath);
    yyjson_doc_free(doc);

    std::cout << "Successfully loaded GLB model: " << filepath << std::endl;
}






void GLTFLoader::loadGLTFModel(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return;
    }

    std::string jsonContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    yyjson_doc* doc = yyjson_read(jsonContent.c_str(), jsonContent.size(), 0);
    if (!doc) {
        std::cerr << "Failed to parse GLTF JSON file." << std::endl;
        return;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);
    std::string basePath = filepath.substr(0, filepath.find_last_of("/\\") + 1);
    parseGLTF(root, basePath);
    yyjson_doc_free(doc);

    std::cout << "Successfully loaded GLTF model: " << filepath << std::endl;
}

void GLTFLoader::printGLBHeaderInfo(const GLBHeader& header) {
    std::cout << "GLB Header Information:" << std::endl;
    std::cout << "Magic: 'glTF'" << std::endl; // Always 'glTF' if the file is valid
    std::cout << "Version: " << header.version << std::endl; // Should be 2
    std::cout << "Length: " << header.length << " bytes" << std::endl; // Total length of the file in bytes
}

void GLTFLoader::printChunkInfo(uint32_t chunkLength, uint32_t chunkType, size_t chunkDataSize) {
    // Convert chunkType to ASCII string
    char chunkTypeStr[5];
    chunkTypeStr[0] = (chunkType >> 0) & 0xFF;
    chunkTypeStr[1] = (chunkType >> 8) & 0xFF;
    chunkTypeStr[2] = (chunkType >> 16) & 0xFF;
    chunkTypeStr[3] = (chunkType >> 24) & 0xFF;
    chunkTypeStr[4] = '\0'; // Null-terminate the string

    std::cout << "Chunk Information:" << std::endl;
    std::cout << "Chunk Length: " << chunkLength << " bytes" << std::endl;
    std::cout << "Chunk Type: " << std::hex << chunkType << " (ASCII: " << chunkTypeStr << ")" << std::dec << std::endl;
    std::cout << "Chunk Data Size: " << chunkDataSize << " bytes" << std::endl;
}

void GLTFLoader::parseGLTF(yyjson_val* root, const std::string& basePath) {
    std::cout << "parsing GLTF file..." << std::endl;

    yyjson_val* bufferViews_val = yyjson_obj_get(root, "bufferViews");
    if (bufferViews_val && yyjson_is_arr(bufferViews_val)) {
        std::cout << "Parsing bufferViews..." << std::endl;
        bufferManager.parseBufferViews(bufferViews_val);
    }

    yyjson_val* accessors_val = yyjson_obj_get(root, "accessors");
    if (accessors_val && yyjson_is_arr(accessors_val)) {
        std::cout << "Starting to parse accessors..." << std::endl;
        accessorManager.parseAccessors(accessors_val);
        std::cout << "Finished parsing accessors." << std::endl;
    }

    yyjson_val* animations_val = yyjson_obj_get(root, "animations");
    if (animations_val && yyjson_is_arr(animations_val)) {
        std::cout << "Parsing animations..." << std::endl;
        animationManager.parseAnimations(animations_val, accessorManager, bufferManager);
    }
    else {
        std::cout << "Animations key not found or is not an array." << std::endl;
    }

    yyjson_val* nodes_val = yyjson_obj_get(root, "nodes");
    if (nodes_val && yyjson_is_arr(nodes_val)) {
        std::cout << "Parsing nodes..." << std::endl;
        nodeManager.parseNodes(nodes_val);
    }

    yyjson_val* meshes_val = yyjson_obj_get(root, "meshes");
    if (meshes_val && yyjson_is_arr(meshes_val)) {
        std::cout << "Parsing meshes..." << std::endl;
        meshManager.parseMeshes(meshes_val);
    }

    yyjson_val* skins_val = yyjson_obj_get(root, "skins");
    if (skins_val && yyjson_is_arr(skins_val)) {
        std::cout << "Parsing skins..." << std::endl;
        meshManager.parseSkins(skins_val);
    }

    yyjson_val* materials_val = yyjson_obj_get(root, "materials");
    yyjson_val* textures_val = yyjson_obj_get(root, "textures");
    yyjson_val* images_val = yyjson_obj_get(root, "images");

    if (materials_val && yyjson_is_arr(materials_val) &&
        textures_val && yyjson_is_arr(textures_val) &&
        images_val && yyjson_is_arr(images_val)) {
        std::cout << "Parsing materials, textures, and images..." << std::endl;
        materialManager.parseMaterials(materials_val, textures_val, images_val);
        materialManager.loadImageData(bufferManager);
    }

    skeleton.initializeSkeleton();
}



void GLTFLoader::loadExternalBuffer(const std::string& uri, const std::string& basePath) {
    std::ifstream file(uri, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open buffer file: " << uri << std::endl;
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    GLTFBuffer::Buffer buffer;
    buffer.data.resize(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data.data()), size)) {
        std::cerr << "Failed to read buffer file: " << uri << std::endl;
        return;
    }

    bufferManager.getBuffers().push_back(buffer);
}

std::vector<glm::vec3> GLTFLoader::getPositions() const {
    std::vector<glm::vec3> positions;
    for (const auto& mesh : meshManager.getMeshes()) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.positionAccessor >= 0) {
                const auto& accessor = accessorManager.getAccessors()[primitive.positionAccessor];
                auto pos = bufferManager.getPositions(accessor);
                positions.insert(positions.end(), pos.begin(), pos.end());
            }
        }
    }
    return positions;
}

std::vector<glm::vec3> GLTFLoader::getNormals() const {
    std::vector<glm::vec3> normals;
    for (const auto& mesh : meshManager.getMeshes()) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.normalAccessor >= 0) {
                const auto& accessor = accessorManager.getAccessors()[primitive.normalAccessor];
                auto norm = bufferManager.getNormals(accessor);
                normals.insert(normals.end(), norm.begin(), norm.end());
            }
        }
    }
    return normals;
}

std::vector<glm::vec2> GLTFLoader::getTexcoords() const {
    std::vector<glm::vec2> texcoords;
    for (const auto& mesh : meshManager.getMeshes()) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.texcoordAccessor >= 0) {
                const auto& accessor = accessorManager.getAccessors()[primitive.texcoordAccessor];
                auto tex = bufferManager.getTexcoords(accessor);
                texcoords.insert(texcoords.end(), tex.begin(), tex.end());
            }
        }
    }
    return texcoords;
}

std::vector<unsigned int> GLTFLoader::getIndices() const {
    std::vector<unsigned int> indices;
    for (const auto& mesh : meshManager.getMeshes()) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.indicesAccessor >= 0) {
                const auto& accessor = accessorManager.getAccessors()[primitive.indicesAccessor];
                auto ind = bufferManager.getIndices(accessor);
                indices.insert(indices.end(), ind.begin(), ind.end());
            }
        }
    }
    return indices;
}


void GLTFLoader::printAnimationNames() const {
    const auto& animations = animationManager.getAnimations();
    if (animations.empty()) {
        std::cout << "No animations found." << std::endl;
    }
    else {
        std::cout << "Animations:" << std::endl;
        for (const auto& animation : animations) {
            std::cout << " - " << animation.name << std::endl;
        }
    }
}

void GLTFLoader::printMeshData() {
    const auto& meshes = meshManager.getMeshes();
    const auto& accessors = accessorManager.getAccessors();

    for (const auto& mesh : meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.positionAccessor >= 0 && primitive.positionAccessor < accessors.size()) {
                const auto& accessor = accessors[primitive.positionAccessor];
                auto positions = bufferManager.getPositions(accessor);
                std::cout << "Positions:" << std::endl;
                for (const auto& pos : positions) {
                    std::cout << glm::to_string(pos) << std::endl;
                }
            }

            if (primitive.normalAccessor >= 0 && primitive.normalAccessor < accessors.size()) {
                const auto& accessor = accessors[primitive.normalAccessor];
                auto normals = bufferManager.getNormals(accessor);
                std::cout << "Normals:" << std::endl;
                for (const auto& norm : normals) {
                    std::cout << glm::to_string(norm) << std::endl;
                }
            }

            if (primitive.texcoordAccessor >= 0 && primitive.texcoordAccessor < accessors.size()) {
                const auto& accessor = accessors[primitive.texcoordAccessor];
                auto texcoords = bufferManager.getTexcoords(accessor);
                std::cout << "Texture Coordinates:" << std::endl;
                for (const auto& tex : texcoords) {
                    std::cout << glm::to_string(tex) << std::endl;
                }
            }
        }
    }
}

void GLTFLoader::printMaterialData() {
    const auto& materials = materialManager.getMaterials();
    const auto& textures = materialManager.getTextures();
    const auto& images = materialManager.getImages();

    std::cout << "Materials:" << std::endl;
    for (const auto& material : materials) {
        std::cout << "Material [" << &material - &materials[0] << "]:" << std::endl;
        std::cout << "Name: " << material.name << std::endl;
        std::cout << "Base Color Texture Index: " << material.baseColorTextureIndex << std::endl;
        if (material.baseColorTextureIndex >= 0 && material.baseColorTextureIndex < textures.size()) {
            const auto& texture = textures[material.baseColorTextureIndex];
            std::cout << "Base Color Texture:" << std::endl;
            std::cout << "  Sampler: " << texture.sampler << std::endl;
            std::cout << "  Source: " << texture.source << std::endl;
            std::cout << "  Name: " << texture.name << std::endl;
            if (texture.source >= 0 && texture.source < images.size()) {
                const auto& image = images[texture.source];
                std::cout << "  URI: " << image.uri << std::endl;
                std::cout << "  Buffer View: " << image.bufferView << std::endl;
                std::cout << "  MIME Type: " << image.mimeType << std::endl;
            }
            else {
                std::cerr << "Error: Invalid image source index in texture: " << texture.source << std::endl;
            }
        }
        else {
            std::cerr << "Error: Invalid base color texture index in material: " << material.baseColorTextureIndex << std::endl;
        }
    }

    std::cout << "Textures:" << std::endl;
    for (const auto& texture : textures) {
        std::cout << "Texture [" << &texture - &textures[0] << "]:" << std::endl;
        std::cout << "Sampler: " << texture.sampler << std::endl;
        std::cout << "Source: " << texture.source << std::endl;
        std::cout << "Name: " << texture.name << std::endl;
        if (texture.source >= 0 && texture.source < images.size()) {
            const auto& image = images[texture.source];
            std::cout << "URI: " << image.uri << std::endl;
            std::cout << "Buffer View: " << image.bufferView << std::endl;
            std::cout << "MIME Type: " << image.mimeType << std::endl;
        }
        else {
            std::cerr << "Error: Invalid image source index in texture: " << texture.source << std::endl;
        }
    }
}
