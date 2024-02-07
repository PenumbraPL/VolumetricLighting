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
#include "Debug.h"
#include "Models.h"

#define PATH "./res/models/DamagedHelmet/"
#define FILE_NAME "DamagedHelmet.gltf"


auto bufferLogger = std::make_shared <debug::BufferLogger>();
auto fileLogger = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/basic-log.txt", true);
auto logger = spdlog::logger("multi_sink", {bufferLogger, fileLogger});
std::vector<std::string> directory;


struct WindowInfo 
windowConfig = {
    1900,
    1000,
    "GLTF Viewer",
    0, 2.f, 0., 0.
};

std::vector<PointLight> lightsData;
std::map <void*, unsigned int> bufferViews;
std::map <void*, unsigned int> textureViews;
std::map <void*, unsigned int> imageViews;
std::vector<Primitive> primitives;
std::vector<Light> lights;
//std::vector<Camera> cameras;
const char* modelPath = PATH;

ConfigContext panelConfig{
    500.f, .001f, 50, 0, 0, 0, 0, 0, 50, 0, 0, false, false,
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.4f, 0.7f, 0.0f, 0.5f },
    { 0.0f, 0.0f, 0.0f },
    0.1f, 0.5f, 0.5f, 0.f,
    &directory,
    std::string(""),
    0, lightsData.data(), lightsData.size()
};
// what if lightsData is empty?

namespace fs = std::filesystem;


/* ============================================================================= */

void init(GLFWwindow** windowPtr, ImGuiIO& io)
{
    logger.info("========== Initialization started ============================================\n");
    if (!glfwInit()) {
        logger.error("========== [GLFW]: Initialization failed =====================================\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        logger.warn("========== [GLFW]: Terminated ================================================\n");
        logger.error("========== [GLFW]: Window initialization failed ==============================\n");
        exit(EXIT_FAILURE);
    }
    *windowPtr = window;

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
        logger.info("========== [GLFW]: Debug context initialize successful =======================\n");
        std::vector<DEBUGPROC> callbacks;
        debug::fill_callback_list(callbacks);
        debug::debug_init(callbacks);
    }
    else {
        logger.warn("========== [GLFW]: Debug context initialize unsuccessful =====================\n");
    }
}

/* ============================================================================= */


int main(void)
{
    GLFWwindow* window;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    init(&window, io);


    /* ================================================ */

    //ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    AkDoc* doc;
    AkVisualScene* scene;

    std::string scenePath = PATH;
    scenePath += FILE_NAME;
    if (ak_load(&doc, scenePath.c_str(), NULL) != AK_OK) {
        logger.error("Document couldn't be loaded\n");
        exit(EXIT_FAILURE);
    }
    else {
        logger.info(print_coord_system(doc->coordSys));
        logger.info(print_doc_information(doc->inf, doc->unit));
        logger.info("==============================================================================\n");
    }
    

    AkCamera* camera = nullptr;
    glm::mat4 View;
    glm::mat4 Projection;
    if (doc->scene.visualScene) {
        scene = (AkVisualScene*)ak_instanceObject(doc->scene.visualScene);
        logger.info("=========================== Visual Scene loaded ==================================================\n");
        if (scene->name) {
            std::string sceneInfo = "======================== Scene name: ";
            sceneInfo += scene->name;
            sceneInfo += "========================\n";
            logger.info(sceneInfo);
        }
        float cameraView[16];
        float cameraProjection[16];

        ak_firstCamera(doc, &camera, cameraView, cameraProjection);
        if (camera) {
            View = glm::make_mat4x4(cameraView);
            Projection = glm::make_mat4x4(cameraProjection);
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

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

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

    init_lights(lightsData);
    int lightsBufferSize = (int) sizeof(PointLight) * lightsData.size();
    unsigned int lightDataSize = (unsigned int) lightsData.size();

    glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);
    glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lightsData.data());


    logger.info("===================== Main loop ==============================================\n");
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glEnable(GL_DEPTH_TEST); 
        //glEnable(GL_BLEND);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0., 1., 1., 1.);


        glm::vec3 eye = panelConfig.getView();
        glm::mat4 LookAt = panelConfig.getLookAt();
        if (!camera) Projection = panelConfig.getProjection(width, height);
        panelConfig.lightsData = lightsData.data();
        panelConfig.lightsSize = lightsData.size();

        env.draw(width, height, Projection, camera);

        glm::vec3 translate = panelConfig.getTranslate();
        glm::vec3 rotate = panelConfig.getRotate();
        PointLight newLight = panelConfig.getLight();
        if (compare_lights(lightsData.data()[panelConfig.lightId], newLight)) {
            lightsData.data()[panelConfig.lightId] = newLight;
            LightsList* ptr = (LightsList*)glMapNamedBuffer(lightsBuffer, GL_WRITE_ONLY);
            memcpy_s((void*)&ptr->list[panelConfig.lightId], sizeof(PointLight), &newLight, sizeof(PointLight));
            glUnmapNamedBuffer(lightsBuffer);
            panelConfig.updateLight();
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
    
    /* ======================================================== */

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
    //

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    logger.info("========== [GLFW]: Terminated ================================================\n");
    logger.info("===================== Exit succeeded =========================================\n");
    return 0;
}
