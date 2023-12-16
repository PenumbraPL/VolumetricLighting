#version 450

uniform mat4 MVP;
in vec3 vCol;
in vec3 vPos;

out vec3 _color;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

void main()
{
    gl_Position = MVP * vec4(vPos, 1.0);
    _color = vCol;
}
