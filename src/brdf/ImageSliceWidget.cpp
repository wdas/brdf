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

#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#else
#include <cmath>
#endif


#include <QMouseEvent>
#include <QString>
#include <iostream>
#include "DGLShader.h"
#include "ImageSliceWidget.h"
#include "DGLFrameBuffer.h"
#include "Paths.h"


ImageSliceWidget::ImageSliceWidget(QWindow *parent, std::vector<brdfPackage> bList )
    : GLWindow(parent)
{
    brdfs = bList;

    NEAR_PLANE = 0.5f;
    FAR_PLANE = 50.0f;
    FOV_Y = 45.0f;

    // default viewing parameters
    lookZoom = 1.0;
    centerX = 0.4;
    centerY = 0.5;

    scaleX = 0.8;
    scaleY = 1.0;

    inTheta = 0.785398163;
    inPhi = 0.785398163;

    phiD = 90;

    brightness = 1.0;
    gamma = 2.2;
    exposure = 0.0;

    useThetaHSquared = false;
    showChroma = false;

    initializeGL();

    quad = new Quad(-1,-1,1,1,0,0,1.57079633,1.57079633);
}

ImageSliceWidget::~ImageSliceWidget()
{
    glcontext->makeCurrent(this);
    glf->glDeleteVertexArrays(1, &vao);
    glf->glDeleteBuffers(2, vbo);
    delete quad;
    delete plotShader;
}

QSize ImageSliceWidget::minimumSizeHint() const
{
    return QSize(256, 256);
}

QSize ImageSliceWidget::sizeHint() const
{
    return QSize(256, 256);
}

void ImageSliceWidget::initializeGL()
{
    glcontext->makeCurrent(this);
    glf->glDisable(GL_DEPTH_TEST);

    plotShader = new DGLShader( (getShaderTemplatesPath() + "Plots.vert").c_str(),
                                (getShaderTemplatesPath() + "Plots.frag").c_str(),
                                (getShaderTemplatesPath() + "Plots.geom").c_str());

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;
    colors.push_back(glm::vec3(0, 0, 0));
    colors.push_back(glm::vec3(0, 0, 0));
    colors.push_back(glm::vec3(0, 0, 0));
    colors.push_back(glm::vec3(0, 0, 0));
    vertices.push_back(glm::vec3(-1, -1, 0));
    vertices.push_back(glm::vec3(1, -1, 0));
    vertices.push_back(glm::vec3(1, 1, 0));
    vertices.push_back(glm::vec3(-1, 1, 0));
    plotShader->enable();
    glf->glGenVertexArrays(1, &vao);
    glf->glBindVertexArray(vao);
    glf->glGenBuffers(2, vbo);
    glf->glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    int vertex_loc = plotShader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glEnableVertexAttribArray(vertex_loc);
        glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    glf->glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.data(), GL_STATIC_DRAW);
    int color_loc = plotShader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glf->glEnableVertexAttribArray(color_loc);
        glf->glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    plotShader->disable();
}


void ImageSliceWidget::drawImageSlice()
{
    DGLShader* shader = NULL;

    // if there's a BRDF, the BRDF pbject sets up and enables the shader
    if( brdfs.size() )
    {
        shader = brdfs[0].brdf->getUpdatedShader( SHADER_IMAGE_SLICE, &brdfs[0] );

        if( shader )
        {
            glm::mat4 id(1.f);
            shader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
            shader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
            shader->setUniformFloat( "incidentPhi", inPhi );
            shader->setUniformFloat( "phiD", phiD*(M_PI/180) );
            shader->setUniformFloat( "brightness", brightness );
            shader->setUniformFloat( "gamma", gamma );
            shader->setUniformFloat( "exposure", exposure );
            shader->setUniformFloat( "showChroma", showChroma );
            shader->setUniformFloat( "useThetaHSquared", useThetaHSquared );
        }
    }

    // the actual image slice quad
    quad->draw(shader);

    // if there was a shader, now we have to disable it
    if( brdfs[0].brdf )
        brdfs[0].brdf->disableShader( SHADER_IMAGE_SLICE );

    // the black border
    plotShader->enable();
    glm::mat4 id(1.f);
    plotShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    plotShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
    float fWidth = float(width()*devicePixelRatio());
    float fHeight = float(height()*devicePixelRatio());
    plotShader->setUniformFloat("viewport_size", fWidth, fHeight);
    plotShader->setUniformFloat("thickness", 1.f);
    glf->glBindVertexArray(vao);
    glf->glDrawArrays(GL_LINE_LOOP, 0, 4);
    plotShader->disable();
}


void ImageSliceWidget::paintGL()
{
    if( !isShowing() || !isExposed())
        return;

    glcontext->makeCurrent(this);

    glf->glClearColor( 1, 1, 1, 1 );
    glf->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glf->glDisable( GL_DEPTH_TEST );

    float sphereMargin = 1.1;
    float fWidth = float(width()*devicePixelRatio());
    float fHeight = float(height()*devicePixelRatio());

    glf->glViewport(0, 0, fWidth, fHeight);

    if( width() > height() )
    {
        float aspect = fWidth / fHeight;
        projectionMatrix = glm::ortho( -aspect * sphereMargin, aspect * sphereMargin, -sphereMargin, sphereMargin );
    }
    else
    {
        float invAspect = fHeight / fWidth;
        projectionMatrix = glm::ortho( -sphereMargin, sphereMargin, -invAspect * sphereMargin, invAspect * sphereMargin );
    }

    // if there are BRDFs, draw the image slice
    if( brdfs.size() )
    {
        drawImageSlice();
    }

    glcontext->swapBuffers(this);
}

void ImageSliceWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void ImageSliceWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    // control+left adjusts the x-scale of the graph
    if( event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier )
    {
        int d = abs(dx) > abs(dy) ? dx : dy;
        scaleX += float(d) * scaleX * 0.05;
        scaleX = std::max<float>( scaleX, 0.01 );
        scaleX = std::min<float>( scaleX, 50.0 );
    }

    // control+right adjusts the x-scale of the graph
    else if( event->buttons() & Qt::RightButton && event->modifiers() & Qt::ControlModifier )
    {
        int d = abs(dx) > abs(dy) ? dx : dy;
        scaleY += float(d) * scaleY * 0.05;
        scaleY = std::max<float>( scaleY, 0.01 );
        scaleY = std::min<float>( scaleY, 50.0 );
    }

    // left mouse button adjusts the viewing dir
    else if (event->buttons() & Qt::LeftButton)
    {
        float xScalar = 1.0 / float(width()*devicePixelRatio());
        float yScalar = 1.0 / float(height()*devicePixelRatio());

        centerX += float(-dx)*xScalar*2.0*lookZoom;
        centerY += float( dy)*yScalar*2.0*lookZoom;
    }

    // right mouse button adjusts the zoom
    else if (event->buttons() & Qt::RightButton)
    {
        // use the dir with the biggest change
        int d = abs(dx) > abs(dy) ? dx : dy;

        lookZoom -= float(d) * lookZoom * 0.05;
        lookZoom = std::max<float>( lookZoom, 0.01f );
        lookZoom = std::min<float>( lookZoom, 50.0f );
    }


    lastPos = event->pos();

    // redraw
    updateGL();
}


void ImageSliceWidget::brightnessChanged( float v )
{
    brightness = v;
    updateGL();
}


void ImageSliceWidget::gammaChanged( float v )
{
    gamma = v;
    updateGL();
}


void ImageSliceWidget::exposureChanged( float v )
{
    exposure = v;
    updateGL();
}

void ImageSliceWidget::phiDChanged( float v )
{
    phiD = v;
    updateGL();
}

void ImageSliceWidget::useThetaHSquaredChanged( int v )
{
    useThetaHSquared = v;
    updateGL();
}

void ImageSliceWidget::showChromaChanged( int v )
{
    showChroma = v;
    updateGL();
}


void ImageSliceWidget::incidentDirectionChanged( float theta, float phi )
{
    inTheta = theta;
    inPhi = phi;

    // repaint!
    updateGL();
}


void ImageSliceWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    brdfs = brdfList;
    updateGL();
}
