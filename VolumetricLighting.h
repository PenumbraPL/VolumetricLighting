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

struct Primitive {
    float* transform;
    float* w_transform;

    uint32_t* ind;
    unsigned int ind_size;
    GLuint* program; // shaders or pipeline

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

    void deleteTranforms() {
        if (transform) free(transform);
        if (w_transform) free(w_transform);
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