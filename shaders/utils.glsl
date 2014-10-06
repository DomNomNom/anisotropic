
bool isZero(float f) {
    return abs(f) < 0.0001;
}


// takes a vector with values (-1, 1) and scales it to fit in the range (0, 1)
vec4  to01(vec4  neg11) {   return (neg11 + vec4(1.0)) * 0.5;   }
vec3  to01(vec3  neg11) {   return (neg11 + vec3(1.0)) * 0.5;   }
float to01(float neg11) {   return (neg11 +      1.0 ) * 0.5;   }
