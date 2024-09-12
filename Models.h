#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "GUI.h"
#include "pch.h"
#include "IO.h"

extern GUI myGui;


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

struct Scene;


class Matrix {
public:
    Matrix() {}
    Matrix(glm::mat4 localTransform) : localTransform{ localTransform } {}

    glm::mat4 localTransform = glm::mat4(1.);
    glm::mat4 Projection = glm::mat4(1.);;
    glm::mat4 MVP = glm::mat4(1.);;
    glm::mat4 MV = glm::mat4(1.);;
};

class GUIMatrix : public Matrix, public Observer {
public:
    GUIMatrix() {}
    GUIMatrix(glm::mat4 localTransform) : Matrix{ localTransform } {}

    void setProjection(int width, int height) {
        //if(!camera)
        Projection = myGui.getProjection(width, height);
        //Projection = Proj;
    }

    void calculateMVP() {
        glm::mat4 LookAt = myGui.getLookAt();
        glm::vec3 translate = myGui.getTranslate();
        glm::vec3 rotate = myGui.getRotate();

        glm::mat4 View =
            glm::rotate(
                glm::rotate(
                    glm::translate(localTransform,
                        translate)
                    , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));

        MV = LookAt * View;
        MVP = Projection * MV;
    }

    virtual void notify() {
        calculateMVP();
    }
};

struct Drawable {
    Drawable(){}
    Drawable(Matrix* transforms) : transforms(transforms) {}
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
    Matrix* transforms;

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
    virtual void draw(Scene& scene);
    void bindVertexArray();
    void bindVertexBuffer(std::map <void*, unsigned int>& bufferViews, GLuint* docDataBuffer);
    void bindTextures();
    GLuint* parseBuffors();
    void allocAll(AkDoc* doc);
    virtual void getLocation(BindingPointCollection uniformNames);
    virtual void deleteTexturesAndSamplers(); // how many to delete?
    virtual void loadMesh() {};
protected:
    void allocUnique();
};


struct Primitive : public Drawable {
    void loadMesh() override {};
    virtual void draw(Scene& scene) override;
}; // without change (maybe declaration of paths?


struct Light : public Drawable {
    enum LightType { POSITIONAL, DIRECTIONAL, AREA } light_type = POSITIONAL;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    void loadMesh() override;
    virtual void draw(Scene& scene) override;
    glm::mat4 calcMV(PointLight& light, Scene& scenes);
    static std::unique_ptr<Drawable> createDrawable() {
        auto lightModel = std::make_unique<Light>();
        lightModel->loadMesh();
        lightModel->createPipeline({ "res/shaders/lamp_vec.glsl", "res/shaders/lamp_frag.glsl" });
        lightModel->getLocation({ { {"MV", "PRJ"}, {"G", "camera"} } });
        return lightModel;
    }
};


struct Environment : public Drawable {
    GLuint skybox;
    GLuint env_sampler;
    
    void loadMesh() override;
    virtual void draw(Scene& scene) override;
    static std::unique_ptr<Drawable> createDrawable() {
        auto env = std::make_unique<Environment>();
        env->transforms = new GUIMatrix(); //TODO: dealloc needed
        env->loadMesh();
        env->createPipeline({ "res/shaders/environment_vec.glsl", "res/shaders/environment_frag.glsl" });
        env->getLocation({ {{"MV", "PRJ"}} });
        return env;
    }
};


struct Cloud : public Drawable, public Observer {
    float g = 0.;
    
    void loadMesh() override;
    virtual void draw(Scene& scene) override;
    virtual void notify() override {
        g = myGui.g;
    }
    static std::unique_ptr<Drawable> createDrawable() {
        auto cld = std::make_unique<Cloud>();
        cld->transforms = new GUIMatrix(); //TODO: dealloc needed
        cld->loadMesh();
        cld->createPipeline({ "res/shaders/depth_ver.glsl", "res/shaders/depth_frag.glsl" });
        cld->getLocation({ { {"MV", "PRJ"}, {"G", "camera"} } });
        return cld;
    }
};


struct Camera : Observer{
    glm::mat4x4 localTransform;
    glm::mat4x4 worldTransform;
    glm::vec4 viewDirection;
    glm::vec3 eye;
    float zNear;
    float zFar;
    int fov;
    glm::mat4 View;
    glm::mat4 Projection;
    glm::vec2 imageDimension;

    virtual void notify() {
        eye = myGui.getView();
        Projection = myGui.getProjection(imageDimension.x, imageDimension.y);
    }
};


struct SceneLights : public Observer{
    std::vector<PointLight> lights;
    GLuint lightsBuffer;
    unsigned int lightDataSize;

    SceneLights();
    ~SceneLights();

    void updateLights(GUI& panelConfig);
    void initLights();
    bool compareLights(PointLight& old_light, PointLight& new_light);
    bool compareLights(LightsList& old_light, LightsList& new_light);
    virtual void notify();
};


struct Scene {
    std::vector<Drawable> primitives;
    Camera cameraEye;
    std::map <void*, unsigned int> bufferViews;
    std::map <void*, unsigned int> textureViews;
    std::map <void*, unsigned int> imageViews;
    GLuint* docDataBuffer;
    SceneLights sceneLights;

    std::unique_ptr<Drawable> skySphere;
    std::unique_ptr<Drawable> cloudCube;
    std::unique_ptr<Drawable> lightModel;
    
    Scene(GUI& gui, WindowInfo& windowConfig);
    ~Scene();
    AkDoc* loadScene(std::string scenePath, std::string sceneName);
    void allocAll(AkDoc* doc);
    GLuint* parseBuffors();
    AkCamera* loadCamera(AkDoc* doc);
    void draw();
    void clear();
};
