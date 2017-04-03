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

#include <QMenuBar>
#include <QMessageBox>
#include "MainWindow.h"
#include "ParameterWindow.h"
#include "PlotCartesianWidget.h"
#include "Plot3DWidget.h"
#include "PlotPolarWidget.h"
#include "LitSphereWindow.h"
#include "PlotCartesianWindow.h"
#include "ImageSliceWindow.h"
#include "IBLWindow.h"
#include "ShowingDockWidget.h"
#include "ViewerWindow.h"



MainWindow::MainWindow()
{
    setWindowTitle( "BRDF Explorer" );

    // create the parameter window
    paramWnd = new ParameterWindow();

    viewer3D = new Plot3DWidget( this->windowHandle(), paramWnd->getBRDFList() );
    connect( paramWnd, SIGNAL(incidentDirectionChanged(float,float)), viewer3D, SLOT(incidentDirectionChanged(float,float)) );
    connect( paramWnd, SIGNAL(graphParametersChanged(bool,bool)), viewer3D, SLOT(graphParametersChanged(bool,bool)) );
    connect( paramWnd, SIGNAL(brdfListChanged(std::vector<brdfPackage>)), viewer3D, SLOT(brdfListChanged(std::vector<brdfPackage>)) );
    plot3D = new ViewerWindow( viewer3D );

    cartesianThetaV = new PlotCartesianWindow( paramWnd, THETA_V_PLOT );
    cartesianThetaH = new PlotCartesianWindow( paramWnd, THETA_H_PLOT );
    cartesianThetaD = new PlotCartesianWindow( paramWnd, THETA_D_PLOT );
    cartesianAlbedo = new PlotCartesianWindow( paramWnd, ALBEDO_PLOT );

    viewer2D = new PlotPolarWidget( this->windowHandle(), paramWnd->getBRDFList() );
    connect( paramWnd, SIGNAL(incidentDirectionChanged(float,float)), viewer2D, SLOT(incidentDirectionChanged(float,float)) );
    connect( paramWnd, SIGNAL(graphParametersChanged(bool,bool)), viewer2D, SLOT(graphParametersChanged(bool,bool)) );
    connect( paramWnd, SIGNAL(brdfListChanged(std::vector<brdfPackage>)), viewer2D, SLOT(brdfListChanged(std::vector<brdfPackage>)) );
    polarPlot = new ViewerWindow(viewer2D);

    viewerSphere = new LitSphereWindow( paramWnd );

    ibl = new IBLWindow( paramWnd );

    imageSlice = new ImageSliceWindow( paramWnd );

    
    
    
    ShowingDockWidget* Plot3DWidget = new ShowingDockWidget("3D Plot", this);
    Plot3DWidget->setWidget( plot3D );
    addDockWidget( Qt::RightDockWidgetArea, Plot3DWidget );
    
    
    

    ShowingDockWidget* paramsWidget = new ShowingDockWidget(tr("BRDF Parameters"), this);
    paramsWidget->setWidget( paramWnd );
    addDockWidget( Qt::LeftDockWidgetArea, paramsWidget );

    ShowingDockWidget* PlotPolarWidget = new ShowingDockWidget(tr("Polar Plot"), this);
    PlotPolarWidget->setWidget( polarPlot );
    addDockWidget( Qt::RightDockWidgetArea, PlotPolarWidget );

    ShowingDockWidget* thetaVWidget = new ShowingDockWidget(tr("Theta V"), this);
    thetaVWidget->setWidget( cartesianThetaV );
    addDockWidget( Qt::RightDockWidgetArea, thetaVWidget );

    ShowingDockWidget* thetaHWidget = new ShowingDockWidget(tr("Theta H"), this);
    thetaHWidget->setWidget( cartesianThetaH );
    addDockWidget( Qt::RightDockWidgetArea, thetaHWidget );

    ShowingDockWidget* thetaDWidget = new ShowingDockWidget(tr("Theta D"), this);
    thetaDWidget->setWidget( cartesianThetaD );
    addDockWidget( Qt::RightDockWidgetArea, thetaDWidget );

    ShowingDockWidget* albedoWidget = new ShowingDockWidget(tr("Albedo"), this);
    albedoWidget->setWidget( cartesianAlbedo );
    addDockWidget( Qt::RightDockWidgetArea, albedoWidget );

    tabifyDockWidget( Plot3DWidget, albedoWidget);
    tabifyDockWidget( albedoWidget, thetaHWidget);
    tabifyDockWidget( thetaHWidget, thetaDWidget );
    tabifyDockWidget( thetaDWidget, thetaVWidget );
    tabifyDockWidget( thetaVWidget, PlotPolarWidget );

    ShowingDockWidget* litSphereWidget = new ShowingDockWidget(tr("Lit Sphere"), this);
    litSphereWidget->setWidget( viewerSphere );
    addDockWidget( Qt::RightDockWidgetArea, litSphereWidget );


    ShowingDockWidget* imageSliceWidget = new ShowingDockWidget(tr("Image Slice"), this);
    imageSliceWidget->setWidget( imageSlice );
    addDockWidget( Qt::RightDockWidgetArea, imageSliceWidget );

    ShowingDockWidget* iblWidget = new ShowingDockWidget(tr("Lit Object"), this);
    iblWidget->setWidget( ibl );
    addDockWidget( Qt::RightDockWidgetArea, iblWidget );


    tabifyDockWidget( imageSliceWidget, iblWidget );
    tabifyDockWidget( iblWidget, litSphereWidget );
    //tabifyDockWidget( litSphereWidget, imageSliceWidget );


    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    setCentralWidget( new QWidget() ); // dummy central widget
    centralWidget()->hide();

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* openBRDF = fileMenu->addAction( "Open BRDF..." );
    openBRDF->setShortcut( QKeySequence("Ctrl+O") );
    connect( openBRDF, SIGNAL(triggered()), paramWnd, SLOT(openBRDFFromFile()) );
    fileMenu->addAction( "&Quit", this, SLOT(close()), QKeySequence("Ctrl+Q") );

    QMenu* utilMenu = menuBar()->addMenu(tr("&Utilities"));
    QAction* reloadAuxShaders = utilMenu->addAction( "Reload Auxiliary Shaders" );
    connect( reloadAuxShaders, SIGNAL(triggered()), ibl->getWidget(), SLOT(reloadAuxShaders()) );

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* helpAbout = helpMenu->addAction( "About..." );
    connect( helpAbout, SIGNAL(triggered()), this, SLOT(about()) );

    // make sure everything has the correct incident direction param values at the start
    paramWnd->emitIncidentDirectionChanged();
}


MainWindow::~MainWindow()
{
}

void MainWindow::refresh()
{
    viewer2D->updateGL();
    viewer3D->updateGL();
    viewerSphere->getWidget()->updateGL();
    cartesianAlbedo->getWidget()->updateGL();
    cartesianThetaD->getWidget()->updateGL();
    cartesianThetaH->getWidget()->updateGL();
    cartesianThetaV->getWidget()->updateGL();
    imageSlice->getWidget()->updateGL();
    ibl->getWidget()->updateGL();
}

void MainWindow::about()
{
    QString copyright = "Copyright Disney Enterprises, Inc. All rights reserved.";
    QMessageBox::about( this, "BRDF Explorer", copyright );

}
