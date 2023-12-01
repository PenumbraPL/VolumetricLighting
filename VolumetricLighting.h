#pragma once

//#define GLEW_STATIC

#include <iostream>
#include <stdio.h>
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

