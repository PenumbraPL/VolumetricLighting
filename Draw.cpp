#include "pch.h"
#include "Draw.h"


void setPointer(GLuint program,
    GLint &mvpBindingLocation,
    GLint &vertexPosBindingLocation,
    GLint &vcol_location) 
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vertexPosBindingLocation);
    glEnableVertexAttribArray(vcol_location);

    glVertexAttribPointer(vertexPosBindingLocation, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float) * 2));
}

void setPointer2(
    GLuint program,
    GLint& mvpBindingLocation,
    GLint& vertexPosBindingLocation, 
    GLint& vcol_location) 
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vertexPosBindingLocation);
    glVertexAttribPointer(vertexPosBindingLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void setPointer3(
    GLuint program, 
    GLint& mvpBindingLocation,
    GLint& vertexPosBindingLocation, 
    GLint& vcol_location) 
{
    mvpBindingLocation = glGetUniformLocation(program, "MVP");
    vertexPosBindingLocation = glGetAttribLocation(program, "vPos");
    
    glVertexAttribFormat(vertexPosBindingLocation, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(vertexPosBindingLocation, 0);
    glEnableVertexAttribArray(vertexPosBindingLocation);
}
