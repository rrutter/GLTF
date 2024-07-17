#ifndef LOADPNG_H
#define LOADPNG_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <memory>
#include <stdexcept>
#include <png.h>

class LoadPNG
{
public:
	unsigned char *loadFile(const char *filename);

	std::unique_ptr<unsigned char[]> loadFromMemory(const unsigned char* data, size_t length);


	// Public getters
	png_uint_32 getWidth() const;
	png_uint_32 getHeight() const;
	int getColorType() const;
	int getBitDepth() const;
private:
	void closeFile(FILE* file) const;

	// Private members
	png_uint_32 width;
	png_uint_32 height;
	int colorType;
	int bitDepth;
	unsigned char* data;
};

#endif