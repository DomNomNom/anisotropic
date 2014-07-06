#pragma once


GLuint png_texture_load(const char *file_name);
GLuint png_cubemap_load(const char *base_name);
GLuint exr_texture_load(const char *file_name); // loads into a float texture
GLuint exr_cubetex_load(const char *base_name, unsigned int r_depth); // loads into a float 3D texture

void exr_texture_save(const char *file_name, const float *pixels, int width, int height);
