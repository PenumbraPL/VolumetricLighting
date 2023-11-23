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
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include "Draw.h"

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
} windowConfig = {
    1280,
    960,
    "Simple Triangles",
    0, 0, 0
};

void fun() {}

ConfigContext panel_config{
    2.f, 0.f, 50, 0,0,0,0, 0, 50, 50, 50
};

double xpos, ypos;

void*
imageLoadFromFile(const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) {
    return stbi_load(path, width, height, components, 0);
}

void*
imageLoadFromMemory(const char* __restrict data,
    size_t                  len,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) {
    return stbi_load_from_memory((stbi_uc const*)data, (int)len, width, height, components, 0);
}

void
imageFlipVerticallyOnLoad(bool flip) {
    stbi_set_flip_vertically_on_load(flip);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    panel_config.p6 += yoffset * -6;
}


static void cursor_position_callback(GLFWwindow* window, double new_xpos, double new_ypos)
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state != GLFW_RELEASE)
    {
        double nx = (1000.f / windowConfig.width) * (new_xpos - xpos);
        double ny = (1000.f / windowConfig.height) * (new_ypos - ypos);
        panel_config.p7 += nx;
        panel_config.p8 += ny;
        
        xpos = new_xpos;
        ypos = new_ypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &xpos, &ypos);
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p7 += 1;
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p7 -= 1;
    }
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p8 += 1;
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        panel_config.p8 -= 1;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        GLFWmousebuttonfun mouse_button_callbacks[2] = { NULL, mouse_button_callback };
        windowConfig.mbutton += 1;
        windowConfig.mbutton %= 2;
        glfwSetMouseButtonCallback(window, mouse_button_callbacks[windowConfig.mbutton]);
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        GLFWcursorposfun cursor_callbacks[2] = { NULL, cursor_position_callback };
        windowConfig.imgui += 1;
        windowConfig.imgui %= 2;
        glfwSetCursorPosCallback(window, cursor_callbacks[windowConfig.imgui]);
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        int mode[2] = { GLFW_CURSOR_DISABLED, GLFW_CURSOR_NORMAL };
        windowConfig.cursor_mode += 1;
        windowConfig.cursor_mode %= 2;
        glfwSetInputMode(window, GLFW_CURSOR, mode[windowConfig.cursor_mode % (sizeof(mode)/sizeof(int))]);
    }
}


void setUniformMVP(GLuint Location, glm::vec3 const& Translate, glm::vec3 const& Rotate)
{
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 ViewTranslate = glm::translate(
        glm::mat4(1.0f), Translate);
    glm::mat4 ViewRotateX = glm::rotate(
        ViewTranslate, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    glm::mat4 View = glm::rotate(ViewRotateX,
        Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(
        glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 MVP = Projection * View * Model;
    glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(MVP));
}

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


int main(void)
{
    std::cout << "=========== Initialization started =========\n";

    GLFWwindow* window;
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 150"; // TO DO
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    glm::vec2 p1(-0.5, 0.5);
    glm::vec2 p2(0.5, 0.5);
    glm::vec2 p3(0, -0.5);



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
    char* v_sh_buffer;
    fopen_s(&fs, "res/vertex2.glsl", "rb");
    if (fs) {
        std::cout << "=================== res/vertex.glsl opened =======================\n";
        fseek(fs, 0, SEEK_END);
        int file_size = ftell(fs);
        rewind(fs);

        v_sh_buffer = (char*)calloc(file_size + 1, 1);
        fread(v_sh_buffer, 1, file_size, fs);
        fclose(fs);
        fs = NULL;
    }
    else {
        std::cout << "=================== Coulnt find res/vertex.glsl =======================\n";
    }
    

    char* f_sh_buffer;
    fopen_s(&fs, "res/fragment2.glsl", "rb");
    if (fs) {
        std::cout << "=================== res/fragment.glsl opened  =======================\n";

        fseek(fs, 0, SEEK_END);
        int file_size = ftell(fs);
        rewind(fs);

        f_sh_buffer = (char*)calloc(file_size + 1, 1);
        fread(f_sh_buffer, 1, file_size, fs);
        fclose(fs);
    }
    else {
        std::cout << "=================== Coulnt find res/fragment.glsl =======================\n";
    }
    
    GLuint vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //glGenBuffers(1, &vertex_buffer);
    //glBufferData(GL_ARRAY_BUFFER, buffer_size, raw_buffer, GL_STATIC_DRAW);    
    //glNamedBufferData(vertex_buffer, buffer_size, raw_buffer, GL_STATIC_DRAW);
    //glBindVertexBuffer(0, vertex_buffer, 0, sizeof(float)*3);


    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (GLchar**) &v_sh_buffer, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (GLchar**) &f_sh_buffer, NULL);
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


    ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    AkDoc* doc;
    AkResult ret;

    if ((ret = ak_load(&doc, "./res/sample.gltf", NULL)) != AK_OK) {
        printf("Document couldn't be loaded\n");
    }
    else {
        printf("sample.gltf loaded sucessful\n");
    }

    AkInstanceBase* instScene;
    AkVisualScene* scene;
    AkCamera* camera;
    AkInstanceGeometry* geometry;
    AkNode* root, * node_ptr;

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));


    int8_t* raw_buffer = nullptr;
    uint32_t* indecies = nullptr;
    unsigned int indecies_size;
    int buffer_size;

    if ((instScene = doc->scene.visualScene)) {
        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        printf("Visual Scene loaded\n");
        std::cout << "Scene name: " << scene->name << std::endl;
        //geometry = ak_libFirstGeom(doc);
        if (scene->lights)
            if (scene->lights->first) {
                AkLight* light = (AkLight*)ak_instanceObject(scene->lights->first->instance);
                std::cout << "Light name: " << light->name << std::endl;
            }
        if (scene->cameras)
            if (scene->cameras->first) {
                AkCamera* camera = (AkCamera*)ak_instanceObject(scene->cameras->first->instance);
                std::cout << "Camera name: " << camera->name << std::endl;
            }
        if (scene->evaluateScene)
            if (scene->evaluateScene->render) {
                const char* render = scene->evaluateScene->render->cameraNode;
                std::cout << "Render: " << render << std::endl;
            }

        root = ak_instanceObjectNode(scene->node);
        node_ptr = root;

        do {
            std::cout << "Node name: " << node_ptr->name << std::endl;
            std::string node_type;
            switch (node_ptr->nodeType) {
            case AK_NODE_TYPE_NODE:             node_type = "node"; break;
            case AK_NODE_TYPE_CAMERA_NODE:      node_type = "camera"; break;
            case AK_NODE_TYPE_JOINT:            node_type = "joint"; break;
            };
            int j = 0;

            std::string geo_type;
            if (node_ptr->geometry) {
                AkGeometry* geometry = ak_instanceObjectGeom(node_ptr);
                AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
                switch ((AkGeometryType)geometry->gdata->type) {
                case AK_GEOMETRY_MESH:
                    geo_type = "mesh";
                    if (mesh) {
                        std::string prim_type;
                        AkMeshPrimitive* prim = mesh->primitive;
                        std::cout << prim_type << std::endl;
                        switch (prim->type) {
                        case AK_PRIMITIVE_LINES:              prim_type = "line"; break;
                        case AK_PRIMITIVE_POLYGONS:           prim_type = "polygon"; break;
                        case AK_PRIMITIVE_TRIANGLES:          prim_type = "triangle"; break;
                        case AK_PRIMITIVE_POINTS:             prim_type = "point"; break;
                        }
                        if (prim->indices) {
                            indecies = (unsigned int*) prim->indices->items;
                            indecies_size = prim->indices->count;
                        }
                        

                        AkBuffer* buffer = prim->input->accessor->buffer;
                        raw_buffer = (int8_t*)buffer->data;
                        int length = prim->input->accessor->byteLength;
                        buffer_size = length;

                        int offset = prim->input->accessor->byteOffset;
                        //int stride = prim->input->accessor->byteStride;
                        int comp_stride = prim->input->accessor->componentBytes;
                        //int count = prim->input->accessor->count;
                        //std::cout << prim->input->semanticRaw << std::endl;
                        int normalize = prim->input->accessor->normalized;

                        int comp_size = prim->input->accessor->componentSize;
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

                        int type = prim->input->accessor->componentType;
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

                        std::cout << "Buffer ptr: " << raw_buffer << " Length: " << length << " Stride: " << comp_stride << " Offset: "
                            << offset << " Comp Size: " << comp_size << " Comp Type: " << type << std::endl;
                        std::cout << ak_meshInputCount(mesh) << std::endl;



                        mvp_location = glGetUniformLocation(program, "MVP");
                        vpos_location = glGetAttribLocation(program, "vPos");
                        int binding_point = 0;

                        glVertexAttribFormat(vpos_location, comp_size, type, normalize, 0); // comp_stride change to comp_size*type
                        glVertexAttribBinding(vpos_location, binding_point);

                        glGenBuffers(1, &vertex_buffer);
                        glNamedBufferData(vertex_buffer, buffer_size, raw_buffer, GL_STATIC_DRAW);
                        glBindVertexBuffer(binding_point, vertex_buffer, offset, comp_stride);
                        glEnableVertexAttribArray(vpos_location);

                    };
                    break;
                case AK_GEOMETRY_SPLINE:
                    geo_type = "spline";
                    break;
                case  AK_GEOMETRY_BREP:
                    geo_type = "brep";
                    break;
                default:
                    geo_type = "other";
                    break;
                };

            }
            std::cout << "No. " << j++ << std::endl;
            std::cout << "Node type: " << geo_type << std::endl;


            node_ptr = node_ptr->next;
        } while (node_ptr);


        ak_firstCamera(doc, &camera, camera_mat, camera_proj);
        std::cout << "Camera:" << camera->name << std::endl;
        for (int i = 0; i < 16; i++) {
            std::cout << camera_mat[i] << ", ";
            if (i % 4 == 3) std::cout << std::endl;
        }
    }

    if (camera_mat) free(camera_mat);
    if (camera_proj) free(camera_proj);



    /* ======================================================== */

    //setPointer(program, mvp_location, vpos_location, vcol_location);
    //setPointer2(program, mvp_location, vpos_location, vcol_location);
    //setPointer3(program, mvp_location, vpos_location, vcol_location);


    glObjectLabel(GL_BUFFER, vertex_buffer, 0, "Vertex Buffer");
    glObjectLabel(GL_SHADER, vertex_shader, 0, "Vertex Shader");
    glObjectLabel(GL_SHADER, fragment_shader, 0, "Fragment Shader");
    glObjectLabel(GL_PROGRAM, program, 0, "Volumetric lighting");


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetScrollCallback(window, scroll_callback);
   




    std::cout << "===================== Main loop ===================\n";
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        //glm::mat4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);
        
        float r, phi, theta;
        float eye_x, eye_y, eye_z;
        float north_x, north_y, north_z;
        r = 0.1 * panel_config.p6;
        theta = 3.14 * panel_config.p7 / 180;
        phi = 3.14 * panel_config.p8 / 180;

        eye_x = r*cos(phi)*cos(theta);
        eye_y = r*sin(phi);
        eye_z = r*cos(phi)*sin(theta);

        float n_phi = phi + 0.01;
        north_x = r * cos(n_phi) * cos(theta);
        north_y = r * sin(n_phi);
        north_z = r * cos(n_phi) * sin(theta);

        glm::vec3 translate = glm::vec3(panel_config.p1 * 0.1, panel_config.p2 * 0.1, panel_config.p3 * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.p4/180, 3.14 * panel_config.p5 / 180, 0.f);

        glm::vec3 camera = glm::vec3(eye_x, eye_y, eye_z);

        glUseProgram(program);
        
        glm::mat4 LookAt = glm::lookAt(camera, glm::vec3(0., 0., 0.), glm::vec3(north_x, north_y, north_z));
        glm::mat4 Projection = glm::perspectiveFov((float) 3.14*panel_config.fov/180, (float) width, (float) height, panel_config.near_plane, panel_config.far_plane);

        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), translate);
        glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
        glm::mat4 View = glm::rotate(ViewRotateX, rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        
        glm::mat4 MVP = Projection * LookAt * View * Model;
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
        
        
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
        //glDrawArrays(GL_POINTS, 0, 1966);
        glDrawElements(GL_TRIANGLES, indecies_size / 3, GL_UNSIGNED_INT, indecies);

        fun();

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io);
        drawRightPanel(io, panel_config);
        
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    free(v_sh_buffer);
    free(f_sh_buffer);

    GLuint buffers[] = {vertex_buffer};
    glDeleteBuffers(1, buffers);
    glDeleteVertexArrays(1, &vao);

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