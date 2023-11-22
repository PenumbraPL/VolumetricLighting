#version 450

uniform mat4 MVP;
in vec3 vPos;
out vec3 _color;

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    _color = vec3(1.0, 1.0, 1.0);
}
