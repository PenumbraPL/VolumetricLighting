#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNor;
layout (location = 2) in vec2 vTex;
uniform mat4 MVP;
uniform mat4 PRJ;

out VS_OUT
{
    vec3 _normal;
    vec3 _color;
    vec2 _texCoords;
    vec3 _view;
    vec3 _position;
} vs_out;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


void main()
{
    vs_out._normal = (MVP * vec4(vNor, 0.)).xyz;
    vs_out._color = vec3(1.0, 1.0, 1.0);    
    vs_out._texCoords = vTex;
    vs_out._view = vec4(MVP * vec4(vPos, 0.)).xyz;
    vs_out._position = vPos;

    gl_Position =  PRJ * MVP * vec4(vPos, 1.0);
}
