#version 450 core

layout (binding = 0) uniform sampler2D tex;

in VS_OUT {
vec3 _normal;
vec3 _color;
vec2 _texCoor;
}fs_in;

out vec4 color;

void main()
{
    color = vec4(fs_in._texCoor, 1.0, 1.0);
}
