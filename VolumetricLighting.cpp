// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "Tools.h"
#include "GUI.h"
#include "IO.h"
#include "Light.h"
#include "Models.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "pch.h"

#include "Debug.h"

namespace fs = std::filesystem;


auto bufferLogger{ std::make_shared <debug::BufferLogger>() };
auto fileLogger{ std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/basic-log.txt", true) };
auto consoleLogger{ std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>() };
auto logger{ spdlog::logger("multi_sink", {bufferLogger, fileLogger, consoleLogger}) };


WindowInfo windowConfig = { 1900, 1000, "GLTF Viewer" };
ConfigContext panelConfig = { "./res/models/GLTFTest/gltfTest.gltf" };



/* ============================================================================= */




void init(GLFWwindow** windowPtr, ImGuiIO& io)
{
    logger.set_pattern("%^[%L][%s:%#]%$  %v ");
    logger.info("========== Initialization started ============================================");
    if (!glfwInit()) {
        logger.error("========== [GLFW]: Initialization failed =====================================");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);


    GLFWwindow* window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        logger.warn("========== [GLFW]: Terminated ================================================");
        logger.error("========== [GLFW]: Window initialization failed ==============================");
        exit(EXIT_FAILURE);
    }
    *windowPtr = window;

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSetKeyCallback(window, control::keyCallback);
    glfwSetErrorCallback(debug::glewCallback);
    glfwSetScrollCallback(window, control::scrollCallback);
    glfwSetMouseButtonCallback(window, control::mouseButtonCallback);
    glfwSetCursorPosCallback(window, control::cursorPositionCallback);
    glfwSetWindowFocusCallback(window, control::focusCallback);

    initializeGLEW();


    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    
    ImGui::StyleColorsDark();

    const char* GLSLVersion{ "#version 450" };
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSLVersion);
  
    

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        logger.info("========== [GLFW]: Debug context initialize successful =======================");
        std::vector<DEBUGPROC> callbacks;
        debug::fillCallbackList(callbacks);
        debug::debugInit(callbacks);
    }
    else {
        logger.warn("========== [GLFW]: Debug context initialize unsuccessful =====================");
    }
}

void drawImgui(ImGuiIO& io) 
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawLeftPanel(io, panelConfig);
    drawRightPanel(io, panelConfig);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/* ============================================================================= */


int main()
{
    GLFWwindow* window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    init(&window, io);

    std::unique_ptr<Drawable> lightModel = LightFactory().createDrawable();
    std::unique_ptr<Drawable> env = EnvironmentFactory().createDrawable();
    std::unique_ptr<Drawable> cld = CloudFactory().createDrawable();

    Scene scenes;
    std::vector<PointLight>& lightsData= scenes.lights;
    panelConfig.lightsData = &scenes.lights;


    /* ================================================ */
    do {
        ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);
        AkDoc* doc = scenes.loadScene(panelConfig.getModelPath(), panelConfig.getModelName());
        AkCamera* camera = scenes.camera(doc);
        glm::mat4& View = scenes.cameraEye.View;
        glm::mat4& Projection = scenes.cameraEye.Projection;
        scenes.allocAll(doc);

        std::vector<Drawable>& primitives = scenes.primitives;
        ShadersSources defaultModel;
        defaultModel[VERTEX] = { "res/shaders/standard_vec.glsl" };
        defaultModel[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
        for (auto& primitive : primitives) primitive.createPipeline(defaultModel);
        std::map <void*, unsigned int>& bufferViews = scenes.bufferViews;
        GLuint* docDataBuffer = scenes.parseBuffors();

        for (auto& primitive : primitives) primitive.getLocation({{
            {"MVP", "PRJ"},
            {"camera", "_metalic", "_roughness", "_albedo_color", "ao_color", "_is_tex_bound"}
            }
        });

        //glDepthRange(panelConfig.near_plane, panelConfig.far_plane);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        GLuint lightsBuffer;
        glGenBuffers(1, &lightsBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsBuffer);

        scenes.initLights();
        int lightsBufferSize = (int)sizeof(PointLight) * lightsData.size();
        unsigned int lightDataSize = (unsigned int)lightsData.size();

        glNamedBufferData(lightsBuffer, sizeof(LightsList) + lightsBufferSize, NULL, GL_DYNAMIC_DRAW);
        glNamedBufferSubData(lightsBuffer, offsetof(LightsList, list), lightsBufferSize, lightsData.data());
        glNamedBufferSubData(lightsBuffer, offsetof(LightsList, size), sizeof(unsigned int), &lightDataSize);

        std::string fileSelected = panelConfig.fileSelection;

        logger.info("===================== Main loop ==============================================");
        while (!glfwWindowShouldClose(window)) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glEnable(GL_DEPTH_TEST);
            //glEnable(GL_BLEND);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0., 1., 1., 1.);

            if (!camera) Projection = panelConfig.getProjection(width, height);
            
            glm::vec3 eye = panelConfig.getView();
            glm::mat4 LookAt = panelConfig.getLookAt();
            glm::vec3 translate = panelConfig.getTranslate();
            glm::vec3 rotate = panelConfig.getRotate();
            glm::mat4 localTransform = glm::mat4(1.);

            glm::mat4 View =
                glm::rotate(
                    glm::rotate(
                        glm::translate(localTransform,
                            translate)
                        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 MVP = Projection * LookAt * View;

            env->draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);
            

            MVP = LookAt * View;

            for (auto& primitive : primitives) {
                primitive.draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);
            }
       
            

            scenes.updateLights(lightsBuffer, lightDataSize, panelConfig);
            for (auto& light : lightsData) {
                glm::mat4x4 View =
                    glm::rotate(
                         glm::rotate(
                            glm::translate(localTransform, light.position)
                        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(.2f));

                glm::mat4 MVP = Projection * LookAt * View * Model;
                lightModel->draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);
            }
            /* ===================== */

            View =
                glm::rotate(
                    glm::rotate(
                        glm::translate(cld->localTransform , translate)
                    , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));

            MVP = LookAt * View * Model;
            ((Cloud*) cld.get())->g = panelConfig.g;
            cld->draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);

            drawImgui(io);
            glfwSwapBuffers(window);
            glfwPollEvents();

            if (panelConfig.fileSelection != fileSelected) break;
        }

        /* ======================================================== */

        glBindProgramPipeline(0);

        for (auto& primitive : primitives) primitive.deleteTexturesAndSamplers();
        for (auto& primitive : primitives) primitive.deletePipeline();

        glDeleteBuffers((GLsizei)bufferViews.size(), docDataBuffer);
        if(docDataBuffer) free(docDataBuffer);
        
        for (auto& primitive : primitives) {
            for (int i = VERTEX; i <= GEOMETRY; i++) {
                if(primitive.bindingLocationIndecies[i]) free(primitive.bindingLocationIndecies[i]);
            }
        }

    } while (!glfwWindowShouldClose(window));

    env->deletePipeline();
    cld->deletePipeline();
    lightModel->deletePipeline();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    logger.info("========== [GLFW]: Terminated ================================================");
    logger.info("===================== Exit succeeded =========================================");
    return 0;
}
