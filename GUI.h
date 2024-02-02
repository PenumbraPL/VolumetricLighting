#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>

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
     
     bool focused1;
     bool focused2;

     float light_ambient[4];
     float light_diffuse[4];
     float light_specular[4];
     float position[3];
     float c, l, q;
     float g;
     std::vector<std::string>* directory;
};

void drawLeftPanel(ImGuiIO& io, ConfigContext& config);
void drawRightPanel(ImGuiIO& io, ConfigContext& config);