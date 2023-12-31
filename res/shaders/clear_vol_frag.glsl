#version 450 core

layout (binding = 0, rg16f) uniform writeonly image2D image;

in VS_OUT{
vec3 _normal;
vec3 _color;
vec2 _texCoords;
vec3 _view;
vec3 _position;
}fs_in;

uint width = 1900;
uint height = 1000;
vec2 size = vec2(width, height);

void main(){
	if(gl_FrontFacing)
	{	
		imageStore(image, ivec2(size.xy*gl_FragCoord.xy), vec4(gl_FragCoord.z, 0.f, 0.f, 0.f));
	}
	else
	{			
		imageStore(image, ivec2(size.xy*gl_FragCoord.xy), vec4(0.f, gl_FragCoord.z, 0.f, 0.f));
	}
}