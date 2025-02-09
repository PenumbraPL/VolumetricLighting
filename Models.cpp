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




void ShadersPipeline::createPipeline(ShadersSources shaderPath)
{
    char* shader[5];
    glGenProgramPipelines(1, &pipeline);
    GLint linkageStatus;

    glGenVertexArrays(1, &vao);

    for (int i = VERTEX; i <= GEOMETRY; i++) {
        if (!shaderPath[i].empty()) {
            shader[i] = readFile(shaderPath[i].c_str());
            if (!shader[i]) {
                logger.error("=================== Coulnt find " + shaderPath[i] + " ==============================");
            }
            programs[i] = glCreateShaderProgramv(ds[i], 1, &shader[i]);
            free(shader[i]);

            glGetProgramiv(programs[i], GL_LINK_STATUS, &linkageStatus);
            if (!linkageStatus) {
                GLchar info[1024];
                glGetProgramInfoLog(programs[i], 1024, NULL, info);
                logger.info(info);
            }
            glUseProgramStages(pipeline, dsb[i], programs[i]);
        }
    }
}

void ShadersPipeline::deletePipeline()
{
    glBindProgramPipeline(0);

    for (int i = VERTEX; i <= GEOMETRY; i++) {
        if (programs[i] != 0xFFFFFFFF) glDeleteProgram(programs[i]);
    }

    glDeleteProgramPipelines(1, &pipeline);
    glDeleteVertexArrays(1, &vao);
}


void ShadersPipeline::bindVertexArray()
{
    glBindVertexArray(vao);
    if (vertexPosBindingLocation != 0xFFFFFFFF) {
        formatAttribute(vertexPosBindingLocation, accessor[POSITION]);
        glEnableVertexArrayAttrib(vao, vertexPosBindingLocation);
    }
    if (normalsBindingLocation != 0xFFFFFFFF) {
        formatAttribute(normalsBindingLocation, accessor[NORMALS]);
        glEnableVertexArrayAttrib(vao, normalsBindingLocation);
    }
    if (textureBindingLocation != 0xFFFFFFFF) {
        formatAttribute(textureBindingLocation, accessor[TEXTURES]);
        glEnableVertexArrayAttrib(vao, textureBindingLocation);
    }
}

void ShadersPipeline::getLocation(BindingPointCollection uniformNames)
{
    GLchar name[32]; // dump - currently it suspect no data to be written to it
    bindingNames = uniformNames;
    for (int i = VERTEX; i <= GEOMETRY; i++) {
        if (uniformNames[i].size()) {
            bindingLocationIndecies[i] = (GLuint*)calloc(uniformNames[i].size(), sizeof(GLuint));
            bindingTypes[i] = (GLenum*)calloc(uniformNames[i].size(), sizeof(GLenum));
            for (int j = 0; j < uniformNames[i].size(); j++) {
                bindingLocationIndecies[i][j] = glGetUniformLocation(programs[i], uniformNames[i].data()[j].c_str());
                if (bindingLocationIndecies[i][j] != 0xFFFFFFFF) {
                    GLint size;
                    glGetActiveUniform(programs[i], bindingLocationIndecies[i][j], 32, NULL, &size, &bindingTypes[i][j], name);
                }
            }
            //glGetUniformIndices(programs[i], uniformNames[i].size(), uniformNames[i].data(), bindingLocationIndecies[i]);
        }
    }

    vertexPosBindingLocation = glGetAttribLocation(programs[VERTEX], "vPos");
    normalsBindingLocation = glGetAttribLocation(programs[VERTEX], "vNor");
    textureBindingLocation = glGetAttribLocation(programs[VERTEX], "vTex");

    //glObjectLabel(GL_BUFFER, buffers[binding_point], -1, "Vertex Buffer");
}

void bindUniform(GLuint program, GLuint location, GLenum type, void* value)
{
    switch (type) {
    case GL_FLOAT:
        glProgramUniform1f(program, location, *((GLfloat*)value));
        break;
    case GL_INT:
        glProgramUniform1i(program, location, *((GLint*)value));
        break;
    case GL_UNSIGNED_INT:
        glProgramUniform1ui(program, location, *((GLuint*)value));
        break;
    case GL_FLOAT_VEC2:
        glProgramUniform2fv(program, location, 1, glm::value_ptr(*((glm::vec2*)value)));
        break;
    case GL_INT_VEC2:
        glProgramUniform2iv(program, location, 1, glm::value_ptr(*((glm::ivec2*)value)));
        break;
    case GL_UNSIGNED_INT_VEC2:
        glProgramUniform2uiv(program, location, 1, glm::value_ptr(*((glm::uvec2*)value)));
        break;
    case GL_FLOAT_VEC3:
        glProgramUniform3fv(program, location, 1, glm::value_ptr(*((glm::vec3*)value)));
        break;
    case GL_INT_VEC3:
        glProgramUniform3iv(program, location, 1, glm::value_ptr(*((glm::ivec3*)value)));
        break;
    case GL_UNSIGNED_INT_VEC3:
        glProgramUniform3uiv(program, location, 1, glm::value_ptr(*((glm::uvec3*)value)));
        break;
    case GL_FLOAT_VEC4:
        glProgramUniform4fv(program, location, 1, glm::value_ptr(*((glm::vec4*)value)));
        break;
    case GL_INT_VEC4:
        glProgramUniform4iv(program, location, 1, glm::value_ptr(*((glm::ivec4*)value)));
        break;
    case GL_UNSIGNED_INT_VEC4:
        glProgramUniform4uiv(program, location, 1, glm::value_ptr(*((glm::uvec4*)value)));
        break;
    case GL_FLOAT_MAT2:
        glProgramUniformMatrix2fv(program, location, 1, GL_FALSE, glm::value_ptr(*((glm::mat2*)value)));
        break;
    case GL_FLOAT_MAT3:
        glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, glm::value_ptr(*((glm::mat3*)value)));
        break;
    case GL_FLOAT_MAT4:
        glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, glm::value_ptr(*((glm::mat4*)value)));
        break;
    }
}

void ShadersPipeline::bindUniform(std::array<std::vector<void*>, 5> values)
{
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < values[i].size(); j++) {
            ::bindUniform(programs[i], bindingLocationIndecies[i][j], bindingTypes[i][j], values[i][j]);
        }
    }
}


void ShadersPipeline::processMesh(AkMeshPrimitive* primitive)
{
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
}


void ShadersPipeline::bindVertexBuffer(std::map <void*, unsigned int>& bufferViews, GLuint* docDataBuffer)
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


void Drawable::loadMatrix(AkNode* node)
{
    float t1[16], t2[16];
    ak_transformCombineWorld(node, t1);
    ak_transformCombine(node, t2);
    worldTransform = glm::make_mat4x4(t1);
    localTransform = glm::make_mat4x4(t2);
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

    shaders.processMesh(primitive);

    AkMaterial* mat = primitive->material;
    //while (mat) {
        material = scene->materials[mat];
    //    mat = (AkMaterial*)mat->base.next;
    //}
}

// hide in some way shaders
void Drawable::allocUnique()
{
    for (int i = 0; i < 7; i++) {
        if (shaders.accessor[i]) {
            glCreateBuffers(1, &primitiveDataBuffer[i]);
            glNamedBufferData(primitiveDataBuffer[i], shaders.accessor[i]->buffer->length, shaders.accessor[i]->buffer->data, GL_STATIC_DRAW);
        }
    }
}

void Drawable::draw(Scene& scene)
{
    glm::vec3 camera_view = scene.cameraEye.eye;
    glm::vec3 camera_dir = glm::vec3(0.) - scene.cameraEye.eye;
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 MV = transforms->MV * Model * localTransform; // check is it correct?
    glm::mat4 inverseMV = glm::inverse(Model) * transforms->inverseMV * localTransform;
    glm::ivec4 isTex;
    isTex.r = material.textures[MET_ROUGH] ? 1 : 0;
    isTex.g = material.textures[MET_ROUGH] ? 1 : 0;
    isTex.b = material.textures[ALBEDO] ? 1 : 0;
    isTex.a = 0;

    shaders.bindVertexArray(); //vao (format)

    glBindProgramPipeline(shaders.pipeline); // shaders
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scene.sceneLights.lightsBuffer);

    // same arguments and same order as createpipeline
    shaders.bindUniform({ { 
        {&MV, &scene.cameraEye.Projection},
        {&camera_view, &material.colors[MET_ROUGH].g, &material.colors[MET_ROUGH].r, 
        &material.colors[ALBEDO], &isTex, &inverseMV} 
    } });

    shaders.bindVertexBuffer(scene.bufferViews, scene.docDataBuffer); // vbo
    //bindTextures();        // delete ? - replacement belowe
    material.bindTextures(); // bind textures

    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
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



void Material::bindTextures()
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

void Material::deleteTexturesAndSamplers()
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



AkCamera* Scene::loadCamera(AkDoc* doc) 
{
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
    primitives.primitives.clear();

    scenePath += sceneName;
    AkDoc* doc;
    if (ak_load(&doc, scenePath.c_str(), NULL) != AK_OK) {
        SPDLOG_LOGGER_ERROR(&logger, "Document couldn't be loaded");
        exit(EXIT_FAILURE);
    }
    else {
        logger.info(printCoordSystem(doc->coordSys));
        logger.info(printDocInformation(doc->inf, doc->unit));
        logger.info("==============================================================================");
    }

    AkVisualScene* scene;
    scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
    if (!doc->scene.visualScene) {
        logger.error("================================== Scene couldnt be loaded! ===============");
        exit(EXIT_FAILURE);
    }
    else {
        std::string sceneInfo = "======================== Scene name: ";
        sceneInfo += scene->name ? scene->name : "";
        sceneInfo += "========================";
        logger.info(sceneInfo);
    }

    AkOneWayIterBase* mat_ptr = doc->lib.materials ? doc->lib.materials->chld : nullptr;
    const unsigned int mat_count = doc->lib.materials ? doc->lib.materials->count : 0;
    for (int i = 0; i < mat_count; i++, mat_ptr = mat_ptr->next) {
        Material material = processMaterial((AkMaterial*) mat_ptr);
        this->materials.insert({mat_ptr, material});
    }

    AkNode* node = ak_instanceObjectNode(scene->node);
    proccessNode(node, primitives.primitives, this);
    primitives.initPrimitives();
    loadCamera(doc);

    allocAll(doc);
    parseBuffors();
    fileListener.reset();

    for (auto& light : sceneLights.lights) {
        Matrix lightTransform;
        lightTransform.MV = ((Light*) lightModel.get())->calcMV(light, *this);
        lightModel->transforms = new Matrix(lightTransform);  //TODO: dealloc needed
    }


    return doc;
}


void FileListener::notify() {
    fileChanged = true;
    logger.info("fileChanged");
}

void FileListener::reset() {
    fileChanged = false;
}


bool Scene::fileChanged()
{
    return fileListener.fileChanged;
}
    
Scene::~Scene()
{
    for (auto& primitive : primitives.primitives) {
        myGui.unsubscribeToView(*static_cast<GUIMatrix*>(primitive.transforms));
    }

    clear();
    for (auto& primitive : primitives.primitives) {
        //primitive.deletePrograms();
        //primitive.deletePipeline();
        //primitive.deleteTexturesAndSamplers();
    }
    skySphere->shaders.deletePipeline();
    cloudCube->shaders.deletePipeline();
    lightModel->shaders.deletePipeline();
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

    this->docDataBuffer = docDataBuffer;

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
            };
        }
    }
    allocAll(doc);
    docDataBuffer = parseBuffors();
}


void Light::draw(Scene& scene)
{
    shaders.bindVertexArray();
    GLuint vcolLocation = glGetAttribLocation(shaders.programs[VERTEX], "vCol");
    if (vcolLocation != 0xFFFFFFFF) {
        glEnableVertexArrayAttrib(shaders.vao, vcolLocation);
    }
    if (shaders.normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(shaders.vao, shaders.normalsBindingLocation);
    if (shaders.textureBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(shaders.vao, shaders.textureBindingLocation);
    //glBindVertexArray(shaders.vao);
    glBindProgramPipeline(shaders.pipeline);

    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(transforms->MV));
    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(scene.cameraEye.Projection));

 
    shaders.bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    material.bindTextures();


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

void Environment::draw(Scene& scene)
{
    shaders.bindVertexArray();
    if(shaders.normalsBindingLocation != 0xFFFFFFFF) glDisableVertexArrayAttrib(shaders.vao, shaders.normalsBindingLocation);
    //glBindVertexArray(shaders.vao);
    glBindProgramPipeline(shaders.pipeline);


    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f));
    glm::mat4 MV = transforms->MV * Model * localTransform;
    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(MV));
    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(scene.cameraEye.Projection));


    shaders.bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    material.bindTextures();

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


void Cloud::draw(Scene& scene)
{
    shaders.bindVertexArray();
    //glBindVertexArray(shaders.vao);
    glBindProgramPipeline(shaders.pipeline);

    glProgramUniform1f(shaders.programs[FRAGMENT], shaders.bindingLocationIndecies[FRAGMENT][0], g);
    glProgramUniform3fv(shaders.programs[FRAGMENT], shaders.bindingLocationIndecies[FRAGMENT][1], 1, glm::value_ptr(scene.cameraEye.eye));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scene.sceneLights.lightsBuffer);

    glm::mat4 inverseMV = transforms->inverseMV * localTransform;
    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][0], 1, GL_FALSE, glm::value_ptr(transforms->MV));
    glProgramUniformMatrix4fv(shaders.programs[VERTEX], shaders.bindingLocationIndecies[VERTEX][1], 1, GL_FALSE, glm::value_ptr(scene.cameraEye.Projection));
    glProgramUniformMatrix4fv(shaders.programs[FRAGMENT], shaders.bindingLocationIndecies[FRAGMENT][2], 1, GL_FALSE, glm::value_ptr(inverseMV));

    shaders.bindVertexBuffer(this->bufferViews, this->docDataBuffer);
    material.bindTextures();

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, verticleIndeciesSize, GL_UNSIGNED_INT, verticleIndecies);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}


/* ================================================ */


void SceneLights::updateLights (GUI& myGui) {
    if (myGui.getLightsSize() != lightDataSize) {
        if (myGui.getLightsSize() > lightDataSize) {
            lightDataSize = myGui.getLightsSize();
            int lightsBufferSize = (int)sizeof(PointLight) * lights.size();
            glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
        }
        lightDataSize = myGui.getLightsSize();
        int lightsBufferSize = (int)sizeof(PointLight) * lights.size();
        glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lights.data());
        glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);
    }
    PointLight newLight = myGui.getLight();
    if (compareLights(lights.data()[myGui.lightId], newLight)) {
        lights.data()[myGui.lightId] = newLight;
        LightsList* ptr = (LightsList*)glMapNamedBuffer(lightsBuffer, GL_WRITE_ONLY);
        memcpy_s((void*)&ptr->list[myGui.lightId], sizeof(PointLight), &newLight, sizeof(PointLight));
        glUnmapNamedBuffer(lightsBuffer);
        myGui.updateLight();
    }
}

void SceneLights::initLights() {
    lights.clear();
    lights.push_back({ glm::vec3(1.5f, 1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f) });
    lights.push_back({ glm::vec3(-1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
    lights.push_back({ glm::vec3(1.5f, -1.5f, 1.5f), 0.1f, 0.5f, 0.5f, glm::vec3(1.f, .9f, .8f), glm::vec3(.7f, .5f, .4f), glm::vec3(.3f, .2f, .1f) });
}


bool SceneLights::compareLights(PointLight& old_light, PointLight& new_light)
{
    return memcmp(&old_light, &new_light, sizeof(PointLight));
}

bool SceneLights::compareLights(LightsList& old_light, LightsList& new_light)
{
    if (old_light.size != new_light.size) return true;

    return memcmp(&old_light.list, &new_light.list, old_light.size * sizeof(PointLight));
}


SceneLights::SceneLights() {
    glGenBuffers(1, &lightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);

    initLights();
    int lightsBufferSize = (int)sizeof(PointLight) * lights.size();
    lightDataSize = (unsigned int)lights.size();

    glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lights.data());
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);
}

SceneLights::~SceneLights() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glDeleteBuffers(1, &lightsBuffer);
}


glm::mat4 Light::calcMV(PointLight& light, Scene& scenes) {
    glm::mat4x4 View =
        glm::translate(localTransform, light.position);
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(.2f));
    glm::mat4 LookAt = myGui.getLookAt();
    glm::mat4 transforms = LookAt * View * Model;
    return transforms;
}

void SceneLights::notify() {
    updateLights(myGui);
}

void Scene::draw() 
{
    skySphere->draw(*this);

    for (auto& primitive : primitives.primitives) {
        primitive.draw(*this);
    }

    for (auto& light : sceneLights.lights) {
        lightModel->transforms->MV = ((Light*)lightModel.get())->calcMV(light, *this);
        lightModel->draw(*this);
    }

    cloudCube->draw(*this);
}

Scene::Scene(GUI& gui, WindowInfo& windowConfig)
{
    lightModel = Light::createDrawable(this);
    skySphere = Environment::createDrawable(this);
    cloudCube = Cloud::createDrawable(this);

    myGui.subscribeToView(*static_cast<GUIMatrix*>(cloudCube->transforms));
    myGui.subscribeToEye(*static_cast<GUIMatrix*>(skySphere->transforms));

    myGui.lightsData = &sceneLights.lights;
    cameraEye.Projection = myGui.getProjection(windowConfig.width, windowConfig.height);
    cameraEye.imageDimension = glm::vec2(windowConfig.width, windowConfig.height);
    myGui.lightsData.subscribe(sceneLights);
    myGui.g.subscribe(*((Cloud*)cloudCube.get()));
    myGui.subscribeToEye(cameraEye);
    myGui.subscribeToLight(sceneLights);
    myGui.selectedSceneFile.subscribe(fileListener);
}

void Scene::clear()
{
    glBindProgramPipeline(0); // TO DO: delete

    for (auto& primitive : primitives.primitives) {
        primitive.material.deleteTexturesAndSamplers();
        primitive.shaders.deletePipeline();
        //if (primitive.transforms) delete primitive.transforms;
    }
    //if(cloudCube->transforms) delete cloudCube->transforms;
    //if(skySphere->transforms) delete skySphere->transforms;
    //if (lightModel->transforms) delete lightModel->transforms;
    //TODO: dealloc of light matrix

    glDeleteBuffers((GLsizei) bufferViews.size(), docDataBuffer);
    if (docDataBuffer) free(docDataBuffer);

    for (auto& primitive : primitives.primitives) {
        for (int i = VERTEX; i <= GEOMETRY; i++) {
            if (primitive.shaders.bindingLocationIndecies[i]) free(primitive.shaders.bindingLocationIndecies[i]);
            if (primitive.shaders.bindingTypes[i]) free(primitive.shaders.bindingTypes[i]);
        } // shaders - delete that
    }
}