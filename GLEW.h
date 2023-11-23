#pragma once
#include "GL/glew.h"

void initializeGLEW(void) {
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "========== [GLEW]: Initialization failed =====================\n";
        std::cout << "\tError:" << glewGetErrorString(err);
    }
    std::cout << "========== [GLEW]: Using GLEW " << glewGetString(GLEW_VERSION) << " ===========\n";

    // glewIsSupported supported from version 1.3
    if (GLEW_VERSION_1_3)
    {
        if (glewIsSupported("GL_VERSION_4_5  GL_ARB_point_sprite"))
        {
            std::cout << "========== [GLEW]: Version 4.5 of OpenGL is supported ========\n";
            std::cout << "========== [GLEW]: Extention GL_ARB_point_sprite is supported \n";
        }
        if (glewIsSupported("GL_VERSION_4_5  GL_KHR_debug"))
        {
            std::cout << "========== [GLEW]: Version 4.5 of OpenGL is supported ========\n";
            std::cout << "========== [GLEW]: Extention GL_KHR_debug is supported =======\n";
        }
        /*
    GL_ARB_buffer_storage
    GL_ARB_clear_buffer_object
    GL_ARB_clear_texture
    GL_ARB_clip_control
    GL_ARB_multi_bind
    GL_ARB_sampler_objects
    GL_ARB_texture_storage
    GL_ARB_vertex_attrib_binding
    */
    }
}