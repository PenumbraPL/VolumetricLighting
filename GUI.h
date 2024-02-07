#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Light.h"
#include "pch.h"

struct ConfigContext{
     float zFar;
     float zNear;
     int fov;
     int xTranslate;
     int yTranslate;
     int zTranslate;
     int xRotate;
     int yRotate;
     float viewDistance;
     float viewPhi;
     float viewTheta;
     
     bool focused1;
     bool focused2;

     float lightAmbient[4];
     float lightDiffuse[4];
     float lightSpecular[4];
     float position[3];
     float c, l, q;
     float g;
     std::vector<std::string> directory;
     std::string fileSelection;
     unsigned int lightId;
     PointLight* lightsData;
     unsigned int lightsSize;
     

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


void drawLeftPanel(ImGuiIO& io, ConfigContext& config);
void drawRightPanel(ImGuiIO& io, ConfigContext& config);
