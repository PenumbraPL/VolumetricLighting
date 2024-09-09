#version 450 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;
uniform mat4 MV = mat4(1.);
uniform mat4 PRJ = mat4(1.);


out VS_OUT
{
	vec2 _texCoords;
} vs_out;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


void main(void){
	gl_Position = PRJ * MV * vec4(vPos, 1.0);
	vs_out._texCoords = vTex;
}