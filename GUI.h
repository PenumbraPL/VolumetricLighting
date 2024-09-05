#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Light.h"
#include "pch.h"


struct Observer {
    virtual void notify();
};

template <typename T>
struct Object {
    Object(T data): data{data} {}
    void subscribe(Observer& observer) {
        observers.push_back(observer);
    }
    operator T() const {
        return data;
    }
    void unsubscribe(Observer& observer) {
        const auto pos{ std::find(std::begin(observers), std::end(observers), observer) };
        if (pos != std::end(observers)) {
            observers.erase(pos);
        }
    }
    void notifyAll() {
        for (auto& observer : observers) {
            observer.notify();
        }
    }
    T data;
    std::vector<Observer> observers;
};


class GUI {
public:
    GUI(std::string fileSelection) : fileSelection{fileSelection} {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = &getIO();
    };
    void deleteImGui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    ImGuiIO& getIO() {
        return ImGui::GetIO();
    }
    void draw() {
        //ImGuiIO& io = getIO();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawLeftPanel(*io);
        drawRightPanel(*io);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void chooseGlfwImpl(GLFWwindow* window) {
        //ImGuiIO& io = getIO();
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io->ConfigWindowsMoveFromTitleBarOnly = true;

        ImGui::StyleColorsDark();

        const char* GLSLVersion{ "#version 450" };
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(GLSLVersion);

    }
    void drawLeftPanel(ImGuiIO& io);
    void drawRightPanel(ImGuiIO& io);
    ~GUI() {
        //deleteImGui();
    };

    std::string fileSelection;
    unsigned int lightId = 0;
    std::vector<PointLight>* lightsData = nullptr;
    bool focused = false;
    std::vector<std::string> directory;

    float zFar{ 500.f };
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
    float lightAmbient[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightDiffuse[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightSpecular[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float c = 0.1f;
    float l = 0.5f;
    float q = 0.5f;
    float g = 0.f;




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

private:
    ImGuiIO* io{ nullptr };
};