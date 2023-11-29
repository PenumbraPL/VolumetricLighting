#version 450 core

uniform mat4 mat_matrix;
uniform mat4 projection;
uniform float dmap_depth;
uniform sampler2D tex_displaysment;

layout (quads, franctional_odd_spacing) in;

out vec2 tc;

out TCS_OUT
{
vec2 tc;
} tes_in[];

in TES_OUT
{
vec2 tc;
vec3 world_view;
vec3 eye_coord;
} tes_out;


void main(void){
vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, gl_tessCoord.x);
vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, gl_tessCoord.x);
vec2 tc = mix(tc2, tc1, gl_TessCoord.y);

vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gs_TessCoord.x);
vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gs_TessCoord.x);
vec4 p = mix(p2, p1, gl_TessCoord.y);

p.y += texture(tex_displacement, tc).r * dmap_depth;

vec4 P_eye = mv_matrix * p;

tes_out.tc = tc;
tes_out.world_coord = p.xyz;
tes_out.eye_coord = P_eye.xyz;

gl_Position = projection * P_eye;
}