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

#ifndef VIEWERANGLEPLOT_H
#define VIEWERANGLEPLOT_H

#include "BRDFBase.h"
#include "SharedContextGLWidget.h"


#define THETA_V_PLOT 0
#define THETA_H_PLOT 1
#define THETA_D_PLOT 2
#define ALBEDO_PLOT 3


class PlotCartesianWidget : public GLWindow
{
        Q_OBJECT

public:
    PlotCartesianWidget( QWindow *parent, std::vector<brdfPackage>, int type );
    ~PlotCartesianWidget();
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setNSamples( int ns ) { nSamples = ns; }
    void setAngleParam( float ap ) { angleParam = ap; }
    void setAngles( float theta, float phi, float angParam ); 

public slots:
    void incidentDirectionChanged( float theta, float phi );
    void graphParametersChanged( bool logPlot, bool nDotL );
    void brdfListChanged( std::vector<brdfPackage> );
    void samplingModeChanged(int newmode);
    void resamplePushed();

protected:
    void initializeGL();
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent ( QMouseEvent * event );

private:
    void resetViewingParams();
    void initializeText();
    void updateAxisVAO();
    void updateInputDataLineVAO();
    void updateProjectionMatrix();
    void updateViewMatrix();

    void drawAxis();
    void drawLabels();
    void renderText(float pos_x, float pos_y, const QString& text, const glm::vec3 &color);
    DGLShader* updateShader(brdfPackage base);

    void drawThetaHSlice( DGLShader* shader );
    void drawThetaLSlice( DGLShader* shader );
    void drawThetaDSlice( DGLShader* shader );
        
    void drawAnglePlot( brdfPackage base );
    
    float zoomLevel[3];
    float lookZoom;
    float centerX;
    float centerY;
    float scaleX;
    float scaleY;
    float fWidth;
    float fHeight;
    float mAspect;
    glm::mat4 projectionMatrix;
    glm::mat4 modelViewMatrix;
    
    float inTheta;
    float inPhi;
    float angleParam;
    
    float NEAR_PLANE;
    float FAR_PLANE;
    float FOV_Y;
    
    float incidentVector[3];
    
    QPoint lastPos;
    
    std::vector<brdfPackage> brdfs;
    
    bool useLogPlot;
    bool useNDotL;
    bool useSampleMult;
    int nSamples;
    int samplingMode;

    int sliceType;

    ///OpenGL resource
    GLuint axisVAO, axisVBO[2];
    int axisNIndices;
    DGLShader* plotShader;
    GLuint textVAO, textVBO, textTexureID;
    DGLShader* textShader;
    GLuint dataLineVAO, dataLineVBO;
    int dataLineNPoints;
};

#endif
