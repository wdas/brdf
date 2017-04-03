// from http://github.prideout.net/strings-inside-vertex-buffers/

#version 410

in int Character;
out int vCharacter;
out int vPosition;

void main()
{
    vCharacter = Character;
    vPosition = gl_VertexID;
    gl_Position = vec4(0, 0, 0, 1);
}
