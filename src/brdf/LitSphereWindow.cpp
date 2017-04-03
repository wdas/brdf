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

#include <QVBoxLayout>
#include <QCheckBox>
#include "LitSphereWindow.h"
#include "LitSphereWidget.h"
#include "ParameterWindow.h"
#include "FloatVarWidget.h"

LitSphereWindow::LitSphereWindow( ParameterWindow* paramWindow )
{
    glWidget = new LitSphereWidget( this->windowHandle(), paramWindow->getBRDFList() );

    // so we can tell the parameter window when the incident vector changes (from dragging on the sphere)
    connect( glWidget, SIGNAL(incidentVectorChanged( float, float )), paramWindow, SLOT(incidentVectorChanged( float, float )) );

    connect( paramWindow, SIGNAL(incidentDirectionChanged(float,float)), glWidget, SLOT(incidentDirectionChanged(float,float)) );
    connect( paramWindow, SIGNAL(brdfListChanged(std::vector<brdfPackage>)), glWidget, SLOT(brdfListChanged(std::vector<brdfPackage>)) );

        
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(QWidget::createWindowContainer(glWidget));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    doubleTheta = new QCheckBox( "Double theta" );
    doubleTheta->setChecked( true );
    connect( doubleTheta, SIGNAL(stateChanged(int)), glWidget, SLOT(doubleThetaChanged(int)) );
    buttonLayout->addWidget(doubleTheta);

    useNDotL = new QCheckBox( "Multiply by N . L" );
    useNDotL->setChecked( true );
    connect( useNDotL, SIGNAL(stateChanged(int)), glWidget, SLOT(useNDotLChanged(int)) );
    buttonLayout->addWidget(useNDotL);

    FloatVarWidget* fv;
        
#if 0
    fv = new FloatVarWidget("Brightness", 0, 100.0, 1.0);
    connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(brightnessChanged(float)));
    mainLayout->addWidget(fv);
#endif
    
    fv = new FloatVarWidget("Gamma", 1.0, 5.0, 2.2);
    connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(gammaChanged(float)));
    mainLayout->addWidget(fv);
    
    fv = new FloatVarWidget("Exposure", -6.0, 6.0, 0.0);
    connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(exposureChanged(float)));
    mainLayout->addWidget(fv);


    setLayout(mainLayout);
    
    setWindowTitle( "Lit Sphere" );
}



void LitSphereWindow::setShowing( bool s )
{
    if( glWidget )
        glWidget->setShowing( s );
}
