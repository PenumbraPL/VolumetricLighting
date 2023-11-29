#version 450 core

layout (binding = 1) uniform sampler2D tex_color;
out vec4 color;

in TES_OUT
{
vec2 tc;
vec3 world_view;
vec3 eye_coord;
} fs_in;

vec4 fog(vec4 c){
flaot z = length(fs_in.eye_coord);
float de = 0.0025 * smoothstep(0.0, 6.0, 10.0 - fs_in.world_coord.y);
float di = 0.0045 * smoothstep(0.0, 40.0, 20.0 - fs_in.world_coord.y);

float extinction = exp(-z*de);
float inscattering = exp(-z * di);

return c* extinction + fog_color * (1.0 - inscattering);
}

void main(void){
vec4 landscape = texture(tex_color, fs_in.tc);
if(enable_fog){
color = fog(landscape);

}else{
}
color = landscape;

}