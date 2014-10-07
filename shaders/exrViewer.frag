#version 330

out vec4 fragColor;

smooth in vec4 pos_uv;
smooth in vec4 pos_project;

uniform float exposure;
uniform float tester;

uniform bool useCache;
uniform sampler2D exr;
uniform sampler3D cache;
uniform float texcoord;
uniform int cacheViewAxis;

void main() {
    if (useCache) {
        switch (cacheViewAxis) {
            case 1: fragColor = texture(cache, vec3(texcoord, pos_uv.y, pos_uv.x));  break;
            case 2: fragColor = texture(cache, vec3(pos_uv.x, texcoord, pos_uv.y));  break;
            case 3: fragColor = texture(cache, vec3(pos_uv.x, pos_uv.y, texcoord));  break;
            default: fragColor = vec4(1.0, 0.5, 0.0, 1.0);
        }
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
