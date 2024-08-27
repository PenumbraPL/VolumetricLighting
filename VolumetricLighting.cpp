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
ConfigContext panelConfig = { "./res/models/Sample2/scene.gltf" };



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

    const char* glsl_version{ "#version 450" };
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
  
    

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        logger.info("========== [GLFW]: Debug context initialize successful =======================");
        std::vector<DEBUGPROC> callbacks;
        debug::fill_callback_list(callbacks);
        debug::debug_init(callbacks);
    }
    else {
        logger.warn("========== [GLFW]: Debug context initialize unsuccessful =====================");
    }
}

void draw_imgui(ImGuiIO& io) 
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

    Light lightModel;
    lightModel.loadMesh();
    std::string lightsPipeline[5] = { "res/shaders/lamp_vec.glsl", "res/shaders/lamp_frag.glsl" };
    lightModel.createPipeline(lightsPipeline);
    std::vector<const char*> uniformNames[5] =
    {{"MVP", "PRJ"}, {"G", "camera"}, {}, {}, {} };
    lightModel.getLocation(uniformNames);


    Environment env;
    env.loadMesh();
    std::string envPipeline[5] = { "res/shaders/environment_vec.glsl", "res/shaders/environment_frag.glsl" };
    env.createPipeline(envPipeline);
    std::vector<const char*> envUniformNames[5] = {{"MVP"}, {}, {}, {}, {}};
    env.getLocation(envUniformNames);


    Cloud cld;
    cld.loadMesh();
    std::string cloudPipeline[5] = { "res/shaders/depth_ver.glsl", "res/shaders/depth_frag.glsl" };
    cld.createPipeline(cloudPipeline);
    std::vector<const char*> cldUniformNames[5] =
    { {"MVP", "PRJ"}, {"G", "camera"}, {}, {}, {} };
    cld.getLocation(cldUniformNames);

    Scene scenes;
    std::vector<PointLight>& lightsData= scenes.lights;
    panelConfig.lightsData = &scenes.lights;


    /* ================================================ */
    do{
        ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);        
        AkDoc* doc = scenes.loadScene(panelConfig.getModelPath(), panelConfig.getModelName());
        AkCamera* camera = scenes.camera(doc);
        glm::mat4& View = scenes.cameraEye.View;
        glm::mat4& Projection = scenes.cameraEye.Projection;
        scenes.allocAll(doc);

        std::vector<Drawable>& primitives = scenes.primitives;
        std::string pipeline[5];
        pipeline[VERTEX] = { "res/shaders/standard_vec.glsl" };
        pipeline[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
        for (auto& primitive : primitives) primitive.createPipeline(pipeline);
        std::map <void*, unsigned int>& bufferViews = scenes.bufferViews;
        GLuint* docDataBuffer = scenes.parseBuffors();


        std::vector<const char*> uniformNames[5] = {
            {"MVP", "PRJ"},
            {"camera", "_metalic", "_roughness", "_albedo_color", "ao_color", "_is_tex_bound"},
            {}, {}, {}
        };
        for (auto& primitive : primitives) primitive.getLocation(uniformNames);

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

            env.draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);
            

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
                lightModel.draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);
            }
            /* ===================== */

            View =
                glm::rotate(
                    glm::rotate(
                        glm::translate(cld.localTransform , translate)
                    , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));

            MVP = LookAt * View * Model;
            cld.g = panelConfig.g;
            cld.draw(lightsBuffer, bufferViews, docDataBuffer, eye, MVP, Projection);

            draw_imgui(io);
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

    env.deletePipeline();
    cld.deletePipeline();
    lightModel.deletePipeline();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    logger.info("========== [GLFW]: Terminated ================================================");
    logger.info("===================== Exit succeeded =========================================");
    return 0;
}
