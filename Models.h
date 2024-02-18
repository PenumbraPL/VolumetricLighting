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
    DRAW_FRAGMENT = GL_FRAGMENT_SHADER,
    DRAW_TESS_CTR = GL_TESS_CONTROL_SHADER,
    DRAW_TESS_EV = GL_TESS_EVALUATION_SHADER,
    DRAW_GEOMETRY = GL_GEOMETRY_SHADER
};

enum DrawShaderBit {
    DRAW_VERTEX_BIT = GL_VERTEX_SHADER_BIT,
    DRAW_FRAGMENT_BIT = GL_FRAGMENT_SHADER_BIT,
    DRAW_TESS_CTR_BIT = GL_TESS_CONTROL_SHADER_BIT,
    DRAW_TESS_EV_BIT = GL_TESS_EVALUATION_SHADER_BIT,
    DRAW_GEOMETRY_BIT = GL_GEOMETRY_SHADER_BIT
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



struct Drawable {
    Drawable() {}
    ~Drawable(){}

    GLuint programs[5];
    GLuint pipeline;
    AkAccessor* accessor[7];

    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    GLuint primitiveDataBuffer[7] = { 0xffffffff };
    GLuint textures[8] = { 0 };
    GLuint texturesType[8] = { 0 };
    GLuint samplers[8] = { 0 }; // alloc ?

    glm::vec4 colors[8] = { glm::vec4(0) };

    glm::mat4 worldTransform;
    glm::mat4 localTransform;

    GLuint vao;

    DrawShader ds[5] = { DRAW_VERTEX, DRAW_FRAGMENT, DRAW_TESS_CTR, DRAW_TESS_EV, DRAW_GEOMETRY };
    DrawShaderBit dsb[5] = { DRAW_VERTEX_BIT, DRAW_FRAGMENT_BIT, DRAW_TESS_CTR_BIT, DRAW_TESS_EV_BIT , DRAW_GEOMETRY_BIT };

    GLuint vertexPosBindingLocation;
    GLuint normalsBindingLocation;
    GLuint textureBindingLocation;
    GLuint* bindingLocationIndecies[5] = { nullptr };

    void createPipeline(std::string shaderPath[5]);
    void deletePipeline();
    void loadMatrix(AkNode* node);
    virtual void processMesh(AkMeshPrimitive* primitive);
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
    void bindVertexArray();
    virtual void getLocation(std::vector<const char*> uniformNames[5]);
    virtual void deleteTexturesAndSamplers(); // how many to delete?

protected:
    void allocUnique();
};


struct P : public Drawable {
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
}; // without change (maybe declaration of paths?

struct L : public Drawable {
    enum LightType { POSITIONAL, DIRECTIONAL, AREA } light_type = POSITIONAL;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    void loadMesh();
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
};

struct E : public Drawable {
    GLuint skybox;
    GLuint env_sampler;
    
    void loadMesh();
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
};

struct C : public Drawable {
    GLuint depthBuffer;
    GLuint lightbuffer;
    GLuint g;
    
    void loadMesh();
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
};


struct Camera {
    glm::mat4x4 localTransform;
    glm::mat4x4 worldTransform;
    glm::vec4 viewDirection;
    float zNear;
    float zFar;
    int fov;
    glm::mat4 View;
    glm::mat4 Projection;
};


struct Scene {
    ~Scene();

    //std::vector<Primitive> primitives;
    std::vector<Drawable> primitives;
    Camera cameraEye;
    std::vector<PointLight> lights;
    std::map <void*, unsigned int> bufferViews;
    std::map <void*, unsigned int> textureViews;
    std::map <void*, unsigned int> imageViews;

    AkDoc* loadScene(std::string scenePath, std::string sceneName);
    void allocAll(AkDoc* doc);
    GLuint* parseBuffors();
    AkCamera* camera(AkDoc* doc);
    void updateLights(GLuint lightsBuffer, unsigned int lightDataSize, ConfigContext& panelConfig);
    void initLights();
    bool compare_lights(PointLight& old_light, PointLight& new_light);
    bool compare_lights(LightsList& old_light, LightsList& new_light);
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