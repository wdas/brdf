#include "glerror.h"

#include "Sphere.h"

Sphere::Sphere(float radius, int nU, int nV) :
    _radius(radius), _nU(nU), _nV(nV), _ready(false)
{
    int nVertices  = (nU + 1) * (nV + 1);
    int nTriangles =  nU * nV * 2;

    _vertices.resize(nVertices);
    _normals.resize(nVertices);
    _tangents.resize(nVertices);
    _texCoords.resize(nVertices);
    _colors.resize(nVertices);
    _indices.resize(3*nTriangles);

    for(int v=0;v<=nV;++v)
    {
        for(int u=0;u<=nU;++u)
        {

            float theta = u / float(nU) * M_PI;
            float phi 	= v / float(nV) * M_PI * 2;

            int index 	= u +(nU+1)*v;

            glm::vec3 vertex, tangent, normal;
            glm::vec2 texCoord;

            // normal
            normal[0] = sin(theta) * cos(phi);
            normal[1] = sin(theta) * sin(phi);
            normal[2] = cos(theta);
            normal = glm::normalize(normal);

            // position
            vertex = normal * _radius;

            // tangent
            theta += M_PI_2;
            tangent[0] = sin(theta) * cos(phi);
            tangent[1] = sin(theta) * sin(phi);
            tangent[2] = cos(theta);
            tangent = glm::normalize(tangent);

            // texture coordinates
            texCoord[1] = u / float(nU);
            texCoord[0] = v / float(nV);

            _vertices[index] = vertex;
            _normals[index] = normal;
            _tangents[index] = tangent;
            _texCoords [index] = texCoord;
            _colors[index] = glm::vec3(0.6f,0.2f,0.8f);
        }
    }

    int index = 0;
    for(int v=0;v<nV;++v)
    {
        for(int u=0;u<nU;++u)
        {
            int vindex 	= u + (nU+1)*v;

            _indices[index+0] = vindex;
            _indices[index+1] = vindex+1 ;
            _indices[index+2] = vindex+1 + (nU+1);

            _indices[index+3] = vindex;
            _indices[index+4] = vindex+1 + (nU+1);
            _indices[index+5] = vindex   + (nU+1);

            index += 6;
        }
    }
}

Sphere::~Sphere()
{
    glf->glDeleteVertexArrays(1, &_vao);
    glf->glDeleteBuffers(6, _bufs);
}

void Sphere::init()
{
    glf->glGenVertexArrays(1, &_vao);
    glf->glGenBuffers(6, _bufs);

    glf->glBindVertexArray(_vao);

    glf->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufs[0]);
    glf->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[1]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[2]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _colors.size(), _colors.data(), GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[3]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _normals.size(), _normals.data(), GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[4]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * _tangents.size(), _tangents.data(), GL_STATIC_DRAW);

    glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[5]);
    glf->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * _texCoords.size(), _texCoords.data(), GL_STATIC_DRAW);

    glf->glBindVertexArray(0);

    _ready = true;
}

void Sphere::draw(const DGLShader *shader)
{
    if (!_ready) {
        init();
    }

    glf->glBindVertexArray(_vao);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[1]);
        glf->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(vertex_loc);
    }

    int color_loc = shader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[2]);
        glf->glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(color_loc);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[3]);
        glf->glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(normal_loc);
    }

    int tangent_loc = shader->getAttribLocation("vtx_tangent");
    if(tangent_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[4]);
        glf->glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(tangent_loc);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texCoord");
    if(texCoord_loc>=0){
        glf->glBindBuffer(GL_ARRAY_BUFFER, _bufs[5]);
        glf->glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glf->glEnableVertexAttribArray(texCoord_loc);
    }

    glf->glDrawElements(GL_TRIANGLES, _indices.size(),  GL_UNSIGNED_INT, 0);

    glf->glBindVertexArray(0);
}
