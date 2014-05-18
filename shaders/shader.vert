#version 330

layout(location = 0)in vec4 vertex;
layout(location = 1)in vec4 normal;
// in vec3 normal;

smooth out vec4 pos_model;
smooth out vec4 pos_world;
smooth out vec4 pos_view;
smooth out vec4 pos_project;

// smooth out vec4 vertex_normal;
smooth out vec4 lights[4];

uniform float time;
uniform vec2 mouse;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform vec4 lightVectors[4];


void main() {

    // we could optimize this by making them producs of each other, but this looks nicer
    pos_model   =                                               vertex;
    pos_world   =                                 modelMatrix * vertex;
    pos_view    =                    viewMatrix * modelMatrix * vertex;
    pos_project = projectionMatrix * viewMatrix * modelMatrix * vertex;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertex;

    for (int i=0; i<4; ++i) {
        lights[i] = viewMatrix * lightVectors[i];
    }

}
