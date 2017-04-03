#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec3 vtx_position;
in vec3 vtx_color;

out vec3 vs_color;

void main(void)
{
    vs_color = vtx_color;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(vtx_position,1);
}
