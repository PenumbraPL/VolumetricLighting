#pragma once
#include "pch.h"
#include "GL/glew.h"
#include <string>

void initialize_GLEW(void);
GLuint wrap_mode(AkWrapMode& wrap);
void set_up_color(AkColorDesc* colordesc, AkMeshPrimitive* prim, GLuint* sampler, GLuint* texture, GLuint* texturesType);
void proccess_node(AkNode* node);
void format_attribute(GLint attr_location, AkAccessor* acc);
char* read_file(const char* file_name);
std::string print_coord_system(AkCoordSys* coord);
std::string print_doc_information(AkDocInf* inf, AkUnit* unit);
GLint check_pipeline_status(GLuint vertex_shader, GLuint fragment_shader);