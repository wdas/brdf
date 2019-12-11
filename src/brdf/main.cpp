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

#include <fstream>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include "MainWindow.h"
#include "ParameterWindow.h"
#include "Paths.h"

#include <iostream>
#include <sstream>
#ifdef WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#define BRDF_VERSION "1.0.0"

bool checkTeapot()
{
    // attempt to open the teapot (to see if the paths are right)
    std::string teapotFilename = getModelsPath() + "teapot.obj";
    std::ifstream in( teapotFilename.c_str() );
    if( in.good() )
        return true;
    return false;
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    setlocale(LC_NUMERIC,"C");

#ifdef WIN32
    // Make sure there is a console to get our stdout,stderr information
    AllocConsole();
    HANDLE stdOutHandle=GetStdHandle(STD_OUTPUT_HANDLE);
    int hConHandle=_open_osfhandle((intptr_t)stdOutHandle,_O_TEXT);
    FILE* fp=_fdopen((int)hConHandle,"w");
    *stdout=*fp;
    setvbuf(stdout,NULL,_IONBF,0);
    HANDLE stdErrHandle=GetStdHandle(STD_ERROR_HANDLE);
    int hConHandleErr=_open_osfhandle((intptr_t) stdErrHandle,_O_TEXT);
    FILE* fperr=_fdopen( (int)hConHandleErr,"w");
    *stderr=*fperr;
    setvbuf(stderr,NULL,_IONBF,0);
    std::ios::sync_with_stdio();
    std::cerr<<"BRDF Version "<<BRDF_VERSION<<std::endl;
//    std::cerr<<"stdout: BRDF Version "<<BRDF_VERSION<<std::endl;
#endif

    // make sure we can open the data files
    if( !checkTeapot() ) {
        QString errString = "Can't open data files.\n\nPlease run BRDF Explorer from the directory containing the data/, images/, probes/, and shaderTemplates/ subdirectories (probably the src/ directory).";
        QMessageBox::critical( NULL, "BRDF Explorer", errString );
        return 1;
    }

    
    // center the window in the middle of the default screen
    QDesktopWidget desktopWidget;
    QRect rect = desktopWidget.screenGeometry();
    rect.adjust( 60, 60, -60, -60 );

    MainWindow main;
    main.setGeometry(rect);
    main.show();
    main.refresh();
    
    
    // open all BRDFs passed in on the commandline
    if( argc > 1 )
    {
        std::vector<std::string> files;
        for( int i = 1; i < argc; i++ )           
            files.push_back( std::string(argv[i]) );
        main.getParameterWindow()->openBRDFFiles( files );
    }
    
    return app.exec();
}
