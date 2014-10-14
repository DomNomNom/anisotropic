#include utils.glsl
#include random.glsl
#include angles.glsl

uniform samplerCube lightmap;
uniform sampler2D lightmap_hdr;

vec4 sampleHdrLightmap(vec3 v) {
    // TODO: special case when abs(v.z)+abs(v.x) == 0
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
    return texture(lightmap, dir);
}

// const uint gammaSamples = 500u;
// const float gammaVariance = 0.5;
const uint gammaSamples = 10u;
const float gammaVariance = 0.001;
// accumulates random samples on the fan described by the texCoords
vec4 accumulateSamples(vec3 texCoords) {

    return vec4(texCoords, 1.0);

    // assert 0 <= texCoords <= 1
    if (!(
        0.0 <= texCoords.x && texCoords.x <= 1.0 &&
        0.0 <= texCoords.y && texCoords.y <= 1.0 &&
        0.0 <= texCoords.z && texCoords.z <= 1.0 &&
        true
    )) {
        // error = true;
    }

    Fan fan = makeFan(texCoords);
    vec3 testTexCoords = makeTexCoords(fan);

    vec3 texCoordsDiff = texCoords - testTexCoords;
    if (!isZero(texCoordsDiff)) {
        error = true;
    }

    // vec3 ret;
    // ret = to01(texCoordsDiff);
    // return vec4(ret, 1.0);

    return sample(fan.dir);  // only one sample in the fans firection





    // vary samples along the fans arc
    vec4 accumulator = vec4(0.0);
    if (gammaSamples < 1u) {
        accumulator = vec4(0.0, 1.0, 0.0, 1.0);  // I am error
    }
    for (uint i=0u; i<gammaSamples; ++i) {
        vec3 sampleCoord = texCoords;
        sampleCoord.z += gammaVariance * rand();
        accumulator += sample(
            makeFan(sampleCoord).dir
        );
    }
    return accumulator / float(gammaSamples);
}
