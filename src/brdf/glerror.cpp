#include "glerror.h"
#include <iostream>
#include <cstdio>
#include <cstring>

#include "SharedContextGLWidget.h"

static const struct {
    GLenum code;
    const char *string;
} errors[]= {
    {GL_NO_ERROR, "no error"},
    {GL_INVALID_ENUM, "invalid enumerant"},
    {GL_INVALID_VALUE, "invalid value"},
    {GL_INVALID_OPERATION, "invalid operation"},
    {GL_OUT_OF_MEMORY, "out of memory"},
    {GL_INVALID_FRAMEBUFFER_OPERATION, "Invalid framebuffer operation"},
    {0, "(unrecognized error code)" }
};

const char* GLErrorString(GLenum errorCode)
{
    for(int i=0; errors[i].string; i++)
    {
        if (errors[i].code == errorCode)
        {
            return errors[i].string;
        }
    }

    return NULL;
}

bool QueryExtension(const char *extName)
{
    /*
    ** Search for extName in the extensions string. Use of strstr()
    ** is not sufficient because extension names can be prefixes of
    ** other extension names. Could use strtok() but the constant
    ** string returned by glGetString might be in read-only memory.
    */
    char *p;
    char *end;
    int extNameLen;

    extNameLen = strlen(extName);

    p = (char *)GLContext::glFuncs()->glGetString(GL_EXTENSIONS);
    if (NULL == p) {
        return false;
    }

    end = p + strlen(p);

    while (p < end) {
        int n = strcspn(p, " ");
        if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
            return true;
        }
        p += (n + 1);
    }
    return false;
}

int PrintOpenGLError(const char *file, int line, const char *msg)
{
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    int retCode = 0;
    GLenum glErr = GLContext::glFuncs()->glGetError();
    while (glErr != GL_NO_ERROR)
    {
        const char *errDescr = (const char *)GLErrorString(glErr);
        if (! errDescr) {
            if (glErr == GL_INVALID_FRAMEBUFFER_OPERATION)
                errDescr = "Invalid framebuffer operation (ext)";
            else
                errDescr = "(unrecognized error code)";
        }

        char output[1000];
        sprintf(output, "OpenGL Error %s (%d) in file %s:%d%s%s%s",
                errDescr, glErr, file, line,
                msg ? " (" : "",
                msg ? msg : "",
                msg ? ")" : "");

        std::cout << output << std::endl;

        retCode = 1;

        glErr = GLContext::glFuncs()->glGetError();
    }
    return retCode;
}
