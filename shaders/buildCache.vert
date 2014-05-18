#version 330

#include to01.glsl

layout(location = 0)in vec4 vertex;
// layout(location = 1)in vec4 normal;

smooth out vec4 pos_uv;
smooth out vec4 pos_project;


void main() {
    pos_uv = to01(vertex);
    pos_project = vertex;
    gl_Position = vertex;
}
