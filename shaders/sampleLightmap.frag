
uniform samplerCube lightmap;
uniform sampler2D lightmap_hdr;

vec4 sampleHdrLightmap(vec3 v) {
    // TODO: special case when abs(v.z)+abs(v.x) == 0
    v = normalize(v);
    return 50.0 * texture(lightmap_hdr, vec2(
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
