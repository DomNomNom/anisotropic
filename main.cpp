#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <string.h>
// #include <random>


// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include "define.h"
#include "G308_Geometry.h"
#include "mytime.h"
#include "shader.h"
#include "textures.h"
#include "tweaker.h"
#include "config.h"

using namespace glm;

// Global Variables
GLuint window;
int window_wd = 600;
int window_ht = 600;
float aspectRatio = window_wd / window_ht;
float window_xOffset;

G308_Geometry* objects = NULL;
uint numObjects; // size of objects
uint currentObject = 2;


G308_Geometry skybox;

Shader shader;
Shader skyboxShader;
bool useShader = true;

vec3 UP = vec3(0, 1, 0);
glm::vec4 eye; // camera position
mat4 modelMatrix;
mat4 viewMatrix;
mat4 projectionMatrix;
mat3 normalMatrix;
mat4 eyeTransform;

float turntableAngle = 0.0;
float turntableElevation = 0.0;
float turntableElevationDefault = -30.0;
float scaleFactor = 1.0;
float cameraDistance = 4.5;
float exposure = 50.0;
bool exposure_enabled = true;

float mouse_x = 0.0;
float mouse_y = 0.0;
float anisotropy = DEFAULT_ANISOTROPY;
float tester2 = 50;
float tangentRotation = 1.0;
int tester_int = 1;
const glm::vec3 cacheResolution = glm::vec3(
    RESOLUTION_ALPHA,
    RESOLUTION_BETA,
    RESOLUTION_GAMMA
);
// const glm::vec3 noInterpBorderScale = glm::vec3(
//     (float)(RESOLUTION_ALPHA - 1) / RESOLUTION_ALPHA,
//     (float)(RESOLUTION_BETA  - 1) / RESOLUTION_BETA,
//     (float)(RESOLUTION_GAMMA - 1) / RESOLUTION_GAMMA
// );

bool viewLock = false;
bool turntable = false;
uint turntableFrame = 0;
uint turntableFrames = 50;

glm::vec4 lightVectors[] = {
    normalize(  glm::vec4(  1.0, 1.0, 1.0, 0.0)), // directional (vector to light)
                glm::vec4( -1.0, -1.0,-1.0, 1.0),  // point
                glm::vec4(  0.0, 20.0, 0.0, 1.0),  // spotPos
    normalize(  glm::vec4(  0.0, -1.0, 0.0, 0.0)), // spotDir
};

int numSamples = NUMSAMPLES_LIVE;
// glm::vec4 sampleDirections[numSamples];


float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};

GLuint lightmap;
GLuint lightmap_hdr;
GLuint cache;

// time stuff
float seconds = 0.f; // since the start of the program
int seconds_floor = 0;
int framecount = 0;
char window_title[200];




// Display function
void DisplayHandler() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // glEnable(GL_COLOR_MATERIAL);
    // GLfloat whiteSpecularMaterial[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,    material_emissive);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,     material_diffuse);;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,     material_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,    material_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,   material_shininess);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D,          cache       );
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,          lightmap_hdr);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP,    lightmap    );


    // matrcies
    modelMatrix = mat4(1.0); // load identity


    // view
    if (turntable && turntableFrame == turntableFrames) {
        turntable = false;
    }
    if (turntable) {
        turntableAngle = 360.0f * (float)turntableFrame / (float)turntableFrames;
        turntableElevation = turntableElevationDefault;
        turntableFrame += 1;
    }
    else {
        turntableAngle     = (mouse_x-0.5f) * -360.0f;
        turntableElevation = (mouse_y-0.5f) * -180.0f;
    }
    eyeTransform = mat4(1.0);
    eyeTransform = rotate(eyeTransform, turntableAngle,     vec3(0.0, 1.0, 0.0));
    eyeTransform = rotate(eyeTransform, turntableElevation, vec3(1.0, 0.0, 0.0));
    eye = glm::vec4(0.0, 0.0, cameraDistance, 1.0);
    eye = eyeTransform * eye;
    viewMatrix = lookAt(vec3(eye), vec3(0, 0, 0), UP);

    // printf("eye: %f %f %f\n",
    //     eye.x, eye.y, eye.z
    // );

    // normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));
    normalMatrix = mat3(modelMatrix);
    // normalMatrix = mat3(1.0);
    projectionMatrix = perspective(
        60.0f,
        (float)window_wd / (float)window_ht,
        0.1f,
        100.f
    );

    // skybox
    skyboxShader.bind();
    glUniform1f( glGetUniformLocation(skyboxShader.id(), "exposure"), exposure);
    glUniform1f( glGetUniformLocation(skyboxShader.id(), "exposure_enabled"), exposure_enabled);
    glUniform1f( glGetUniformLocation(skyboxShader.id(), "anisotropy"), anisotropy);
    glUniform1i( glGetUniformLocation(skyboxShader.id(), "lightmap"),     0); //Texture unit 0
    glUniform1i( glGetUniformLocation(skyboxShader.id(), "lightmap_hdr"), 1); //Texture unit 1
    glUniform1i( glGetUniformLocation(skyboxShader.id(), "cache"),        2); //Texture unit 2
    glUniform4fv(glGetUniformLocation(skyboxShader.id(), "eye"), 1, value_ptr(eye));
    glUniform4fv(glGetUniformLocation(skyboxShader.id(), "lightVectors"), 4, value_ptr(lightVectors[0]));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id(), "modelMatrix"     ), 1, false, value_ptr(modelMatrix       ));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id(), "viewMatrix"      ), 1, false, value_ptr(viewMatrix        ));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.id(), "projectionMatrix"), 1, false, value_ptr(projectionMatrix  ));
    glUniformMatrix3fv(glGetUniformLocation(skyboxShader.id(), "normalMatrix"    ), 1, false, value_ptr(normalMatrix      ));
    skybox.RenderGeometry();
    skyboxShader.unbind();


    modelMatrix = rotate(modelMatrix, 90.f, UP);
    if (currentObject == 2) { // teapot
        modelMatrix = translate(modelMatrix, vec3(0.0, -0.5, 0.0));
        modelMatrix = scale(modelMatrix, vec3(0.3));
    }
    else {
        modelMatrix = scale(modelMatrix, vec3(1.5));
    }
    normalMatrix = mat3(modelMatrix);


    if (useShader) {

        shader.bind();

        numSamples = (int) tester2;

        // set shader variables
        glUniform1i( glGetUniformLocation(shader.id(), "tester_int"), tester_int);
        glUniform1f( glGetUniformLocation(shader.id(), "exposure"), exposure);
        glUniform1i( glGetUniformLocation(shader.id(), "exposure_enabled"),  exposure_enabled);
        glUniform1i( glGetUniformLocation(shader.id(), "lightmap"),     0); //Texture unit 0
        glUniform1i( glGetUniformLocation(shader.id(), "lightmap_hdr"), 1); //Texture unit 1
        glUniform1i( glGetUniformLocation(shader.id(), "numSamples"), numSamples);
        glUniform1f( glGetUniformLocation(shader.id(), "time"),  seconds);
        glUniform1f( glGetUniformLocation(shader.id(), "anisotropy" ),  anisotropy);
        glUniform1f( glGetUniformLocation(shader.id(), "tester2"),  tester2);
        glUniform1f( glGetUniformLocation(shader.id(), "tangentRotation"),  tangentRotation);
        glUniform2f( glGetUniformLocation(shader.id(), "mouse"), mouse_x, mouse_y);
        glUniform3f(glGetUniformLocation(shader.id(), "cacheResolution"), cacheResolution.x, cacheResolution.y, cacheResolution.z);
        // glUniform3fv(glGetUniformLocation(shader.id(), "sampleDirections"), numSamples, value_ptr(sampleDirections[0]));
        glUniform4fv(glGetUniformLocation(shader.id(), "eye"), 1, value_ptr(eye));
        glUniform4fv(glGetUniformLocation(shader.id(), "lightVectors"), 4, value_ptr(lightVectors[0]));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "modelMatrix"     ), 1, false, value_ptr(modelMatrix       ));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "viewMatrix"      ), 1, false, value_ptr(viewMatrix        ));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projectionMatrix"), 1, false, value_ptr(projectionMatrix  ));
        glUniformMatrix3fv(glGetUniformLocation(shader.id(), "normalMatrix"    ), 1, false, value_ptr(normalMatrix      ));


    }

    // iterate over the geometries?
    objects[currentObject].RenderGeometry();


    if (useShader) shader.unbind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glutSwapBuffers();
    glutPostRedisplay();  // active rendering
}

// Reshape function
void reshapeHandler(int wd, int ht) {
    window_wd = wd;
    window_ht = ht;

    window_xOffset = (wd - (ht/aspectRatio)) / 2.0f;

    glViewport(window_xOffset, 0, window_ht/aspectRatio, window_ht);
}

void MouseMoveHander(int x, int y){
    if (!viewLock) {
        mouse_x = x / (float)window_wd;
        mouse_y = y / (float)window_ht;
    }
}

void MouseHandler(int, int state, int, int) {
    if (state == GLUT_DOWN) {
        if (!viewLock) {
            printf("\n");
            printf("mouse_x = %f;\n", mouse_x);
            printf("mouse_y = %f;\n", mouse_y);
        }
        viewLock  = !viewLock;
    }
    // glutPostRedisplay();
}


// Input
void KeyHandler(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit (0);
            break;
        case 's': // toggle shaders
            useShader = !useShader;
            break;
        case 't': turntable = true; turntableFrame = 0;  break;
        case 'e': exposure_enabled = !exposure_enabled; break;
        case '1': tester_int = 0;   break;
        case '2': tester_int = 1;   break;
        case '3': tester_int = 2;   break;
        case '4': tester_int = 3;   break;
        case '5': tester_int = 4;   break;
        case '6': tester_int = 5;   break;
        case '7': tester_int = 6;   break;
        case '8': tester_int = 7;   break;
        case '9': tester_int = 8;   break;

        // changing the model
        case '[': if (currentObject > 0           ) currentObject -= 1;    break;
        case ']': if (currentObject < numObjects-1) currentObject += 1;    break;

        // specific camera angles
        case 'm':
            viewLock = true;
            mouse_x = 0.361667;
            mouse_y = 0.393333;
            break;
        case 'n':
            viewLock = true;
            mouse_x = 0.460000;
            mouse_y = 0.761667;
            break;
        case 'b':
            viewLock = true;
            mouse_x = 0.25;
            mouse_y = 0.5;
            break;

    }

    // glutPostRedisplay();
}

void AnimateScene(void) {
    float timeDiff = time_dt();
    // printf("%f\n", timeDiff);
    seconds += timeDiff;
    turntableAngle = seconds * -300.0; // rotate slowly along the vertical axis

    framecount += 1;
    if (floor(seconds) > seconds_floor) { // we are in a new second
        seconds_floor = floor(seconds);
        sprintf(window_title, "Anisotropic Shader: %.3f  %02dfps", anisotropy, framecount);
        glutSetWindowTitle(window_title);
        framecount = 0;
    }

    glutPostRedisplay();  // Force redraw
}

// http://stackoverflow.com/questions/686353/c-random-float-number-generation
float random(float min, float max) {
    return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(max-min)));
}


int main(int argc, char** argv) {
    time_init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(window_wd, window_ht);
    window = glutCreateWindow("Anisotropic Shader!    :D");

    glutDisplayFunc(DisplayHandler);
    glutReshapeFunc(reshapeHandler);
    glutIdleFunc(AnimateScene);
    glutKeyboardFunc(KeyHandler);
    glutMouseFunc(MouseHandler);
    glutMotionFunc(MouseMoveHander);
    glutPassiveMotionFunc(MouseMoveHander);


    // the .objs we are going to render
    numObjects = 3;
    objects = new G308_Geometry[numObjects];
    objects[0].ReadOBJ("./assets/sphereWithNormals.obj");
    objects[1].ReadOBJ("./assets/cylinder100.obj");
    objects[2].ReadOBJ("./assets/teapot.obj");
    for (uint i=0; i<numObjects; ++i) {
        objects[i].CreateGLPolyGeometry();
        // objects[i].CreateGLWireGeometry();
    }

    skybox.ReadOBJ("assets/box.obj");
    skybox.CreateGLPolyGeometry();

    // initialize sampleDirections
    // G308_Geometry *temp = new G308_Geometry();
    // temp->ReadOBJ("assets/sphere162.obj");
    // for (int i=0; i<numSamples; ++i) {
        // sampleDirections[i].x = temp->m_pVertexArray[i].x;
        // sampleDirections[i].y = temp->m_pVertexArray[i].y;
        // sampleDirections[i].z = temp->m_pVertexArray[i].z;
    // }
    // delete temp;

    shader.init(
        "shader.vert",
        "shader.frag"
    );
    skyboxShader.init(
        "shader.vert",
        "skybox.frag"
    );

    lightmap_hdr = exr_texture_load("assets/exr/vuw_sunny_hdr_mod1_320_32.exr");
    lightmap = png_cubemap_load("assets/bright_dots/");

    cache = 0;
    if (cacheType == ARC) {
        // cache = exr_cubetex_load("assets/cache2/cache", RESOLUTION_GAMMA);
    }
    else if (cacheType == SPHERICAL_HARMONIC) {
        // TODO
    }

    tweak(&anisotropy);
    tweak(&tester2);
    tweak(&exposure);
    tweak(&tangentRotation);

    glutMainLoop();

    if (objects != NULL) delete[] objects;

    return 0;
}
