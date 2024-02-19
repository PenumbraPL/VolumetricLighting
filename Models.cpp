#include "GL/glew.h"
#include "pch.h"
#include "Models.h"
#include "Tools.h"

extern spdlog::logger logger;


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
        //std::cout << "=================== Coulnt find " << vertexShaderPath << " ==============================\n";
        logger.error("=================== Coulnt find " + vertexShaderPath + " ==============================\n");
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        //std::cout << "=================== Coulnt find " << fragmentShaderPath << " ============================\n";
        logger.error("=================== Coulnt find " + fragmentShaderPath + " ============================\n");
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

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
        }
        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
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

    std::string scene_path = "res/models/environment/";
    scene_path += "env_sphere.gltf";

    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Environment mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Environment mesh couldn't be loaded\n");
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
                    verticleIndecies = (uint32_t*)primitive->indices->items;
                    verticleIndeciesSize = (unsigned int) primitive->indices->count;
                }

                int set = primitive->input->set;
                pos = ak_meshInputGet(primitive, "POSITION", set);
                tex = ak_meshInputGet(primitive, "TEXCOORD", set);

                primitiveDataBuffer = (GLuint*)calloc(2, sizeof(GLuint));
                
                if (primitiveDataBuffer) {
                    glCreateBuffers(2, primitiveDataBuffer);
                    glNamedBufferData(primitiveDataBuffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
                    glNamedBufferData(primitiveDataBuffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
                }
            }
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

    float r = 0.1f * panelConfig.viewDistance;
    float phi = panelConfig.viewPhi;
    float theta = panelConfig.viewTheta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panelConfig.xRotate / 180, 3.14 * panelConfig.yRotate / 180, 0.f);

    glBindProgramPipeline(pipeline);

    glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.zNear, panelConfig.zFar);
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

    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
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
    prjBindingLocation = glGetUniformLocation(vertexProgram, "PRJ");
    cameraBindingLocation = glGetUniformLocation(fragmentProgram, "camera");
    gBindingLocation = glGetUniformLocation(fragmentProgram, "G");
    camDirBindingLocation = glGetUniformLocation(fragmentProgram, "direction");
    metalicBindingLocation = glGetUniformLocation(fragmentProgram, "_metalic");
    roughnessBindingLocation = glGetUniformLocation(fragmentProgram, "_roughness");
    albedoBindingLocation = glGetUniformLocation(fragmentProgram, "_albedo_color");
    aoBindingLocation = glGetUniformLocation(fragmentProgram, "ao_color");

    isTexBindingLocation = glGetUniformLocation(fragmentProgram, "_is_tex_bound");


    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (normalsBindingLocation != -1) glEnableVertexAttribArray(normalsBindingLocation);
    if (textureBindingLocation != -1) glEnableVertexAttribArray(textureBindingLocation);
    if (isTexBindingLocation != -1) glEnableVertexAttribArray(isTexBindingLocation);
    if (prjBindingLocation != -1) glEnableVertexAttribArray(prjBindingLocation);
    if (cameraBindingLocation != -1) glEnableVertexAttribArray(cameraBindingLocation);

    if (metalicBindingLocation != -1) glEnableVertexAttribArray(metalicBindingLocation);
    if (roughnessBindingLocation != -1) glEnableVertexAttribArray(roughnessBindingLocation);
    if (albedoBindingLocation != -1) glEnableVertexAttribArray(albedoBindingLocation);
    if (aoBindingLocation != -1) glEnableVertexAttribArray(aoBindingLocation);

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

    {
        glm::ivec4 isTex;
        isTex.r = textures[MET_ROUGH] ? 1 : 0;
        isTex.g = textures[MET_ROUGH] ? 1 : 0;
        isTex.b = textures[ALBEDO] ? 1 : 0;
        isTex.a = 0;
        glProgramUniform4iv(programs[FRAGMENT], isTexBindingLocation, 1, glm::value_ptr(isTex));

        glProgramUniform1f(programs[FRAGMENT], roughnessBindingLocation, colors[MET_ROUGH].r);
        glProgramUniform1f(programs[FRAGMENT], metalicBindingLocation, colors[MET_ROUGH].g);
       // glProgramUniform1f(programs[FRAGMENT], aoBindingLocation, colors[AO].x);
        glProgramUniform4fv(programs[FRAGMENT], albedoBindingLocation, 1, glm::value_ptr(colors[ALBEDO]));
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

    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
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
        logger.error("=================== Coulnt find " + vertexShaderPath + " ==============================\n");
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        logger.error("=================== Coulnt find " + fragmentShaderPath + " ============================\n");
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

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
        }
        glGetProgramiv(programs[FRAGMENT], GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(programs[FRAGMENT], 1024, NULL, info);

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
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
    if(textures) memset(textures, 0, sizeof(GLuint) * 8);
    texturesType = (GLuint*)calloc(8, sizeof(GLuint));
    if(texturesType) memset(texturesType, 0, sizeof(GLuint) * 8);
    return textures;
}

GLuint* Primitive::createSamplers() 
{
    samplers = (GLuint*)calloc(8, sizeof(GLuint));
    if(samplers) memset(samplers, 0, sizeof(GLuint) * 8);
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
        logger.error("=================== Coulnt find " + vertexShaderPath + " ==============================\n");
    }

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) {
        logger.error("=================== Coulnt find " + fragmentShaderPath + " ============================\n");
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

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
        }
        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
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

    std::string scene_path = "res/models/";
    scene_path += "lamp.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Light mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Light mesh couldn't be loaded\n");
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
                    verticleIndecies = (uint32_t*)primitive->indices->items;
                    verticleIndeciesSize = (unsigned int) primitive->indices->count;
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
    AkCamera* camera,
    glm::mat4x4& transform) 
{
    mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
    GLuint vcol_location = glGetAttribLocation(vertexProgram, "vCol");
    GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");

    if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
    if (vcol_location != -1) format_attribute(vcol_location, nor->accessor);

    if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
    if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
    if (vcol_location != -1) glEnableVertexAttribArray(vcol_location);

    float r = 0.1f * panelConfig.viewDistance;
    float phi = panelConfig.viewPhi;
    float theta = panelConfig.viewTheta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float correctedTheta = glm::fmod(glm::abs(theta), 6.28f);
    if (correctedTheta > 3.14 / 2. && correctedTheta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(panelConfig.xTranslate * 0.1, panelConfig.yTranslate * 0.1, panelConfig.zTranslate * 0.1);
    glm::vec3 rotate = glm::vec3(3.14 * panelConfig.xRotate / 180, 3.14 * panelConfig.yRotate / 180, 0.f);

    glBindProgramPipeline(0);
    glBindProgramPipeline(pipeline);

    glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.zNear, panelConfig.zFar);
    else Projection = Proj;


    glm::mat4 View = 
        glm::rotate(
            glm::rotate(
                glm::translate(
                    transform//localTransform
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
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


/* ================================================= */



    void Drawable::createPipeline(std::string shaderPath[5])
    {
        char* shader[5];
        glGenProgramPipelines(1, &pipeline);
        GLint linkageStatus;

        glGenVertexArrays(1, &vao);

        for (int i = VERTEX; i <= GEOMETRY; i++) {
            if (!shaderPath[i].empty()) {
                shader[i] = read_file(shaderPath[i].c_str());
                if (!shader[i]) {
                    logger.error("=================== Coulnt find " + shaderPath[i] + " ==============================\n");
                }
                programs[i] = glCreateShaderProgramv(ds[i], 1, &shader[i]);
                free(shader[i]);

                glGetProgramiv(programs[i], GL_LINK_STATUS, &linkageStatus);
                if (!linkageStatus) {
                    GLchar info[1024];
                    glGetProgramInfoLog(programs[i], 1024, NULL, info);
                    logger.error(info);
                }
                glUseProgramStages(pipeline, dsb[i], programs[i]);
            }
        }
    }

    void Drawable::deletePipeline()
    {
        glBindProgramPipeline(0);

        for (int i = VERTEX; i <= GEOMETRY; i++) {
            if (programs[i] != 0xFFFFFFFF) glDeleteProgram(programs[i]);
        } // program[i] == 0 ?

        glDeleteProgramPipelines(1, &pipeline);
        glDeleteVertexArrays(1, &vao);
    } // delete(primtitiveDataBuffer)?

    
    void Drawable::loadMatrix(AkNode* node)
    {
        float* t1 = (float*)calloc(16, sizeof(float));
        float* t2 = (float*)calloc(16, sizeof(float));
        ak_transformCombineWorld(node, t1);
        ak_transformCombine(node, t2);
        worldTransform = glm::make_mat4x4(t1);
        localTransform = glm::make_mat4x4(t2);
        free(t1);
        free(t2);
    }

    // TO DO: everything setup here should be placed in one class
    void Drawable::processMesh(AkMeshPrimitive* primitive)
    {
        if (primitive->indices) {
            verticleIndecies = (uint32_t*)primitive->indices->items;
            verticleIndeciesSize = (unsigned int)primitive->indices->count;
        }

        int set = primitive->input->set;
        AkInput* wgs = ak_meshInputGet(primitive, "WEIGHTS", set);
        AkInput* jts = ak_meshInputGet(primitive, "JOINTS", set);
        AkInput* pos = ak_meshInputGet(primitive, "POSITION", set);
        AkInput* tex = ak_meshInputGet(primitive, "TEXCOORD", set); // if indexed then multiple parts to proccess
        AkInput* nor = ak_meshInputGet(primitive, "NORMAL", set);

        AkInput* col = ak_meshInputGet(primitive, "COLOR", set);
        AkInput* tan = ak_meshInputGet(primitive, "TANGENT", set);

        //std::cout << ak_meshInputCount(mesh) << std::endl;

        accessor[POSITION] = pos ? pos->accessor : nullptr;
        accessor[TEXTURES] = tex ? tex->accessor : nullptr;
        accessor[NORMALS] = nor ? nor->accessor : nullptr;
        accessor[WEIGTHS] = wgs ? wgs->accessor : nullptr;
        accessor[JOINTS] = jts ? jts->accessor : nullptr;
        accessor[COLORS] = col ? col->accessor : nullptr;
        accessor[TANGENTS] = tan ? tan->accessor : nullptr;

        if (primitive->material) {
            AkMaterial* mat = primitive->material;
            AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
            AkTechniqueFxCommon* tch = ef->profile->technique->common;
            if (tch) {
                set_up_color(tch->ambient, primitive, *this, AMBIENT, panelConfig);
                set_up_color(tch->emission, primitive, *this, EMISIVE, panelConfig);
                set_up_color(tch->diffuse, primitive, *this, DIFFUSE, panelConfig);
                set_up_color(tch->specular, primitive, *this, SPECULAR, panelConfig);

                switch (tch->type) {
                case AK_MATERIAL_METALLIC_ROUGHNESS: {
                    AkMetallicRoughness* mr = (AkMetallicRoughness*)tch;
                    AkColorDesc alb_cd;
                    AkColorDesc mr_cd;
                    AkColor col;
                    mr_cd.color = &col;

                    alb_cd.color = &mr->albedo;
                    alb_cd.texture = mr->albedoTex;
                    mr_cd.color->rgba.R = mr->metallic;
                    mr_cd.color->rgba.G = mr->roughness;
                    mr_cd.texture = mr->metalRoughTex;
                    set_up_color(&alb_cd, primitive, *this, ALBEDO, panelConfig);
                    set_up_color(&mr_cd, primitive, *this, MET_ROUGH, panelConfig);
                    break;
                }

                case AK_MATERIAL_SPECULAR_GLOSSINES: {
                    AkSpecularGlossiness* sg = (AkSpecularGlossiness*)tch;
                    AkColorDesc sg_cd;
                    AkColorDesc dif_cd;
                    sg_cd.color = &sg->specular;
                    sg_cd.texture = sg->specGlossTex;
                    dif_cd.color = &sg->diffuse;
                    dif_cd.texture = sg->diffuseTex;
                    set_up_color(&sg_cd, primitive, *this, SP_GLOSSINESS, panelConfig);
                    set_up_color(&dif_cd, primitive, *this, SP_DIFFUSE, panelConfig);
                    break;
                }
                };
            }
        }
    }



    void Drawable::allocUnique()
    {
        //unsigned int bs = std::size(accessor) - std::count(std::begin(accessor), std::end(accessor), nullptr);
        //check what is avalable and insert it into gpu buffer
        //glCreateBuffers(bs, primitiveDataBuffer);
        //primitiveDataBuffer = (GLuint*)calloc(bs, sizeof(GLuint));
        //// delete
        //if (primitiveDataBuffer) {
            // is nessesery ?
            
            for (int i = 0; i < 7; i++) {
                if (accessor[i]) {
                    glCreateBuffers(1, &primitiveDataBuffer[i]);
                    glNamedBufferData(primitiveDataBuffer[i], accessor[i]->buffer->length, accessor[i]->buffer->data, GL_STATIC_DRAW);
                }
            }
        //}
    }

    void Drawable::bindVertexArray()
    {
        glBindVertexArray(vao);
        if (vertexPosBindingLocation != 0xFFFFFFFF) {
            format_attribute(vertexPosBindingLocation, accessor[POSITION]);
            glEnableVertexArrayAttrib(vao, vertexPosBindingLocation);
        }
        if (normalsBindingLocation != 0xFFFFFFFF) {
            format_attribute(normalsBindingLocation, accessor[NORMALS]);
            glEnableVertexArrayAttrib(vao, normalsBindingLocation);
        }
        if (textureBindingLocation != 0xFFFFFFFF) {
            format_attribute(textureBindingLocation, accessor[TEXTURES]);
            glEnableVertexArrayAttrib(vao, textureBindingLocation);
        }
    }

    void Drawable::draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection)
    {
        bindVertexArray();

        glBindVertexArray(vao);
        glBindProgramPipeline(pipeline);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);

        glm::vec3 camera_view = eye;
        glm::vec3 camera_dir = glm::vec3(0.) - eye;

        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        glm::mat4 MVPPos = MVP * Model * localTransform; // check is it correct?


        glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MVPPos));
        glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(Projection));
        glProgramUniform3fv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][0], 1, glm::value_ptr(camera_view));
        //glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][1], panelConfig.g);
        //glProgramUniform3fv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][1], 1, glm::value_ptr(camera_dir));

        {
            glm::ivec4 isTex;
            isTex.r = textures[MET_ROUGH] ? 1 : 0;
            isTex.g = textures[MET_ROUGH] ? 1 : 0;
            isTex.b = textures[ALBEDO] ? 1 : 0;
            isTex.a = 0;

            glProgramUniform4iv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][5], 1, glm::value_ptr(isTex));
            glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][2], colors[MET_ROUGH].r);
            glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][1], colors[MET_ROUGH].g);
            //glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][4], colors[AO].x);
            glProgramUniform4fv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][3], 1, glm::value_ptr(colors[ALBEDO]));
        }

        bindVertexBuffer(bufferViews, docDataBuffer);
        bindTextures();

        glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    }



    void Drawable::getLocation(std::vector<const char*> uniformNames[5])
    {
        for (int i = VERTEX; i <= GEOMETRY; i++) {
            if (uniformNames[i].size()) {
                bindingLocationIndecies[i] = (GLuint*)calloc(uniformNames[i].size(), sizeof(GLuint));
                for (int j = 0; j < uniformNames[i].size(); j++) {
                    bindingLocationIndecies[i][j] = glGetUniformLocation(programs[i], uniformNames[i].data()[j]);
                }
                //glGetUniformIndices(programs[i], uniformNames[i].size(), uniformNames[i].data(), bindingLocationIndecies[i]);
            }
        }

        //for (int i = VERTEX; i <= GEOMETRY; i++) {
        //    for (int j = 0; j < uniformNames[i].size(); j++) {
        //        if (bindingLocationIndecies[i][j] != 0xFFFFFFFF) {
        //            glEnableVertexAttribArray(bindingLocationIndecies[i][j]);
        //        }
        //    }
        //}

        vertexPosBindingLocation = glGetAttribLocation(programs[VERTEX], "vPos");
        normalsBindingLocation = glGetAttribLocation(programs[VERTEX], "vNor");
        textureBindingLocation = glGetAttribLocation(programs[VERTEX], "vTex");

        //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
    }



    void Drawable::allocAll(AkDoc* doc)
    {
        bufferViews.clear();
        textureViews.clear();
        imageViews.clear();

        // What with and libimages ??
        int j = 0;
        FListItem* i = doc->lib.images;
        if (i) {
            do {
                AkImage* img = (AkImage*)i->data;
                imageViews.insert({ {img, 0} });
                i = i->next;
            } while (i);
            for (auto& u : imageViews) {
                u.second = j++;
            }
        }

        j = 0;
        FListItem* t = doc->lib.textures;
        if (t) {
            do {
                AkTexture* tex = (AkTexture*)t->data;
                textureViews.insert({ {tex, 0} });
                t = t->next;
            } while (t);
            for (auto& u : textureViews) {
                u.second = j++;
            }
        }

        j = 0;
        FListItem* b = (FListItem*)doc->lib.buffers;
        if (b) {
            do {
                AkBuffer* buf = (AkBuffer*)b->data;
                bufferViews.insert({ {buf, 0} });
                b = b->next;
            } while (b);
            for (auto& u : bufferViews) {
                u.second = j++;
            }
        }
    }

    GLuint* Drawable::parseBuffors()
    {
        //  glGenVertexArrays(1, &vao);
         // glBindVertexArray(vao);

        GLuint* docDataBuffer = (GLuint*)calloc(bufferViews.size(), sizeof(GLuint));
        glCreateBuffers((GLsizei)bufferViews.size(), docDataBuffer);
        for (auto& buffer : bufferViews) {
            unsigned int i = bufferViews[buffer.first];
            glNamedBufferData(docDataBuffer[i], ((AkBuffer*)buffer.first)->length, ((AkBuffer*)buffer.first)->data, GL_STATIC_DRAW);
        }
        return docDataBuffer;
    }




    void Drawable::bindVertexBuffer(std::map <void*, unsigned int>& bufferViews, GLuint* docDataBuffer)
    {
        int j;
        int binding_point;

        if (vertexPosBindingLocation != 0xFFFFFFFF) {
            j = bufferViews[accessor[POSITION]->buffer];
            binding_point = 0;
            glVertexAttribBinding(vertexPosBindingLocation, binding_point);
            glBindVertexBuffer(binding_point, docDataBuffer[j], accessor[POSITION]->byteOffset, accessor[POSITION]->componentBytes);
        }
        if (normalsBindingLocation != 0xFFFFFFFF) {
            j = bufferViews[accessor[NORMALS]->buffer];
            binding_point = 1;
            glVertexAttribBinding(normalsBindingLocation, binding_point);
            glBindVertexBuffer(binding_point, docDataBuffer[j], accessor[NORMALS]->byteOffset, accessor[NORMALS]->componentBytes);
        }

        if (textureBindingLocation != 0xFFFFFFFF) {
            j = bufferViews[accessor[TEXTURES]->buffer];
            binding_point = 2;
            glVertexAttribBinding(textureBindingLocation, binding_point);
            glBindVertexBuffer(binding_point, docDataBuffer[j], accessor[TEXTURES]->byteOffset, accessor[TEXTURES]->componentBytes);
        }
    }


    void Drawable::bindTextures()
    {
        for (unsigned int type = AMBIENT; type < TT_SIZE; type++) {
            GLuint sampler = samplers[(TextureType)type];
            GLuint texture = textures[(TextureType)type];
            if (sampler && texture) {
                glBindSampler(type, sampler);
                glActiveTexture(GL_TEXTURE0 + type);
                glBindTexture(texturesType[type], texture);
            }
            else {
                glActiveTexture(GL_TEXTURE0 + type);
                glBindTexture(GL_TEXTURE_2D, 0); // needs change
            }
        }
    }


    void Drawable::deleteTexturesAndSamplers()
    {
        for (unsigned int type = AMBIENT; type < TT_SIZE; type++) {
            if (textures[type]) {
                glDeleteTextures(1, &textures[type]);
            }
            if (samplers[type]) {
                glDeleteSamplers(1, &samplers[type]);
            }
        }
    }


/* ================================================ */

    void Scene::updateLights(GLuint lightsBuffer, unsigned int lightDataSize, ConfigContext& panelConfig) {
        if (panelConfig.getLightsSize() != lightDataSize) {
            if (panelConfig.getLightsSize() > lightDataSize) {
                lightDataSize = panelConfig.getLightsSize();
                int lightsBufferSize = (int)sizeof(PointLight) * lights.size();
                glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
            }
            lightDataSize = panelConfig.getLightsSize();
            int lightsBufferSize = (int)sizeof(PointLight) * lights.size();
            glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lights.data());
            glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);
        }
        PointLight newLight = panelConfig.getLight();
        if (compare_lights(lights.data()[panelConfig.lightId], newLight)) {
            lights.data()[panelConfig.lightId] = newLight;
            LightsList* ptr = (LightsList*)glMapNamedBuffer(lightsBuffer, GL_WRITE_ONLY);
            memcpy_s((void*)&ptr->list[panelConfig.lightId], sizeof(PointLight), &newLight, sizeof(PointLight));
            glUnmapNamedBuffer(lightsBuffer);
            panelConfig.updateLight();
        }
    }

    void Scene::initLights() {
        lights.clear();
        lights.push_back({ glm::vec3(1.5f, 1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f) });
        lights.push_back({ glm::vec3(-1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
        lights.push_back({ glm::vec3(1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
    }


    bool Scene::compare_lights(PointLight& old_light, PointLight& new_light)
    {
        return memcmp(&old_light, &new_light, sizeof(PointLight));
    }

    bool Scene::compare_lights(LightsList& old_light, LightsList& new_light)
    {
        if (old_light.size != new_light.size) return true;

        return memcmp(&old_light.list, &new_light.list, old_light.size * sizeof(PointLight));
    }



    AkCamera* Scene::camera(AkDoc* doc) {
        AkVisualScene* scene;
        AkCamera* cam = nullptr;
        if (doc->scene.visualScene) {
            scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);

            float cameraView[16];
            float cameraProjection[16];
            ak_firstCamera(doc, &cam, cameraView, cameraProjection);
            if (cam) {
                cameraEye.View = glm::make_mat4x4(cameraView);
                cameraEye.Projection = glm::make_mat4x4(cameraProjection);
            }
            else if (scene->cameras) {
                if (scene->cameras->first) {
                    cam = (AkCamera*)ak_instanceObject(scene->cameras->first->instance);
                }
            }
            if (cam) std::cout << "Camera name: " << cam->name << std::endl; // log
        }
        return cam;
    }




    AkDoc* Scene::loadScene(std::string scenePath, std::string sceneName)
    {
        primitives.clear();

        scenePath += sceneName;
        AkDoc* doc;
        if (ak_load(&doc, scenePath.c_str(), NULL) != AK_OK) {
            logger.error("Document couldn't be loaded\n");
            exit(EXIT_FAILURE);
        }
        else {
            logger.info(print_coord_system(doc->coordSys));
            logger.info(print_doc_information(doc->inf, doc->unit));
            logger.info("==============================================================================\n");
        }

        AkVisualScene* scene;
        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        if (!doc->scene.visualScene) {
            logger.error("================================== Scene couldnt be loaded! ===============\n");
            exit(EXIT_FAILURE);
        }
        else {
            std::string sceneInfo = "======================== Scene name: ";
            sceneInfo += scene->name;
            sceneInfo += "========================\n";
            logger.info(sceneInfo);
        }

        AkNode* node = ak_instanceObjectNode(scene->node);
        proccess_node(node, primitives);

        return doc;
    }
    
    Scene::~Scene()
    {
        for (auto& primitive : primitives) {
            //primitive.deletePrograms();
            //primitive.deletePipeline();
            //primitive.deleteTexturesAndSamplers();
        }
    }

    void Scene::allocAll(AkDoc* doc)
    {
        bufferViews.clear();
        textureViews.clear();
        imageViews.clear();

        // What with and libimages ??
        int j = 0;
        FListItem* i = doc->lib.images;
        if (i) {
            do {
                AkImage* img = (AkImage*)i->data;
                imageViews.insert({ {img, 0} });
                i = i->next;
            } while (i);
            for (auto& u : imageViews) {
                u.second = j++;
            }
        }

        j = 0;
        FListItem* t = doc->lib.textures;
        if (t) {
            do {
                AkTexture* tex = (AkTexture*)t->data;
                textureViews.insert({ {tex, 0} });
                t = t->next;
            } while (t);
            for (auto& u : textureViews) {
                u.second = j++;
            }
        }

        j = 0;
        FListItem* b = (FListItem*)doc->lib.buffers;
        if (b) {
            do {
                AkBuffer* buf = (AkBuffer*)b->data;
                bufferViews.insert({ {buf, 0} });
                b = b->next;
            } while (b);
            for (auto& u : bufferViews) {
                u.second = j++;
            }
        }
    }
    GLuint* Scene::parseBuffors()
    {
        //  glGenVertexArrays(1, &vao);
         // glBindVertexArray(vao);

        GLuint* docDataBuffer = (GLuint*)calloc(bufferViews.size(), sizeof(GLuint));
        glCreateBuffers((GLsizei)bufferViews.size(), docDataBuffer);
        for (auto& buffer : bufferViews) {
            unsigned int i = bufferViews[buffer.first];
            glNamedBufferData(docDataBuffer[i], ((AkBuffer*)buffer.first)->length, ((AkBuffer*)buffer.first)->data, GL_STATIC_DRAW);
        }
        return docDataBuffer;
    }


/* ================================================ */


void Cloud::createPipeline(int width, int height) 
{
    this->width = width;
    this->height = height;

    std::string vertexShaderPath = "res/shaders/standard_vec.glsl";
    std::string fragmentShaderPath = "res/shaders/depth_frag.glsl";

    char* vertexShader = read_file(vertexShaderPath.c_str());
    if (!vertexShader)  logger.error("=================== Coulnt find " + vertexShaderPath + " ==============================\n");

    char* fragmentShader = read_file(fragmentShaderPath.c_str());
    if (!fragmentShader) logger.error("=================== Coulnt find " + fragmentShaderPath + " ============================\n");


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

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
        }

        glGetProgramiv(fragmentProgram, GL_LINK_STATUS, &linkageStatus);
        if (!linkageStatus) {
            GLchar info[1024];
            glGetProgramInfoLog(fragmentProgram, 1024, NULL, info);

            //fwrite(info, 1024, 1, stdout);
            logger.error(info);
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

    std::string scene_path = "res/models/cube/";
    scene_path += "Cube.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Cloud mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Cloud mesh couldn't be loaded\n");
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
                    verticleIndecies = (uint32_t*)primitive->indices->items;
                    verticleIndeciesSize = (unsigned int) primitive->indices->count;
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

    float r = 0.1f * panelConfig.viewDistance;
    float phi = panelConfig.viewPhi;
    float theta = panelConfig.viewTheta;

    glm::mat4x4 Projection;

    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    eye = glm::vec3(eye.z, eye.y, eye.x);

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }

    glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panelConfig.xRotate / 180, 3.14 * panelConfig.yRotate / 180, 0.f);

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
        Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.zNear, panelConfig.zFar);
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
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}





/* ====================================== */

void L::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/";
    scene_path += "lamp.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Light mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Light mesh couldn't be loaded\n");
        return;
    }

    scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
    AkNode* node = ak_instanceObjectNode(scene->node);
    loadMatrix(node);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node);
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                processMesh(mesh->primitive);
                //allocUnique();
            };
        }
    }
    allocAll(doc);
    docDataBuffer = parseBuffors();
}
//
//void L::getLocation(std::vector<const char*> uniformNames[5])
//{
//
//}
//mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
//GLuint vcol_location = glGetAttribLocation(vertexProgram, "vCol");
//GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");

//if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
//if (vcol_location != -1) format_attribute(vcol_location, nor->accessor);

//if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
//if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
//if (vcol_location != -1) glEnableVertexAttribArray(vcol_location);


void L::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* docDataBuffer,
    glm::vec3& eye,
    glm::mat4& MVP,
    glm::mat4& Projection)
{
    //float r = 0.1f * panelConfig.viewDistance;
    //float phi = panelConfig.viewPhi;
    //float theta = panelConfig.viewTheta;

    //glm::mat4x4 Projection;

    //glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    //eye = glm::vec3(eye.z, eye.y, eye.x);

    //glm::vec3 north = glm::vec3(0., 1., 0.);
    //float correctedTheta = glm::fmod(glm::abs(theta), 6.28f);
    //if (correctedTheta > 3.14 / 2. && correctedTheta < 3.14 * 3. / 2.) {
    //    north = glm::vec3(0., -1., 0.);
    //}

    //glm::vec3 translate = glm::vec3(0.);// glm::vec3(panelConfig.xTranslate * 0.1, panelConfig.yTranslate * 0.1, panelConfig.zTranslate * 0.1);
    //glm::vec3 rotate = glm::vec3(0.);// glm::vec3(3.14 * panelConfig.xRotate / 180, 3.14 * panelConfig.yRotate / 180, 0.f);



    //glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    //if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.zNear, panelConfig.zFar);
    //else Projection = Proj;


    //glm::mat4 View =
    //    glm::rotate(
    //        glm::rotate(
    //            glm::translate(
    //                localTransform
    //                , translate)
    //            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
    //        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
    //glm::mat4 Mat = Projection * MVP * View * Model;

    bindVertexArray();
    GLuint vcolLocation = glGetAttribLocation(programs[VERTEX], "vCol");
    if (vcolLocation != 0xFFFFFFFF) {
        //format_attribute(vcolLocation, accessor[COLORS]);
        glEnableVertexArrayAttrib(vao, vcolLocation);
    }
    if (normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, normalsBindingLocation);
    if (textureBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, textureBindingLocation);
    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);

    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
    //glm::mat4 lightMVP = MVP * Model * localTransform;
    //glm::mat4 lightMVP = MVP;
    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MVP));

    //int binding_point = 0;
    //glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    //glBindVertexBuffer(binding_point, primitiveDataBuffer[POSITION], accessor[POSITION]->byteOffset, accessor[POSITION]->componentBytes);
    
    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();


    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void E::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/environment/";
    scene_path += "env_sphere.gltf";

    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Environment mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Environment mesh couldn't be loaded\n");
        return;
    }

    scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
    AkNode* node = ak_instanceObjectNode(scene->node);
    loadMatrix(node);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node);
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                processMesh(mesh->primitive);
                allocUnique();
            }
        }
    }
    allocAll(doc);
    docDataBuffer = parseBuffors();

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


void E::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* docDataBuffer,
    glm::vec3& eye,
    glm::mat4& MVP,
    glm::mat4& Projection)
{

    //glm::mat4 View = glm::rotate(
    //    glm::rotate(
    //        glm::translate(
    //            localTransform
    //            , translate)
    //        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
    //    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
    //glm::mat4 Mat = Projection * MVP * View * Model;

    bindVertexArray();
    if(normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, normalsBindingLocation);
    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);


    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
    glm::mat4 envMVP = MVP * Model * localTransform;
    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(envMVP));

    //int binding_point = 0;
    //glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    //glBindVertexBuffer(binding_point, primitiveDataBuffer[POSITION], accessor[POSITION]->byteOffset, accessor[POSITION]->componentBytes);

    //binding_point = 1;
    //glVertexAttribBinding(textureBindingLocation, binding_point);
    //glBindVertexBuffer(binding_point, primitiveDataBuffer[TEXTURES], accessor[TEXTURES]->byteOffset, accessor[TEXTURES]->componentBytes);
    
    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();

    glBindSampler(0, env_sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skybox);

    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
}


void C::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/cube/";
    scene_path += "Cube.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger.error("Cloud mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        logger.error("Cloud mesh couldn't be loaded\n");
        return;
    }

    scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
    AkNode* node = ak_instanceObjectNode(scene->node);
    loadMatrix(node);

    if (node->geometry) {
        AkGeometry* geometry = ak_instanceObjectGeom(node);
        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
        if ((AkGeometryType)geometry->gdata->type) {
            if (mesh) {
                processMesh(mesh->primitive);
                //allocUnique();
            };
        }
    }
    allocAll(doc);
    docDataBuffer = parseBuffors();
}

//
//mvpBindingLocation = glGetUniformLocation(vertexProgram, "MVP");
//prjBindingLocation = glGetUniformLocation(vertexProgram, "PRJ");
//GLuint gBindingLocation = glGetUniformLocation(fragmentProgram, "G");
//GLuint cameraBindingLocation = glGetUniformLocation(fragmentProgram, "camera");
//
//GLuint vtex_location = glGetAttribLocation(vertexProgram, "vTex");
//GLuint vertexPosBindingLocation = glGetAttribLocation(vertexProgram, "vPos");
//
//if (vertexPosBindingLocation != -1) format_attribute(vertexPosBindingLocation, pos->accessor);
//if (vtex_location != -1) format_attribute(vtex_location, tex->accessor);
//
//if (mvpBindingLocation != -1) glEnableVertexAttribArray(mvpBindingLocation);
//if (prjBindingLocation != -1) glEnableVertexAttribArray(prjBindingLocation);
//if (vertexPosBindingLocation != -1) glEnableVertexAttribArray(vertexPosBindingLocation);
//if (vtex_location != -1) glEnableVertexAttribArray(vtex_location);

void C::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* docDataBuffer,
    glm::vec3& eye,
    glm::mat4& MVP,
    glm::mat4& Projection)
{


    //float r = 0.1f * panelConfig.viewDistance;
    //float phi = panelConfig.viewPhi;
    //float theta = panelConfig.viewTheta;

    //glm::mat4x4 Projection;

    //glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));
    //eye = glm::vec3(eye.z, eye.y, eye.x);

    //glm::vec3 north = glm::vec3(0., 1., 0.);
    //float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    //if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
    //    north = glm::vec3(0., -1., 0.);
    //}

    //glm::vec3 translate = glm::vec3(0., 0., 0.);// glm::vec3(panelConfig.tr_x * 0.1, panelConfig.tr_y * 0.1, panelConfig.tr_z * 0.1);
    //glm::vec3 rotate = glm::vec3(0., 0., 0.);//glm::vec3(3.14 * panelConfig.xRotate / 180, 3.14 * panelConfig.yRotate / 180, 0.f);

        //glm::mat4 LookAt = glm::lookAt(eye, glm::vec3(0.), north);
    //if (!camera) {
    //    Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.zNear, panelConfig.zFar);
    //}
    //else {
    //    Projection = Proj;
    //}

    //glm::mat4 View =
    //    glm::rotate(
    //        glm::rotate(
    //            glm::translate(
    //                localTransform
    //                , translate)
    //            , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
    //        rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));
    //MVP = LookAt * View * Model;

    bindVertexArray();
    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);

    glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][0], g);
    glProgramUniform3fv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][1], 1, glm::value_ptr(eye));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);

    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MVP));
    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(Projection));

    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();

    //for (unsigned int texturesType = AMBIENT; texturesType < TT_SIZE; texturesType++) {
    //    glActiveTexture(GL_TEXTURE0 + texturesType);
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //}

    //int binding_point = 0;
    //glVertexAttribBinding(vertexPosBindingLocation, binding_point);
    //glBindVertexBuffer(binding_point, primitiveDataBuffer[POSITION], accessor[POSITION]->byteOffset, accessor[POSITION]->componentBytes);

    //binding_point = 1;
    //glVertexAttribBinding(textureBindingLocation, binding_point);
    //glBindVertexBuffer(binding_point, primitiveDataBuffer[TEXTURES], accessor[TEXTURES]->byteOffset, accessor[TEXTURES]->componentBytes);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}
