
// represents a fan of directions emmitted from (0 0 0)
struct Fan {
    // fan width is ommitted
    // a soft constraint: dir.y * g.y <= 0.0

    // center of the fan's direction
    vec3 dir;

    // the orientation of the fan defined by the normal to the plane
    // this is orientation of the Gamma axis (hence g)
    vec3 g;
};



// takes   (0..tau  -pi/2..pi/2  0..pi )
// returns (0..1        0..1     0..1  )
vec3 getTexCoords(float alpha, float beta, float gamma) {
    vec3 texCoords = vec3(alpha, (beta + pi/2.0)*2.0, gamma*2.0);
    return texCoords / tau;  //  0..tau  --> 0..1
}

// inverse of getTexCoords()
// takes   (0..1        0..1     0..1  )
// returns (0..tau  -pi/2..pi/2  0..pi )
vec3 getAngles(vec3 texCoords) {
    vec3 ABG = texCoords * tau;
    ABG.y *= 0.5;
    ABG.y -= pi/2.0;
    ABG.z *= 0.5;
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
// corresponding angles: (0..tau  -pi/2..pi/2  0..pi)
vec3 makeTexCoords(Fan fan) {

    // TODO: special cases for abs(dir.y) == 1

    // sanitize our input
    fan.dir = normalize(fan.dir);
    fan.g   = normalize(fan.g);
    if (fan.dir.y * fan.g.y > 0.0) {
        fan.g *= -1.0;  // we can rotate the fan about fan.dir 180 degrees and nothing should change
    }

    // vectors for alpha and gamma space
    vec3 horizontal   = normalize(vec3( fan.g.x, 0.0,  fan.g.z)); // the hoizontal direction of fan.g
    vec3 biHorizontal = -normalize(vec3(-fan.g.z, 0.0,  fan.g.x)); // cross(fan.g, (0 1 0)). The line at which the expaned fan meets the x/z plane
    vec3 elevationVector = cross(fan.g, biHorizontal);            // a vector tangent to the fan at the horizontal plane



    // alpha
    float alpha = myAtan2(biHorizontal.z, biHorizontal.x);


    // beta
    float beta = atan(
        fan.dir.y,
        dot(fan.dir, horizontal)
    );
    // assert -pi/2 <= beta <= pi/2
    // since fan.dir.y * fan.g.y <= 0.0
    if (abs(beta) > pi/2.0 + 0.01) {
        error = true;
    }
    beta = clamp(beta, -pi/2.0, pi/2.0); // numerical stability


    // gamma
    float gamma = myAtan2(  // find the angle of fan.dir projected onto the fan-circle plane
        dot(fan.dir, elevationVector),  // gamma-space y
        dot(fan.dir, biHorizontal)      // gamma-space x
    );
    // force gamma in range 0..pi by applying a fan identity
    if (gamma > pi) {
        // alpha += pi;
        // if (alpha > tau) { alpha -= tau; }
        // beta *= -1.0;
        // gamma -= pi;
        // gamma -= pi/2;
        // gamma *= 40;
    }

    vec3 texCoords = getTexCoords(alpha, beta, gamma);
    if (gamma > pi) {
        // WTF WTFWTF WTFWTFWTFWRF
        texCoords.z = -texCoords.z +2;
    }

    // some assertion statements
    if (!(
        fan.dir.y * fan.g.y <= 0.0                  &&
        isZero(dot(fan.dir, fan.g))                 &&

        // alpha space
        isZero(horizontal.y)                        &&
        isZero(biHorizontal.y)                      &&
        isZero(dot(horizontal, biHorizontal))       &&

        // // gamma space
        isZero(dot(biHorizontal, fan.g))            &&
        isZero(dot(biHorizontal, elevationVector))  &&
        isZero(dot(fan.g,        elevationVector))  &&

        // // horizontal and fan.dir are on a plane with normal vector g
        // // the outer cross() asserts that the vectors are scalar multiples of each other
        isZero(length(cross(cross(biHorizontal, fan.dir), fan.g))) &&

        0.0 <= texCoords.x && texCoords.x <= 1.0    &&
        0.0 <= texCoords.y && texCoords.y <= 1.0    &&
        0.0 <= texCoords.z && texCoords.z <= 1.0    &&

        true
    )) {
        error = true;
    }

    return texCoords;
}








// these two give the same results
mat3 makeTransform1(float alpha, float beta, float gamma) {
    mat3 alphaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), alpha);
    mat3 betaTransform  = rotationMatrix3(vec3(1.0, 0.0, 0.0), beta );
    mat3 gammaTransform = rotationMatrix3(vec3(0.0, 1.0, 0.0), gamma);
    return alphaTransform * betaTransform * gammaTransform;
}

mat3 makeTransform2(float alpha, float beta, float gamma) {
    mat3 alphaTransform = rotationMatrix3(                                 vec3(0.0, 1.0, 0.0), alpha);
    mat3 betaTransform  = rotationMatrix3(                alphaTransform * vec3(1.0, 0.0, 0.0), beta );
    mat3 gammaTransform = rotationMatrix3(betaTransform * alphaTransform * vec3(0.0, 1.0, 0.0), gamma);
    return gammaTransform * betaTransform * alphaTransform;  // NOTE: reversed order
}

// returns a Fan for the given texCoords
// inverse of makeTexCoords()
Fan makeFan(vec3 texCoords) {
    vec3 ABG = getAngles(texCoords);
    mat3 transform = makeTransform1(ABG.x, ABG.y, ABG.z);
    return Fan(
        transform * vec3(1.0, 0.0, 0.0),
        transform * vec3(0.0, 1.0, 0.0)
    );
}



