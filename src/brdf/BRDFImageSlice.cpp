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

#include <string>
#include <fstream>
#include "BRDFImageSlice.h"
#include "DGLShader.h"
#include "Paths.h"


#define BRDF_SAMPLING_RES_THETA_H       90
#define BRDF_SAMPLING_RES_THETA_D       90
#define BRDF_SAMPLING_RES_PHI_D         360



BRDFImageSlice::BRDFImageSlice()
    : imageSliceData(NULL), width(-1), height(-1)
{
    std::string path = (getShaderTemplatesPath() + "imageSlice.func");
 
    // read the shader
    std::ifstream ifs( path.c_str() );
    std::string temp;
    while( getline( ifs, temp ) )
        brdfFunction += (temp + "\n");
}



BRDFImageSlice::~BRDFImageSlice()
{
    printf( "glDeleteTextures( 1, &texID );\n" );
    glf->glDeleteTextures( 1, &texID );
}


std::string BRDFImageSlice::getBRDFFunction()
{
    return brdfFunction;
}



bool BRDFImageSlice::loadImage( const char* filename )
{
    // the BRDF's name is just the filename
    name = std::string(filename);

    width = height = 90;
    imageSliceData = new float[width*height*3];

    for( int i = 0; i < width*height; i++ )
    {
        imageSliceData[i*3+0] = 1.0;
        imageSliceData[i*3+1] = 0.2;
        imageSliceData[i*3+2] = 0.6;
    }

    return true;
}


void BRDFImageSlice::initGL()
{
    if( initializedGL )
        return;
    

    // create the texture id
    glf->glGenTextures( 1, &texID );
    glf->glBindTexture( GL_TEXTURE_2D, texID );

    // turn on linear filtering
    glf->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glf->glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    // load the data
    glf->glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, imageSliceData );
    glf->glBindTexture( GL_TEXTURE_2D, 0 );

    // don't need to keep it anymore
    delete[] imageSliceData;
    imageSliceData = NULL;
    
    initializedGL = true;
}


void BRDFImageSlice::adjustShaderPreRender( DGLShader* shader )
{
    shader->setUniformTexture( "imageSliceData", texID );

    BRDFBase::adjustShaderPreRender( shader );
}


void BRDFImageSlice::adjustShaderPostRender( DGLShader* )
{
    //if( shader )
        //shader->DisableTexture( GL_TEXTURE0 );
}
