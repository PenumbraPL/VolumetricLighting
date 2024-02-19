#version 450

in vec3 vPos;
in vec3 vCol;
uniform mat4 MVP;

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
    if(length(vCol) < 1e-5){
        _color = vPos;
    } else{
        _color = vCol;
    }
}
