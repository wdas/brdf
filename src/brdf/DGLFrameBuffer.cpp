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

#include <GL/glew.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include "DGLFrameBuffer.h"

    
DGLFrameBuffer::DGLFrameBuffer( int w, int h, std::string name )
              : _width(w), _height(h), _name(name)
{
    // generate an ID for the framebuffer object
    glGenFramebuffers( 1, &_fboID );
}


void DGLFrameBuffer::addColorBuffer( int attachmentPoint, GLenum type )
{
    // make sure there's enough room in the attachments vector
    if( (int)_colorAttachments.size() < attachmentPoint + 1 )
        _colorAttachments.resize( attachmentPoint + 1 );

    bind();
    
    // Now setup a texture to render to
    glGenTextures( 1, &_colorAttachments[attachmentPoint].bufferID );
    glBindTexture( GL_TEXTURE_2D, _colorAttachments[attachmentPoint].bufferID );
    glTexImage2D( GL_TEXTURE_2D, 0, type, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    // to (eventually) get mipmapping working, we'll need to do something like:
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glGenerateMipmapEXT(GL_TEXTURE_2D);

    // And attach it to the FBO so we can render to it
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_2D, _colorAttachments[attachmentPoint].bufferID, 0 );
    
    unbind();
}


void DGLFrameBuffer::addDepthBuffer()
{
    // Now setup a texture to render to
    glGenTextures( 1, &_depthAttachment.bufferID );
    glBindTexture( GL_TEXTURE_2D, _depthAttachment.bufferID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _width, _height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );

    bind();
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthAttachment.bufferID, 0 );
    unbind();
}


void DGLFrameBuffer::attachExistingColorBuffer( int attachmentPoint, GLuint bufferID, bool takeOwnership )
{
    // make sure there's enough room in the attachments vector
    if( (int)_colorAttachments.size() < attachmentPoint + 1 )
        _colorAttachments.resize( attachmentPoint + 1 );
   
    // attach the given buffer to the FBO
    _colorAttachments[attachmentPoint].bufferID = bufferID;
    _colorAttachments[attachmentPoint].owned = takeOwnership;

    bind();
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_2D, _colorAttachments[attachmentPoint].bufferID, 0 );
    unbind();
}


void DGLFrameBuffer::attachExistingDepthBuffer( GLuint bufferID, bool takeOwnership )
{
    _depthAttachment.bufferID = bufferID;
    _depthAttachment.owned = takeOwnership;

    bind();
    glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, bufferID, 0 );
    unbind();
}


DGLFrameBuffer::~DGLFrameBuffer()
{
    // delete the depth texture (if this object owns it)
    if( _depthAttachment.owned )
        glDeleteTextures( 1, &_depthAttachment.bufferID );

    // same with the attached color buffer textures
    for( int i = 0; i < (int)_colorAttachments.size(); i++ ) {
        if( _colorAttachments[i].owned )
            glDeleteTextures( 1, &_colorAttachments[i].bufferID );
    }
    _colorAttachments.clear();

    // finally get rid of the framebuffer itself
    glDeleteFramebuffers( 1, &_fboID );
}



void DGLFrameBuffer::bind()
{
    GLint tmp;
 
    // save the previously bound FBO   
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &tmp );
    _lastBoundBuffer = tmp;
    
    // First we bind the FBO so we can render to it
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
  
    // adjust the viewport for this FBO
    glPushAttrib( GL_VIEWPORT_BIT );
    glViewport( 0, 0, _width, _height );
}


void DGLFrameBuffer::unbind()
{
    // pops the viewport bit
    glPopAttrib();
    
    // rebind whatever was bound before this FBO
    glBindFramebuffer( GL_FRAMEBUFFER, _lastBoundBuffer );
}


void DGLFrameBuffer::clear()
{
    GLbitfield mask = 0;

    // build a mask of stuff to clear
    if( _colorAttachments.size() )
        mask |= GL_COLOR_BUFFER_BIT;
    if( _depthAttachment.bufferID )
        mask |= GL_DEPTH_BUFFER_BIT;

    glClear( mask );
}


bool DGLFrameBuffer::checkStatus( bool printError )
{
    bool statusOK = true;

    // bind the framebuffer so we can check it
    bind();
    
    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    
    // did the setup work correctly?
    if( status != GL_FRAMEBUFFER_COMPLETE ) {

        // nope.
        statusOK = false;

        // (possibly) print out the 
        if( printError ) {
            if( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT ) {
                printf( "FBO is missing a required image/buffer attachment\n" );
            } else if( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT ) {
                printf( "FBO has no images/buffers attached\n" );
            } else if( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER ) {
                printf( "FBO is trying to draw to a color buffer that has no attachment\n" );
            } else if( GL_FRAMEBUFFER_UNSUPPORTED ) {
                printf( "The FBO format is not supported by current graphics card or driver\n" );
            } else {
                printf( "Unknown error from glCheckFramebufferStatus\n" );
            }
        }
    }
    
    unbind();
    return statusOK;
}


void DGLFrameBuffer::enableOutputBuffers( int buf0, int buf1, int buf2, int buf3, int buf4, int buf5, int buf6, int buf7 )
{
    GLenum outputBuffers[8];
    int numBufs = 0;

    // zero means don't enable the given output buffer. This is only useful if the user
    // is enabling non-sequential output buffers, which would be weird but possible.    
    memset( outputBuffers, 0, sizeof(GLenum)*8 );

    if( buf0 >= 0 ) { outputBuffers[0] = GL_COLOR_ATTACHMENT0 + buf0; numBufs = 1; }
    if( buf1 >= 0 ) { outputBuffers[1] = GL_COLOR_ATTACHMENT0 + buf1; numBufs = 2; }
    if( buf2 >= 0 ) { outputBuffers[2] = GL_COLOR_ATTACHMENT0 + buf2; numBufs = 3; }
    if( buf3 >= 0 ) { outputBuffers[3] = GL_COLOR_ATTACHMENT0 + buf3; numBufs = 4; }
    if( buf4 >= 0 ) { outputBuffers[4] = GL_COLOR_ATTACHMENT0 + buf4; numBufs = 5; }
    if( buf5 >= 0 ) { outputBuffers[5] = GL_COLOR_ATTACHMENT0 + buf5; numBufs = 6; }
    if( buf6 >= 0 ) { outputBuffers[6] = GL_COLOR_ATTACHMENT0 + buf6; numBufs = 7; }
    if( buf7 >= 0 ) { outputBuffers[7] = GL_COLOR_ATTACHMENT0 + buf7; numBufs = 8; }
    
    glDrawBuffers( numBufs, outputBuffers );
}


GLuint DGLFrameBuffer::frameBufferID()
{
    return _fboID;
}


GLuint DGLFrameBuffer::colorBufferID( int attachmentPoint )
{
    return _colorAttachments[attachmentPoint].bufferID;
}


GLuint DGLFrameBuffer::depthBufferID()
{
    return _depthAttachment.bufferID;
}


std::string DGLFrameBuffer::name()
{
    return _name;
}
