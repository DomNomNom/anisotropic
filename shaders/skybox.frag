#version 330

out vec4 fragColor;

#include constants.glsl
#include sampleLightmap.frag

smooth in vec4 pos_view;
smooth in vec4 pos_world;
smooth in vec4 pos_model;
smooth in vec4 pos_screen;

smooth in vec4 vertex_normal;
smooth in vec4 lights[4];
// varying vec4 tangent;

uniform float exposure = 1.0;
uniform float time;
uniform float tester;
uniform vec2 mouse;
uniform vec4 eye;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;


vec4 pos2cam; // world space

void main() {
    pos2cam = eye - pos_world;

    fragColor = sample(-pos2cam.xyz);
    fragColor = 1.0 - exp(-exposure * fragColor);

    // fragColor = vec4(0.0, 1.0, 0.0, 1.0);

}
