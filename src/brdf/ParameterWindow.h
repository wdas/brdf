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

#ifndef PARAM_WINDOW_H
#define PARAM_WINDOW_H

#include <QWidget>
#include <vector>
#include "BRDFBase.h"

class QSlider;
class QLabel;
class QLineEdit;
class QGridLayout;
class QVBoxLayout;
class QCheckBox;
class FloatVarWidget;
class QFrame;
class QComboBox;
class ParameterGroupWidget;
class QFileDialog;



class ParameterWindow : public QWidget
{
    Q_OBJECT

public:
    ParameterWindow();
    ~ParameterWindow();
    
    std::vector<brdfPackage> getBRDFList();

    void openBRDFFile( std::string, bool emitChanges = true );
    void openBRDFFiles( std::vector<std::string> );

signals:
    void redraw( std::vector<brdfPackage>, float theta, float phi, bool logPlot, bool nDotL );
    void soloModeChanged( ParameterGroupWidget*, bool withColors );

    void incidentDirectionChanged( float theta, float phi );
    void graphParametersChanged( bool logPlot, bool nDotL );
    void brdfListChanged( std::vector<brdfPackage> );

public slots:
     
    void openBRDFFromFile();

    void emitIncidentDirectionChanged();
    void emitGraphParametersChanged();
    void emitBRDFListChanged();

private slots:
        
    void incidentVectorChanged( float newTheta, float newPhi );
    void soloBRDF( ParameterGroupWidget*, bool withColors );
    void removeBRDF( ParameterGroupWidget* );

private:

    void createLayout();
    ParameterGroupWidget* addBRDFWidget( BRDFBase* b );
    void setBRDFColorMask( brdfPackage& pkg );
    
    FloatVarWidget* incidentThetaWidget;
    FloatVarWidget* incidentPhiWidget;
    
    QVBoxLayout* cmdLayout;
    QGridLayout* mainLayout;

    QComboBox* channelComboBox;
    QCheckBox* logPlotCheckbox;
    QCheckBox* nDotLCheckbox;
    
    QFrame* spaceFiller;
    
    float theta, phi;
    bool useLogPlot;
    bool useNDotL;

    ParameterGroupWidget* soloBRDFWidget;
    bool soloBRDFUsesColors;

    QFileDialog* fileDialog;
};

#endif
