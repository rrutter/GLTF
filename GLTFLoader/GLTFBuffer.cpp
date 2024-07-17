#include "GLTFBuffer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

void GLTFBuffer::parseBuffers(yyjson_val* buffersArray, const std::string& basePath) {
    size_t idx, max;
    yyjson_val* buffer_val;
    yyjson_arr_foreach(buffersArray, idx, max, buffer_val) {
        Buffer buffer;

        yyjson_val* uri_val = yyjson_obj_get(buffer_val, "uri");
        if (uri_val) {
            buffer.uri = yyjson_get_str(uri_val);
            std::cout << "Buffer [" << idx << "] URI: " << buffer.uri << std::endl;
        }

        yyjson_val* byteLength_val = yyjson_obj_get(buffer_val, "byteLength");
        if (byteLength_val) {
            buffer.byteLength = yyjson_get_uint(byteLength_val);
            std::cout << "Buffer [" << idx << "] Byte Length: " << buffer.byteLength << std::endl;
        }
        else {
            std::cerr << "Buffer [" << idx << "] has no byteLength specified." << std::endl;
        }

        buffers.push_back(buffer);
    }

    // Debug output to confirm buffers initialization
    std::cout << "Total buffers parsed: " << buffers.size() << std::endl;
    for (size_t i = 0; i < buffers.size(); ++i) {
        std::cout << "Buffer [" << i << "]: URI: " << buffers[i].uri << ", Byte Length: " << buffers[i].byteLength << std::endl;
    }
}

void GLTFBuffer::parseBufferViews(yyjson_val* bufferViewsArray) {
    size_t idx, max;
    yyjson_val* bufferView_val;
    yyjson_arr_foreach(bufferViewsArray, idx, max, bufferView_val) {
        BufferView bufferView;
        bufferView.buffer = yyjson_get_int(yyjson_obj_get(bufferView_val, "buffer"));
        bufferView.byteOffset = yyjson_get_uint(yyjson_obj_get(bufferView_val, "byteOffset"));
        bufferView.byteLength = yyjson_get_uint(yyjson_obj_get(bufferView_val, "byteLength"));
        yyjson_val* byteStride_val = yyjson_obj_get(bufferView_val, "byteStride");
        bufferView.byteStride = byteStride_val ? yyjson_get_uint(byteStride_val) : 0;
        yyjson_val* target_val = yyjson_obj_get(bufferView_val, "target");
        bufferView.target = target_val ? (yyjson_get_str(target_val) ? yyjson_get_str(target_val) : "") : "";

        bufferView.extensions = yyjson_obj_get(bufferView_val, "extensions");
        bufferView.extras = yyjson_obj_get(bufferView_val, "extras");

        if (bufferView.buffer < 0 || bufferView.buffer >= buffers.size()) {
            std::cerr << "Error: Invalid buffer index in buffer view: " << bufferView.buffer << std::endl;
        }
        else {
            if(showDebug)printBufferViewInfo(bufferView, idx);
            bufferViews.push_back(bufferView);
        }
    }
}

void GLTFBuffer::loadBufferData(Buffer& buffer, const std::string& basePath) {
    if (buffer.uri.empty()) {
        throw std::runtime_error("Buffer URI is empty");
    }

    std::ifstream file(basePath + buffer.uri, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open buffer file: " + buffer.uri);
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string fileContent = ss.str();

    buffer.data.assign(fileContent.begin(), fileContent.end());

    if (buffer.data.size() != buffer.byteLength) {
        throw std::runtime_error("Buffer size mismatch for: " + buffer.uri);
    }
}

void GLTFBuffer::loadEmbeddedBufferData(const std::vector<unsigned char>& binChunkData) {
    std::cout << "Entering loadEmbeddedBufferData with binChunkData size: " << binChunkData.size() << std::endl;

    if (buffers.empty()) {
        std::cerr << "No buffers to load data into." << std::endl;
        return;
    }

    for (size_t i = 0; i < buffers.size(); ++i) {
        auto& buffer = buffers[i];
        if (buffer.uri.empty()) {
            buffer.data = binChunkData;
            buffer.byteLength = binChunkData.size();
            std::cout << "Buffer [" << i << "] data loaded. Byte Length: " << buffer.byteLength << ", Data Size: " << buffer.data.size() << std::endl;
        }
        else {
            std::cerr << "Buffer [" << i << "] URI is not empty, expected embedded data." << std::endl;
        }
    }

    // Additional check to ensure buffer data is set correctly
    for (size_t i = 0; i < buffers.size(); ++i) {
        const auto& buffer = buffers[i];
        std::cout << "Final Buffer [" << i << "] Info - Byte Length: " << buffer.byteLength << ", Data Size: " << buffer.data.size() << std::endl;
    }
}

std::vector<GLTFBuffer::Buffer>& GLTFBuffer::getBuffers() {
    return buffers;
}

std::vector<GLTFBuffer::BufferView>& GLTFBuffer::getBufferViews() {
    return bufferViews;
}

void GLTFBuffer::printBufferInfo(const Buffer& buffer, size_t index) const {
    std::cout << "Buffer Info [" << index << "]:" << std::endl;
    std::cout << "URI: " << buffer.uri << std::endl;
    std::cout << "Byte Length: " << buffer.byteLength << std::endl;
    std::cout << "Data Size: " << buffer.data.size() << " bytes" << std::endl;
}

void GLTFBuffer::printBufferViewInfo(const BufferView& bufferView, size_t index) const {
    std::cout << "Buffer View Info [" << index << "]:" << std::endl;
    std::cout << "Buffer Index: " << bufferView.buffer << std::endl;
    std::cout << "Byte Offset: " << bufferView.byteOffset << std::endl;
    std::cout << "Byte Length: " << bufferView.byteLength << std::endl;
    std::cout << "Byte Stride: " << bufferView.byteStride << std::endl;
    std::cout << "Target: " << bufferView.target << std::endl;
}

std::vector<glm::vec3> GLTFBuffer::getPositions(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::vec3> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec3);
    size_t count = accessor.count;

    data.resize(count);

    for (size_t i = 0; i < count; ++i) {
        size_t offset = byteOffset + i * stride;
        if (offset + sizeof(glm::vec3) > buffer.data.size()) {
            std::cerr << "Error: Buffer overflow when accessing data." << std::endl;
            break;
        }
        std::memcpy(&data[i], &buffer.data[offset], sizeof(glm::vec3));
    }

    return data;
}

std::vector<glm::vec3> GLTFBuffer::getNormals(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::vec3> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec3);
    size_t count = accessor.count;

    data.resize(count);

    for (size_t i = 0; i < count; ++i) {
        size_t offset = byteOffset + i * stride;
        if (offset + sizeof(glm::vec3) > buffer.data.size()) {
            std::cerr << "Error: Buffer overflow when accessing data." << std::endl;
            break;
        }
        std::memcpy(&data[i], &buffer.data[offset], sizeof(glm::vec3));
    }

    return data;
}

std::vector<glm::vec2> GLTFBuffer::getTexcoords(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::vec2> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec2);
    size_t count = accessor.count;

    data.resize(count);

    for (size_t i = 0; i < count; ++i) {
        size_t offset = byteOffset + i * stride;
        if (offset + sizeof(glm::vec2) > buffer.data.size()) {
            std::cerr << "Error: Buffer overflow when accessing data." << std::endl;
            break;
        }
        std::memcpy(&data[i], &buffer.data[offset], sizeof(glm::vec2));
    }

    return data;
}

std::vector<unsigned int> GLTFBuffer::getIndices(const GLTFAccessor::Accessor& accessor) const {
    std::vector<unsigned int> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t stride = sizeof(unsigned short);  // Assuming UNSIGNED_SHORT for indices
    size_t count = accessor.count;

    data.resize(count);

    for (size_t i = 0; i < count; ++i) {
        size_t offset = byteOffset + i * stride;
        if (offset + sizeof(unsigned short) > buffer.data.size()) {
            std::cerr << "Error: Buffer overflow when accessing data." << std::endl;
            break;
        }
        unsigned short index;
        std::memcpy(&index, &buffer.data[offset], sizeof(unsigned short));
        data[i] = index;
    }

    return data;
}

std::vector<glm::vec4> GLTFBuffer::getColors(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::vec4> colors;
    const auto& bufferView = bufferViews[accessor.bufferView];
    const auto& buffer = buffers[bufferView.buffer];
    const unsigned char* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    size_t count = accessor.count;

    size_t stride = bufferView.byteStride ? bufferView.byteStride : sizeof(glm::vec4);
    colors.resize(count);
    for (size_t i = 0; i < count; ++i) {
        std::memcpy(&colors[i], data + i * stride, sizeof(glm::vec4));
    }

    return colors;
}

std::vector<glm::vec4> GLTFBuffer::getJoints(const GLTFAccessor::Accessor& accessor) const {
    const auto& bufferView = bufferViews[accessor.bufferView];
    const auto& buffer = buffers[bufferView.buffer];
    const float* data = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

    std::vector<glm::vec4> joints;
    for (size_t i = 0; i < accessor.count; ++i) {
        glm::vec4 joint(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
        joints.push_back(joint);
    }
    return joints;
}

std::vector<glm::vec4> GLTFBuffer::getWeights(const GLTFAccessor::Accessor& accessor) const {
    const auto& bufferView = bufferViews[accessor.bufferView];
    const auto& buffer = buffers[bufferView.buffer];
    const float* data = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

    std::vector<glm::vec4> weights;
    for (size_t i = 0; i < accessor.count; ++i) {
        glm::vec4 weight(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
        weights.push_back(weight);
    }
    return weights;
}

std::vector<float> GLTFBuffer::getAccessorDataFloat(const GLTFAccessor::Accessor& accessor) const {
    std::vector<float> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t count = accessor.count;

    data.resize(count);
    std::memcpy(data.data(), buffer.data.data() + byteOffset, count * sizeof(float));

    return data;
}

std::vector<glm::vec3> GLTFBuffer::getAccessorDataVec3(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::vec3> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t count = accessor.count;

    data.resize(count);
    std::memcpy(data.data(), buffer.data.data() + byteOffset, count * sizeof(glm::vec3));

    return data;
}

std::vector<glm::quat> GLTFBuffer::getAccessorDataQuat(const GLTFAccessor::Accessor& accessor) const {
    std::vector<glm::quat> data;

    if (accessor.bufferView < 0 || accessor.bufferView >= bufferViews.size()) {
        std::cerr << "Error: Invalid bufferView index in accessor." << std::endl;
        return data;
    }

    const BufferView& bufferView = bufferViews[accessor.bufferView];
    const Buffer& buffer = buffers[bufferView.buffer];

    size_t byteOffset = accessor.byteOffset + bufferView.byteOffset;
    size_t count = accessor.count;

    data.resize(count);
    std::memcpy(data.data(), buffer.data.data() + byteOffset, count * sizeof(glm::quat));

    return data;
}