#pragma once


GLuint png_texture_load(const char *file_name);
GLuint png_cubemap_load(const char *base_name);
GLuint exr_texture_load(const char *file_name); // loads into a float texture

void exr_texture_save(const char *fileName, const float *pixels, int width, int height);
