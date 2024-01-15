#include "Draw.h"

void setPointer(GLuint program, GLint &mvp_location, GLint &vpos_location, GLint &vcol_location) {
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glEnableVertexAttribArray(vcol_location);

    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)0);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float) * 2));
}

void setPointer2(GLuint program, GLint& mvp_location, GLint& vpos_location, GLint& vcol_location) {
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
}

void setPointer3(GLuint program, GLint& mvp_location, GLint& vpos_location, GLint& vcol_location) {
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    
    glVertexAttribFormat(vpos_location, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(vpos_location, 0);
    glEnableVertexAttribArray(vpos_location);
}