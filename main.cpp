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

using namespace glm;

// Global Variables
GLuint window;
int window_wd = 600;
int window_ht = 600;
float aspectRatio = window_wd / window_ht;
float window_xOffset;

G308_Geometry* g_pGeometry = NULL;
int objects; // size of g_pGeometry

Shader shader;
bool useShader = true;

vec3 UP = vec3(0, 1, 0);
mat4 modelMatrix;
mat4 viewMatrix;
mat4 projectionMatrix;
mat3 normalMatrix;

float turntableAngle = 0.f;
float scaleFactor = 1.f;

float seconds = 0.f; // since the start
float mouse_x = 0.0;
float mouse_y = 0.0;
float tester = 0.0;

bool animated = false;
bool turntable = false;

glm::vec4 eye = glm::vec4(0.0, 0.0, 5.0, 1.0);
glm::vec4 lightVectors[] = {
    normalize(  glm::vec4(  1.0, 1.0, 1.0, 0.0)), // directional (vector to light)
                glm::vec4( -1.0, -1.0,-1.0, 1.0),  // point
                glm::vec4(  0.0, 20.0, 0.0, 1.0),  // spotPos
    normalize(  glm::vec4(  0.0, -1.0, 0.0, 0.0)), // spotDir
};

const int numSamples = 162;
glm::vec4 sampleDirections[numSamples];


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
GLuint texture_hammering;

int framecount = 0;
int seconds_floor = 0;
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

    tester = mouse_x - 0.5;
    tester *= 20.0;
    tester = tester*tester*tester*tester*tester;
    // printf("tester %f\n", tester);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, lightmap);

    if (turntable) {
        turntableAngle = 50 * seconds;
    }
    else {
        turntableAngle = (mouse_x+0.25f) * 360.0f;
    }

    // matrcies
    modelMatrix = mat4(1.0); // load identity
    modelMatrix = rotate(modelMatrix, 90.f, UP);
    modelMatrix = rotate(modelMatrix, (mouse_y-0.5f) * 180.0f, vec3(0.0, 0.0, 1.0));
    modelMatrix = rotate(modelMatrix,  turntableAngle, vec3(0.0, 1.0, 0.0));
    // printf("omg %d\n", (mouse_y-0.5f) * 180.0f);
    // modelMatrix = rotate(modelMatrix, 0.1f *turntableAngle, UP);
    modelMatrix = scale(modelMatrix, vec3(1.5));
    // modelMatrix = translate(modelMatrix, vec3(0, 0, 0));
    normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));
    viewMatrix = lookAt(vec3(eye), vec3(0, 0, 0), UP);
    projectionMatrix = perspective(
        60.0f,
        (float)window_wd / (float)window_ht,
        0.1f,
        100.f
    );


    if (useShader) {
        shader.bind();

        // set shader variables
        glUniform1f( glGetUniformLocation(shader.id(), "time"),  seconds);
        glUniform1f( glGetUniformLocation(shader.id(), "tester"),  tester);
        glUniform2f( glGetUniformLocation(shader.id(), "mouse"), mouse_x, mouse_y);
        glUniform1i( glGetUniformLocation(shader.id(), "skybox"), 0); //Texture unit 0
        glUniform1i( glGetUniformLocation(shader.id(), "numSamples"), numSamples); //Texture unit 0
        glUniform3fv(glGetUniformLocation(shader.id(), "sampleDirections"), numSamples, value_ptr(sampleDirections[0]));
        glUniform3fv(glGetUniformLocation(shader.id(), "eye"), 1, value_ptr(eye));
        glUniform4fv(glGetUniformLocation(shader.id(), "lightVectors"), 4, value_ptr(lightVectors[0]));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "modelMatrix"     ), 1, false, value_ptr(modelMatrix       ));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "viewMatrix"      ), 1, false, value_ptr(viewMatrix        ));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "projectionMatrix"), 1, false, value_ptr(projectionMatrix  ));
        glUniformMatrix3fv(glGetUniformLocation(shader.id(), "normalMatrix"    ), 1, false, value_ptr(normalMatrix      ));


    }

    g_pGeometry[0].RenderGeometry();


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
    mouse_x = x / (float)window_wd;
    mouse_y = y / (float)window_ht;
}

void MouseHandler(int, int , int, int) {
    glutPostRedisplay();
}


void KeyHandler(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit (0);
            break;
        case 's': // toggle shaders
            useShader = !useShader;
            break;
        case 'a': animated  = !animated;    break;
        case 't': turntable = !turntable;   break;
    }

    glutPostRedisplay();
}

void AnimateScene(void) {
    seconds += time_dt();
    turntableAngle = seconds * -300.0; // rotate slowly along the vertical axis

    framecount += 1;
    if (floor(seconds) > seconds_floor) { // we are in a new second
        seconds_floor = floor(seconds);
        sprintf(window_title, "Anisotropic Shader!    %2dfps   :D", framecount);
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
    objects = argc - 1;
    if (objects < 1) {
        printf("I require one model file. example: assets/sphere.obj\n");
        exit(EXIT_FAILURE);
    }

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


    g_pGeometry = new G308_Geometry[objects];
    for (int i=0; i<objects; ++i) {
        g_pGeometry[i].ReadOBJ(argv[i+1]);        // 1) read OBJ function
        g_pGeometry[i].CreateGLPolyGeometry();    // 2) create GL Geometry as polygon
        g_pGeometry[i].CreateGLWireGeometry();    // 3) create GL Geometry as wireframe
    }

    // initialize sampleDirections
    G308_Geometry *temp = new G308_Geometry();
    temp->ReadOBJ("assets/sphere162.obj");
    for (int i=0; i<numSamples; ++i) {
        sampleDirections[i].x = temp->m_pVertexArray[i].x;
        sampleDirections[i].y = temp->m_pVertexArray[i].y;
        sampleDirections[i].z = temp->m_pVertexArray[i].z;
    }
    delete temp;

    shader.init(
        "shaders/shader.vert",
        "shaders/shader.frag"
    );
    lightmap = png_cubemap_load("assets/beach_bright128/");

    glutMainLoop();

    if (g_pGeometry != NULL) delete[] g_pGeometry;

    return 0;
}
