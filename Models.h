#pragma once
#include "GLFW/glfw3.h"
#include "GUI.h"
#include "pch.h"

extern ConfigContext panelConfig;

enum TextureType {
    AMBIENT,
    EMISIVE,
    DIFFUSE,
    SPECULAR,
    SP_GLOSSINESS,
    MET_ROUGH,
    ALBEDO,
    SP_DIFFUSE,
    TT_SIZE,
    SKYBOX,
    AO
};

enum ShaderTypes {
    VERTEX,
    FRAGMENT,
    TESS_CTR,
    TESS_EV,
    GEOMETRY
};

enum AccessorTypes {
    POSITION,
    TEXTURES,
    NORMALS,
    COLORS,
    WEIGTHS,
    JOINTS,
    TANGENTS
};

enum DrawShader {
    DRAW_VERTEX = GL_VERTEX_SHADER,
    DRAW_TESS_CTR = GL_TESS_CONTROL_SHADER,
    DRAW_TESS_EV = GL_TESS_EVALUATION_SHADER,
    DRAW_GEOMETRY = GL_GEOMETRY_SHADER,
    DRAW_FRAGMENT = GL_FRAGMENT_SHADER
};

enum DrawShaderBit {
    DRAW_VERTEX_BIT = GL_VERTEX_SHADER_BIT,
    DRAW_TESS_CTR_BIT = GL_TESS_CONTROL_SHADER_BIT,
    DRAW_TESS_EV_BIT = GL_TESS_EVALUATION_SHADER_BIT,
    DRAW_GEOMETRY_BIT = GL_GEOMETRY_SHADER_BIT,
    DRAW_FRAGMENT_BIT = GL_FRAGMENT_SHADER_BIT
};



void* imageLoadFromFile(
    const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components);

void* imageLoadFromMemory(
    const char* __restrict data,
    size_t                  len,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components);

void imageFlipVerticallyOnLoad(bool flip);



struct Drawable {
    Drawable(){}
    ~Drawable() {}


    GLuint programs[5];
    GLuint pipeline;
    AkAccessor* accessor[7];

    GLuint* bindingLocations;
    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    GLuint* primitiveDataBuffer;

    GLuint* textures = nullptr;
    GLuint* texturesType = nullptr;
    GLuint* samplers = nullptr;
    // alloc

    glm::mat4 worldTransform;
    glm::mat4 localTransform;

    DrawShader ds[5] = { DRAW_VERTEX, DRAW_TESS_CTR, DRAW_TESS_EV, DRAW_GEOMETRY, DRAW_FRAGMENT };
    DrawShaderBit dsb[5] = { DRAW_VERTEX_BIT, DRAW_TESS_CTR_BIT, DRAW_TESS_EV_BIT , DRAW_GEOMETRY_BIT, DRAW_FRAGMENT_BIT };



    //virtual void createPipeline(std::string shaderPath[5])
    //{
    //    char* shader[5];
    //    glGenProgramPipelines(1, &pipeline);
    //    GLint linkageStatus;

    //    for (int i = 0; i < 5; i++) {
    //        if (shaderPath[i].empty()) {
    //            shader[i] = read_file(shaderPath[i].c_str());
    //            if (!shader[i]) {
    //                logger.error("=================== Coulnt find " + shaderPath[i] + " ==============================\n");
    //            }
    //            programs[i] = glCreateShaderProgramv(ds[i], 1, &shader[i]);
    //            free(shader[i]);

    //            glGetProgramiv(programs[i], GL_LINK_STATUS, &linkageStatus);
    //            if (!linkageStatus) {
    //                GLchar info[1024];
    //                glGetProgramInfoLog(programs[i], 1024, NULL, info);
    //                logger.error(info);
    //            }
    //            glUseProgramStages(pipeline, dsb[i], programs[i]);
    //        }
    //    }
    //}

    //virtual void deletePipeline()
    //{
    //    for (int i = 0; i < 5; i++) {
    //        if(programs[i]) glDeleteProgram(programs[i]);
    //    } // program[i] == 0 ?

    //    glBindProgramPipeline(0);
    //    glDeleteProgramPipelines(1, &pipeline);
    //}

    //virtual void loadMesh(std::string scenePath, std::string sceneName)
    //{
    //    scenePath += sceneName;
    //    AkDoc* doc;
    //    if (ak_load(&doc, scenePath.c_str(), NULL) != AK_OK) {
    //        logger.error("Environment mesh couldn't be loaded\n");
    //        return;
    //    }
    //    if (!doc->scene.visualScene) {
    //        logger.error("Environment mesh couldn't be loaded\n");
    //        return;
    //    }

    //    AkVisualScene* scene;
    //    scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
    //    AkNode* node = ak_instanceObjectNode(scene->node);

    //    float* t1 = (float*)calloc(16, sizeof(float));
    //    float* t2 = (float*)calloc(16, sizeof(float));
    //    ak_transformCombineWorld(node, t1);
    //    ak_transformCombine(node, t2);
    //    worldTransform = glm::make_mat4x4(t1);
    //    localTransform = glm::make_mat4x4(t2);
    //    free(t1);
    //    free(t2);

    //    if (node->geometry) {
    //        AkGeometry* geometry = ak_instanceObjectGeom(node);
    //        AkMesh* mesh = (AkMesh*)ak_objGet(geometry->gdata);
    //        if ((AkGeometryType)geometry->gdata->type) {
    //            if (mesh) {
    //                AkMeshPrimitive* primitive = mesh->primitive;
    //                if (primitive->indices) {
    //                    verticleIndecies = (uint32_t*)primitive->indices->items;
    //                    verticleIndeciesSize = (unsigned int)primitive->indices->count;
    //                }

    //                int set = primitive->input->set;
    //                AkInput* wgs = ak_meshInputGet(primitive, "WEIGHTS", set);
    //                AkInput* jts = ak_meshInputGet(primitive, "JOINTS", set);
    //                AkInput* pos = ak_meshInputGet(primitive, "POSITION", set);
    //                AkInput* tex = ak_meshInputGet(primitive, "TEXCOORD", set); // if indexed then multiple parts to proccess
    //                AkInput* nor = ak_meshInputGet(primitive, "NORMAL", set);

    //                AkInput* col = ak_meshInputGet(primitive, "COLOR", set);
    //                AkInput* tan = ak_meshInputGet(primitive, "TANGENT", set);

    //                //std::cout << ak_meshInputCount(mesh) << std::endl;

    //                accessor[AccessorTypes::POSITION] = pos ? pos->accessor : nullptr;
    //                accessor[AccessorTypes::TEXTURES] = tex ? tex->accessor : nullptr;
    //                accessor[AccessorTypes::NORMALS] = nor ? nor->accessor : nullptr;
    //                accessor[AccessorTypes::WEIGTHS] = wgs ? wgs->accessor : nullptr;
    //                accessor[AccessorTypes::JOINTS] = jts ? jts->accessor : nullptr;
    //                accessor[AccessorTypes::COLORS] = col ? col->accessor : nullptr;
    //                accessor[AccessorTypes::TANGENTS] = tan ? tan->accessor : nullptr;



    //                if (primitive->material) {
    //                    AkMaterial* mat = primitive->material;
    //                    AkEffect* ef = (AkEffect*)ak_instanceObject(&mat->effect->base);
    //                    AkTechniqueFxCommon* tch = ef->profile->technique->common;
    //                    if (tch) {
    //                        set_up_color(tch->ambient, primitive, &samplers[AMBIENT], &textures[AMBIENT], &texturesType[AMBIENT], panelConfig);
    //                        set_up_color(tch->emission, primitive, &samplers[EMISIVE], &textures[EMISIVE], &texturesType[EMISIVE], panelConfig);
    //                        set_up_color(tch->diffuse, primitive, &samplers[DIFFUSE], &textures[DIFFUSE], &texturesType[DIFFUSE], panelConfig);
    //                        set_up_color(tch->specular, primitive, &samplers[SPECULAR], &textures[SPECULAR], &texturesType[SPECULAR], panelConfig);

    //                        switch (tch->type) {
    //                        case AK_MATERIAL_METALLIC_ROUGHNESS: {
    //                            AkMetallicRoughness* mr = (AkMetallicRoughness*)tch;
    //                            AkColorDesc alb_cd;
    //                            AkColorDesc mr_cd;
    //                            alb_cd.color = &mr->albedo;
    //                            alb_cd.texture = mr->albedoTex;
    //                            mr_cd.color = &mr->albedo;//&mr->roughness;
    //                            mr_cd.texture = mr->metalRoughTex;
    //                            set_up_color(&alb_cd, primitive, &samplers[ALBEDO], &textures[ALBEDO], &texturesType[ALBEDO], panelConfig);
    //                            set_up_color(&mr_cd, primitive, &samplers[MET_ROUGH], &textures[MET_ROUGH], &texturesType[MET_ROUGH], panelConfig);
    //                            break;
    //                        }

    //                        case AK_MATERIAL_SPECULAR_GLOSSINES: {
    //                            AkSpecularGlossiness* sg = (AkSpecularGlossiness*)tch;
    //                            AkColorDesc sg_cd;
    //                            AkColorDesc dif_cd;
    //                            sg_cd.color = &sg->specular;
    //                            sg_cd.texture = sg->specGlossTex;
    //                            dif_cd.color = &sg->diffuse;
    //                            dif_cd.texture = sg->diffuseTex;
    //                            set_up_color(&sg_cd, primitive, &samplers[SP_GLOSSINESS], &textures[SP_GLOSSINESS], &texturesType[SP_GLOSSINESS], panelConfig);
    //                            set_up_color(&dif_cd, primitive, &samplers[SP_DIFFUSE], &textures[SP_DIFFUSE], &texturesType[SP_DIFFUSE], panelConfig);
    //                            break;
    //                        }
    //                        };
    //                        //std::cout << "Is double sized: " << (tch->doubleSided ? "True" : "False");
    //                    }
    //                }



    //                primitiveDataBuffer = (GLuint*)calloc(2, sizeof(GLuint));

    //                if (primitiveDataBuffer) {
    //                    glCreateBuffers(2, primitiveDataBuffer);
    //                    glNamedBufferData(primitiveDataBuffer[0], pos->accessor->buffer->length, pos->accessor->buffer->data, GL_STATIC_DRAW);
    //                    glNamedBufferData(primitiveDataBuffer[1], tex->accessor->buffer->length, tex->accessor->buffer->data, GL_STATIC_DRAW);
    //                }
    //            }
    //        }
    //        if (node->next) {
    //            node = node->next;
    //           // proccess_node(node, primitives);
    //        }
    //        if (node->chld) {
    //            node = node->chld;
    //           // proccess_node(node, primitives);
    //        }
    //    }

/*        glCreateTextures(GL_TEXTURE_2D, 1, &skybox);
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
    }*/


    //virtual void draw(int width, int height, glm::mat4 Proj, AkCamera* camera);
};


struct Environment {
    glm::mat4x4 localTransform = glm::mat4x4(0.);
    glm::mat4x4 worldTransform = glm::mat4x4(0.);

    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertexProgram;
    GLuint fragmentProgram;
    GLuint pipeline;
    GLuint skybox;
    GLuint env_sampler;
    glm::mat4 MVP = glm::mat4x4(0.);
    GLuint* primitiveDataBuffer = nullptr;
    GLuint mvpBindingLocation;

    void createPipeline();
    void deletePipeline();
    void loadMesh();
    void draw(
        int width, 
        int height, 
        glm::mat4 Proj, 
        AkCamera* camera);
};


struct Primitive {
    float* localTransform;
    float* worldTransform;

    uint32_t* verticleIndecies;
    unsigned int verticleIndeciesSize;
    GLuint* programs;
    GLuint pipeline;

    GLuint* textures = nullptr;
    glm::vec4 colors[8] = { glm::vec4(1.)};
    GLuint* texturesType = nullptr;
    GLuint* samplers = nullptr;

    AkAccessor* wgs;
    AkAccessor* jts;
    AkAccessor* pos;
    AkAccessor* tex;
    AkAccessor* nor;
    AkAccessor* col;
    AkAccessor* tan;

    GLuint mvpBindingLocation,
        prjBindingLocation,
        cameraBindingLocation,
        gBindingLocation,
        isTexBindingLocation,
        camDirBindingLocation, 
        vertexPosBindingLocation, 
        normalsBindingLocation,
        textureBindingLocation;
    GLuint metalicBindingLocation,
        roughnessBindingLocation,
        albedoBindingLocation,
        aoBindingLocation;


    void getLocation();
    void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews, 
        GLuint* buffers,
        glm::vec3& eye,
        glm::mat4& LookAt,
        glm::mat4& Projection,
        glm::vec3& translate, 
        glm::vec3& rotate);
    float* setTransform(void);
    float* setWorldTransform(void);
    void deleteTransforms();
    GLuint* createPrograms();
    void createPipeline();
    void deletePipeline();
    void deletePrograms();
    GLuint* createTextures();
    GLuint* createSamplers();
    void deleteTexturesAndSamplers();
};


struct Light {
    enum LightType { POSITIONAL, DIRECTIONAL, AREA } light_type = POSITIONAL;
    glm::mat4x4 localTransform;
    glm::mat4x4 worldTransform;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    AkInput* pos = nullptr;
    AkInput* nor = nullptr;

    GLuint vertexProgram;
    GLuint fragmentProgram;
    GLuint pipeline;
    GLuint mvpBindingLocation;
    GLuint primitiveDataBuffer;

    void createPipeline();
    void deletePipeline();
    void loadMesh();
    void drawLight(
        int width, 
        int height, 
        glm::mat4 Proj, 
        AkCamera* camera,
        glm::mat4x4& transform);
};


struct Camera {
    glm::mat4x4 localTransform;
    glm::mat4x4 worldTransform;
    glm::vec4 viewDirection;
    float zNear;
    float zFar;
    int fov;
};


struct Cloud {
    glm::mat4x4 localTransform = glm::mat4x4(0.);
    glm::mat4x4 worldTransform = glm::mat4x4(0.);

    int width = 0;
    int height = 0;

    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertexProgram;
    GLuint fragmentProgram;
    GLuint pipeline;

    glm::mat4 MVP;
    GLuint* primitiveDataBuffer;
    GLuint mvpBindingLocation;
    GLuint prjBindingLocation;

    GLuint depthBuffer;

    void createPipeline(int width, int height);
    void deletePipeline();
    void loadMesh();
    void draw(
        int width, 
        int height, 
        glm::mat4 Proj, 
        AkCamera* camera, 
        float& g, 
        GLuint lightbuffer);
};