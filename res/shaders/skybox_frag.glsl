#version 450

layout (binding = 0) uniform samplerCube tex_cubemap;
layout (location = 0) out vec4 color;

in VS_OUT
{
vec3 tc;
} fs_in;

void main(void){
color = texture(tex_cubemap, fs_in.tc);
}