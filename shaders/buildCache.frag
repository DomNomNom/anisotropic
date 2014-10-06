#version 330

out vec4 fragColor;

smooth in vec4 pos_uv;
smooth in vec4 pos_project;

uniform float time;

bool error = false;

#include constants.glsl
#include random.glsl
#include sampleLightmap.frag
#include utils.glsl
#include rotationMatrix.glsl
#include angles.glsl

uniform float gammaTexCoord;
uniform float range_tangent;
uniform float range_bitangent;

// const uint gammaSamples = 500u;
// const float gammaVariance = 0.5;
const uint gammaSamples = 10u;
const float gammaVariance = 0.001;



void main() {
    vec3 ABG_angles = getAngles(pos_uv.x, pos_uv.y, gammaTexCoord);
    float alpha = ABG_angles.x;
    float beta  = ABG_angles.y;  // counter clockwise when viewed from +x
    float gamma = ABG_angles.z;

    vec3 sampleDir = vec3(1.0, 0.0,  0.0); // r
    vec3 tangent   = vec3(0.0, 0.0, -1.0);
    vec3 bitangent = cross(sampleDir, tangent); // (0, 1, 0)

    mat3 alphaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), alpha);
    mat3 betaTransform  = rotationMatrix3(alphaTransform * sampleDir, beta);
    // mat3 gammaTransform = rotationMatrix3(
    //     betaTransform * alphaTransform * bitangent, // ?
    //     gamma
    // );
    // fragColor = sample(gammaTransform * betaTransform * alphaTransform * sampleDir);

    fragColor = vec4(0.0);  // accumulator
    if (gammaSamples < 1u) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);  // I am error
    }
    for (uint i=0u; i<gammaSamples; ++i) {
        mat3 gammaTransform = rotationMatrix3(
            betaTransform * alphaTransform * bitangent,
            gamma + gammaVariance * rand()
        );
        fragColor += sample(gammaTransform * betaTransform * alphaTransform * sampleDir);
    }
    fragColor /= float(gammaSamples);

    // fragColor = texture(lightmap_hdr, pos_uv.xy);
    // fragColor = 1.0 - exp(-gamma*100 * fragColor);
    // fragColor = vec4(gamma, 0.0, 0.0, 1.0);

    // fragColor = sample(normalize(vec3(1.0, pos.y, 0.0)));
    // float y = cos(pos_uv.y);
    // fragColor = sample(normalize(vec3(1.0, pos.y, )))

    // fragColor = vec4(pos_uv.x, pos_uv.y, gamma, 1.0);
    // fragColor =
    if (error) {
        fragColor = vec4(1.0, 0.0, 1.0, 1.0);;
    }
}
