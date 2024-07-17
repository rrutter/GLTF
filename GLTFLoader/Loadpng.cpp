#include "Loadpng.h"

void LoadPNG::closeFile(FILE* file) const {
    if (file) {
        fclose(file);
    }
}

unsigned char* LoadPNG::loadFile(const char* filename)
{
    FILE* rawFile = nullptr;
    errno_t err = fopen_s(&rawFile, filename, "rb");
    if (err != 0)
    {
        throw std::runtime_error("File doesn't exist: " + std::string(filename));
    }

    auto fileDeleter = [this](FILE* file) { closeFile(file); };
    std::unique_ptr<FILE, decltype(fileDeleter)> file(rawFile, fileDeleter);

    unsigned char header[8];
    png_structp pngPtr = nullptr;
    png_infop infoPtr = nullptr;
    std::vector<png_bytep> rowPointers;
    std::unique_ptr<unsigned char[]> imageData;

    if (fread(header, 1, sizeof(header), file.get()) != sizeof(header) || !png_check_sig(header, sizeof(header)))
    {
        throw std::runtime_error("Unrecognized image format");
    }

    pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr)
    {
        throw std::runtime_error("Failed to create PNG read struct");
    }

    infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        png_destroy_read_struct(&pngPtr, NULL, NULL);
        throw std::runtime_error("Failed to create PNG info struct");
    }

    if (setjmp(png_jmpbuf(pngPtr)))
    {
        png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
        throw std::runtime_error("Invalid data image");
    }

    png_set_read_fn(pngPtr, (png_voidp)file.get(), NULL);
    png_set_sig_bytes(pngPtr, 8);

    png_read_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL);
    png_read_update_info(pngPtr, infoPtr);
    png_uint_32 rowBytes = png_get_rowbytes(pngPtr, infoPtr);

    imageData = std::make_unique<unsigned char[]>(rowBytes * height);
    rowPointers.resize(height);

    for (png_uint_32 i = 0; i < height; ++i)
    {
        rowPointers[i] = imageData.get() + i * rowBytes;
    }

    png_read_image(pngPtr, rowPointers.data());

    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);

    data = imageData.release();
    return data;
}

std::unique_ptr<unsigned char[]> LoadPNG::loadFromMemory(const unsigned char* data, size_t length) {
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr) {
        throw std::runtime_error("Failed to create PNG read struct");
    }

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        png_destroy_read_struct(&pngPtr, NULL, NULL);
        throw std::runtime_error("Failed to create PNG info struct");
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
        throw std::runtime_error("Invalid data image");
    }

    std::unique_ptr<unsigned char[]> imageData;
    std::vector<png_bytep> rowPointers;

    png_set_read_fn(pngPtr, (png_voidp)&data, [](png_structp pngPtr, png_bytep outBytes, png_size_t byteCountToRead) {
        auto data = (const unsigned char**)png_get_io_ptr(pngPtr);
        memcpy(outBytes, *data, byteCountToRead);
        *data += byteCountToRead;
        });

    png_read_info(pngPtr, infoPtr);
    png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL);
    png_read_update_info(pngPtr, infoPtr);
    png_uint_32 rowBytes = png_get_rowbytes(pngPtr, infoPtr);

    imageData = std::make_unique<unsigned char[]>(rowBytes * height);
    rowPointers.resize(height);

    for (png_uint_32 i = 0; i < height; ++i) {
        rowPointers[i] = imageData.get() + i * rowBytes;
    }

    png_read_image(pngPtr, rowPointers.data());

    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);

    return imageData;
}

// Getter methods
png_uint_32 LoadPNG::getWidth() const {
    return width;
}

png_uint_32 LoadPNG::getHeight() const {
    return height;
}

int LoadPNG::getColorType() const {
    return colorType;
}

int LoadPNG::getBitDepth() const {
    return bitDepth;
}