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

#include <stdio.h>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QFileDialog>
#include <vector>
#include "ParameterWindow.h"
#include "FloatVarWidget.h"
#include "ParameterGroupWidget.h"
#include "BRDFBase.h"



ParameterWindow::ParameterWindow()
				: incidentThetaWidget(NULL),
                  incidentPhiWidget(NULL),
                  cmdLayout(NULL),
			      channelComboBox(NULL),
                  logPlotCheckbox(NULL), nDotLCheckbox(NULL),
                  soloBRDFWidget(NULL),
                  soloBRDFUsesColors(false)
{
	theta = 0.785398163;
	phi = 0.785398163;
	useLogPlot = false;
	useNDotL = false;

	//mainLayout = new QVBoxLayout;
	mainLayout = new QGridLayout;

	createLayout();

    setLayout(mainLayout);

    setWindowTitle( "Parameters" );
    setMaximumWidth( 300 );
}


ParameterWindow::~ParameterWindow()
{
}


void ParameterWindow::createLayout()
{
    int addIndex = 0;
            
    channelComboBox = new QComboBox();
    connect( channelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(emitBRDFListChanged()) );
	channelComboBox->addItem( "Red Channel" );
	channelComboBox->addItem( "Green Channel" );
	channelComboBox->addItem( "Blue Channel" );
	channelComboBox->addItem( "Luminance" );
    channelComboBox->setCurrentIndex( 3 );
    mainLayout->addWidget( channelComboBox, addIndex++, 0, 1, 2 );


    logPlotCheckbox = new QCheckBox( "Log plot:  y = log10( x + 1.0 )" );
    logPlotCheckbox->setChecked( useLogPlot );
    connect( logPlotCheckbox, SIGNAL(stateChanged(int)), this, SLOT(emitGraphParametersChanged()) );
    mainLayout->addWidget( logPlotCheckbox, addIndex++, 0, 1, 2 );
    
    nDotLCheckbox = new QCheckBox( "Multiply by N . L" );
    nDotLCheckbox->setChecked( useNDotL );
    connect( nDotLCheckbox, SIGNAL(stateChanged(int)), this, SLOT(emitGraphParametersChanged()) );
    mainLayout->addWidget( nDotLCheckbox, addIndex++, 0, 1, 2 );
    
    
    incidentThetaWidget = new FloatVarWidget("Incident angle - thetaL", 0, 90, glm::degrees(theta) );
    incidentPhiWidget = new FloatVarWidget("Incident angle - phiL", 0, 360, glm::degrees(phi) );
    connect(incidentThetaWidget, SIGNAL(valueChanged(float)), this, SLOT(emitIncidentDirectionChanged()));
    connect(incidentPhiWidget, SIGNAL(valueChanged(float)), this, SLOT(emitIncidentDirectionChanged()));
    mainLayout->addWidget( incidentThetaWidget, addIndex++, 0, 1, 2 );
    mainLayout->addWidget( incidentPhiWidget, addIndex++, 0, 1, 2 );


    // make the container frame and its layout
    QFrame* cmdFrame = new QFrame;
    cmdFrame->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    cmdLayout = new QVBoxLayout;
    cmdLayout->setMargin( 5 );
    


    cmdFrame->setLayout( cmdLayout );


    // add the scroll area that comprises the parameters list
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable( true );
    scrollArea->setWidget( cmdFrame );
    mainLayout->addWidget( scrollArea, addIndex++, 0, 1, 2 );

    fileDialog = new QFileDialog(this, "Open BRDF File(s)", ".", "BRDF Files (*.brdf *.binary *.dat *.bparam)");
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
}



void ParameterWindow::incidentVectorChanged( float newTheta, float newPhi )
{
    incidentThetaWidget->setValue( glm::degrees(newTheta) );
    incidentPhiWidget->setValue( glm::degrees(newPhi) );
}


void ParameterWindow::openBRDFFromFile()
{
    if (fileDialog->exec()) {
        QStringList fileNames = fileDialog->selectedFiles();
        for (int i = 0, n = fileNames.size(); i < n; i++) {
            openBRDFFile( fileNames[i].toStdString().c_str() );
        }
    }
}



ParameterGroupWidget* ParameterWindow::addBRDFWidget( BRDFBase* b )
{
    ParameterGroupWidget* pgw = new ParameterGroupWidget( this, b );
    connect( pgw,  SIGNAL(removeBRDFButtonPushed( ParameterGroupWidget* )),
             this, SLOT(removeBRDF( ParameterGroupWidget* )) );
    connect( pgw,  SIGNAL(enteringSoloMode(ParameterGroupWidget*,bool)),
             this, SLOT(soloBRDF(ParameterGroupWidget*,bool)) );
    connect( this, SIGNAL(soloModeChanged(ParameterGroupWidget*,bool)),
             pgw,  SLOT(brdfBeingSoloed(ParameterGroupWidget*,bool)) );
    cmdLayout->insertWidget( 0, pgw );


    return pgw;
}



void ParameterWindow::removeBRDF( ParameterGroupWidget* p )
{
    // if we're removing the soloed BRDF, make sure to turn off soloing
    if( p == soloBRDFWidget )
        soloBRDF( NULL, false );


	// look for the parameter group widget and remove it.
	// Note: manually searching like this shouldn't be necessary, but I've 
	// run into all kinds of instability and bizarre behavior trying to make
	// Qt behave here. This works; I'll take it.
	for( int i = 0; i < cmdLayout->count(); i++ )
	{
		// if we found the parameter group, remove it from the layout
		if( cmdLayout->itemAt(i)->widget() == p )
		{
			QLayoutItem* child = cmdLayout->takeAt(i);
			child->widget()->deleteLater();
			delete child;
			break;
		}
	}

	// force a redraw
    emitBRDFListChanged();
}




void ParameterWindow::openBRDFFiles( std::vector<std::string> files )
{
    for( unsigned i = 0; i < files.size(); i++ )
        openBRDFFile( files[i], bool(i == files.size() - 1) );
}


void ParameterWindow::openBRDFFile( std::string filename, bool emitChanges )
{
    BRDFBase* b = createBRDFFromFile( filename );
    
    // silently exit if nothing comes back
    if( !b )
        return;
    
    // add the new widget to the top
    ParameterGroupWidget* pgw = addBRDFWidget( b );
        
    // if a BRDF is being soloed, we want to solo this new one instead
    if( soloBRDFWidget )
        soloBRDF( pgw, soloBRDFUsesColors );

    // soloBRDF will emit the BRDF change, but if we don't call that we need to do it ourselves
    else if( emitChanges )
        emitBRDFListChanged();
}



void ParameterWindow::soloBRDF( ParameterGroupWidget* pgw, bool withColors )
{
    soloBRDFWidget = pgw;
    soloBRDFUsesColors = withColors;

    // update all the parameter group widgets
    emit( soloModeChanged( pgw, withColors ) );

    // redraw
    emitBRDFListChanged();
}




void ParameterWindow::setBRDFColorMask( brdfPackage& pkg )
{
    // set the color mask
    if( channelComboBox )
    {
        if( channelComboBox->currentIndex() == 0 )          // red
            pkg.setColorMask( 1, 0, 0 );
        else if( channelComboBox->currentIndex() == 1 )     // green
            pkg.setColorMask( 0, 1, 0 );
        else if( channelComboBox->currentIndex() == 2 )     // blue
            pkg.setColorMask( 0, 0, 1 );
        else if( channelComboBox->currentIndex() == 3 )     // luminance
            pkg.setColorMask( 0.3, 0.59, 0.11 );
    }
    else pkg.setColorMask( 0.3, 0.59, 0.11 );             // default to luminance
}




void ParameterWindow::emitIncidentDirectionChanged()
{
    // if the parameter controls exist, update the parameter values
    if( incidentThetaWidget )
        theta = glm::radians( incidentThetaWidget->getValue() );
    if( incidentPhiWidget )
        phi = glm::radians( incidentPhiWidget->getValue() );

    emit( incidentDirectionChanged(theta,phi) );
}


void ParameterWindow::emitGraphParametersChanged()
{
    // if the parameter controls exist, update the parameter values
    if( logPlotCheckbox )
        useLogPlot = logPlotCheckbox->isChecked();
    if( nDotLCheckbox )
        useNDotL = nDotLCheckbox->isChecked();

    emit( graphParametersChanged(useLogPlot, useNDotL) );
}


void ParameterWindow::emitBRDFListChanged()
{
    std::vector<brdfPackage> brdfList;

    // if we're soloing a single BRDF, only deal with that one
    if( soloBRDFWidget )
    {
        BRDFBase* brdf = soloBRDFWidget->getUpdatedBRDF();
        if( brdf )
        {
            brdfPackage pkg;
            pkg.dirty = soloBRDFWidget->isDirty();

            // set the BRDF
            pkg.brdf = brdf;

            // if we are we displaying all three channels, 
            if( soloBRDFUsesColors )
            {
                // add the red channel
                pkg.setColorMask( 1, 0, 0 );
                pkg.setDrawColor( 0.65, 0, 0 );
                brdfList.push_back( pkg );

                // add the green channel
                pkg.setColorMask( 0, 1, 0 );
                pkg.setDrawColor( 0, 0.65, 0 );
                brdfList.push_back( pkg );

                // add the blue channel
                pkg.setColorMask( 0, 0, 1 );
                pkg.setDrawColor( 0, 0, 0.65 );
                brdfList.push_back( pkg );
            }

            // otherwise set the draw color and color mask normally
            else
            {
                QColor drawColor = soloBRDFWidget->getDrawColor();
                pkg.setDrawColor( drawColor.redF(), drawColor.greenF(), drawColor.blueF() );
                setBRDFColorMask( pkg );

                brdfList.push_back( pkg );
            }
            
            // no longer dirty, until it dirties itself
            soloBRDFWidget->setDirty( false );
        }
    }

    // otherwise use them all
    else
    {
        brdfList = getBRDFList();
    }
    
    // reset dirty flags on the parameter group widgets
    //resetDirtyFlags();
    
    emit( brdfListChanged(brdfList) );
}




std::vector<brdfPackage> ParameterWindow::getBRDFList()
{
    std::vector<brdfPackage> brdfList;

    // if there's no command layout, there are no parameters to read
    if( !cmdLayout )
        return brdfList;

    // loop through all the BRDFs and stick them on the list if they're enabled
    for( int i = 0; i < cmdLayout->count(); i++ )
    {
        // find out if this is a BRDF parameter group
        ParameterGroupWidget* pgw = dynamic_cast<ParameterGroupWidget*>(cmdLayout->itemAt(i)->widget());
        if( !pgw )
            continue;

        BRDFBase* brdf = pgw->getUpdatedBRDF();
        if( brdf )
        {
            brdfPackage pkg;

            // set the BRDF
            pkg.brdf = brdf;

            // set the draw color
            QColor drawColor = pgw->getDrawColor();
            pkg.setDrawColor( drawColor.redF(), drawColor.greenF(), drawColor.blueF() );

            setBRDFColorMask( pkg );
            
            // set the dirty flag (based on the parameter group widget's dirty flag)
            pkg.dirty = pgw->isDirty();
            pgw->setDirty( false );

            brdfList.push_back( pkg );           
        }
    }

    return brdfList;
}

