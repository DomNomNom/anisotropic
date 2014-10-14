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
    vec4 sampled     = accumulateSamples(texCoords);  // this is the expensive bit
    fragColor = sampled;

    // debug
    Fan fan = makeFan(texCoords);
    vec3 testTexCoords = makeTexCoords(fan);

    vec4 sampledTest = accumulateSamples(testTexCoords);
    vec3 ret;
    // if (rand() > 0.0) { ret = vec3(sampled.x, 0.0,     0.0); }
    // else              { ret = vec3(0.0, sampledTest.x, 0.0); }
    // ret = vec3(0.0, sampledTest.x, 0.0);
    ret = to01(texCoords - testTexCoords);
    // ret = testTexCoords;
    // ret = texCoords;
    // ret = to01(fan.dir);
    // ret = texCoords;

    // fragColor = vec4(ret, 1.0);

    // if (!isClose(texCoords, testTexCoords)) {
    //     fragColor = vec4(to01(testTexCoords - texCoords), 1.0);
    // }

    if (error) {
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);
    }




}
