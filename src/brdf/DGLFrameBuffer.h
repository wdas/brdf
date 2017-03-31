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

#ifndef _DGL_FRAME_BUFFER_H_
#define _DGL_FRAME_BUFFER_H_

#include "SharedContextGLWidget.h"

#include <vector>
#include <string>

class DGLFrameBuffer : public GLContext
{
private:

    // structure for tracking textureIDs, and whether the DGLFrameBuffer
    // "owns" them (and should therefore delete them on destruction)
    struct BufferAttachment
    {
        BufferAttachment() : bufferID(0), owned(true) {}
        GLuint bufferID;
        bool owned;
    };

public:        
    DGLFrameBuffer( int width, int height, std::string name = "" );
    ~DGLFrameBuffer();

    // bind and unbind the FBO (so we can render to it)
    void bind();
    void unbind();
    
    // calls glClear for the attached buffer(s)
    void clear();

    // returns the width and height for the attached buffers
    int width() { return _width; }
    int height() { return _height; }
    
    // returns IDs for the FBO and the different attached textures/buffers
    GLuint frameBufferID();
    GLuint colorBufferID( int attachmentPoint = 0 );
    GLuint depthBufferID();
    
    // used for MRT: enable the different buffers to render to
    void enableOutputBuffers( int buf0 = -1, int buf1 = -1, int buf2 = -1, int buf3 = -1,
                              int buf4 = -1, int buf5 = -1, int buf6 = -1, int buf7 = -1 );
    
    // create and add buffers to the framebuffer object
    void addColorBuffer( int attachmentPoint, GLenum type = GL_RGBA8 );
    void addDepthBuffer();

    // attach existing buffers to the framebuffer object
    void attachExistingColorBuffer( int attachmentPoint, GLuint bufferID, bool takeOwnership = false );
    void attachExistingDepthBuffer( GLuint bufferID, bool takeOwnership = false );

    // check to see if the FBO setup worked
    bool checkStatus( bool printError = true );

    // return the (optional) name given to this FBO
    std::string name();
    
private:

    // the ID of the FBO itself
    GLuint _fboID;
    
    // the last bound FBO (so we can reattach it after unbinding)
    GLint  _lastBoundBuffer;

    // IDs of the different textures/buffers attached to this FBO
    std::vector<BufferAttachment> _colorAttachments;
    BufferAttachment _depthAttachment;

    // dimensions of the attached textures/buffers
    int _width, _height;
    // dimensions of the current viewport
    int _viewport[4];

    // (optional) name of the FBO
    std::string _name;
};

#endif
