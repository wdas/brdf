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
#include "LitSphereWidget.h"


#define SQR(x) ((x)*(x))

void normalize( float* q )
{
	float l = sqrt( q[0]*q[0] + q[1]*q[1] + q[2]*q[2] );
	q[0] /= l;
	q[1] /= l;
	q[2] /= l;
}

LitSphereWidget::LitSphereWidget(QWidget *parent, std::vector<brdfPackage> brdfList )
    : SharedContextGLWidget(parent)
{
    brdfs = brdfList;
    
    NEAR_PLANE = 0.5f;
    FAR_PLANE = 50.0f;
    FOV_Y = 45.0f;
    
    brightness = 1.0;
    gamma = 2.2;
    exposure = 0.0;
    
    inTheta = 0.785398163;
    inPhi = 0.785398163;
    
    sphereMargin = 1.1;

    doubleTheta = true;
    useNDotL = true;
}

LitSphereWidget::~LitSphereWidget()
{
    makeCurrent();
}

QSize LitSphereWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize LitSphereWidget::sizeHint() const
{
    return QSize(256, 256);
}

void LitSphereWidget::initializeGL()
{
	glewInit();

    glClearColor( 0.25, 0.25, 0.25, 1 );	
}


void LitSphereWidget::drawSphere( double, int lats, int longs )
{
	DGLShader* shader = NULL;

	// if there's a BRDF, the BRDF pbject sets up and enables the shader
	if( brdfs.size() )
	{
		shader = brdfs[0].brdf->getUpdatedShader( SHADER_LITSPHERE, &brdfs[0] );
		shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
		shader->setUniformFloat( "incidentTheta", inTheta );
		shader->setUniformFloat( "incidentPhi", inPhi );

		shader->setUniformFloat( "brightness", brightness );
		shader->setUniformFloat( "gamma", gamma );
		shader->setUniformFloat( "exposure", exposure );
        shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
	}

	
	int i, j;
	for(i = 0; i <= lats; i++)
	{
		double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
		double z0  = sin(lat0);
		double zr0 =  cos(lat0);
		
		double lat1 = M_PI * (-0.5 + (double) i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);
		
		glBegin(GL_QUAD_STRIP);
		for(j = 0; j <= longs; j++)
		{
			double lng = 2 * M_PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);
			
			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
  
    // if there was a shader, now we have to disable it
    if( brdfs[0].brdf )
        brdfs[0].brdf->disableShader( SHADER_LITSPHERE );
}

void LitSphereWidget::paintGL()
{
    if( !isShowing() )
        return;
    
    glLineWidth( 1.5 );
    
    
    float useTheta = inTheta;
    if( doubleTheta )
        useTheta *= 2.0;
    incidentVector[0] = sin(useTheta) * cos(inPhi);
    incidentVector[1] = sin(useTheta) * sin(inPhi);
    incidentVector[2] = cos(useTheta);
    
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    
    
    float fWidth = float(width());
    float fHeight = float(height());
    if( width() > height() )
    {
        float aspect = fWidth / fHeight;
        glOrtho( -aspect * sphereMargin, aspect * sphereMargin, -sphereMargin, sphereMargin, NEAR_PLANE, FAR_PLANE );
    }
    else
    {
        float invAspect = fHeight / fWidth;
        glOrtho( -sphereMargin, sphereMargin, -invAspect * sphereMargin, invAspect * sphereMargin, NEAR_PLANE, FAR_PLANE );
    }
    
    
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt( 0, 0, 2.75,
               0, 0, 0,
               0, 1, 0 );

    if( brdfs.size() )
        drawSphere( 1.0, 100, 100 );
    
    
    glPopAttrib();
}


void LitSphereWidget::brightnessChanged( float v )
{
    brightness = v;
    updateGL();
}


void LitSphereWidget::gammaChanged( float v )
{
    gamma = v;
    updateGL();
}


void LitSphereWidget::exposureChanged( float v )
{
    exposure = v;
    updateGL();
}


void toThetaPhi( float x, float y, float z, float& theta, float& phi )
{
    theta = acos( z / sqrt( x*x + y*y + z*z ) );
    phi = atan2( y, x );
    if( phi < 0.0 )
        phi = -phi;
    else
        phi = 6.28318531 - phi;

}


void LitSphereWidget::mouseMoveEvent(QMouseEvent *event)
{
    float x, y, z;

    // ignore if the left mouse button isn't being held down
    if( !(event->buttons() & Qt::LeftButton) )
        return;
    
    float mx = (float)event->x();
    float my = (float)event->y();
    float centerX = float(width()) * 0.5;
    float centerY = float(height()) * 0.5;
    float radius;
    
    if( height() > width() )
        radius = float(width()) * 0.5 / sphereMargin;
    else
        radius = float(height()) * 0.5 / sphereMargin;
    
    // bail if we're outside the sphere
    x = mx - centerX;
    y = my - centerY;
    float dist = sqrt( x*x + y*y );
    if( dist > radius )
        return;
    
    // make coordinates on the unit sphere
    x /= radius;
    y /= radius;
    z = sqrt( 1 - x*x - y*y );


    float theta, phi;
    toThetaPhi( x, y, z, theta, phi );

    emit( incidentVectorChanged(theta, phi) );
}


void LitSphereWidget::doubleThetaChanged( int dp )
{
    doubleTheta = bool(dp);
    updateGL();
}

void LitSphereWidget::useNDotLChanged( int val )
{
    useNDotL = bool(val);
    updateGL();
}


void LitSphereWidget::incidentDirectionChanged( float theta, float phi )
{
    inTheta = theta;
    inPhi = phi;

    // repaint!
    updateGL();
}


void LitSphereWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    brdfs = brdfList;
    updateGL();
}

