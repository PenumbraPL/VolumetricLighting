#version 450 core

layout (binding = 0) uniform sampler2D amb_tex;
layout (binding = 1) uniform sampler2D emi_tex;
layout (binding = 2) uniform sampler2D dif_tex;
layout (binding = 3) uniform sampler2D sp_tex;
layout (binding = 4) uniform sampler2D sp_gl_tex;
layout (binding = 5) uniform sampler2D mr_tex;
layout (binding = 6) uniform sampler2D alb_tex;
layout (binding = 7) uniform sampler2D sp_dif_tex;
layout (binding = 8) uniform sampler2D tex_envmap;

uniform bool isTexture = true;

in VS_OUT {
vec3 _normal;
vec3 _color;
vec2 _texCoor;
vec3 _view;
}fs_in;

out vec4 color;

void main()
{
    if(isTexture){
        color = texture(alb_tex, fs_in._texCoor);
    }else{
        color = vec4(fs_in._normal, 1.0);
    }
}
