#version 330

out vec4 fragColor;

#include constants.glsl

smooth in vec4 pos_uv;
smooth in vec4 pos_project;

uniform sampler2D exr;
uniform float exposure;
uniform float tester;

void main() {
    fragColor = texture(exr, pos_uv.xy);
    fragColor = 1.0 - exp(-exposure * fragColor);
}
