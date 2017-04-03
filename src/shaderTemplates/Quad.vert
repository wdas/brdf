#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec2 vtx_position;
in vec2 vtx_texCoord;

out vec2 texCoord;

void main(void)
{
    texCoord = vtx_texCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(vtx_position,0,1);
}
