#version 450

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

layout (std140) uniform constants
{
mat4 mv_matrix;
mat4 projectin;
mat4 view_matrix;
};

unifrom vec3 light_pos = vec3(100.0, 100.0, 100.0);
unifrom vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
unifrom vec3 specular_albedo = vec3(0.7);
unifrom float specular_power = 128.0;
unifrom vec3 ambient = vec3(0.1, 0.1, 0.1);

out VS_OUT
{
vec3 color;
} vs_out;

void main(void){
vec4 P = mv_matrix * position;
vec3 N = mat3(mv_matrix) * normal;
vec3 L = light_pos - P.xyz;
vec3 V = -P.xyz;

N = normalize(N);
L = normalize(L);
V = normalize(V);

vec3 R = reflect(-L, N);
vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;

vs_out.color = ambient + diffuse + specular;

gl_Position = projection * P;

}