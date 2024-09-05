#pragma once
#include "pch.h"
#include "GL/glew.h"
#include "Models.h"

void initializeGLEW(void);
GLuint wrapMode(AkWrapMode& wrap);
void setUpColor(
    AkColorDesc* colordesc,
    AkMeshPrimitive* prim,
    Drawable& primitive,
    enum TextureType type,
    GUI& panelConfig);
void proccessNode(AkNode* node, std::vector<Drawable>& primitives);
void formatAttribute(GLint attr_location, AkAccessor* acc);
char* readFile(const char* file_name);
std::string printCoordSystem(AkCoordSys* coord);
std::string printDocInformation(AkDocInf* inf, AkUnit* unit);
GLint checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader);

/* ============================== */
void setPointer(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
void setPointer2(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
void setPointer3(GLuint program, GLint& mvpBindingLocation, GLint& vertexPosBindingLocation, GLint& vcol_location);
