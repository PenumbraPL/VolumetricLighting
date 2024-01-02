#pragma once
#include "GL/glew.h"

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