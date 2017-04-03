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

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include "DGLShader.h"


DGLShader::DGLShader()
        : _vertexShaderID(0),
          _geometryShaderID(0),
          _fragmentShaderID(0),
          _programID(0),
          _printErrors(true),
          _ready(false)
{
    // reserve some space in the bound textures array
    _boundTextures.reserve(32);
}


DGLShader::DGLShader( std::string vertFilename, std::string fragFilename,  std::string geomFilename )
        : _vertexShaderID(0),
          _geometryShaderID(0),
          _fragmentShaderID(0),
          _programID(0),
          _printErrors(true),
          _ready(false)
{
    setVertexShaderFromFile( vertFilename );
    setFragmentShaderFromFile( fragFilename );
    setGeometryShaderFromFile( geomFilename );
    create();
}


DGLShader::~DGLShader()
{
    clear();
}


void DGLShader::setVertexShaderFromFile( std::string filename )
{
    _vertexShaderFilename = filename;
}


void DGLShader::setGeometryShaderFromFile( std::string filename )
{
    _geometryShaderFilename = filename;
}


void DGLShader::setFragmentShaderFromFile( std::string filename )
{
    _fragmentShaderFilename = filename;
}


void DGLShader::setVertexShaderFromString( std::string filename )
{
    _vertexShaderString = filename;
}


void DGLShader::setGeometryShaderFromString( std::string filename )
{
    _geometryShaderString = filename;
}


void DGLShader::setFragmentShaderFromString( std::string filename )
{
    _fragmentShaderString = filename;
}


bool DGLShader::compileAndAttachShader( GLuint& shaderID, GLenum shaderType, const char* shaderTypeStr, std::string filename, std::string contents )
{


    // if there's no filename and no contents, then a shader for this type hasn't been
    // specified. That's not (necessarily) an error, so return true - if it's a problem
    // then there will be a link error.
    if( !filename.length() && !contents.length() )
        return true;


    // create this shader if it hasn't been created yet
    if( !shaderID )
        shaderID = glf->glCreateShader( shaderType );

    // If a shader was passed in, just use that. Otherwise try and load from the filename.
    if( !contents.length() ) {

        // try and load the file
        if( !readFile( filename, contents ) ) {
            printf( "DGLShader: Error reading file %s\n", filename.c_str() );
            return false;
        }
    }

    // try to compile the shader
    const char* source = contents.c_str();
    glf->glShaderSource( shaderID, 1, &source, NULL );
    glf->glCompileShader( shaderID );

    // if it didn't work, print the error message
    GLint compileStatus;
    glf->glGetShaderiv( shaderID, GL_COMPILE_STATUS, &compileStatus );
    if( !compileStatus ) {

        // perhaps print the linker error
        if( _printErrors ) {

            // get the length of the error message
            GLint errorMessageLength;
            glf->glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &errorMessageLength );

            // get the error message itself
            char* errorMessage = new char[errorMessageLength];
            glf->glGetShaderInfoLog( shaderID, errorMessageLength, &errorMessageLength, errorMessage );

            // print the error
            if( filename.length() )
                printf( "GLSL %s Compile Error (file: %s)\n", shaderTypeStr, filename.c_str() );
            else
                printf( "GLSL %s Compile Error (no shader file)\n", shaderTypeStr );
            printf( "================================================================================\n" );

            // also display shader source
            {
                GLint length;
                glf->glGetShaderiv(shaderID, GL_SHADER_SOURCE_LENGTH, &length);
                GLchar *source= new GLchar [length];
                glf->glGetShaderSource(shaderID, length, NULL, source);

                printf("%s\n", source);
                delete [] source;
            }

            printf( "%s\n", errorMessage );
            printf( "\n" );

            delete[] errorMessage;
        }

        return false;
    }

    // attach it to the program object
    glf->glAttachShader( _programID, shaderID );

    return true;
}


void DGLShader::clear()
{
    // delete the different shader types
    if( _vertexShaderID ) {
        glf->glDetachShader( _programID, _vertexShaderID );
        glf->glDeleteShader( _vertexShaderID );
    }

    if( _fragmentShaderID ) {
        glf->glDetachShader( _programID, _fragmentShaderID );
        glf->glDeleteShader( _fragmentShaderID );
    }

    if( _geometryShaderID ) {
        glf->glDetachShader( _programID, _geometryShaderID );
        glf->glDeleteShader( _geometryShaderID );
    }

    // finally, delete the program object itself
    glf->glDeleteProgram( _programID );

    // reset the IDs
    _vertexShaderID = 0;
    _geometryShaderID = 0;
    _fragmentShaderID = 0;
    _programID = 0;
}


bool DGLShader::create( bool linkNow )
{
    _ready = false;

    if( !_programID )
        _programID = glf->glCreateProgram();

    // compile and attach the different shader objects
    if( !compileAndAttachShader( _vertexShaderID, GL_VERTEX_SHADER, "Vertex Shader", _vertexShaderFilename, _vertexShaderString ) )
        return false;

    if( !compileAndAttachShader( _geometryShaderID, GL_GEOMETRY_SHADER, "Geometry Shader", _geometryShaderFilename, _geometryShaderString ) )
        return false;

    if( !compileAndAttachShader( _fragmentShaderID, GL_FRAGMENT_SHADER, "Fragment Shader", _fragmentShaderFilename, _fragmentShaderString ) )
        return false;

    if( linkNow )
        return link();

    return true;
}


bool DGLShader::link()
{
    // link the different shaders that are attached to the program object
    glf->glLinkProgram( _programID );

    // did the link work?
    GLint linkedOK = 0;
    glf->glGetProgramiv( _programID, GL_LINK_STATUS, &linkedOK);

    if( !linkedOK ) {

        // perhaps print the linker error
        if( _printErrors ) {

            // get the length of the error message
            GLint errorMessageLength;
            glf->glGetProgramiv( _programID, GL_INFO_LOG_LENGTH, &errorMessageLength );

            // get the error message itself
            char* errorMessage = new char[errorMessageLength];
            glf->glGetProgramInfoLog( _programID, errorMessageLength, &errorMessageLength, errorMessage );

            // print it
            printf( "GLSL Linker Error:\n" );
            printf( "================================================================================\n" );
            printf( "%s\n", errorMessage );
            printf( "\n" );

            delete[] errorMessage;
        }

        return false;
    }

    // this shader is ready to go
    _ready = true;

    return true;
}


void DGLShader::reload()
{
    // recreate all the shaders
    create( true );
}


void DGLShader::enable()
{
    // enable the shader
    glf->glUseProgram( _programID );

    // reset the auto-texture binding stuff
    _firstAvailableTextureUnit = 0;
    _boundTextures.clear();
}


bool DGLShader::setUniformTexture( const char* textureName, GLint textureID, GLenum target, int textureUnit, bool enableTextureTarget )
{
    int textureUnitToUse = 0;

    GLint uniformLocation = glf->glGetUniformLocation( _programID, textureName );
    if( uniformLocation == -1 )
        return false;

    // if a texture unit was specified, use that
    if( textureUnit >= 0 )
        textureUnitToUse = textureUnit;

    // otherwise look for the first available texture unit and use that
    else {

        // start by looking at the next texture unit (after the last one that was used)
        int textureUnitToTry = _firstAvailableTextureUnit;

        // keep looking until we find a free texture unit
        while( textureUnitToTry < (int)_boundTextures.size() && _boundTextures[textureUnitToTry].bound )
            textureUnitToTry++;

        // found one!
        textureUnitToUse = textureUnitToTry;
        _firstAvailableTextureUnit = textureUnitToUse + 1;
    }

    // make sure the vector is large enough
    if( textureUnitToUse >= (int)_boundTextures.size() )
        _boundTextures.resize( textureUnitToUse + 1 );

    // add the texture information to the vector
    BoundTexture tex;
    tex.target = target;
    tex.unit = textureUnitToUse;
    tex.bound = true;
    _boundTextures[textureUnitToUse] = tex;

    // setup the actual texture
    glf->glUniform1i( uniformLocation, textureUnitToUse );
    glf->glActiveTexture( GL_TEXTURE0 + textureUnitToUse );
    glf->glBindTexture( target, textureID );

    // potentially enable it (this is an option because enabling certain texture types breaks things)
    if( enableTextureTarget )
        glf->glEnable( target );

    return true;
}


void DGLShader::disable( bool disableTextureUnits )
{
    // disable the shader
    glf->glUseProgram(0);

    // possibly disable the textures that were bound to this shader
    if( disableTextureUnits ) {

        // look at the texture units that were bound
        for( int i = 0; i < (int)_boundTextures.size(); i++ ) {

            // if the given texture was bound, disable it
            if( _boundTextures[i].bound ) {
                disableTexture( _boundTextures[i].target, _boundTextures[i].unit );
                _boundTextures[i].bound = false;
            }
        }
    }
}


void DGLShader::disableTexture( GLenum target, int unit )
{
    glf->glActiveTexture( GL_TEXTURE0 + unit );
    //glDisable( target );
    glf->glBindTexture( target, 0 );
}


void DGLShader::setUniformFloat( const char* uniformName, float a )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform1f( uniformLocation, a );
}


void DGLShader::setUniformFloat( const char* uniformName, float a, float b )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform2f( uniformLocation, a, b );
}


void DGLShader::setUniformFloat( const char* uniformName, float a, float b, float c )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform3f( uniformLocation, a, b, c );
}


void DGLShader::setUniformFloat( const char* uniformName, float a, float b, float c, float d )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform4f( uniformLocation, a, b, c, d );
}


void DGLShader::setUniformFloatArray( const char* uniformName, int elementCount, int arrayLength, float* arrayData )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    if( elementCount == 1 )
        glf->glUniform1fv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 2 )
        glf->glUniform2fv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 3 )
        glf->glUniform3fv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 4 )
        glf->glUniform4fv( uniformLocation, arrayLength, arrayData );

    else return;
}


void DGLShader::setUniformInt( const char* uniformName, int a )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform1i( uniformLocation, a );
}


void DGLShader::setUniformInt( const char* uniformName, int a, int b )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform2i( uniformLocation, a, b );
}

void DGLShader::setUniformInt( const char* uniformName, int a, int b, int c )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform3i( uniformLocation, a, b, c );
}


void DGLShader::setUniformInt( const char* uniformName, int a, int b, int c, int d )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniform4i( uniformLocation, a, b, c, d );
}


void DGLShader::setUniformIntArray( const char* uniformName, int elementCount, int arrayLength, int* arrayData )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    if( elementCount == 1 )
        glf->glUniform1iv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 2 )
        glf->glUniform2iv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 3 )
        glf->glUniform3iv( uniformLocation, arrayLength, arrayData );

    else if( elementCount == 4 )
        glf->glUniform4iv( uniformLocation, arrayLength, arrayData );

    else return;
}


void DGLShader::setUniformMatrix4( const char* uniformName, float* matrix )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniformMatrix4fv( uniformLocation, 1, false, matrix );
}

void DGLShader::setUniformMatrix3( const char* uniformName, float* matrix )
{
    GLint uniformLocation = glf->glGetUniformLocation( _programID, uniformName );
    if( uniformLocation == -1 )
        return;

    glf->glUniformMatrix3fv( uniformLocation, 1, false, matrix );
}



bool DGLShader::readFile( std::string filename, std::string& contents )
{
    contents = "";
    std::ifstream file;

    // try to open the file
    file.open( filename.c_str() );
    if( !file.is_open() )
        return false;

    // read it line by line
    std::string line;
    while( !file.eof() )
    {
        getline( file, line );
        contents += line + "\n";
    }
    file.close();

    return true;
}

int DGLShader::getAttribLocation(const char* name) const
{
    return glf->glGetAttribLocation(_programID, name);
}
