#pragma once

//#define GLEW_STATIC

#include <iostream>
#include <stdio.h>
#include <regex>
#include <map>
#include "GLEW.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtx/polar_coordinates.hpp>
#include <stdlib.h>
#include "GUI.h"
#include "Debug.h"
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/common.hpp"
#include "Draw.h"
#include "ds/forward-list-common.h"

//#define AK_STATIC 1
#include "ak/assetkit.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

struct WindowInfo {
    int width;
    int height;
    const char* title;
    int cursor_mode;
    int imgui;
    int mbutton;
};

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};
struct {
    float x, y, z;
}cube[8] = {
    -0.25f, -0.25f, -0.25f,
    -0.25f, 0.25f, -0.25f,
    0.25f, -0.25f, -0.25f,
    0.25f, 0.25f, -0.25f,
    0.25f, -0.25f, 0.25f,
    0.25f, 0.25f, 0.25f,
    -0.25f, -0.25f, 0.25f,
    -0.25f, 0.25f, 0.25f,
};

enum TextureType {
    AMBIENT,
    EMISIVE,
    DIFFUSE,
    SPECULAR,
    SP_GLOSSINESS,
    MAT_ROUGH,
    ALBEDO,
    SP_DIFFUSE,
};

enum ShaderTypes {
    VERTEX,
    FRAGMENT,
    TESS_CTR,
    TESS_EV,
    GEOMETRY
};

GLint checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader) {
    GLint v_comp_status, f_comp_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);
        std::cout << "Vertex Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        char comp_info[1024];
        memset(comp_info, '\0', 1024);
        //glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);
        std::cout << "Fragment Shader: ";
        fwrite(comp_info, 1024, 1, stdout);
    }
    return (!v_comp_status || !f_comp_status) ? 0 : 1;
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

struct Primitive {
    float* transform;
    float* w_transform;

    uint32_t* ind;
    unsigned int ind_size;
    GLuint* programs; // shaders or pipeline
    GLuint pipeline;

    GLuint* textures = nullptr;
    GLuint* tex_type = nullptr;
    GLuint* samplers = nullptr;



    //struct Tex{
    //    Primitive* p;
    //    Tex(Primitive* p) : p(p) {}
    //    GLuint* operator[](enum TextureType t) {
    //        return  &p->textures[t];
    //    }
    //} te = Primitive::Tex(this);


    //struct Samp {
    //    Primitive* p;
    //    Samp(Primitive* p): p(p) {}
    //    GLuint* operator[](enum TextureType t) {
    //        return &p->samplers[t];
    //    }
    //} samp = Primitive::Samp(this);

    //friend Primitive::Tex;
    //friend Primitive::Samp;

    AkAccessor* wgs;
    AkAccessor* jts;
    AkAccessor* pos;
    AkAccessor* tex;
    AkAccessor* nor;
    AkAccessor* col;
    AkAccessor* tan;

    float* setTransform(void) {
        return transform = (float*)calloc(16, sizeof(float));
    }

    float* setWorldTransform(void) {
        return w_transform = (float*)calloc(16, sizeof(float));
    }

    void deleteTransforms() {
        if (transform) free(transform);
        if (w_transform) free(w_transform);
    }

    GLuint* createPrograms() {
        programs = (GLuint*)calloc(5, sizeof(GLuint));
        memset(textures, -1, 5);
        return programs;
    }

    void createPipeline() {

        char* v_sh_buffer = read_file("res/vertex3.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/fragment3.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";

        createPrograms();
        //GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        //glObjectLabel(GL_SHADER, vertex_shader, -1, "Vertex Shader");
        //glShaderSource(vertex_shader, 1, &v_sh_buffer, NULL);
        //glCompileShader(vertex_shader);

        //GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        //glObjectLabel(GL_SHADER, fragment_shader, -1, "Fragment Shader");
        //glShaderSource(fragment_shader, 1, &f_sh_buffer, NULL);
        //glCompileShader(fragment_shader);

        GLint status = 1;
        ///* ======================================================== */
        //GLint status = checkPipelineStatus(vertex_shader, fragment_shader);
        ///* ======================================================== */

        //program = glCreateProgram();
        //glObjectLabel(GL_PROGRAM, program, -1, "Volumetric lighting");

        programs[VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        programs[FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);
       
        // Cleanup
        free(v_sh_buffer);
        free(f_sh_buffer);

        //GetProgramPipelineInfoLog
        //ValidateProgramPipeline
        if (status) {
            //glAttachShader(program, vertex_shader);
            //glAttachShader(program, fragment_shader);
            //glLinkProgram(program);
            //glDetachShader(program, vertex_shader);
            //glDetachShader(program, fragment_shader);




            GLint link_status;
            //glGetProgramiv(program, GL_LINK_STATUS, &link_status);
            //if (!link_status) {
            //    GLchar comp_info[1024];
            //    glGetProgramInfoLog(program, 1024, NULL, comp_info);

            //    fwrite(comp_info, 1024, 1, stdout);
            //}
            glGetProgramiv(programs[VERTEX], GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(programs[VERTEX], 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(programs[FRAGMENT], GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(programs[FRAGMENT], 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
        }
        //glDeleteShader(vertex_shader);
        //glDeleteShader(fragment_shader);

        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, programs[VERTEX]);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, programs[FRAGMENT]);
    }
    void deletePipeline() {
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
        //glDeleteProgram(program);

        deletePrograms();
    }


    void deletePrograms() {
        for (int i = 0; i < 5; i++) glDeleteProgram(programs[i]);
        if (programs) free(programs);
    }

    GLuint* createTextures() {
        textures = (GLuint*)calloc(8, sizeof(GLuint));
        memset(textures, -1, 8);
        tex_type = (GLuint*)calloc(8, sizeof(GLuint));
        memset(tex_type, -1, 8);
        return textures;
    }

    GLuint* createSamplers() {
        samplers = (GLuint*)calloc(8, sizeof(GLuint));
        memset(samplers, -1, 8);
        return samplers;
    }

    void deleteTexturesAndSamplers() {
        if (textures) {
            glDeleteTextures(8, textures);
            free(textures);
        }
        if (tex_type) {
            free(tex_type);
        }
        if (samplers) {
            glDeleteSamplers(8, samplers);
            free(samplers);
        }
    }
};

struct Light {
    enum LightType {POSITIONAL, DIRECTIONAL, AREA} light_type;
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction;
    glm::vec4 color;
    int intensity;
};

struct Camera {
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction;
    float zNear;
    float zFar;
    int fov;
};