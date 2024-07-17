#ifndef GLTFAccessOR_H
#define GLTFAccessOR_H
#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "yyjson.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <fstream>

class GLTFAccessor {
public:
    struct Accessor {
        int bufferView;
        size_t byteOffset;
        int componentType;
        bool normalized;
        size_t count;
        std::string type;
        std::vector<glm::vec3> max;
        std::vector<glm::vec3> min;
        bool sparse;
        std::string name;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    struct BufferView {
        int buffer;
        size_t byteOffset;
        size_t byteLength;
        size_t byteStride;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    struct Buffer {
        std::vector<unsigned char> data;
        std::string uri;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    void parseAccessors(yyjson_val* accessorsArray);
    void parseBufferViews(yyjson_val* bufferViewsArray);
    void parseBuffers(yyjson_val* buffersArray);

    const std::vector<Accessor>& getAccessors() const;

private:
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;
    std::vector<Buffer> buffers;

    void printAccessorInfo(const Accessor& accessor, size_t index);
    void printBufferViewInfo(const BufferView& bufferView, size_t index);
    void printBufferInfo(const Buffer& buffer, size_t index);

    std::string getComponentTypeName(int componentType);
    std::string getTypeName(const std::string& type);

    bool showDebug = false;
};

#endif // GLTFAccessOR_H
