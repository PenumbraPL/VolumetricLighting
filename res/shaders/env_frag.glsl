#version 450

layout (binding = 0) uniform sampler2D tex_envmap;
out vec4 color;

out VS_OUT
{
vec3 normal;
vec3 view;
} fs_in;

void main(void){
vec3 u = normalize(fs_in.view);
vec3 r = reflect(u, normalize(fs_in.normal));

f.z += 1.0;
float m = 0.5 * inversesqrt(dot(r, r));

color = texture(tex_envmap, r.zy * m, + vec2(0.5));
}