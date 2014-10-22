#include utils.glsl
#include random.glsl
#include angles.glsl

uniform samplerCube lightmap;
uniform sampler2D lightmap_hdr;
uniform float anisotropy;
uniform int numSamples;

// returns incoming intensity of lightmap_hdr (spherical mapping)
vec4 sampleHdrLightmap(vec3 v) {
    // TODO: avoid NaNs when abs(v.z)+abs(v.x) == 0
    v = normalize(v);
    return texture(lightmap_hdr, vec2(
        ( 0.5 / pi) * atan(v.z, v.x),   // horizontal angle
        (-1.0 / pi) * asin(v.y) - 0.5   // vertical angle
    ));
}

// takes a lightmap sample in the given direction
// this function exists to make it easy to switch between lightmaps
vec4 sample(vec3 dir) {
    return sampleHdrLightmap(dir);
    return texture(lightmap, dir);  // cubemap
}

// accumulates random samples on the fan described by the texCoords
vec4 accumulateSamples(vec3 texCoords) {

    // assert 0 <= texCoords <= 1
    if (!(
        0.0 <= texCoords.x && texCoords.x <= 1.0 &&
        0.0 <= texCoords.y && texCoords.y <= 1.0 &&
        0.0 <= texCoords.z && texCoords.z <= 1.0 &&
        true
    )) {
        error = true;
    }

    // assert that our mapping is invertible
    Fan fan = makeFan(texCoords);
    vec3 testTexCoords = makeTexCoords(fan);
    vec3 texCoordsDiff = texCoords - testTexCoords;
    if (!isZero(texCoordsDiff)) {
        error = true;
        return vec4(to01(texCoordsDiff), 1.0);
    }

    // you should really sample more than 0 times...
    if (numSamples < 1) {
        error = true;
        return vec4(0.0, 1.0, 0.0, 1.0);
    }


    // vary samples along the arc
    vec4 accumulator = vec4(0.0);
    for (int i=0; i<numSamples; ++i) {
        vec3 sampleCoord = texCoords;
        sampleCoord.z += anisotropy * rand();  // vary the gamma angle
        accumulator += sample(
            makeFan(sampleCoord).dir
        );
    }
    return accumulator / float(numSamples);
}
