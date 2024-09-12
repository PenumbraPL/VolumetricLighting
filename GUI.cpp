#include "pch.h"
#include "GUI.h"
#include "Tools.h"
#include "Debug.h"

namespace fs = std::filesystem;

extern std::shared_ptr<debug::BufferLogger> bufferLogger;


PointLight* GUI::getLightsData() 
{
    return lightsData.get()->data();
}

std::size_t GUI::getLightsSize()
{
    return lightsData.get()->size();
}


PointLight GUI::getLight()
{
    glm::vec3 ambient{ lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    glm::vec3 diffuse{ lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    glm::vec3 specular{ lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    glm::vec3 position{ this->position[0], this->position[1], this->position[2] };

    return { position, c, l, q, ambient, diffuse, specular };
}

void GUI::updateLight() 
{
    auto& light = getLightsData()[lightId];
    light.ambient = { lightAmbient[0], lightAmbient[1], lightAmbient[2] };
    light.diffuse = { lightDiffuse[0], lightDiffuse[1], lightDiffuse[2] };
    light.specular = { lightSpecular[0], lightSpecular[1], lightSpecular[2] };
    light.position = { position[0], position[1], position[2] };
}


glm::vec3 GUI::getTranslate()
{
    return  glm::vec3{ xTranslate * 0.02, yTranslate * 0.02, zTranslate * 0.02 };
}

glm::vec3 GUI::getRotate()
{
    return glm::vec3{ 3.14 * xRotate / 180, 3.14 * yRotate / 180, 0.f };
}


glm::vec3 GUI::getView()
{
    float r{ 0.1f * this->viewDistance };
    float phi{ this->viewPhi };
    float theta{ this->viewTheta };
    glm::vec3 eye{ r * glm::euclidean(glm::vec2{ theta, phi }) };
 
    return glm::vec3{eye.z, eye.y, eye.x};
}


glm::mat4 GUI::getLookAt()
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


glm::mat4 GUI::getProjection(int width, int height) 
{
    return glm::perspectiveFov((float)3.14 * fov / 180, (float)width, (float)height, (float)zNear, (float) zFar);
}


std::string GUI::getModelPath() 
{
    //std::cout << fileSelection.substr(0, fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return selectedSceneFile.get().substr(0, selectedSceneFile.get().find_last_of("/") + 1);
}


std::string GUI::getModelName() 
{
    //std::cout << "model name: " << fileSelection.substr(fileSelection.find_last_of("/")+1).c_str() << std::endl;
    return selectedSceneFile.get().substr(selectedSceneFile.get().find_last_of("/")+1);
}




void folderContent(
    std::string& path, 
    std::vector<std::string>& content, 
    int& i, 
    Object<std::string>& selected,
    std::string& extension)
{
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            if (entry.path().filename().extension() == extension) {
                std::string fileName = entry.path().filename().generic_string();
                std::string filePath = entry.path().generic_string();
                content.push_back(filePath);

                if (ImGui::Selectable(fileName.c_str(), content.at(content.size() - 1) == selected.get())) {
                    selected.get() = filePath;
                    selected.notifyAll();
                }
                ImGui::SameLine(200); ImGui::Text(filePath.c_str());
            }
        }
        else if (entry.is_directory()) {
            std::string subpath = entry.path().generic_string();

            if (ImGui::TreeNode((void*)(intptr_t)i, entry.path().filename().generic_string().c_str())){
                folderContent(subpath, content, i, selected, extension);
                ImGui::TreePop();
            }
            i++;
        }
    }
}

void GUI::drawLeftPanel(ImGuiIO& io) 
{
    static bool show_shader_dialog{ false };
    static bool selected[3] = { false, false, false };
    //static std::string selected2 = 0;
    static Object<std::string> shaderSelection{ std::string() };
    static Object<std::string> lastShaderSelection{std::string()};
    static char* fileText[1] = { nullptr };


    if (ImGui::Begin("Control")) {
        workSpaceFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) ? true : false;

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("LeftPanelBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Config")) {
                if (ImGui::SliderFloat("g const", &g.get(), - 0.99999f, 0.99999f, "ratio = %.3f")) {
                    g.notifyAll();
                }
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
                    //if (directory) {
                    int i = 0;
                    std::string path = "res/models/";
                    std::vector<std::string> tree;
                    std::string extension = ".gltf";

                    folderContent(path, tree, i, selectedSceneFile, extension);
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

                    folderContent(path2, tree2, j, shaderSelection, extension2);
                    ImGui::TreePop();
                }

                    show_shader_dialog = !shaderSelection.get().empty();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Lights")) {
              
                for (int i = 0; i < getLightsSize(); i++) {
                    std::string text = "Light " + std::to_string(i);
                    if (ImGui::Selectable(text.c_str(), i == lightId)) {
                        lightId = i;
                        lightId.notifyAll();
                    }
                }
                ImGui::Separator();
                if (ImGui::Button("Add Light")) {
                    lightsData.get()->emplace_back(PointLight());
                    lightsData.notifyAll();
                }
                ImGui::SameLine(300); 
                if (ImGui::Button("Delete Light")) {
                    if (lightsData.get()->size() > 0) {
                        lightsData.get()->erase(lightsData.get()->begin() + lightId);
                        lightsData.notifyAll();
                    }
                }
                ImGui::EndTabItem();
                ImGui::Separator();

                auto& light = getLightsData()[lightId];
                
                float* ambient{ static_cast<float*>(glm::value_ptr(light.ambient)) };
                float* diffuse{ static_cast<float*>(glm::value_ptr(light.diffuse)) };
                float* specular{ static_cast<float*>(glm::value_ptr(light.specular)) };
                float* position{ static_cast<float*>(glm::value_ptr(light.position)) };
                for (int i = 0; i < 3; i++) {
                    lightAmbient[i] = ambient[i];
                    lightDiffuse[i] = diffuse[i];
                    lightSpecular[i] = specular[i];
                    this->position[i] = position[i];
                }

                if (ImGui::ColorEdit3("ambient light", &lightAmbient.get())) {
                    lightAmbient.notifyAll();
                };
                if (ImGui::ColorEdit3("diffuse light", &lightDiffuse.get())) {
                    lightDiffuse.notifyAll();
                };
                if (ImGui::ColorEdit3("specular light", &lightSpecular.get())) {
                    lightSpecular.notifyAll();
                };
                if (ImGui::SliderFloat("const", &c.get(), 0.0f, 1.0f)) {
                    c.notifyAll();
                };
                if (ImGui::SliderFloat("linear", &l.get(), 0.0f, 1.0f)) {
                    l.notifyAll();
                };
                if (ImGui::SliderFloat("quad", &q.get(), 0.0f, 1.0f)) {
                    q.notifyAll();
                };
                if (ImGui::SliderFloat("x", &this->position[0], -1.0f, 1.0f)) {
                    this->position.notifyAll();
                };
                if (ImGui::SliderFloat("y", &this->position[1], -1.0f, 1.0f)) {
                    this->position.notifyAll();
                };
                if (ImGui::SliderFloat("z", &this->position[2], -1.0f, 1.0f)) {
                    this->position.notifyAll();
                };

            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
    if (show_shader_dialog) {
        ImGui::Begin("Shader", &show_shader_dialog);

        static char text[2*4096] = {'\0'};
        
        if (shaderSelection.get() != lastShaderSelection.get()) {
            if (*fileText) free(*fileText);
            *fileText = readFile(shaderSelection.get().c_str());
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
                fopen_s(&fs, shaderSelection.get().c_str(), "wb");
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
            shaderSelection.get().clear();
            if (*fileText) free(*fileText);
        }
    }
}

void GUI::drawRightPanel(ImGuiIO& io)
{
    if (ImGui::Begin("View")) {
        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Far plane:"); 
        if (ImGui::SliderFloat("Fp", &zFar.get(), 0.1f, 200.0f)) {
            zFar.notifyAll();
        }
        ImGui::Text("Near plane:"); 
        if (ImGui::SliderFloat("Np", &zNear.get(), 0.0001f, 10.0f)) {
            zNear.notifyAll();
        }
        ImGui::Text("fov:"); 
        if (ImGui::SliderInt("fov", &fov.get(), 10, 120)) {
            fov.notifyAll();
        }
        ImGui::Separator();

        if(ImGui::SliderInt("Translation X", &xTranslate.get(), -100, 100)) xTranslate.notifyAll();
        if(ImGui::SliderInt("Translation Y", &yTranslate.get(), -100, 100)) yTranslate.notifyAll();
        if (ImGui::SliderInt("Translation Z", &zTranslate.get(), -100, 100)) zTranslate.notifyAll();
        if (ImGui::SliderInt("Rotation X", &xRotate.get(), 0, 360)) xRotate.notifyAll();
        if (ImGui::SliderInt("Rotation Y", &yRotate.get(), 0, 360)) yRotate.notifyAll();
        if (ImGui::SliderFloat("Camera distance", &viewDistance.get(), 0, 360)) viewDistance.notifyAll();
        if (ImGui::SliderAngle("Camera phi", &viewPhi.get(), 0, 360)) viewPhi.notifyAll();
        if (ImGui::SliderAngle("Camera theta", &viewTheta.get(), 0, 360)) viewTheta.notifyAll();
        ImGui::Separator();

        ImGui::Button("Save Image");
        ImGui::SameLine(); ImGui::Button("Save Clip");
        ImGui::End();
    }
}


GUI::GUI(std::string selectedSceneFile) : selectedSceneFile{ selectedSceneFile } {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
};

void GUI::deleteImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

ImGuiIO& GUI::getIO() {
    return ImGui::GetIO();
}


void GUI::draw() {
    ImGuiIO& io = getIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawLeftPanel(io);
    drawRightPanel(io);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void GUI::chooseGlfwImpl(GLFWwindow* window) {
    ImGuiIO& io = getIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsDark();

    const char* GLSLVersion{ "#version 450" };
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSLVersion);

}

void GUI::subscribeToView(Observer& observer) {
    xTranslate.subscribe(observer);
    yTranslate.subscribe(observer);
    zTranslate.subscribe(observer);
    xRotate.subscribe(observer);
    yRotate.subscribe(observer);
    viewDistance.subscribe(observer);
    viewPhi.subscribe(observer);
    viewTheta.subscribe(observer);
}

void GUI::unsubscribeToView(Observer& observer)
{
    xTranslate.unsubscribe(observer);
    yTranslate.unsubscribe(observer);
    zTranslate.unsubscribe(observer);
    xRotate.unsubscribe(observer);
    yRotate.unsubscribe(observer);
    viewDistance.unsubscribe(observer);
    viewPhi.unsubscribe(observer);
    viewTheta.unsubscribe(observer);
}

void GUI::subscribeToEye(Observer& observer) {
    viewDistance.subscribe(observer);
    viewPhi.subscribe(observer);
    viewTheta.subscribe(observer);
    zFar.subscribe(observer);
    zNear.subscribe(observer);
    fov.subscribe(observer);
}

void GUI::subscribeToLight(Observer& observer)
{
    position.subscribe(observer);
    lightId.subscribe(observer);
    lightAmbient.subscribe(observer);
    lightDiffuse.subscribe(observer);
    lightSpecular.subscribe(observer);
}