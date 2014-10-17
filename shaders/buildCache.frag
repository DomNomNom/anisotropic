#version 330

out vec4 fragColor;

smooth in vec4 pos_uv;
// smooth in vec4 pos_project;

// uniform float time;

bool error = false;

#include constants.glsl
// #include random.glsl
// #include utils.glsl
// #include rotationMatrix.glsl
// #include angles.glsl
#include sampleLightmap.frag

uniform float gammaTexCoord;
uniform float range_tangent;
uniform float range_bitangent;


void main() {
    vec3 texCoords = vec3(pos_uv.x, pos_uv.y, gammaTexCoord);
    fragColor = accumulateSamples(texCoords);  // this is the expensive bit

    if (error) {
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }

}
