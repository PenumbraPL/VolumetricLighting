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


GUI myGui{ "./res/models/GLTFTest/gltfTest.gltf" };


int main()
{
    GLFWwindow* window{ initContext() };
    myGui.chooseGlfwImpl(window);

    Matrix transform;
    std::unique_ptr<Drawable> lightModel{ LightFactory().createDrawable() };
    std::unique_ptr<Drawable> skySphere{ EnvironmentFactory().createDrawable() };
    std::unique_ptr<Drawable> cloudCube{ CloudFactory().createDrawable() };

    Scene scenes;
    myGui.lightsData = &scenes.sceneLights.lights;
    scenes.cameraEye.Projection = myGui.getProjection(windowConfig.width, windowConfig.height);

    transform.setProjection(windowConfig.width, windowConfig.height);
    myGui.subscribeToView(transform);

    myGui.subscribeToEye(scenes.cameraEye);
    
    FileListener fileListener;
    myGui.selectedSceneFile.subscribe(fileListener);
    myGui.lightsData.subscribe(scenes.sceneLights);
    myGui.g.subscribe(*((Cloud*) cloudCube.get()));
    
    skySphere->Proj = scenes.cameraEye.Projection;
    skySphere->transforms = &transform;

    cloudCube->Proj = scenes.cameraEye.Projection;
    cloudCube->transforms = &transform;

    /* ================================================ */
    do {
        scenes.loadScene(myGui.getModelPath(), myGui.getModelName());

        std::vector<Drawable>& primitives{ scenes.primitives };
        for (auto& primitive : primitives) {
            ShadersSources defaultModel;
            defaultModel[VERTEX] = { "res/shaders/standard_vec.glsl" };
            defaultModel[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
            primitive.createPipeline(defaultModel);
            primitive.getLocation({ {
                {"MV", "PRJ"},
                {"camera", "_metalic", "_roughness", "_albedo_color", "ao_color", "_is_tex_bound"}
            } });
            primitive.Proj = scenes.cameraEye.Projection;
            primitive.transforms = &transform;
        }
        for (auto& light : scenes.sceneLights.lights) {
            glm::mat4 transforms = ((Light*)lightModel.get())->calcMV(light, scenes);
            Matrix lightTransform;
            lightTransform.MV = transforms;
            lightModel->transforms = new Matrix(lightTransform);
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
           
            skySphere->draw(scenes);

            
            for (auto& primitive : primitives) {
                primitive.draw(scenes);
            }
       
            for (auto& light : scenes.sceneLights.lights) {
                lightModel->transforms->MV = ((Light*) lightModel.get())->calcMV(light, scenes);
                lightModel->draw(scenes);
            }

            cloudCube->draw(scenes);

            myGui.draw();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        logger.info("===================== End of loop ==============================================");


        glBindProgramPipeline(0);

        for (auto& primitive : primitives) primitive.deleteTexturesAndSamplers();
        for (auto& primitive : primitives) primitive.deletePipeline();


        glDeleteBuffers((GLsizei) scenes.bufferViews.size(), scenes.docDataBuffer);
        if(scenes.docDataBuffer) free(scenes.docDataBuffer);
        
        for (auto& primitive : primitives) {
            for (int i = VERTEX; i <= GEOMETRY; i++) {
                if(primitive.bindingLocationIndecies[i]) free(primitive.bindingLocationIndecies[i]);
            }
        }

    } while (!glfwWindowShouldClose(window));

    skySphere->deletePipeline();
    cloudCube->deletePipeline();
    lightModel->deletePipeline();

    myGui.deleteImGui();
    glfwTerminate();
    logger.info("========== [GLFW]: Terminated ================================================");
    logger.info("===================== Exit succeeded =========================================");
    return 0;
}
