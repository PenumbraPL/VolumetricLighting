#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Light.h"
#include "pch.h"


struct Observer {
    virtual void notify() {};
};

template <typename T, int N = 1>
struct Object {
    Object(T data): data{data} {}
    Object(std::initializer_list<T> data) {
        std::size_t minSize = data.size() > N ? N : data.size();
        int i = 0;
        for (auto& d : data) {
            this->data[i++] = d;
            if (i >= minSize) break;
        }
    }
    ~Object() {
        observers.clear();
    }
    void subscribe(Observer& observer) {
        observers.push_back(&observer);
        observer.notify();
    }

    T& operator[] (int i) {
        return data.data()[i];
    }
    operator T () const {
        return data.data()[0];
    }
    T& get() {
        return *data.data();
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
    std::array<T, N> data;
    std::vector<Observer*> observers;
};


class GUI {
public:
    Object<std::string> selectedSceneFile;
    Object<unsigned int> lightId = 0;
    Object<std::vector<PointLight>*> lightsData{ nullptr };
    bool workSpaceFocused = false;
    std::vector<std::string> directory;

    Object<float> zFar{ 500.f };
    Object<float> zNear{ .001f };
    Object<int> fov{ 50 };
    Object<int> xTranslate{ 0 };
    Object<int> yTranslate{ 0 };
    Object<int> zTranslate{ 0 };
    Object<int> xRotate{ 0 };
    Object<int> yRotate{ 0 };
    Object<float> viewDistance{ 50 };
    Object<float> viewPhi{ 0 };
    Object<float> viewTheta{ 0 };
    Object<float, 4> lightAmbient = {0.4f, 0.7f, 0.0f, 0.5f};
    Object<float, 4> lightDiffuse = { 0.4f, 0.7f, 0.0f, 0.5f };
    Object<float, 4> lightSpecular = { 0.4f, 0.7f, 0.0f, 0.5f };
    Object<float, 3> position = { 0.0f, 0.0f, 0.0f };
    Object<float> c{ 0.1f };
    Object<float> l{ 0.5f };
    Object<float> q{ 0.5f };
    Object<float> g{ 0.f };


    void subscribeToView(Observer& observer);
    void unsubscribeToView(Observer& observer);
    void subscribeToEye(Observer& observer);
    void subscribeToLight(Observer& observer);
    GUI(std::string selectedSceneFile);
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