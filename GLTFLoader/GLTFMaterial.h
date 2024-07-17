#ifndef GLTFMATERIAL_H
#define GLTFMATERIAL_H

#include <string>
#include <vector>
#include "yyjson.h"
#include "GLTFBuffer.h"
#include <iostream>
#include <cstring>
#include "Loadpng.h"

class GLTFMaterial {
public:
    struct Material {
        std::string name;
        int baseColorTextureIndex;
    };

    struct Texture {
        int sampler;
        int source;
        std::string name;
    };

    struct Image {
        std::string uri;
        int bufferView;
        std::string mimeType;
        std::vector<unsigned char> data;
        png_uint_32 width;
        png_uint_32 height;
        int colorType;
        int bitDepth;
    };

    void parseMaterials(yyjson_val* materialsArray, yyjson_val* texturesArray, yyjson_val* imagesArray);
    void loadImageData(GLTFBuffer& bufferManager);

    const std::vector<Material>& getMaterials() const;
    const std::vector<Texture>& getTextures() const;
    const std::vector<Image>& getImages() const;

private:
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Image> images;

    void parseMaterial(yyjson_val* material_val);
    void parseTexture(yyjson_val* texture_val);
    void parseImage(yyjson_val* image_val);
};

#endif // GLTFMATERIAL_H
