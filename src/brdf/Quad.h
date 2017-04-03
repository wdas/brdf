#ifndef _QUAD_H_
#define _QUAD_H_

#include "BRDFBase.h"
#include "DGLShader.h"

class Quad : GLContext
{
public:
    Quad();
    Quad(float x1, float y1, float x2, float y2);
    Quad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2);
    ~Quad();

    void init();

    void setTexCoord(unsigned int id, float x, float y);
    void setPos(unsigned int id, float x, float y);

    void draw(const DGLShader* shader);

private:
    float _texCoord[8];
    float _pos[8];
    unsigned int _indices[6];

    bool _ready;

    GLuint _vao;
    GLuint _bufs[3];
};

#endif
