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

#include <QElapsedTimer>

#include <iostream>
#include <string.h>
#include <float.h>
#include <algorithm>
#include "SimpleModel.h"


const int MAX_VERTICES_PER_FACE = 255;


SimpleModel::SimpleModel()
{
    madeVBO = false;
    loaded = false;
}


SimpleModel::~SimpleModel()
{
    clear();
}


void SimpleModel::clear()
{
    vertexData.clear();
    normalData.clear();
    loaded = false;
    maxX = maxY = maxZ = -FLT_MAX;
    minX = minY = minZ = FLT_MAX;
    numTriangles = false;

    // delete the VBO if it exists
    if( madeVBO )
    {
        if( normalBuffer )
            glf->glDeleteBuffers( 1, &normalBuffer );
        if( vertexBuffer )
            glf->glDeleteBuffers( 1, &vertexBuffer );
    }

    vertexBuffer = normalBuffer = 0;
    madeVBO = false;
}


bool SimpleModel::loadOBJ( const char* filename, bool unitize )
{
    clear();

    QElapsedTimer timer;
    timer.start();

    FILE* file = fopen( filename, "r" );
    if( !file )
        return false;

    std::vector<float3> vertices;
    std::vector<float3> normals;

    if( !dataPass( file, vertices, normals ) ) {
        fclose(file);
        return false;
    }

    if( unitize )
        unitizeVertices( vertices );

    rewind( file );
    if( !facePass( file, vertices, normals ) ) {
        fclose(file);
        return false;
    }

    fclose(file);
    loaded = true;

    qint64 loading_time = timer.elapsed();
    std::cout << " Time to load OBJ: " << loading_time << " ms" << std::endl;

    return true;
}


bool SimpleModel::dataPass( FILE* file, std::vector<float3>& vertices, std::vector<float3>& normals )
{
    maxX = maxY = maxZ = -FLT_MAX;
    minX = minY = minZ = FLT_MAX;

    vertices.clear();
    normals.clear();
    numTriangles = 0;

    char line[2048];
    while (fgets(line, sizeof(line), file))
    {
        char* end = &line[strlen(line)-1];
        if (*end == '\n') *end = '\0'; // strip trailing nl

        // vertices
        if( line[0] == 'v' && line[1] == ' ' ) {
            float3 data;
            if( sscanf(line, "v %f %f %f", &data.x, &data.y, &data.z) == 3 ) {
                maxX = std::max<float>( maxX, data.x );
                minX = std::min<float>( minX, data.x );
                maxY = std::max<float>( maxY, data.y );
                minY = std::min<float>( minY, data.y );
                maxZ = std::max<float>( maxZ, data.z );
                minZ = std::min<float>( minZ, data.z );
                vertices.push_back( data );
            }
        }

        // vertex normals
        else if( line[0] == 'v' && line[1] == 'n' ) {
            float3 data;            
            if( sscanf(line, "vn %f %f %f", &data.x, &data.y, &data.z) == 3 ) {
                normals.push_back( data );
            }
        }
    }

    return true;
}



bool SimpleModel::facePass( FILE* file, std::vector<float3>& vertices, std::vector<float3>& normals )
{
    vertexData.clear();
    normalData.clear();
    numTriangles = 0;

    char line[2048];
    while (fgets(line, sizeof(line), file))
    {
        char* end = &line[strlen(line)-1];
        if (*end == '\n') *end = '\0'; // strip trailing nl

        // face
        if( line[0] == 'f' && line[1] == ' ' ) {
            
            int numVerts = 0;
            int numNorms = 0;
            int normIndices[MAX_VERTICES_PER_FACE];
            int vertIndices[MAX_VERTICES_PER_FACE];

            int vertidx,texidx,normidx;
            const char* cp = &line[2];
            while (*cp == ' ') cp++;
            while (sscanf(cp, "%d//%d", &vertidx, &normidx) == 2) {
                if( numVerts >= MAX_VERTICES_PER_FACE || vertidx > (int)vertices.size() - 0 ||
                    numNorms >= MAX_VERTICES_PER_FACE || normidx > (int)normals.size() - 0 )
                    return false;
                vertIndices[numVerts++] = vertidx;
                normIndices[numNorms++] = normidx;
                while (*cp && *cp != ' ') cp++;
                while (*cp == ' ') cp++;
            }
            while (sscanf(cp, "%d/%d/%d", &vertidx, &texidx, &normidx) == 3) {
                if( numVerts >= MAX_VERTICES_PER_FACE || vertidx > (int)vertices.size() - 0 ||
                    numNorms >= MAX_VERTICES_PER_FACE || normidx > (int)normals.size() - 0 )
                    return false;
                vertIndices[numVerts++] = vertidx;
                normIndices[numNorms++] = normidx;
                while (*cp && *cp != ' ') cp++;
                while (*cp == ' ') cp++;
            }
            while (sscanf(cp, "%d/%d", &vertidx, &texidx) == 2) {
                if( numVerts >= MAX_VERTICES_PER_FACE || vertidx > (int)vertices.size() - 0 )
                    return false;
                vertIndices[numVerts++] = vertidx;
                while (*cp && *cp != ' ') cp++;
                while (*cp == ' ') cp++;
            }
            while (sscanf(cp, "%d", &vertidx) == 1) {
                if( numVerts >= MAX_VERTICES_PER_FACE || vertidx > (int)vertices.size() - 0 )
                    return false;
                vertIndices[numVerts++] = vertidx;
                while (*cp && *cp != ' ') cp++;
                while (*cp == ' ') cp++;
            }

            if (*cp)
                return false;

            // world's most naive trianglization
            for( int i = 1; i < numVerts - 1; i++ ) {
                vertexData.push_back( vertices[vertIndices[0] - 1] );
                vertexData.push_back( vertices[vertIndices[i] - 1] );               
                vertexData.push_back( vertices[vertIndices[i+1] - 1] );

                // if we have a normal for each vertex, use them. Otherwise generate some
                // ugly facety normals just so we have something.
                if( numVerts == numNorms ) {
                    normalData.push_back( normals[normIndices[0] - 1] );
                    normalData.push_back( normals[normIndices[i] - 1] );
                    normalData.push_back( normals[normIndices[i+1] - 1] );      
                } else {
                    float3 v1 = vertices[vertIndices[0] - 1] - vertices[vertIndices[i] - 1];
                    float3 v2 = vertices[vertIndices[0] - 1] - vertices[vertIndices[i+1] - 1];
                    float3 n = v1.cross(v2).getNormalized();

                    normalData.push_back( n );
                    normalData.push_back( n );
                    normalData.push_back( n );
                }

                numTriangles++;
            }

        }
    }

    return true;
}



void SimpleModel::unitizeVertices( std::vector<float3>& vertices )
{
    float scaleX = 2.0 / (maxX - minX);
    float scaleY = 2.0 / (maxY - minY);
    float scaleZ = 2.0 / (maxZ - minZ);
    float centerX = minX + (maxX - minX) * 0.5;
    float centerY = minY + (maxY - minY) * 0.5;
    float centerZ = minZ + (maxZ - minZ) * 0.5;

    float scaleMin = std::min( scaleX, std::min( scaleY, scaleZ ) );
    for( int i = 0; i < (int)vertices.size(); i++ ) {
        vertices[i].x = (vertices[i].x - centerX) * scaleMin;
        vertices[i].y = (vertices[i].y - centerY) * scaleMin;
        vertices[i].z = (vertices[i].z - centerZ) * scaleMin;
    }
}

void SimpleModel::createVBO()
{
    glf->glGenVertexArrays(1, &vao);
    glf->glBindVertexArray(vao);

    glf->glGenBuffers( 1, &normalBuffer );
    glf->glBindBuffer( GL_ARRAY_BUFFER, normalBuffer );
    glf->glBufferData( GL_ARRAY_BUFFER, sizeof(float3) * 3 * numTriangles, (void*)&normalData[0], GL_STATIC_DRAW );

    glf->glGenBuffers( 1, &vertexBuffer );
    glf->glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
    glf->glBufferData( GL_ARRAY_BUFFER, sizeof(float3) * 3 * numTriangles, (void*)&vertexData[0], GL_STATIC_DRAW );\

    glf->glBindVertexArray(0);

    madeVBO = true;
}


void SimpleModel::drawVBO(DGLShader* shader)
{
    if( !loaded )
        return;

    if( !madeVBO )
        createVBO();

    // draw!
    glf->glBindVertexArray(vao);

    int vert_loc = shader->getAttribLocation("vtx_position");
    if(vert_loc >= 0) {
        glf->glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
        glf->glVertexAttribPointer(vert_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(vert_loc);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc >= 0){
        glf->glBindBuffer( GL_ARRAY_BUFFER, normalBuffer );
        glf->glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(normal_loc);
    }

    glf->glDrawArrays(GL_TRIANGLES, 0, numTriangles * 3);

    glf->glBindVertexArray(0);
}
