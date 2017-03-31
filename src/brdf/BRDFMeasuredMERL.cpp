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

#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#endif


#include <cstdlib>
#include <string>
#include <fstream>
#include <string.h>
#include "BRDFMeasuredMERL.h"
#include "DGLShader.h"
#include "Paths.h"

#define BRDF_SAMPLING_RES_THETA_H       90
#define BRDF_SAMPLING_RES_THETA_D       90
#define BRDF_SAMPLING_RES_PHI_D         360



BRDFMeasuredMERL::BRDFMeasuredMERL()
                 : brdfData(NULL)
{
    std::string path = getShaderTemplatesPath() + "measured.func";

    // read the shader
    std::ifstream ifs( path.c_str() );
    std::string temp;
    while( getline( ifs, temp ) )
        brdfFunction += (temp + "\n");
    addFloatParameter("inTheta", 0,M_PI_2, M_PI_4);
    addBoolParameter("convolution", false);
    addBoolParameter("swapVL", false);
    addBoolParameter("LinearFiltering", false);
}



BRDFMeasuredMERL::~BRDFMeasuredMERL()
{
    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glf->glDeleteBuffers( 1, &tbo);
}


std::string BRDFMeasuredMERL::getBRDFFunction()
{
    return brdfFunction;
}


bool BRDFMeasuredMERL::loadMERLData( const char* filename )
{
    // the BRDF's name is just the filename
    name = std::string(filename);

    // read in the MERL BRDF data
    FILE *f = fopen(filename, "rb");
    if (!f)
            return false;

    int dims[3];
    if (fread(dims, sizeof(int), 3, f) != 3) {
        fprintf(stderr, "read error\n");
        fclose(f);
        return false;
    }
    numBRDFSamples = dims[0] * dims[1] * dims[2];
    if (numBRDFSamples != BRDF_SAMPLING_RES_THETA_H *
                            BRDF_SAMPLING_RES_THETA_D *
                            BRDF_SAMPLING_RES_PHI_D / 2)
    {
        fprintf(stderr, "Dimensions don't match\n");
        fclose(f);
        return false;
    }

    // read the data
    double* brdf = (double*) malloc (sizeof(double)*3*numBRDFSamples);
    if (fread(brdf, sizeof(double), 3*numBRDFSamples, f) != size_t(3*numBRDFSamples)) {
        fprintf(stderr, "read error\n");
        fclose(f);
    return false;
    }
    fclose(f);

    // now transform it to RGBA floats
    brdfData = new float[ numBRDFSamples * 3 ];
    for( int i = 0; i < numBRDFSamples; i++ )
    {
            brdfData[i*3 + 0] = brdf[i*3 + 0];
            brdfData[i*3 + 1] = brdf[i*3 + 1];
            brdfData[i*3 + 2] = brdf[i*3 + 2];
    }

    // now we can dump the old data
    free( brdf );

    return true;
}


void BRDFMeasuredMERL::initGL()
{
    if( initializedGL )
        return;

    // create buffer object
    glf->glGenBuffers(1, &tbo);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);

    // initialize buffer object
    unsigned int numBytes = numBRDFSamples * 3 * sizeof(float);
    //printf( "size = %d bytes (%f megs)\n", numBytes, float(numBytes) / 1048576.0f );
    glf->glBufferData( GL_TEXTURE_BUFFER, numBytes, 0, GL_STATIC_DRAW );

    //tex
    glf->glGenTextures(1, &tex);
    glf->glBindTexture(GL_TEXTURE_BUFFER, tex);
    glf->glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, tbo);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, 0);

    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    float* p = (float*)glf->glMapBuffer( GL_TEXTURE_BUFFER, GL_WRITE_ONLY );

    memcpy( p, brdfData, numBytes );
    glf->glUnmapBuffer(GL_TEXTURE_BUFFER);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, 0);

    delete[] brdfData;
    brdfData = NULL;

    initializedGL = true;
}


void BRDFMeasuredMERL::adjustShaderPreRender( DGLShader* shader )
{
    shader->setUniformTexture( "measuredData", tex, GL_TEXTURE_BUFFER );

    BRDFBase::adjustShaderPreRender( shader );
}


