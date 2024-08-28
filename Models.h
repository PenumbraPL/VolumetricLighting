#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GUI.h"
#include "pch.h"

extern ConfigContext panelConfig;

typedef std::vector<std::string> BindingPointList;
typedef std::array<BindingPointList, 5> BindingPointCollection;
typedef std::array<std::string, 5> ShadersSources;

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



struct Drawable {
    Drawable() {}
    ~Drawable(){}

    GLuint programs[5] = { 0xffffffff };
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


    std::map <void*, unsigned int> bufferViews;
    std::map <void*, unsigned int> textureViews;
    std::map <void*, unsigned int> imageViews;
    GLuint* docDataBuffer;

    void createPipeline(ShadersSources shadersPaths);
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
    void bindVertexBuffer(std::map <void*, unsigned int>& bufferViews, GLuint* docDataBuffer);
    void bindTextures();
    GLuint* parseBuffors();
    void allocAll(AkDoc* doc);
    virtual void getLocation(BindingPointCollection uniformNames);
    virtual void deleteTexturesAndSamplers(); // how many to delete?

protected:
    void allocUnique();
};


struct Primitive : public Drawable {
    virtual void draw(
        GLuint& lights_buffer,
        std::map <void*, unsigned int>& bufferViews,
        GLuint* docDataBuffer,
        glm::vec3& eye,
        glm::mat4& MVP,
        glm::mat4& Projection);
}; // without change (maybe declaration of paths?


struct Light : public Drawable {
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


struct Environment : public Drawable {
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


struct Cloud : public Drawable {
    float g = 0.;
    
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
