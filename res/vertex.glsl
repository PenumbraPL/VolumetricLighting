#version 450

uniform mat4 MVP;
in vec3 vCol;
in vec2 vPos;
out vec3 _color;

void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    _color = vCol;
}
