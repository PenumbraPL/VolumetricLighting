#include "pch.h"
#include "GUI.h"
#include "Tools.h"
#include "Debug.h"

namespace fs = std::filesystem;

extern std::shared_ptr<debug::BufferLogger> bufferLogger;

PointLight ConfigContext::getLight()
{
    //glm::vec3 ambient = lightsData[lightId].ambient;
    //glm::vec3 diffuse = lightsData[lightId].diffuse;
    //glm::vec3 specular = lightsData[lightId].specular;
    //glm::vec3 position = lightsData[lightId].position;
    //float c = lightsData[lightId].constant;
    //float l = lightsData[lightId].linear;
    //float q = lightsData[lightId].quadratic;

    glm::vec3 ambient = glm::vec3{ lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    glm::vec3 diffuse = glm::vec3{ lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    glm::vec3 specular = glm::vec3{ lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    glm::vec3 position = glm::vec3{ this->position[0], this->position[1], this->position[2] };


    return { position, c, l, q, ambient, diffuse, specular };
}

void ConfigContext::updateLight() {
    lightsData[lightId].ambient = { lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    lightsData[lightId].diffuse = { lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    lightsData[lightId].specular = { lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    lightsData[lightId].position = { position[0], position[1], position[2] };
}


glm::vec3 ConfigContext::getTranslate()
{
    return  glm::vec3(xTranslate * 0.02, yTranslate * 0.02, zTranslate * 0.02);
}

glm::vec3 ConfigContext::getRotate()
{
    return glm::vec3(3.14 * xRotate / 180, 3.14 * yRotate / 180, 0.f);
}


glm::vec3 ConfigContext::getView()
{
    float r = 0.1f * viewDistance;
    float phi = this->viewPhi;
    float theta = this->viewTheta;
    glm::vec3 eye = r * glm::euclidean(glm::vec2(theta, phi));

    return glm::vec3(eye.z, eye.y, eye.x);
}


glm::mat4 ConfigContext::getLookAt()
{
    float theta = this->viewTheta;
    glm::vec3 eye = getView();

    glm::vec3 north = glm::vec3(0., 1., 0.);
    float corrected_theta = glm::fmod(glm::abs(theta), 6.28f);
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3(0., -1., 0.);
    }
    return  glm::lookAt(eye, glm::vec3(0.), north);
}


glm::mat4 ConfigContext::getProjection(int width, int height) {
    return glm::perspectiveFov((float)3.14 * fov / 180, (float)width, (float)height, zNear, zFar);
}


std::string ConfigContext::getModelPath() {
    std::cout << fileSelection.substr(0, fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return fileSelection.substr(0, fileSelection.find_last_of("/")+1);
}


std::string ConfigContext::getModelName() {
    std::cout << "model name: " << fileSelection.substr(fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return fileSelection.substr(fileSelection.find_last_of("/")+1);
}




void folder_content(
    std::string& path, 
    std::vector<std::string>& content, 
    int& i, 
    std::string& selected) 
{
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            if (entry.path().filename().extension() == ".gltf") {
                std::string fileName = entry.path().filename().generic_string();
                std::string filePath = entry.path().generic_string();
                content.push_back(filePath);

                if(ImGui::Selectable(fileName.c_str(), content.at(content.size() - 1) == selected)) selected = filePath;
                ImGui::SameLine(200); ImGui::Text(filePath.c_str());
            }
        }
        else if (entry.is_directory()) {
            std::string subpath = entry.path().generic_string();

            if (ImGui::TreeNode((void*)(intptr_t)i, entry.path().filename().generic_string().c_str())){
                folder_content(subpath, content, i, selected);
                ImGui::TreePop();
            }
            i++;
        }
    }
}

void drawLeftPanel(ImGuiIO& io, ConfigContext& config) 
{
    static bool show_shader_dialog = false;
    static bool selected[3] = { false, false, false };
    //static std::string selected2 = 0;

    if (ImGui::Begin("Control")) {
        config.focused1 = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) ? true : false;
        

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("LeftPanelBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Config")) {
       
                ImGui::SliderFloat("g const", &config.g, -0.99999f, 0.99999f, "ratio = %.3f");


                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug")) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Debug info")) {
                   // int wrap_width = 400;
                   // ImDrawList* draw_list = ImGui::GetWindowDrawList();

                    unsigned int lines = 20;
                    float scroll_x = ImGui::GetScrollX();
                    float scroll_max_x = ImGui::GetScrollMaxX();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
                    ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * lines + 30);
                    ImGui::BeginChild("scrolling", scrolling_child_size, 0, ImGuiWindowFlags_HorizontalScrollbar);

                    //ImVec2 pos = ImGui::GetCursorScreenPos();
                    //ImVec2 marker_min = ImVec2(pos.x + wrap_width, pos.y);
                    //ImVec2 marker_max = ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight());
                    //ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                    ImGui::Text(bufferLogger->getBuffer().c_str(), scroll_x - 20, scroll_max_x);// wrap_width);
                    //draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
                    //ImGui::PopTextWrapPos();

                    ImGui::EndChild();
                    ImGui::PopStyleVar(2);
                    ImGui::Spacing();
                    ImGui::TreePop();
                }


                ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                if (ImGui::TreeNode("Mouse + Keyboard")) {
                    if (ImGui::IsMousePosValid()) {
                        ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
                    }
                    else {
                        ImGui::Text("Mouse pos: <INVALID>");
                    }
                    ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
                    ImGui::Text("Mouse down:");
                    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
                        if (ImGui::IsMouseDown(i)) {
                            ImGui::SameLine();
                            ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
                        }
                    }
                    ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

                    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { 
                        return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
                    } }; 
                    ImGuiKey start_key = (ImGuiKey)0;

                    ImGui::Text("Keys down:");
                    for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { 
                        if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue;
                        ImGui::SameLine();
                        ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
                    }
                    ImGui::Text("Keys mods: %s%s%s%s",
                        io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
                    ImGui::Text("Chars queue:");
                    for (int i = 0; i < io.InputQueueCharacters.Size; i++) { 
                        ImWchar c = io.InputQueueCharacters[i];
                        ImGui::SameLine();
                        ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c);
                    } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
                    ImGui::TreePop();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Scene")) {
                //if (config.directory) {
                    int i = 0;
                    std::string path = "res/models/";
                    std::vector<std::string> tree;

                    folder_content(path, tree, i, config.fileSelection);
               // }


                //    
                //ImGui::Selectable("Scene One", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                //ImGui::Selectable("Scene Two", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
                //ImGui::Selectable("Scene Three", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                //if (selected[1])
                //    show_shader_dialog = true;
                //else
                //    show_shader_dialog = false;

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Lights")) {
              
                for (int i = 0; i < config.lightsSize; i++) {
                    std::string text = "Light " + std::to_string(i);
                    if(ImGui::Selectable(text.c_str(), i == config.lightId)) config.lightId = i;
                }
                ImGui::Separator();
                if(ImGui::Button("Add Light")) config.lightsSize++; 
                ImGui::SameLine(300); if(ImGui::Button("Delete Light")) config.lightsSize > 0 ? config.lightsSize-- : 0;
                ImGui::EndTabItem();
                ImGui::Separator();

                glm::vec3 ambient = config.lightsData[config.lightId].ambient;
                glm::vec3 diffuse = config.lightsData[config.lightId].diffuse;
                glm::vec3 specular = config.lightsData[config.lightId].specular;
                glm::vec3 position = config.lightsData[config.lightId].position;
                config.lightAmbient[0] = ambient.x;
                config.lightAmbient[1] = ambient.y;
                config.lightAmbient[2] = ambient.z;
                config.lightDiffuse[0] = diffuse.x;
                config.lightDiffuse[1] = diffuse.y;
                config.lightDiffuse[2] = diffuse.z;
                config.lightSpecular[0] = specular.x;
                config.lightSpecular[1] = specular.y;
                config.lightSpecular[2] = specular.z;
                config.position[0] = position.x;
                config.position[1] = position.y;
                config.position[2] = position.z;

                ImGui::ColorEdit3("ambient light", config.lightAmbient);
                ImGui::ColorEdit3("diffuse light", config.lightDiffuse);
                ImGui::ColorEdit3("specular light", config.lightSpecular);
                ImGui::SliderFloat("const", &config.c, 0.0f, 1.0f);
                ImGui::SliderFloat("linear", &config.l, 0.0f, 1.0f);
                ImGui::SliderFloat("quad", &config.q, 0.0f, 1.0f);
                ImGui::SliderFloat("x", &config.position[0], -1.0f, 1.0f);
                ImGui::SliderFloat("y", &config.position[1], -1.0f, 1.0f);
                ImGui::SliderFloat("z", &config.position[2], -1.0f, 1.0f);


                //config.lightsData[config.lightId].ambient = { config.lightAmbient[0], config.lightAmbient[1], config.lightAmbient[2] };
                //config.lightsData[config.lightId].diffuse = { config.lightDiffuse[0], config.lightDiffuse[1], config.lightDiffuse[2] };
                //config.lightsData[config.lightId].specular = { config.lightSpecular[0], config.lightSpecular[1], config.lightSpecular[2] };
                //config.lightsData[config.lightId].position = { config.position[0], config.position[1], config.position[2] };

            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
    if (show_shader_dialog) {
        bool opened = ImGui::Begin("Shader", &show_shader_dialog);
        static char text[1024 * 16] =
            "/*\n"
            " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
            " the hexadecimal encoding of one offending instruction,\n"
            " more formally, the invalid operand with locked CMPXCHG8B\n"
            " instruction bug, is a design flaw in the majority of\n"
            " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
            " processors (all in the P5 microarchitecture).\n"
            "*/\n\n"
            "label:\n"
            "\tlock cmpxchg8b eax\n";

        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", &flags, ImGuiInputTextFlags_ReadOnly);
        ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", &flags, ImGuiInputTextFlags_AllowTabInput);
        ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", &flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
        ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
        ImGui::Button("Update");
        ImGui::SameLine(); ImGui::Button("Restore");

        ImGui::End();
        if (!opened) {
            selected[1] = false;
        }
    }
}

void drawRightPanel(ImGuiIO& io, ConfigContext &config) 
{
    if (ImGui::Begin("View")) {
        config.focused2 = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) ? true : false;
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Far plane:"); ImGui::SliderFloat("Fp", &config.zFar, 0.1f, 200.0f);
        ImGui::Text("Near plane:"); ImGui::SliderFloat("Np", &config.zNear, 0.0001f, 10.0f);
        ImGui::Text("fov:"); ImGui::SliderInt("fov", &config.fov, 10, 120);
        ImGui::Separator();

        ImGui::SliderInt("Translation X", &config.xTranslate, -100, 100);
        ImGui::SliderInt("Translation Y", &config.yTranslate, -100, 100);
        ImGui::SliderInt("Translation Z", &config.zTranslate, -100, 100);
        ImGui::SliderInt("Rotation X", &config.xRotate, 0, 360);
        ImGui::SliderInt("Rotation Y", &config.yRotate, 0, 360);
        ImGui::SliderFloat("Camera distance", &config.viewDistance, 0, 360);
        ImGui::SliderAngle("Camera phi", &config.viewPhi, 0, 360);
        ImGui::SliderAngle("Camera theta", &config.viewTheta, 0, 360);
        ImGui::Separator();

        ImGui::Button("Save Image");
        ImGui::SameLine(); ImGui::Button("Save Clip");
        ImGui::End();
    }
}
