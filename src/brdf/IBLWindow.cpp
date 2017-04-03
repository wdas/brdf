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
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include "IBLWindow.h"
#include "IBLWidget.h"
#include "ParameterWindow.h"
#include "FloatVarWidget.h"
#include "Paths.h"

IBLWindow::IBLWindow( ParameterWindow* paramWindow )
{
    glWidget = new IBLWidget( this, paramWindow->getBRDFList() );

    connect( paramWindow, SIGNAL(incidentDirectionChanged(float,float)), glWidget, SLOT(incidentDirectionChanged(float,float)) );
    connect( paramWindow, SIGNAL(brdfListChanged(std::vector<brdfPackage>)), glWidget, SLOT(brdfListChanged(std::vector<brdfPackage>)) );

    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(QWidget::createWindowContainer(glWidget));


    FloatVarWidget* fv;
        
#if 0
        fv = new FloatVarWidget("Brightness", 0, 100.0, 1.0);
        connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(brightnessChanged(float)));
        mainLayout->addWidget(fv);
#endif

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    
    iblCombo = new QComboBox();
    iblCombo->setMinimumWidth( 100 );
    connect( iblCombo, SIGNAL(activated(int)), glWidget, SLOT(renderingModeChanged(int)) );
    buttonLayout->addWidget( iblCombo );
    

    QCheckBox* keepAddingSamplesCheckbox = new QCheckBox( "Keep Sampling" );
    keepAddingSamplesCheckbox->setChecked(true);
    connect( keepAddingSamplesCheckbox, SIGNAL(stateChanged(int)), glWidget, SLOT(keepAddingSamplesChanged(int)) );
    buttonLayout->addWidget(keepAddingSamplesCheckbox);
    
    
    // add the change probe button
    QPushButton* probeButton = new QPushButton();
    QPixmap* probePixmap = new QPixmap( (getImagesPath() + "imageSmall.png").c_str() );
    probeButton->setIconSize( QSize(probePixmap->width(), probePixmap->height()) );
    probeButton->setIcon( QIcon(*probePixmap) );
    probeButton->setFixedWidth( 30 );
    probeButton->setFixedHeight( 24 );
    probeButton->setToolTip( "Switch Environment Probe" );
    connect( probeButton, SIGNAL(clicked()), this, SLOT(loadIBLButtonClicked()) );
    buttonLayout->addWidget( probeButton );
    
    
    // add the change model button
    QPushButton* modelButton = new QPushButton();
    QPixmap* modelPixmap = new QPixmap( (getImagesPath() + "modelSmall.png").c_str() );
    modelButton->setIconSize( QSize(probePixmap->width(), modelPixmap->height()) );
    modelButton->setIcon( QIcon(*modelPixmap) );
    modelButton->setFixedWidth( 30 );
    modelButton->setFixedHeight( 24 );
    modelButton->setToolTip( "Switch Model" );
    connect( modelButton, SIGNAL(clicked()), this, SLOT(loadModelButtonClicked()) );
    buttonLayout->addWidget( modelButton );
    
    
    fv = new FloatVarWidget("Gamma", 1.0, 5.0, 2.2);
    connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(gammaChanged(float)));
    mainLayout->addWidget(fv);
    
    fv = new FloatVarWidget("Exposure", -12.0, 12.0, 0.0);
    connect(fv, SIGNAL(valueChanged(float)), glWidget, SLOT(exposureChanged(float)));
    mainLayout->addWidget(fv);


    setLayout(mainLayout);
    
    setWindowTitle( "Lit Sphere" );

    probeFileDialog =  new QFileDialog(this, "Open Cube Map", "", "Ptex Cube Maps (*.penv *.ptex *.ptx)");
    probeFileDialog->setFileMode(QFileDialog::ExistingFile);

    modelFileDialog =  new QFileDialog(this, "Open Model", "", "OBJ files (*.obj)");
    modelFileDialog->setFileMode(QFileDialog::ExistingFile);
}



void IBLWindow::loadIBLButtonClicked()
{
    if( !glWidget )
        return;

    if (probeFileDialog->exec()) {
        QStringList fileNames = probeFileDialog->selectedFiles();
        if (fileNames.size() > 0)
            glWidget->loadIBL( fileNames[0].toStdString().c_str() );
        glWidget->redrawAll();
    }
}



void IBLWindow::loadModelButtonClicked()
{
    if( !glWidget )
        return;

    if (modelFileDialog->exec()) {
        QStringList fileNames = modelFileDialog->selectedFiles();
        if (fileNames.size() > 0)
            glWidget->loadModel( fileNames[0].toStdString().c_str() );
        glWidget->redrawAll();
    }
}


void IBLWindow::renderingModeReset( bool hasBRDFIS )
{      
    int prevCurIndex = iblCombo->currentIndex();
    
    iblCombo->clear();
    iblCombo->addItem( "No IBL" );
    iblCombo->addItem( "IBL: No IS" );
    iblCombo->addItem( "IBL: IBL IS" );
    
    if( hasBRDFIS )
    {
        iblCombo->addItem( "IBL: BRDF IS" );
        iblCombo->addItem( "IBL: MIS" );
    }
   
    // make sure the previous mode is available with the new brdf
    if( prevCurIndex >= 0 && prevCurIndex < iblCombo->count() )
    {
        iblCombo->setCurrentIndex( prevCurIndex );
    }
    else
    {
        // if it's not, set the mode to regular IBL sampling
        iblCombo->setCurrentIndex( RENDER_REGULAR_SAMPLING );
        glWidget->renderingModeChanged( RENDER_REGULAR_SAMPLING );
    }
}


void IBLWindow::setShowing( bool s )
{
    if( glWidget ){
        glWidget->setShowing( s );
        if(s)
            glWidget->updateGL();
    }
}

