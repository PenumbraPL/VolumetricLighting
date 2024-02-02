#pragma once
#include "GLFW/glfw3.h"
#include "pch.h";
#include "GUI.h"

extern ConfigContext panel_config;

enum TextureType {
    AMBIENT,
    EMISIVE,
    DIFFUSE,
    SPECULAR,
    SP_GLOSSINESS,
    MAT_ROUGH,
    ALBEDO,
    SP_DIFFUSE,
    TT_SIZE,
    SKYBOX
};

enum ShaderTypes {
    VERTEX,
    FRAGMENT,
    TESS_CTR,
    TESS_EV,
    GEOMETRY
};



struct Primitive {
    float* transform;
    float* w_transform;

    uint32_t* ind;
    unsigned int ind_size;
    GLuint* programs;
    GLuint pipeline;

    GLuint* textures = nullptr;
    GLuint* tex_type = nullptr;
    GLuint* samplers = nullptr;

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

        char* v_sh_buffer = read_file("res/shaders/standard_vec.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/shaders/pbr_with_ext_light_frag.glsl");
        if (!f_sh_buffer)  std::cout << "=================== Coulnt find res/fragment.glsl ============================\n";

        createPrograms();

        programs[VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &v_sh_buffer);
        programs[FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &f_sh_buffer);
        //glObjectLabel(GL_PROGRAM, vertex_shader, -1, "Primitive Vertex Shader");
        //glObjectLabel(GL_PROGRAM, fragment_shader, -1, "Primitive Fragment Shader");

        free(v_sh_buffer);
        free(f_sh_buffer);
        GLint status = 1;

        if (status) {
            GLint link_status;

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

        glGenProgramPipelines(1, &pipeline);
        glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, programs[VERTEX]);
        glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, programs[FRAGMENT]);
        //glObjectLabel(GL_PROGRAM_PIPELINE, fragment_shader, -1, "Primitive Pipeline");
    }
    void deletePipeline() {
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);

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
    enum LightType { POSITIONAL, DIRECTIONAL, AREA } light_type = POSITIONAL;
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
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

        char* v_sh_buffer = read_file("res/shaders/lamp_vec.glsl");
        if (!v_sh_buffer) {
            std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";
        }

        char* f_sh_buffer = read_file("res/shaders/lamp_frag.glsl");
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
        glDeleteBuffers(1, &buffer);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
    }

    void loadMesh() {
        AkDoc* doc;
        AkVisualScene* scene;
        AkInstanceGeometry* geometry;

        std::string scene_path = "res/modeles/";
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
    GLuint pipeline;

    glm::mat4 MVP;
    GLuint* buffer;
    GLuint mvp_location;
    GLuint prj_location;

    GLuint depth_buffer;
    unsigned int zero = 0;


    void createPipeline(int width, int height) {
        this->width = width;
        this->height = height;

        char* v_sh_buffer = read_file("res/shaders/standard_vec.glsl");
        if (!v_sh_buffer)  std::cout << "=================== Coulnt find res/vertex.glsl ==============================\n";

        char* f_sh_buffer = read_file("res/shaders/depth_frag.glsl");
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

        glGenBuffers(1, &depth_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, depth_buffer);
        glNamedBufferData(depth_buffer, width * height * 16, NULL, GL_DYNAMIC_COPY);

    }

    void deletePipeline() {
        glDeleteProgram(vertex_program);
        glDeleteProgram(fragment_program);
        glDeleteBuffers(2, buffer);
        glDeleteBuffers(1, &depth_buffer);
        free(buffer);
        glBindProgramPipeline(0);
        glDeleteProgramPipelines(1, &pipeline);
    }

    void loadMesh() {
        AkDoc* doc;
        AkVisualScene* scene;
        AkInstanceGeometry* geometry;

        std::string scene_path = "res/models/cube/";
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

    void draw(int width, int height, glm::mat4 Proj, AkCamera* camera, float& g, GLuint lightbuffer) {
        mvp_location = glGetUniformLocation(vertex_program, "MVP");
        prj_location = glGetUniformLocation(vertex_program, "PRJ");
        GLuint g_location = glGetUniformLocation(fragment_program, "G");
        GLuint camera_location = glGetUniformLocation(fragment_program, "camera");

        GLuint vtex_location = glGetAttribLocation(vertex_program, "vTex");
        GLuint vpos_location = glGetAttribLocation(vertex_program, "vPos");

        if (vpos_location != -1) formatAttribute(vpos_location, pos->accessor);
        if (vtex_location != -1) formatAttribute(vtex_location, tex->accessor);

        if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
        if (prj_location != -1) glEnableVertexAttribArray(prj_location);
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

        int binding_point = 0;
        glVertexAttribBinding(vpos_location, binding_point);
        glBindVertexBuffer(binding_point, buffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

        glBindProgramPipeline(pipeline);

        glProgramUniform1f(fragment_program, g_location, g);
        glProgramUniform3fv(fragment_program, camera_location, 1, glm::value_ptr(eye));
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightbuffer);

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
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
        MVP = LookAt * View * Model;
        glProgramUniformMatrix4fv(vertex_program, mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
        glProgramUniformMatrix4fv(vertex_program, prj_location, 1, GL_FALSE, glm::value_ptr(Projection));

        for (unsigned int tex_type = AMBIENT; tex_type < TT_SIZE; tex_type++) {
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
        glEnable(GL_BLEND);
        glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
    }
};