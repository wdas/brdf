#version 410

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

uniform mat4 projectionMatrix;
uniform vec2 viewport_size;
uniform float nearPlane_dist;
uniform float thickness;

in vec3 vs_color[];
in vec3 vs_pos[];

out vec3  v_color;
out vec2  v_texCoord;

void clipLineSegmentToNearPlane(vec3 p0, vec3 p1,
    out vec3 positionWC, out bool culledByNearPlane)
{
    culledByNearPlane = false;
    
    vec3 p1ToP0 = p1 - p0;
    float magnitude = length(p1ToP0);
    vec3 direction = normalize(p1ToP0);
    float endPoint0Distance =  -(nearPlane_dist + p0.z);
    float denominator = -direction.z;
    
    if (endPoint0Distance < 0.0 && abs(denominator) < 0.001)
    {
        // the line segment is parallel to and behind the near plane
        culledByNearPlane = true;
    }
    else if (endPoint0Distance < 0.0 && abs(denominator) > 0.001)
    {
        // ray-plane intersection:
        //  t = (-plane distance - dot(plane normal, ray origin))
        //  t /= dot(plane normal, ray direction);
        float t = (nearPlane_dist + p0.z) / denominator;
        
        if (t < 0.0 || t > magnitude)
        {
            // the segment intersects the near plane, 
            // but the entire segment is behind the  near plane
            culledByNearPlane = true;
        }
        else
        {
            // segment intersects the near plane, find intersection
            p0 = p0 + t * direction;
        }
    }
    
    vec4 positionNDC = projectionMatrix * (vec4(p0, 1.0));
    vec3 position = 
    positionWC = (positionNDC.xyz / positionNDC.w) * vec3(viewport_size,1);
}

void main(void)
{
    vec3 start, end;
    bool culled;
    clipLineSegmentToNearPlane(vs_pos[0],vs_pos[1],start,culled);
    if(culled) return;
    clipLineSegmentToNearPlane(vs_pos[1],vs_pos[0],end,culled);

    vec2 axis = end.xy - start.xy;
    float l = length(axis);
    axis = normalize(axis);
    vec2 side = vec2(-axis.y,axis.x);

    vec2 a = (start.xy + side*thickness);
    vec2 b = (end.xy   + side*thickness);
    vec2 c = (end.xy   - side*thickness);
    vec2 d = (start.xy - side*thickness);

    gl_Position = vec4(a / viewport_size,start.z,1);
    v_color = vs_color[0];
    v_texCoord = vec2(0,thickness);
    EmitVertex();

    gl_Position = vec4(d / viewport_size,start.z,1);
    v_color = vs_color[0];
    v_texCoord = vec2(0,-thickness);
    EmitVertex();

    gl_Position = vec4(b / viewport_size,end.z,1);
    v_color = vs_color[1];
    v_texCoord = vec2(l,thickness);
    EmitVertex();

    gl_Position = vec4(c / viewport_size,end.z,1);
    v_color = vs_color[1];
    v_texCoord = vec2(l,-thickness);
    EmitVertex();

EndPrimitive();
}
