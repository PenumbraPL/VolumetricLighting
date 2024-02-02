#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "pch.h";


void formatAttribute(GLint attr_location, AkAccessor* acc) {
    int comp_size = acc->componentSize;;
    int type = acc->componentType;
    GLuint normalize = acc->normalized ? GL_TRUE : GL_FALSE;
    size_t offset = acc->byteOffset;
    int comp_stride = acc->componentBytes;
    size_t length = acc->byteLength;

    switch (comp_size) {
    case AK_COMPONENT_SIZE_SCALAR:                comp_size = 1; break;
    case AK_COMPONENT_SIZE_VEC2:                  comp_size = 2; break;
    case AK_COMPONENT_SIZE_VEC3:                  comp_size = 3; break;
    case AK_COMPONENT_SIZE_VEC4:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT2:                  comp_size = 4; break;
    case AK_COMPONENT_SIZE_MAT3:                  comp_size = 9; break;
    case AK_COMPONENT_SIZE_MAT4:                  comp_size = 16; break;
    case AK_COMPONENT_SIZE_UNKNOWN:
    default:                                      comp_size = 1; break;
    }

    switch (type) {
    case AKT_FLOAT:						type = GL_FLOAT; break;
    case AKT_UINT:						type = GL_UNSIGNED_INT; break;
    case AKT_BYTE:						type = GL_BYTE; break;
    case AKT_UBYTE:						type = GL_UNSIGNED_BYTE; break;
    case AKT_SHORT:						type = GL_SHORT; break;
    case AKT_USHORT:					type = GL_UNSIGNED_SHORT; break;
    case AKT_UNKNOWN:
    case AKT_NONE:
    default:                            type = GL_INT; break;
    };

    glVertexAttribFormat(attr_location, comp_size, type, normalize, 0);
}

char* read_file(const char* file_name) {
    FILE* fs;
    fopen_s(&fs, file_name, "rb");

    if (!fs) {
        return nullptr;
    }

    fseek(fs, 0, SEEK_END);
    int file_size = ftell(fs);
    rewind(fs);

    char* buffer = (char*)calloc(file_size + 1, 1);
    if (buffer) fread(buffer, 1, file_size, fs);
    fclose(fs);

    return buffer;
}