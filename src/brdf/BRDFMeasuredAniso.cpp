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

/*
Anisotropic Data Files (http://people.csail.mit.edu/addy/research/brdf/)

For standard parameterization, dim[0..3] correspond to [ theta_in, theta_out, phi_diff, phi_in ].
Range: theta_in, theta_out - [0..pi/2],
  phi_in [0..2pi],
  phi_diff - [0..pi] if half_data==1, [0..2pi] if half_data==0.

Bins without (reliable) measurements are filled with negative values
and should be ignored when using the data.

Header format:
Index
0	    dim[0]	Number of bins for the 0th dimension
1	    dim[1]	Number of bins for the 1st dimension
2	    dim[2]	Number of bins for the 2nd dimension
3	    dim[3]	Number of bins for the 3rd dimension
4	    reserved
5	    reserved
6	    paramType	0 - reserved, 1 - Standard Parameterization
7	    binType	0 - Uniform binning, 1 - reserved
8	    reserved
9	    half_data	0 - phi_diff range [0..2pi], 1 - phi_diff range [0..pi]
10	  num_channels	always 3 (RGB)
11	  reserved
12	  reserved
13	  reserved
14	  reserved
15	  reserved
*/

#include <cstdlib>
#include <string>
#include <fstream>
#include <string.h>
#include "BRDFMeasuredAniso.h"
#include "DGLShader.h"
#include "Paths.h"

BRDFMeasuredAniso::BRDFMeasuredAniso() :brdfData(NULL) {
    std::string path = getShaderTemplatesPath() + "measuredAniso.func";

    //read the shader
    std::ifstream ifs( path.c_str() );
    std::string temp;
    while( getline( ifs, temp ) ) brdfFunction += temp + "\n";
}

BRDFMeasuredAniso::~BRDFMeasuredAniso() {
    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glf->glDeleteBuffers(1, &tbo);
}

bool BRDFMeasuredAniso::loadAnisoData(const char *filename) {
    //BRDF name is filename
    name = std::string(filename);

    //read in Anisotropic BRDF data header
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    int header[16];
    if (fread(header, sizeof(int), 16, f) != 16) {
        fprintf(stderr, "read error\n");
        fclose(f);
        return false;
    }
    numBRDFSamples = header[0] * header[1] * header[2] * header[3];
    int nchannels = header[10];
    for (int i=0; i<16; i++) printf("%d \n", header[i]);
    fflush(stdout);

    //read data
    brdfData = (float*) malloc(sizeof(float)*3*numBRDFSamples);
    if (fread(brdfData, sizeof(float), nchannels*numBRDFSamples, f) != (unsigned)nchannels*numBRDFSamples) {
        fprintf(stderr, "read error\n");
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

void BRDFMeasuredAniso::initGL() {
    if( initializedGL ) return;

    //create buffer object
    glf->glGenBuffers(1, &tbo);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);

    //initialize buffer object
    unsigned int numBytes = numBRDFSamples*3*sizeof(float)/2;
    glf->glBufferData( GL_TEXTURE_BUFFER, numBytes, 0, GL_STATIC_DRAW );

    //tex
    glf->glGenTextures(1, &tex);
    glf->glBindTexture(GL_TEXTURE_BUFFER, tex);
    glf->glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, tbo);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, 0);

    glf->glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    float* p = (float*)glf->glMapBuffer( GL_TEXTURE_BUFFER, GL_WRITE_ONLY );

    float *halfdata = new float[numBytes];
    for (int i=0; i<numBRDFSamples*3; i++)
        if(i % 2 == 0) halfdata[i/2]= (brdfData[i] + brdfData[i+1])/2.0;

    memcpy( p, halfdata, numBytes );
    glf->glUnmapBuffer(GL_TEXTURE_BUFFER);
    glf->glBindBuffer(GL_TEXTURE_BUFFER, 0);


    delete[] halfdata;
    delete[] brdfData;
    brdfData = NULL;
    initializedGL = true;
}

void BRDFMeasuredAniso::adjustShaderPreRender(DGLShader *shader) {
    shader->setUniformTexture( "measuredDataAniso", tex, GL_TEXTURE_BUFFER );
    BRDFBase::adjustShaderPreRender( shader );
}
