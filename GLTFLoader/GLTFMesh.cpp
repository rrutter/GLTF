#include "GLTFMesh.h"
#include <iostream>

void GLTFMesh::parseMeshes(yyjson_val* meshesArray) {
    size_t idx, max;
    yyjson_val* mesh_val;
    yyjson_arr_foreach(meshesArray, idx, max, mesh_val) {
        Mesh mesh;
        yyjson_val* name_val = yyjson_obj_get(mesh_val, "name");
        mesh.name = name_val ? yyjson_get_str(name_val) : "";

        yyjson_val* primitives_val = yyjson_obj_get(mesh_val, "primitives");
        if (primitives_val && yyjson_is_arr(primitives_val)) {
            size_t prim_idx, prim_max;
            yyjson_val* primitive_val;
            yyjson_arr_foreach(primitives_val, prim_idx, prim_max, primitive_val) {
                Primitive primitive;
                parsePrimitive(primitive, primitive_val);
                mesh.primitives.push_back(primitive);
            }
        }

        mesh.extensions = yyjson_obj_get(mesh_val, "extensions");
        mesh.extras = yyjson_obj_get(mesh_val, "extras");

        if (showDebug) printMeshInfo(mesh, idx);
        meshes.push_back(mesh);
    }
}

void GLTFMesh::parseSkins(yyjson_val* skinsArray) {
    size_t idx, max;
    yyjson_val* skin_val;
    yyjson_arr_foreach(skinsArray, idx, max, skin_val) {
        Skin skin;
        yyjson_val* joints_val = yyjson_obj_get(skin_val, "joints");
        if (joints_val && yyjson_is_arr(joints_val)) {
            size_t joint_idx, joint_max;
            yyjson_val* joint_val;
            yyjson_arr_foreach(joints_val, joint_idx, joint_max, joint_val) {
                skin.joints.push_back(yyjson_get_int(joint_val));
            }
        }

        yyjson_val* ibm_acc = yyjson_obj_get(skin_val, "inverseBindMatrices");
        skin.inverseBindMatricesAccessor = ibm_acc ? yyjson_get_int(ibm_acc) : -1;

        skin.extensions = yyjson_obj_get(skin_val, "extensions");
        skin.extras = yyjson_obj_get(skin_val, "extras");

        if (showDebug) printSkinInfo(skin, idx);
        skins.push_back(skin);
    }
}

void GLTFMesh::parsePrimitive(Primitive& primitive, yyjson_val* primitive_val) {
    yyjson_val* attributes_val = yyjson_obj_get(primitive_val, "attributes");
    if (attributes_val) {
        yyjson_val* pos_acc = yyjson_obj_get(attributes_val, "POSITION");
        primitive.positionAccessor = pos_acc ? yyjson_get_int(pos_acc) : -1;

        yyjson_val* norm_acc = yyjson_obj_get(attributes_val, "NORMAL");
        primitive.normalAccessor = norm_acc ? yyjson_get_int(norm_acc) : -1;

        yyjson_val* tex_acc = yyjson_obj_get(attributes_val, "TEXCOORD_0");
        primitive.texcoordAccessor = tex_acc ? yyjson_get_int(tex_acc) : -1;

        yyjson_val* col_acc = yyjson_obj_get(attributes_val, "COLOR_0");
        primitive.colorAccessor = col_acc ? yyjson_get_int(col_acc) : -1;

        yyjson_val* joints_acc = yyjson_obj_get(attributes_val, "JOINTS_0");
        primitive.jointsAccessor = joints_acc ? yyjson_get_int(joints_acc) : -1;

        yyjson_val* weights_acc = yyjson_obj_get(attributes_val, "WEIGHTS_0");
        primitive.weightsAccessor = weights_acc ? yyjson_get_int(weights_acc) : -1;
    }

    yyjson_val* ind_acc = yyjson_obj_get(primitive_val, "indices");
    primitive.indicesAccessor = ind_acc ? yyjson_get_int(ind_acc) : -1;

    yyjson_val* mat = yyjson_obj_get(primitive_val, "material");
    primitive.materialIndex = mat ? yyjson_get_int(mat) : -1;

    // Parse morph targets
    yyjson_val* morphTargets_val = yyjson_obj_get(primitive_val, "targets");
    if (morphTargets_val && yyjson_is_arr(morphTargets_val)) {
        size_t morph_idx, morph_max;
        yyjson_val* morph_val;
        yyjson_arr_foreach(morphTargets_val, morph_idx, morph_max, morph_val) {
            MorphTarget morphTarget;
            parseMorphTarget(morphTarget, morph_val);
            primitive.morphTargets.push_back(morphTarget);
        }
    }

    primitive.extensions = yyjson_obj_get(primitive_val, "extensions");
    primitive.extras = yyjson_obj_get(primitive_val, "extras");
}

void GLTFMesh::parseMorphTarget(MorphTarget& morphTarget, yyjson_val* morph_val) {
    yyjson_val* pos_acc = yyjson_obj_get(morph_val, "POSITION");
    if (pos_acc) {
        // Load positions
        // This is a placeholder, replace with actual data loading
        morphTarget.positions.push_back(glm::vec3(0.0f)); // Replace with actual loading logic
    }

    yyjson_val* norm_acc = yyjson_obj_get(morph_val, "NORMAL");
    if (norm_acc) {
        // Load normals
        // This is a placeholder, replace with actual data loading
        morphTarget.normals.push_back(glm::vec3(0.0f)); // Replace with actual loading logic
    }
}

bool GLTFMesh::hasMorphTargets(int meshIndex, int primitiveIndex) const {
    if (meshIndex < 0 || meshIndex >= meshes.size()) {
        throw std::out_of_range("Invalid mesh index");
    }
    const Mesh& mesh = meshes[meshIndex];
    if (primitiveIndex < 0 || primitiveIndex >= mesh.primitives.size()) {
        throw std::out_of_range("Invalid primitive index");
    }
    return !mesh.primitives[primitiveIndex].morphTargets.empty();
}

const std::vector<glm::vec3>& GLTFMesh::getMorphTargetPositions(int meshIndex, int primitiveIndex, int targetIndex) const {
    if (meshIndex < 0 || meshIndex >= meshes.size()) {
        throw std::out_of_range("Invalid mesh index");
    }
    const Mesh& mesh = meshes[meshIndex];
    if (primitiveIndex < 0 || primitiveIndex >= mesh.primitives.size()) {
        throw std::out_of_range("Invalid primitive index");
    }
    const Primitive& primitive = mesh.primitives[primitiveIndex];
    if (targetIndex < 0 || targetIndex >= primitive.morphTargets.size()) {
        throw std::out_of_range("Invalid morph target index");
    }
    return primitive.morphTargets[targetIndex].positions;
}

const std::vector<glm::vec3>& GLTFMesh::getMorphTargetNormals(int meshIndex, int primitiveIndex, int targetIndex) const {
    if (meshIndex < 0 || meshIndex >= meshes.size()) {
        throw std::out_of_range("Invalid mesh index");
    }
    const Mesh& mesh = meshes[meshIndex];
    if (primitiveIndex < 0 || primitiveIndex >= mesh.primitives.size()) {
        throw std::out_of_range("Invalid primitive index");
    }
    const Primitive& primitive = mesh.primitives[primitiveIndex];
    if (targetIndex < 0 || targetIndex >= primitive.morphTargets.size()) {
        throw std::out_of_range("Invalid morph target index");
    }
    return primitive.morphTargets[targetIndex].normals;
}

const std::vector<GLTFMesh::MorphTarget>& GLTFMesh::getMorphTargets(int meshIndex, int primitiveIndex) const {
    if (meshIndex < 0 || meshIndex >= meshes.size()) {
        throw std::out_of_range("Invalid mesh index");
    }
    const Mesh& mesh = meshes[meshIndex];
    if (primitiveIndex < 0 || primitiveIndex >= mesh.primitives.size()) {
        throw std::out_of_range("Invalid primitive index");
    }
    return mesh.primitives[primitiveIndex].morphTargets;
}

const std::vector<GLTFMesh::Primitive> GLTFMesh::getPrimitives() const {
    std::vector<Primitive> allPrimitives;
    for (const auto& mesh : meshes) {
        for (const auto& primitive : mesh.primitives) {
            allPrimitives.push_back(primitive);
        }
    }
    return allPrimitives;
}

void GLTFMesh::printMeshInfo(const Mesh& mesh, size_t index) const {
    std::cout << "Mesh Info [" << index << "]:" << std::endl;
    std::cout << "Name: " << (mesh.name.empty() ? "None" : mesh.name) << std::endl;
    std::cout << "Primitives: " << mesh.primitives.size() << std::endl;
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        const auto& primitive = mesh.primitives[i];
        std::cout << "  Primitive [" << i << "]:" << std::endl;
        std::cout << "    Position Accessor: " << primitive.positionAccessor << std::endl;
        std::cout << "    Normal Accessor: " << primitive.normalAccessor << std::endl;
        std::cout << "    Texcoord Accessor: " << primitive.texcoordAccessor << std::endl;
        std::cout << "    Color Accessor: " << primitive.colorAccessor << std::endl;
        std::cout << "    Indices Accessor: " << primitive.indicesAccessor << std::endl;
        std::cout << "    Material Index: " << primitive.materialIndex << std::endl;
        std::cout << "    Morph Targets: " << primitive.morphTargets.size() << std::endl;
        for (size_t j = 0; j < primitive.morphTargets.size(); ++j) {
            const auto& morphTarget = primitive.morphTargets[j];
            std::cout << "      Morph Target [" << j << "]:" << std::endl;
            std::cout << "        Positions: " << morphTarget.positions.size() << std::endl;
            std::cout << "        Normals: " << morphTarget.normals.size() << std::endl;
        }
        std::cout << "    Extensions: " << (primitive.extensions ? "Yes" : "No") << std::endl;
        std::cout << "    Extras: " << (primitive.extras ? "Yes" : "No") << std::endl;
    }
    std::cout << "Extensions: " << (mesh.extensions ? "Yes" : "No") << std::endl;
    std::cout << "Extras: " << (mesh.extras ? "Yes" : "No") << std::endl;
}

void GLTFMesh::printSkinInfo(const Skin& skin, size_t index) const {
    std::cout << "Skin Info [" << index << "]:" << std::endl;
    std::cout << "Joints: " << skin.joints.size() << std::endl;
    std::cout << "Inverse Bind Matrices Accessor: " << skin.inverseBindMatricesAccessor << std::endl;
    std::cout << "Extensions: " << (skin.extensions ? "Yes" : "No") << std::endl;
    std::cout << "Extras: " << (skin.extras ? "Yes" : "No") << std::endl;
}

const std::vector<GLTFMesh::Mesh>& GLTFMesh::getMeshes() const {
    return meshes;
}

const std::vector<GLTFMesh::Skin>& GLTFMesh::getSkins() const {
    return skins;
}