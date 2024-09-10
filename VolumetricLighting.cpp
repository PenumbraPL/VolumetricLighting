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


/* ============================================================================= */


class FileListener : public Observer{
public:
    bool fileChanged{ false };
    virtual void notify() override {
        fileChanged = true;
        logger.info("fileChanged");
    }

    void reset() {
        fileChanged = false;
    }
};


/* ============================================================================= */


GLFWwindow* initContext()
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

    ak_imageInitLoader(imageLoadFromFile, imageLoadFromMemory, imageFlipVerticallyOnLoad);

    return window;
}

/* ============================================================================= */


GUI myGui{ "./res/models/Latern/Lantern.gltf" };


int main()
{
    GLFWwindow* window{ initContext() };
    myGui.chooseGlfwImpl(window);

    Scene scenes{ myGui, windowConfig };

    FileListener fileListener;
    myGui.selectedSceneFile.subscribe(fileListener);

    /* ================================================ */
    do {
        scenes.loadScene(myGui.getModelPath(), myGui.getModelName());

        for (auto& primitive : scenes.primitives) {
            ShadersSources defaultModel;
            defaultModel[VERTEX] = { "res/shaders/standard_vec.glsl" };
            defaultModel[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
            primitive.createPipeline(defaultModel);
            primitive.getLocation({ {
                {"MV", "PRJ"},
                {"camera", "_metalic", "_roughness", "_albedo_color", "ao_color", "_is_tex_bound"}
            } });
            myGui.subscribeToView(primitive.transforms);
        }
        for (auto& light : scenes.sceneLights.lights) {
            Matrix lightTransform;
            lightTransform.MV = ((Light*)scenes.lightModel.get())->calcMV(light, scenes);
            scenes.lightModel->transforms = Matrix(lightTransform);
        }
        //glDepthRange(myGui.near_plane, myGui.far_plane);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fileListener.reset();

        logger.info("===================== Main loop ==============================================");
        while (!glfwWindowShouldClose(window) && !fileListener.fileChanged) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glEnable(GL_DEPTH_TEST);
            //glEnable(GL_BLEND);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0., 1., 1., 1.);

            scenes.draw();

            myGui.draw();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        logger.info("===================== End of loop ==============================================");
        
        for (auto& primitive : scenes.primitives) {
            myGui.unsubscribeToView(primitive.transforms);
        }

        glBindProgramPipeline(0);
        scenes.clear();
    } while (!glfwWindowShouldClose(window));

    myGui.deleteImGui();
    glfwTerminate();
    logger.info("========== [GLFW]: Terminated ================================================");
    logger.info("===================== Exit succeeded =========================================");
    return 0;
}
