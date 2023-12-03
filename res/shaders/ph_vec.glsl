#version 450 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

layout (std140) uniform constants
{
mat4 mv_matrix;
mat4 projection;
mat4 view_matrix;
};

unifrom vec3 light_pos = vec3(100.0, 100.0, 100.0);

void main(void){
vec4 P = mv_matrix*positoin;
vs_out.N = mat3(mv_matrix) * normal;
vs_out.L = light_pos - P.xyz;
vs_out.V = -P.xyz;
gl_Position = projection * P;
}