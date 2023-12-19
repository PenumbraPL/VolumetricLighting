#version 450 core

layout (binding = 0, r32ui) coherent uniform uimage2D head_pointer;

void main(void){
    ivec2 P = ivec2(gl_FragCoord.xy);
    imageStore(head_pointer, P, uvec4(0xFFFFFFFF));
}
