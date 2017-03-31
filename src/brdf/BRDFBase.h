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

#ifndef BRDF_BASE_H
#define BRDF_BASE_H

#define BRDF_ANALYTIC 0
#define BRDF_MEASURED 1

#include <string>
#include <vector>

#include "SharedContextGLWidget.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

class DGLShader;
class BRDFBase;

#define BRDF_VAR_FLOAT 0
#define BRDF_VAR_BOOL 1
#define BRDF_VAR_COLOR 2


#define NUM_SHADERS                 10
#define SHADER_DUMMY                0
#define SHADER_REFLECTOMETER        1
#define SHADER_POLAR                2
#define SHADER_LITSPHERE            3
#define SHADER_CARTESIAN            4
#define SHADER_CARTESIAN_THETA_H    5
#define SHADER_CARTESIAN_THETA_D    6
#define SHADER_IMAGE_SLICE          7
#define SHADER_IBL                  8
#define SHADER_CARTESIAN_ALBEDO     9


struct brdfFloatParam
{
    std::string name;
    float minVal;
    float maxVal;
    float defaultVal;
    float currentVal;
};

struct brdfBoolParam
{
    std::string name;
    bool defaultVal;
    bool currentVal;
};

struct brdfColorParam
{
    std::string name;
    float defaultVal[3];
    float currentVal[3];
};


struct brdfPackage
{
    brdfPackage()
    {
        // play it safe - don't assume other code will set this correctly
        dirty = true;
    }

    void setDrawColor( float dr, float dg, float db )
    {
        drawColor[0] = dr;
        drawColor[1] = dg;
        drawColor[2] = db;
    }

    void setColorMask( float mr, float mg, float mb )
    {
        colorMask[0] = mr;
        colorMask[1] = mg;
        colorMask[2] = mb;
    }

    BRDFBase* brdf;
    float drawColor[3];
    float colorMask[3];
    bool dirty;
};


struct shaderInfo
{
    shaderInfo()
    {
        shader = NULL;
    }

    std::string vertexShaderFilename;
    std::string fragmentShaderFilename;
    std::string geometryShaderFilename;
    DGLShader* shader;
};


class BRDFBase
{
friend class BRDFAnalytic;
friend BRDFBase* createBRDFFromFile( std::string filename );

public:
    BRDFBase();
    virtual ~BRDFBase();

    virtual std::string getName() { return name; }
    virtual void setName( std::string n ) { name = n; }

    bool loadBRDF( const char* filename );

    int getParameterCount() { return parameterTypes.size(); }
    int getParameterType(int i) { return parameterTypes[i]; }

    int getFloatParameterCount();
    brdfFloatParam* getFloatParameter( int paramIndex );
    void setFloatParameterValue( int paramIndex, float value );

    int getBoolParameterCount();
    brdfBoolParam* getBoolParameter( int paramIndex );
    void setBoolParameterValue( int paramIndex, bool value );

    int getColorParameterCount();
    brdfColorParam* getColorParameter( int paramIndex );
    void setColorParameterValue( int paramIndex,float r, float g, float b  );

    DGLShader* getUpdatedShader( int shaderType, brdfPackage* = NULL );
    void disableShader( int shaderType );

    void saveParamsFile( const char* filename );

    virtual bool hasISFunction() { return false; }

    // create a new BRDF based on this one
    virtual BRDFBase* cloneBRDF(bool resetToDefaults);

protected:

    virtual void addFloatParameter( std::string name, float min, float max, float value );
    virtual void addBoolParameter( std::string name, bool value );
    virtual void addColorParameter( std::string name, float r, float g, float b );

    virtual void initGL() { initializedGL = true; }

    virtual bool canReadSectionType( std::string ) { return false; }

    virtual bool beginFile() { return true; }
    virtual bool beginSection( std::string ) { return true; }
    virtual bool endSection( std::string ) { return true; }
    virtual bool processLineFromSection( std::string, std::string ) { return true; }
    virtual bool endFile() { return true; }

    virtual std::string getBRDFFunction();
    virtual std::string getISFunction();

    virtual void adjustShaderPreRender( DGLShader* );
    virtual void adjustShaderPostRender( DGLShader* ) {}
    void setColorsFromPackage( DGLShader*, brdfPackage* );
    void syncParametersIntoBRDF( BRDFBase* );

    bool initializedGL;

    std::vector<int> parameterTypes;
    std::vector<brdfFloatParam> floatParameters;
    std::vector<brdfBoolParam> boolParameters;
    std::vector<brdfColorParam> colorParameters;
    std::string name;

private:

    bool processParameterLine( std::string line );

    std::string loadShaderFromFile( std::string, std::string = "", std::string = "" );

    bool compileShader(DGLShader*& shader, std::string vs, std::string fs , std::string gs);

    shaderInfo shaders[NUM_SHADERS];
};


// factory function
BRDFBase* createBRDFFromFile( std::string filename );


#endif
