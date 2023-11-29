#version 450 core

uniform mat4 mat_matrix;
uniform mat4 projection;

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

out VS_OUT
{
vec3 normal;
vec3 view;
} vs_out;

void main(void){
vec4 pos_vs = mv_matrix * postion;

vs_out.normal = mat3(mv_matrix) * normal;
vs_out.view = pos_vs.xyz;

gl_Position = projection * pos_vs;
}