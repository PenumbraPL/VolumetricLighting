#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNor; // if not used it is deleted
layout (location = 2) in vec2 vTex; // if not used it is deleted

out VS_OUT{
vec3 _normal;
vec3 _color;
vec2 _texCoor;
vec3 _view;
vec3 _position;
}vs_out;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

uniform mat4 MVP;
uniform mat4 PRJ;

void main()
{
    vs_out._texCoor = vTex;
    vec4 g = MVP * vec4(vNor, 0.);
    vs_out._normal = g.xyz;
    vs_out._position = vPos;
    vs_out._view = vec4(MVP * vec4(vPos, 0.)).xyz;
    gl_Position =  PRJ * MVP * vec4(vPos, 1.0);
    vs_out._color = vec3(1.0, 1.0, 1.0);
}
