#ifndef GLTFMESH_H
#define GLTFMESH_H
#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "yyjson.h"
#include "Vertex.h"

class GLTFMesh {
public:
    struct MorphTarget {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
    };

    struct Primitive {
        int positionAccessor;
        int normalAccessor;
        int texcoordAccessor;
        int colorAccessor;
        int indicesAccessor;
        int materialIndex;
        int jointsAccessor;
        int weightsAccessor;
        std::vector<MorphTarget> morphTargets;
        yyjson_val* extensions = nullptr;
        yyjson_val* extras = nullptr;
    };

    struct Mesh {
        std::string name;
        std::vector<Primitive> primitives;
        yyjson_val* extensions = nullptr;
        yyjson_val* extras = nullptr;
    };

    struct Skin {
        std::vector<int> joints;
        int inverseBindMatricesAccessor;
        yyjson_val* extensions = nullptr;
        yyjson_val* extras = nullptr;
    };

    void parseMeshes(yyjson_val* meshesArray);
    void parseSkins(yyjson_val* skinsArray);
    const std::vector<Mesh>& getMeshes() const;
    const std::vector<Skin>& getSkins() const;

    // New helper functions
    bool hasMorphTargets(int meshIndex, int primitiveIndex) const;
    const std::vector<glm::vec3>& getMorphTargetPositions(int meshIndex, int primitiveIndex, int targetIndex) const;
    const std::vector<glm::vec3>& getMorphTargetNormals(int meshIndex, int primitiveIndex, int targetIndex) const;
    const std::vector<MorphTarget>& getMorphTargets(int meshIndex, int primitiveIndex) const;

    const std::vector<GLTFMesh::Primitive> getPrimitives() const;

private:
    std::vector<Mesh> meshes;
    std::vector<Skin> skins;
    std::vector<Vertex> vertices;

    void parsePrimitive(Primitive& primitive, yyjson_val* primitive_val);
    void parseMorphTarget(MorphTarget& morphTarget, yyjson_val* morph_val);
    void printMeshInfo(const Mesh& mesh, size_t index) const;
    void printSkinInfo(const Skin& skin, size_t index) const;
    bool showDebug = true;
};


#endif // GLTFMESH_H
