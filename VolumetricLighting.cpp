// VolumetricLighting.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define GLEW_STATIC

#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"


struct WindowInfo {
    int width;
    int height;
    const char* title;
};


void error_callback(int code, const char* description)
{
    std::cout << code << " " << description << std::endl;
}

int main(void)
{
    std::cout << "=========== Initialization started =========\n";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window;
    WindowInfo windowConfig = {
        1280,
        960,
        "Simple Triangles"
    };
    
    if (!glfwInit()) {
        std::cout << "========== [GLFW]: Initialization failed =========\n";
        return 1;
    }
    window = glfwCreateWindow(windowConfig.width, windowConfig.height, windowConfig.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "========== [GLFW]: Terminated =========\n";
        std::cout << "========== [GLFW]: Window initialization failed =========\n";
        return 1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "========== [GLEW]: Initialization failed =========\n";
        std::cout << "\tError:" << glewGetErrorString(err);
    }
    std::cout << "========== [GLEW]: Using GLEW " << glewGetString(GLEW_VERSION) << " ================\n";
    
    // glewIsSupported supported from version 1.3
    if(GLEW_VERSION_1_3)
    {
        if (glewIsSupported("GL_VERSION_4_5  GL_ARB_point_sprite"))
        {
            std::cout << "========== [GLEW]: Version 4.5 of OpenGL is supported =========\n";
            std::cout << "========== [GLEW]: Extention GL_ARB_point_sprite is supported =========\n";
        }
        /*
        GL_ARB_buffer_storage
        GL_ARB_clear_buffer_object
        GL_ARB_clear_texture
        GL_ARB_clip_control
        GL_ARB_multi_bind
        GL_ARB_sampler_objects
        GL_ARB_texture_storage
        GL_ARB_vertex_attrib_binding
        */
    }    

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 150"; // TO DO
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    glm::vec2 p1(-0.5, 0.5);
    glm::vec2 p2(0.5, 0.5);
    glm::vec2 p3(0, -0.5);

    glfwSetErrorCallback(error_callback);

    std::cout << "===================== Main loop ===================\n";
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBegin(GL_TRIANGLES);
        glVertex2f (-0.5, 0.5);
        glVertex2f (0.5, 0.5);
        glVertex2f (0, -0.5);
        glEnd();


        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool show_demo_window = false;
        bool show_shader_dialog = false;
        bool show_another_window = true;
        static int e = 0;

        
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Left panel");
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            
            if (ImGui::BeginTabBar("LeftPanelBar", tab_bar_flags))
            {
            if (ImGui::BeginTabItem("Config"))
            {
                ImGui::Text("Option1: ");
                ImGui::SameLine(); ImGui::Checkbox("c", &show_demo_window);
                ImGui::Text("Option2: ");
                ImGui::SameLine(); ImGui::SliderFloat("s", &f, 0.0f, 1.0f);
                ImGui::Text("Option3: ");
                ImGui::SameLine();  ImGui::Button("b");
                ImGui::Text("Option4: ");
                ImGui::SameLine(); ImGui::RadioButton("radio a", &e, 0);
                ImGui::Text("Option5: ");
                ImGui::SameLine(); ImGui::SameLine();  ImGui::RadioButton("radio b", &e, 1);
                // Combo Boxes are also called "Dropdown" in other systems
            // Expose flags as checkbox for the demo
                static ImGuiComboFlags flags = 0;
                /*
                ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", &flags, ImGuiComboFlags_PopupAlignLeft);
                ImGui::SameLine();
                if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", &flags, ImGuiComboFlags_NoArrowButton))
                    flags &= ~ImGuiComboFlags_NoPreview;     // Clear the other flag, as we cannot combine both
                if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", &flags, ImGuiComboFlags_NoPreview))
                    flags &= ~ImGuiComboFlags_NoArrowButton; // Clear the other flag, as we cannot combine both
                    */
                    // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
                    // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
                    // stored in the object itself, etc.)
                const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
                static int item_current_idx = 0;
                const char* combo_preview_value = items[item_current_idx];
                if (ImGui::BeginCombo("combo 1", combo_preview_value, flags))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                    {
                        const bool is_selected = (item_current_idx == n);
                        if (ImGui::Selectable(items[n], is_selected))
                            item_current_idx = n;
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::Button("Click");
                //ImGui::PopStyleColor(3);
                //ImGui::PopID();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug"))
            {
                int wrap_width = 200;
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                for (int n = 0; n < 2; n++)
                {
                    ImGui::Text("Test paragraph %d:", n);
                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImVec2 marker_min = ImVec2(pos.x + wrap_width, pos.y);
                    ImVec2 marker_max = ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight());
                    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
                    if (n == 0)
                        ImGui::Text("The lazy dog is a good dog. This paragraph should fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
                    else
                        ImGui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");

                    // Draw actual text bounding box, following by marker of our expected limit (should not overlap!)
                    draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
                    draw_list->AddRectFilled(marker_min, marker_max, IM_COL32(255, 0, 255, 255));
                    ImGui::PopTextWrapPos();
                }
               
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene"))
            {
                static bool selected[3] = { false, false, false };
                ImGui::Selectable("Scene One", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                ImGui::Selectable("Scene Two", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
                ImGui::Selectable("Scene Three", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                if (&selected[0])
                    show_shader_dialog = true;
                else
                    show_shader_dialog = false;

                if (ImGui::TreeNode("Scene")) {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::TreePop();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tests"))
            {
                static bool selected[3] = { false, false, false };
                ImGui::Selectable("Scene One", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                ImGui::Selectable("Scene Two", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
                ImGui::Selectable("Scene Three", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
            }

            ImGui::End();
        }
        if (show_shader_dialog) {
            ImGui::Begin("Shader", &show_shader_dialog);
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
        }


        {
            float far_plane;
            float near_plane;
            int pov;
            int p1;
            int p2;
            int p3;
            int p4;


            ImGui::Begin("Right Panel");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("Far plane:"); ImGui::SliderFloat("Fp", &far_plane, 0.0f, 1.0f);
            ImGui::Text("Near plane:"); ImGui::SliderFloat("Np", &near_plane, 0.0f, 1.0f);
            ImGui::Text("POV:"); ImGui::SliderInt("pov", &pov, 10, 120);

            ImGui::Separator();

            ImGui::SliderInt("Param1", &p1, 10, 120);
            ImGui::SliderInt("Param2", &p2, 10, 120);
            ImGui::SliderInt("Param3", &p3, 10, 120);
            ImGui::SliderInt("Param4", &p4, 10, 120);

            ImGui::Separator();
            ImGui::Button("Save Image");
            ImGui::SameLine(); ImGui::Button("Save Clip");
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    std::cout << "========== [GLFW]: Terminated =========\n";
    std::cout << "===================== Exit succeeded ===================\n";
    return 0;
}