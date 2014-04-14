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

#include "G308_Geometry.h"
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

#include <iostream>

using namespace std;

bool vertexNormals = false;

G308_Point *verticies[3]; // pointers to points  of the current triangle
G308_Normal  *normals[3]; // pointers to normals of the current triangle

G308_Geometry::G308_Geometry(void) {
    m_pVertexArray = NULL;
    m_pNormalArray = NULL;
    m_pUVArray = NULL;
    m_pTriangles = NULL;

    mode = G308_SHADE_POLYGON;

    m_nNumPoint = m_nNumUV = m_nNumPolygon = 0;
    m_glGeomListPoly = m_glGeomListWire = 0;
}

G308_Geometry::~G308_Geometry(void) {
    if (m_pVertexArray  != NULL)    delete[] m_pVertexArray;
    if (m_pNormalArray  != NULL)    delete[] m_pNormalArray;
    if (m_pUVArray      != NULL)    delete[] m_pUVArray;
    if (m_pTriangles    != NULL)    delete[] m_pTriangles;
}

float length(G308_Normal *n) {
    return sqrt(
        (n->x * n->x)  +
        (n->y * n->y)  +
        (n->z * n->z)
    );
}

void normalize(G308_Normal *n) {
    float len = length(n);
    if (!(len <0 || len > 0)) {
        printf("can't normalize!\n");
        n->x = 0;
        n->y = 0;
        n->z = 1;
        return;
    }

    len = 1.0f / len;
    n->x *= len;
    n->y *= len;
    n->z *= len;
}

void add_equals(G308_Normal *n, G308_Normal *toAdd) {
    n->x += toAdd->x;
    n->y += toAdd->y;
    n->z += toAdd->z;
}

void G308_Geometry::calculateNormal(G308_Triangle* face, G308_Normal *target) {
    // vector subtraction
    G308_Point diff12;
    G308_Point diff13;
    diff12.x = m_pVertexArray[face->v2].x - m_pVertexArray[face->v1].x;
    diff12.y = m_pVertexArray[face->v2].y - m_pVertexArray[face->v1].y;
    diff12.z = m_pVertexArray[face->v2].z - m_pVertexArray[face->v1].z;
    diff13.x = m_pVertexArray[face->v3].x - m_pVertexArray[face->v1].x;
    diff13.y = m_pVertexArray[face->v3].y - m_pVertexArray[face->v1].y;
    diff13.z = m_pVertexArray[face->v3].z - m_pVertexArray[face->v1].z;

    // cross product
    target->x = (diff12.y*diff13.z - diff12.z*diff13.y);
    target->y = (diff12.z*diff13.x - diff12.x*diff13.z);
    target->z = (diff12.x*diff13.y - diff12.y*diff13.x);

    normalize(target);
}

//-------------------------------------------------------
// This function read obj file having
// triangle faces consist of vertex v, texture coordinate vt, and normal vn
// e.g. f v1/vt1/vn1 v2/vt1/vn2 v3/vt3/vn3
//
// [Assignment1]
// You can revise the following function for reading other variations of obj file
// 1) f v1//vn1 v2//vn2 v3//vn3 ; no texture coordinates such as bunny.obj
// 2) f v1 v2 v3 ; no normal and texture coordinates such as dragon.obj
// The case 2) needs additional functions to build vertex normals
//--------------------------------------------------------
void G308_Geometry::ReadOBJ(const char *filename) {
    FILE* fp;
    char mode, vmode;
    char str[200];
    int v1, v2, v3, n1, n2, n3, t1, t2, t3;

    float x, y, z;
    float u, v;

    G308_Normal norm;
    G308_Triangle *tri;

    numVert = numNorm = numUV = numFace = 0;

    fp = fopen(filename, "r");
    if (fp == NULL)
        printf("Error reading %s file\n", filename);

    // Check number of vertex, normal, uvCoord, and Face
    while (fgets(str, 200, fp) != NULL) {
        sscanf(str, "%c%c", &mode, &vmode);
        switch (mode) {
        case 'v': /* vertex, uv, normal */
                 if (vmode == 't') numUV++;     // uv coordinate
            else if (vmode == 'n') numNorm++;   // normal
            else if (vmode == ' ') numVert++;   // vertex
            break;
        case 'f': /* faces */
            numFace++;
            break;
        }
    }

    m_nNumPoint = numVert;
    m_nNumUV = numUV;
    m_nNumPolygon = numFace;
    m_nNumNormal = numNorm;

    // dragon = (numNorm == 0);
    // if (dragon) {
    //     if (vertexNormals) m_nNumNormal = numVert;
    //     else               m_nNumNormal = numFace;
    // }

    printf("Reading %s: Point %d, UV %d, Normal %d, Face %d\n", filename, numVert, numUV, numNorm, numFace);
    //-------------------------------------------------------------
    //  Allocate memory for array
    //-------------------------------------------------------------

    if (m_pVertexArray  != NULL)    delete[] m_pVertexArray;
    if (m_pNormalArray  != NULL)    delete[] m_pNormalArray;
    if (m_pUVArray      != NULL)    delete[] m_pUVArray;
    if (m_pTriangles    != NULL)    delete[] m_pTriangles;

    if (m_nNumNormal == 0) {
        m_nNumNormal = ((vertexNormals==true)? numVert : numFace);
    }

    m_pVertexArray  = new G308_Point[m_nNumPoint];
    m_pNormalArray  = new G308_Normal[m_nNumNormal];
    m_pUVArray      = new G308_UVcoord[m_nNumUV];
    m_pTriangles    = new G308_Triangle[m_nNumPolygon];

    // initialize the normals
    for (int i=0; i<m_nNumNormal; ++i) {
        m_pNormalArray[i].x = 0.0;
        m_pNormalArray[i].y = 0.0;
        m_pNormalArray[i].z = 0.0;
    }

    //-----------------------------------------------------------
    //  Read obj file
    //-----------------------------------------------------------
    numVert = numNorm = numUV = numFace = 0;

    fseek(fp, 0L, SEEK_SET); // reset to the start of the file
    while (fgets(str, 200, fp) != NULL) {
        sscanf(str, "%c%c", &mode, &vmode);
        switch (mode) {
        case 'v': /* vertex, uv, normal */
            if (vmode == 't') { // uv coordinate
                sscanf(str, "vt %f %f", &u, &v);
                m_pUVArray[numUV].u = u;
                m_pUVArray[numUV].v = v;
                numUV++;
            } else if (vmode == 'n') { // normal
                sscanf(str, "vn %f %f %f", &x, &y, &z);
                m_pNormalArray[numNorm].x = x;
                m_pNormalArray[numNorm].y = y;
                m_pNormalArray[numNorm].z = z;
                numNorm++;
            } else if (vmode == ' ') { // vertex
                sscanf(str, "v %f %f %f", &x, &y, &z);
                m_pVertexArray[numVert].x = x;
                m_pVertexArray[numVert].y = y;
                m_pVertexArray[numVert].z = z;
                numVert++;
            }
            break;
        case 'f': /* faces : stored value is index - 1 since our index starts from 0, but obj starts from 1 */
            // DEBUG remove me
            // v1 = -1;
            // v2 = -1;
            // v3 = -1;
            // n1 = -1;
            // n2 = -1;
            // n3 = -1;
            // t1 = -1;
            // t2 = -1;
            // t3 = -1;

            // parse with fallbacks
            if (sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) != 9)
            if (sscanf(str, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3) != 6)
            if (sscanf(str, "f %d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3, &t3) != 6)
            if (sscanf(str, "f %d %d %d", &v1, &v2, &v3) != 3) {
                printf("error parsing this line: %s", str);
                exit(0);
            }

            tri = m_pTriangles + numFace; // shorthand

            // Vertex indicies for triangle:
            if (numVert != 0) {
                tri->v1 = v1 - 1;
                tri->v2 = v2 - 1;
                tri->v3 = v3 - 1;
            }

            // Normal indicies for triangle
            if (numNorm != 0) {
                tri->n1 = n1 - 1;
                tri->n2 = n2 - 1;
                tri->n3 = n3 - 1;
            }
            else {
                // vertexNormals = false;
                if (vertexNormals) {
                    tri->n1 = tri->v1;  // each vertex has a normal (mapped 1-1 by array indecies)
                    tri->n2 = tri->v2;
                    tri->n3 = tri->v3;
                    norm.x = norm.y = norm.z = 0;
                    calculateNormal(m_pTriangles+numFace, &norm);
                    // printf("normal %f %f %f \n", norm.x, norm.y, norm.z);
                    add_equals(m_pNormalArray + tri->n1, &norm);
                    add_equals(m_pNormalArray + tri->n2, &norm);
                    add_equals(m_pNormalArray + tri->n3, &norm);
                }
                else { // plane normals
                    calculateNormal(m_pTriangles+numFace, m_pNormalArray+numFace);
                    tri->n1 = numFace;
                    tri->n2 = numFace;
                    tri->n3 = numFace;
                }
            }

            // UV indicies for triangle
            if (numUV != 0) {
                tri->t1 = t1 - 1;
                tri->t2 = t2 - 1;
                tri->t3 = t3 - 1;
            }

            numFace++;
            break;
        default:
            break;
        }
    }

    if (vertexNormals) {
        // normalize as we added all the face normals together
        for (int i=0; i<numVert; ++i) {
            // printf("sum %f %f %f \n", m_pNormalArray[i].x, m_pNormalArray[i].y, m_pNormalArray[i].z);
            normalize(m_pNormalArray+i);
            // m_pNormalArray[i].x = 0.0;
            // m_pNormalArray[i].y = 0.0;
            // m_pNormalArray[i].z = 1.0;
        }
    }


    fclose(fp);
    // printf("Reading OBJ file is DONE.\n");

}


void vertex(G308_Point *v) {
    glVertex3f(v->x, v->y, v->z);
}

void normal(G308_Normal *n) {
    glNormal3f(n->x, n->y, n->z);
}

void line(int a, int b) {
    normal(normals[a]); vertex(verticies[a]);
    normal(normals[b]); vertex(verticies[b]);
}

//--------------------------------------------------------------
// [Assignment1]
// Fill the following function to create display list
// of the obj file to show it as polygon
//--------------------------------------------------------------
void G308_Geometry::CreateGLPolyGeometry() {
    if (m_glGeomListPoly != 0){
        glDeleteLists(m_glGeomListPoly, 1);
    }
    // Assign a display list; return 0 if err
    m_glGeomListPoly = glGenLists(1);
    glNewList(m_glGeomListPoly, GL_COMPILE);

    glBegin(GL_TRIANGLES);
    for (int faceIndex=0; faceIndex<numFace; ++faceIndex) {
        G308_Triangle *face = m_pTriangles + faceIndex;
        normal(m_pNormalArray + face->n1); vertex(m_pVertexArray + face->v1);
        normal(m_pNormalArray + face->n2); vertex(m_pVertexArray + face->v2);
        normal(m_pNormalArray + face->n3); vertex(m_pVertexArray + face->v3);
    }
    glEnd();

    glEndList();
}


//--------------------------------------------------------------
// [Assignment1]
// Fill the following function to create display list
// of the obj file to show it as wireframe
//--------------------------------------------------------------
void G308_Geometry::CreateGLWireGeometry() {
    if (m_glGeomListWire != 0){
        glDeleteLists(m_glGeomListWire, 1);
    }
    // Assign a display list; return 0 if err
    m_glGeomListWire = glGenLists(1);
    glNewList(m_glGeomListWire, GL_COMPILE);

    glBegin(GL_LINES);
    for (int faceIndex=0; faceIndex<numFace; ++faceIndex) {
        G308_Triangle *face = m_pTriangles + faceIndex;
        verticies[0] = m_pVertexArray + face->v1;
        verticies[1] = m_pVertexArray + face->v2;
        verticies[2] = m_pVertexArray + face->v3;
        normals[0] = m_pNormalArray + face->n1;
        normals[1] = m_pNormalArray + face->n2;
        normals[2] = m_pNormalArray + face->n3;

        if (mode==G308_SHADE_WIREFRAME || mode==G308_SHADE_WIREFRAME2) line(0, 1);
        if (mode==G308_SHADE_WIREFRAME || mode==G308_SHADE_WIREFRAME3) line(1, 2);
        if (mode==G308_SHADE_WIREFRAME || mode==G308_SHADE_WIREFRAME4) line(2, 0);
    }
    glEnd();

    glEndList();
}


void G308_Geometry::toggleMode() {
    setMode((mode+1) % (G308_SHADE_MAX_MODE+1));
}

void G308_Geometry::setMode(int m) {
    // error handling
    if (mode < G308_SHADE_POLYGON && mode > G308_SHADE_MAX_MODE) {
        printf("invalid set mode: %d\n", m);
        return;
    }

    mode = m;
    CreateGLWireGeometry();
}

void G308_Geometry::RenderGeometry() {
    glShadeModel(GL_SMOOTH);

    if (mode == G308_SHADE_POLYGON) {
        // glutSolidTeapot(5.0);
        glCallList(m_glGeomListPoly);
    }
    else if (mode >= G308_SHADE_WIREFRAME && mode <= G308_SHADE_WIREFRAME4) {
        // glutWireTeapot(5.0);
        glCallList(m_glGeomListWire);

    } else {
        printf("OMG! Wrong Shading Mode. \n");
    }

}


















