
vec3 debugVector;

// requires

// #include random.glsl

#include constants.glsl
// #include angles.glsl
#include sampleLightmap.frag

const int availableSamples = 162;
uniform vec3[availableSamples] sampleDirections;
uniform sampler3D cache;




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

// r: reflectedDir
// g: gammaNormal
// dot(r, g) == 0.0
vec4 cacheSample(Fan fan) {
    // return vec4(0.0, to01(r.y), 0.0, 1.0);
    vec3 texCoords = makeTexCoords(fan);
    // Fan checkFan = makeFan(texCoords);

    // // return vec4(0.0, 0.0, texCoords.z, 1.0);
    // // return vec4(0.0, texCoords.y,  0.0, 1.0);
    // // return vec4(texCoords.x, 0.0 , 0.0, 1.0);


    // checkFan.dir    = normalize(checkFan.dir);
    // checkFan.g      = normalize(checkFan.g);
    // fan.dir         = normalize(fan.dir);
    // fan.g           = normalize(fan.g);
    // if (!isClose(fan.dir,    checkFan.dir     )) { return vec4(to01(fan.dir - checkFan.dir), 1.0); }
    // if (!(
    //     isClose(fan.g,  checkFan.g) ||
    //     isClose(fan.g, -checkFan.g)
    // )) {
    //     return vec4(to01(fan.g   - checkFan.g  ), 1.0);
    // }

    // texCoords *= noInterpBorderScale;
    // texCoords += vec3(0.5) * ;

    return texture(cache, texCoords);
}


vec4 anisotropic(vec3 normal, vec3 cam2pos, vec3 tangent, float anisotropy) {
    vec3 biTangent = normalize(cross(normal, tangent));
    vec3 r = normalize(reflect(cam2pos, normal));
    vec3 g = normalize(cross(r, biTangent));  // gammaNormal
    Fan fan = Fan(r, g);

    // // the debug ring of truth
    // float r = length(pos_model.zy);
    // if (0.05 < r && r < 0.1 && pos_model.x > 0) return errorColor;

    // return cacheSample(pos_model.xyz, normalize(cross(pos_model.xyz, biTangent)));
    if (tester_int == 0) {
        return cacheSample(fan);
    }

    // TODO: revert
    if (tester_int == 1) {
        vec3 texCoords = makeTexCoords(fan);
        return accumulateSamples(texCoords);
    }
    if (tester_int == 2) {
        vec3 texCoords = makeTexCoords(fan);
        texCoords = makeTexCoords(makeFan(texCoords));
        return accumulateSamples(texCoords);
    }
    if (tester_int == 3) {
        return sample(r);
    }


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

        if (tester_int == 4) {
            reflectedDir = rotationMatrix3(g, rand() * (tau) * anisotropy) * r;
        }
        else if (tester_int == 5) {
            // sampled arc approach
            reflectedDir = rotationMatrix3(tangent, rand() * (tau) * anisotropy) * r;
        }
        else if (tester_int == 6) {
            // varied normal approach
            vec3 reflectionNormal = normal + anisotropy * rand() * biTangent;
            reflectionNormal = hemisphere(normalize(reflectionNormal), normal);
            reflectedDir = normalize(reflect(cam2pos, reflectionNormal));
        }
        else {
            error = true;
            reflectedDir = vec3(0.0);
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
