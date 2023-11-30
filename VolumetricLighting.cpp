// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define GLEW_STATIC

#include <iostream>
#include <stdio.h>
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


void set_up_color(AkColorDesc* colordesc){
    if (colordesc) {
    if (colordesc->texture) {
        AkTextureRef* tex = colordesc->texture;
        std::cout << "Texture path: " << tex->texture->image->initFrom->ref;
        //data?
        //next?
        AkSampler* sampler = tex->texture->sampler;
        //next?
        AkTypeId type = tex->texture->type;
        switch (type) {
        case AKT_SAMPLER1D:
        case AKT_SAMPLER2D:
        case AKT_SAMPLER3D:
        case AKT_SAMPLER_CUBE:
        case AKT_SAMPLER_RECT:
        case AKT_SAMPLER_DEPTH:
            break;
        }
        //AkInput* tex_coord = ak_meshInputGet(prim, tex->coordInputName, set);
    }
}
}


char* read_file(const char* file_name) {
    FILE* fs;
    fopen_s(&fs, file_name, "rb");

    fseek(fs, 0, SEEK_END);
    int file_size = ftell(fs);
    rewind(fs);

    char* buffer = (char*)calloc(file_size + 1, 1);
    fread(buffer, 1, file_size, fs);
    fclose(fs);

    return buffer;
}

ConfigContext panel_config{
    2.f, 0.f, 50, 0, 0, 0, 0, 0, 50, 50, 50
};

void checkPipelineStatus(GLuint vertex_shader, GLuint fragment_shader, GLuint program) {
    GLint v_comp_status, f_comp_status, link_status;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_comp_status);
    if (!v_comp_status) {
        GLchar comp_info[1024];
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(vertex_shader, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_comp_status);
    if (!f_comp_status) {
        GLchar comp_info[1024];
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, NULL);
        glGetShaderInfoLog(fragment_shader, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (!link_status) {
        GLchar comp_info[1024];
        glGetProgramInfoLog(program, 1024, NULL, comp_info);

        fwrite(comp_info, 1024, 1, stdout);
    }
}

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


void proccess_node(AkNode* node, AkNode* node_ptr) {
    float* word_transform = (float*)calloc(16, sizeof(float));
    ak_transformCombineWorld(node_ptr, word_transform);
    float* transform = (float*)calloc(16, sizeof(float));
    ak_transformCombine(node_ptr, transform);

    std::string geo_type;
    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node); // if geometry
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        switch ((AkGeometryType)geometry->gdata->type) { //if gdata
        case AK_GEOMETRY_MESH:
            geo_type = "mesh";
            if (mesh) {
                GLuint prim_type;
                std::cout << "Mesh name:" << mesh->name << std::endl;   // should i insert mesh->name ??
                std::cout << "Mesh center:" << mesh->center << std::endl; // same
                for (int i = 0; i < mesh->primitiveCount; i++) {/*prim = prim->next;*/ }
                AkMeshPrimitive* prim = mesh->primitive;
                switch (prim->type) {
                case AK_PRIMITIVE_LINES:              prim_type = GL_LINES; break;
                case AK_PRIMITIVE_POLYGONS:           prim_type = GL_POLYGON; break;
                case AK_PRIMITIVE_TRIANGLES:          prim_type = GL_TRIANGLES; break;
                case AK_PRIMITIVE_POINTS:
                default:                              prim_type = GL_POINTS; break;
                }
                //if (prim->indices) {
                //    indecies = (unsigned int*)prim->indices->items;
                //    indecies_size = prim->indices->count;
                //}
                std::cout << "Primitive center: " << prim->center << std::endl;
                int set = prim->input->set;

                if (prim->material) {
                    AkMaterial* mat = prim->material;
                    AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
                    AkTechniqueFxCommon* tch = ef->profile->technique->common;
                    if (tch) {
                        set_up_color(tch->ambient);
                        set_up_color(tch->emission);
                        set_up_color(tch->diffuse);
                        set_up_color(tch->specular);

                        switch (tch->type) {
                        case AK_MATERIAL_METALLIC_ROUGHNESS:
                            std::cout << "metalic roughness\n";
                            break;
                        case AK_MATERIAL_SPECULAR_GLOSSINES:
                            std::cout << "specular glossines\n";
                            break;
                        };
                        std::cout << "Is double sized: " << tch->doubleSided ? "True" : "False";
                    }
                }


                AkInput* wgs = ak_meshInputGet(prim, "WEIGHTS", set);
                AkInput* jts = ak_meshInputGet(prim, "JOINTS", set);
                AkInput* pos = ak_meshInputGet(prim, "POSITION", set);
                AkInput* tex = ak_meshInputGet(prim, "TEXCOORD", set);
                AkInput* nor = ak_meshInputGet(prim, "NORMAL", set);
                //Color tangent

                AkBuffer* buffer = prim->input->accessor->buffer;
                //raw_buffer = (int8_t*)buffer->data;
                //int length = prim->input->accessor->byteLength;
                //buffer_size = length;

                int offset = prim->input->accessor->byteOffset;
                //int stride = prim->input->accessor->byteStride;
                int comp_stride = prim->input->accessor->componentBytes;
                //int count = prim->input->accessor->count;
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
                //<< " Length: " << length
                std::cout  << " Stride: " << comp_stride << " Offset: "
                    << offset << " Comp Size: " << comp_size << " Comp Type: " << type << std::endl;
                std::cout << ak_meshInputCount(mesh) << std::endl;

            };
            break;
        case AK_GEOMETRY_SPLINE: geo_type = "spline"; break;
        case  AK_GEOMETRY_BREP:  geo_type = "brep";   break;
        default:                 geo_type = "other";  break;
        };

    }
    std::cout << "Node name: " << node->name << std::endl;
    //std::cout << "No. " << j++ << std::endl;
    std::cout << "Node type: " << geo_type << std::endl;
    
    free(transform);
    free(word_transform);

    if (node->next) {
        node = node->next;
        proccess_node(node, node_ptr);
    }
    if (node->chld) {
        node = node->chld;
        proccess_node(node, node_ptr);
    }
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


std::string printCoordSys(AkCoordSys* coord) {
    if (coord) {
        AkAxis axis[] = { coord->axis.fwd,
        coord->axis.right,
        coord->axis.up,
        coord->cameraOrientation.fwd,
        coord->cameraOrientation.right,
        coord->cameraOrientation.up };
        std::string ax_name[] = { "axis FW:" ,"axis RH:" ,"axis UP:", "camera FW:", "camera RH:", "camera UP : "};

        AkAxisRotDirection axis_dir = coord->rotDirection;
        std::string coordString;

        for (int i = 0; i < sizeof(axis)/sizeof(AkAxis); i++) {
            std::string st;
            switch (axis[i]) {
            case AK_AXIS_NEGATIVE_X: st = "NEGATIVE_X"; break;
            case AK_AXIS_NEGATIVE_Y: st = "NEGATIVE_Y"; break;
            case AK_AXIS_NEGATIVE_Z: st = "NEGATIVE_Z"; break;
            case AK_AXIS_POSITIVE_X: st = "POSITIVE_X"; break;
            case AK_AXIS_POSITIVE_Y: st = "POSITIVE_Y"; break;
            case AK_AXIS_POSITIVE_Z: st = "POSITIVE_Z"; break;
            }
            coordString += ax_name[i] + " " + st + "\n";
        }
        switch (axis_dir) {
        case AK_AXIS_ROT_DIR_LH: coordString += "rot dir: ROT LEFT\n";  break;
        case AK_AXIS_ROT_DIR_RH: coordString += "rot dir: ROT RIGHT\n";  break;
        }
        return coordString;
    }
    return "CoordSys is nullptr!\n";
}

std::string printInf(AkDocInf* inf, AkUnit* unit) {
    std::string infString;
    if (inf && unit) {
        infString += "Units: " + std::string(unit->name) + " ";
        infString += unit->dist;
        infString += "\nPath: " + std::string(inf->name);
        infString += "\nFlip Image: ";
        infString += inf->flipImage ? "True" : "False";
        infString+= "\n";
        if (AK_FILE_TYPE_GLTF == inf->ftype) {
            infString += "Type: GLTF\n";
        }else {
            infString += "Unknown type\n";
        }
        
        return infString;
    }
    return "AkDocInf or AkUnit is nullptr!\n";
}

int main(void)
{
    std::cout << "========== Initialization started ============================\n";
    if (!glfwInit()) {
        std::cout << "========== [GLFW]: Initialization failed =====================\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "========== [GLFW]: Terminated ================================\n";
        std::cout << "========== [GLFW]: Window initialization failed ==============\n";
        return 1;
    }
    

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetKeyCallback(window, key_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetScrollCallback(window, scroll_callback);

    initializeGLEW();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 450";
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "========== [GLFW]: Debug context initialize successful =========\n";
        std::vector<DEBUGPROC> callbacks;
        callback_list(callbacks);
        debug_init(callbacks);
    }  else {
        std::cout << "========== [GLFW]: Debug context initialize unsuccessful =========\n";
    }

    
    char* v_sh_buffer = read_file("res/vertex2.glsl");
    if (!v_sh_buffer){
        std::cout << "=================== Coulnt find res/vertex.glsl =======================\n";
    }
    
    char* f_sh_buffer = read_file("res/fragment2.glsl");
    if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl =======================\n";
    

    GLuint vao, vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glObjectLabel(GL_SHADER, vertex_shader, -1, "Vertex Shader");
    glShaderSource(vertex_shader, 1, &v_sh_buffer, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glObjectLabel(GL_SHADER, fragment_shader, -1, "Fragment Shader");
    glShaderSource(fragment_shader, 1, &f_sh_buffer, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glObjectLabel(GL_PROGRAM, program, -1, "Volumetric lighting");
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    //GLuint pipeline;
    //glGenProgramPipelines(1, &pipeline);
    //glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, fragment_shader);
    //glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, vertex_shader);
    
    /* ======================================================== */
    checkPipelineStatus(vertex_shader, fragment_shader, program);
    /* ======================================================== */


    ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    AkDoc* doc;
    AkVisualScene* scene;
    AkCamera* camera;
    AkInstanceGeometry* geometry;
    AkNode* root, * node_ptr;

    std::string scene_path = "./res/ship_in_clouds/scene.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        std::cout << "Document couldn't be loaded\n";
    }else {
        std::cout << printCoordSys(doc->coordSys);
        std::cout << printInf(doc->inf, doc->unit);
        std::cout << "===============================================================\n";
    }

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));
    int8_t* raw_buffer;
    uint32_t* indecies = nullptr;
    unsigned int indecies_size = 0;
    int buffer_size = 0;
    
    if (doc->scene.visualScene) {
        scene = (AkVisualScene*) ak_instanceObject(doc->scene.visualScene);
        std::cout << "Visual Scene loaded\n";

        if(scene->name) std::cout << "Scene name: " << scene->name << std::endl;
        if (scene->lights)
            if (scene->lights->first) {
                AkLight* light = (AkLight*) ak_instanceObject(scene->lights->first->instance);
                if(light)
                std::cout << "Light name: " << light->name << std::endl;
            }
        if (scene->cameras)
            if (scene->cameras->first) {
                AkCamera* camera = (AkCamera*) ak_instanceObject(scene->cameras->first->instance);
                if(camera)
                std::cout << "Camera name: " << camera->name << std::endl;
            }
        ak_firstCamera(doc, &camera, camera_mat, camera_proj);
        if (camera)
            std::cout << "Camera:" << camera->name << std::endl;
        for (int i = 0; i < 16; i++) {
            std::cout << camera_mat[i] << ", ";
            if (i % 4 == 3) std::cout << std::endl;
        }


        AkMaterial* m = (AkMaterial*) doc->lib.materials->chld;
        do {
            AkEffect* ef = (AkEffect*)ak_instanceObject(&m->effect->base);
            AkTechniqueFxCommon* tch = ef->profile->technique->common;
            m = (AkMaterial*)m->base.next;
        }
        while (m);

        //AkNode* node_ptr = (AkNode*) doc->lib.nodes->chld;
        AkNode* node_ptr = ak_instanceObjectNode(scene->node);
        AkNode* node = node_ptr;
        int j = 0;

        proccess_node(node, node_ptr);
    }

    glm::mat4 Camera = glm::make_mat4x4(camera_mat);
    glm::mat4 Projection = glm::make_mat4x4(camera_proj);
    if (camera_mat) free(camera_mat);
    if (camera_proj) free(camera_proj);



    /* ======================================================== */

    mvp_location = glGetUniformLocation(program, "MVP");



   




    std::cout << "===================== Main loop ===================\n";
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;
        
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float r = 0.1 * panel_config.p6;
        float theta = 3.14 * panel_config.p7 / 180;
        float phi = 3.14 * panel_config.p8 / 180;

        glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
        eye = glm::vec3(eye.z, eye.y, eye.x);
        glm::vec3 north = r * glm::euclidean(glm::vec2(theta, phi+0.01));
        if (theta > 90 && theta < 270) {
            north = glm::vec3(north.z, -north.y, north.x);
        } else {
            north = glm::vec3(north.z, north.y, north.x);
        }

        glm::vec3 translate = glm::vec3(panel_config.p1 * 0.1, panel_config.p2 * 0.1, panel_config.p3 * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.p4/180, 3.14 * panel_config.p5 / 180, 0.f);

        glUseProgram(program);
        //glBindProgramPipeline(pipeline);

        glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
        //glm::mat4 Projection = glm::perspectiveFov((float) 3.14*panel_config.fov/180, (float) width, (float) height, panel_config.near_plane, panel_config.far_plane);

        glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), translate);
        glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
        glm::mat4 View = glm::rotate(ViewRotateX, rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));   
        glm::mat4 MVP = Projection * LookAt * View * Model;
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
        
        
        glDrawElements(GL_TRIANGLES, indecies_size, GL_UNSIGNED_INT, indecies);

        //glBindProgramPipeline(0);

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

    //glDeleteProgramPipelines(1, &pipeline);

    //GLuint buffers[] = {vertex_buffer};
    //glDeleteBuffers(1, buffers);
    glDeleteVertexArrays(1, &vao);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glDeleteProgram(program);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    std::cout << "========== [GLFW]: Terminated ================================\n";
    std::cout << "===================== Exit succeeded =========================\n";
    return 0;
}