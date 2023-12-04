#version 450 core

layout (binding = 0) uniform sampler2D tex;

uniform bool isTexture = true;

in VS_OUT {
vec3 _normal;
vec3 _color;
vec2 _texCoor;
}fs_in;

out vec4 color;

void main()
{
    if(isTexture){
        color = texture(tex, fs_in._texCoor);
    }else{
        color = vec4(fs_in._normal, 1.0);
    }
}
