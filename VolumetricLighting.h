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

struct Primitive {
    float* transform;
    float* w_transform;

    uint32_t* ind;
    unsigned int ind_size;
    GLuint* program;

    GLuint* amb_sampler = nullptr;
    GLuint* amb_texture = nullptr;

    GLuint* emi_sampler = nullptr;
    GLuint* emi_texture = nullptr;

    GLuint* diff_sampler = nullptr;
    GLuint* diff_texture = nullptr;

    GLuint* spec_sampler = nullptr;
    GLuint* spec_texture = nullptr;

    GLuint* sg_sampler = nullptr;
    GLuint* sg_texture = nullptr;

    GLuint* mr_sampler = nullptr;
    GLuint* mr_texture = nullptr;

    GLuint* alb_sampler = nullptr;
    GLuint* alb_texture = nullptr;

    GLuint* dif_sampler = nullptr;
    GLuint* dif_texture = nullptr;

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