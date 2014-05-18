#pragma once


#include <ImfRgbaFile.h>

GLuint png_texture_load(const char *file_name);
GLuint png_cubemap_load(const char *base_name);
GLuint exr_texture_load(const char *file_name);

void exr_texture_save(const char *fileName, const Imf::Rgba *pixels, int width, int height);
