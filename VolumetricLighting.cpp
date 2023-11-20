// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define GLEW_STATIC

#include <iostream>
#include <stdio.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
//#include "glm/matrix.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdlib.h>
#include "GUI.h"
#include "Debug.h"

struct WindowInfo {
    int width;
    int height;
    const char* title;
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

static const GLchar* vertex_shader_text[] ={
"#version 450\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"out vec3 _color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    _color = vCol;\n"
"}\n" };

static const GLchar* fragment_shader_text[] = {
"#version 450\n"
"in vec3 _color;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = vec4(_color, 1.0);\n"
"}\n"
};

static GLchar vertex_shader_test[100][256];
static GLchar fragment_shader_test[100][256];

void error_callback(int code, const char* description)
{
    std::cout << code << " " << description << std::endl;
}

int main(void)
{
    std::cout << "=========== Initialization started =========\n";

    GLFWwindow* window;
    WindowInfo windowConfig = {
        1280,
        960,
        "Simple Triangles"
    };
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    if (!glfwInit()) {
        std::cout << "========== [GLFW]: Initialization failed =========\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "========== [GLFW]: Terminated =========\n";
        std::cout << "========== [GLFW]: Window initialization failed =========\n";
        return 1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "========== [GLEW]: Initialization failed =========\n";
        std::cout << "\tError:" << glewGetErrorString(err);
    }
    std::cout << "========== [GLEW]: Using GLEW " << glewGetString(GLEW_VERSION) << " ================\n";
    
    // glewIsSupported supported from version 1.3
    if(GLEW_VERSION_1_3)
    {
        if (glewIsSupported("GL_VERSION_4_5  GL_ARB_point_sprite"))
        {
            std::cout << "========== [GLEW]: Version 4.5 of OpenGL is supported =========\n";
            std::cout << "========== [GLEW]: Extention GL_ARB_point_sprite is supported =========\n";
        }
        if(glewIsSupported("GL_VERSION_4_5  GL_KHR_debug"))
        {
            std::cout << "========== [GLEW]: Version 4.5 of OpenGL is supported =========\n";
            std::cout << "========== [GLEW]: Extention GL_KHR_debug is supported =========\n";
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 150"; // TO DO
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    glm::vec2 p1(-0.5, 0.5);
    glm::vec2 p2(0.5, 0.5);
    glm::vec2 p3(0, -0.5);

    glfwSetErrorCallback(error_callback);
    
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        std::cout << "========== [GLFW]: Debug context initialize successful =========\n";
        std::vector<DEBUGPROC> callbacks;
        callback_list(callbacks);
        debug_init(callbacks);
    }
    else {
        std::cout << "========== [GLFW]: Debug context initialize unsuccessful =========\n";
    }


    
    FILE  *fs;
    char* buffer;
    fopen_s(&fs, "res/vertex.glsl", "rb");
    if (fs) {
        std::cout << "=================== res/vertex.glsl opened =======================\n";
        fseek(fs, 0, SEEK_END);
        int file_size = ftell(fs);
        rewind(fs);

        buffer = (char*)calloc(file_size + 1, 1);
        fread(buffer, 1, file_size, fs);
        fclose(fs);
        fs = NULL;
    }
    else {
        std::cout << "=================== Coulnt find res/vertex.glsl =======================\n";
    }
    

    char* fbuffer;
    fopen_s(&fs, "res/fragment.glsl", "rb");
    if (fs) {
        std::cout << "=================== res/fragment.glsl opened  =======================\n";

        fseek(fs, 0, SEEK_END);
        int file_size = ftell(fs);
        rewind(fs);

        fbuffer = (char*)calloc(file_size + 1, 1);
        fread(fbuffer, 1, file_size, fs);
        fclose(fs);
    }
    else {
        std::cout << "=================== Coulnt find res/fragment.glsl =======================\n";
    }
    
    //fwrite(fragment_shader_test, 1024, 1, stdout);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (GLchar**) &buffer, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (GLchar**) &fbuffer, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    /* ======================================================== */

    GLint v_comp_status, f_comp_status, link_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        GLint v_comp_len = 0; // niepotrzebne chyba ze z ifem
        GLchar comp_info[1024];
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &v_comp_len);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        GLint f_comp_len = 0; // niepotrzebne chyba ze z ifem
        GLchar comp_info[1024];
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &f_comp_len);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        GLchar comp_info[1024];
        glGetProgramInfoLog(program, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }

    /* ======================================================== */


    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

//    std::cout << mvp_location << " " << vpos_location << " " << vcol_location << std::endl;

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)(sizeof(float) * 2));

    glObjectLabel(GL_BUFFER, vertex_buffer, 0, "Vertex Buffer");
    glObjectLabel(GL_SHADER, vertex_shader, 0, "Vertex Shader");
    glObjectLabel(GL_SHADER, fragment_shader, 0, "Fragment Shader");
    glObjectLabel(GL_PROGRAM, program, 0, "Volumetric lighting");


    std::cout << "===================== Main loop ===================\n";
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glm::mat4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);
        
        /*glBegin(GL_TRIANGLES);
        glVertex2f (-0.5, 0.5);
        glVertex2f (0.5, 0.5);
        glVertex2f (0, -0.5);
        glEnd();*/

        m = glm::mat4(1.);
        //glm::mat4x4_rotate_Z(m, m, (float)glfwGetTime());
        m = glm::rotate(m, glm::radians((float)glfwGetTime()), glm::vec3(0, 0, 1));
        //glm::mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);

        m = m * p * mvp;// glm::mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

       

        drawLeftPanel(io);
        drawRightPanel(io);
        

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    GLuint buffers[] = {vertex_buffer};
    glDeleteBuffers(1, buffers);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glDeleteProgram(program);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    std::cout << "========== [GLFW]: Terminated =========\n";
    std::cout << "===================== Exit succeeded ===================\n";
    return 0;
}