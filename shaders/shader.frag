#version 330

out vec4 fragColor;


smooth in vec4 pos_view;
smooth in vec4 pos_world;
smooth in vec4 pos_model;
smooth in vec4 pos_project;

smooth in vec4 vertex_normal;
smooth in vec4 lights[4];
// varying vec4 tangent;

uniform float exposure = 1.0;
uniform float time;
uniform float tester = 5.0;
uniform float tester2;
uniform int tester_int;
uniform vec2 mouse;
uniform vec4 eye;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;


bool error = false; // if true the shader will return the error colour (blue) at the end


#include to01.glsl
#include rotationMatrix.glsl
#include anisotropic.frag



void main() {
    // pos2cam = vec4(normalize(-pos_view.xyz), 0.0);
    vec4 pos2cam = eye - pos_world; // world space

    // normal = vec4(normalize(-pos_world.xyz), 0.0);
    vec4 modelNormal;
    if (abs(pos_model.y) > 0.999) {
        modelNormal = vec4(0.0, pos_model.y, 0.0, 0.0);
    }
    else {
        modelNormal = vec4(pos_model.x, 0.0, pos_model.z, 0.0);
    }
    vec3 normal  = normalize(modelMatrix * modelNormal).xyz;
    vec3 tangent = normalize(modelMatrix * vec4(pos_model.z, 0.0, -pos_model.x, 0.0)).xyz;

    fragColor = anisotropic(normal, -pos2cam.xyz, tangent, tester);
    // fragColor = texture(lightmap_hdr, pos_screen.xy / pos_screen.z ) * 100.0;

    fragColor = 1.0 - exp(-exposure * fragColor);

    // fragColor = vec4(normal, 1.0);

    // if (numSamples > availableSamples) {
    //     error = true;
    // }
    if (error) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }

}
