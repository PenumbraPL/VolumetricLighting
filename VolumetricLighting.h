#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "pch.h"

//#define GLEW_STATIC
#include "GUI.h"
#include "Draw.h"
#include "IO.h"


void formatAttribute(GLint attr_location, AkAccessor* acc);
char* read_file(const char* file_name);


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



ConfigContext panel_config{
    500.f, .001f, 50, 0, 0, 0, 0, 0, 50, 0, 0, false, false, { 0.4f, 0.7f, 0.0f, 0.5f }, { 0.4f, 0.7f, 0.0f, 0.5f },{ 0.4f, 0.7f, 0.0f, 0.5f }, { 0.0f, 0.0f, 0.0f }, 0.1, 0.5, 0.5
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

std::vector<PointLight> lights_list;


void init_lights(void) {
    lights_list.push_back({ glm::vec3(1.5, 1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.), glm::vec3(1., 1., 1.) });
    lights_list.push_back({ glm::vec3(-1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1)});
    lights_list.push_back({ glm::vec3(1.5, -1.5, 1.5), 0.1, 0.5, 0.5, glm::vec3(1., .9, .8), glm::vec3(.7, .5, .4), glm::vec3(.3, .2, .1) });
}

bool compare_lights(PointLight& old_light, PointLight& new_light) {
    return memcmp(&old_light, &new_light, sizeof(PointLight));
}

bool compare_lights(LightsList& old_light, LightsList& new_light) {
    if (old_light.size != new_light.size) return true;
    
    return memcmp(&old_light.list, &new_light.list, old_light.size * sizeof(PointLight));
}


void insert_tree(ConfigContext& context, std::vector<std::string> & tree) {
    context.directory = &tree;
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

        char* v_sh_buffer = read_file("res/shaders/environment_vec.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/shaders/environment_frag.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";


        vertex_program = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        fragment_program = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);
        free(v_sh_buffer);
        free(f_sh_buffer);
        GLint status = 1;

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

        std::string scene_path = "res/models/environment/";
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
        char* skybox_image = (char*)imageLoadFromFile("res/models/environment/Environment.jpg", &width, &height, &comp);
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

        glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panel_config.tr_x * 0.1, panel_config.tr_y * 0.1, panel_config.tr_z * 0.1);
        glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panel_config.rot_x / 180, 3.14 * panel_config.rot_y / 180, 0.f);

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

