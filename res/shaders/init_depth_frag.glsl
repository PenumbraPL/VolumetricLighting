#version 450 core

layout (binding = 2, offset = 0) uniform atomic_uint fill_counter;
layout (binding = 0, r32ui) coherent uniform uimage2D head_pointer;

struct list_item{
	vec4 color;
	float depth;
	int facing;
	uint next;
};

layout (binding = 1, std430) buffer list_item_block
{
	list_item item[];
};


in VS_OUT{
vec3 _normal;
vec3 _color;
vec2 _texCoords;
vec3 _view;
vec3 _position;
} fs_in;

layout (location = 0) out vec4 color;

void main(void){
	
	ivec2 P = ivec2(gl_FragCoord.xy);
	uint index = atomicCounterIncrement(fill_counter);
	uint old_head = imageAtomicExchange(head_pointer, P, index);

	item[index].color = vec4(fs_in._color, 1.0);
	item[index].depth = gl_FragCoord.z;
	item[index].facing = gl_FrontFacing ? 1 : 0;
	item[index].next = old_head;
	
	//float v = 0.;
	if(!gl_FrontFacing){
		color.xyz -= gl_FragCoord.z / (30*gl_FragCoord.w);
	}else{
		color.xyz += gl_FragCoord.z / (30*gl_FragCoord.w);
	}
	color.w = 1;
	//color = vec4(v,v,v,1.);
};