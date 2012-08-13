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
#include <stdlib.h>
#include "./ptex/Ptexture.h"
#include "IBLWidget.h"
#include "SimpleModel.h"
#include "DGLFrameBuffer.h"
#include "angleConvert.h"
#include "DGLShader.h"
#include <string>
#include <iostream>
#include "Paths.h"

int PrintOpenGLError(const char *file, int line, const char *msg = 0)
{
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    int retCode = 0;
    GLenum glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        const char *errDescr = (const char *)gluErrorString(glErr);
        if (! errDescr) {
            if (glErr == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)
                errDescr = "Invalid framebuffer operation (ext)";
            else
                errDescr = "(unrecognized error code)";
        }

        char output[1000];
        sprintf(output, "OpenGL Error %s (%d) in file %s:%d%s%s%s",
                errDescr, glErr, file, line,
                msg ? " (" : "",
                msg ? msg : "",
                msg ? ")" : "");

        std::cout << output << std::endl;

        if (GLEW_GREMEDY_string_marker) {
            glStringMarkerGREMEDY(0, output);
        }

        retCode = 1;

        glErr = glGetError();
    }
    return retCode;
}


#define CKGL() PrintOpenGLError(__FILE__, __LINE__)


bool invertMatrix4( const float m[16], float inverse[16] )
{
    float invTemp[16];
    float determinant;

    invTemp[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    invTemp[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    invTemp[2]  =  m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
    invTemp[3]  = -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
    invTemp[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    invTemp[5]  =  m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    invTemp[6]  = -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
    invTemp[7]  =  m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
    invTemp[8]  =  m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    invTemp[9]  = -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    invTemp[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
    invTemp[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
    invTemp[12] = -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    invTemp[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    invTemp[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
    invTemp[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];

    // determinant = 0 means a singular matrix w/ no inverse
    determinant = m[0]*invTemp[0] + m[1]*invTemp[4] + m[2]*invTemp[8] + m[3]*invTemp[12];
    if( determinant == 0 ) return false;

    // divide by determinant
    determinant = 1.0 / determinant;
    for( int i = 0; i < 16; i++ )
        inverse[i] = invTemp[i] * determinant;

    return true;
}


// probability textures:
// R component: PDF
// G component: CDF
// B component: inverse CDF


IBLWidget::IBLWidget(QWidget *parent, std::vector<brdfPackage> bList )
    : SharedContextGLWidget(parent), meshDisplayListID(0), fbo(NULL),
      numSampleGroupsRendered(0), renderWithIBL(false), keepAddingSamples(false), 
      lastBRDFUsed(NULL), model(NULL)
{
    connect( this, SIGNAL(resetRenderingMode(bool)), parent, SLOT(renderingModeReset(bool)) );
    comp = NULL;
    fbo = NULL;
    
    brdfs = bList;
    
    iblRenderingMode = RENDER_NO_IBL;
    
    NEAR_PLANE = 0.01f;
    FAR_PLANE = 50.0f;
    FOV_Y = 45.0f;
    
    resetViewingParams();

    gamma = 2.2;
    exposure = 0.0;

    inTheta = 0.785398163;
    inPhi = 0.785398163;

    stepSize = 271;
    totalSamples = stepSize * 15;
    randomizeSampleGroupOrder();

    updateTimer = new QTimer(this);
    connect( updateTimer, SIGNAL(timeout()), this, SLOT(updateTimerFired()) );
    
    
    faceWidth = 128;
    faceHeight = 128;
    numColumns = faceWidth * 6;
    numRows = faceHeight;
}


IBLWidget::~IBLWidget()
{
    makeCurrent();
    delete model;
}


void IBLWidget::resetViewingParams()
{
    envPhi = 0.0;
    envTheta = 0.0;
    
    lookPhi = -0.85;
    lookTheta = 1.57;
    lookZoom = 0.75;
}


void IBLWidget::randomizeSampleGroupOrder()
{
    sampleGroupOrder = new int[stepSize];

    // start with everything in-order
    for( int i = 0; i < stepSize; i++ )
        sampleGroupOrder[i] = i;

    // now swap random elements a bunch of times to shuffle the pass order
    for( int i = 0; i < stepSize*100; i++ )
    {
        int a = rand() % stepSize;
        int b = rand() % stepSize;
        int temp = sampleGroupOrder[a];
        sampleGroupOrder[a] = sampleGroupOrder[b];
        sampleGroupOrder[b] = temp;
    }
}



QSize IBLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize IBLWidget::sizeHint() const
{
    return QSize(256, 256);
}

void IBLWidget::initializeGL()
{
    glewInit();

    

    model = new SimpleModel();

    loadIBL( (getProbesPath() + "beach.penv").c_str() );
    loadModel( (getModelsPath() + "teapot.obj").c_str() );

    // load the shaders
    resultShader = new DGLShader( "", (getShaderTemplatesPath() + "IBLResult.frag").c_str() );
    compShader = new DGLShader( "", (getShaderTemplatesPath() + "IBLComp.frag").c_str() );

    glClearColor( 0, 0, 0, 0 );	
}



bool IBLWidget::recreateFBO()
{
    // if the FBO is the right size, no need to create it
    if( fbo && fbo->width() == width() && fbo->height() == height() )
        return false;

    if (fbo) delete fbo;
    if (comp) delete comp;

    fbo = new DGLFrameBuffer( width(), height(), "FBO" );
    fbo->addColorBuffer( 0, GL_RGBA32F_ARB );
    fbo->addDepthBuffer();
    fbo->checkStatus();

    comp = new DGLFrameBuffer( width(), height(), "Comp" );
    comp->addColorBuffer( 0, GL_RGBA32F_ARB );
    comp->checkStatus();

    resetComps();

    return true;
}


void IBLWidget::setupProjectionMatrix()
{
    float fWidth = float(width());
    float fHeight = float(height());
    if( width() > height() )
    {
        float aspect = fWidth / fHeight;
        glOrtho( -aspect, aspect, -1.0, 1.0, NEAR_PLANE, FAR_PLANE );
    }
    else
    {
        float invAspect = fHeight / fWidth;
        glOrtho( -1.0, 1.0, -invAspect, invAspect, NEAR_PLANE, FAR_PLANE );
    }
}


void IBLWidget::renderObject()
{
    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);


    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable( GL_DEPTH_TEST );
    glLineWidth( 1.5 );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    setupProjectionMatrix();
    
    
    float lookVec[3];
    lookVec[0] = sin(lookTheta) * cos(lookPhi);
    lookVec[2] = sin(lookTheta) * sin(lookPhi);
    lookVec[1] = cos(lookTheta);

    
    lookVec[0] *= 25.0;
    lookVec[1] *= 25.0;
    lookVec[2] *= 25.0;
    
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( lookVec[0], lookVec[1], lookVec[2],
               0, 0, 0,
               0, 1, 0 );

    float scale = 1.0 / lookZoom;
    glScalef( scale, scale, scale );
    if( brdfs.size() )
        drawObject();
    
    glPopAttrib();
}


void IBLWidget::paintGL()
{
    if( !isShowing() )
        return;
    

    recreateFBO();


    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glRotatef( radiansToDegrees(envPhi), 0, 1, 0 );
    glRotatef( radiansToDegrees(envTheta), 1, 0, 0 );  
    glGetFloatv( GL_MODELVIEW_MATRIX, envRotMatrix );
    glPopMatrix();
    
    
    
    invertMatrix4( envRotMatrix, envRotMatrixInverse );


    if( numSampleGroupsRendered < stepSize )
    {
        fbo->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           
        renderObject();
        fbo->unbind();

        ///////////////////////////////////////
        compResult();

        // another sample group rendered!
        numSampleGroupsRendered++;

        if( renderWithIBL )
        {
            if( numSampleGroupsRendered < stepSize )
                startTimer();
            else
            {
                printf( "done!\n" );
                stopTimer();
            }
        }
        ///////////////////////////////////////
    }

    
    // now draw the final result
    drawResult();
}


void IBLWidget::resetComps()
{
    numSampleGroupsRendered = 0;

    if( comp )
    {
        comp->bind();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        comp->unbind();
    }

    updateGL();
}


void IBLWidget::mousePressEvent( QMouseEvent* event )
{
    lastPos = event->pos();
}


void IBLWidget::mouseReleaseEvent ( QMouseEvent* )
{
    //updateGL();
}

void IBLWidget::mouseMoveEvent( QMouseEvent* event )
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    // left mouse button adjusts the viewing dir (of either the object or the envmap)
    if (event->buttons() & Qt::LeftButton)
    {
        // if control is pressed, adjust the environment map rotation
        if( event->modifiers() & Qt::ControlModifier )
        {
            // no real need to clamp these to anything...
            envPhi += float(-dx) * 0.02;
            envTheta += float(-dy) * 0.03;
        }

        // otherwise, adjust the model view direction
        else
        {
            lookPhi += float(dx) * 0.02;
            lookTheta += float(-dy) * 0.03;
            if( lookTheta < 0.001 )
            lookTheta = 0.001;
            if( lookTheta > (M_PI - 0.001) )
            lookTheta = (M_PI - 0.001);
        }
    }

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
    resetComps();
    updateGL();
}


void IBLWidget::gammaChanged( float v )
{
    gamma = v;
    updateGL();
}


void IBLWidget::exposureChanged( float v )
{
    exposure = v;
    updateGL();
}


void IBLWidget::drawObject()
{
    DGLShader* shader = NULL;

    // if there's a BRDF, the BRDF pbject sets up and enables the shader
    if( brdfs.size() )
    {
        shader = brdfs[0].brdf->getUpdatedShader( SHADER_IBL, &brdfs[0] );

        if( shader )
        {
            shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
            shader->setUniformFloat( "gamma", gamma );
            shader->setUniformFloat( "exposure", exposure );
            shader->setUniformTexture( "envCube", envTexID, GL_TEXTURE_CUBE_MAP );

            shader->setUniformTexture( "probTex", probTexID );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            shader->setUniformTexture( "marginalProbTex", marginalProbTexID );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            shader->setUniformFloat( "texDims", float(envTex.w), float(envTex.h) );

            shader->setUniformMatrix4( "envRotMatrix", envRotMatrix );
            shader->setUniformMatrix4( "envRotMatrixInverse", envRotMatrixInverse );

            shader->setUniformInt( "totalSamples", totalSamples );
            shader->setUniformInt( "stepSize", stepSize );
            shader->setUniformInt( "passNumber", sampleGroupOrder[numSampleGroupsRendered] );

            shader->setUniformFloat( "renderWithIBL", (renderWithIBL) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useIBLImportance", bool(iblRenderingMode == RENDER_IBL_IS) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useBRDFImportance", bool(iblRenderingMode == RENDER_BRDF_IS) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useMIS", bool(iblRenderingMode == RENDER_MIS) ? 1.0 : 0.0 );
        }
    }

    model->drawVBO();

    // if there was a shader, now we have to disable it
    if( brdfs[0].brdf )
    {
        brdfs[0].brdf->disableShader( SHADER_IBL );
    }
}


void IBLWidget::drawSphere( double, int lats, int longs )
{
    float radius = 2.0;

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
            glVertex3f(radius * x * zr0, radius * y * zr0, radius * z0);
            glNormal3f(radius * x * zr1, radius * y * zr1, radius * z1);
            glVertex3f(radius * x * zr1, radius * y * zr1, radius * z1);
        }
        glEnd();
    }
  
}


void IBLWidget::drawResult()
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    

    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    gluOrtho2D(0.0, (float)width(), 0.0, (float)height());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0, 1.0, 1.0);

    int x1 = 0;
    int y1 = 0;
    int x2 = width();
    int y2 = height();

    resultShader->enable();
    resultShader->setUniformTexture( "resultTex", comp->colorBufferID() );
    //resultShader->BindAndEnableTexture( "resultTex", fbo->GetColorTextureID(), GL_TEXTURE0 );

    resultShader->setUniformTexture( "envCube", envTexID, GL_TEXTURE_CUBE_MAP );

    resultShader->setUniformFloat( "gamma", gamma );
    resultShader->setUniformFloat( "exposure", exposure );
    resultShader->setUniformFloat( "aspect", float(width()) / float(height()) );
    resultShader->setUniformFloat( "renderWithIBL", renderWithIBL ? 1.0 : 0.0 );
    resultShader->setUniformMatrix4( "envRotMatrix", envRotMatrix );

    resultShader->setUniformTexture( "probTex", probTexID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    resultShader->setUniformTexture( "marginalProbTex", marginalProbTexID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );


    

    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2i(x1, y1);

        glTexCoord2f(1.0, 0.0);
        glVertex2i(x2, y1);

        glTexCoord2f(1.0, 1.0);
        glVertex2i(x2, y2);

        glTexCoord2f(0.0, 1.0);
        glVertex2i(x1, y2);
    glEnd();



    resultShader->disable();

    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    CKGL();

    
}



void IBLWidget::compResult()
{
    comp->bind();

    if( numSampleGroupsRendered )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE );
    }
    else
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    gluOrtho2D(0.0, (float)width(), 0.0, (float)height());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0, 1.0, 1.0);

    int x1 = 0;
    int y1 = 0;
    int x2 = width();
    int y2 = height();

    compShader->enable();
    compShader->setUniformTexture( "resultTex", fbo->colorBufferID() );
    //printf( "display with numSampleGroupsRendered = %d\n", numSampleGroupsRendered );

    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2i(x1, y1);

        glTexCoord2f(1.0, 0.0);
        glVertex2i(x2, y1);

        glTexCoord2f(1.0, 1.0);
        glVertex2i(x2, y2);

        glTexCoord2f(0.0, 1.0);
        glVertex2i(x1, y2);
    glEnd();

    compShader->disable();

    glDisable( GL_BLEND );

    comp->unbind();
}


void IBLWidget::updateTimerFired()
{
    if( keepAddingSamples )
        updateGL();
}


void IBLWidget::startTimer()
{
    if( !updateTimer->isActive() )
        //updateTimer->start( 25 );
        updateTimer->start( 50 );
}


void IBLWidget::stopTimer()
{
    updateTimer->stop();
}



void IBLWidget::keepAddingSamplesChanged(int rs)
{
    keepAddingSamples = bool(rs);
}


void IBLWidget::createGLSamplingTextures()
{
    glBindTexture( GL_TEXTURE_2D, probTexID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_INTENSITY32F_ARB, probTex.w, probTex.h, 0, GL_RED, GL_FLOAT, probTex.getPtr() );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glBindTexture( GL_TEXTURE_2D, marginalProbTexID );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_INTENSITY32F_ARB, marginalProbTex.w, marginalProbTex.h, 0, GL_RED, GL_FLOAT, marginalProbTex.getPtr() );
    glBindTexture( GL_TEXTURE_2D, 0 );
}


void IBLWidget::loadModel( const char* filename )
{
    if( model ) {
        model->loadOBJ( filename );
    }
}


void IBLWidget::loadIBL( const char* filename )
{
    printf( "opening %s\n", filename );

    // try and load it
    Ptex::String error;
    PtexTexture* tx = PtexTexture::open(filename, error);
    if (!tx) return;


    Ptex::DataType dataType = tx->dataType();
    int numChannels = tx->numChannels();
    int numFaces=tx->numFaces();
    int faceWidth=0, faceHeight=0;

    if( dataType != Ptex::dt_float || numChannels != 3 || numFaces != 6 )
    {
        printf( "not the right kind\n" );
        return;
    }

    for( int i = 0; i < numFaces; i++ )
    {
        int face = i;


        Ptex::Res res = tx->getFaceInfo(i).res;
        if( i == 0 )
        {
            faceWidth = res.u();
            faceHeight = res.v();

            // printf( "size: %dx%d    (%dx%d)\n", faceWidth, faceHeight, faceWidth*6, faceHeight );

            envTex.create( faceWidth*6, faceHeight );
        }

        if( faceWidth != res.u() || faceHeight != res.v() )
        {
            printf( "Error loading ptex file\n" );
            return;
        }

        float* faceData = (float*)malloc( faceWidth*faceHeight*numChannels*sizeof(float) );
        tx->getData( i, faceData, 0 );

        // copy the data into the envmap texture
        for( int j = 0; j < faceHeight * faceWidth; j++ )
        {
            color3 c( faceData[j*3+0], faceData[j*3+1], faceData[j*3+2] );  
            envTex.setPixel( faceWidth*face + j % faceWidth, j / faceWidth, c );
        }

        free( faceData );

    }
    tx->release();

    // allocate texture names (if we haven't before)
    if( envTexID == 0 ) {
        glGenTextures( 1, &envTexID );
        glGenTextures( 1, &probTexID );
        glGenTextures( 1, &marginalProbTexID );
    }

    // now that the envmap tex is loaded, compute the sampling data
    computeEnvMapSamplingData();
    createGLSamplingTextures();


    glGenTextures(1, &envTexID);
    glBindTexture( GL_TEXTURE_CUBE_MAP, envTexID );
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);


    BitmapContainer<color3> flip;
    flip.create(envTex.w, envTex.h);//get face flipped to match GL thinking!
    for (int y=0; y < envTex.h; y++){
        color3* in = &envTex.data[y*envTex.w];
        color3* out = &flip.data[(envTex.h-y-1)*envTex.w];
        for (int x=0; x < envTex.w; x++)
            out[x] = in[x];
    }

    BitmapContainer<color3> tmp;
    tmp.create(envTex.h, envTex.h);


    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*0, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );


    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*1, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*2, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*3, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*4, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );
    


    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*5, y));
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );


    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

    CKGL();
}


void IBLWidget::incidentDirectionChanged( float theta, float phi )
{
    inTheta = theta;
    inPhi = phi;

    // we only need to repaint if we're not in IBL mode
    if( !renderWithIBL )
    {
        resetComps();
        updateGL();
    }
}

void IBLWidget::brdfListChanged( std::vector<brdfPackage> brdfList )
{
    brdfs = brdfList;

    if( brdfs[0].brdf != lastBRDFUsed )
    {
        if( brdfs[0].brdf && brdfs[0].brdf->hasISFunction() )
            emit( resetRenderingMode(true) );
        else
            emit( resetRenderingMode(false) );
    }
    
    if( !brdfs.size() || (brdfs[0].dirty || brdfs[0].brdf != lastBRDFUsed) )
    {
        resetComps();
        updateGL();
        lastBRDFUsed = brdfs[0].brdf;
    }
}


void IBLWidget::reloadAuxShaders()
{
    if( resultShader )
        resultShader->reload();
    if( compShader )
        compShader->reload();

    resetComps();
    updateGL();
}



void IBLWidget::redrawAll()
{
    resetComps();
    updateGL();
}




double IBLWidget::calculateProbs( const double* pdf, float* data, int numElements )
{
    std::vector<double> cdf(numElements);

    // sum PDF
    double pdfSum = 0;
    for( int i = 0; i < numElements; i++ )
        pdfSum += pdf[i];

    if (pdfSum <= 0) {
        // degenerate - no probability to reach this area
        // make uniform CDF
        double cdfScale = 1.0/numElements;
        for( int i = 0; i < numElements; i++ ) {
            cdf[i] = (i+1) * cdfScale;
        }
    }
    else {
        // compute the CDF based on normalized pdf
        double cdfTotal = 0.0;
        double cdfScale = 1/pdfSum;
        for( int i = 0; i < numElements; i++ ) {
            cdfTotal += pdf[i] * cdfScale;
            cdf[i] = cdfTotal;
        }
    }

    // compute the inverse CDF
    // (center samples between 0..1 range with implied (0,0) and (1,1) points)
    int yi = 0;
    double oneOverNumElements = 1.0 / numElements;
    for( int xi = 0; xi < numElements; xi++ )
    {
        // find segment spanning target x value
        double x = (xi+.5) * oneOverNumElements;
        while( cdf[yi] < x && yi < numElements-1)
            yi++;

        // interpolate segment to get corresponding y value
        double xa = yi > 0 ? cdf[yi-1] : 0;
        double ya = yi * oneOverNumElements;
        double xb = cdf[yi];
        double yb = (yi+1) * oneOverNumElements;
        data[xi] = float(ya + (yb-ya)/(xb-xa) * (x-xa));
    }

    return pdfSum;
}



void IBLWidget::computeEnvMapSamplingData()
{
    // create the "images" to store the tex and marginal tex
    probTex.create( envTex.w, envTex.h );
    marginalProbTex.create( envTex.h, 1 );

    std::vector<double> marginalPdf(envTex.h);
    std::vector<double> conditionalPdf(envTex.w);

    // loop through each of the rows in the image
    for( int row = 0; row < envTex.h; row++ )
    {
        double y = (row + 0.5)/envTex.h * 2 - 1, ysquared = y*y;

        // loop through the pixels of this row, computing and storing a probability for each pixel
        for( int col = 0; col < envTex.w; col++ )
        {
            // compute the PDF value for this pixel
            // (compensate for cubemap distortion - see pbrt v2 pg 947)
            double x = ((col % envTex.h) + 0.5)/envTex.h * 2 - 1, xsquared = x*x;
            double undistort = pow(xsquared + ysquared + 1, -1.5);
            conditionalPdf[col] = envTex.getPixel( col, row ).luminance() * undistort;
        }

        // compute the CDF and inverse CDF for this row
        double pdfSum = calculateProbs( &conditionalPdf[0], (float*)probTex.getPtr(row), envTex.w );

        // save the integral of the PDF for this row in the marginal image
        marginalPdf[row] = pdfSum;
    }

    // compute the CDF and inverse CDF for the marginal image
    calculateProbs(&marginalPdf[0], (float*)marginalProbTex.getPtr(), envTex.h );
}


void IBLWidget::renderingModeChanged( int newmode )
{   
    iblRenderingMode = newmode;
    
    renderWithIBL = bool(iblRenderingMode > RENDER_NO_IBL);
    
    resetComps();
}


void IBLWidget::mouseDoubleClickEvent ( QMouseEvent *  )
{
    resetViewingParams();
    resetComps();
    updateGL();
}