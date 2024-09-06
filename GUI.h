#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Light.h"
#include "pch.h"


struct Observer {
    virtual void notify() {};
};

template <typename T>
struct Object {
    Object(T data): data{data} {}
    void subscribe(Observer& observer) {
        observers.push_back(&observer);
        observer.notify();
    }
    operator T() const {
        return data;
    }
    void unsubscribe(Observer& observer) {
        const auto pos{ std::find(std::begin(observers), std::end(observers), &observer) };
        if (pos != std::end(observers)) {
            observers.erase(pos);
        }
    }
    void notifyAll() {
        for (auto& observer : observers) {
            observer->notify();
        }
    }
    T data;
    std::vector<Observer*> observers;
};


class GUI {
public:
    std::string fileSelection;
    unsigned int lightId = 0;
    std::vector<PointLight>* lightsData = nullptr;
    bool workSpaceFocused = false;
    std::vector<std::string> directory;

    float zFar{ 500.f };
    float zNear = .001f;
    int fov = 50;
    Object<int> xTranslate{ 0 };
    Object<int> yTranslate{ 0 };
    Object<int> zTranslate{ 0 };
    Object<int> xRotate{ 0 };
    Object<int> yRotate{ 0 };
    Object<float> viewDistance{ 50 };
    Object<float> viewPhi{ 0 };
    Object<float> viewTheta{ 0 };
    float lightAmbient[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightDiffuse[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float lightSpecular[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float c = 0.1f;
    float l = 0.5f;
    float q = 0.5f;
    float g = 0.f;


    void subscribeToView(Observer& observer) {
        xTranslate.subscribe(observer);
        yTranslate.subscribe(observer);
        zTranslate.subscribe(observer);
        xRotate.subscribe(observer);
        yRotate.subscribe(observer);
        viewDistance.subscribe(observer);
        viewPhi.subscribe(observer);
        viewTheta.subscribe(observer);
    }
    GUI(std::string fileSelection);
    void deleteImGui();
    ImGuiIO& getIO();
    void draw();
    void chooseGlfwImpl(GLFWwindow* window);
    void drawLeftPanel(ImGuiIO& io);
    void drawRightPanel(ImGuiIO& io);
    ~GUI() {};
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
};