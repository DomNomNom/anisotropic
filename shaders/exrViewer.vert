#version 330

#include utils.glsl

layout(location = 0)in vec4 vertex;
// layout(location = 1)in vec4 normal;

smooth out vec4 pos_uv;
smooth out vec4 pos_project;


void main() {
    // pos_uv = 0.5*(vertex+0.5);
    pos_uv = to01(vertex);
    // pos_uv.y *= -1.0;
    pos_project = vertex;
    gl_Position = vertex;
}
