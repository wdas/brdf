/*
Copyright Disney Enterprises, Inc. All rights reserved.

This license governs use of the accompanying software. If you use the software, you
accept this license. If you do not accept the license, do not use the software.

1. Definitions
The terms "reproduce," "reproduction," "derivative works," and "distribution" have
the same meaning here as under U.S. copyright law. A "contribution" is the original
software, or any additions or changes to the software. A "contributor" is any person
that distributes its contribution under this license. "Licensed patents" are a
contributor's patent claims that read directly on its contribution.

2. Grant of Rights
(A) Copyright Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free copyright license to reproduce its contribution, prepare
derivative works of its contribution, and distribute its contribution or any derivative
works that you create.
(B) Patent Grant- Subject to the terms of this license, including the license
conditions and limitations in section 3, each contributor grants you a non-exclusive,
worldwide, royalty-free license under its licensed patents to make, have made,
use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the
software or derivative works of the contribution in the software.

3. Conditions and Limitations
(A) No Trademark License- This license does not grant you rights to use any
contributors' name, logo, or trademarks.
(B) If you bring a patent claim against any contributor over patents that you claim
are infringed by the software, your patent license from such contributor to the
software ends automatically.
(C) If you distribute any portion of the software, you must retain all copyright,
patent, trademark, and attribution notices that are present in the software.
(D) If you distribute any portion of the software in source code form, you may do
so only under this license by including a complete copy of this license with your
distribution. If you distribute any portion of the software in compiled or object code
form, you may only do so under a license that complies with this license.
(E) The software is licensed "as-is." You bear the risk of using it. The contributors
give no express warranties, guarantees or conditions. You may have additional
consumer rights under your local laws which this license cannot change.
To the extent permitted under your local laws, the contributors exclude the
implied warranties of merchantability, fitness for a particular purpose and non-
infringement.
*/

#include <GL/glew.h>
#include <QtGui>
#include <QString>
#include <math.h>
#include "DGLShader.h"
#include "Plot3DWidget.h"
#include "geodesicHemisphere.h"

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif //M_PI_2

Plot3DWidget::Plot3DWidget( QWidget *parent, std::vector<brdfPackage> bList )
    : SharedContextGLWidget(parent)
{
    brdfs = bList;
    
    NEAR_PLANE = 0.5f;
    FAR_PLANE = 100.0f;
    FOV_Y = 45.0f;
    
    resetViewingParams();
    
    inTheta = 0.785398163;
    inPhi = 0.785398163;
    
    useNDotL = false;
    useLogPlot = false;
}

void Plot3DWidget::resetViewingParams()
{
    lookPhi = 0;
    lookTheta = 0.785398163;
    lookZoom = 4.0;
}

Plot3DWidget::~Plot3DWidget()
{
    makeCurrent();
    
    // delete the VBO
    glDeleteBuffers(1, &hemisphereVerticesVBO);
}

QSize Plot3DWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize Plot3DWidget::sizeHint() const
{
    return QSize(768, 512);
}

void Plot3DWidget::initializeGL()
{
    glewInit();
    
    makeGeodesicHemisphereVBO();
    
    glClearColor( 0.2, 0.2, 0.2, 0.2 );
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
}


void Plot3DWidget::DrawBRDFHemisphere( brdfPackage pkg )
{   
    DGLShader* shader = NULL;
    
    // if there's a BRDF, the BRDF pbject sets up and enables the shader
    if( pkg.brdf )
    {
        shader = pkg.brdf->getUpdatedShader( SHADER_REFLECTOMETER, &pkg );
        shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
        shader->setUniformFloat( "incidentTheta", inTheta );
        shader->setUniformFloat( "incidentPhi", inPhi );
        shader->setUniformFloat( "useLogPlot", useLogPlot ? 1.0 : 0.0 );
        shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
    }
    
    // setup to draw the VBO
    glVertexPointer( 3, GL_FLOAT, 0, NULL );
    glBindBuffer(GL_ARRAY_BUFFER, hemisphereVerticesVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    // ... draw it...
    glDrawArrays(GL_TRIANGLES, 0, numTrianglesInHemisphere*3);
    
    // ... and now we're done!
    glDisableClientState(GL_VERTEX_ARRAY);
    
    // if there was a shader, now we have to disable it
    if( pkg.brdf )
        pkg.brdf->disableShader( SHADER_REFLECTOMETER );
}


void Plot3DWidget::paintGL()
{
    if( !isShowing() )
        return;    
    
    float planeSize = 3.0;
    float planeHeight = -0.01;
    float vectorLength = 5.0;
    
    
    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);
    
    
    glDisable( GL_LIGHTING );
    glDisable( GL_COLOR_MATERIAL );	
    glLineWidth( 2.0 );
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    // try and set some better viewing planes based on zoom factor... this only kinda works
    float nearPlane = std::min<float>( lookZoom*0.1, 0.5 );
    float farPlane =  std::min<float>( lookZoom*10.0, 100.0 );


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    gluPerspective(FOV_Y, float(width()) / float(height()), nearPlane, farPlane);
    
    
    float lookVec[3];
    lookVec[0] = sin(lookTheta) * cos(lookPhi);
    lookVec[1] = sin(lookTheta) * sin(lookPhi);
    lookVec[2] = cos(lookTheta);
    
    lookVec[0] *= lookZoom;
    lookVec[1] *= lookZoom;
    lookVec[2] *= lookZoom;
    
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( lookVec[0], lookVec[1], lookVec[2],
               0, 0, 0,
               0, 0, 1 );
    
    
    // draw the hemipshere
    for( int i = 0; i < (int)brdfs.size(); i++ )
        DrawBRDFHemisphere( brdfs[i] );
    
    
    
    // draw the incident vector
    glColor3f( 0, 1, 1 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( vectorLength*incidentVector[0], vectorLength*incidentVector[1], vectorLength*incidentVector[2] );
    glEnd();
    
    // draw the surface normal
    glColor3f( 0, 0, 1 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0, vectorLength );
    glEnd();
    
    // draw the reflection vector
    glColor3f( 1, 0, 1 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( -vectorLength*incidentVector[0], -vectorLength*incidentVector[1], vectorLength*incidentVector[2] );
    glEnd();
    
    // draw the U and V lines
    glColor3f( 1, 0, 0 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( planeSize, 0, 0 );
    glEnd();
    glColor3f( 0, 1, 0 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, planeSize, 0 );
    glEnd();
    
    
    
    // draw the unit circle
    float ucInc = 6.28318531 / 60.0f;
    float ucAngle = 0.0;
    glColor3f( 0.8, 0.8, 0 );
    glBegin( GL_LINE_LOOP );
    for( int i = 0; i < 60; i++ )
    {
        glVertex3f( cos(ucAngle), sin(ucAngle), 0.0 );
        ucAngle += ucInc;
    }
    glEnd();
    
    
    // draw the plane
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-planeSize, -planeSize, planeHeight);

    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(planeSize, -planeSize, planeHeight);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(planeSize, planeSize, planeHeight);

    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-planeSize, planeSize, planeHeight);
    glEnd();

    glPopAttrib();
}

void Plot3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void Plot3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    
    // left mouse button adjusts the viewing dir
    if (event->buttons() & Qt::LeftButton)
    {
        // adjust the vectors by which we look at the 3D BDRT	
        lookPhi += float(-dx) * 0.01;	

        lookTheta += float(-dy) * 0.01;
        if( lookTheta < 0.001 )
        lookTheta = 0.001;
        if( lookTheta > M_PI_2 )
        lookTheta = M_PI_2;
    }
    
    // right mouse button adjusts the zoom
    else if (event->buttons() & Qt::RightButton)
    {
        // use the dir with the biggest change
        int d = abs(dx) > abs(dy) ? dx : dy;
        
        lookZoom -= float(d) * lookZoom * 0.05;
        lookZoom = std::max<float>( lookZoom, 0.01f );
        lookZoom = std::min<float>( lookZoom, 100.0f );
    }
    
    
    lastPos = event->pos();
    
    // redraw
    updateGL();
}


// subdivide the triangle up to a certain depth
void Plot3DWidget::subdivideTriangle(float* vertices, int& index, float *v1, float *v2, float *v3, int depth)
{
    float v12[3];
    float v13[3];
    float v23[3];
    
    if( depth == 0 )
    {
        vertices[index++] = v1[0];
        vertices[index++] = v1[1];
        vertices[index++] = v1[2];
        
        vertices[index++] = v2[0];
        vertices[index++] = v2[1];
        vertices[index++] = v2[2];

        vertices[index++] = v3[0];
        vertices[index++] = v3[1];
        vertices[index++] = v3[2];

        return;
    }
    
    for( int i = 0; i < 3; i++ )
    {
        v12[i] = (v2[i] - v1[i])*0.5 + v1[i];
        v13[i] = (v3[i] - v1[i])*0.5 + v1[i];
        v23[i] = (v3[i] - v2[i])*0.5 + v2[i];
    }
    
    subdivideTriangle(vertices, index, v1, v12, v13, depth - 1);
    subdivideTriangle(vertices, index, v12, v2, v23, depth - 1);
    subdivideTriangle(vertices, index, v13, v23, v3, depth - 1);
    subdivideTriangle(vertices, index, v13, v12, v23, depth - 1);
}


void Plot3DWidget::makeGeodesicHemisphereVBO()
{
    float* hemisphereVertices;
    int numSubdivisions = 6;
    int memIndex = 0;
    
    // allocate enough memory for all the vertices in the hemisphere
    numTrianglesInHemisphere = 40 * int(powf(4.0, float(numSubdivisions)));
    hemisphereVertices = new float[ numTrianglesInHemisphere * 3 * 3 ];
    //printf( "numTrianglesInHemisphere: %d\n", numTrianglesInHemisphere );
    
    
    // Generate and bind the vertex buffer object
    glGenBuffers( 1, &hemisphereVerticesVBO );
    glBindBuffer( GL_ARRAY_BUFFER, hemisphereVerticesVBO );
    
    
    // recursively divide the hemisphere triangles to get a nicely tessellated hemisphere
    for( int i = 0; i < 40; i++ )
    {
        subdivideTriangle( hemisphereVertices, memIndex,
                        geodesicHemisphereVerts[i][0],
                        geodesicHemisphereVerts[i][1],
                        geodesicHemisphereVerts[i][2], numSubdivisions );
    }
    
    // copy the data into a buffer on the GPU
    glBufferData(GL_ARRAY_BUFFER, numTrianglesInHemisphere*sizeof(float)*9, hemisphereVertices, GL_STATIC_DRAW);
    
    // now that the hemisphere vertices are on the GPU, we're done with the local copy
    delete[] hemisphereVertices;
}



void Plot3DWidget::incidentDirectionChanged( float theta, float phi )
{
    // update the incident direction
    inTheta = theta;
    inPhi = phi;

    // repaint!
    updateGL();
}


void Plot3DWidget::graphParametersChanged( bool logPlot, bool nDotL )
{
    // update the graph parameters
    useLogPlot = logPlot;
    useNDotL = nDotL;
    
    // repaint!
    updateGL();
}


void Plot3DWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    // replace the BRDF pointers
    brdfs = brdfList;
    
    // repaint!
    updateGL();
}


void Plot3DWidget::mouseDoubleClickEvent ( QMouseEvent * )
{
    resetViewingParams();
    updateGL();
}
