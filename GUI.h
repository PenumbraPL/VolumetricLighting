#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Light.h"
#include "pch.h"


struct ConfigContext{
    std::string fileSelection;

    float zFar = 500.f;
    float zNear = .001f;
    int fov = 50;
    int xTranslate = 0;
    int yTranslate = 0;
    int zTranslate = 0;
    int xRotate = 0;
    int yRotate = 0;
    float viewDistance = 50;
    float viewPhi = 0;
    float viewTheta = 0;
     
    bool focused = false;

    float lightAmbient[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightDiffuse[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightSpecular[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float c = 0.1f;
    float l = 0.5f;
    float q = 0.5f;
    float g = 0.f;

    std::vector<std::string> directory;
    unsigned int lightId = 0;
    std::vector<PointLight>* lightsData = nullptr;
     
    PointLight* getLightsData();
    std::size_t getLightsSize();
    PointLight getLight();
    void updateLight();
    glm::vec3 getTranslate();
    glm::vec3 getRotate();
    glm::vec3 getView();
    glm::mat4 getLookAt();
    glm::mat4 getProjection(int width, int height);
    std::string getModelPath();
    std::string getModelName();
};


class GUI {
public:
    GUI() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    };
    void deleteImGui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    ImGuiIO& getIO() {
        return ImGui::GetIO();
    }
    void draw(ConfigContext& panelConfig) {
        ImGuiIO& io = getIO();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(io, panelConfig);
        drawRightPanel(io, panelConfig);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void drawLeftPanel(ImGuiIO& io, ConfigContext& config);
    void drawRightPanel(ImGuiIO& io, ConfigContext& config);
    ~GUI() {
        //deleteImGui();
    };
private:
    //ImGuiIO& io{  };
};