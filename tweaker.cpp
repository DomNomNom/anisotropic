#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>

#include <GL/glut.h>

bool initialized = false;

GLuint tweaker_window;
int tweaker_window_wd = 300;
int tweaker_window_ht = 500;

const float backgroundShade = 0.2;
const float barShade = 0.4;
const float oneShade = 0.7;


std::vector<float *> vars; // the variables we are going to tweak

bool tweaking = false;
unsigned int startSelectedBar = 0;

// float extremify(float x) {
//     x *= 1.0 / x_one;
//     x *= x*x*x*x; // x**5
//     return x;
// }

// // inverse of x**5. also handles negative numbers
// float fifthRoot(float x) {
//     bool negative = (x < 0);
//     if (negative) x *= -1;
//     x = pow(x, 1.0/5.0); // positive 5th root
//     if (negative) x *= -1;
//     return x;
// }

// float extremify_inverse(float x) {
//     x = fifthRoot(x);
//     x *= x_one;
//     return x;
// }

// // http://stackoverflow.com/questions/374316/round-a-double-to-x-significant-figures
// static double RoundToSignificantDigits(double d, int digits){
//     if(d == 0)
//         return 0;

//     double scale = pow(10, floor(log10(abs(d))) + 1);
//     return scale * round(d / scale, digits);
// }

// http://stackoverflow.com/questions/13094224/a-c-routine-to-round-a-float-to-n-significant-digits
float RoundToSignificantDigits(float a, int digits) {
  int   exp_base10 = round(log10(a));
  float man_base10 = a*pow(10.0, -exp_base10);
  float factor     = pow(10.0, -digits+1);
  float truncated_man_base10 = man_base10 - fmod(man_base10, factor);
  float rounded_remainder    = fmod(man_base10, factor)/factor;

  rounded_remainder = rounded_remainder > 0.5 ? 1.0*factor : 0.0;

  return (truncated_man_base10 + rounded_remainder)*pow(10.0, exp_base10) ;
}

float extremify(float x) {
    bool negative = (x < 0.0);
    if (negative) x *= -1.0;
    x *= 2.0;
    x = pow(10, x);
    x -= 1.0;
    x *= 0.1;
    x *= x;
    x = RoundToSignificantDigits(x, 2);
    if (negative) x *= -1.0;
    return x;
}

float extremify_inverse(float x) {
    bool negative = (x < 0.0);
    if (negative) x *= -1.0;
    x = sqrt(x);
    x /= 0.1;
    x += 1.0;
    x = log10(x);
    x /= 2.0;
    if (negative) x *= -1.0;
    return x;
}

const float x_one = extremify_inverse(1.0); // the openGL coord where the var result will be 1
const float x_one_wd = 0.005; // how wide the vertical bar is

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: not use glRectf()... but it is convenient

    float barHeight = 2.0f / vars.size();
    for (unsigned int i=0; i<vars.size(); ++i) {
        // render the horizontal bar
        glColor3f(barShade, barShade, barShade);
        float top = (2.0f * i) / vars.size() - 1.0f;
        top *= -1;
        top -= barHeight;

        glRectf(
            0.0f,
            top,
            extremify_inverse(*vars[i]),
            top + barHeight
        );

        // draw the number on the left
        glColor3f(1.0, 1.0, 1.0);
        glRasterPos2f(-1, top);
        char string[80];
        sprintf(string, "%f", *vars[i]);
        for (int i=0; i<80 && string[i] != '\0'; ++i) {
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);

        }
        // glutStrokeString(GLUT_STROKE_ROMAN, "omg");

    }


    glColor3f(oneShade, oneShade, oneShade);
    glRectf(
        x_one - x_one_wd, -1.0,
        x_one + x_one_wd, +1.0
    );
    glRectf(
        -x_one - x_one_wd, -1.0,
        -x_one + x_one_wd, +1.0
    );

    glutSwapBuffers();

    glutPostRedisplay();  // active rendering
}

// note: this may return something >= vars.size();
unsigned int varsIndex(float y) {
    return (y / (float)tweaker_window_ht) * vars.size();
}

static void MouseMoveHander(int x, int y){
    if (!tweaking) return;

    // select which bar we are going to change
    unsigned int selectedBar = (
        // allow changing of other bars in one drag if they press shift
        (glutGetModifiers() == GLUT_ACTIVE_SHIFT) ?
        varsIndex(y) : startSelectedBar
    );
    if (selectedBar >= vars.size()) return;


    float X = x / (float)tweaker_window_wd;
    X *= 2.0f;
    X -= 1.0f; // get it in the range (-1, 1);
    *(vars[selectedBar]) = extremify(X);
    // printf("tweaked to: %f %f %f\n",
    //     *(vars[selectedBar]),
    //     X,
    //     extremify_inverse(extremify(X))
    // );

    // printf("moving %d %d\n", x, y);

    glutPostRedisplay();
}

static void MouseHandler(int button, int state, int, int y) {
    if (button == 0) {
        if (state == GLUT_DOWN) {
            tweaking = true;
            startSelectedBar = varsIndex(y);
        }
        else {
            tweaking = false;
        }
    }

    glutPostRedisplay();
}

static void KeyHandler(unsigned char key, int, int) {
    switch (key) {
        case 27: // Escape -> exit
            exit(0);
            break;
    }

    glutPostRedisplay();
}

void initialize() {
    glutInitWindowSize(tweaker_window_wd, tweaker_window_ht);
    tweaker_window = glutCreateWindow("tweaker");

    glutPositionWindow(650,40);  //define a window position for second window
    glClearColor(backgroundShade, backgroundShade, backgroundShade, 1.0f);
    // glutReshapeFunc(handleResize); // register callbacks for second window, which is now current
    glutDisplayFunc(display);
    glutMouseFunc(MouseHandler);
    glutMotionFunc(MouseMoveHander);
    glutPassiveMotionFunc(MouseMoveHander);
    glutKeyboardFunc(KeyHandler);



    initialized = true;
}



void tweak(float *var) {
    vars.push_back(var);

    if (!initialized) {
        initialize();
    }
}
