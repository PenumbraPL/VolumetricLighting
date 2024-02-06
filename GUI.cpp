#include "pch.h"
#include "GUI.h"
#include "Tools.h"
#include "Debug.h"

namespace fs = std::filesystem;

extern std::shared_ptr<debug::BufferLogger> bufferLogger;

PointLight getLight(ConfigContext& panelConfig) 
{
    glm::vec3 ambient = { panelConfig.light_ambient[0], panelConfig.light_ambient[1], panelConfig.light_ambient[2] };
    glm::vec3 diffuse = { panelConfig.light_diffuse[0], panelConfig.light_diffuse[1], panelConfig.light_diffuse[2] };
    glm::vec3 specular = { panelConfig.light_specular[0], panelConfig.light_specular[1], panelConfig.light_specular[2] };
    glm::vec4 position = { panelConfig.position[0], panelConfig.position[1], panelConfig.position[2], 1. };

    //lightsData[lightId]

    float l_position[16] = {};
    l_position[0] = 1.;
    l_position[5] = 1.;
    l_position[10] = 1.;
    l_position[15] = 1.;

    //glm::mat4 View = glm::rotate(
    //    glm::rotate(
    //        glm::translate(
    //            glm::make_mat4x4(l_position)
    //            , translate)
    //        , rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f)),
    //    rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    //glm::mat4 MVP = Projection * LookAt * View * Model;

    glm::vec4 new_position = position;

    return  { new_position, panelConfig.c, panelConfig.l, panelConfig.q, ambient, diffuse, specular };
}

void insert_tree(ConfigContext& context, std::vector<std::string>& tree) 
{
    context.directory = &tree;
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
    bool show_demo_window = false;
    static bool show_shader_dialog = false;
    bool show_another_window = true;
    static int e = 0;
    static bool selected[3] = { false, false, false };
    //static std::string selected2 = 0;
    static float g = 0.123f;

    if(ImGui::Begin("Control")) {
        config.focused1 = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow) ? true : false;

        static float f = 0.0f;
        static int counter = 0;

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("LeftPanelBar", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Config")) {
                ImGui::ColorEdit3("ambient light", config.light_ambient);
                ImGui::ColorEdit3("diffuse light", config.light_diffuse);
                ImGui::ColorEdit3("specular light", config.light_specular);
                ImGui::SliderFloat("const", &config.c, 0.0f, 1.0f);
                ImGui::SliderFloat("linear", &config.l, 0.0f, 1.0f);
                ImGui::SliderFloat("quad", &config.q, 0.0f, 1.0f);
                ImGui::SliderFloat("g const", &config.g, -0.99999f, 0.99999f, "ratio = %.3f");
                ImGui::SliderFloat("x", &config.position[0], -1.0f, 1.0f);
                ImGui::SliderFloat("y", &config.position[1], -1.0f, 1.0f);
                ImGui::SliderFloat("z", &config.position[2], -1.0f, 1.0f);

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug")) {
                int wrap_width = 400;
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                for (int n = 0; n < 1; n++) {
                    ImGui::Text("Test paragraph %d:", n);
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImVec2 marker_min = ImVec2(pos.x + wrap_width, pos.y);
                    ImVec2 marker_max = ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight());
                    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                    if (n == 0) {
                        ImGui::Text(bufferLogger->getBuffer().c_str(), wrap_width);
                    }
                    else {
                        ImGui::Text("Logger error!");
                    }
                    draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
                    ImGui::PopTextWrapPos();
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

                    // We iterate both legacy native range and named ImGuiKey ranges, which is a little odd but this allows displaying the data for old/new backends.
                    // User code should never have to go through such hoops! You can generally iterate between ImGuiKey_NamedKey_BEGIN and ImGuiKey_NamedKey_END.
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
                    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
                    ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
#else
                    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { 
                        return key < 512 && ImGui::GetIO().KeyMap[key] != -1;
                    } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
                    ImGuiKey start_key = (ImGuiKey)0;
#endif
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
                if (config.directory) {
                    int i = 0;
                    std::string path = "res/models/";
                    std::vector<std::string> tree;

                    folder_content(path, tree, i, config.fileSelection);
                }


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
                static bool selected[3] = { false, false, false };
                ImGui::Selectable("Light 1", &selected[0]);
                ImGui::Separator();
                ImGui::Button("Add Light"); ImGui::SameLine(300);
                ImGui::Button("Delete Light");
                ImGui::EndTabItem();
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
        ImGui::Text("Far plane:"); ImGui::SliderFloat("Fp", &config.far_plane, 0.1f, 200.0f);
        ImGui::Text("Near plane:"); ImGui::SliderFloat("Np", &config.near_plane, 0.0001f, 10.0f);
        ImGui::Text("fov:"); ImGui::SliderInt("fov", &config.fov, 10, 120);
        ImGui::Separator();

        ImGui::SliderInt("Translation X", &config.tr_x, -100, 100);
        ImGui::SliderInt("Translation Y", &config.tr_y, -100, 100);
        ImGui::SliderInt("Translation Z", &config.tr_z, -100, 100);
        ImGui::SliderInt("Rotation X", &config.rot_x, 0, 360);
        ImGui::SliderInt("Rotation Y", &config.rot_y, 0, 360);
        ImGui::SliderFloat("Camera distance", &config.dist, 0, 360);
        ImGui::SliderAngle("Camera phi", &config.phi, 0, 360);
        ImGui::SliderAngle("Camera theta", &config.theta, 0, 360);
        ImGui::Separator();

        ImGui::Button("Save Image");
        ImGui::SameLine(); ImGui::Button("Save Clip");
        ImGui::End();
    }
}
