#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

struct ConfigContext{
    float far_plane;
     float near_plane;
     int fov;
     int p1;
     int p2;
     int p3;
     int p4;
     int p5;
     int p6;
     int p7;
     int p8;
};

void drawLeftPanel(ImGuiIO& io);
void drawRightPanel(ImGuiIO& io, ConfigContext &config);