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


#include <QtGui>
#include <QString>
#include <math.h>
#include <iostream>
#include "DGLShader.h"
#include "PlotCartesianWidget.h"
#include "Paths.h"

PlotCartesianWidget::PlotCartesianWidget(QWindow *parent, std::vector<brdfPackage> bList, int type )
    : GLWindow(parent)
{
    sliceType = type;
    brdfs = bList;

    resetViewingParams();

    inTheta = 0.785398163;
    inPhi = 0.785398163;

    useLogPlot = false;
    useNDotL = false;
    useSampleMult = false;

    initializeGL();
}


void PlotCartesianWidget::resetViewingParams()
{
  // default viewing parameters
  lookZoom = 1.0;
  centerX = 0.4;
  centerY = 0.5;

  scaleX = 0.8;
  scaleY = 1.0;

  // default viewing parameters
  zoomLevel[0] = 0.8f;
  zoomLevel[1] = 0.5f;
  zoomLevel[2] = 0.3f;

  updateProjectionMatrix();
  updateViewMatrix();
}

PlotCartesianWidget::~PlotCartesianWidget()
{
    glcontext->makeCurrent(this);
    glf->glDeleteVertexArrays(1,&axisVAO);
    glf->glDeleteBuffers(2,axisVBO);

    glf->glDeleteTextures(1,&textTexureID);
    glf->glDeleteVertexArrays(1,&textVAO);
    glf->glDeleteBuffers(1,&textVBO);

    glf->glDeleteVertexArrays(1, &dataLineVAO);
    glf->glDeleteBuffers(1, &dataLineVBO);

    delete plotShader;
    delete textShader;
}

QSize PlotCartesianWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize PlotCartesianWidget::sizeHint() const
{
    return QSize(256, 256);
}

void PlotCartesianWidget::initializeGL()
{
    glcontext->makeCurrent(this);

    glf->glClearColor( 1, 1, 1, 1 );
    glf->glDisable(GL_DEPTH_TEST);

    // this being a line graph, turn on line smoothing
    glf->glEnable( GL_LINE_SMOOTH );
    glf->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glf->glEnable(GL_BLEND);
    glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initializeText();

    plotShader = new DGLShader( (getShaderTemplatesPath() + "Plots.vert").c_str(),
                                (getShaderTemplatesPath() + "Plots.frag").c_str(),
                                (getShaderTemplatesPath() + "Plots.geom").c_str());
    glf->glGenVertexArrays(1,&axisVAO);
    glf->glGenBuffers(2, axisVBO);
    updateAxisVAO();

    glf->glGenVertexArrays(1, &dataLineVAO);
    glf->glGenBuffers(1, &dataLineVBO);
    fWidth = float(width()*devicePixelRatio());
    fHeight = float(height()*devicePixelRatio());
    updateInputDataLineVAO();
}

void PlotCartesianWidget::resamplePushed()
{
    useSampleMult = true;
    updateGL();
    useSampleMult = false;
}

void PlotCartesianWidget::initializeText()
{
    // Create font texture
    glf->glGenTextures( 1, &textTexureID );
    glf->glBindTexture( GL_TEXTURE_2D, textTexureID );

    glf->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glf->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glf->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glf->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    QImage img;
    if(!img.load(QString((getImagesPath() + "verasansmono.png").c_str())))
        std::cout << "Can't load font texture" << std::endl;

    glf->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.convertToFormat(QImage::Format_RGB888).bits());
    glf->glBindTexture( GL_TEXTURE_2D, 0 );

    textShader = new DGLShader( (getShaderTemplatesPath() + "Text.vert").c_str(), (getShaderTemplatesPath() + "Text.frag").c_str(), (getShaderTemplatesPath() + "Text.geom").c_str() );

    glf->glGenVertexArrays(1,&textVAO);
    glf->glGenBuffers(1,&textVBO);
}

void PlotCartesianWidget::updateAxisVAO()
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;

    float minX = -M_PI_2;
    float maxX = M_PI_2;

    float zoomY = lookZoom * 1.0f/scaleY;
    // y ticks
    for( int i = 1; i <= 10; i++ )
    {
        colors.push_back(glm::vec3(0.6, 0.6, 0.6));
        colors.push_back(glm::vec3(0.6, 0.6, 0.6));
        float yPos = i;
        vertices.push_back(glm::vec3(minX, yPos, 0));
        vertices.push_back(glm::vec3(maxX, yPos, 0));
    }
    for( int i = 20; i <= 100; i += 10 )
    {
        colors.push_back(glm::vec3(0.6, 0.6, 0.6));
        colors.push_back(glm::vec3(0.6, 0.6, 0.6));
        float yPos = i;
        vertices.push_back(glm::vec3(minX, yPos, 0));
        vertices.push_back(glm::vec3(maxX, yPos, 0));
    }
    if(zoomY < zoomLevel[0]) {
        for( float i = 0.2f; i<1; i+=0.2f)
        {
            colors.push_back(glm::vec3(0.8, 0.8, 0.8));
            colors.push_back(glm::vec3(0.8, 0.8, 0.8));
            float yPos = i;
            vertices.push_back(glm::vec3(minX, yPos, 0));
            vertices.push_back(glm::vec3(maxX, yPos, 0));
        }
        if(zoomY < zoomLevel[1]) {
            for( float i = 0.1f; i< 1; i+=0.2f)
            {
                colors.push_back(glm::vec3(0.9, 0.9, 0.9));
                colors.push_back(glm::vec3(0.9, 0.9, 0.9));
                float yPos = i;
                vertices.push_back(glm::vec3(minX, yPos, 0));
                vertices.push_back(glm::vec3(maxX, yPos, 0));
            }
            if(zoomY < zoomLevel[2]) {
                for( float i = 0.05f; i<1; i+=0.1f)
                {
                    colors.push_back(glm::vec3(0.95, 0.95, 0.95));
                    colors.push_back(glm::vec3(0.95, 0.95, 0.95));
                    float yPos = i;
                    vertices.push_back(glm::vec3(minX, yPos, 0));
                    vertices.push_back(glm::vec3(maxX, yPos, 0));
                }
            }
        }
    }


    // x ticks & lines
    for( int i = -6; i <= 6; i++ )
    {
        colors.push_back(glm::vec3(0,0,0));
        colors.push_back(glm::vec3(0,0,0));
        float xPos = float(i) * 0.261799388;
        vertices.push_back(glm::vec3(xPos, 0, 0));
        vertices.push_back(glm::vec3(xPos, -0.02, 0));
        if(zoomY < zoomLevel[0]) {
            colors.push_back(glm::vec3(.9, .9, .9));
            colors.push_back(glm::vec3(.9, .9, .9));
            vertices.push_back(glm::vec3(xPos, 0, 0));
            vertices.push_back(glm::vec3(xPos, 1, 0));
        }
    }

    // x axis
    colors.push_back(glm::vec3(0.4,0.4,0.4));
    colors.push_back(glm::vec3(0.4,0.4,0.4));
    vertices.push_back(glm::vec3( minX, 0, 0 ));
    vertices.push_back(glm::vec3(  maxX, 0, 0 ));

    // y axis
    colors.push_back(glm::vec3(0.4,0.4,0.4));
    colors.push_back(glm::vec3(0.4,0.4,0.4));
    vertices.push_back(glm::vec3( 0, 0, 0 ));
    vertices.push_back(glm::vec3( 0, 100.0, 0 ));



    glf->glBindVertexArray(axisVAO);

    glf->glBindBuffer(GL_ARRAY_BUFFER, axisVBO[0]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    int vertex_loc = plotShader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glEnableVertexAttribArray(vertex_loc);
        glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glf->glBindBuffer(GL_ARRAY_BUFFER, axisVBO[1]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.data(), GL_STATIC_DRAW);

    int color_loc = plotShader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glf->glEnableVertexAttribArray(color_loc);
        glf->glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glf->glBindVertexArray(0);
    glf->glBindBuffer(GL_ARRAY_BUFFER, 0);

    axisNIndices = vertices.size();
}

void PlotCartesianWidget::updateInputDataLineVAO()
{
    // the line!
    int nlinesegments = fWidth;

    double thetastart = (centerX + -1.0*mAspect*lookZoom)/scaleX;
    double thetaend = (centerX + 1.0*mAspect*lookZoom)/scaleX;
    if (thetastart < -0.5*M_PI || sliceType == THETA_H_PLOT) thetastart = -0.5*M_PI;
    if (thetaend > 0.5*M_PI  || sliceType == THETA_H_PLOT) thetaend = 0.5*M_PI;
    const double inc = (thetaend-thetastart) / (double)nlinesegments;

    std::vector<glm::vec3> vertices;
    vertices.reserve(nlinesegments);

    vertices.push_back(glm::vec3(thetastart, 0.0, thetastart ));
    for( int i = 1; i < nlinesegments - 1 ; i++ )
    {
        double theta = thetastart + (i+0.5) * inc;
        if (sliceType == THETA_H_PLOT) {
            // square thetaH to concentrate samples around thetaH==0
            theta *= theta * (1.0/(0.5*M_PI)) * (theta > 0 ? -1 : 1);
        } else if (sliceType == ALBEDO_PLOT) {
            // limit number of directions albedo is calculated on
            if (theta>=-1.0*inc && (i%4 == 0 || i==nlinesegments-1) )
                vertices.push_back(glm::vec3(theta, 0.0, theta ));
        }
        vertices.push_back(glm::vec3(theta, 0.0, theta ));
    }
    vertices.push_back(glm::vec3(thetaend, 0.0, thetaend));

    if(!vertices.empty())
        vertices.push_back(vertices.front());

    dataLineNPoints = vertices.size()-3;

    glf->glBindVertexArray(dataLineVAO);
    glf->glBindBuffer(GL_ARRAY_BUFFER, dataLineVBO);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    const int vertex_loc = 0;

    glf->glEnableVertexAttribArray(vertex_loc);
    glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glf->glBindVertexArray(0);
    glf->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlotCartesianWidget::updateProjectionMatrix()
{
    projectionMatrix = glm::ortho(centerX + -1.f*mAspect*lookZoom,
                                  centerX + 1.f*mAspect*lookZoom,
                                  centerY + -1.f*lookZoom,
                                  centerY + 1.f*lookZoom);
}

void PlotCartesianWidget::updateViewMatrix()
{
    modelViewMatrix = glm::scale(glm::mat4(1.0) , glm::vec3(scaleX, scaleY, 1.0));
}

void PlotCartesianWidget::paintGL()
{
    if( !isShowing() || !isExposed())
        return;

    glcontext->makeCurrent(this);

    glf->glClearColor(1,1,1,1);
    glf->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glf->glDisable(GL_DEPTH_TEST);

    float currentFWidth = float(width()*devicePixelRatio());
    float currentFHeight = float(height()*devicePixelRatio());
    if(currentFWidth != fWidth || currentFHeight != fHeight)
    {
        fWidth = currentFWidth;
        fHeight = currentFHeight;
        mAspect = fWidth / fHeight;
        updateInputDataLineVAO();
        updateProjectionMatrix();
    }

    glf->glViewport(0, 0, fWidth, fHeight);
    glf->glEnable(GL_MULTISAMPLE);

    drawAxis();
    drawLabels();

    glf->glBindVertexArray(dataLineVAO);
    for( int i = 0; i < (int)brdfs.size(); i++ )
    {
        DGLShader* shader = updateShader( brdfs[i] );
        glf->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0,  dataLineNPoints);
        shader->disable();
    }
    glf->glBindVertexArray(0);

    glf->glDisable(GL_BLEND);
    glf->glEnable(GL_DEPTH_TEST);

    glcontext->swapBuffers(this);
}

void PlotCartesianWidget::drawAxis()
{
    plotShader->enable();
    plotShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    plotShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(modelViewMatrix));
    plotShader->setUniformFloat("viewport_size", fWidth, fHeight);
    plotShader->setUniformFloat("thickness", 2.5f);
    glf->glBindVertexArray(axisVAO);
    glf->glDrawArrays(GL_LINES, 0, axisNIndices);
    glf->glBindVertexArray(0);
    plotShader->disable();
}

void PlotCartesianWidget::drawLabels()
{
    glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glf->glEnable(GL_BLEND);
    glf->glBindVertexArray(textVAO);
    glm::vec3 c1(0.6,0.6,0.6);
    glm::vec3 c2(0.3,0.3,0.3);

    renderText( 0.0, -0.52 * 33.33 / height(), "0", c1 );
    renderText( 0.785398163-0.02*lookZoom, -0.52 * 33.33 / height(), "45", c1 );
    renderText( 1.57079633-0.02*lookZoom, -0.52 * 33.33 / height(),  "90", c1 );
    renderText( -0.785398163-0.02*lookZoom, -0.52 * 33.33 / height(), "45", c1 );
    renderText( -1.57079633-0.02*lookZoom, -0.52 * 33.33 / height(),  "90", c1 );

    float zoomY = lookZoom * 1.0f/scaleY;

    if (zoomY < zoomLevel[0]) {
        for( int i = 1; i < 5; i++)
            renderText( 0.05-0.005*lookZoom, float(i)*0.2-0.02*lookZoom, QString::number(i*0.2), c1 );
        if (zoomY < zoomLevel[1]) {
            for( int i = 0; i < 5; i++)
                renderText( 0.05-0.005*lookZoom, 0.1+float(i)*0.2-0.02*lookZoom, QString::number(0.1+i*0.2), c1 );
            if (zoomY < zoomLevel[2]) {
                for( int i = 0; i < 10; i++)
                    renderText( 0.05-0.005*lookZoom, 0.05+float(i)*0.1-0.02*lookZoom, QString::number(0.05+i*0.1), c1 );
            }
        }
    }

    for( int i = 1; i <= 9; i++ )
        renderText( 0.05-0.005*lookZoom, float(i)-0.02*lookZoom, QString::number(i), c2 );
    for( int i = 10; i <= 100; i += 10 )
        renderText( -0.05-0.005*lookZoom, float(i), QString::number(i), c2 );
    glf->glBindVertexArray(0);
    glf->glDisable(GL_BLEND);
    glf->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlotCartesianWidget::renderText(float pos_x, float pos_y, const QString& text, const glm::vec3& color)
{
    textShader->enable();
    textShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    textShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(modelViewMatrix));
    textShader->setUniformFloat("TextColor", color.x, color.y, color.z );
    textShader->setUniformFloat("CellSize", 1.0f / 16.f, (300.0f / 384.f) / 6.f);
    textShader->setUniformFloat("CellOffset", 0.5f / 256.f, 0.5f / 256.f);
    textShader->setUniformFloat("RenderSize", 0.5 * 16 / fWidth, 0.5 * 33.33 / fHeight);
    textShader->setUniformFloat("RenderOrigin", pos_x, pos_y);
    textShader->setUniformTexture("Sampler",textTexureID);

    glf->glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(char) * text.size(), text.toStdString().c_str(), GL_STATIC_DRAW);

    int character_loc = textShader->getAttribLocation("Character");
    glf->glVertexAttribIPointer(character_loc, 1, GL_UNSIGNED_BYTE, 1, 0);
    glf->glEnableVertexAttribArray(character_loc);

    glf->glDrawArrays(GL_POINTS, 0, text.size());

    textShader->disable();
    glf->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

DGLShader* PlotCartesianWidget::updateShader( brdfPackage pkg )
{
    if( !pkg.brdf )
        return NULL;
    DGLShader* shader = NULL;
    int shaderType = 0;

    if( sliceType == THETA_V_PLOT )
    {
        shaderType = SHADER_CARTESIAN;
        shader = pkg.brdf->getUpdatedShader( shaderType, &pkg );
        if( shader ) shader->setUniformFloat( "phiV", angleParam );
    }
    else if( sliceType == THETA_H_PLOT )
    {
        shaderType = SHADER_CARTESIAN_THETA_H;
        shader = pkg.brdf->getUpdatedShader( shaderType, &pkg );
        if( shader ) shader->setUniformFloat( "thetaD", angleParam );
    }
    else if( sliceType == THETA_D_PLOT )
    {
        shaderType = SHADER_CARTESIAN_THETA_D;
        shader = pkg.brdf->getUpdatedShader( shaderType, &pkg );
        if( shader ) shader->setUniformFloat( "thetaH", angleParam );
    }
    else if( sliceType == ALBEDO_PLOT )
    {
        shaderType = SHADER_CARTESIAN_ALBEDO;
        shader = pkg.brdf->getUpdatedShader( shaderType, &pkg );
        if( shader ) {
            shader->setUniformInt( "nSamples", nSamples );
            shader->setUniformFloat( "sampleMultOn", useSampleMult );
            shader->setUniformInt( "samplingMode", samplingMode );
        }
    }

    if( shader )
    {
        shader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
        shader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(modelViewMatrix));
        shader->setUniformFloat( "useLogPlot", useLogPlot ? 1.0 : 0.0 );
        shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
        shader->setUniformFloat( "viewport_size", width()*devicePixelRatio(), height()*devicePixelRatio());
        shader->setUniformFloat( "thickness", 3.f);
        shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
        shader->setUniformFloat( "incidentTheta", inTheta );
        shader->setUniformFloat( "incidentPhi", inPhi );
    }
    return shader;
}

void PlotCartesianWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void PlotCartesianWidget::mouseMoveEvent(QMouseEvent *event)
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
        updateInputDataLineVAO();
        updateViewMatrix();
    }

    // control+right adjusts the x-scale of the graph
    else if( event->buttons() & Qt::RightButton && event->modifiers() & Qt::ControlModifier )
    {
        int d = abs(dx) > abs(dy) ? dx : dy;
        scaleY += float(d) * scaleY * 0.05;
        scaleY = std::max<float>( scaleY, 0.01 );
        scaleY = std::min<float>( scaleY, 50.0 );
        updateAxisVAO();
        updateViewMatrix();
    }

    // left mouse button adjusts the viewing dir
    else if (event->buttons() & Qt::LeftButton)
    {
        float xScalar = 1.0 / float(width()*devicePixelRatio());
        float yScalar = 1.0 / float(height()*devicePixelRatio());
        float maxScalar = std::max( xScalar, yScalar );

        centerX += float(-dx)*maxScalar*2.0*lookZoom;
        centerY += float( dy)*maxScalar*2.0*lookZoom;
        updateProjectionMatrix();
        updateInputDataLineVAO();
    }

    // right mouse button adjusts the zoom
    else if (event->buttons() & Qt::RightButton)
    {
        // use the dir with the biggest change
        int d = abs(dx) > abs(dy) ? dx : dy;

        lookZoom -= float(d) * lookZoom * 0.05;
        lookZoom = std::max<float>( lookZoom, 0.01f );
        lookZoom = std::min<float>( lookZoom, 50.0f );

        updateAxisVAO();
        updateInputDataLineVAO();
        updateProjectionMatrix();
    }

    lastPos = event->pos();

    // redraw
    updateGL();
}

void PlotCartesianWidget::incidentDirectionChanged( float theta, float phi )
{
    // update the incident direction
    inTheta = theta;
    inPhi = phi;
    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);

    // repaint!
    updateGL();
}

void PlotCartesianWidget::graphParametersChanged( bool logPlot, bool nDotL )
{
    // update the graph parameters
    useLogPlot = logPlot;
    useNDotL = nDotL;

    // repaint!
    updateGL();
}

void PlotCartesianWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    // replace the BRDF pointers
    brdfs = brdfList;

    // repaint!
    updateGL();
}

void PlotCartesianWidget::setAngles( float theta, float phi, float angParam )
{
    angleParam = angParam;
    incidentDirectionChanged(theta, phi);
}

void PlotCartesianWidget::samplingModeChanged(int newmode)
{
  samplingMode = newmode;
  updateGL();
}

void PlotCartesianWidget::mouseDoubleClickEvent ( QMouseEvent *  )
{
    resetViewingParams();
    updateInputDataLineVAO();
    updateGL();
}
