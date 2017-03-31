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

#ifndef _SIMPLE_MODEL_H_
#define _SIMPLE_MODEL_H_

#include <vector>
#include <stdio.h>
#include <math.h>
#include "DGLShader.h"

/*
Very simple class for reading/displaying OBJs. Nothing fancy.
*/

class SimpleModel : GLContext
{

struct float3
{
    float3() : x(0.0), y(0.0), z(0.0) {}
    float3( float ix, float iy, float iz ) : x(ix), y(iy), z(iz) {}

    float length() {
        return (float)sqrt((x*x)+(y*y)+(z*z));  
    }

    float3 operator-( const float3& rhs ) {
        return float3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    float3 getNormalized() {
        float ilen = 1.0 / length();
        return float3( x * ilen, y * ilen, z * ilen );
    }

    float3 cross( const float3& rhs ) {
        return float3(y*rhs.z - z*rhs.y, z*rhs.x - x*rhs.z, x*rhs.y - y*rhs.x);
    }

    float x, y, z;
};

public:
    
    SimpleModel();
    ~SimpleModel();
    
    bool loadOBJ( const char* filename, bool unitize = true );
    bool isLoaded() { return loaded; }

    void drawVBO(DGLShader* shader);

    void clear();

private:

    bool dataPass( FILE* file, std::vector<float3>&, std::vector<float3>& );
    bool facePass( FILE* file, std::vector<float3>&, std::vector<float3>& );
    void unitizeVertices( std::vector<float3>& );
    void createVBO();

    std::vector<float3> vertexData;
    std::vector<float3> normalData;
    float minX, maxX, minY, maxY, minZ, maxZ;
    int numTriangles;
    GLuint vao;
    GLuint vertexBuffer;
    GLuint normalBuffer;
    bool madeVBO;
    bool loaded;
};


#endif
