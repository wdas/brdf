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

#ifndef _SHARED_CONTEXT_GL_WIDGET_H
#define _SHARED_CONTEXT_GL_WIDGET_H

#ifndef OPENGL_MAJOR_VERSION
  #define OPENGL_MAJOR_VERSION 4
#endif
#ifndef OPENGL_MINOR_VERSION
  #define OPENGL_MINOR_VERSION 1
#endif

#define OPENGL_CORE_FUNCS_HELPER2(a, b) QOpenGLFunctions_##a##_##b##_Core
#define OPENGL_CORE_FUNCS_HELPER(a, b) OPENGL_CORE_FUNCS_HELPER2(a, b)
#define OPENGL_CORE_FUNCS OPENGL_CORE_FUNCS_HELPER(OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION)
#define QUOTE(a) #a
#define QUOTE_AND_EXPAND(a) QUOTE(a)
#define OPENGL_CORE_FUNCS_INCLUDE QUOTE_AND_EXPAND(OPENGL_CORE_FUNCS)

#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWindow>

#include OPENGL_CORE_FUNCS_INCLUDE

#include "ShowingBase.h"
#include "glerror.h"

/*

The point of this class:
Each GL view has its own context, but we want to be able to share textures, etc.
between contexts; this class makes that possible without too much pain. The first
widget created gets saved in a static variable, and any further widgets attempt to
establish sharing with that first widget.

A bit of a hack, but it works.

*/

class GLContext
{
public:
    typedef OPENGL_CORE_FUNCS GlFuncs;

    static void initOpenGLContext(QWindow *window);
    static void cleanOpenGLContext();
    static GlFuncs* glFuncs() { return glf; }
    static QSurfaceFormat surfaceFormat();

protected:
    static int shareCount;

    static QOpenGLContext *glcontext;
    static GlFuncs        *glf;
};

class GLWindow : public QWindow, public ShowingBase, public GLContext
{
public:
    GLWindow(QWindow *parent);
    virtual ~GLWindow() {}

    virtual void updateGL();
    virtual void initializeGL() = 0;
    virtual void paintGL() = 0;

protected:
    void exposeEvent(QExposeEvent *ev);
};

#endif
