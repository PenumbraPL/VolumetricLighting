#version 450 core

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main(void){
    const vec4 vertices[] = vec4[]( vec4(-1.0, -1.0, 0.5, 1.0),
                                    vec4( 1.0, -1.0, 0.5, 1.0),
                                    vec4(-1.0,  1.0, 0.5, 1.0),
                                    vec4( 1.0,  1.0, 0.5, 1.0) );

    gl_Position = vertices[gl_VertexID];
}
