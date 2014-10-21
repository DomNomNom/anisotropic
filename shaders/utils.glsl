
bool isZero(float f) {
    return abs(f) < 0.0009;
}

bool isZero(vec3 v) {
    return (
        isZero(v.x) &&
        isZero(v.y) &&
        isZero(v.z)
    );
}

bool isClose(vec3 a, vec3 b) {
    return isZero(a-b);
}



// takes a vector with values (-1, 1) and scales it to fit in the range (0, 1)
vec4  to01(vec4  neg11) {   return (neg11 + vec4(1.0)) * 0.5;   }
vec3  to01(vec3  neg11) {   return (neg11 + vec3(1.0)) * 0.5;   }
float to01(float neg11) {   return (neg11 +      1.0 ) * 0.5;   }



// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
// angle in radians (default GLSL)
mat4 rotationMatrix4(vec3 axis, float angle) {;
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(
        oc * axis.x * axis.x + c,          oc * axis.x * axis.y + axis.z * s, oc * axis.z * axis.x - axis.y * s, 0.0,
        oc * axis.x * axis.y - axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z + axis.x * s, 0.0,
        oc * axis.z * axis.x + axis.y * s, oc * axis.y * axis.z - axis.x * s, oc * axis.z * axis.z + c,          0.0,
        0.0,                               0.0,                               0.0,                               1.0
    );
}

// 3x3 version of the above
mat3 rotationMatrix3(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat3(
        oc * axis.x * axis.x + c,          oc * axis.x * axis.y + axis.z * s, oc * axis.z * axis.x - axis.y * s,
        oc * axis.x * axis.y - axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z + axis.x * s,
        oc * axis.z * axis.x + axis.y * s, oc * axis.y * axis.z - axis.x * s, oc * axis.z * axis.z + c
    );
}

