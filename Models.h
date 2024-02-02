#pragma once
#include "GLFW/glfw3.h"
#include "pch.h";
#include "GUI.h"

extern ConfigContext panel_config;

enum TextureType {
    AMBIENT,
    EMISIVE,
    DIFFUSE,
    SPECULAR,
    SP_GLOSSINESS,
    MAT_ROUGH,
    ALBEDO,
    SP_DIFFUSE,
    TT_SIZE,
    SKYBOX
};

enum ShaderTypes {
    VERTEX,
    FRAGMENT,
    TESS_CTR,
    TESS_EV,
    GEOMETRY
};


void formatAttribute(GLint attr_location, AkAccessor* acc);
char* read_file(const char* file_name);
void*
imageLoadFromFile(const char* __restrict path,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components);

void*
imageLoadFromMemory(const char* __restrict data,
    size_t                  len,
    int* __restrict width,
    int* __restrict height,
    int* __restrict components);

void
imageFlipVerticallyOnLoad(bool flip);



struct Environment {
    glm::mat4x4 transform = glm::mat4x4(0.);
    glm::mat4x4 w_transform = glm::mat4x4(0.);

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint pipeline;
    GLuint skybox;
    GLuint env_sampler;
    glm::mat4 MVP;
    GLuint* buffer;
    GLuint mvp_location;

    void createPipeline();
    void deletePipeline();
    void loadMesh();
    void draw(int width, int height, glm::mat4 Proj, AkCamera* camera);
};





struct Primitive {
    float* transform;
    float* w_transform;

    uint32_t* ind;
    unsigned int ind_size;
    GLuint* programs;
    GLuint pipeline;

    GLuint* textures = nullptr;
    GLuint* tex_type = nullptr;
    GLuint* samplers = nullptr;

    AkAccessor* wgs;
    AkAccessor* jts;
    AkAccessor* pos;
    AkAccessor* tex;
    AkAccessor* nor;
    AkAccessor* col;
    AkAccessor* tan;

    GLuint mvp_location, prj_location, camera_location, g_location, is_tex_location,
        dir_location, vpos_location, norm_location, tex_location;


    void getLocation();
    void draw(GLuint& lights_buffer, std::map <void*, unsigned int>& bufferViews, GLuint* buffers,
        glm::vec3& eye,
        glm::mat4& LookAt, glm::mat4& Projection,
        glm::vec3& translate, glm::vec3& rotate);
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
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction = glm::vec4(0, 0, 0, 0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float intensity = 1.0;

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* nor = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint pipeline;
    GLuint mvp_location;
    GLuint buffer;

    void createPipeline();
    void deletePipeline();
    void loadMesh();
    void drawLight(int width, int height, glm::mat4 Proj, AkCamera* camera);
};


struct Camera {
    glm::mat4x4 transform;
    glm::mat4x4 w_transform;
    glm::vec4 direction;
    float zNear;
    float zFar;
    int fov;
};


struct Cloud {
    glm::mat4x4 transform = glm::mat4x4(0.);
    glm::mat4x4 w_transform = glm::mat4x4(0.);

    int width = 0;
    int height = 0;

    uint32_t* ind = nullptr;
    unsigned int ind_size;
    AkInput* pos = nullptr;
    AkInput* tex = nullptr;

    GLuint vertex_program;
    GLuint fragment_program;
    GLuint pipeline;

    glm::mat4 MVP;
    GLuint* buffer;
    GLuint mvp_location;
    GLuint prj_location;

    GLuint depth_buffer;
    unsigned int zero = 0;


    void createPipeline(int width, int height);
    void deletePipeline();
    void loadMesh();
    void draw(int width, int height, glm::mat4 Proj, AkCamera* camera, float& g, GLuint lightbuffer);
};