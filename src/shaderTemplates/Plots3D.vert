#version 410

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec3 vtx_position;
in vec3 vtx_color;

out vec3 vs_pos;
out vec3 vs_color;

void main(void)
{
    vs_color = vtx_color;
    vec4 view_pos = modelViewMatrix * vec4(vtx_position,1);
    vs_pos = view_pos.xyz;
    gl_Position = projectionMatrix * view_pos;
}
