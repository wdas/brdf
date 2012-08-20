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
#include "PlotPolarWidget.h"


PlotPolarWidget::PlotPolarWidget(QWidget *parent, std::vector<brdfPackage> bList )
    : SharedContextGLWidget(parent)
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
}

PlotPolarWidget::~PlotPolarWidget()
{
    makeCurrent();
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
	glewInit();

    glClearColor( 1, 1, 1, 1 );	
    glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);

	// this being a line graph, turn on line smoothing
	glEnable( GL_LINE_SMOOTH );
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
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
			shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
			shader->setUniformFloat( "incidentTheta", inTheta );
			shader->setUniformFloat( "incidentPhi", inPhi );
			shader->setUniformFloat( "useLogPlot", useLogPlot ? 1.0 : 0.0 );
			shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
		}
	}

	glLineWidth( 1 );
	glBegin( GL_LINE_LOOP );
	float inc = 3.14159265 / 360.;
	float angle = 0.0;

	for( int i = 0; i <= 360; i++ )
	{		
		glVertex3f( cos(angle), sin(angle), angle );
		angle += inc;
	}
	glEnd();

	// if there was a shader, now we have to disable it
    if( pkg.brdf )
        pkg.brdf->disableShader( SHADER_POLAR );
}


void PlotPolarWidget::paintGL()
{
    if( !isShowing() )
        return;
    
    float vectorSize = 2.0;
    glLineWidth( 1.5 );
    
    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);
    
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    
    float aspect = float(width()) / float(height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(centerX + -1.0*aspect*lookZoom, centerX + 1.0*aspect*lookZoom,
            centerY + -1.0*lookZoom, centerY + 1.0*lookZoom);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    
    // normal vector
    glColor3ub( 173, 255, 173 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, vectorSize, 0 );
    glEnd();
    
    // bottom of the hemisphere
    glColor3f( 0, 0, 0 );
    glBegin( GL_LINES );
    glVertex3f( -vectorSize, 0, 0 );
    glVertex3f( vectorSize, 0, 0 );
    glEnd();
    
    
    glColor3ub( 173, 255, 255 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( vectorSize*cos(1.57079633 - inTheta) * -1.0, vectorSize*sin(1.57079633 - inTheta), 0 );
    glEnd();
    
    glColor3ub( 255, 173, 255 );
    glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( vectorSize*cos(1.57079633 - inTheta), vectorSize*sin(1.57079633 - inTheta), 0 );
    glEnd();
    
    
    // draw the hemisphere
    glColor3ub( 226, 226, 173 );
    glBegin( GL_LINE_STRIP );
    float inc = 3.14159265 / 180.;
    float angle = 0.0;
    
    for( int i = 0; i <= 180; i++ )
    {
        glVertex3f( cos(angle), sin(angle), 0 );
        angle += inc;
    }
    glEnd();
    
    // draw the hemispheres
    for( int i = 0; i < (int)brdfs.size(); i++ )
        DrawBRDFHemisphere( brdfs[i] );
    
    glPopAttrib();
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
