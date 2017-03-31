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
#include <sstream>
#include <vector>
#include "BRDFBase.h"
#include "BRDFAnalytic.h"
#include "BRDFMeasuredMERL.h"
#include "BRDFMeasuredAniso.h"
#include "BRDFImageSlice.h"
#include "trim.h"
#include "DGLShader.h"
#include "Paths.h"



BRDFBase::BRDFBase() : initializedGL(false)
{
    std::string templateDir = getShaderTemplatesPath();

    shaders[SHADER_REFLECTOMETER].vertexShaderFilename       = templateDir + "brdftemplate3D.vert";
    shaders[SHADER_REFLECTOMETER].fragmentShaderFilename     = templateDir + "brdftemplate3D.frag";

    shaders[SHADER_POLAR].vertexShaderFilename               = templateDir + "brdftemplate2D.vert";
    shaders[SHADER_POLAR].fragmentShaderFilename             = templateDir + "brdftemplate2D.frag";
    shaders[SHADER_POLAR].geometryShaderFilename             = templateDir + "brdftemplatePlot.geom";

    shaders[SHADER_LITSPHERE].vertexShaderFilename           = templateDir + "brdftemplatesphere.vert";
    shaders[SHADER_LITSPHERE].fragmentShaderFilename         = templateDir + "brdftemplatesphere.frag";

    shaders[SHADER_CARTESIAN].vertexShaderFilename           = templateDir + "brdftemplateAnglePlot.vert";
    shaders[SHADER_CARTESIAN].fragmentShaderFilename         = templateDir + "brdftemplateAnglePlot.frag";
    shaders[SHADER_CARTESIAN].geometryShaderFilename         = templateDir + "brdftemplatePlot.geom";

    shaders[SHADER_CARTESIAN_THETA_H].vertexShaderFilename   = templateDir + "brdftemplateAnglePlotThetaH.vert";
    shaders[SHADER_CARTESIAN_THETA_H].fragmentShaderFilename = templateDir + "brdftemplateAnglePlotThetaH.frag";
    shaders[SHADER_CARTESIAN_THETA_H].geometryShaderFilename = templateDir + "brdftemplatePlot.geom";

    shaders[SHADER_CARTESIAN_THETA_D].vertexShaderFilename   = templateDir + "brdftemplateAnglePlotThetaD.vert";
    shaders[SHADER_CARTESIAN_THETA_D].fragmentShaderFilename = templateDir + "brdftemplateAnglePlotThetaD.frag";
    shaders[SHADER_CARTESIAN_THETA_D].geometryShaderFilename = templateDir + "brdftemplatePlot.geom";

    shaders[SHADER_CARTESIAN_ALBEDO].vertexShaderFilename    = templateDir + "brdftemplateAnglePlotAlbedo.vert";
    shaders[SHADER_CARTESIAN_ALBEDO].fragmentShaderFilename  = templateDir + "brdftemplateAnglePlotAlbedo.frag";
    shaders[SHADER_CARTESIAN_ALBEDO].geometryShaderFilename  = templateDir + "brdftemplateAnglePlotAlbedo.geom";

    shaders[SHADER_IMAGE_SLICE].vertexShaderFilename         = templateDir + "brdftemplateImageSlice.vert";
    shaders[SHADER_IMAGE_SLICE].fragmentShaderFilename       = templateDir + "brdftemplateImageSlice.frag";

    shaders[SHADER_IBL].vertexShaderFilename                 = templateDir + "brdfIBL.vert";
    shaders[SHADER_IBL].fragmentShaderFilename               = templateDir + "brdfIBL.frag";
}


BRDFBase::~BRDFBase()
{
    // nuke the shaders
    for( int i = 0; i < NUM_SHADERS; i++ )
    {
        if( shaders[i].shader )
            delete shaders[i].shader;
    }
}


bool BRDFBase::loadBRDF( const char* filename )
{
    // for now, the BRDF name is just the filename
    name = std::string( filename );

    std::ifstream ifs( filename );
    std::string line, trimmedLine;
    std::string token;
    std::string currentSection;

    if( !beginFile() )
        return false;

    // first line needs to be the BRDF type
    if( !getline( ifs, line ) )
        return false;

    // make sure we know how to read this
    std::string type = trim(line);
    type = type.substr( 0, type.find( ' ' ) );
    if( type != "analytic" && type != "bparam" )
        return false;


    while( getline( ifs, line ) )
    {
        trimmedLine = line;
        trim( trimmedLine );

        // skip blank lines
        if( trimmedLine.length() == 0 )
            continue;

        // skip comments
        if( trimmedLine[0] == '#' )
            continue;

        // grab the first token from the line
        std::stringstream os(trimmedLine);
        os >> token;

        // beginning of a section?
        if( token == "::begin" )
        {
            os >> token;

            // still in another section?
            if( currentSection != "" )
                return false;
            currentSection = token;
            beginSection( currentSection );

            // make sure we know what this is
            if( currentSection != "parameters" && !canReadSectionType(currentSection) )
                printf( "Warning: Don't know what to do with %s.\n", currentSection.c_str() );
        }

        // end of a section?
        else if( token == "::end" )
        {
            os >> token;

            // can't nest modes, so this needs to be the end of the current section
            if( token != currentSection )
                return false;

            endSection( currentSection );
            currentSection = "";
        }

        // just a line in a section
        else
        {
            // if we're reading parameters, then... read a parameter
            if( currentSection == "parameters" )
            {
                if( !processParameterLine( trimmedLine ) )
                    return false;
            }

            // derived class can read whatever
            else
            {
                if( !processLineFromSection( currentSection, line ) )
                    return false;
            }
        }
    }

    if( !endFile() )
        return false;


    return true;
}


void BRDFBase::addFloatParameter( std::string name, float min, float max, float value )
{
    brdfFloatParam param;
    param.name = name;
    param.minVal = min;
    param.maxVal = max;
    param.defaultVal = value;
    param.currentVal = value;

    floatParameters.push_back( param );
    parameterTypes.push_back( BRDF_VAR_FLOAT );
}


void BRDFBase::addBoolParameter( std::string name, bool value )
{
    brdfBoolParam param;
    param.name = std::string( name );
    param.defaultVal = value;
    param.currentVal = value;
    boolParameters.push_back( param );
    parameterTypes.push_back( BRDF_VAR_BOOL );
}

void BRDFBase::addColorParameter( std::string name, float r, float g, float b )
{
    brdfColorParam param;

    param.name = name;
    param.currentVal[0] = param.defaultVal[0] = r;
    param.currentVal[1] = param.defaultVal[1] = g;
    param.currentVal[2] = param.defaultVal[2] = b;

    colorParameters.push_back( param );
    parameterTypes.push_back( BRDF_VAR_COLOR );
}

bool BRDFBase::processParameterLine( std::string line )
{
    std::stringstream os(line);
    std::string token;
    os >> token;

    // we only handle floats and bools right now
    if( token == "bool" )
    {
        std::string name;
        bool value;

        os >> name;
        os >> value;

        addBoolParameter( name, value );
    }
    else if( token == "float" )
    {
        std::string name;
        float min, max, value;

        os >> name;
        os >> min;
        os >> max;
        os >> value;

        addFloatParameter( name, min, max, value );
    }
    else if( token == "color" )
    {
        std::string name;
        float r, g, b;

        os >> name;
        os >> r;
        os >> g;
        os >> b;

        addColorParameter( name, r, g, b );
    }
    else
        return false;

    return true;
}


DGLShader* BRDFBase::getUpdatedShader( int shaderType, brdfPackage* pkg )
{
    initGL();

    // if there aren't any shaders, try to compile one
    if( !shaders[shaderType].shader )
        compileShader( shaders[shaderType].shader,
                       shaders[shaderType].vertexShaderFilename,
                       shaders[shaderType].fragmentShaderFilename,
                       shaders[shaderType].geometryShaderFilename);

    // assuming the compilation worked...
    DGLShader* shader = shaders[shaderType].shader;
    if( shader )
    {
        shader->enable();

        // let any derived classes add stuff to the shader
        adjustShaderPreRender( shader );

        // sets colors
        setColorsFromPackage( shader, pkg );

        return shader;
    }

    // if it didn't... sorry, folks, but there's no shader.
    return NULL;
}



void BRDFBase::disableShader( int shaderType )
{
    DGLShader* shader = shaders[shaderType].shader;

    // it's a problem if there's no shader... but not one we can fix here
    if( !shader )
        return;

    adjustShaderPostRender( shader );
    shader->disable();
}



std::string BRDFBase::loadShaderFromFile( std::string filename, std::string chunkToInsert, std::string isFuncToInsert )
{
    std::ifstream ifs( filename.c_str() );
    std::string line;
    std::string completeShader;

    // read the file line by line, and optionally, replace the below token with chunkToInsert
    while( getline( ifs, line ) )
    {
        // at the top of the shader we insert all the uniforms for this shader
        if( line == "::INSERT_UNIFORMS_HERE::" )
        {
            for( int i = 0; i < (int)floatParameters.size(); i++ )
                completeShader += "uniform float " + floatParameters[i].name + ";\n";
            for( int i = 0; i < (int)boolParameters.size(); i++ )
                completeShader += "uniform bool " + boolParameters[i].name + ";\n";
            for( int i = 0; i < (int)colorParameters.size(); i++ )
                completeShader += "uniform vec3 " + colorParameters[i].name + ";\n";
        }
        else if( chunkToInsert.length() && line == "::INSERT_BRDF_FUNCTION_HERE::" )
        {
            completeShader += std::string("\n") + chunkToInsert + "\n";
        }
        else if( isFuncToInsert.length() && line == "::INSERT_IS_FUNCTION_HERE::" )
        {
            completeShader += std::string("\n") + isFuncToInsert + "\n";
        }
        else
        {
            completeShader += line + "\n";
        }
    }


    return completeShader;
}



bool BRDFBase::compileShader( DGLShader*& shader, std::string vs, std::string fs, std::string gs )
{
    // nuke the shader if it exists
    if( shader )
        delete shader;

    // here's the tricky bit: load the shader templates, sticking in the BRDF function where needed
    std::string vertShader = loadShaderFromFile( vs, getBRDFFunction(), getISFunction() );
    std::string fragShader = loadShaderFromFile( fs, getBRDFFunction(), getISFunction() );

/*
    printf( "==========\n" );
    printf( "%s\n", fragShader.c_str() );
    printf( "==========\n" );
*/

    // try and compile it
    //IC: This expects file names!s
//	shader = new DGLShader( vertShader, fragShader );
    shader = new DGLShader();
    shader->setVertexShaderFromString(vertShader);
    shader->setFragmentShaderFromString(fragShader);
    if(!gs.empty())
        shader->setGeometryShaderFromFile(gs);
    shader->create();
    return shader->ready();
}



std::string BRDFBase::getBRDFFunction()
{
    std::string func = "float BRDF( vec3 toLight, vec3 toViewer, vec3 normal, vec3 tangent, vec3 bitangent )\n";
    func += "{ return 1.0; }\n";

    return func;
}




std::string BRDFBase::getISFunction()
{
    std::string func = "vec3 sampleBRDF( float u, float v, vec3 normal, vec3 tangent, vec3 bitangent, out vec3 wi, out float pdf )\n";
    func += "{ return vec3(0.0); }\n";

    return func;
}



void BRDFBase::setFloatParameterValue( int index, float value )
{
    if( index >= 0 && index < (int)floatParameters.size() )
        floatParameters[index].currentVal = value;
}


brdfFloatParam* BRDFBase::getFloatParameter( int index )
{
    if( index >= 0 && index < (int)floatParameters.size() )
        return &floatParameters[index];
    return NULL;
}


int BRDFBase::getFloatParameterCount()
{
    return (int)floatParameters.size();
}



void BRDFBase::setBoolParameterValue( int index, bool value )
{
    if( index >= 0 && index < (int)boolParameters.size() )
        boolParameters[index].currentVal = value;
}


brdfBoolParam* BRDFBase::getBoolParameter( int index )
{
    if( index >= 0 && index < (int)boolParameters.size() )
        return &boolParameters[index];
    return NULL;
}


int BRDFBase::getBoolParameterCount()
{
    return (int)boolParameters.size();
}


void BRDFBase::setColorParameterValue( int index, float r, float g, float b )
{
    if( index >= 0 && index < (int)colorParameters.size() )
    {
        colorParameters[index].currentVal[0] = r;
        colorParameters[index].currentVal[1] = g;
        colorParameters[index].currentVal[2] = b;
    }
}


brdfColorParam* BRDFBase::getColorParameter( int index )
{
    if( index >= 0 && index < (int)colorParameters.size() )
        return &colorParameters[index];
    return NULL;
}


int BRDFBase::getColorParameterCount()
{
    return (int)colorParameters.size();
}



void BRDFBase::adjustShaderPreRender( DGLShader* shader )
{
    for( int i = 0; i < (int)floatParameters.size(); i++ )
    {
        shader->setUniformFloat( (char*)floatParameters[i].name.c_str(), floatParameters[i].currentVal );
    }

    for( int i = 0; i < (int)boolParameters.size(); i++ )
    {
        shader->setUniformInt( (char*)boolParameters[i].name.c_str(), boolParameters[i].currentVal );
    }

    for( int i = 0; i < (int)colorParameters.size(); i++ )
    {
        shader->setUniformFloat( (char*)colorParameters[i].name.c_str(),  colorParameters[i].currentVal[0],
                                        colorParameters[i].currentVal[1], colorParameters[i].currentVal[2] );
    }
}



void BRDFBase::setColorsFromPackage( DGLShader* shader, brdfPackage* pkg )
{
    if( !shader || !pkg )
        return;

    shader->setUniformFloat( "drawColor", pkg->drawColor[0], pkg->drawColor[1], pkg->drawColor[2] );
    shader->setUniformFloat( "colorMask", pkg->colorMask[0], pkg->colorMask[1], pkg->colorMask[2] );
}


void BRDFBase::syncParametersIntoBRDF( BRDFBase* newBRDF )
{
    // for each parameter in the new BRDF, see if we have a matching one
    for( int i = 0; i < (int)newBRDF->floatParameters.size(); i++ )
    {
        for( int j = 0; j < (int)floatParameters.size(); j++ )
        {
            if( newBRDF->floatParameters[i].name == floatParameters[j].name )
            {
                    // we found a match - copy the old value to the new BRDF
                    newBRDF->floatParameters[i].currentVal = floatParameters[j].currentVal;
            }
        }
    }

    for( int i = 0; i < (int)newBRDF->boolParameters.size(); i++ )
    {
        for( int j = 0; j < (int)boolParameters.size(); j++ )
        {
            if( newBRDF->boolParameters[i].name == boolParameters[j].name )
            {
                    // we found a match - copy the old value to the new BRDF
                    newBRDF->boolParameters[i].currentVal = boolParameters[j].currentVal;
            }
        }
    }

    for( int i = 0; i < (int)newBRDF->colorParameters.size(); i++ )
    {
        for( int j = 0; j < (int)colorParameters.size(); j++ )
        {
            if( newBRDF->colorParameters[i].name == colorParameters[j].name )
            {
                    // we found a match - copy the old value to the new BRDF
                    newBRDF->colorParameters[i].currentVal[0] = colorParameters[j].currentVal[0];
                    newBRDF->colorParameters[i].currentVal[1] = colorParameters[j].currentVal[1];
                    newBRDF->colorParameters[i].currentVal[2] = colorParameters[j].currentVal[2];
            }
        }
    }
}


void BRDFBase::saveParamsFile( const char* filename )
{
    FILE* out = fopen( filename, "wt" );
    if( !out )
        return;

    fprintf( out, "bparam %s\n\n", name.c_str() );
    fprintf( out, "::begin parameters\n" );

    // write out the current values of the parameters
    for( int i = 0; i < (int)floatParameters.size(); i++ )
        fprintf( out, "float %s %f %f %f\n", floatParameters[i].name.c_str(),
                 floatParameters[i].minVal, floatParameters[i].maxVal, floatParameters[i].currentVal );
    for( int i = 0; i < (int)boolParameters.size(); i++ )
        fprintf( out, "bool %s %d\n", boolParameters[i].name.c_str(),
                 boolParameters[i].currentVal ? 1 : 0 );
    for( int i = 0; i < (int)colorParameters.size(); i++ )
        fprintf( out, "color %s %f %f %f\n", colorParameters[i].name.c_str(),
                                             colorParameters[i].currentVal[0],
                                             colorParameters[i].currentVal[1],
                                             colorParameters[i].currentVal[2] );

    fprintf( out, "::end parameters\n" );
    fclose( out );
}



BRDFBase* BRDFBase::cloneBRDF(bool resetToDefaults)
{
    BRDFBase* b = createBRDFFromFile( name );

    if( b && resetToDefaults == false )
        syncParametersIntoBRDF( b );

    return b;
}



//////////////////////////////////////////////////////////////////////////////////////////////


// this is a factory function that creates a BRDF class of a given type, based on the extension of the input file
BRDFBase* createBRDFFromFile( std::string filename )
{
    std::string extension = filename.substr( filename.find_last_of( '.' ) +1 );
    BRDFBase* b = NULL;
    bool success = false;

    // if it's got a .binary extension, we'll guess it's a MERL file
    if( extension == "binary" )
    {
        BRDFMeasuredMERL* mb = new BRDFMeasuredMERL;
        success = mb->loadMERLData( filename.c_str() );
        b = mb;
    }

    // .dat is an Anisotropic BRDF
    else if( extension == "dat" )
    {
      BRDFMeasuredAniso* mb = new BRDFMeasuredAniso;
      success = mb->loadAnisoData( filename.c_str() );
      b = mb;
    }

    // .brdf is an analytic BRDF file
    else if( extension == "brdf" )
    {
        b = new BRDFAnalytic;
        success = b->loadBRDF( filename.c_str() );
    }

    // .brdf is an analytic BRDF file
    else if( extension == "bparam" )
    {
        // extract the BRDF filename from the first line from the bparam file
        std::ifstream ifs( filename.c_str() );
        std::string line;
        getline( ifs, line );
        line = trim(line);
        std::string dataFile = line.substr( line.find( ' ' ) + 1 );

        // try and load the bparam file as an analytic, to extract the parameters
        BRDFBase* dummy = new BRDFAnalytic;
        success = dummy->loadBRDF( filename.c_str() );
        if( !success )
        {
            delete dummy;
            return NULL;
        }

        // create a new BRDF from the datafile name
        b = createBRDFFromFile( dataFile );
        if( !b )
            return NULL;

        // now that we've got an actual BRDF, and a fake BRDF with parameters, sync 'em
        dummy->syncParametersIntoBRDF( b );

        // all done with the dummy BRDF class
        delete dummy;

        b->setName( filename );
    }

    // .brdf is an image slice
    else if( extension == "tif" )
    {
        BRDFImageSlice* mb = new BRDFImageSlice;
        success = mb->loadImage( filename.c_str() );
        b = mb;
    }

    // silently ignore everything else
    else return NULL;

    if( !success )
    {
        delete b;
        return NULL;
    }

    return b;
}
