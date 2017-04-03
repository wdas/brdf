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

#ifndef IBLWIDGET_H
#define IBLWIDGET_H

#include <vector>

#include "bitmapContainer.h"
#include "BRDFBase.h"
#include "SharedContextGLWidget.h"
#include "Quad.h"

class DGLFrameBuffer;
class DGLShader;
class SimpleModel;


#define RENDER_NO_IBL 0
#define RENDER_REGULAR_SAMPLING 1
#define RENDER_IBL_IS 2
#define RENDER_BRDF_IS 3
#define RENDER_MIS 4



class IBLWidget : public GLWindow
{
    Q_OBJECT

public:
    IBLWidget( QWidget *parent, std::vector<brdfPackage> );
    ~IBLWidget();
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    
    void loadIBL( const char* filename );
    void loadModel( const char* filename );
    void redrawAll();

public slots:
    void incidentDirectionChanged( float theta, float phi );
    void brdfListChanged( std::vector<brdfPackage> );
    void gammaChanged( float );
    void exposureChanged( float );
    void updateTimerFired();
    void keepAddingSamplesChanged(int);
    void renderingModeChanged(int);
    
    void reloadAuxShaders();

protected:
    void initializeGL();
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent ( QMouseEvent * event );
    void mouseDoubleClickEvent ( QMouseEvent * event );
    
signals:
    void resetRenderingMode(bool);

private:

    void resetViewingParams();

    void setupProjectionMatrix();
    void renderObject();
    void drawObject();
    void drawSphere( double, int lats, int longs );

    bool recreateFBO();

    void drawResult();
    void compResult();
    void resetComps();

    void startTimer();
    void stopTimer();

    void randomizeSampleGroupOrder();

    void updateEnvRot();

    float gamma;
    float exposure;
    
    float NEAR_PLANE;
    float FAR_PLANE;
    float FOV_Y;

    float inTheta, inPhi;

    float lookPhi;
    float lookTheta;
    float lookZoom;

    float envPhi;
    float envTheta;
        
    glm::mat4 projectionMatrix;
    glm::mat4 modelViewMatrix;
    glm::mat3 normalMatrix;
    glm::mat4 envRotMatrix;
    glm::mat4 envRotMatrixInverse;

    Quad* quad;
    
    int mSize;

    QPoint lastPos;
    bool tumbling;
    
    std::vector<brdfPackage> brdfs;
    
    GLuint envCube;

    float incidentVector[3];

    GLuint meshDisplayListID;


    DGLFrameBuffer* comp;
    DGLFrameBuffer* fbo;
    DGLShader* resultShader;
    DGLShader* compShader;

    QTimer* updateTimer;

    int numSampleGroupsRendered;
    int* sampleGroupOrder;
    int stepSize;
    int totalSamples;

    bool renderWithIBL;
    bool keepAddingSamples;
    BRDFBase* lastBRDFUsed;

    void computeEnvMapSamplingData();
    double calculateProbs( const double* pdf, float* data, int numElements );
    void createGLSamplingTextures();

    GLuint envTexID;
    BitmapContainer<color3> envTex;

    GLuint probTexID;
    BitmapContainer<float> probTex;

    GLuint marginalProbTexID;
    BitmapContainer<float> marginalProbTex;

    int faceWidth;
    int faceHeight;
    int numColumns;
    int numRows;
    
    int iblRenderingMode;

    SimpleModel* model;
};

#endif
