#version 450 core

uniform mat4 MVP;
uniform mat4 PRJ;

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

out VS_OUT
{
vec2 texCoor;
} vs_out;

void main(void){
gl_Position = MVP * vec4(vPos, 1.0);
vs_out.texCoor = vTex;
}