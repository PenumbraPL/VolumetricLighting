#version 450

layout (location = 0) out vec4 color;

in VS_OUT
{
vec3 N;
vec3 L;
vec3 V;
} fs_in;

unifrom vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
unifrom vec3 specular_albedo = vec3(0.7);
unifrom float specular_power = 128.0;

void main(void){

vec3 N = normalize(fs_in.N);
vec3 L = normalize(fs_in.L);
vec3 V = normalize(fs_in.V);
vec3 H = normalize(L + V);


vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;
vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

color = vec4(diffuse + specular, 1.0);
}