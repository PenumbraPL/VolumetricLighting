#version 450 core

uniform mat4 mat_matrix;

out VS_OUT
{
vec3 tc;
} vs_out;

void main(void){

vec3[4] vertices = vec3[4](vec3(-1.0, -1.0, 1.0),
							vec3(1.0, -1.0, 1.0),
							vec3(-1.0, 1.0, 1.0),
							vec3(1.0, 1.0, 1.0));

vs_out.tc = mat3(view_matrix) * vertices[gl_VertexID];

gl_Position = vec4(verices[gl_VertexID], 1.0);
}