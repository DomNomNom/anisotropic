
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "shader.h"
#include "textures.h"
#include "config.h"
#include "mytime.h"

const int width  = 256 * 2;
const int height = 256 * 2;
const float pi = 3.141592653589793;

float mouse_x = 0.5;
float mouse_y = 0.5;


// in radians
float range_tangent   = 90.0  /360* 2.0*pi;
float range_bitangent =  0.0  /360* 2.0*pi;

GLuint window;
Shader shader;

GLuint texture;

bool useExposure = false;
bool useCache = true;
GLuint cache;
uint cacheViewAxis = 3;

float seconds = 0.0;

// draws two triangles that cover the screen
void drawWindowSizedQuad() {
    float tv = 1.0;
    glBegin(GL_TRIANGLES);
        glTexCoord2f(0, 0); glVertex3f(-tv,-tv, 0.0);
        glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
        glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);

        glTexCoord2f(1, 1); glVertex3f( tv, tv, 0.0);
        glTexCoord2f(0, 1); glVertex3f(-tv, tv, 0.0);
        glTexCoord2f(1, 0); glVertex3f( tv,-tv, 0.0);
    glEnd();
}

void displayHandler() {
    seconds += time_dt();

    // if (useCache) {
    //     int slice = (int)(mouse_x * RESOLUTION_GAMMA);
    //     char texturePath[50];
    //     sprintf(texturePath, "assets/cache2/cache%02d.exr", slice);
    //     printf("slice: %d\n", slice);
    //     texture = exr_texture_load(texturePath);
    // }

    glViewport(0, 0, width, height);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, cache);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture);

    shader.bind();

    // glUniform1f( glGetUniformLocation(shader.id(), "tester"), mouse_x);
    glUniform1i( glGetUniformLocation(shader.id(), "exr"), 0);
    glUniform1i( glGetUniformLocation(shader.id(), "cache"), 1);
    glUniform1i( glGetUniformLocation(shader.id(), "useCache"), useCache);
    glUniform1i( glGetUniformLocation(shader.id(), "useExposure"), useExposure);
    glUniform1i( glGetUniformLocation(shader.id(), "cacheViewAxis"), cacheViewAxis);
    glUniform1f( glGetUniformLocation(shader.id(), "texcoord"), mouse_x);
    glUniform1f( glGetUniformLocation(shader.id(), "time"), seconds);
    glUniform1f( glGetUniformLocation(shader.id(), "exposure"), 100 * mouse_y);

    // glUniform1f( glGetUniformLocation(shader.id(), "exposure"),  mouse_x);
    printf("mouse %f\n", mouse_x);

    drawWindowSizedQuad();

    shader.unbind();


    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();

    // printf("%f %f\n", mouse_x * 2 * pi, mouse_y);
    // printf("rendered. \n");

    glutPostRedisplay();
}

void keyHandler(unsigned char key, int, int) {
    switch (key) {
        case '1': cacheViewAxis = 1;    break;
        case '2': cacheViewAxis = 2;    break;
        case '3': cacheViewAxis = 3;    break;
        case 'e': useExposure = !useExposure;  break;
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit(0);
            break;
    }
}


void MouseHandler(int, int state, int, int) {
    if (state == GLUT_DOWN) {
        printf("mouse %f\n", mouse_x);
    }
    // glutPostRedisplay();
}

void mouseMoveHander(int x, int y){
    mouse_x = x / (float) width ;
    mouse_y = y / (float) height;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    window = glutCreateWindow("Viewing EXR!    :D");

    glutDisplayFunc(displayHandler);
    glutKeyboardFunc(keyHandler);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);


    shader.init(
        "exrViewer.vert",
        "exrViewer.frag"
    );


    // texture = exr_texture_load("assets/cache/test.exr");
    // texture = exr_texture_load("./assets/cache/cache02.exr");
    // texture = exr_texture_load("./assets/cache/cache04.exr");
    if (useCache) {
        cache = exr_cubetex_load("assets/cache2/cache", RESOLUTION_GAMMA);
    }
    else {
        texture = exr_texture_load("assets/exr/vuw_sunny_hdr_mod1_320_32.exr");
    }


    // tweak(&exposure);

    glutMainLoop();
    return 0;
}
