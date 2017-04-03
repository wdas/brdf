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
This is DGLShader, a class that simplifies working with GLSL shaders in OpenGL.

CREATING SHADERS:
-------------------------------------------------------
To use, create an instance of the object, give it filenames for the different
shaders, and call create, like such:

    DGLShader* shader = new DGLShader();
    shader->setVertexShaderFromFile( "/home/greg/shader.vert" );
    shader->setFragmentShaderFromFile( "/home/greg/shader.frag" );
    shader->create();

Alternatively, you can set the shader code directly instead of from a file:

    shader->setVertexShaderFromString( "void main() {... and so on... " );

And if you just want to create a vertex+fragment shader combo, there's a
convenience constructor for that (create is called for you in this case):

   DGLShader* shader = new DGLShader(  "/home/greg/shader.vert",
                                       "/home/greg/shader.frag");

SETTING PARAMETERS:
-------------------------------------------------------
The setUniformFloat and setUniformInt functions let you set uniform parameters
with 1-4 components - they come in as a float, vec2, vec3, or vec4 for floats.
Int params can come in as a bool/int/uint, bvec2/uvec2/ivec2, etc. Remember
to call enable() before setting params.

Use them like so:

    shader->enable();
    shader->setUniformInt( "paramName", 23 );
    shader->setUniformFloat( "paramName", 0.5, 0.786, 1234.45 );

setUniformFloatArray and setUniformIntArray let you send uniform arrays, like so:

    shader->setUniformIntArray( "enabledLights", 1, 8, (int*)enabledLights );

... passes down an array of 8 single-component floats (as opposed to vec2, etc.)

setUniformMatrix4 lets you pass in a 4x4 matrix.


SETTING TEXTURES:
-------------------------------------------------------
Bind and enable uniform sampler textures using setUniformTexture. Doing this can
be as simple as:

    shader->setUniformTexture( "tex", textureID );

... where tex is the uniform name, and textureID is the id of an OpenGL texture.
The class takes care of choosing a texture unit to bind the texture to. Other
optional parameters allow you to choose the target (normally GL_TEXTURE_2D) and
to force binding to a particular texture unit if you want to do it that way.


ENABLING/DISABLING:
-------------------------------------------------------
To enable/disable the shader, call enable() and disable(), respectively.

disable() also disables all the bound textures unless you pass in false.

*/

#ifndef _DGL_SHADER_H_
#define _DGL_SHADER_H_

#include <string>
#include <vector>

#include "SharedContextGLWidget.h"


class DGLShader : public GLContext
{
private:

    // structure to keep track of the textures that have been bound to this shader.
    // the "bound" parameter is necessary because the _boundTextures vector may
    // contain more BoundTexture objects than actual bound textures (if the user)
    // specifies their own binding point, for example, so we need to keep track
    // of which ones are actually used.
    struct BoundTexture
    {
        BoundTexture() : target(GL_TEXTURE_2D), unit(-1), bound(false) {}
        GLenum target;
        int unit;
        bool bound;
    };

public:

    DGLShader();
    ~DGLShader();

    // convenience constructor, which sets up a vertex and fragment shader
    DGLShader( std::string vertFilename, std::string fragFilename,  std::string geomFilename="" );

    // sets filenames for the different shader types. Note that this does
    // NOT create the objects - you need to call create() to do that.
    void setVertexShaderFromFile( std::string filename );
    void setGeometryShaderFromFile( std::string filename );
    void setFragmentShaderFromFile( std::string filename );

    // set actual shader code for the different shader types. Note that this does
    // NOT create the objects - you need to call create() to do that.
    void setVertexShaderFromString( std::string );
    void setGeometryShaderFromString( std::string );
    void setFragmentShaderFromString( std::string );

    // the function that actually compiles the shaders. Disable linkNow if you need
    // to do some further setup before linking.
    bool create( bool linkNow = true );

    // link the shaders
    bool link();

    // returns true if the shader is ready to be used
    bool ready() { return _ready; }

    // delete all the program and shader objects
    void clear();

    // reload everything - simply calls create(true)
    void reload();

    // enable and disable the shader. Pass false to disable() if you don't want
    // the texture units to be automatically disabled.
    void enable();
    void disable( bool disableTextureUnits = true );

    // functions for setting float uniforms
    void setUniformFloat( const char* uniformName, float );
    void setUniformFloat( const char* uniformName, float, float );
    void setUniformFloat( const char* uniformName, float, float, float );
    void setUniformFloat( const char* uniformName, float, float, float, float );
    void setUniformFloatArray( const char* uniformName, int elementCount, int arrayLength, float* arrayData );

    // functions for setting int uniforms
    void setUniformInt( const char* uniformName, int);
    void setUniformInt( const char* uniformName, int, int );
    void setUniformInt( const char* uniformName, int, int, int );
    void setUniformInt( const char* uniformName, int, int, int, int );
    void setUniformIntArray( const char* uniformName, int elementCount, int arrayLength, int* arrayData );

    // function for setting a 4x4 matrix uniform
    void setUniformMatrix4( const char* uniformName, float* matrix );
    // function for setting a 3x3 matrix uniform
    void setUniformMatrix3( const char* uniformName, float* matrix );

    // function for setting a uniform texture:
    // textureName: name of the shader uniform
    // textureID: OpenGL texture ID of the texture you wish to bind
    // target: the kind of texture: GL_TEXTURE_2D, GL_TEXTURE_BUFFER, etc.
    // unit: the texture unit you want to bind to: -1 means it's figure out automatically
    // enableTextureTarget: whether glEnable should be called on the target type
    bool setUniformTexture( const char* textureName, GLint textureID,
                            GLenum target = GL_TEXTURE_2D, int unit = -1,
                            bool enableTextureTarget = false );

    // calls glDisable on the texture unit and the texture type
    void disableTexture( GLenum target, int unit );

    // returns the IDs of the program and shader objects. Usually not needed unless you
    // need to do things that this class doesn't/shouldn't support.
    GLuint getVertexShaderID()    { return _vertexShaderID; }
    GLuint getGeometryShaderID()  { return _geometryShaderID; }
    GLuint getFragmentShaderID()  { return _fragmentShaderID; }
    GLuint getProgramID()         { return _programID; }

    // returns the index of the generic attribute name
    int getAttribLocation(const char* name) const;

    // whether or not to print the error messages
    void printErrors( bool pe ) { _printErrors = pe; }

private:

    // get the contents of the supplied filename and put it in contents
    bool readFile( std::string filename, std::string& contents );

    // do the actual work of compiling the shader and attach it to the program object
    bool compileAndAttachShader( GLuint& shaderID, GLenum shaderType, const char* shaderTypeStr,
                                 std::string filename, std::string contents );

    // contents/filenames for the shaders
    std::string _vertexShaderFilename;
    std::string _vertexShaderString;

    std::string _geometryShaderFilename;
    std::string _geometryShaderString;

    std::string _fragmentShaderFilename;
    std::string _fragmentShaderString;

    // IDs of the different objects
    GLuint _vertexShaderID;
    GLuint _geometryShaderID;
    GLuint _fragmentShaderID;
    GLuint _programID;

    // where we keep track of the textures and where they are bound
    int _firstAvailableTextureUnit;
    std::vector<BoundTexture> _boundTextures;

    // whether to actually print any GLSL errors that occur
    bool _printErrors;

    // whether the shader is ready to go
    bool _ready;
};

#endif
