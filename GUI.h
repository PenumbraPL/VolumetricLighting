#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

struct ConfigContext{
     float far_plane;
     float near_plane;
     int fov;
     int tr_x;
     int tr_y;
     int tr_z;
     int rot_x;
     int rot_y;
     float dist;
     float phi;
     float theta;
};

void drawLeftPanel(ImGuiIO& io);
void drawRightPanel(ImGuiIO& io, ConfigContext &config);