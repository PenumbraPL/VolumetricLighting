// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "pch.h"

//#define GLEW_STATIC
#include "GUI.h"
#include "Draw.h"
#include "IO.h"
#include "Tools.h"
#include "Light.h"

#define PATH "./res/models/DamagedHelmet/"
#define FILE_NAME "DamagedHelmet.gltf"
#include "Debug.h"
#include "Models.h"


ConfigContext panelConfig{
    500.f, .001f, 50, 0, 0, 0, 0, 0, 50, 0, 0, false, false,
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.0f, 0.0f, 0.0f },
    0.1f, 0.5f, 0.5f
};
auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
struct WindowInfo {
    int width;
    int height;
    const char* title;
    int cursor_mode;
    int imgui;
    int mbutton;
} windowConfig = {
    1900,
    1000,
    "GLTF Viewer",
    0, 0, 0
};

double xpos, ypos;
float mouseSpeed = 2.f;
std::vector<glm::mat4x4*> mats;
glm::vec4 cam;
std::vector<PointLight> lightsData;
std::map <void*, unsigned int> bufferViews;
std::map <void*, unsigned int> textureViews;
std::map <void*, unsigned int> imageViews;
std::vector<Primitive> primitives;
const char* model_path = PATH;
std::vector<Light> lights;
std::vector<Camera> cameras;

namespace fs = std::filesystem;


/* ============================================================================= */



int main(void)
{
    logger->info("========== Initialization started ============================================\n");
    if (!glfwInit()) {
        logger->error("========== [GLFW]: Initialization failed =====================================\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        logger->warn("========== [GLFW]: Terminated ================================================\n");
        logger->error("========== [GLFW]: Window initialization failed ==============================\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    
    glfwSetKeyCallback(window, control::key_callback);
    glfwSetErrorCallback(debug::glew_callback);
    glfwSetScrollCallback(window, control::scroll_callback);
    glfwSetMouseButtonCallback(window, control::mouse_button_callback);
    glfwSetCursorPosCallback(window, control::cursor_position_callback);
    glfwSetWindowFocusCallback(window, control::focus_callback);

    initialize_GLEW();
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 450";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    int flags; 
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        logger->info("========== [GLFW]: Debug context initialize successful =======================\n");
        std::vector<DEBUGPROC> callbacks;
        debug::fill_callback_list(callbacks);
        debug::debug_init(callbacks);
    }  
    else {
        logger->warn("========== [GLFW]: Debug context initialize unsuccessful =====================\n");
    }
    
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* ================================================ */

    //ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    AkDoc* doc;
    AkVisualScene* scene;
    AkCamera* camera = nullptr;

    std::string scene_path = PATH;
    scene_path += FILE_NAME;
    if (ak_load(&doc, scene_path.c_str(), NULL) != AK_OK) {
        logger->error("Document couldn't be loaded\n");
        exit(EXIT_FAILURE);
    }
    else {
        logger->info(print_coord_system(doc->coordSys));
        logger->info(print_doc_information(doc->inf, doc->unit));
        logger->info("==============================================================================\n");
    }

    float* camera_mat = (float*)calloc(16, sizeof(float));
    float* camera_proj = (float*)calloc(16, sizeof(float));
    glm::mat4 Projection;
    glm::mat4 Camera;

    if (doc->scene.visualScene) {
        scene = (AkVisualScene*) ak_instanceObject(doc->scene.visualScene);
        logger->info("=============== Visual Scene loaded ====");
        if (scene->name) {
            logger->info("Scene name: ");
            logger->info(scene->name);
            logger->info("============\n");
        }

        ak_firstCamera(doc, &camera, camera_mat, camera_proj);
        if (camera) {
            Projection = glm::make_mat4x4(camera_proj);
            Camera = glm::make_mat4x4(camera_mat);
            for (int i = 0; i < 16; i++) {
                std::cout << camera_mat[i] << ", ";
                if (i % 4 == 3) std::cout << std::endl;
            }
        } 
        else if (scene->cameras) {
            if (scene->cameras->first) {
                camera = (AkCamera*)ak_instanceObject(scene->cameras->first->instance);
            }
        }
        if (camera) std::cout << "Camera name: " << camera->name << std::endl;


        AkNode* node = ak_instanceObjectNode(scene->node);
        proccess_node(node); // pointer to pointer?
    }

    if (camera_mat) free(camera_mat);
    if (camera_proj) free(camera_proj);

    // choose shaders from compiled set
    //primitives[0].program = &program;


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
        } 
        while (b);
        for (auto& u : bufferViews) {
            u.second = j++;
        }
    }


    /* ======================================================== */

   
    GLuint* docDataBuffer = (GLuint*) calloc(bufferViews.size(), sizeof(GLuint));
    glCreateBuffers((GLsizei) bufferViews.size(), docDataBuffer);
    for (auto &buffer : bufferViews) {
        unsigned int i = bufferViews[buffer.first];
        glNamedBufferData(docDataBuffer[i], ((AkBuffer*) buffer.first)->length, ((AkBuffer*) buffer.first)->data, GL_STATIC_DRAW);
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    for (auto& p : primitives) p.getLocation();

    for (auto& l : lights)    l.createPipeline();

    Environment env;
    env.loadMesh();
    env.createPipeline();
    
    Cloud cld;
    cld.loadMesh();
    cld.createPipeline(width, height);


    //glDepthRange(panelConfig.near_plane, panelConfig.far_plane);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    GLuint lightsBuffer;
    glGenBuffers(1, &lightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);

    init_lights();
    int lightsBufferSize = (int) sizeof(PointLight) * lightsData.size();
    unsigned int lightDataSize = (unsigned int) lightsData.size();

    glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lightsData.data());


    logger->info("===================== Main loop ==============================================\n");
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glEnable(GL_DEPTH_TEST); 
        //glEnable(GL_BLEND);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0., 1., 1., 1.);


        glm::vec3 eye = panelConfig.polar();
        glm::mat4 LookAt = panelConfig.preperLookAt();
        if (!camera) Projection = glm::perspectiveFov((float)3.14 * panelConfig.fov / 180, (float)width, (float)height, panelConfig.near_plane, panelConfig.far_plane);
        

        env.draw(width, height, Projection, camera);

        glm::vec3 translate = panelConfig.getTranslate();
        glm::vec3 rotate = panelConfig.getRotate();
        PointLight newLight = getLight(panelConfig);
        if (compare_lights(lightsData.data()[0], newLight)) {
            lightsData.data()[0] = newLight;
            LightsList* ptr = (LightsList*)glMapNamedBuffer(lightsBuffer, GL_WRITE_ONLY);
            memcpy_s((void*)&ptr->list[0], sizeof(PointLight), &newLight, sizeof(PointLight));
            glUnmapNamedBuffer(lightsBuffer);
        }


        for (auto& p : primitives) {
            p.draw(lightsBuffer, bufferViews, docDataBuffer,
                eye, LookAt, Projection, translate, rotate);
        }
        cld.draw(width, height, Projection, camera, panelConfig.g, lightsBuffer);
        for (auto& l : lights)     l.drawLight(width, height, Projection, camera);



        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io, panelConfig);
        drawRightPanel(io, panelConfig);
        
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glBindProgramPipeline(0);

    env.deletePipeline();
    cld.deletePipeline();

    for (auto& p : primitives) p.deleteTransforms();
    for (auto& p : primitives) p.deleteTexturesAndSamplers();
    for (auto& p : primitives) p.deletePipeline();
    for (auto& l : lights)     l.deletePipeline();


    glDeleteBuffers((GLsizei) bufferViews.size(), docDataBuffer); 
    free(docDataBuffer);
    glDeleteVertexArrays(1, &vao);


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    logger->info("========== [GLFW]: Terminated ================================================\n");
    logger->info("===================== Exit succeeded =========================================\n");
    return 0;
}
