
smooth in vec4 pos_project;
uniform float time;

vec2 randomV = pos_project.xy * sin(time);  // requires access to screenspace coordinate
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
