#include "GLTFAccessor.h"

void GLTFAccessor::parseAccessors(yyjson_val* accessorsArray) {
    std::cout << "Parsing Accessors..." << std::endl; // Debug message

    size_t idx, max;
    yyjson_val* accessor_val;
    yyjson_arr_foreach(accessorsArray, idx, max, accessor_val) {
        Accessor accessor;

        yyjson_val* bufferView_val = yyjson_obj_get(accessor_val, "bufferView");
        accessor.bufferView = bufferView_val ? yyjson_get_int(bufferView_val) : -1;
        if (accessor.bufferView == -1) {
            std::cerr << "Warning: Accessor [" << idx << "] has an invalid or missing bufferView." << std::endl;
        }

        yyjson_val* byteOffset_val = yyjson_obj_get(accessor_val, "byteOffset");
        accessor.byteOffset = byteOffset_val ? yyjson_get_uint(byteOffset_val) : 0;

        yyjson_val* componentType_val = yyjson_obj_get(accessor_val, "componentType");
        accessor.componentType = componentType_val ? yyjson_get_int(componentType_val) : -1;
        if (accessor.componentType == -1) {
            std::cerr << "Warning: Accessor [" << idx << "] has an invalid or missing componentType." << std::endl;
        }

        yyjson_val* normalized_val = yyjson_obj_get(accessor_val, "normalized");
        accessor.normalized = normalized_val ? yyjson_get_bool(normalized_val) : false;

        yyjson_val* count_val = yyjson_obj_get(accessor_val, "count");
        accessor.count = count_val ? yyjson_get_uint(count_val) : 0;
        if (accessor.count == 0) {
            std::cerr << "Warning: Accessor [" << idx << "] has an invalid or missing count." << std::endl;
        }

        yyjson_val* type_val = yyjson_obj_get(accessor_val, "type");
        accessor.type = type_val ? yyjson_get_str(type_val) : "";
        if (accessor.type.empty()) {
            std::cerr << "Warning: Accessor [" << idx << "] has an invalid or missing type." << std::endl;
        }

        yyjson_val* max_val = yyjson_obj_get(accessor_val, "max");
        if (max_val && yyjson_is_arr(max_val)) {
            size_t max_idx, max_max;
            yyjson_val* val;
            yyjson_arr_foreach(max_val, max_idx, max_max, val) {
                accessor.max.push_back(glm::vec3(yyjson_get_real(val)));
            }
        }

        yyjson_val* min_val = yyjson_obj_get(accessor_val, "min");
        if (min_val && yyjson_is_arr(min_val)) {
            size_t min_idx, min_max;
            yyjson_val* val;
            yyjson_arr_foreach(min_val, min_idx, min_max, val) {
                accessor.min.push_back(glm::vec3(yyjson_get_real(val)));
            }
        }

        yyjson_val* sparse_val = yyjson_obj_get(accessor_val, "sparse");
        accessor.sparse = sparse_val ? yyjson_get_bool(sparse_val) : false;
        if (accessor.sparse) {
            std::cerr << "Warning: Accessor [" << idx << "] has sparse data which is not yet handled." << std::endl;
        }

        yyjson_val* name_val = yyjson_obj_get(accessor_val, "name");
        accessor.name = name_val ? yyjson_get_str(name_val) : "";

        accessor.extensions = yyjson_obj_get(accessor_val, "extensions");
        accessor.extras = yyjson_obj_get(accessor_val, "extras");

        accessors.push_back(accessor);
    }

    std::cout << "Completed parsing Accessors." << std::endl; // Debug message after parsing
    for (size_t i = 0; i < accessors.size(); ++i) {
        if (showDebug) printAccessorInfo(accessors[i], i);
    }
}

void GLTFAccessor::parseBufferViews(yyjson_val* bufferViewsArray) {
    std::cout << "Parsing Buffer Views..." << std::endl; // Debug message

    size_t idx, max;
    yyjson_val* bufferView_val;
    yyjson_arr_foreach(bufferViewsArray, idx, max, bufferView_val) {
        BufferView bufferView;

        yyjson_val* buffer_val = yyjson_obj_get(bufferView_val, "buffer");
        bufferView.buffer = buffer_val ? yyjson_get_int(buffer_val) : -1;
        if (bufferView.buffer == -1) {
            std::cerr << "Warning: BufferView [" << idx << "] has an invalid or missing buffer." << std::endl;
        }

        yyjson_val* byteOffset_val = yyjson_obj_get(bufferView_val, "byteOffset");
        bufferView.byteOffset = byteOffset_val ? yyjson_get_uint(byteOffset_val) : 0;

        yyjson_val* byteLength_val = yyjson_obj_get(bufferView_val, "byteLength");
        bufferView.byteLength = byteLength_val ? yyjson_get_uint(byteLength_val) : 0;
        if (bufferView.byteLength == 0) {
            std::cerr << "Warning: BufferView [" << idx << "] has an invalid or missing byteLength." << std::endl;
        }

        yyjson_val* byteStride_val = yyjson_obj_get(bufferView_val, "byteStride");
        bufferView.byteStride = byteStride_val ? yyjson_get_uint(byteStride_val) : 0;

        bufferView.extensions = yyjson_obj_get(bufferView_val, "extensions");
        bufferView.extras = yyjson_obj_get(bufferView_val, "extras");

        bufferViews.push_back(bufferView);
    }

    std::cout << "Completed parsing Buffer Views." << std::endl; // Debug message after parsing
    for (size_t i = 0; i < bufferViews.size(); ++i) {
        if (showDebug) printBufferViewInfo(bufferViews[i], i);
    }
}

void GLTFAccessor::parseBuffers(yyjson_val* buffersArray) {
    std::cout << "Parsing Buffers..." << std::endl; // Debug message

    size_t idx, max;
    yyjson_val* buffer_val;
    yyjson_arr_foreach(buffersArray, idx, max, buffer_val) {
        Buffer buffer;

        yyjson_val* uri_val = yyjson_obj_get(buffer_val, "uri");
        buffer.uri = uri_val ? yyjson_get_str(uri_val) : "";

        yyjson_val* byteLength_val = yyjson_obj_get(buffer_val, "byteLength");
        size_t byteLength = byteLength_val ? yyjson_get_uint(byteLength_val) : 0;
        if (byteLength == 0) {
            std::cerr << "Warning: Buffer [" << idx << "] has an invalid or missing byteLength." << std::endl;
        }

        buffer.data.resize(byteLength);

        // Assuming the uri points to a local file, load the file into the data vector
        if (!buffer.uri.empty()) {
            std::ifstream file(buffer.uri, std::ios::binary);
            if (file) {
                file.read(reinterpret_cast<char*>(buffer.data.data()), byteLength);
                file.close();
            }
            else {
                std::cerr << "Error: Could not open buffer file " << buffer.uri << std::endl;
            }
        }

        buffer.extensions = yyjson_obj_get(buffer_val, "extensions");
        buffer.extras = yyjson_obj_get(buffer_val, "extras");

        buffers.push_back(buffer);
    }

    std::cout << "Completed parsing Buffers." << std::endl; // Debug message after parsing
    for (size_t i = 0; i < buffers.size(); ++i) {
        if (showDebug) printBufferInfo(buffers[i], i);
    }
}

const std::vector<GLTFAccessor::Accessor>& GLTFAccessor::getAccessors() const {
    return accessors;
}

void GLTFAccessor::printAccessorInfo(const Accessor& accessor, size_t index) {
    std::cout << "Accessor Info [" << index << "]:" << std::endl;
    std::cout << "Buffer View: " << accessor.bufferView << std::endl;
    std::cout << "Byte Offset: " << accessor.byteOffset << std::endl;
    std::cout << "Component Type: " << getComponentTypeName(accessor.componentType) << std::endl;
    std::cout << "Normalized: " << (accessor.normalized ? "true" : "false") << std::endl;
    std::cout << "Count: " << accessor.count << std::endl;
    std::cout << "Type: " << getTypeName(accessor.type) << std::endl;
    std::cout << "Max: " << (accessor.max.empty() ? "N/A" : glm::to_string(accessor.max[0])) << std::endl;
    std::cout << "Min: " << (accessor.min.empty() ? "N/A" : glm::to_string(accessor.min[0])) << std::endl;
    std::cout << "Sparse: " << (accessor.sparse ? "true" : "false") << std::endl;
    std::cout << "Name: " << accessor.name << std::endl;
}

void GLTFAccessor::printBufferViewInfo(const BufferView& bufferView, size_t index) {
    std::cout << "Buffer View Info [" << index << "]:" << std::endl;
    std::cout << "Buffer: " << bufferView.buffer << std::endl;
    std::cout << "Byte Offset: " << bufferView.byteOffset << std::endl;
    std::cout << "Byte Length: " << bufferView.byteLength << std::endl;
    std::cout << "Byte Stride: " << bufferView.byteStride << std::endl;
}

void GLTFAccessor::printBufferInfo(const Buffer& buffer, size_t index) {
    std::cout << "Buffer Info [" << index << "]:" << std::endl;
    std::cout << "URI: " << buffer.uri << std::endl;
    std::cout << "Data Size: " << buffer.data.size() << std::endl;
}

std::string GLTFAccessor::getComponentTypeName(int componentType) {
    switch (componentType) {
    case 5120: return "BYTE";
    case 5121: return "UNSIGNED_BYTE";
    case 5122: return "SHORT";
    case 5123: return "UNSIGNED_SHORT";
    case 5125: return "UNSIGNED_INT";
    case 5126: return "FLOAT";
    default: return "UNKNOWN";
    }
}

std::string GLTFAccessor::getTypeName(const std::string& type) {
    if (type == "SCALAR") return "SCALAR";
    if (type == "VEC2") return "VEC2";
    if (type == "VEC3") return "VEC3";
    if (type == "VEC4") return "VEC4";
    if (type == "MAT2") return "MAT2";
    if (type == "MAT3") return "MAT3";
    if (type == "MAT4") return "MAT4";
    return "UNKNOWN";
}
