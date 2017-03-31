#version 410

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

uniform vec2 viewport_size;
uniform float thickness;

in vec3 vs_color[];

out vec3  v_color;
out vec2  v_texCoord;


void main(void)
{
    vec2 start  = (gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w) * viewport_size;
    vec2 end    = (gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w) * viewport_size;

    vec2 axis = end - start;
    float l = length(axis);
    axis = normalize(axis);
    vec2 side = vec2(-axis.y,axis.x);

    vec2 a = (start + side*thickness);
    vec2 b = (end   + side*thickness);
    vec2 c = (end   - side*thickness);
    vec2 d = (start - side*thickness);

    gl_Position = vec4(a / viewport_size,0,1);
    v_color = vs_color[0];
    v_texCoord = vec2(0,thickness);
    EmitVertex();

    gl_Position = vec4(d / viewport_size,0,1);
    v_color = vs_color[0];
    v_texCoord = vec2(0,-thickness);
    EmitVertex();

    gl_Position = vec4(b / viewport_size,0,1);
    v_color = vs_color[1];
    v_texCoord = vec2(l,thickness);
    EmitVertex();

    gl_Position = vec4(c / viewport_size,0,1);
    v_color = vs_color[1];
    v_texCoord = vec2(l,-thickness);
    EmitVertex();

EndPrimitive();
}
