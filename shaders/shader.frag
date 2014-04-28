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

uniform float time;
uniform float tester = 5.0;
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

vec4 pos2cam; // world space
vec3 normal;

vec3 tangent;
vec3 biTangent;

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

// // http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
// mat4 rotationMatrix(vec3 axis, float angle) {;
//     axis = normalize(axis);
//     float s = sin(angle);
//     float c = cos(angle);
//     float oc = 1.0 - c;
//     return mat4(
//         oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
//         oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
//         oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
//         0.0, 0.0, 0.0, 1.0
//     );
// }

vec4 sampleHdrLightmap(vec3 v) {
    // TODO: special case when abs(v.z)+abs(v.x) == 0
    // TODO: convert to cubemap?
    v = normalize(v);
    return 50.0 * texture(lightmap_hdr, vec2(
        ( 0.5 / pi) * atan(v.z, v.x), // horizontal angle
        (-1.0 / pi) * asin(v.y) - 0.5       // vertical angle
    ));
}

// if v is not in the hemisphere in the direction of hemiDir
// it will be mirrored along a plane defined by the normal hemiDir.
// we assume v and hemiDir are normalized
vec3 hemisphere(vec3 v, vec3 hemiDir) {
    float d = dot(v, hemiDir);
    if (d < 0) {
        return v - 2.0 * d*hemiDir;
    }
    return v;
}

// https://developer.blender.org/file/data/aqyezezn632wm4nxw5mc/PHID-FILE-ue4z6fkynybg3bmmcoeu/Ashikhmin_final.txt
/* Ashikhmin anisotropic specular*/
float Ashikhmin_Spec(vec3 n, vec3 l, vec3 v, float roughu, float roughv, float spec_shader) {
    // float i, nh, nl, nv, hv, hu, hw, hl,
    float fresnel_frac, div, exp=0.0;

    /* half-way vector */
    vec3 h = normalize(l + v);

    vec3 w, u; /* tangent & biTangent vectors */
    // /* TODO: select main anisotropic axis */
    // if (n.x == 0.0 && n.z==0.0) {
    //     u = vec3(0.0, (n.y<0.0) ? -1.0 : 1.0, 0.0);
    //     w = vec3(0.0, 0.0, 1.0);
    // }
    // else {
    //     float d = 1.0 / sqrt(n.z*n.z + n.x*n.x);
    //     u = vec3(n.z * d, 0.0, -n.x * d);
    //     w = normalize(cross(u, n));
    // }
    w = tangent;
    u = biTangent;

    float nh = dot(n, h); /* Dot product between surface normal and half-way vector */
    float nl = dot(n, l); /* Dot product between surface normal and light vector */
    float nv = dot(n, v); /* Dot product between surface normal and view vector */
    float hl = dot(h, l); /* Dot product between light vector and half-way vector */
    float hv = dot(h, v); /* Dot product between view vector and half-way vector */
    float hu = dot(h, u); /* Dot product between rough u direction and half-way vector */
    float hw = dot(h, w); /* Dot product between rough v direction and half-way vector */

    if (nh <= 0.0) return 0.0;
    if (nl <= 0.0) return 0.0;
    if (nv <= 0.0) nv = 0.0;
    if (hl <= 0.0) hl = 0.0;
    if (hv <= 0.0) hv = 0.0;

    return nl;

    /* Schlick's approximation to Fresnel fraction */
    if (spec_shader > 1.0) spec_shader = 1.0;
    fresnel_frac = spec_shader + ((1.0 - spec_shader) * pow((1.0 - max(hv,hl)), 5));
    // return fresnel_frac;

    /* Anisotropic Phong model */
    div = 1.0 - pow(nh, 2);
    if (div > 0.0) {
        exp = ((roughu * hu * hu) + (roughv * hw *hw)) / div;
    }

    return (
        nl *
        (sqrt((roughu + 1.0)*(roughv + 1.0)) / (8.0 * pi * (max(hv,hl) * max(nv,nl)))) *
        pow(nh,exp) //* fresnel_frac
    );
}

vec4 light(vec4 pos2light) {
    pos2light = normalize(pos2light);
    vec3 cam2pos = -pos2cam.xyz;


    // vec4 n_dash = vec4(0.0);
    // n_dash += normal  * dot(normal,  pos2light);
    // n_dash += tangent * dot(tangent, pos2light);
    // n_dash.w = 0.0;
    // n_dash = normalize(n_dash);

    // normal = n_dash;
    // return tangent;

    vec4 accumulator = vec4(0.0);

    // for (int i=0; i<numSamples; ++i) {
    //     // vec3 sampleDir = sampleDirections[i];
    //     vec3 sampleDir = rand3D();
    //     sampleDir = hemisphere(sampleDir, normal);
    //     accumulator += texture(lightmap, sampleDir) * Ashikhmin_Spec(normal, sampleDir, cam2pos, 1.0, 0.0, 0.1);
    // }
    // return 10.0 * accumulator / numSamples;


    // for (int i=0; i<numSamples; ++i) {
    //     vec3 reflectionNormal = normal + dot(0.9*rand3D(), biTangent) * biTangent;
    //     accumulator += texture(lightmap, normalize(reflect(
    //         cam2pos,
    //         reflectionNormal
    //     )));
    // }
    // return accumulator / numSamples;


    // for (int i=0; i<numSamples; ++i) {
    //     vec3 reflectionNormal = normal + dot(0.9*sampleDirections[i], biTangent.xyz) * biTangent;
    //     accumulator += texture(lightmap, normalize(reflect(cam2pos, reflectionNormal)));
    // }
    // return accumulator / numSamples;

    return sampleHdrLightmap(reflect(cam2pos, normal));
    return texture(lightmap, reflect(cam2pos, normal));


    // phong
    float Idiff = dot(normal, -pos2light.xyz);  // Diffuse

    // Specular
    vec3 R = normalize(reflect(cam2pos, normal)); // reflected. (pos to eye-ish)
    float base = clamp(dot(R, pos2light.xyz), 0.0, 1.0);
    float Ispec = 1.0 * pow(base, 40.0);
    return vec4((
            light_ambient +
            light_diffuse  * clamp(Idiff, 0.0, 1.0) +
            light_specular * clamp(Ispec, 0.0, 1.0)
        ),
        0.0
    );
}

void main() {
    // pos2cam = vec4(normalize(-pos_view.xyz), 0.0);
    pos2cam = eye - pos_world;

    // normal = vec4(normalize(-pos_world.xyz), 0.0);
    vec4 modelNormal;
    if (abs(pos_model.y) > 0.999) {
        modelNormal = vec4(0.0, pos_model.y, 0.0, 0.0);
    }
    else {
        modelNormal = vec4(pos_model.x, 0.0, pos_model.z, 0.0);
    }
    normal  = normalize(modelMatrix * modelNormal).xyz;
    tangent = normalize(modelMatrix * vec4(pos_model.z, 0.0, -pos_model.x, 0.0)).xyz;
    biTangent = normalize(vec3(cross(normal, tangent)));

    fragColor = light(lights[0]);
    // fragColor = texture(lightmap_hdr, pos_screen.xy / pos_screen.z ) * 100.0;

    fragColor = 1.0 - exp(-exposure * fragColor);

    // fragColor = eye;

    if (numSamples > availableSamples) {
        error = true;
    }
    if (error) {
        fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }

}
