// from http://github.prideout.net/strings-inside-vertex-buffers/

#version 410

uniform sampler2D Sampler;
uniform vec3 TextColor;

in vec2 gTexCoord;
out vec4 fragColor;

void main()
{
    float A = texture(Sampler, gTexCoord).r;
    fragColor = vec4(TextColor, A);
}
