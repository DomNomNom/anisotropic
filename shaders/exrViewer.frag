#version 330

out vec4 fragColor;

#include constants.glsl

smooth in vec4 pos_uv;
smooth in vec4 pos_project;

uniform float exposure;
uniform float tester;

uniform bool useCache;
uniform sampler2D exr;
uniform sampler3D cache;
uniform float texcoord;

void main() {
    if (useCache) {
        fragColor = texture(cache, vec3(
            texcoord,
            pos_uv.y,
            pos_uv.x
        ));
    }
    else {
        fragColor = texture(exr, pos_uv.xy);
    }
    fragColor = 1.0 - exp(-exposure * fragColor);

    // fragColor = vec4(
    //     pos_uv.x,
    //     pos_uv.y,
    //     texcoord,
    //     1.0
    // );
}
