#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include "pch.h"
#include "Light.h"

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


     PointLight getLight() 
     {
         glm::vec3 ambient = { light_ambient[0], light_ambient[1], light_ambient[2] };
         glm::vec3 diffuse = { light_diffuse[0], light_diffuse[1], light_diffuse[2] };
         glm::vec3 specular = { light_specular[0], light_specular[1], light_specular[2] };
         glm::vec4 position = { position[0], position[1], position[2], 1. };

         //float l_position[16] = {};
         //l_position[0] = 1.;
         //l_position[5] = 1.;
         //l_position[10] = 1.;
         //l_position[15] = 1.;

         //glm::mat4 View = glm::rotate(
         //    glm::rotate(
         //        glm::translate(
         //            glm::make_mat4x4(l_position)
         //            , translate)
         //        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
         //    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
         //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
         //glm::mat4 MVP = Projection * LookAt * View * Model;

         PointLight p;
         p.position = position;
         p.ambient = ambient;
         p.diffuse = diffuse;
         p.specular = specular;
         p.constant = c;
         p.quadratic = q;
         p.linear = l;

         return  p;// { position, c, l, q, ambient, diffuse, specular };
     }


     glm::vec3 getTranslate() 
     {
         return  glm::vec3(tr_x * 0.02, tr_y * 0.02, tr_z * 0.02);
     }

     glm::vec3 getRotate() 
     {
         return glm::vec3(3.14 * rot_x / 180, 3.14 * rot_y / 180, 0.f);
     }


     glm::vec3 polar() 
     {
         float r = 0.1 * dist;
         float phi = this->phi;
         float theta = this->theta;
         glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));

         return glm::vec3(eye.z, eye.y, eye.x);
     }


     glm::mat4 preperLookAt() 
     {
         float theta = this->theta;
         glm::vec3 eye = polar();

         glm::vec3 north = glm::vec3(0., 1., 0.);
         float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
         if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
             north = glm::vec3(0., -1., 0.);
         }
         return  glm::lookAt(eye, glm::vec3(0.), north);
     }

};

PointLight getLight(ConfigContext& panelConfig);
void insert_tree(ConfigContext& context, std::vector<std::string>& tree);

void drawLeftPanel(ImGuiIO& io, ConfigContext& config);
void drawRightPanel(ImGuiIO& io, ConfigContext& config);
