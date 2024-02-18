#pragma once
#include "pch.h"
#include "GL/glew.h"
#include <string>
#include "Models.h"

void initialize_GLEW(void);
GLuint wrap_mode(AkWrapMode& wrap);
void set_up_color(AkColorDesc* colordesc, AkMeshPrimitive* prim, Primitive& primitive, enum TextureType type, ConfigContext& panelConfig);
void set_up_color(
    AkColorDesc* colordesc,
    AkMeshPrimitive* prim,
    Drawable& primitive,
    enum TextureType type,
    ConfigContext& panelConfig);
void proccess_node(AkNode* node, std::vector<Primitive>& primitives);
void proccess_node(AkNode* node, std::vector<Drawable>& primitives);
void format_attribute(GLint attr_location, AkAccessor* acc);
char* read_file(const char* file_name);
std::string print_coord_system(AkCoordSys* coord);
std::string print_doc_information(AkDocInf* inf, AkUnit* unit);
GLint check_pipeline_status(GLuint vertex_shader, GLuint fragment_shader);

/* ============================== */
void setPointer(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
void setPointer2(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
void setPointer3(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
