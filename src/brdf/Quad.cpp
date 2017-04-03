#include "glerror.h"

#include "Quad.h"

#include <iostream>

using namespace std;

Quad::Quad()
    :_ready(false)
{
    Quad(-1.f, -1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f);
}

Quad::Quad(float x1, float y1, float x2, float y2)
    :_ready(false)
{
    Quad(x1, y1, x2, y2, 0.f, 0.f, 1.f, 1.f);
}

Quad::Quad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2)
    :_ready(false)
{
    setTexCoord(0, u1, v1);
    setPos     (0, x1, y1);

    setTexCoord(1, u2, v1);
    setPos     (1, x2, y1);

    setTexCoord(2, u2, v2);
    setPos     (2, x2, y2);

    setTexCoord(3, u1, v2);
    setPos     (3, x1, y2);

    _indices[0] = 0;
    _indices[1] = 1;
    _indices[2] = 3;

    _indices[3] = 1;
    _indices[4] = 2;
    _indices[5] = 3;
}

Quad::~Quad()
{
    glf->glDeleteVertexArrays(1, &_vao);
    glf->glDeleteBuffers(3, _bufs);
}

void  Quad::init()
{
    glf->glGenVertexArrays(1, &_vao);
    glf->glGenBuffers(3, _bufs);

    glf->glBindVertexArray(_vao);

    glf->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[0]);
    glf->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 6, _indices,  GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[1]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, _pos, GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[2]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, _texCoord, GL_STATIC_DRAW);

    glf->glBindVertexArray(0);

    _ready = true;
}


void Quad::setTexCoord(unsigned int id, float x, float y)
{
    _texCoord[2*id  ] = x;
    _texCoord[2*id+1] = y;
}


void Quad::setPos(unsigned int id, float x, float y)
{
    _pos[2*id  ] = x;
    _pos[2*id+1] = y;
}


void Quad::draw(const DGLShader* shader)
{
    if (!_ready) {
        init();
    }

    glf->glBindVertexArray(_vao);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[1]);
        glf->glVertexAttribPointer(vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(vertex_loc);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texCoord");
    if(texCoord_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[2]);
        glf->glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(texCoord_loc);
    }

    glf->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glf->glBindVertexArray(0);
}
