#include "GL/glew.h"
#include "pch.h"
#include "Models.h"



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



/* ================================================ */


void Environment::createPipeline() {

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

void Environment::deletePipeline() {
    glDeleteProgram(vertex_program);
    glDeleteProgram(fragment_program);
    glDeleteBuffers(2, buffer);
    free(buffer);
    glDeleteSamplers(1, &env_sampler);
    glDeleteTextures(1, &skybox);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Environment::loadMesh() {
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

void Environment::draw(int width, int height, glm::mat4 Proj, AkCamera* camera) {
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



/* ================================================ */

void Primitive::getLocation() {
    GLuint& vertex_program = programs[VERTEX];
    GLuint& fragment_program = programs[FRAGMENT];
    // TO DO
    mvp_location = glGetUniformLocation(vertex_program, "MVP");
    norm_location = glGetAttribLocation(vertex_program, "vNor");
    vpos_location = glGetAttribLocation(vertex_program, "vPos");
    tex_location = glGetAttribLocation(vertex_program, "vTex");
    is_tex_location = glGetUniformLocation(fragment_program, "isTexture");
    prj_location = glGetUniformLocation(vertex_program, "PRJ");
    camera_location = glGetUniformLocation(fragment_program, "camera");
    g_location = glGetUniformLocation(fragment_program, "G");
    dir_location = glGetUniformLocation(fragment_program, "direction");

    if (mvp_location != -1) glEnableVertexAttribArray(mvp_location);
    if (vpos_location != -1) glEnableVertexAttribArray(vpos_location);
    if (norm_location != -1) glEnableVertexAttribArray(norm_location);
    if (tex_location != -1) glEnableVertexAttribArray(tex_location);
    if (is_tex_location != -1) glEnableVertexAttribArray(is_tex_location);
    if (prj_location != -1) glEnableVertexAttribArray(prj_location);
    if (camera_location != -1) glEnableVertexAttribArray(camera_location);


    if (vpos_location != -1) formatAttribute(vpos_location, pos);
    if (norm_location != -1) formatAttribute(norm_location, nor);
    if (tex_location != -1) formatAttribute(tex_location, tex);
    //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
}


void Primitive::draw(GLuint& lights_buffer, std::map <void*, unsigned int>& bufferViews, GLuint* buffers,
    glm::vec3& eye,
glm::mat4& LookAt, glm::mat4& Projection,
glm::vec3& translate, glm::vec3& rotate){
    //glm::vec3 eye;
    //glm::mat4 LookAt, Projection;
    //glm::vec3 translate, rotate;
    
    glBindProgramPipeline(pipeline);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);

    glm::mat4 View = glm::rotate(
        glm::rotate(
            glm::translate(
                glm::make_mat4x4(transform)
                , translate)
            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 MVP = LookAt * View * Model;
    glm::vec3 camera_view = glm::vec4(eye, 1.0);
    glm::vec3 camera_dir = glm::vec3(0.) - eye;

    glProgramUniformMatrix4fv(programs[VERTEX], mvp_location, 1, GL_FALSE, glm::value_ptr(MVP));
    glProgramUniformMatrix4fv(programs[VERTEX], prj_location, 1, GL_FALSE, glm::value_ptr(Projection));
    glProgramUniform3fv(programs[FRAGMENT], camera_location, 1, glm::value_ptr(camera_view));
    glProgramUniform1f(programs[FRAGMENT], g_location, panel_config.g);
    glProgramUniform3fv(programs[FRAGMENT], dir_location, 1, glm::value_ptr(camera_dir));

    if (!textures[ALBEDO])
        glProgramUniform1ui(programs[FRAGMENT], is_tex_location, GL_FALSE);

    int j = bufferViews[pos->buffer];
    int binding_point = 0;
    glVertexAttribBinding(vpos_location, binding_point);
    glBindVertexBuffer(binding_point, buffers[j], pos->byteOffset, pos->componentBytes);

    if (norm_location != -1) {
        j = bufferViews[nor->buffer];
        binding_point = 1;
        glVertexAttribBinding(norm_location, binding_point);
        glBindVertexBuffer(binding_point, buffers[j], nor->byteOffset, nor->componentBytes);
    }

    if (tex_location != -1) {
        j = bufferViews[tex->buffer];
        binding_point = 2;
        glVertexAttribBinding(tex_location, binding_point);
        glBindVertexBuffer(binding_point, buffers[j], tex->byteOffset, tex->componentBytes);
    }
    for (unsigned int type = AMBIENT; type < TT_SIZE; type++) {
        GLuint sampler = samplers[(TextureType)type];
        GLuint texture = textures[(TextureType)type];
        if (sampler && texture) {
            //glProgramUniform1ui(programs[FRAGMENT], is_tex_location, GL_TRUE);
            glBindSampler(type, sampler);
            glActiveTexture(GL_TEXTURE0 + type);
            glBindTexture(tex_type[type], texture);
        }
        else {
            glActiveTexture(GL_TEXTURE0 + type);
            glBindTexture(GL_TEXTURE_2D, 0); // needs change
        }

        /*
        else {
            glProgramUniform1ui(programs[FRAGMENT], is_tex_location, GL_FALSE);
        }*/
    }

    glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
}

float* Primitive::setTransform(void) {
    return transform = (float*)calloc(16, sizeof(float));
}

float* Primitive::setWorldTransform(void) {
    return w_transform = (float*)calloc(16, sizeof(float));
}

void Primitive::deleteTransforms() {
    if (transform) free(transform);
    if (w_transform) free(w_transform);
}

GLuint* Primitive::createPrograms() {
    programs = (GLuint*)calloc(5, sizeof(GLuint));
    memset(textures, -1, 5);
    return programs;
}

void Primitive::createPipeline() {

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
void Primitive::deletePipeline() {
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);

    deletePrograms();
}


void Primitive::deletePrograms() {
    for (int i = 0; i < 5; i++) glDeleteProgram(programs[i]);
    if (programs) free(programs);
}

GLuint* Primitive::createTextures() {
    textures = (GLuint*)calloc(8, sizeof(GLuint));
    memset(textures, 0, 8);
    tex_type = (GLuint*)calloc(8, sizeof(GLuint));
    memset(tex_type, 0, 8);
    return textures;
}

GLuint* Primitive::createSamplers() {
    samplers = (GLuint*)calloc(8, sizeof(GLuint));
    memset(samplers, 0, 8);
    return samplers;
}

void Primitive::deleteTexturesAndSamplers() {
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


/* ================================================ */

void Light::createPipeline() {

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

void Light::deletePipeline() {
    glDeleteProgram(vertex_program);
    glDeleteProgram(fragment_program);
    glDeleteBuffers(1, &buffer);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Light::loadMesh() {
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

void Light::drawLight(int width, int height, glm::mat4 Proj, AkCamera* camera) {
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


/* ================================================ */


void Cloud::createPipeline(int width, int height) {
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

void Cloud::deletePipeline() {
    glDeleteProgram(vertex_program);
    glDeleteProgram(fragment_program);
    glDeleteBuffers(2, buffer);
    glDeleteBuffers(1, &depth_buffer);
    free(buffer);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Cloud::loadMesh() {
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

void Cloud::draw(int width, int height, glm::mat4 Proj, AkCamera* camera, float& g, GLuint lightbuffer) {
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
