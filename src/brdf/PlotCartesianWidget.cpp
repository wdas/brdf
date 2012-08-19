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
#include "PlotCartesianWidget.h"


PlotCartesianWidget::PlotCartesianWidget(QWidget *parent, std::vector<brdfPackage> bList, int type )
  : SharedContextGLWidget(parent)
{
  sliceType = type;
  brdfs = bList;

  NEAR_PLANE = 0.5f;
  FAR_PLANE = 50.0f;
  FOV_Y = 45.0f;

  resetViewingParams();

  inTheta = 0.785398163;
  inPhi = 0.785398163;

  useLogPlot = false;
  useNDotL = false;
  useSampleMult = false;
}


void PlotCartesianWidget::resetViewingParams()
{
  // default viewing parameters
  lookZoom = 1.0;
  centerX = 0.4;
  centerY = 0.5;

  scaleX = 0.8;
  scaleY = 1.0;
}


PlotCartesianWidget::~PlotCartesianWidget()
{
  makeCurrent();

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

void PlotCartesianWidget::resamplePushed()
{
  useSampleMult = true;
  updateGL();
  useSampleMult = false;
}



void PlotCartesianWidget::drawAnglePlot( brdfPackage pkg )
{
  DGLShader* shader = NULL;
  int shaderType = 0;

  // if there's a BRDF, the BRDF pbject sets up and enables the shader
  if( pkg.brdf )
  {
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
      shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
      shader->setUniformFloat( "incidentTheta", inTheta );
      shader->setUniformFloat( "incidentPhi", inPhi );
      shader->setUniformFloat( "useLogPlot", useLogPlot ? 1.0 : 0.0 );
      shader->setUniformFloat( "useNDotL", useNDotL ? 1.0 : 0.0 );
    }
  }

  glLineWidth( 1 );

  // the line!
  int nlinesegments = width();
  float aspect = float(width()) / float(height());
  double thetastart = (centerX + -1.0*aspect*lookZoom)/scaleX;
  double thetaend = (centerX + 1.0*aspect*lookZoom)/scaleX;
  if (thetastart < -0.5*M_PI || sliceType == THETA_H_PLOT) thetastart = -0.5*M_PI;
  if (thetaend > 0.5*M_PI  || sliceType == THETA_H_PLOT) thetaend = 0.5*M_PI;
  const double inc = (thetaend-thetastart) / (double)nlinesegments;

  glBegin( GL_LINE_STRIP );

  for( int i = 0; i < nlinesegments; i++ )
  {
    float theta = thetastart + (i+0.5) * inc;
    if (sliceType == THETA_H_PLOT) {
      // square thetaH to concentrate samples around thetaH==0
      theta *= theta * (1.0/(0.5*M_PI)) * (theta > 0 ? -1 : 1);
    }
    if (sliceType == ALBEDO_PLOT) {
      // limit number of directions albedo is calculated on
      if (theta>=-1.0*inc && (i%4 == 0 || i==nlinesegments-1) )
        glVertex3f( theta, 0.0, theta );
    }
    else
      glVertex3f( theta, 0.0, theta );
  }
  glEnd();

  // if there was a shader, now we have to disable it
  if( pkg.brdf )
    pkg.brdf->disableShader( shaderType );
}


void PlotCartesianWidget::paintGL()
{
  if( !isShowing() )
    return;

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
  glScalef( scaleX, scaleY, 1.0 );

  // y ticks
  glBegin( GL_LINES );
  if(lookZoom<0.8) {
    glColor3f( 0.8, 0.8, 0.8 );
    for( float i = 0.2; i<1; i+=0.2)
    {
      float yPos = i;
      glVertex3f( -1.57079633, yPos, 0 );
      glVertex3f( 1.57079633, yPos, 0 );
    }
    if(lookZoom<0.5) {
      glColor3f( 0.9, 0.9, 0.9 );
      for( float i = 0.1; i< 1; i+=0.2)
      {
        float yPos = i;
        glVertex3f( -1.57079633, yPos, 0 );
        glVertex3f( 1.57079633, yPos, 0 );
      }
      if(lookZoom<0.3) {
        glColor3f( 0.95, 0.95, 0.95);
        for( float i = 0.05; i<1; i+=0.1)
        {
          float yPos = i;
          glVertex3f( -1.57079633, yPos, 0 );
          glVertex3f( 1.57079633, yPos, 0 );
        }
      }
    }
  }
  glColor3f( 0.6, 0.6, 0.6 );
  for( int i = 1; i <= 10; i++ )
  {
    float yPos = float(i);
    glVertex3f( -1.57079633, yPos, 0 );
    glVertex3f( 1.57079633, yPos, 0 );
  }
  for( int i = 20; i <= 100; i += 10 )
  {
    float yPos = float(i);
    glVertex3f( -1.57079633, yPos, 0 );
    glVertex3f( 1.57079633, yPos, 0 );
  }
  glEnd();

  // x ticks & lines
  glColor3f( 0, 0, 0 );
  glBegin( GL_LINES );
  for( int i = -6; i <= 6; i++ )
  {
    glColor3f( 0, 0, 0 );
    float xPos = float(i) * 0.261799388;
    glVertex3f( xPos, 0, 0 );
    glVertex3f( xPos, -0.02, 0 );
    if(lookZoom<0.8) {
      glColor3f( .9, .9, .9 );
      glVertex3f( xPos, 0, 0 );
      glVertex3f( xPos, 1, 0 );
    }

  }
  glEnd();

  // x axis
  glColor3f( .4, .4, .4 );
  glBegin( GL_LINES );
  glVertex3f( -1.57079633, 0, 0 );
  glVertex3f(  1.57079633, 0, 0 );
  glEnd();

  // y axis
  glColor3f( .4, .4, .4 );
  glBegin( GL_LINES );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, 100.0, 0 );
  glEnd();

  // some labels - these get screwed up with zooming, but it's a start
  renderText( -0.05*lookZoom, -0.02-0.05*lookZoom, 0.0, "0" );
  renderText( 0.785398163-0.05*lookZoom, -0.02-0.05*lookZoom, 0.0, "45" );
  renderText( 1.57079633-0.05*lookZoom, -0.02-0.05*lookZoom, 0.0, "90" );
  renderText( -0.785398163-0.05*lookZoom, -0.02-0.05*lookZoom, 0.0, "45" );
  renderText( -1.57079633-0.05*lookZoom, -0.02-0.05*lookZoom, 0.0, "90" );

  if (lookZoom < 0.8) {
    glColor3f( .6, .6, .6 );
    for( int i = 1; i < 5; i++)
      renderText( 0.0, float(i)*0.2-0.02*lookZoom, 0.0, QString::number(i*0.2) );
    if (lookZoom < 0.5) {
      for( int i = 0; i < 5; i++)
        renderText( 0.0, 0.1+float(i)*0.2-0.02*lookZoom, 0.0, QString::number(0.1+i*0.2) );
      if (lookZoom < 0.3) {
        for( int i = 0; i < 10; i++)
          renderText( 0.0, 0.05+float(i)*0.1-0.02*lookZoom, 0.0, QString::number(0.05+i*0.1) );
      }
    }
  }
  glColor3f( .3, .3, .3 );
  for( int i = 1; i <= 9; i++ )
    renderText( 0.0, float(i)-0.02*lookZoom, 0.0, QString::number(i) );
  for( int i = 10; i <= 100; i += 10 )
    renderText( -0.1, float(i), 0.0, QString::number(i) );


  for( int i = 0; i < (int)brdfs.size(); i++ )
    drawAnglePlot( brdfs[i] );

  glPopAttrib();
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


void PlotCartesianWidget::incidentDirectionChanged( float theta, float phi )
{
  // update the incident direction
  inTheta = theta;
  inPhi = phi;

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
  inTheta = theta;
  inPhi = phi;
  angleParam = angParam;

  // repaint!
  updateGL();
}

void PlotCartesianWidget::samplingModeChanged(int newmode) {
  samplingMode = newmode;
  updateGL();
}

void PlotCartesianWidget::mouseDoubleClickEvent ( QMouseEvent *  )
{
  resetViewingParams();
  updateGL();
}
