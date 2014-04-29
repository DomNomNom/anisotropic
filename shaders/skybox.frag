#version 330

out vec4 fragColor;


smooth in vec4 pos_view;
smooth in vec4 pos_world;
smooth in vec4 pos_model;
smooth in vec4 pos_screen;

smooth in vec4 vertex_normal;
smooth in vec4 lights[4];
// varying vec4 tangent;

uniform samplerCube lightmap;
uniform sampler2D lightmap_hdr;

uniform float exposure = 1.0;
uniform float time;
uniform float tester;
uniform vec2 mouse;
uniform vec4 eye;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

// common constants
const float pi = 3.141592653589793;
const float sqrt_2 = sqrt(2.0);
const float sqrt_pi = sqrt(pi);



vec4 pos2cam; // world space


// takes a vector with values (-1, 1) and scales it to fit in the range (0, 1)
vec4  to01(vec4  neg11) {   return (neg11 + vec4(1.0)) * 0.5;   }
vec3  to01(vec3  neg11) {   return (neg11 + vec3(1.0)) * 0.5;   }
float to01(float neg11) {   return (neg11 +      1.0 ) * 0.5;   }


vec4 sampleHdrLightmap(vec3 v) {
    // TODO: special case when abs(v.z)+abs(v.x) == 0
    v = normalize(v);
    return 50.0 * texture(lightmap_hdr, vec2(
        ( 0.5 / pi) * atan(v.z, v.x), // horizontal angle
        (-1.0 / pi) * asin(v.y) - 0.5       // vertical angle
    ));
}

// takes a lightmap sample in the given direction
// this function exists to make it easy to switch between lightmaps
vec4 sample(vec3 dir) {
    return texture(lightmap, dir);
    return sampleHdrLightmap(dir);
}

void main() {
    pos2cam = eye - pos_world;


    // fragColor = texture(lightmap, -pos2cam.xyz);
    fragColor = sample(-pos2cam.xyz);
    fragColor = 1.0 - exp(-exposure * fragColor);

    // fragColor = vec4(0.0, 1.0, 0.0, 1.0);

}
