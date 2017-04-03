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

#ifndef IMAGESLICEWIDGET_H
#define IMAGESLICEWIDGET_H

#include "BRDFBase.h"
#include "SharedContextGLWidget.h"
#include "Quad.h"

class DGLShader;

class ImageSliceWidget : public GLWindow
{
    Q_OBJECT

public:
    ImageSliceWidget( QWindow *parent, std::vector<brdfPackage> );
    ~ImageSliceWidget();
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

private slots:
    void incidentDirectionChanged( float theta, float phi );
    void brdfListChanged( std::vector<brdfPackage> );

    void phiDChanged( float );
    void brightnessChanged( float );
    void gammaChanged( float );
    void exposureChanged( float );
    void useThetaHSquaredChanged( int );
    void showChromaChanged( int );

protected:
    void initializeGL();
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:

    void drawThetaHSlice( DGLShader* shader );
    void drawThetaLSlice( DGLShader* shader );
    void drawThetaDSlice( DGLShader* shader );
        
    void drawImageSlice();
    
    float lookZoom;
    float centerX;
    float centerY;
    float scaleX;
    float scaleY;

    float phiD;

    float brightness;
    float gamma;
    float exposure;
    
    float inTheta;
    float inPhi;
    
    float NEAR_PLANE;
    float FAR_PLANE;
    float FOV_Y;
    
    float incidentVector[3];
    
    QPoint lastPos;
    
    std::vector<brdfPackage> brdfs;
    
    bool useThetaHSquared;
    bool showChroma;

    glm::mat4 projectionMatrix;

    Quad* quad;
    DGLShader* plotShader;
    GLuint vao;
    GLuint vbo[2];
};

#endif
