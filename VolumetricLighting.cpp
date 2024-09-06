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


class Matrix : public Observer {
public:
    Matrix() {}
    Matrix(glm::mat4 localTransform) : localTransform{ localTransform } {}

    void setProjection(int width, int height) {
        //if(!camera)
            Projection = myGui.getProjection(width, height);
        //Projection = Proj;
    }

    void calculateMVP() {
        glm::vec3 eye = myGui.getView();
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

    glm::mat4 localTransform = glm::mat4(1.);
    glm::mat4 Projection = glm::mat4(1.);;
    glm::mat4 MVP = glm::mat4(1.);;
    glm::mat4 MV = glm::mat4(1.);;
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

    std::unique_ptr<Drawable> lightModel{ LightFactory().createDrawable() };
    std::unique_ptr<Drawable> skySphere{ EnvironmentFactory().createDrawable() };
    std::unique_ptr<Drawable> cloudCube{ CloudFactory().createDrawable() };

    Scene scenes;
    myGui.lightsData = &scenes.sceneLights.lights;
    scenes.cameraEye.Projection = myGui.getProjection(windowConfig.width, windowConfig.height);

    Matrix transform;
    transform.setProjection(windowConfig.width, windowConfig.height);
    myGui.subscribeToView(transform);

    Matrix cloudTransform{ cloudCube->localTransform };
    cloudTransform.setProjection(windowConfig.width, windowConfig.height);
    myGui.subscribeToView(cloudTransform);
    myGui.subscribeToEye(scenes.cameraEye);
    
    
    /* ================================================ */
    do {
        AkDoc* doc{ scenes.loadScene(myGui.getModelPath(), myGui.getModelName()) };
        scenes.allocAll(doc);

        std::vector<Drawable>& primitives{ scenes.primitives };
        ShadersSources defaultModel;
        defaultModel[VERTEX] = { "res/shaders/standard_vec.glsl" };
        defaultModel[FRAGMENT] = { "res/shaders/pbr_with_ext_light_frag.glsl" };
        for (auto& primitive : primitives) primitive.createPipeline(defaultModel);
        scenes.parseBuffors();

        for (auto& primitive : primitives) primitive.getLocation({{
            {"MVP", "PRJ"},
            {"camera", "_metalic", "_roughness", "_albedo_color", "ao_color", "_is_tex_bound"}
            }
        });

        //glDepthRange(myGui.near_plane, myGui.far_plane);
        glDepthFunc(GL_LEQUAL);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        std::string fileSelected = myGui.fileSelection;

        logger.info("===================== Main loop ==============================================");
        while (!glfwWindowShouldClose(window)) {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glEnable(GL_DEPTH_TEST);
            //glEnable(GL_BLEND);
            glViewport(0, 0, width, height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0., 1., 1., 1.);

            //glm::mat4 Projection = myGui.getProjection(width, height);
            //glm::vec3 eye = myGui.getView();
            glm::mat4 LookAt = myGui.getLookAt();
            glm::vec3 translate = myGui.getTranslate();
            glm::vec3 rotate = myGui.getRotate();
            glm::mat4 localTransform = glm::mat4(1.);

            skySphere->draw(transform.MVP, scenes);
            
            for (auto& primitive : primitives) {
                primitive.draw(transform.MV, scenes);
            }
       
            scenes.sceneLights.updateLights(myGui);
            for (auto& light : scenes.sceneLights.lights) {
                glm::mat4x4 View =
                    glm::rotate(
                         glm::rotate(
                            glm::translate(localTransform, light.position)
                        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
                    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(.2f));

                glm::mat4 Projection = scenes.cameraEye.Projection;
                glm::mat4 MVP = Projection * LookAt * View * Model;
                lightModel->draw(MVP, scenes);
            }
            /* ===================== */


            ((Cloud*) cloudCube.get())->g = myGui.g;
            cloudCube->draw(transform.MV, scenes);

            myGui.draw();
            glfwSwapBuffers(window);
            glfwPollEvents();

            if (myGui.fileSelection != fileSelected) break;
        }

        /* ======================================================== */

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
