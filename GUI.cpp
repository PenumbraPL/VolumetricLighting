#include "pch.h"
#include "GUI.h"
#include "Tools.h"
#include "Debug.h"

namespace fs = std::filesystem;

extern std::shared_ptr<debug::BufferLogger> bufferLogger;


PointLight* ConfigContext::getLightsData() 
{
    return lightsData->data();
}

std::size_t ConfigContext::getLightsSize()
{
    return lightsData->size();
}


PointLight ConfigContext::getLight()
{
    glm::vec3 ambient{ lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    glm::vec3 diffuse{ lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    glm::vec3 specular{ lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    glm::vec3 position{ this->position[0], this->position[1], this->position[2] };

    return { position, c, l, q, ambient, diffuse, specular };
}

void ConfigContext::updateLight() 
{
    auto& light = getLightsData()[lightId];
    light.ambient = { lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    light.diffuse = { lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    light.specular = { lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    light.position = { position[0], position[1], position[2] };
}


glm::vec3 ConfigContext::getTranslate()
{
    return  glm::vec3{ xTranslate * 0.02, yTranslate * 0.02, zTranslate * 0.02 };
}

glm::vec3 ConfigContext::getRotate()
{
    return glm::vec3{ 3.14 * xRotate / 180, 3.14 * yRotate / 180, 0.f };
}


glm::vec3 ConfigContext::getView()
{
    float r{ 0.1f * this->viewDistance };
    float phi{ this->viewPhi };
    float theta{ this->viewTheta };
    glm::vec3 eye{ r * glm::euclidean(glm::vec2{ theta, phi }) };
 
    return glm::vec3{eye.z, eye.y, eye.x};
}


glm::mat4 ConfigContext::getLookAt()
{
    float theta{ this->viewTheta };
    glm::vec3 eye{ getView() };

    glm::vec3 north{ 0., 1., 0. };
    float corrected_theta{ glm::fmod(glm::abs(theta), 6.28f) };
    if (corrected_theta > 3.14 / 2. && corrected_theta < 3.14 * 3. / 2.) {
        north = glm::vec3{0., -1., 0.};
    }
    return  glm::lookAt(eye, glm::vec3{ 0. }, north);
}


glm::mat4 ConfigContext::getProjection(int width, int height) 
{
    return glm::perspectiveFov((float)3.14 * fov / 180, (float)width, (float)height, zNear, zFar);
}


std::string ConfigContext::getModelPath() 
{
    //std::cout << fileSelection.substr(0, fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return fileSelection.substr(0, fileSelection.find_last_of("/")+1);
}


std::string ConfigContext::getModelName() 
{
    //std::cout << "model name: " << fileSelection.substr(fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return fileSelection.substr(fileSelection.find_last_of("/")+1);
}




void folder_content(
    std::string& path, 
    std::vector<std::string>& content, 
    int& i, 
    std::string& selected,
    std::string& extension)
{
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            if (entry.path().filename().extension() == extension) {
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
                folder_content(subpath, content, i, selected, extension);
                ImGui::TreePop();
            }
            i++;
        }
    }
}

void drawLeftPanel(ImGuiIO& io, ConfigContext& config) 
{
    static bool show_shader_dialog{ false };
    static bool selected[3] = { false, false, false };
    //static std::string selected2 = 0;
    static std::string shaderSelection;
    static std::string lastShaderSelection;
    static char* fileText[1] = { nullptr };


    if (ImGui::Begin("Control")) {
        config.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ? true : false;

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("LeftPanelBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Config")) {
       
                ImGui::SliderFloat("g const", &config.g, -0.99999f, 0.99999f, "ratio = %.3f");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug")) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Debug info")) {

                    unsigned int lines = 20;
                    float scroll_x = ImGui::GetScrollX();
                    float scroll_max_x = ImGui::GetScrollMaxX();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
                    ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * lines + 30);
                    ImGui::BeginChild("scrolling", scrolling_child_size, 0, ImGuiWindowFlags_HorizontalScrollbar);
                    ImGui::Text(bufferLogger->getBuffer().c_str(), scroll_x - 20, scroll_max_x);

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

                    struct funcs { 
                        static bool IsLegacyNativeDupe(ImGuiKey key) { 
                            return key < 512 && ImGui::GetIO().KeyMap[key] != -1;}
                    }; 
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
                ImGui::SetNextItemOpen(true);
                if (ImGui::TreeNode("Scenes")) {
                    //if (config.directory) {
                    int i = 0;
                    std::string path = "res/models/";
                    std::vector<std::string> tree;
                    std::string extension = ".gltf";

                    folder_content(path, tree, i, config.fileSelection, extension);
                    // }
                    ImGui::Separator();
                    ImGui::TreePop();
                }
                ImGui::SetNextItemOpen(true);
                if (ImGui::TreeNode("Shaders")) {
                    int j{ 0 };
                    std::string path2{ "res/shaders/" };
                    std::vector<std::string> tree2;
                    std::string extension2{ ".glsl" };

                    folder_content(path2, tree2, j, shaderSelection, extension2);
                    ImGui::TreePop();
                }

                    show_shader_dialog = !shaderSelection.empty();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Lights")) {
              
                for (int i = 0; i < config.getLightsSize(); i++) {
                    std::string text = "Light " + std::to_string(i);
                    if(ImGui::Selectable(text.c_str(), i == config.lightId)) config.lightId = i;
                }
                ImGui::Separator();
                if(ImGui::Button("Add Light")) config.lightsData->emplace_back(PointLight()); 
                ImGui::SameLine(300); 
                if (ImGui::Button("Delete Light")) {
                    if (config.lightsData->size() > 0) {
                        config.lightsData->erase(config.lightsData->begin() + config.lightId);
                    }
                }
                ImGui::EndTabItem();
                ImGui::Separator();

                auto& light = config.getLightsData()[config.lightId];
                glm::vec3 ambient{ light.ambient };
                glm::vec3 diffuse{ light.diffuse };
                glm::vec3 specular{ light.specular };
                glm::vec3 position{ light.position };
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

            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
    if (show_shader_dialog) {
        ImGui::Begin("Shader", &show_shader_dialog);

        static char text[2*4096] = {'\0'};
        
        if (shaderSelection != lastShaderSelection) {
            if (*fileText) free(*fileText);
            *fileText = read_file(shaderSelection.c_str());
            if (*fileText) strcpy(text, *fileText);
            lastShaderSelection = shaderSelection;
        }

        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        ImGui::CheckboxFlags("Read only", &flags, ImGuiInputTextFlags_ReadOnly);
        ImGui::CheckboxFlags("Allow tab input", &flags, ImGuiInputTextFlags_AllowTabInput);
        ImGui::CheckboxFlags("Ctrl enter for new line", &flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
        ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
        
        if (ImGui::Button("Update")) {
            ImGui::OpenPopup("Update?");           
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Update?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("You will override your file. Do you want to continue?");
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                FILE* fs;
                fopen_s(&fs, shaderSelection.c_str(), "wb");
                unsigned int bufferSize = strlen(text);
                if (fs && bufferSize) {
                    fwrite(text, 1, bufferSize, fs);
                    fclose(fs);
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
        ImGui::SameLine(); 
        if (ImGui::Button("Restore")) {
            if(*fileText) strcpy(text, *fileText);
        }

        ImGui::End();
        if (!show_shader_dialog) {
            shaderSelection.clear();
            if (*fileText) free(*fileText);
        }
    }
}

void drawRightPanel(ImGuiIO& io, ConfigContext &config) 
{
    if (ImGui::Begin("View")) {
        
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
