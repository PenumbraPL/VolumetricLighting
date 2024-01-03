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
#include <filesystem>
#include <map>

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
    SIZE,
    SKYBOX
};

enum ShaderTypes {
    VERTEX,
    FRAGMENT,
    TESS_CTR,
    TESS_EV,
    GEOMETRY
};

ConfigContext panel_config{
    500.f, .001f, 50, 0, 0, 0, 0, 0, 50, 0, 0, { 0.4f, 0.7f, 0.0f, 0.5f }, { 0.4f, 0.7f, 0.0f, 0.5f },{ 0.4f, 0.7f, 0.0f, 0.5f }, { 0.0f, 0.0f, 0.0f }, 0.1, 0.5, 0.5
};


void insert_tree(ConfigContext& context, std::vector<std::string> & tree) {
    context.directory = &tree;
}


void formatAttribute(GLint attr_location, AkAccessor* acc) {
    int comp_size       = acc->componentSize;;
    int type            = acc->componentType;
    GLuint normalize    = acc->normalized ? GL_TRUE : GL_FALSE;
    size_t offset       = acc->byteOffset;
    int comp_stride     = acc->componentBytes;
    size_t length       = acc->byteLength;

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

    glVertexAttribFormat(attr_location, comp_size, type, normalize, 0);
}

void*
imageLoadFromFile(const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) {
    if (std::string::npos != std::string(path).find(".png", 0)) {
        return stbi_load(path, width, height, components, STBI_rgb_alpha);
    }
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

        char* v_sh_buffer = read_file("res/vertex7.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/fragment7.glsl");
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
        memset(textures, 0, 8);
        tex_type = (GLuint*)calloc(8, sizeof(GLuint));
        memset(tex_type, 0, 8);
        return textures;
    }

    GLuint* createSamplers() {
        samplers = (GLuint*)calloc(8, sizeof(GLuint));
        memset(samplers, 0, 8);
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
    enum LightType {POSITIONAL, DIRECTIONAL, AREA} light_type = POSITIONAL;
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction = glm::vec4(0,0,0,0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* nor = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint pipeline;
    GLuint mvp_location;
    GLuint buffer;

    void createPipeline() {

        char* v_sh_buffer = read_file("res/lamp_vec.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/lamp_frag.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";


        GLint status = 1;


        vertex_program = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);

        free(v_sh_buffer);
        free(f_sh_buffer);


        if (status) {
            GLint link_status;

            glGetProgramiv(vertex_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(vertex_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
        }

        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertex_program);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragment_program);

         
    }

    void deletePipeline() {
        glDeleteProgram(vertex_program);
        glDeleteProgram(fragment_program);
        glDeleteBuffers(1, &buffer);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
    }

    void loadMesh() {
        AkDoc* doc;
        AkVisualScene* scene;
        AkInstanceGeometry* geometry;

        std::string scene_path = "res/";
        scene_path += "lamp.gltf";
        if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
            std::cout << "Light mesh couldn't be loaded\n";
            return;
        }
        if (!doc->scene.visualScene) {
            std::cout << "Light mesh couldn't be loaded\n";
            return;
        }
        
        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        AkNode* node = ak_instanceObjectNode(scene->node);

        if (node->geometry) {
            AkGeometry* geometry = ak_instanceObjectGeom(node);
            AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
            if ((AkGeometryType)geometry->gdata->type) {
                if (mesh) {
                    AkMeshPrimitive* prim = mesh->primitive;

                    if (prim->indices) {
                        ind = (uint32_t*)prim->indices->items;
                        ind_size = prim->indices->count;
                    }

                    int set = prim->input->set;
                    pos = ak_meshInputGet(prim, "POSITION", set);
                    nor = ak_meshInputGet(prim, "NORMAL", set);
                
                    glCreateBuffers(1, &buffer);
                    glNamedBufferData(buffer, pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                };
            }
        }
    }

    void drawLight(int width, int height, glm::mat4 Proj, AkCamera* camera) {
        mvp_location = glGetUniformLocation(vertex_program, "MVP");
        GLuint vcol_location = glGetAttribLocation(vertex_program, "vCol");
        GLuint vpos_location = glGetAttribLocation(vertex_program, "vPos");

        if (vpos_location != -1) formatAttribute(vpos_location, pos->accessor);
        if (vcol_location != -1) formatAttribute(vcol_location, nor->accessor);

        if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
        if (vpos_location != -1) glEnableVertexAttribArray(vpos_location);
        if (vcol_location != -1) glEnableVertexAttribArray(vcol_location);

        float r = 0.1 * panel_config.dist;
        float phi = panel_config.phi;
        float theta = panel_config.theta;

        glm::mat4x4 Projection;

        glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
        eye = glm::vec3(eye.z, eye.y, eye.x);

        glm::vec3 north = glm::vec3(0., 1., 0.);
        float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
        if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
            north = glm::vec3(0., -1., 0.);
        }

        glm::vec3 translate = glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);
        
        glBindProgramPipeline(0);
        glBindProgramPipeline(pipeline);

        glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        else Projection = Proj;

        glm::mat4 View = glm::rotate(
            glm::rotate(
                glm::translate(
                    transform
                    , translate)
                , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
            rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
        glm::mat4 MVP = Projection * LookAt * View * Model;
        glProgramUniformMatrix4fv(vertex_program, mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));

        int binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer, pos->accessor->byteOffset, pos->accessor->componentBytes);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
};

struct Camera {
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction;
    float zNear;
    float zFar;
    int fov;
};




struct Environment {
    glm::mat4x4 transform = glm::mat4x4(0.);
    glm::mat4x4 w_transform = glm::mat4x4(0.);

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint pipeline;
    GLuint skybox;
    GLuint env_sampler;
    glm::mat4 MVP;
    GLuint* buffer;
    GLuint mvp_location;

    void createPipeline() {

        char* v_sh_buffer = read_file("res/shaders/env_vec.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/shaders/env_frag.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";


        GLint status = 1;


        vertex_program = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);

        free(v_sh_buffer);
        free(f_sh_buffer);


        if (status) {
            GLint link_status;

            glGetProgramiv(vertex_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(vertex_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
        }

        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertex_program);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragment_program);


    }

    void deletePipeline() {
        glDeleteProgram(vertex_program);
        glDeleteProgram(fragment_program);
        glDeleteBuffers(2, buffer);
        free(buffer);
        glDeleteSamplers(1, &env_sampler);
        glDeleteTextures(1, &skybox);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
    }

    void loadMesh() {
        AkDoc* doc;
        AkVisualScene* scene;
        AkInstanceGeometry* geometry;

        std::string scene_path = "res/environment/";
        scene_path += "env_sphere.gltf";
        if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
            std::cout << "Environment mesh couldn't be loaded\n";
            return;
        }
        if (!doc->scene.visualScene) {
            std::cout << "Environment mesh couldn't be loaded\n";
            return;
        }

        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        AkNode* node = ak_instanceObjectNode(scene->node);

        float* t1 = (float*)calloc(16, sizeof(float));
        float* t2 = (float*)calloc(16, sizeof(float));
        ak_transformCombineWorld(node, t1);
        ak_transformCombine(node, t2);
        w_transform = glm::make_mat4x4(t1);
        transform = glm::make_mat4x4(t2);
        free(t1);
        free(t2);

        if (node->geometry) {
            AkGeometry* geometry = ak_instanceObjectGeom(node);
            AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
            if ((AkGeometryType)geometry->gdata->type) {
                if (mesh) {
                    AkMeshPrimitive* prim = mesh->primitive;

                    if (prim->indices) {
                        ind = (uint32_t*)prim->indices->items;
                        ind_size = prim->indices->count;
                    }

                    int set = prim->input->set;
                    pos = ak_meshInputGet(prim, "POSITION", set);
                    tex = ak_meshInputGet(prim, "TEXCOORD", set);

                    buffer = (GLuint*)calloc(2, sizeof(GLuint));
                    glCreateBuffers(2, buffer);
                    glNamedBufferData(buffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                    glNamedBufferData(buffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
                };
            }
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &skybox);
        int width, height, comp;
        char* skybox_image = (char*)imageLoadFromFile("res/environment/Environment.jpg", &width, &height, &comp);
        glTextureStorage2D(skybox, 1, GL_RGB8, width, height);
        glTextureSubImage2D(skybox, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, skybox_image);

        stbi_image_free(skybox_image);
        glGenSamplers(1, &env_sampler);
        glSamplerParameteri(env_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glSamplerParameteri(env_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glSamplerParameteri(env_sampler, GL_TEXTURE_WRAP_R, GL_CLAMP);
        glSamplerParameteri(env_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(env_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }

    void draw(int width, int height, glm::mat4 Proj, AkCamera* camera) {
        mvp_location = glGetUniformLocation(vertex_program, "MVP");
        GLuint vtex_location = glGetAttribLocation(vertex_program, "vTex");
        GLuint vpos_location = glGetAttribLocation(vertex_program, "vPos");

        if (vpos_location != -1) formatAttribute(vpos_location, pos->accessor);
        if (vtex_location != -1) formatAttribute(vtex_location, tex->accessor);

        if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
        if (vpos_location != -1) glEnableVertexAttribArray(vpos_location);
        if (vtex_location != -1) glEnableVertexAttribArray(vtex_location);

        float r = 0.1 * panel_config.dist;
        float phi = panel_config.phi;
        float theta = panel_config.theta;

        glm::mat4x4 Projection;

        glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
        eye = glm::vec3(eye.z, eye.y, eye.x);

        glm::vec3 north = glm::vec3(0., 1., 0.);
        float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
        if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
            north = glm::vec3(0., -1., 0.);
        }

        glm::vec3 translate = glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);

        glBindProgramPipeline(pipeline);

        glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        else Projection = Proj;

        glm::mat4 View = glm::rotate(
            glm::rotate(
                glm::translate(
                    transform
                    , translate)
                , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
            rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
        MVP = Projection * LookAt * View * Model;
        glProgramUniformMatrix4fv(vertex_program, mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));

        int binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

        binding_point = 1;
        glVertexAttribBinding(vtex_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], tex->accessor->byteOffset, tex->accessor->componentBytes);



        glBindSampler(0, env_sampler);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox);

        glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
    }
};


struct Cloud {
    glm::mat4x4 transform = glm::mat4x4(0.);
    glm::mat4x4 w_transform = glm::mat4x4(0.);

    int width = 0;
    int height = 0;

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint init_fragment_program;
    GLuint clear_fragment_program;
    GLuint vertex_plane_program;
    GLuint pipeline;
    GLuint init_pipeline;
    GLuint clear_pipeline;

    glm::mat4 MVP;
    GLuint* buffer;
    GLuint mvp_location;
    GLuint prj_location;
    GLuint img_location;


    GLuint depth_buffer;
    GLuint atomic_buffer;
    GLuint head_pointer_image;
    GLuint depth_image;

    unsigned int zero = 0;
    
    void createPipeline(int width, int height) {
        this->width = width;
        this->height = height;

        char* v_sh_buffer = read_file("res/shaders/depth_ver.glsl");
        if (!v_sh_buffer)  std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        
        char* vd_sh_buffer = read_file("res/shaders/depth2_ver.glsl");
        if (!vd_sh_buffer)  std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";

        char* f_sh_buffer = read_file("res/shaders/depth_frag.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";
        
        char* if_sh_buffer = read_file("res/shaders/vol_frag.glsl");
        if (!if_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";

        char* cf_sh_buffer = read_file("res/shaders/clear_vol_frag.glsl");
        if (!cf_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";


        vertex_program = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        vertex_plane_program = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vd_sh_buffer);
        fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);
        init_fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &if_sh_buffer);
        clear_fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &cf_sh_buffer);


        free(v_sh_buffer);
        free(vd_sh_buffer);
        free(f_sh_buffer);
        free(if_sh_buffer);
        free(cf_sh_buffer);


        GLint status = 1;
        if (status) {
            GLint link_status;

            glGetProgramiv(vertex_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(vertex_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(vertex_plane_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(vertex_plane_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(init_fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(init_fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
            glGetProgramiv(clear_fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(clear_fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }
        }


        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertex_program);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragment_program);

        glGenProgramPipelines(1, &init_pipeline);
        glUseProgramStages(init_pipeline, GL_VERTEX_SHADER_BIT, vertex_program);
        glUseProgramStages(init_pipeline, GL_FRAGMENT_SHADER_BIT, init_fragment_program);

        glGenProgramPipelines(1, &clear_pipeline);
        glUseProgramStages(clear_pipeline, GL_VERTEX_SHADER_BIT, vertex_program);
        glUseProgramStages(clear_pipeline, GL_FRAGMENT_SHADER_BIT, clear_fragment_program);


        
        glGenBuffers(1, &atomic_buffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_buffer);
        glNamedBufferData(atomic_buffer, sizeof(unsigned int), NULL, GL_DYNAMIC_COPY);
        
        glGenBuffers(1, &depth_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, depth_buffer);
        glNamedBufferData(depth_buffer, width*height*16, NULL, GL_DYNAMIC_COPY);

        //glTexImage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1024, 1024, 0, GL_R32UI, image, pixels);
        glGenTextures(1, &head_pointer_image);
        glBindTexture(GL_TEXTURE_2D, head_pointer_image);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
        
        glGenTextures(1, &depth_image);
        glBindTexture(GL_TEXTURE_2D, depth_image);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, width, height);
    }

    void deletePipeline() {
        glDeleteProgram(vertex_program);
        glDeleteProgram(fragment_program);
        glDeleteProgram(vertex_plane_program);
        glDeleteProgram(init_fragment_program);
        glDeleteProgram(clear_fragment_program);
        glDeleteBuffers(2, buffer);
        glDeleteBuffers(1, &atomic_buffer);
        glDeleteBuffers(1, &depth_buffer);
        glDeleteTextures(1, &head_pointer_image);
        glDeleteTextures(1, &depth_image);
        free(buffer);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
        glDeleteProgramPipelines(1, &init_pipeline);
        glDeleteProgramPipelines(1, &clear_pipeline);
    }

    void loadMesh() {
        AkDoc* doc;
        AkVisualScene* scene;
        AkInstanceGeometry* geometry;

        std::string scene_path = "res/cube/";
        scene_path += "Cube.gltf";
        if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
            std::cout << "Cloud mesh couldn't be loaded\n";
            return;
        }
        if (!doc->scene.visualScene) {
            std::cout << "Cloud mesh couldn't be loaded\n";
            return;
        }

        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        AkNode* node = ak_instanceObjectNode(scene->node);

        float* t1 = (float*)calloc(16, sizeof(float));
        float* t2 = (float*)calloc(16, sizeof(float));
        ak_transformCombineWorld(node, t1);
        ak_transformCombine(node, t2);
        w_transform = glm::make_mat4x4(t1);
        transform = glm::make_mat4x4(t2);
        free(t1);
        free(t2);

        if (node->geometry) {
            AkGeometry* geometry = ak_instanceObjectGeom(node);
            AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
            if ((AkGeometryType)geometry->gdata->type) {
                if (mesh) {
                    AkMeshPrimitive* prim = mesh->primitive;

                    if (prim->indices) {
                        ind = (uint32_t*)prim->indices->items;
                        ind_size = prim->indices->count;
                    }

                    int set = prim->input->set;
                    pos = ak_meshInputGet(prim, "POSITION", set);
                    tex = ak_meshInputGet(prim, "TEXCOORD", set);

                    buffer = (GLuint*)calloc(2, sizeof(GLuint));
                    glCreateBuffers(2, buffer);
                    glNamedBufferData(buffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                    glNamedBufferData(buffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
                };
            }
        }
    }

    void draw(int width, int height, glm::mat4 Proj, AkCamera* camera) {
        mvp_location = glGetUniformLocation(vertex_program, "MVP");
        prj_location = glGetUniformLocation(vertex_program, "PRJ");
        img_location = glGetUniformLocation(clear_fragment_program, "image");

        GLuint vtex_location = glGetAttribLocation(vertex_program, "vTex");
        GLuint vpos_location = glGetAttribLocation(vertex_program, "vPos");

        if (vpos_location != -1) formatAttribute(vpos_location, pos->accessor);
        if (vtex_location != -1) formatAttribute(vtex_location, tex->accessor);

        if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
        if (prj_location != -1) glEnableVertexAttribArray(prj_location);
        if (img_location != -1) glEnableVertexAttribArray(img_location);
        if (vpos_location != -1) glEnableVertexAttribArray(vpos_location);
        if (vtex_location != -1) glEnableVertexAttribArray(vtex_location);


        glClearTexImage(depth_image, 0, GL_RG, GL_FLOAT, NULL);
        //glTextureSubImage2D(depth_image, 0, 0, 0, width, height, GL_RG, GL_FLOAT, NULL);

        float r = 0.1 * panel_config.dist;
        float phi = panel_config.phi;
        float theta = panel_config.theta;

        glm::mat4x4 Projection;

        glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
        eye = glm::vec3(eye.z, eye.y, eye.x);

        glm::vec3 north = glm::vec3(0., 1., 0.);
        float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
        if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
            north = glm::vec3(0., -1., 0.);
        }

        glm::vec3 translate = glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);
   
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        //glBindTexture(GL_TEXTURE_2D, head_pointer_image);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        
        //glBindImageTexture(img_location, depth_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
        //glBindProgramPipeline(clear_pipeline);
        int binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        //img_location = glGetUniformLocation(init_fragment_program, "image");
        //glBindImageTexture(img_location, depth_image, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
//        glBindProgramPipeline(init_pipeline);
        glBindProgramPipeline(pipeline);


        glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        else Projection = Proj;

        glm::mat4 View = glm::rotate(
            glm::rotate(
                glm::translate(
                    transform
                    , translate)
                , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
            rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
        MVP = LookAt * View * Model;
        glProgramUniformMatrix4fv(vertex_program, mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
        glProgramUniformMatrix4fv(vertex_program, prj_location, 1, GL_FALSE, glm::value_ptr(Projection));


        
        //static const unsigned int zero = 0;
        //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, atomic_buffer);
        //glNamedBufferSubData(atomic_buffer, 0, sizeof(zero), &zero);
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, depth_buffer);
        //glBindImageTexture(0, head_pointer_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
      

        //std::cout << "Zero: " << zero << std::endl;

        for (unsigned int tex_type = AMBIENT; tex_type < SIZE; tex_type++) {
                glActiveTexture(GL_TEXTURE0 + tex_type);
                glBindTexture(GL_TEXTURE_2D, 0);
        }

        binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

        binding_point = 1;
        glVertexAttribBinding(vtex_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], tex->accessor->byteOffset, tex->accessor->componentBytes);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        /*
    
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        glBindProgramPipeline(pipeline);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);*/
    }
};





struct Cloud2 {
    glm::mat4x4 transform = glm::mat4x4(0.);
    glm::mat4x4 w_transform = glm::mat4x4(0.);

    int width = 0;
    int height = 0;

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint init_fragment_program;
    GLuint clear_fragment_program;
    GLuint vertex_plane_program;
    GLuint pipeline;
    GLuint init_pipeline;
    GLuint clear_pipeline;

    glm::mat4 MVP;
    GLuint* buffer;
    GLuint camera_location;

    GLuint g_location;
    GLuint depth_buffer;
    GLuint atomic_buffer;
    GLuint head_pointer_image;
    unsigned int zero = 0;

    void createPipeline(int width, int height) {
        this->width = width;
        this->height = height;

        char* f_sh_buffer = read_file("res/shaders/depth_frag2.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";


        fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);
        
        free(f_sh_buffer);


        GLint status = 1;
        if (status) {
            GLint link_status;


            glGetProgramiv(fragment_program, GL_LINK_STATUS, &link_status);
            if (!link_status) {
                GLchar comp_info[1024];
                glGetProgramInfoLog(fragment_program, 1024, NULL, comp_info);

                fwrite(comp_info, 1024, 1, stdout);
            }

        }


        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragment_program);

    }

    void deletePipeline() {
        glDeleteProgram(fragment_program);
        //glDeleteBuffers(2, buffer);
        //free(buffer);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
    }


    void draw(int width, int height, glm::mat4 Proj, glm::vec3& camera, GLuint lights_buffer) {
        camera_location = glGetUniformLocation(fragment_program, "camera");
        g_location = glGetUniformLocation(fragment_program, "G");
        if (camera_location != -1) glEnableVertexAttribArray(camera_location);

        /*float r = 0.1 * panel_config.dist;
        float phi = panel_config.phi;
        float theta = panel_config.theta;

        glm::mat4x4 Projection;

        glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
        eye = glm::vec3(eye.z, eye.y, eye.x);

        glm::vec3 north = glm::vec3(0., 1., 0.);
        float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
        if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
            north = glm::vec3(0., -1., 0.);
        }

        glm::vec3 translate = glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
        glm::vec3 rotate = glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);*/

        glBindProgramPipeline(pipeline);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);
        glProgramUniform3fv(fragment_program, camera_location, 1, glm::value_ptr(camera));
        glProgramUniform1f(fragment_program, g_location, panel_config.g);
        /*
        glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panel_config.fov / 180, (float)width, (float)height, panel_config.near_plane, panel_config.far_plane);
        else Projection = Proj;

        glm::mat4 View = glm::rotate(
            glm::rotate(
                glm::translate(
                    transform
                    , translate)
                , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
            rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
        MVP = LookAt * View * Model;
        glProgramUniformMatrix4fv(vertex_program, mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
        glProgramUniformMatrix4fv(vertex_program, prj_location, 1, GL_FALSE, glm::value_ptr(Projection));

        for (unsigned int tex_type = AMBIENT; tex_type < SIZE; tex_type++) {
            glActiveTexture(GL_TEXTURE0 + tex_type);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        int binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

        binding_point = 1;
        glVertexAttribBinding(vtex_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], tex->accessor->byteOffset, tex->accessor->componentBytes);*/

        //glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
        glDisable(GL_DEPTH_TEST);
        //glFrontFace(GL_CCW);
        //glEnable(GL_BLEND);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //glFrontFace(GL_CW);
        glEnable(GL_DEPTH_TEST);

    }
};



struct PointLight {
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;
    float dummy[2];
    glm::vec3 ambient;
    float dummy2[1];
    glm::vec3 diffuse;
    float dummy3[1];
    glm::vec3 specular;
    float dummy4[1];
};

std::vector<PointLight> lights_list;


void init_lights(void) {
    lights_list.push_back({ glm::vec3(1.5, 1.5, 1.5), 0.1, 0.5, 0.5, {0,0}, glm::vec3(1., 1., 1.), {0}, glm::vec3(1., 1., 1.), {0}, glm::vec3(1., 1., 1.), { 0 } });
    lights_list.push_back({ glm::vec3(-1.5, -1.5, 1.5), 0.1, 0.5, 0.5, {0,0}, glm::vec3(1., .9, .8), {0}, glm::vec3(.7, .5, .4), {0}, glm::vec3(.3, .2, .1), {0} });
}

bool compare_lights(PointLight& old_light, PointLight& new_light) {
    return memcmp(&old_light, &new_light, sizeof(PointLight));
}

