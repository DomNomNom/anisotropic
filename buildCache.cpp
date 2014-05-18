
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "shader.h"
#include "textures.h"

const int width  = 256 * 2;
const int height = 256 * 2;
const float pi = 3.141592653589793;

Imf::Rgba pixelData[width * height];
float pixelData_float[width * height * 4];

float mouse_x = 0.0;
float mouse_y = 0.0;



// in radians
float range_tangent   = 90.0  /360* 2.0*pi;
float range_bitangent =  0.0  /360* 2.0*pi;

GLuint window;
Shader shader;

GLuint lightmap;
GLuint lightmap_hdr;

GLuint framebufferName; // The frame buffer object
GLuint renderTexture;   // The texture we're going to render to


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


    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
    glViewport(0, 0, width, height);
    // check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("bad framebuffer\n");
        exit(-1);
    }

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,       lightmap_hdr);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_CUBE_MAP, lightmap);


    // glUniform1f( glGetUniformLocation(shader.id(), "exposure"), 7.f);
    // glUniform1f( glGetUniformLocation(shader.id(), "tester"), tester);
    shader.bind();

    float gamma =  mouse_x * 2 * pi;
    // printf("gamma (degrees) %3.f  y %3.f\n", mouse_x*360.0, mouse_y*360.0);

    glUniform1f( glGetUniformLocation(shader.id(), "hello"),        gamma);
    glUniform1i( glGetUniformLocation(shader.id(), "lightmap"),     0); //Texture unit 0
    glUniform1i( glGetUniformLocation(shader.id(), "lightmap_hdr"), 1); //Texture unit 1
    glUniform1f( glGetUniformLocation(shader.id(), "range_tangent"),    range_tangent  );
    glUniform1f( glGetUniformLocation(shader.id(), "range_bitangent"),  range_bitangent);


    drawWindowSizedQuad();

    shader.unbind();


    // display the framebuffer
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);// Render to the screen
    glViewport(0, 0, width, height);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTexture);

    glColor4f(1,1,1,1);
    drawWindowSizedQuad();
    glDisable(GL_TEXTURE_2D);

    // save output
    glReadPixels(0,0, width, height, GL_RGBA, GL_HALF_FLOAT, pixelData);
    // glReadPixels(0,0, width, height, GL_RGBA, GL_FLOAT, pixelData_float);
    // for (int i=0; i<width*height; ++i) {
    //     pixelData[i].r = pixelData_float[i*4 + 0];
    //     pixelData[i].g = pixelData_float[i*4 + 1];
    //     pixelData[i].b = pixelData_float[i*4 + 2];
    //     pixelData[i].a = pixelData_float[i*4 + 3];
    // }
    glReadPixels(0,0, width, height, GL_RGBA, GL_FLOAT, pixelData_float);
    // int zeros = 0;
    // for (int i=0; i<width*height; ++i) {
    //     float sum = (
    //         pixelData_float[i*4 + 0] +
    //         pixelData_float[i*4 + 1] +
    //         pixelData_float[i*4 + 2]
    //     //  pixelData_float[i*4 + 3]
    //     );
    //     if (sum == 0.f) zeros += 1;
    // }
    // printf("minmax %f\n", zeros / float(width*height));
    exr_texture_save("assets/cache/test.exr", pixelData, width, height);


    glutSwapBuffers();

    // printf("%f %f\n", mouse_x * 2 * pi, mouse_y);
    // printf("rendered. \n");

    glutPostRedisplay();
}

void keyHandler(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            glutDestroyWindow(window);
            exit (0);
            break;
    }
}

void mouseMoveHander(int x, int y){
    mouse_x = x / (float) width ;
    mouse_y = y / (float) height;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    window = glutCreateWindow("Building the cache!    :D");

    glutDisplayFunc(displayHandler);
    glutKeyboardFunc(keyHandler);
    glutMotionFunc(mouseMoveHander);
    glutPassiveMotionFunc(mouseMoveHander);


    shader.init(
        "buildCache.vert",
        "buildCache.frag"
    );

    // float foo = 5.123456789;
    // // half bar = foo;
    // // foo =  bar;
    // printf("hai %f\n", foo);

    lightmap_hdr = exr_texture_load("assets/exr/vuw_sunny_hdr_mod1_320_32.exr");
    lightmap     = png_cubemap_load("assets/bright_dots/");
    // lightmap  = png_cubemap_load("assets/beach_bright128/");


    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glDisable(GL_TEXTURE_2D);

    // create framebufferName and attach texture A to it
    glGenFramebuffers(1, &framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);



    glutMainLoop();
    return 0;
}
