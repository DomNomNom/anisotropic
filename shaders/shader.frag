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

uniform float time;
uniform float tester;
uniform vec2 mouse;
uniform vec4 eye;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

const int availableSamples = 162;
uniform int numSamples;
uniform vec3[availableSamples] sampleDirections;

// common constants
const float pi = 3.141592653589793;
const float sqrt_2 = sqrt(2.0);
const float sqrt_pi = sqrt(pi);

const vec3 light_ambient  = vec3(1.0, 1.0, 1.0) * 0.03;
const vec3 light_diffuse  = vec3(1.0, 0.0, 0.0) * 1.0;
const vec3 light_specular = vec3(1.0, 1.0, 1.0) * 1.0;

const float exposure = 1.0;

bool error = false; // if true the shader will return the error colour (blue) at the end

vec4 pos2cam; // world space?
vec4 normal;


// takes a vector with values (-1, 1) and scales it to fit in the range (0, 1)
vec4  to01(vec4  neg11) {   return (neg11 + vec4(1.0)) * 0.5;   }
vec3  to01(vec3  neg11) {   return (neg11 + vec3(1.0)) * 0.5;   }
float to01(float neg11) {   return (neg11 +      1.0 ) * 0.5;   }

vec2 randomV = pos_screen.xy * sin(time);
float rand() { // returns a random value within the range -1.0 to 1.0
    float random = fract(sin(dot(randomV.xy, vec2(12.9898, 78.233)))* 43758.5453)  *2.0 - 1.0;
    randomV = vec2(random, randomV.y*0.6364+randomV.x*0.2412+1.3);
    return random;
}

vec4 rand3D_vec4() {
    return vec4(rand(), rand(), rand(), 0.0);
}
vec3 rand3D() {
    return vec3(rand(), rand(), rand());
}

void BRDF(vec4 pos2light) {

}

vec4 phong(vec4 pos2light) {
    pos2light = normalize(pos2light);

    vec4   tangent = normalize(modelMatrix * vec4(pos_model.z, 0.0, -pos_model.x, 0.0));
    vec4 coTangent = normalize(vec4(cross(normal.xyz, tangent.xyz), 0.0));

    // vec4 n_dash = vec4(0.0);
    // n_dash += normal  * dot(normal,  pos2light);
    // n_dash += tangent * dot(tangent, pos2light);
    // n_dash.w = 0.0;
    // n_dash = normalize(n_dash);

    // normal = n_dash;
    // return tangent;

    float Idiff = dot(normal, -pos2light);  // Diffuse

    // Specular
    vec3 R = normalize(reflect(-pos2cam, normal)).xyz; // reflected. (pos to eye-ish)
    float base = clamp(dot(R, pos2light.xyz), 0.0, 1.0);
    float Ispec = 1.0 * pow(base, 40.0);

    vec4 accumulator = vec4(0.0);
    for (int i=0; i<numSamples; ++i) {
        vec4 reflectionNormal = normal + dot(0.9*rand3D_vec4(), coTangent) * coTangent;
        accumulator += texture(lightmap, normalize(reflect(
            -pos2cam,
            reflectionNormal
        )).xyz);
    }
    return accumulator / numSamples;


    for (int i=0; i<numSamples; ++i) {
        vec4 reflectionNormal = normal + dot(0.9*sampleDirections[i], coTangent.xyz) * coTangent;
        accumulator += texture(lightmap, normalize(reflect(
            -pos2cam,
            reflectionNormal
        )).xyz);
    }
    return accumulator / numSamples;

    return texture(lightmap, reflect(-pos2cam, normal).xyz);
    return vec4((
            light_ambient +
            light_diffuse  * clamp(Idiff, 0.0, 1.0) +
            light_specular * clamp(Ispec, 0.0, 1.0)
        ),
        0.0
    );
}

void main() {
    pos2cam = vec4(normalize(-pos_view.xyz), 0.0);

    // normal = vec4(normalize(-pos_world.xyz), 0.0);
    if (abs(pos_model.y) > 0.999) {
        normal = vec4(0.0, pos_model.y, 0.0, 0.0);
    }
    else {
        normal = vec4(pos_model.x, 0.0, pos_model.z, 0.0);
    }
    normal = normalize(modelMatrix * normal);

    fragColor = phong(lights[0]);


    fragColor = 1.0 - exp(-exposure * fragColor);

    if (numSamples > availableSamples) {
        error = true;
    }
    if (error) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }

}
