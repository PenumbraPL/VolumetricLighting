#version 450 core

layout (binding = 0) uniform sampler2D tex_envmap;
out vec4 color;

in VS_OUT
{
vec2 texCoor;
} fs_in;

void main(void){

color = texture(tex_envmap, fs_in.texCoor);
}