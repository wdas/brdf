#ifndef _SPHERE_H
#define _SPHERE_H

#if defined(_MSC_VER)
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#else
#include <cmath>
#endif

#include <vector>
#include <glm/glm.hpp>

#include "DGLShader.h"

class Sphere : GLContext
{

public:
    Sphere(float radius=1.f, int nU=100, int nV=100);
    ~Sphere();
    void init();
    void draw(const DGLShader *shader);
    float radius() const { return _radius; }
    int lats() const { return _nU; }
    int longs() const { return _nV; }

private :
    GLuint _vao;
    GLuint _bufs[6];

    std::vector<int>            _indices;   /* vertex indices */
    std::vector<glm::vec3>		_vertices;  /* 3D positions */
    std::vector<glm::vec3>		_colors;    /* colors */
    std::vector<glm::vec3>		_normals;   /* 3D normals */
    std::vector<glm::vec3>		_tangents;  /* 3D tangent to surface */
    std::vector<glm::vec2>		_texCoords; /* 2D texture coordinates */

    float _radius;
    int   _nU, _nV;
    bool  _ready;
};

#endif
