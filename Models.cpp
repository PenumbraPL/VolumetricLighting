#include "GL/glew.h"
#include "pch.h"
#include "Models.h"
#include "Tools.h"


void* imageLoadFromFile(
    const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) 
{
    if (std::string::npos != std::string(path).find(".png", 0)) {
        return stbi_load(path, width, height, components, STBI_rgb_alpha);
    }
    return stbi_load(path, width, height, components, 0);
}

void* imageLoadFromMemory(
    const char* __restrict data,
    size_t                  len,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components) 
{
    return stbi_load_from_memory((stbi_uc const*)data, (int)len, width, height, components, 0);
}

void imageFlipVerticallyOnLoad(bool flip) 
{
    stbi_set_flip_vertically_on_load(flip);
}



/* ================================================ */


void Environment::createPipeline() 
{
    std::string vertexShaderPath = "res/shaders/environment_vec.glsl";
    std::string fragmentShaderPath = "res/shaders/environment_frag.glsl";

    char* vertexShader = read_file(vertexShaderPath.c_str());
    if (!vertexShader) {
        std::cout << "=================== Coulnt find " << vertexShaderPath << " ==============================\n";
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        std::cout << "=================== Coulnt find " << fragmentShaderPath << " ============================\n";
    }

    vertexProgram   = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShader);
    fragmentProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShader);
    free(vertexShader);
    free(fragmentShader);
    GLint status = 1;

    if (status) {
        GLint linkageStatus;

        glGetProgramiv(vertexProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(vertexProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
    }

    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertexProgram);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragmentProgram);

}

void Environment::deletePipeline() 
{
    glDeleteProgram(vertexProgram);
    glDeleteProgram(fragmentProgram);
    glDeleteBuffers(2, primitiveDataBuffer);
    free(primitiveDataBuffer);
    glDeleteSamplers(1, &env_sampler);
    glDeleteTextures(1, &skybox);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Environment::loadMesh() 
{
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
    worldTransform = glm::make_mat4x4(t1);
    localTransform = glm::make_mat4x4(t2);
    free(t1);
    free(t2);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node);
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                AkMeshPrimitive* primitive = mesh->primitive;

                if (primitive->indices) {
                    ind = (uint32_t*)primitive->indices->items;
                    ind_size = primitive->indices->count;
                }

                int set = primitive->input->set;
                pos = ak_meshInputGet(primitive, "POSITION", set);
                tex = ak_meshInputGet(primitive, "TEXCOORD", set);

                primitiveDataBuffer = (GLuint*)calloc(2, sizeof(GLuint));
                glCreateBuffers(2, primitiveDataBuffer);
                glNamedBufferData(primitiveDataBuffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                glNamedBufferData(primitiveDataBuffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
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

void Environment::draw(int width, int height, glm::mat4 Proj, AkCamera* camera) 
{
    mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
    GLuint textureBindingLocation = glGetAttribLocation(vertexProgram, "vTex");
    GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");

    if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
    if (textureBindingLocation != -1) format_attribute(textureBindingLocation, tex->accessor);

    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (textureBindingLocation != -1) glEnableVertexAttribArray(textureBindingLocation);

    float r = 0.1 * panelConfig.dist;
    float phi = panelConfig.phi;
    float theta = panelConfig.theta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panelConfig.rot_x / 180, 3.14 * panelConfig.rot_y / 180, 0.f);

    glBindProgramPipeline(pipeline);

    glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.near_plane, panelConfig.far_plane);
    else Projection = Proj;

    glm::mat4 View = glm::rotate(
        glm::rotate(
            glm::translate(
                localTransform
                , translate)
            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
    MVP = Projection * LookAt * View * Model;
    glProgramUniformMatrix4fv(vertexProgram, mvpBindingLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    int binding_point = 0;
    glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

    binding_point = 1;
    glVertexAttribBinding(textureBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer[binding_point], tex->accessor->byteOffset, tex->accessor->componentBytes);


    glBindSampler(0, env_sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skybox);

    glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
}



/* ================================================ */

void Primitive::getLocation() 
{
    GLuint& vertexProgram = programs[VERTEX];
    GLuint& fragmentProgram = programs[FRAGMENT];
    // TO DO
    mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
    normalsBindingLocation = glGetAttribLocation(vertexProgram, "vNor");
    vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");
    textureBindingLocation = glGetAttribLocation(vertexProgram, "vTex");
    isTexBindingLocation = glGetUniformLocation(fragmentProgram, "isTexture");
    prjBindingLocation = glGetUniformLocation(vertexProgram, "PRJ");
    cameraBindingLocation = glGetUniformLocation(fragmentProgram, "camera");
    gBindingLocation = glGetUniformLocation(fragmentProgram, "G");
    camDirBindingLocation = glGetUniformLocation(fragmentProgram, "direction");

    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (normalsBindingLocation != -1) glEnableVertexAttribArray(normalsBindingLocation);
    if (textureBindingLocation != -1) glEnableVertexAttribArray(textureBindingLocation);
    if (isTexBindingLocation != -1) glEnableVertexAttribArray(isTexBindingLocation);
    if (prjBindingLocation != -1) glEnableVertexAttribArray(prjBindingLocation);
    if (cameraBindingLocation != -1) glEnableVertexAttribArray(cameraBindingLocation);


    if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos);
    if (normalsBindingLocation != -1) format_attribute(normalsBindingLocation, nor);
    if (textureBindingLocation != -1) format_attribute(textureBindingLocation, tex);
    //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
}


void Primitive::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* buffers,
    glm::vec3& eye,
    glm::mat4& LookAt,
    glm::mat4& Projection,
    glm::vec3& translate, 
    glm::vec3& rotate) 
{
    //glm::vec3 eye;
    //glm::mat4 LookAt, Projection;
    //glm::vec3 translate, rotate;
    
    glBindProgramPipeline(pipeline);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);

    glm::mat4 View = 
         glm::rotate(
            glm::rotate(
                glm::translate(
                    glm::make_mat4x4(localTransform)
                , translate)
            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 MVP = LookAt * View * Model;
    glm::vec3 camera_view = glm::vec4(eye, 1.0);
    glm::vec3 camera_dir = glm::vec3(0.) - eye;

    glProgramUniformMatrix4fv(programs[VERTEX], mvpBindingLocation, 1, GL_FALSE, glm::value_ptr(MVP));
    glProgramUniformMatrix4fv(programs[VERTEX], prjBindingLocation, 1, GL_FALSE, glm::value_ptr(Projection));
    glProgramUniform3fv(programs[FRAGMENT], cameraBindingLocation, 1, glm::value_ptr(camera_view));
    glProgramUniform1f(programs[FRAGMENT], gBindingLocation, panelConfig.g);
    glProgramUniform3fv(programs[FRAGMENT], camDirBindingLocation, 1, glm::value_ptr(camera_dir));

    if (!textures[ALBEDO]) {
        glProgramUniform1ui(programs[FRAGMENT], isTexBindingLocation, GL_FALSE);
    }

    int j = bufferViews[pos->buffer];
    int binding_point = 0;
    glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, buffers[j], pos->byteOffset, pos->componentBytes);

    if (normalsBindingLocation != -1) {
        j = bufferViews[nor->buffer];
        binding_point = 1;
        glVertexAttribBinding(normalsBindingLocation, binding_point);
        glBindVertexBuffer(binding_point, buffers[j], nor->byteOffset, nor->componentBytes);
    }

    if (textureBindingLocation != -1) {
        j = bufferViews[tex->buffer];
        binding_point = 2;
        glVertexAttribBinding(textureBindingLocation, binding_point);
        glBindVertexBuffer(binding_point, buffers[j], tex->byteOffset, tex->componentBytes);
    }
    for (unsigned int type = AMBIENT; type < TT_SIZE; type++) {
        GLuint sampler = samplers[(TextureType)type];
        GLuint texture = textures[(TextureType)type];
        if (sampler && texture) {
            //glProgramUniform1ui(programs[FRAGMENT], is_tex_location, GL_TRUE);
            glBindSampler(type, sampler);
            glActiveTexture(GL_TEXTURE0 + type);
            glBindTexture(texturesType[type], texture);
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

float* Primitive::setTransform(void) 
{
    return localTransform = (float*)calloc(16, sizeof(float));
}

float* Primitive::setWorldTransform(void) 
{
    return worldTransform = (float*)calloc(16, sizeof(float));
}

void Primitive::deleteTransforms() 
{
    if (localTransform) free(localTransform);
    if (worldTransform) free(worldTransform);
}

GLuint* Primitive::createPrograms() 
{
    programs = (GLuint*)calloc(5, sizeof(GLuint));
    memset(textures, -1, 5);
    return programs;
}

void Primitive::createPipeline() 
{
    std::string vertexShaderPath = "res/shaders/standard_vec.glsl";
    std::string fragmentShaderPath = "res/shaders/pbr_with_ext_light_frag.glsl";

    char* vertexShader = read_file(vertexShaderPath.c_str());
    if (!vertexShader) {
        std::cout << "=================== Coulnt find " << vertexShaderPath << " ==============================\n";
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        std::cout << "=================== Coulnt find " << fragmentShaderPath << " ============================\n";
    }

    createPrograms();

    programs[VERTEX] = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShader);
    programs[FRAGMENT] = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShader);
    //glObjectLabel(GL_PROGRAM, vertex_shader, -1, "Primitive Vertex Shader");
    //glObjectLabel(GL_PROGRAM, fragment_shader, -1, "Primitive Fragment Shader");

    free(vertexShader);
    free(fragmentShader);
    GLint status = 1;

    if (status) {
        GLint linkageStatus;

        glGetProgramiv(programs[VERTEX], GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(programs[VERTEX], 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
        glGetProgramiv(programs[FRAGMENT], GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(programs[FRAGMENT], 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
    }

    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, programs[VERTEX]);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, programs[FRAGMENT]);
    //glObjectLabel(GL_PROGRAM_PIPELINE, fragment_shader, -1, "Primitive Pipeline");
}

void Primitive::deletePipeline() 
{
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);

    deletePrograms();
}


void Primitive::deletePrograms() 
{
    for (int i = 0; i < 5; i++) glDeleteProgram(programs[i]);
    if (programs) free(programs);
}

GLuint* Primitive::createTextures() 
{
    textures = (GLuint*)calloc(8, sizeof(GLuint));
    memset(textures, 0, 8);
    texturesType = (GLuint*)calloc(8, sizeof(GLuint));
    memset(texturesType, 0, 8);
    return textures;
}

GLuint* Primitive::createSamplers() 
{
    samplers = (GLuint*)calloc(8, sizeof(GLuint));
    memset(samplers, 0, 8);
    return samplers;
}

void Primitive::deleteTexturesAndSamplers() 
{
    if (textures) {
        glDeleteTextures(8, textures);
        free(textures);
    }
    if (texturesType) {
        free(texturesType);
    }
    if (samplers) {
        glDeleteSamplers(8, samplers);
        free(samplers);
    }
}


/* ================================================ */

void Light::createPipeline() 
{
    std::string vertexShaderPath = "res/shaders/lamp_vec.glsl";
    std::string fragmentShaderPath = "res/shaders/lamp_frag.glsl";

    char* vertexShader = read_file(vertexShaderPath.c_str());
    if (!vertexShader) {
        std::cout << "=================== Coulnt find " << vertexShaderPath << " ==============================\n";
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        std::cout << "=================== Coulnt find " << fragmentShaderPath << " ============================\n";
    }

    vertexProgram = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShader);
    fragmentProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShader);
    free(vertexShader);
    free(fragmentShader);
    GLint status = 1;

    if (status) {
        GLint linkageStatus;

        glGetProgramiv(vertexProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(vertexProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
    }

    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertexProgram);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragmentProgram);
}

void Light::deletePipeline() 
{
    glDeleteProgram(vertexProgram);
    glDeleteProgram(fragmentProgram);
    glDeleteBuffers(1, &primitiveDataBuffer);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Light::loadMesh() 
{
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
                AkMeshPrimitive* primitive = mesh->primitive;

                if (primitive->indices) {
                    ind = (uint32_t*)primitive->indices->items;
                    ind_size = primitive->indices->count;
                }

                int set = primitive->input->set;
                pos = ak_meshInputGet(primitive, "POSITION", set);
                nor = ak_meshInputGet(primitive, "NORMAL", set);

                glCreateBuffers(1, &primitiveDataBuffer);
                glNamedBufferData(primitiveDataBuffer, pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
            };
        }
    }
}

void Light::drawLight(
    int width,
    int height,
    glm::mat4 Proj,
    AkCamera* camera) 
{
    mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
    GLuint vcol_location = glGetAttribLocation(vertexProgram, "vCol");
    GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");

    if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
    if (vcol_location != -1) format_attribute(vcol_location, nor->accessor);

    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (vcol_location != -1) glEnableVertexAttribArray(vcol_location);

    float r = 0.1 * panelConfig.dist;
    float phi = panelConfig.phi;
    float theta = panelConfig.theta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float correctedTheta = glm::fmod(glm::abs(theta), 6.28f);
    if (correctedTheta > 3.14 / 2. && correctedTheta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    glm::vec3 rotate = glm::vec3(3.14 * panelConfig.rot_x / 180, 3.14 * panelConfig.rot_y / 180, 0.f);

    glBindProgramPipeline(0);
    glBindProgramPipeline(pipeline);

    glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.near_plane, panelConfig.far_plane);
    else Projection = Proj;

    glm::mat4 View = 
        glm::rotate(
            glm::rotate(
                glm::translate(
                    localTransform
                , translate)
            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
    glm::mat4 MVP = Projection * LookAt * View * Model;
    glProgramUniformMatrix4fv(vertexProgram, mvpBindingLocation, 1, GL_FALSE, glm::value_ptr(MVP));

    int binding_point = 0;
    glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer, pos->accessor->byteOffset, pos->accessor->componentBytes);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


/* ================================================ */


void Cloud::createPipeline(int width, int height) 
{
    this->width = width;
    this->height = height;

    std::string vertexShaderPath = "res/shaders/standard_vec.glsl";
    std::string fragmentShaderPath = "res/shaders/depth_frag.glsl";

    char* vertexShader = read_file(vertexShaderPath.c_str());
    if (!vertexShader)  std::cout << "=================== Coulnt find " << vertexShaderPath << " ==============================\n";

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader)  std::cout << "=================== Coulnt find " << fragmentShaderPath << " ============================\n";


    vertexProgram = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShader);
    fragmentProgram = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShader);
    free(vertexShader);
    free(fragmentShader);
    GLint status = 1;

    if (status) {
        GLint linkageStatus;

        glGetProgramiv(vertexProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(vertexProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }

        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            fwrite(info, 1024, 1, stdout);
        }
    }


    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vertexProgram);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fragmentProgram);

    glGenBuffers(1, &depthBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, depthBuffer);
    glNamedBufferData(depthBuffer, width * height * 16, NULL, GL_DYNAMIC_COPY);
}

void Cloud::deletePipeline() 
{
    glDeleteProgram(vertexProgram);
    glDeleteProgram(fragmentProgram);
    glDeleteBuffers(2, primitiveDataBuffer);
    glDeleteBuffers(1, &depthBuffer);
    free(primitiveDataBuffer);
    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
}

void Cloud::loadMesh() 
{
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
    worldTransform = glm::make_mat4x4(t1);
    localTransform = glm::make_mat4x4(t2);
    free(t1);
    free(t2);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node);
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                AkMeshPrimitive* primitive = mesh->primitive;

                if (primitive->indices) {
                    ind = (uint32_t*)primitive->indices->items;
                    ind_size = primitive->indices->count;
                }

                int set = primitive->input->set;
                pos = ak_meshInputGet(primitive, "POSITION", set);
                tex = ak_meshInputGet(primitive, "TEXCOORD", set);

                primitiveDataBuffer = (GLuint*)calloc(2, sizeof(GLuint));
                glCreateBuffers(2, primitiveDataBuffer);
                glNamedBufferData(primitiveDataBuffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                glNamedBufferData(primitiveDataBuffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
            };
        }
    }
}

void Cloud::draw(
    int width,
    int height,
    glm::mat4 Proj,
    AkCamera* camera,
    float& g,
    GLuint lightbuffer) 
{
    mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
    prjBindingLocation = glGetUniformLocation(vertexProgram, "PRJ");
    GLuint gBindingLocation = glGetUniformLocation(fragmentProgram, "G");
    GLuint cameraBindingLocation = glGetUniformLocation(fragmentProgram, "camera");

    GLuint vtex_location = glGetAttribLocation(vertexProgram, "vTex");
    GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");

    if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
    if (vtex_location != -1) format_attribute(vtex_location, tex->accessor);

    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (prjBindingLocation != -1) glEnableVertexAttribArray(prjBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (vtex_location != -1) glEnableVertexAttribArray(vtex_location);

    float r = 0.1 * panelConfig.dist;
    float phi = panelConfig.phi;
    float theta = panelConfig.theta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panelConfig.rot_x / 180, 3.14 * panelConfig.rot_y / 180, 0.f);

    int binding_point = 0;
    glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

    glBindProgramPipeline(pipeline);

    glProgramUniform1f(fragmentProgram, gBindingLocation, g);
    glProgramUniform3fv(fragmentProgram, cameraBindingLocation, 1, glm::value_ptr(eye));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightbuffer);

    glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    if (!camera) {
        Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.near_plane, panelConfig.far_plane);
    }
    else {
        Projection = Proj;
    }

    glm::mat4 View = 
        glm::rotate(
            glm::rotate(
                glm::translate(
                    localTransform
                , translate)
            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
    MVP = LookAt * View * Model;
    glProgramUniformMatrix4fv(vertexProgram, mvpBindingLocation, 1, GL_FALSE, glm::value_ptr(MVP));
    glProgramUniformMatrix4fv(vertexProgram, prjBindingLocation, 1, GL_FALSE, glm::value_ptr(Projection));

    for (unsigned int texturesType = AMBIENT; texturesType < TT_SIZE; texturesType++) {
        glActiveTexture(GL_TEXTURE0 + texturesType);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    binding_point = 0;
    glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer[binding_point], pos->accessor->byteOffset, pos->accessor->componentBytes);

    binding_point = 1;
    glVertexAttribBinding(vtex_location, binding_point);
    glBindVertexBuffer(binding_point, primitiveDataBuffer[binding_point], tex->accessor->byteOffset, tex->accessor->componentBytes);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, ind_size, GL_UNSIGNED_INT, ind);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}
