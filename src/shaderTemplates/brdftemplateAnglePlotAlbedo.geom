#version 410

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 6) out;

uniform vec2 viewport_size;
uniform float thickness;

in vec4 v_albedoData[];

out vec2 v_texCoord;
out vec4 albedoData;

void main(void)
{
    vec2 prev  = (gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w) * viewport_size;
    vec2 start = (gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w) * viewport_size;
    vec2 end   = (gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w) * viewport_size;
    vec2 next  = (gl_in[3].gl_Position.xy/gl_in[3].gl_Position.w) * viewport_size;

    // determine the tangent of each of the 3 segments (previous, current, next)
    vec2 v0 = normalize(start-prev);
    vec2 v1 = normalize(end-start);
    vec2 v2 = normalize(next-end);

    // determine the normal of each of the 3 segments
    vec2 n0 = vec2(-v0.y, v0.x);
    vec2 n1 = vec2(-v1.y, v1.x);
    vec2 n2 = vec2(-v2.y, v2.x);

    // mittering
    vec2 miter_a = normalize(n0 + n1);	// miter at start of current segment
    vec2 miter_b = normalize(n1 + n2);	// miter at end of current segment

    // determine the length of the miter by projecting it onto normal and then inverse it
    float length_a = thickness / dot(miter_a, n1);
    float length_b = thickness / dot(miter_b, n1);

    vec2 a = (start - length_a * miter_a);
    vec2 b = (end   - length_b * miter_b);
    vec2 c = (end   + length_b * miter_b);
    vec2 d = (start + length_a * miter_a);

    float l = length(end - start);

    if( prev == start )
        gl_Position = vec4((start - n1*thickness) / viewport_size,0,1);
    else
        gl_Position = vec4((a / viewport_size),0,1);
    v_texCoord = vec2(0,thickness);
    albedoData = v_albedoData[1];
    EmitVertex();

    if( prev == start )
        gl_Position = vec4((start + n1*thickness) / viewport_size,0,1);
    else
        gl_Position = vec4((d / viewport_size),0,1);
    v_texCoord = vec2(0,-thickness);
    albedoData = v_albedoData[1];
    EmitVertex();

    if( end == next )
        gl_Position = vec4((end - n1*thickness) / viewport_size,0,1);
    else
        gl_Position = vec4((b / viewport_size),0,1);
    v_texCoord = vec2(l,-thickness);
    albedoData = v_albedoData[2];
    EmitVertex();

    if( end == next )
        gl_Position = vec4((end + n1*thickness) / viewport_size,0,1);
    else
        gl_Position = vec4((c / viewport_size),0,1);
    v_texCoord = vec2(l,thickness);
    albedoData = v_albedoData[2];
    EmitVertex();

    EndPrimitive();
}
