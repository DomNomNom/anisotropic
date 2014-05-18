#version 330

out vec4 fragColor;

#include constants.glsl
#include sampleLightmap.frag
// #include to01.glsl
#include rotationMatrix.glsl

smooth in vec4 pos_screen;
smooth in vec4 pos_project;

uniform float hello;
uniform float range_tangent;
uniform float range_bitangent;

void main() {
    float alpha = pos_screen.x * 2.0 * pi;
    float beta  = pos_screen.y * 2.0 * pi;  // counter clockwise when viewed from +x

    // float alpha = pos_project.x * pi;
    // float beta  = pos_project.y * pi;  // counter clockwise when viewed from +x
    float gamma = hello;

    // float alpha = pos.x * 1.0 * pi;
    // float beta  = 0.0 * pi;  // counter clockwise when viewed from +x
    // float gamma = pos.y * 0.5 * pi;

    vec3 normal    = vec3(1.0, 0.0,  0.0);
    vec3 tangent   = vec3(0.0, 0.0, -1.0);
    vec3 bitangent = cross(normal, tangent);

    mat3 alphaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), alpha);
    mat3 betaTransform  = rotationMatrix3(alphaTransform * normal, beta);
    mat3 gammaTransform = rotationMatrix3(
        betaTransform * alphaTransform * bitangent,
        gamma
    );


    fragColor = sample(gammaTransform * betaTransform * alphaTransform * normal);
    // fragColor = 1.0 - exp(-1 * fragColor);
    // fragColor = vec4(hello, 0.0, 0.0, 1.0);

    // fragColor = sample(normalize(vec3(1.0, pos.y, 0.0)));
    // float y = cos(pos_screen.y);
    // fragColor = sample(normalize(vec3(1.0, pos.y, )))

    // fragColor = vec4(pos_screen.x, pos_screen.y, hello, 1.0);
}
