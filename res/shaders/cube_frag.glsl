#version 450

layout (binding = 0) uniform samplerCube tex_cubemap;
out vec4 color;

in VS_OUT
{
vec3 normal;
vec3 view;
} fs_in;

void main(void){
vec3 r = reflect(fs_in.view, normalize(fs_in.normal));
color = texture(tex_cubemap, r);
}