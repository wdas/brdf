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

#ifdef _MSC_VER
    #include <windows.h>
	#define _USE_MATH_DEFINES
	#include <cmath> 
#endif

#include <QTimer>
#include <QWidget>
#include <QMouseEvent>
#include <QString>
#include <math.h>
#include <stdlib.h>
#include "ptex/Ptexture.h"
#include "IBLWidget.h"
#include "SimpleModel.h"
#include "DGLFrameBuffer.h"
#include "DGLShader.h"
#include <string>
#include <iostream>
#include "Paths.h"
#include "glerror.h"


// probability textures:
// R component: PDF
// G component: CDF
// B component: inverse CDF


IBLWidget::IBLWidget(QWidget *parent, std::vector<brdfPackage> bList )
    : GLWindow(parent->windowHandle()), meshDisplayListID(0), fbo(NULL),
      numSampleGroupsRendered(0), renderWithIBL(false), keepAddingSamples(true),
      lastBRDFUsed(NULL), model(NULL)
{
    connect( this, SIGNAL(resetRenderingMode(bool)), parent, SLOT(renderingModeReset(bool)) );
    comp = NULL;
    fbo = NULL;

    brdfs = bList;

    iblRenderingMode = RENDER_IBL_IS;

    NEAR_PLANE = 0.01f;
    FAR_PLANE = 50.0f;
    FOV_Y = 45.0f;

    resetViewingParams();

    gamma = 2.2;
    exposure = 0.0;

    inTheta = 0.785398163;
    inPhi = 0.785398163;

    stepSize = 271;
    totalSamples = stepSize * 15;//500
    randomizeSampleGroupOrder();

    updateTimer = new QTimer(this);
    connect( updateTimer, SIGNAL(timeout()), this, SLOT(updateTimerFired()) );


    faceWidth = 128;
    faceHeight = 128;
    numColumns = faceWidth * 6;
    numRows = faceHeight;

    quad = NULL;

    envTexID = 0;

    initializeGL();
}


IBLWidget::~IBLWidget()
{
    glcontext->makeCurrent(this);
    delete model;
    delete quad;
}


void IBLWidget::resetViewingParams()
{
    envPhi = 0.0;
    envTheta = 0.0;

    lookPhi = -0.85;
    lookTheta = 1.57;
    lookZoom = 1;
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
    glcontext->makeCurrent(this);

    model = new SimpleModel();

    loadIBL( (getProbesPath() + "beach.penv").c_str() );
    loadModel( (getModelsPath() + "sphere.obj").c_str() );

    // load the shaders
    resultShader = new DGLShader( (getShaderTemplatesPath() + "Quad.vert").c_str(), (getShaderTemplatesPath() + "IBLResult.frag").c_str() );
    compShader = new DGLShader( (getShaderTemplatesPath() + "Quad.vert").c_str(), (getShaderTemplatesPath() + "IBLComp.frag").c_str() );
}



bool IBLWidget::recreateFBO()
{
    mSize = width() < height() ? width() : height();
    mSize *= devicePixelRatio();

    // if the FBO is the right size, no need to create it
    if( fbo && fbo->width() == mSize && fbo->height() == mSize )
        return false;

    if(quad) delete quad;
    quad = new Quad(0.f, 0.f, mSize, mSize, 0.f, 0.f, 1.f, 1.f);

    if (fbo) delete fbo;
    if (comp) delete comp;

    fbo = new DGLFrameBuffer( mSize, mSize, "FBO" );
    fbo->addColorBuffer( 0, GL_RGBA32F );
    fbo->addDepthBuffer();
    fbo->checkStatus();

    comp = new DGLFrameBuffer( mSize, mSize, "Comp" );
    comp->addColorBuffer( 0, GL_RGBA32F );
    comp->checkStatus();

    glf->glDisable( GL_BLEND );
    resetComps();

    return true;
}


void IBLWidget::setupProjectionMatrix()
{
    float fWidth = mSize;
    float fHeight = mSize;
    if( width() > height() )
    {
        float aspect = fWidth / fHeight;
        projectionMatrix = glm::ortho( -aspect, aspect, -1.f, 1.f, NEAR_PLANE, FAR_PLANE );
    }
    else
    {
        float invAspect = fHeight / fWidth;
        projectionMatrix = glm::ortho( -1.f, 1.f, -invAspect, invAspect, NEAR_PLANE, FAR_PLANE );
    }
}


void IBLWidget::renderObject()
{
    incidentVector[0] = sin(inTheta) * cos(inPhi);
    incidentVector[1] = sin(inTheta) * sin(inPhi);
    incidentVector[2] = cos(inTheta);

    glf->glEnable( GL_DEPTH_TEST );

    setupProjectionMatrix();

    glm::vec3 lookVec;
    lookVec[0] = sin(lookTheta) * cos(lookPhi) * 25.f;
    lookVec[2] = sin(lookTheta) * sin(lookPhi) * 25.f;
    lookVec[1] = cos(lookTheta) * 25.f;

    modelViewMatrix = glm::lookAt(lookVec, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    float scale = 1.f / lookZoom;
    modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(scale, scale, scale));

    normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));

    if( brdfs.size() )
        drawObject();
}

void IBLWidget::paintGL()
{
    if(!isShowing())
        return;

    glcontext->makeCurrent(this);

    glf->glClearColor( 0, 0, 0, 0 );
    glf->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    recreateFBO();

    envRotMatrix = glm::rotate(glm::mat4(1.f), envPhi, glm::vec3(0, 1, 0));
    envRotMatrix = glm::rotate(envRotMatrix, envTheta, glm::vec3(0, 1, 0));
    envRotMatrixInverse = glm::inverse(envRotMatrix);

    if( numSampleGroupsRendered < stepSize )
    {
        fbo->bind();
        glf->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

    glcontext->swapBuffers(this);
}


void IBLWidget::resetComps()
{
    numSampleGroupsRendered = 0;

    if( comp )
    {
        comp->bind();
        glf->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

        // redraw
        resetComps();
        updateGL();
    }

    else if (event->buttons() & Qt::RightButton)
    {
        // use the dir with the biggest change
        int d = abs(dx) > abs(dy) ? dx : dy;

        lookZoom -= float(d) * lookZoom * 0.05;
        lookZoom = std::max<float>( lookZoom, 0.01f );
        lookZoom = std::min<float>( lookZoom, 100.0f );

        // redraw
        resetComps();
        updateGL();
    }


    lastPos = event->pos();
}


void IBLWidget::gammaChanged( float v )
{
    gamma = v;
    redrawAll();
}


void IBLWidget::exposureChanged( float v )
{
    exposure = v;
    redrawAll();
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
            shader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
            shader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(modelViewMatrix));
            shader->setUniformMatrix3("normalMatrix",  glm::value_ptr(normalMatrix));

            shader->setUniformFloat( "incidentVector", incidentVector[0], incidentVector[1], incidentVector[2] );
            shader->setUniformFloat( "gamma", gamma );
            shader->setUniformFloat( "exposure", exposure );

            shader->setUniformTexture( "envCube", envTexID, GL_TEXTURE_CUBE_MAP );

            shader->setUniformTexture( "probTex", probTexID );
            glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            shader->setUniformTexture( "marginalProbTex", marginalProbTexID );
            glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            shader->setUniformFloat( "texDims", float(envTex.w), float(envTex.h) );

            shader->setUniformMatrix4( "envRotMatrix", glm::value_ptr(envRotMatrix) );
            shader->setUniformMatrix4( "envRotMatrixInverse", glm::value_ptr(envRotMatrixInverse) );

            shader->setUniformInt( "totalSamples", totalSamples );
            shader->setUniformInt( "stepSize", stepSize );
            shader->setUniformInt( "passNumber", sampleGroupOrder[numSampleGroupsRendered] );

            shader->setUniformFloat( "renderWithIBL", (renderWithIBL) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useIBLImportance", bool(iblRenderingMode == RENDER_IBL_IS) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useBRDFImportance", bool(iblRenderingMode == RENDER_BRDF_IS) ? 1.0 : 0.0 );
            shader->setUniformFloat( "useMIS", bool(iblRenderingMode == RENDER_MIS) ? 1.0 : 0.0 );
        }
    }

    CKGL();

    model->drawVBO(shader);

    CKGL();

    // if there was a shader, now we have to disable it
    if( brdfs[0].brdf )
    {
        brdfs[0].brdf->disableShader( SHADER_IBL );
    }
}


void IBLWidget::drawResult()
{
    glf->glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glf->glClear(GL_DEPTH_BUFFER_BIT);
    glf->glDisable(GL_DEPTH_TEST);

    glf->glViewport((width() * devicePixelRatio() - mSize)/2, (height() * devicePixelRatio() - mSize)/2, mSize, mSize);
    projectionMatrix = glm::ortho(0.f, (float)mSize, 0.f, (float)mSize);

    resultShader->enable();

    CKGL();

    glm::mat4 id(1.f);
    resultShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    resultShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
    resultShader->setUniformTexture( "resultTex", comp->colorBufferID() );

    resultShader->setUniformTexture( "envCube", envTexID, GL_TEXTURE_CUBE_MAP );

    resultShader->setUniformFloat( "gamma", gamma );
    resultShader->setUniformFloat( "exposure", exposure );
    resultShader->setUniformFloat( "aspect", float(mSize) / float(mSize) );
    resultShader->setUniformFloat( "renderWithIBL", renderWithIBL ? 1.0 : 0.0 );
    resultShader->setUniformMatrix4( "envRotMatrix", glm::value_ptr(envRotMatrix) );

    resultShader->setUniformTexture( "probTex", probTexID );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    resultShader->setUniformTexture( "marginalProbTex", marginalProbTexID );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    quad->draw(resultShader);

    resultShader->disable();

    glf->glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void IBLWidget::compResult()
{
    comp->bind();

    if( numSampleGroupsRendered )
    {
        glf->glEnable( GL_BLEND );
        glf->glBlendFunc( GL_ONE, GL_ONE );
    }
    else
        glf->glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    projectionMatrix = glm::ortho(0.f, (float)mSize, 0.f, (float)mSize);

    compShader->enable();

    glm::mat4 id(1.f);
    compShader->setUniformMatrix4("projectionMatrix", glm::value_ptr(projectionMatrix));
    compShader->setUniformMatrix4("modelViewMatrix",  glm::value_ptr(id));
    compShader->setUniformTexture("resultTex", fbo->colorBufferID());
    //printf( "display with numSampleGroupsRendered = %d\n", numSampleGroupsRendered );

    quad->draw(compShader);

    compShader->disable();

    glf->glDisable( GL_BLEND );

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
        updateTimer->start( 50 );
}

void IBLWidget::stopTimer()
{
    updateTimer->stop();
}

void IBLWidget::keepAddingSamplesChanged(int rs)
{
    keepAddingSamples = bool(rs);
    if(keepAddingSamples)
        updateGL();
}

void IBLWidget::createGLSamplingTextures()
{
    glf->glBindTexture( GL_TEXTURE_2D, probTexID );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glf->glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, probTex.w, probTex.h, 0, GL_RED, GL_FLOAT, probTex.getPtr() );
    glf->glBindTexture( GL_TEXTURE_2D, 0 );

    glf->glBindTexture( GL_TEXTURE_2D, marginalProbTexID );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glf->glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glf->glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, marginalProbTex.w, marginalProbTex.h, 0, GL_RED, GL_FLOAT, marginalProbTex.getPtr() );
    glf->glBindTexture( GL_TEXTURE_2D, 0 );
}

void IBLWidget::loadModel( const char* filename )
{
    printf( "opening %s... ", filename );
    if( model ) {
        if(model->loadOBJ( filename )){
            printf( "success\n");
            return;
        }
    }
    printf( "failed\n");
}

void IBLWidget::loadIBL( const char* filename )
{
    printf( "opening %s... ", filename );

    // try and load it
    Ptex::String error;
    PtexTexture* tx = PtexTexture::open(filename, error);
    if (!tx) {
        printf( "failed\n");
        return;
    }

    CKGL();

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
            printf( "error loading ptex file\n" );
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
    printf( "success\n");

    // allocate texture names (if we haven't before)
    if( envTexID == 0 ) {
        glf->glGenTextures( 1, &envTexID );
        glf->glGenTextures( 1, &probTexID );
        glf->glGenTextures( 1, &marginalProbTexID );
    }

    CKGL();

    // now that the envmap tex is loaded, compute the sampling data
    computeEnvMapSamplingData();
    createGLSamplingTextures();

    CKGL();

    glf->glGenTextures(1, &envTexID);
    glf->glBindTexture( GL_TEXTURE_CUBE_MAP, envTexID );
    glf->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);


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

    CKGL();

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*0, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );


    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*1, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*2, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*3, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*4, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );


    CKGL();

    for (int y=0; y < tmp.h; y++)
        for (int x=0; x < tmp.w; x++)
            tmp.setPixel(x, y,flip.getPixel(x+tmp.w*5, y));
    glf->glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , 0,GL_R11F_G11F_B10F, tmp.w, tmp.h, 0, GL_RGB, GL_FLOAT, tmp.getPtr() );

    CKGL();

    glf->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glf->glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

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
