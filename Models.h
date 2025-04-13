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
typedef std::map <void*, unsigned int> OrderedAssets;

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
    Matrix(float scale, glm::mat4 localTransform) : scale{ scale }, localTransform { localTransform } {}

    float scale = 1.f;
    glm::mat4 localTransform;
    glm::mat4 Projection = glm::mat4(1.);
    glm::mat4 MVP = glm::mat4(1.);
    glm::mat4 MV = glm::mat4(1.);
    glm::mat4 inverseMV = glm::mat4(1.);
};

class GUIMatrix : public Matrix, public Observer {
public:
    GUIMatrix() {}
    GUIMatrix(float scale, glm::mat4 localTransform) : Matrix{ scale, localTransform } {}

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
                    glm::translate(glm::mat4(1.),
                        translate)
                    , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));

        MV = LookAt * View;
        MVP = Projection * MV;
        inverseMV = glm::inverse(LookAt) *
            glm::rotate(
                glm::rotate(
                    glm::translate(glm::mat4(1.),
                        -translate)
                    , -rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                -rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));


        // Propebly to relocate in the future
        glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        MV = MV * Model * localTransform; // check is it correct?
        inverseMV = glm::inverse(Model) * inverseMV * localTransform;
    }

    virtual void notify() {
        calculateMVP();
    }
};

struct Material {
    //AkAccessor* accessor[7]; // it is nessessery here?

    GLuint textures[8] = { 0 };
    GLuint texturesType[8] = { 0 };
    GLuint samplers[8] = { 0 }; // alloc ?
    glm::vec4 colors[8] = { glm::vec4(0) };

    void bindTextures();
    void bindVertexBuffer(OrderedAssets& bufferViews, GLuint* docDataBuffer);
    void deleteTexturesAndSamplers(); // how many to delete?
};

struct ShadersPipeline {
    GLuint programs[5] = { 0xffffffff };
    GLuint pipeline;
    GLuint vao;

    AkAccessor* accessor[7];

    GLuint vertexPosBindingLocation;
    GLuint normalsBindingLocation;
    GLuint textureBindingLocation;
    GLuint* bindingLocationIndecies[5] = { nullptr };
    BindingPointCollection bindingNames;
    GLenum* bindingTypes[5] = { nullptr };

    DrawShader ds[5] = { DRAW_VERTEX, DRAW_FRAGMENT, DRAW_TESS_CTR, DRAW_TESS_EV, DRAW_GEOMETRY };
    DrawShaderBit dsb[5] = { DRAW_VERTEX_BIT, DRAW_FRAGMENT_BIT, DRAW_TESS_CTR_BIT, DRAW_TESS_EV_BIT , DRAW_GEOMETRY_BIT };

    void createPipeline(ShadersSources shaderPath);
    void deletePipeline();
    void bindVertexArray();
    void getLocation(BindingPointCollection uniformNames);
    void processMesh(AkMeshPrimitive* primitive);
    void bindVertexBuffer(OrderedAssets& bufferViews, GLuint* docDataBuffer);
    void bindUniform(std::array<std::vector<void*>, 5> values);
};


struct PrintableResources {
    uint32_t* verticleIndecies = nullptr;
    unsigned int verticleIndeciesSize;
    OrderedAssets bufferViews;
    OrderedAssets textureViews;
    OrderedAssets imageViews;

    GLuint* docDataBuffer; //bufferView pointer after allocation on GPU
};



struct Drawable {
    Drawable(){
        transforms = new Matrix{}; // TODO: dealloc 
    }
    Drawable(Matrix* transforms) : transforms(transforms) {}
    ~Drawable(){}

    Material material;
    ShadersPipeline shaders;
    Scene* scene;
    Matrix* transforms;
    PrintableResources allAssets;

    void loadMatrix(AkNode* node);
    virtual void processMesh(AkMeshPrimitive* primitive);
    virtual void draw(Scene& scene);
    void bindVertexBuffer(OrderedAssets& bufferViews, GLuint* docDataBuffer);
    GLuint* parseBuffors();
    void allocAll(AkDoc* doc);
    virtual void loadMesh() {};
};


struct Primitives {
    std::vector<Drawable> primitives;

    OrderedAssets bufferViews; // why?
    OrderedAssets textureViews; // why?
    OrderedAssets imageViews; // why?
    GLuint* docDataBuffer; // why?


    void initPrimitives(){
        for (auto& primitive : primitives) {
            ShadersSources defaultModel;
            defaultModel[VERTEX] = { "res/shaders/standard_vec.glsl" };
            defaultModel[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
            primitive.shaders.createPipeline(defaultModel);
            primitive.shaders.getLocation({ {
                {"MV", "PRJ"},
                {"camera", "_metalic", "_roughness", "_albedo_color", /*"ao_color",*/ "_is_tex_bound", "inverseMV"}
            } });
            primitive.transforms = new GUIMatrix(1.0, primitive.transforms->localTransform);   //TODO: dealloc needed
            myGui.subscribeToView(*static_cast<GUIMatrix*>(primitive.transforms));
        }
    }
    void clear() {
        primitives.clear();
    }
}; 


struct Light : public Drawable {
    enum LightType { POSITIONAL, DIRECTIONAL, AREA } light_type = POSITIONAL;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    void loadMesh() override;
    virtual void draw(Scene& scene) override;
    glm::mat4 calcMV(PointLight& light, Scene& scenes);
    static std::unique_ptr<Drawable> createDrawable(Scene* scene) {
        auto lightModel = std::make_unique<Light>();
        lightModel->scene = scene;
        lightModel->loadMesh();
        lightModel->shaders.createPipeline({ "res/shaders/lamp_vec.glsl", "res/shaders/lamp_frag.glsl" });
        lightModel->shaders.getLocation({ { {"MV", "PRJ"}, {"G", "camera"} } });
        return lightModel;
    }
};


struct Environment : public Drawable {
    GLuint skybox;
    GLuint env_sampler;

    void loadMesh() override;
    virtual void draw(Scene& scene) override;
    static std::unique_ptr<Drawable> createDrawable(Scene* scene) {
        auto env = std::make_unique<Environment>();
        env->scene = scene;
        env->transforms = new GUIMatrix(2., env->transforms->localTransform); //TODO: dealloc needed
        env->loadMesh();
        env->shaders.createPipeline({ "res/shaders/environment_vec.glsl", "res/shaders/environment_frag.glsl" });
        env->shaders.getLocation({ {{"MV", "PRJ"}} });
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
    static std::unique_ptr<Drawable> createDrawable(Scene* scene) {
        auto cld = std::make_unique<Cloud>();
        cld->scene = scene;
        cld->transforms = new GUIMatrix(2., cld->transforms->localTransform); //TODO: dealloc needed
        cld->loadMesh();
        cld->shaders.createPipeline({ "res/shaders/depth_ver.glsl", "res/shaders/depth_frag.glsl" });
        cld->shaders.getLocation({ { {"MV", "PRJ"}, {"G", "camera", "inverseMV"}}});
        return cld;
    }
};


struct Camera : Observer{
    //glm::mat4x4 localTransform;
    //glm::mat4x4 worldTransform;
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


/* ============================================================================= */

class FileListener : public Observer {
public:
    bool fileChanged{ false };
    virtual void notify() override;
    void reset();
};

/* ============================================================================= */


struct Scene {
    Primitives primitives; // unique ptr?
    Camera cameraEye;
    //OrderedAssets bufferViews; // why?
    //OrderedAssets textureViews; // why?
    //OrderedAssets imageViews; // why?
    std::map <void*, Material> materials;
    //GLuint* docDataBuffer; // why?
    SceneLights sceneLights;

    FileListener fileListener;
    std::unique_ptr<Drawable> skySphere; // generic list of ... ?
    std::unique_ptr<Drawable> cloudCube; // generic list of ... ?
    std::unique_ptr<Drawable> lightModel; // generic list of ... ?

    void populateScene(GUI& gui, WindowInfo& windowConfig);
    Scene(GUI& gui, WindowInfo& windowConfig);
    ~Scene();
    AkDoc* loadScene(std::string scenePath, std::string sceneName);
    void allocAll(AkDoc* doc);
    GLuint* parseBuffors();
    AkCamera* loadCamera(AkDoc* doc);
    void draw();
    void clear();
    bool fileChanged();
};
