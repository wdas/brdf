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
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "PlotCartesianWindow.h"
#include "PlotCartesianWidget.h"
#include "ParameterWindow.h"
#include "FloatVarWidget.h"

PlotCartesianWindow::PlotCartesianWindow( ParameterWindow* pWindow, int type ) : lockCheckBox(NULL)
{
    paramWindow = pWindow;
    sliceType = type;
    resampleButton = NULL;

    glWidget = new PlotCartesianWidget( this->windowHandle(), pWindow->getBRDFList(), sliceType );
    connect( pWindow, SIGNAL(incidentDirectionChanged(float,float)), this, SLOT(incidentDirectionChanged(float,float)) );
    connect( pWindow, SIGNAL(graphParametersChanged(bool,bool)), glWidget, SLOT(graphParametersChanged(bool,bool)) );
    connect( pWindow, SIGNAL(brdfListChanged(std::vector<brdfPackage>)), glWidget, SLOT(brdfListChanged(std::vector<brdfPackage>)) );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(QWidget::createWindowContainer(glWidget));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    switch (sliceType) {
      case THETA_V_PLOT :
        sliceParamWidget = new FloatVarWidget("PhiV", 0, 360, 45 );
        connect( sliceParamWidget, SIGNAL(valueChanged(float)), this, SLOT(angleParamChanged()) );
        buttonLayout->addWidget(sliceParamWidget);
        lockCheckBox = new QCheckBox( "Lock" );
        lockCheckBox->setChecked( true );
        connect( lockCheckBox, SIGNAL(clicked()),
                this,          SLOT(lockChanged()) );
        buttonLayout->addWidget( lockCheckBox );

        sliceParamWidget->setEnabled( false ); break;
      case THETA_H_PLOT :
        sliceParamWidget = new FloatVarWidget("ThetaD", 0, 90, 0 );
        connect( sliceParamWidget, SIGNAL(valueChanged(float)), this, SLOT(angleParamChanged()) );
        buttonLayout->addWidget(sliceParamWidget); break;
      case THETA_D_PLOT :
        sliceParamWidget = new FloatVarWidget("ThetaH", 0, 90, 0 );
        connect( sliceParamWidget, SIGNAL(valueChanged(float)), this, SLOT(angleParamChanged()) );
        buttonLayout->addWidget(sliceParamWidget); break;
      case ALBEDO_PLOT :
        char albedolabel[128];
        sprintf(albedolabel, "Number of Samples");
        sliceParamWidget = new FloatVarWidget(albedolabel, 1000, 10000, 2500);
        glWidget->setNSamples( sliceParamWidget->getValue() );
        connect( sliceParamWidget, SIGNAL(valueChanged(float)), this, SLOT(angleParamChanged()) );
        buttonLayout->addWidget(sliceParamWidget);
        resampleButton = new QPushButton("Resample x10");
        connect( resampleButton, SIGNAL(clicked()), this, SLOT(resamplePushed()) );
        buttonLayout->addWidget(resampleButton);
        sliceCombo = new QComboBox();
        sliceCombo->addItem("Cosine Sampling");
        sliceCombo->addItem("Uniform Sampling");
        sliceCombo->addItem("Polar Sampling");
        sliceCombo->addItem("BlinnPhong Sampling");
        sliceCombo->addItem("M.I. Sampling");
        sliceCombo->setCurrentIndex(MISAMPLING);
        connect(sliceCombo, SIGNAL(activated(int)), glWidget, SLOT(samplingModeChanged(int)));
        glWidget->samplingModeChanged(MISAMPLING);
        buttonLayout->addWidget(sliceCombo);
        break;
      default :
        sliceParamWidget = NULL;
    }
    setLayout(mainLayout);   
    setWindowTitle( "Lit Sphere" );
}



void PlotCartesianWindow::angleParamChanged()
{
    if( glWidget && sliceParamWidget) {
      glWidget->setNSamples( (int)sliceParamWidget->getValue() );
      glWidget->setAngleParam( glm::radians( sliceParamWidget->getValue() ) );
      glWidget->updateGL();
    }
}

void PlotCartesianWindow::resamplePushed()
{
  if( glWidget && sliceParamWidget)
  {
    glWidget->setAngleParam( glm::radians(sliceParamWidget->getValue()) );
    glWidget->resamplePushed();
  }
}


void PlotCartesianWindow::lockChanged()
{
    if( !glWidget || !sliceParamWidget)
      return;
    
    sliceParamWidget->setEnabled( !lockCheckBox->isChecked() );

    // is the slider being re-locked?
    if( lockCheckBox->isChecked() )
    {
        paramWindow->emitIncidentDirectionChanged();
    } 
}


void PlotCartesianWindow::incidentDirectionChanged( float incidentTheta, float incidentPhi )
{
    // don't want to mess around with null pointers
    if( !glWidget)
        return;

    float angleParameter = 0.0;

    if( sliceParamWidget )
    {
      if( sliceType == THETA_V_PLOT )
          angleParameter = lockCheckBox->isChecked() ? incidentPhi : glm::radians( sliceParamWidget->getValue() );
      else
          angleParameter = glm::radians( sliceParamWidget->getValue() );
    }

    glWidget->setAngles( incidentTheta, incidentPhi, angleParameter );

    // update the angle parameter
    if( lockCheckBox && lockCheckBox->isChecked() && sliceType == THETA_V_PLOT )
        sliceParamWidget->setValue( glm::degrees(angleParameter) );
}

void PlotCartesianWindow::resizeEvent(QResizeEvent * event)
{
    Q_UNUSED(event)
    glWidget->updateGL();
}

void PlotCartesianWindow::setShowing( bool s )
{
    if( glWidget )
        glWidget->setShowing( s );
}

