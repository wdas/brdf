#version 410

uniform vec3 drawColor;

in vec3 normal;

out vec4 fragColor;

void main(void)
{

    fragColor = vec4(drawColor,1);
}
