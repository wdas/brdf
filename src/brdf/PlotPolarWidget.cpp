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


#include <QMouseEvent>
#include <QString>
#include <math.h>
#include <iostream>
#include "DGLShader.h"
#include "PlotPolarWidget.h"
#include "Paths.h"


PlotPolarWidget::PlotPolarWidget(QWindow *parent, std::vector<brdfPackage> bList )
    : GLWindow(parent)
{
	brdfs = bList;

	NEAR_PLANE = 0.5f;
	FAR_PLANE = 50.0f;
	FOV_Y = 45.0f;

	inTheta = 0.785398163;
	inPhi = 0.785398163;

    resetViewingParams();

	useLogPlot = false;
	useNDotL = false;

    initializeGL();
}

PlotPolarWidget::~PlotPolarWidget()
{
    glcontext->makeCurrent(this);
    delete plotShader;
}

void PlotPolarWidget::resetViewingParams()
{
    // default viewing parameters
    lookZoom = 1.0;
    centerX = 0;
    centerY = 0.75;
}

QSize PlotPolarWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize PlotPolarWidget::sizeHint() const
{
    return QSize(512, 300);
}

void PlotPolarWidget::initializeGL()
{
    glcontext->makeCurrent(this);

    // this being a line graph, turn on line smoothing
    glf->glEnable( GL_LINE_SMOOTH );
    glf->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glf->glEnable(GL_BLEND);
    glf->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    plotShader = new DGLShader( (getShaderTemplatesPath() + "Plots.vert").c_str(),
                                (getShaderTemplatesPath() + "Plots.frag").c_str(),
                                (getShaderTemplatesPath() + "Plots.geom").c_str());
}


void PlotPolarWidget::DrawBRDFHemisphere( brdfPackage pkg )
{
    DGLShader* shader = NULL;

    // if there's a BRDF, the BRDF pbject sets up and enables the shader
    if( pkg.brdf )
    {
        shader = pkg.brdf->getUpdatedShader( SHADER_POLAR, &pkg );
        if( shader )
        {
            glm::mat4 id(1.f);
            shader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
            shader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
            shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
            shader->setUniformFloat( "incidentTheta", inTheta );
            shader->setUniformFloat( "incidentPhi", inPhi );
            shader->setUniformFloat( "useLogPlot", useLogPlot ? 1.0 : 0.0 );
            shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
            shader->setUniformFloat( "viewport_size", width()*devicePixelRatio(), height()*devicePixelRatio());
            shader->setUniformFloat( "thickness", 3.f);
        }
    }

    float inc = 3.14159265 / 360.;
    float angle = 0.0;

    std::vector<glm::vec3> vertices;
    vertices.reserve(360);

    for( int i = 0; i <= 360; i++ )
    {
        vertices.push_back(glm::vec3(cos(angle), sin(angle), angle));
        angle += inc;
    }

    GLuint vao;
    glf->glGenVertexArrays(1, &vao);
    glf->glBindVertexArray(vao);
    GLuint vbo;
    glf->glGenBuffers(1, &vbo);
    glf->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glEnableVertexAttribArray(vertex_loc);
        glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glf->glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, vertices.size());

    glf->glBindVertexArray(0);

    // if there was a shader, now we have to disable it
    if( pkg.brdf )
        pkg.brdf->disableShader( SHADER_POLAR );

    glf->glDeleteVertexArrays(1,&vao);
    glf->glDeleteBuffers(1,&vbo);
}


void PlotPolarWidget::paintGL()
{
    if(!isShowing() || !isExposed())
        return;

    glcontext->makeCurrent(this);

    CKGL();

    float vectorSize = 2.0;

    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);

    glf->glEnable(GL_MULTISAMPLE);
    glf->glClearColor( 1, 1, 1, 1 );
    glf->glDisable(GL_DEPTH_TEST);
    glf->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float fWidth = float(width()*devicePixelRatio());
    float fHeight = float(height()*devicePixelRatio());

    glf->glViewport(0, 0, fWidth, fHeight);

    float aspect = fWidth / fHeight;
    projectionMatrix = glm::ortho(centerX + -1.0*aspect*lookZoom, centerX + 1.0*aspect*lookZoom,
                                  centerY + -1.0*lookZoom, centerY + 1.0*lookZoom);

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;

    // normal vector
    const float c = 173.f/255.f;
    colors.push_back(glm::vec3(c, 1, c));
    colors.push_back(glm::vec3(c, 1, c));
    vertices.push_back(glm::vec3(0, 0, 0));
    vertices.push_back(glm::vec3(0, vectorSize, 0));

    // bottom of the hemisphere
    colors.push_back(glm::vec3(0, 0, 0));
    colors.push_back(glm::vec3(0, 0, 0));
    vertices.push_back(glm::vec3(-vectorSize, 0, 0));
    vertices.push_back(glm::vec3(vectorSize, 0, 0));

    colors.push_back(glm::vec3(c, 1, 1));
    colors.push_back(glm::vec3(c, 1, 1));
    vertices.push_back(glm::vec3(0, 0, 0));
    vertices.push_back(glm::vec3( vectorSize*cos(1.57079633 - inTheta) * -1.0, vectorSize*sin(1.57079633 - inTheta), 0 ));

    colors.push_back(glm::vec3(1, c, 1));
    colors.push_back(glm::vec3(1, c, 1));
    vertices.push_back(glm::vec3(0, 0, 0));
    vertices.push_back(glm::vec3( vectorSize*cos(1.57079633 - inTheta), vectorSize*sin(1.57079633 - inTheta), 0 ));

    plotShader->enable();
    glm::mat4 id(1.f);
    plotShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    plotShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
    plotShader->setUniformFloat("viewport_size", fWidth, fHeight);
    plotShader->setUniformFloat("thickness", 2.5f);

    GLuint vao;
    glf->glGenVertexArrays(1, &vao);
    glf->glBindVertexArray(vao);
    GLuint vbo[2];
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

    glf->glDrawArrays(GL_LINES, 0, vertices.size());


    // draw the hemisphere
    colors.clear();
    vertices.clear();

    const float c2 = 226.f/255.f;

    float inc = 3.14159265 / 180.;
    float angle = 0.0;

    for( int i = 0; i <= 180; i++ )
    {
        colors.push_back(glm::vec3(c2,c2,c));
        vertices.push_back(glm::vec3(cos(angle), sin(angle), 0));
        angle += inc;
    }

    glf->glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glf->glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * colors.size(), colors.data(), GL_STATIC_DRAW);

    glf->glDrawArrays(GL_LINE_STRIP, 0, vertices.size());

    glf->glBindVertexArray(0);

    plotShader->disable();

    glf->glDeleteVertexArrays(1,&vao);
    glf->glDeleteBuffers(2,vbo);

    // draw the hemispheres
    for( int i = 0; i < (int)brdfs.size(); i++ )
        DrawBRDFHemisphere( brdfs[i] );

    CKGL();

    glcontext->swapBuffers(this);
}

void PlotPolarWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void PlotPolarWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

	// left mouse button adjusts the viewing dir
	if (event->buttons() & Qt::LeftButton)
	{
		float xScalar = 1.0 / float(width());
		float yScalar = 1.0 / float(height());
        float maxScalar = std::max( xScalar, yScalar );

		centerX += float(-dx)*maxScalar*2.0*lookZoom;
		centerY += float( dy)*maxScalar*2.0*lookZoom;
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


void PlotPolarWidget::incidentDirectionChanged( float theta, float phi )
{
    // update the incident direction
    inTheta = theta;
    inPhi = phi;

    // repaint!
    updateGL();
}


void PlotPolarWidget::graphParametersChanged( bool logPlot, bool nDotL )
{
    // update the graph parameters
    useLogPlot = logPlot;
    useNDotL = nDotL;
    
    // repaint!
    updateGL();
}


void PlotPolarWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    // replace the BRDF pointers
    brdfs = brdfList;
    
    // repaint!
    updateGL();
}

void PlotPolarWidget:: mouseDoubleClickEvent ( QMouseEvent *  )
{
    resetViewingParams();
    updateGL();
}

void PlotPolarWidget::setShowing( bool s )
{
    _showing = s;
    if(s)
        updateGL();
}
