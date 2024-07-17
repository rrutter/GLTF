#ifndef VERTEX_H
#define VERTEX_H

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 weights;
    glm::ivec4 joints;

    Vertex(
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& norm = glm::vec3(0.0f),
        const glm::vec2& tex = glm::vec2(0.0f),
        const glm::vec4& w = glm::vec4(0.0f),
        const glm::ivec4& j = glm::ivec4(0)
    ) : position(pos), normal(norm), texCoord(tex), weights(w), joints(j) {}
};

#endif // VERTEX_H