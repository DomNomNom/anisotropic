// represents a collection of directions shaped like a
struct Fan {
    // center of the fan's direction
    vec3 dir;

    // the orientation of the fan defined by the normal to the plane
    // this is orientation of the Gamma axis (hence g)
    // constraint when making texture coordinates: dot(dir, g) == 0.0
    vec3 g;

    // fan width is ommitted as it is presumed to be constant
};


// note: tau is a constant defined to be 2.0*pi

// takes   (0..tau      0..pi/2  0..tau )
// returns (0..1        0..1     0..1  )
vec3 anglesToTexCoords(float alpha, float beta, float gamma) {
    vec3 texCoords = vec3(alpha, beta*4.0, gamma);
    return texCoords / tau;  //  0..tau  --> 0..1
}

// inverse of anglesToTexCoords()
// takes   (0..1        0..1     0..1  )
// returns (0..tau  -pi/2..pi/2  0..pi )
vec3 texCoordsToAngles(vec3 texCoords) {
    vec3 ABG = texCoords * tau;
    ABG.y *= 0.25;
    return ABG;
}


// like the usual atan2 but returns results in the range 0..tau
// optimization: allow angles to be in range -pi..pi and just use the GLSL atan(y,x)
float myAtan2(float y, float x) {
    float ret = atan(y, x);
    if (ret < 0.0) {
        ret += tau;
    }
    return ret;
}

// returns (alpha beta gamma) where the ranges are 0..1 therefore texture coordinates.
// the angle ranges of
// names:                (alpha      beta      gamma )
// returned values:      (0..1        0..1     0..1  )
// corresponding angles: (0..tau      0..pi/2  0..tau)
vec3 makeTexCoords(Fan fan) {

    // sanitize our input
    fan.dir = normalize(fan.dir);
    fan.g   = normalize(fan.g);

    // force fan.g.y >= 0.0
    // this constraint will force 0 <= beta <= pi/2
    // when g is reversed, the fan remains the same
    if (fan.g.y < 0.0) {
        fan.g *= -1.0;
    }


    // vectors for alpha and gamma space
    // there is a special case when abs(dir.y) == 1

    // the hoizontal direction on the top of the gamma plane
    vec3 horizontal   = normalize(-vec3(fan.g.x, 0.0, fan.g.z));
    // the right side when looking down on the gamma plane
    vec3 biHorizontal = normalize(cross(horizontal, vec3(0.0, 1.0, 0.0)));
    // a vector tangent to the fan at the horizontal plane
    vec3 elevation    = normalize(cross(fan.g, biHorizontal));

    // alpha
    // clockwise rotation around Y
    float alpha = myAtan2(-biHorizontal.z, biHorizontal.x);

    // beta
    float beta = acos(fan.g.y);

    // gamma
    // find the angle of fan.dir projected onto the fan-circle plane
    float gamma = myAtan2(
        dot(fan.dir, elevation),    // gamma-space y
        dot(fan.dir, biHorizontal)  // gamma-space x
    );
    if (isZero(gamma-tau)) {  // enforce consistency due to floating point errors
        gamma = 0.0;
    }

    vec3 texCoords = anglesToTexCoords(alpha, beta, gamma);

    // some assertion statements
    if (!(
        isZero(dot(fan.dir, fan.g))                 &&

        // alpha space
        isZero(horizontal.y)                        &&
        isZero(biHorizontal.y)                      &&
        isZero(dot(horizontal, biHorizontal))       &&

        // gamma space
        isZero(dot(biHorizontal, fan.g))            &&
        isZero(dot(biHorizontal, elevation))        &&
        isZero(dot(fan.g,        elevation))        &&

        // horizontal and fan.dir are on a plane with normal vector g
        // the outer cross() asserts that the vectors are scalar multiples of each other
        isZero(length(cross(cross(biHorizontal, fan.dir), fan.g))) &&

        0.0 <= texCoords.x && texCoords.x <= 1.0    &&
        0.0 <= texCoords.y && texCoords.y <= 1.0    &&
        0.0 <= texCoords.z && texCoords.z <= 1.0    &&

        true  // this is here to allow for easier code bisection when debugging
    )) {
        error = true;
    }

    return texCoords;
}




// returns a 3x3 rotation matrix for the given angles
// note: each of the axis refers to an axis in the transformed space
mat3 makeTransform(float alpha, float beta, float gamma) {
    mat3 alphaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), alpha);
    mat3 betaTransform  = rotationMatrix3(vec3(1.0, 0.0, 0.0), beta );
    mat3 gammaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), gamma);
    return alphaTransform * betaTransform * gammaTransform;
}

// returns a Fan for the given texCoords
// inverse of makeTexCoords()
Fan makeFan(vec3 texCoords) {
    vec3 ABG = texCoordsToAngles(texCoords);
    mat3 transform = makeTransform(ABG.x, ABG.y, ABG.z);
    return Fan(
        transform * vec3(1.0, 0.0, 0.0),
        transform * vec3(0.0, 1.0, 0.0)
    );
}



