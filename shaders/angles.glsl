
// represents a arc on a sphere centered on (0 0 0)
struct Arc {
    // arc width is ommitted

    // center of the arc's direction
    vec3 dir;

    // note that this tangent is not the surface tangent,
    // it's the tangent of the arc
    // tangent.x >= 0.
    // this is a convention since tangent can be inverted and it's still the same Arc
    vec3 tangent;
};


vec3 getTexCoords(float alpha, float beta, float gamma) {
    // current ranges: (0..tau  -pi/2..pi/2  0..pi)
    vec3 texCoords = vec3(alpha, (beta+pi/2.0)*2.0, gamma*2.0);
    return texCoords / tau;  //  0..tau  --> 0..1
}

// inverse of getTexCoords
vec3 getAngles(float alpha, float beta, float gamma) {
    vec3 ABG_angles = vec3(alpha, beta, gamma) * tau;
    ABG_angles.y *= 0.5;
    ABG_angles.y -= pi/2.0;
    ABG_angles.z *= 0.5;
    return ABG_angles;
}


// returns (alpha beta gamma) where the ranges are 0..1 therefore texture coordinates.
// the angle ranges of
// names:                (alpha      beta      gamma )
// returned values:      (0..1        0..1     0..1  )
// corresponding angles: (0..tau  -pi/2..pi/2  0..pi)
vec3 makeAlphaBetaGamma(Arc arc) {
    // make sure tangent.x >= 0
    if (arc.tangent.x < 0.0) {
        arc.tangent *= -1.0;
    }

    // gamma normal
    vec3 g = cross(arc.dir, arc.tangent);

    // horizontal is the line at which the arc meets the x/z plane.
    vec3 horizontal = normalize(cross(g, vec3(0.0, 1.0, 0.0)));  // TODO: Optimize

    float alpha = atan(horizontal.z, horizontal.x);
    if (alpha < 0.0) {
        alpha += tau;
    }

      // a vector tangent to the arc at the horizontal plane
    vec3 elevationVector = normalize(cross(g, horizontal));
    float beta = asin(elevationVector.y);  // range: -pi/2..pi/2

    // find the angle of arc.dir projected onto the arc-circle plane
    float gamma = atan(
        dot(arc.dir, elevationVector),  // arc-plane y
        dot(arc.dir, horizontal)        // arc-plane x
    );

    // stop beta from being negative
    if (gamma > pi) {
        alpha += pi;
        beta *= -1.0;
        gamma += pi;
        if (alpha > tau) { alpha -= tau; }
        if (gamma > tau) { gamma -= tau; }
    }

    vec3 texCoords = getTexCoords(alpha, beta, gamma);


    // some assertion statements
    if (!(
        arc.tangent.x >= 0.0                    &&
        isZero(horizontal.y)                    &&

        isZero(dot(arc.dir, arc.tangent))       &&
        isZero(dot(arc.dir, g))                 &&
        isZero(dot(arc.tangent, g))             &&

        // arc-circle space
        isZero(dot(horizontal, g))               &&
        isZero(dot(horizontal, elevationVector)) &&
        isZero(dot(g,          elevationVector)) &&

        texCoords.x >= 0.0 && texCoords.x <= 1.0 &&
        texCoords.y >= 0.0 && texCoords.y <= 1.0 &&
        texCoords.z >= 0.0 && texCoords.z <= 1.0 &&

        // the outer cross() asserts that the vectors are scalar multiples of each other
        isZero(length(cross(cross(horizontal, arc.dir), g)))
    )) {
        error = true;
    }
    return texCoords;
}

// // returns an arc from a alphaBetaGamma vector
// // ABG should be in texture coordinates
// Arc getArc(vec3 ABG) {

// }
