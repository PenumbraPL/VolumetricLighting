#include "GL/glew.h"
#include "pch.h"
#include "Models.h"
#include "Tools.h"
#include <spdlog/spdlog.h>

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
                SPDLOG_LOGGER_ERROR(&logger, "=================== Coulnt find " + shaderPath[i] + " ==============================");
            }
            programs[i] = glCreateShaderProgramv(ds[i], 1, &shader[i]);
            free(shader[i]);

            glGetProgramiv(programs[i], GL_LINK_STATUS, &linkageStatus);
            if (!linkageStatus) {
                GLchar info[1024];
                glGetProgramInfoLog(programs[i], 1024, NULL, info);
                SPDLOG_LOGGER_ERROR(&logger, info);
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
    }

    glDeleteProgramPipelines(1, &pipeline);
    glDeleteVertexArrays(1, &vao);
}

    
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
/*
cloud
    glGenBuffers(1, &depthBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, depthBuffer);
    glNamedBufferData(depthBuffer, width * height * 16, NULL, GL_DYNAMIC_COPY);

*/
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


// unused
void Drawable::allocUnique()
{
    for (int i = 0; i < 7; i++) {
        if (accessor[i]) {
            glCreateBuffers(1, &primitiveDataBuffer[i]);
            glNamedBufferData(primitiveDataBuffer[i], accessor[i]->buffer->length, accessor[i]->buffer->data, GL_STATIC_DRAW);
        }
    }
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
            SPDLOG_LOGGER_ERROR(&logger, "Document couldn't be loaded");
            exit(EXIT_FAILURE);
        }
        else {
            logger.info(print_coord_system(doc->coordSys));
            logger.info(print_doc_information(doc->inf, doc->unit));
            logger.info("==============================================================================");
        }

        AkVisualScene* scene;
        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        if (!doc->scene.visualScene) {
            SPDLOG_LOGGER_ERROR(&logger, "================================== Scene couldnt be loaded! ===============");
            exit(EXIT_FAILURE);
        }
        else {
            std::string sceneInfo = "======================== Scene name: ";
            sceneInfo += scene->name;
            sceneInfo += "========================";
            SPDLOG_LOGGER_INFO(&logger, sceneInfo);
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


/* ====================================== */
    //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");

void Light::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/";
    scene_path += "lamp.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        SPDLOG_LOGGER_ERROR(&logger, "Light mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        SPDLOG_LOGGER_ERROR(&logger, "Light mesh couldn't be loaded\n");
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
            };
        }
    }
    allocAll(doc);
    docDataBuffer = parseBuffors();
}


void Light::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* docDataBuffer,
    glm::vec3& eye,
    glm::mat4& MVP,
    glm::mat4& Projection)
{
    bindVertexArray();
    GLuint vcolLocation = glGetAttribLocation(programs[VERTEX], "vCol");
    if (vcolLocation != 0xFFFFFFFF) {
        glEnableVertexArrayAttrib(vao, vcolLocation);
    }
    if (normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, normalsBindingLocation);
    if (textureBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, textureBindingLocation);
    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);

    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MVP));
 
    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();


    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void Environment::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/environment/";
    scene_path += "env_sphere.gltf";

    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        SPDLOG_LOGGER_ERROR(&logger, "Environment mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        SPDLOG_LOGGER_ERROR(&logger, "Environment mesh couldn't be loaded\n");
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
/*
    glDeleteSamplers(1, &env_sampler);
    glDeleteTextures(1, &skybox);
*/

void Environment::draw(
    GLuint& lights_buffer,
    std::map <void*, unsigned int>& bufferViews,
    GLuint* docDataBuffer,
    glm::vec3& eye,
    glm::mat4& MVP,
    glm::mat4& Projection)
{
    bindVertexArray();
    if(normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(vao, normalsBindingLocation);
    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);


    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
    glm::mat4 envMVP = MVP * Model * localTransform;
    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(envMVP));

    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();

    glBindSampler(0, env_sampler);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skybox);

    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
}



void Cloud::loadMesh()
{
    AkDoc* doc;
    AkVisualScene* scene;

    std::string scene_path = "res/models/cube/";
    scene_path += "Cube.gltf";
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        SPDLOG_LOGGER_ERROR(&logger, "Cloud mesh couldn't be loaded\n");
        return;
    }
    if (!doc->scene.visualScene) {
        SPDLOG_LOGGER_ERROR(&logger, "Cloud mesh couldn't be loaded\n");
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


void Cloud::draw(
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

    glProgramUniform1f(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][0], g);
    glProgramUniform3fv(programs[FRAGMENT], bindingLocationIndecies[FRAGMENT][1], 1, glm::value_ptr(eye));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_buffer);

    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MVP));
    glProgramUniformMatrix4fv(programs[VERTEX], bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(Projection));

    bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    bindTextures();

     glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}
