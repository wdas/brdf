#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec3 vtx_position;
in vec3 vtx_normal;

out vec3 normal;

void main(void)
{
    normal = vtx_normal;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(vtx_position,1);
}
