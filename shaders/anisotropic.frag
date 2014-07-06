
// requires

#include constants.glsl
#include sampleLightmap.frag

const int availableSamples = 162;
uniform int numSamples;
uniform vec3[availableSamples] sampleDirections;
uniform sampler3D cache;


vec2 randomV = pos_project.xy * sin(time);
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


// if v is not in the hemisphere in the direction of hemiDir
// it will be mirrored along a plane defined by the normal hemiDir.
// we assume v and hemiDir are of length 1
vec3 hemisphere(vec3 v, vec3 hemiDir) {
    float d = dot(v, hemiDir);
    if (d < 0) {
        return v - 2.0 * d*hemiDir;
    }
    return v;
}

// // https://developer.blender.org/file/data/aqyezezn632wm4nxw5mc/PHID-FILE-ue4z6fkynybg3bmmcoeu/Ashikhmin_final.txt
// /* Ashikhmin anisotropic specular*/
// float Ashikhmin_Spec(vec3 n, vec3 l, vec3 v, float roughu, float roughv, float spec_shader) {
//     float fresnel_frac, div, exp=0.0;

//     /* half-way vector */
//     vec3 h = normalize(l + v);

//     vec3 w, u; /* tangent & biTangent vectors */
//     w = tangent;
//     u = biTangent;

//     float nh = dot(n, h); /* Dot product between surface normal and half-way vector */
//     float nl = dot(n, l); /* Dot product between surface normal and light vector */
//     float nv = dot(n, v); /* Dot product between surface normal and view vector */
//     float hl = dot(h, l); /* Dot product between light vector and half-way vector */
//     float hv = dot(h, v); /* Dot product between view vector and half-way vector */
//     float hu = dot(h, u);  Dot product between rough u direction and half-way vector
//     float hw = dot(h, w); /* Dot product between rough v direction and half-way vector */

//     if (nh <= 0.0) return 0.0;
//     if (nl <= 0.0) return 0.0;
//     if (nv <= 0.0) nv = 0.0;
//     if (hl <= 0.0) hl = 0.0;
//     if (hv <= 0.0) hv = 0.0;
//     // return nl;


//     /* Schlick's approximation to Fresnel fraction */
//     if (spec_shader > 1.0) spec_shader = 1.0;
//     fresnel_frac = spec_shader + ((1.0 - spec_shader) * pow((1.0 - max(hv,hl)), 5));
//     // return fresnel_frac * 100000.0;

//     /* Anisotropic Phong model */
//     div = 1.0 - pow(nh, 2);
//     if (div > 0.0) {
//         exp = ((roughu * hu*hu) + (roughv * hw*hw)) / div;
//     }

//     return (
//         // nl *
//         (sqrt((roughu + 1.0)*(roughv + 1.0)) / (8.0 * pi * (max(hv,hl) * max(nv,nl))))
//         // pow(nh,exp) //* fresnel_frac
//     );
// }


float sq(float x) {  return x * x;  }  // squares a number


// http://en.wikipedia.org/wiki/Specular_highlight#Ward_anisotropic_distribution
float ward_spec(vec3 n, vec3 l, vec3 r, vec3 x, vec3 y, float ax, float ay) {
    vec3 h = normalize(l+r);

    float nl = dot(n, l);
    float nr = dot(n, r);
    if (nl < 0.0) return 0.0;
    if (nr < 0.0) return 0.0;

    return (
        (1.0 / sqrt(nl * nr)) *
        nl / (4.0 * pi * ax * ay) *
        exp(-2.0 * (
            (
                sq(dot(h, x) / ax) +
                sq(dot(h, y) / ay)
            )
            / (1.0 + dot(h, n))
        ))
    );
}

/*
// r = reflectedDir
// g = gammaNormal
vec4 cacheSample(vec3 r, vec3 g) {


    // look up something in the texture
    vec3 horizontal = normalize(cross(tangent, vec3(0.0, 1.0, 0.0)));
    // if (horizontal.z < 0.0) horizontal *= -1.0; // force horizontal.z >= 0
    float alpha = acos(horizontal.x); // dot(horizontal, vec3(1.0, 0.0, 0.0))  // TODO optimize with definition of horizontal
    if (horizontal.z > 0) alpha = 2.0*pi-alpha; // case when alpha < 0 (or >180)

    // vec3 gammaPlaneNormal = cross(normalReflectedDir, horizontal);  // up when normalReflectedDir==(1 0 0) and tangent=(0 0 -1)
    // vec3 elevationVector = normalize(cross(tangent, horizontal));
    // float beta = asin(elevationVector.y);
    float beta = asin(tangent.y);
    // if (elevationVector.y < 0) return errorColor;beta *= -1.0; // ensure the sign is correct

    // float gamma = -acos(dot(normalReflectedDir, horizontal));
    // if (normalReflectedDir.y < 0) gamma *= -1.0; // ensure the sign is correct
    // float gamma = asin(-0.99999 * length(cross(horizontal, normalReflectedDir))); // ? == asin(-length(cross(horizontal, normalReflectedDir)))
    // float gamma = asin(-1.00001 * length(cross(horizontal, normalReflectedDir)));
    // float gamma = asin(clamp(-1.0 * length(cross(horizontal, r)), -1.0, 1.0));
    float gamma = acos(dot(r, horizontal));
    // if (r.y < 0.0) gamma = 2.0*pi-gamma;
    if (r.y < 0.0) gamma = -gamma;

    // // cartesian coords
    // beta = radians(90.0);
    // gamma = asin(normalReflectedDir.y);
    // alpha = atan(-normalReflectedDir.z, normalReflectedDir.x);

    vec3 texCoords = vec3(alpha, beta, 2.0 * gamma);
    // texCoords = texCoords.xzy;
    texCoords /= 2.0 * pi;  //  (0 2pi)  --> (0 1)
    if (normalReflectedDir.x > 0.99 && normalReflectedDir.x < 0.999) return errorColor; // the debug ring of truth

    // return vec4(texCoords, 1.0);
    if (texCoords.x < -1.0 || texCoords.x > 1.0) return errorColor;
    if (texCoords.y < -1.0 || texCoords.y > 1.0) return errorColor;
    if (texCoords.z < -1.0 || texCoords.z > 1.0) return errorColor;
    // return vec4(to01(elevationVector), 1.0);
    return vec4(
        0.0,
        to01(texCoords.z), // to01(gamma / pi),
        0.0,
        1.0
    );
    // return vec4(to01(normalize(normalReflectedDir)), 1.0);
    // return sample(pos_world.xyz);
    // return vec4(to01(texCoords), 1.0);
    return texture(cache, texCoords);
}
*/

vec4 anisotropic(vec3 normal, vec3 cam2pos, vec3 tangent, float anisotropy) {
    vec3 biTangent = normalize(cross(normal, tangent));
    vec3 normalReflectedDir = normalize(reflect(cam2pos, normal));

    // return vec4 cacheSample(pos_world.xyz, tangent);
    // return vec4 cacheSample(normalReflectedDir, tangent);


    // pos2light = normalize(pos2light);

    // vec4 n_dash = vec4(0.0);
    // n_dash += normal  * dot(normal,  pos2light);
    // n_dash += tangent * dot(tangent, pos2light);
    // n_dash.w = 0.0;
    // n_dash = normalize(n_dash);

    // normal = n_dash;
    // return tangent;






    // ====== stochastic sampling from here on ======

    vec4 accumulator = vec4(0.0);  // accumulates reflected light

    // // sample the lightmap and multiply by the BDRF
    // for (int i=0; i<numSamples; ++i) {
    //     // vec3 sampleDir = sampleDirections[i];
    //     vec3 sampleDir = rand3D();
    //     sampleDir = hemisphere(sampleDir, normal);
    //     // if (dot(normal, sampleDir) < 0)
    //     //     error = true;
    //     // accumulator += sample(sampleDir) * dot(normal, sampleDir);
    //     // accumulator += sample(sampleDir) * Ashikhmin_Spec(normal, sampleDir, normalize(-cam2pos), 100.0, 100.0, 0.5);
    //     accumulator += sample(sampleDir) * ward_spec(normal, sampleDir, normalize(-cam2pos), tangent, biTangent, anisotropy, anisotropy * 0.1);
    // }
    // return 10.0 * accumulator / numSamples;

    // biased sampling by varying-the-normal approach.
    for (int i=0; i<numSamples; ++i) {
        vec3 reflectedDir;

        if (tester_int == 0) {
            // varied normal approach
            vec3 reflectionNormal = normal + anisotropy * rand() * biTangent;
            reflectionNormal = hemisphere(normalize(reflectionNormal), normal);
            reflectedDir = normalize(reflect(cam2pos, reflectionNormal));
        }
        else if (tester_int == 1) {
            vec3 reflectionNormal = normal + anisotropy * rand() * biTangent;
            vec3 reflectedDir1 = normalize(reflect(cam2pos, normalize(normal + 0.001*biTangent)));
            vec3 reflectedDir2 = normalize(reflect(cam2pos, normalize(normal - 0.001*biTangent)));
            vec3 g = cross(normalReflectedDir, normalize(reflectedDir2 - reflectedDir1)); // gammaNormal
            reflectedDir = rotationMatrix3(g, rand() * (pi/2) * anisotropy) * normalReflectedDir;
        }
        else {
            // sampled arc approach
            reflectedDir = rotationMatrix3(tangent, rand() * (pi/2) * anisotropy) * normalReflectedDir;
        }

        // two ways to deal reflected vectors that go into the surface (reflect along normal or make the sample be zero)
        if (dot(reflectedDir, normal) >= 0) {
            accumulator += sample(reflectedDir);
        }
        // else {
        //     // error = true;
        //     accumulator += vec4(1.0, 0.0, 0.0, 0.0);
        // }

        // reflectedDir = hemisphere(reflectedDir, normal);
        // accumulator += sample(reflectedDir);
    }
    return accumulator / numSamples;


    // for (int i=0; i<numSamples; ++i) {
    //     vec3 reflectionNormal = normal + dot(0.9*sampleDirections[i], biTangent.xyz) * biTangent;
    //     accumulator += texture(lightmap, normalize(reflect(cam2pos, reflectionNormal)));
    // }
    // return accumulator / numSamples;

    return sample(reflect(cam2pos, normal));


}
