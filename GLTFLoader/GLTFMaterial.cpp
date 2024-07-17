#include "GLTFMaterial.h"

extern LoadPNG png;

void GLTFMaterial::parseMaterials(yyjson_val* materialsArray, yyjson_val* texturesArray, yyjson_val* imagesArray) {
    size_t idx, max;
    yyjson_val* val;

    // Parse materials
    yyjson_arr_foreach(materialsArray, idx, max, val) {
        parseMaterial(val);
    }

    // Parse textures
    yyjson_arr_foreach(texturesArray, idx, max, val) {
        parseTexture(val);
    }

    // Parse images
    yyjson_arr_foreach(imagesArray, idx, max, val) {
        parseImage(val);
    }
}

void GLTFMaterial::parseMaterial(yyjson_val* material_val) {
    Material material;
    material.name = yyjson_get_str(yyjson_obj_get(material_val, "name")) ? yyjson_get_str(yyjson_obj_get(material_val, "name")) : "";
    yyjson_val* pbrMetallicRoughness = yyjson_obj_get(material_val, "pbrMetallicRoughness");
    if (pbrMetallicRoughness) {
        yyjson_val* baseColorTexture = yyjson_obj_get(pbrMetallicRoughness, "baseColorTexture");
        if (baseColorTexture) {
            material.baseColorTextureIndex = yyjson_get_int(yyjson_obj_get(baseColorTexture, "index"));
        }
    }
    materials.push_back(material);

    std::cout << "Parsed Material: " << material.name << ", Base Color Texture Index: " << material.baseColorTextureIndex << std::endl;
}

void GLTFMaterial::parseTexture(yyjson_val* texture_val) {
    Texture texture;
    texture.sampler = yyjson_get_int(yyjson_obj_get(texture_val, "sampler"));
    texture.source = yyjson_get_int(yyjson_obj_get(texture_val, "source"));
    texture.name = yyjson_get_str(yyjson_obj_get(texture_val, "name")) ? yyjson_get_str(yyjson_obj_get(texture_val, "name")) : "";
    textures.push_back(texture);

    std::cout << "Parsed Texture: Sampler: " << texture.sampler << ", Source: " << texture.source << ", Name: " << texture.name << std::endl;
}

void GLTFMaterial::parseImage(yyjson_val* image_val) {
    Image image;
    image.uri = yyjson_get_str(yyjson_obj_get(image_val, "uri")) ? yyjson_get_str(yyjson_obj_get(image_val, "uri")) : "";
    image.bufferView = yyjson_get_int(yyjson_obj_get(image_val, "bufferView"));
    image.mimeType = yyjson_get_str(yyjson_obj_get(image_val, "mimeType")) ? yyjson_get_str(yyjson_obj_get(image_val, "mimeType")) : "";
    images.push_back(image);

    std::cout << "Parsed Image: URI: " << image.uri << ", Buffer View: " << image.bufferView << ", MIME Type: " << image.mimeType << std::endl;
}

void GLTFMaterial::loadImageData(GLTFBuffer& bufferManager) {
    const auto& bufferViews = bufferManager.getBufferViews();
    const auto& buffers = bufferManager.getBuffers();

    for (auto& image : images) {
        if (image.mimeType == "image/png") {
            if (image.uri.empty() && image.bufferView >= 0 && image.bufferView < bufferViews.size()) {
                const auto& bufferView = bufferViews[image.bufferView];
                if (bufferView.buffer >= 0 && bufferView.buffer < buffers.size()) {
                    const auto& buffer = buffers[bufferView.buffer];
                    size_t byteOffset = bufferView.byteOffset;
                    size_t byteLength = bufferView.byteLength;

                    std::cout << "Buffer View Index: " << image.bufferView << ", Buffer Index: " << bufferView.buffer << std::endl;
                    std::cout << "Byte Offset: " << byteOffset << ", Byte Length: " << byteLength << std::endl;
                    std::cout << "Buffer Data Size: " << buffer.data.size() << std::endl;

                    if (byteOffset + byteLength <= buffer.data.size()) {
                        try {
                            LoadPNG loader;
                            std::unique_ptr<unsigned char[]> rawData = loader.loadFromMemory(buffer.data.data() + byteOffset, byteLength);
                            size_t dataSize = loader.getWidth() * loader.getHeight() * (loader.getColorType() == PNG_COLOR_TYPE_RGB ? 3 : 4);

                            image.data.assign(rawData.get(), rawData.get() + dataSize);
                            image.width = loader.getWidth();
                            image.height = loader.getHeight();
                            image.colorType = loader.getColorType();
                            image.bitDepth = loader.getBitDepth();

                            std::cout << "Loaded embedded PNG image: " << image.bufferView << " (" << image.width << "x" << image.height << ")" << std::endl;
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Error loading embedded PNG image: " << e.what() << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Error: Buffer overflow when accessing image data for buffer view: " << image.bufferView << std::endl;
                    }
                }
                else {
                    std::cerr << "Error: Invalid buffer index in buffer view: " << bufferView.buffer << std::endl;
                }
            }
            else if (!image.uri.empty()) {
                try {
                    LoadPNG loader;
                    unsigned char* rawData = loader.loadFile(image.uri.c_str());
                    size_t dataSize = loader.getWidth() * loader.getHeight() * (loader.getColorType() == PNG_COLOR_TYPE_RGB ? 3 : 4);

                    image.data.assign(rawData, rawData + dataSize);
                    image.width = loader.getWidth();
                    image.height = loader.getHeight();
                    image.colorType = loader.getColorType();
                    image.bitDepth = loader.getBitDepth();

                    delete[] rawData;  // Remember to delete the raw data after copying

                    std::cout << "Loaded PNG image: " << image.uri << " (" << image.width << "x" << image.height << ")" << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Error loading PNG image: " << e.what() << std::endl;
                }
            }
        }
        else {
            std::cerr << "Unsupported image MIME type: " << image.mimeType << std::endl;
        }
    }
}







const std::vector<GLTFMaterial::Material>& GLTFMaterial::getMaterials() const {
    return materials;
}

const std::vector<GLTFMaterial::Texture>& GLTFMaterial::getTextures() const {
    return textures;
}

const std::vector<GLTFMaterial::Image>& GLTFMaterial::getImages() const {
    return images;
}
