#version 410

uniform float thickness;
const float antialias = 5.0;

in vec3  v_color;
in vec2  v_texCoord;

out vec4 fragColor;

void main(void)
{
    float distance = v_texCoord.y;
    float d = abs(distance) - thickness + antialias;
    float alpha = 1.0;
    if( d > 0.0 )
    {
        alpha = d/(antialias);
        alpha = exp(-alpha*alpha);
    }
    fragColor = vec4(v_color,alpha);
}
