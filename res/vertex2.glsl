#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNor; // if not used it is deleted

out VS_OUT{
vec3 _normal;
vec3 _color;
}vs_out;

uniform mat4 MVP;

void main()
{
    vs_out._normal = vNor + vec3(1.0,1.0,1.0);
    gl_Position = MVP * vec4(vPos, 1.0);
    vs_out._color = vec3(1.0, 1.0, 1.0);
}
