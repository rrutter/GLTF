#ifndef GLTFBUFFER_H
#define GLTFBUFFER_H

#include <string>
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "yyjson.h"
#include "GLTFAccessor.h"

class GLTFBuffer {
public:
    struct Buffer {
        std::string uri;
        std::vector<unsigned char> data;
        size_t byteLength;
    };

    struct BufferView {
        int buffer;
        size_t byteOffset;
        size_t byteLength;
        size_t byteStride;
        std::string target;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    void parseBuffers(yyjson_val* buffersArray, const std::string& basePath);
    void parseBufferViews(yyjson_val* bufferViewsArray);
    std::vector<Buffer>& getBuffers();
    std::vector<BufferView>& getBufferViews();

    std::vector<glm::vec3> getPositions(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec3> getNormals(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec2> getTexcoords(const GLTFAccessor::Accessor& accessor) const;
    std::vector<unsigned int> getIndices(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec4> getColors(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec4> getJoints(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec4> getWeights(const GLTFAccessor::Accessor& accessor) const;
    std::vector<float> getAccessorDataFloat(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::vec3> getAccessorDataVec3(const GLTFAccessor::Accessor& accessor) const;
    std::vector<glm::quat> getAccessorDataQuat(const GLTFAccessor::Accessor& accessor) const;
    void loadEmbeddedBufferData(const std::vector<unsigned char>& binChunkData);

private:
    std::vector<Buffer> buffers;
    std::vector<BufferView> bufferViews;
    void loadBufferData(Buffer& buffer, const std::string& basePath);
    void printBufferInfo(const Buffer& buffer, size_t index) const;
    void printBufferViewInfo(const BufferView& bufferView, size_t index) const;
    bool showDebug = false;
};

#endif // GLTFBUFFER_H
