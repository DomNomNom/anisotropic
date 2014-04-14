
//---------------------------------------------------------------------------
//
// Copyright (c) 2012 Taehyun Rhee
//
// This software is provided 'as-is' for assignment of COMP308
// in ECS, Victoria University of Wellington,
// without any express or implied warranty.
// In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

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
mat3 normalMatrix;

float turntableAngle = 0.f;
float scaleFactor = 1.f;

float seconds = 0.f; // since the start
float mouse_x = 0.0;
float mouse_y = 0.0;
float tester = 0.0;

bool animated = false;
bool turntable = true;

// which harmonic to choose
int sh_l = 4;
int sh_m = 3;

const unsigned int weights_size = 36;
float weights[weights_size] = {
    0,
    1, 0, 0,
    0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
float sinSpeeds[weights_size];

glm::vec4 eye = glm::vec4(0.0, 0.0, 10.0, 1.0);
glm::vec4 lightVectors[] = {
    normalize(  glm::vec4(  1.0, 1.0, -1.0, 0.0)), // directional (vector to light)
                glm::vec4( -1.0, -1.0,-1.0, 1.0),  // point
                glm::vec4(  0.0, 20.0, 0.0, 1.0),  // spotPos
    normalize(  glm::vec4(  0.0, -1.0, 0.0, 0.0)), // spotDir
};


float light_direction[] = {1.0f, 0.0f, 0.0f};
GLfloat light_ambient[]     = {0.1, 0.1, 0.1, 1.0};
GLfloat light_diffuse[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat light_specular[]    = {1.0, 1.0, 1.0, 1.0};
GLfloat material_emissive[] = {0.0, 0.0, 0.0, 1.0};
GLfloat material_diffuse[]  = {1.0, 0.0, 0.0, 1.0};
GLfloat material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
GLfloat material_specular[] = {8.8, 8.8, 8.8, 1.0};
GLfloat material_shininess[] = {89};

GLuint texture_colour_ramp;
GLuint texture_hammering;

// Display function
void DisplayHandler() {
    printf("omg?\n");
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
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, texture_colour_ramp);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, texture_hammering);

    if (turntable) {
        turntableAngle = 50 * seconds;
    }
    else {
        turntableAngle = (mouse_x+0.25f) * 360.0f;
    }
    printf("omg? \n");
    // load matrix
    modelMatrix = mat4(1.0); // load identity
    modelMatrix = rotate(modelMatrix, 90.f, UP);
    // modelMatrix = rotate(modelMatrix,  turntableAngle, vec3(0.0, 1.0, 0.0));
    modelMatrix = rotate(modelMatrix, (mouse_y-0.5f) * 180.0f, vec3(0.0, 0.0, 1.0));
    printf("omg %d\n", (mouse_y-0.5f) * 180.0f);
    // modelMatrix = rotate(modelMatrix, 0.1f *turntableAngle, UP);
    modelMatrix = scale(modelMatrix, vec3(1.5));
    // modelMatrix = translate(modelMatrix, vec3(0, 0, 0));
    normalMatrix = transpose(inverse(mat3(viewMatrix * modelMatrix)));

    if (useShader) {
        shader.bind();

        if (animated) {
            float weightScaling = (weights_size==36)? 0.3 : 0.8; // so that we get values that are slightly below +-1.0
            for (unsigned int i=0; i<weights_size; ++i) {
                weights[i] = weightScaling * sin(sinSpeeds[i] * seconds);
            }
        }

        // printf("wigh: %f\n", weights[0]);

        // set shader variables
        glUniform1f(glGetUniformLocation(shader.id(), "time"),  seconds);
        glUniform1f(glGetUniformLocation(shader.id(), "tester"),  tester);
        glUniform2f(glGetUniformLocation(shader.id(), "mouse"), mouse_x, mouse_y);
        glUniform1i(glGetUniformLocation(shader.id(), "l"), sh_l);
        glUniform1i(glGetUniformLocation(shader.id(), "m"), sh_m);
        glUniform1fv(glGetUniformLocation(shader.id(), "weights"), weights_size, weights);
        glUniform3fv(glGetUniformLocation(shader.id(), "eye"), 1, value_ptr(eye));
        glUniform4fv(glGetUniformLocation(shader.id(), "lightVectors"), 4, value_ptr(lightVectors[0]));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "modelMatrix" ), 1, false, value_ptr(modelMatrix ));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "viewMatrix"  ), 1, false, value_ptr(viewMatrix  ));
        glUniformMatrix3fv(glGetUniformLocation(shader.id(), "normalMatrix"), 1, false, value_ptr(normalMatrix));


    }

    for (int i=0; i<objects; ++i) {
        g_pGeometry[i].RenderGeometry();
    }

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



// Set Light
void SetLightHandler() {
    glPushMatrix();
        glRotatef(60, 0, 0, 1);
        glRotatef(-30, 0, 1, 0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_direction);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glEnable(GL_LIGHT0);
    glPopMatrix();
}

void MouseMoveHander(int x, int y){
    mouse_x = x / (float)window_wd;
    mouse_y = y / (float)window_ht;
    printf("o\n");
}

void MouseHandler(int, int , int, int) {
    // printf("tester value: %f\n", tester);
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
        // case '=':  sh_l += 1; break;
        // case '+':  sh_l += 1; break;
        // case '-':  sh_l -= 1; break;
        // case ']':  sh_m += 1; break;
        // case '[':  sh_m -= 1; break;
        case 'a': animated  = !animated;    break;
        case 't': turntable = !turntable;   break;
    }

    // sh_l = clamp(sh_l, 0, 5);
    // sh_m = clamp(sh_m, -sh_l, sh_l);
    // printf("l m: %d %d\n", sh_l, sh_m);

    glutPostRedisplay();
}

void AnimateScene(void) {
    seconds += time_dt();
    turntableAngle = seconds * -300.0; // rotate slowly along the vertical axis

    glutPostRedisplay();  // Force redraw
}

// Set Camera Position
void SetCameraHandler() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        G308_FOVY,
        (double) window_wd / (double) window_ht,
        G308_ZNEAR_3D,
        G308_ZFAR_3D
    );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // gluLookAt(eye.x, eye.y, eye.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    viewMatrix = lookAt(vec3(eye), vec3(0, 0, 0), UP);
}


// http://stackoverflow.com/questions/686353/c-random-float-number-generation
float random(float min, float max) {
    return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(max-min)));
}


int main(int argc, char** argv) {
    time_init();
    objects = argc - 1;
    if (objects < 1) {
        printf("I require one model file. example: assets/gunblade.obj\n");
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


    SetLightHandler();
    SetCameraHandler();

    // texture_colour_ramp = png_texture_load("assets/colourRamp.png");
    // texture_hammering  =  png_texture_load("assets/hammering.png");

    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    shader.init(
        "shaders/shader.vert",
        "shaders/shader.frag"
    );

    for (unsigned int i=0; i<weights_size; ++i) {
        sinSpeeds[i] = random(0.1, 2.5);
        // sinSpeeds[i] = (i%2)? 1 : 1.5;
        tweak(weights + i);
    }

    glutMainLoop();

    if (g_pGeometry != NULL) delete[] g_pGeometry;

    return 0;
}
