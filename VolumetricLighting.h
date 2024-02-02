#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "pch.h"

//#define GLEW_STATIC
#include "GUI.h"
#include "Draw.h"
#include "IO.h"

#define PATH "./res/models/DamagedHelmet/"
#define FILE_NAME "DamagedHelmet.gltf"

void formatAttribute(GLint attr_location, AkAccessor* acc);
char* read_file(const char* file_name);
std::string printCoordSys(AkCoordSys* coord);
std::string printInf(AkDocInf* inf, AkUnit* unit);


void initializeGLEW(void) {
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "========== [GLEW]: Initialization failed =====================================\n";
        std::cout << "\tError:" << glewGetErrorString(err);
    }
    std::cout << "========== [GLEW]: Using GLEW " << glewGetString(GLEW_VERSION) << " =========================================\n";


    if (GLEW_VERSION_1_3) // glewIsSupported supported from version 1.3
    {
        std::string versionName = "GL_VERSION_4_5";
        std::string extensionList[] = {
            "GL_ARB_separate_shader_objects",
            "GL_ARB_shader_image_load_store",
            "GL_ARB_texture_storage",
            "GL_ARB_vertex_attrib_binding",
            "GL_ARB_vertex_attrib_64bit",
            "GL_KHR_debug",
            "GL_NV_shader_buffer_load"
        };
        for (auto& ext : extensionList) {
            if (!glewIsSupported((versionName + " " + ext).c_str()))
            {
                std::cout << "========== [GLEW]: For " + versionName + " extension " + ext + " isn't supported \n";
            }
        }
    }
    else {
        std::cout << "========== [GLEW]: OpenGL's extensions support haven't been verified! ============================\n";
    }
}


struct WindowInfo {
    int width;
    int height;
    const char* title;
    int cursor_mode;
    int imgui;
    int mbutton;
};



struct PointLight {
    alignas(16) glm::vec3 position = glm::vec3(0.);

    float constant = 0.5f;
    float linear = 0.5f;
    float quadratic = 0.5f;

    alignas(16) glm::vec3 ambient = glm::vec3(0.8);
    alignas(16) glm::vec3 diffuse = glm::vec3(0.8);
    alignas(16) glm::vec3 specular = glm::vec3(0.8);
};
struct LightsList {
    unsigned int size;
    alignas(16) PointLight list[];
};

