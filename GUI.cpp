#include "GUI.h"

void drawLeftPanel(ImGuiIO& io) {
    bool show_demo_window = false;
    static bool show_shader_dialog = false;
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
                int wrap_width = 400;
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

                if (ImGui::TreeNode("Scrolling"))
                {
                    // Vertical scroll functions

                    static int track_item = 50;
                    static bool enable_track = true;
                    static bool enable_extra_decorations = false;
                    static float scroll_to_off_px = 0.0f;
                    static float scroll_to_pos_px = 200.0f;

                    ImGui::Checkbox("Decoration", &enable_extra_decorations);

                    ImGui::Checkbox("Track", &enable_track);
                    ImGui::PushItemWidth(100);
                    ImGui::SameLine(140); enable_track |= ImGui::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");

                    bool scroll_to_off = ImGui::Button("Scroll Offset");
                    ImGui::SameLine(140); scroll_to_off |= ImGui::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");

                    bool scroll_to_pos = ImGui::Button("Scroll To Pos");
                    ImGui::SameLine(140); scroll_to_pos |= ImGui::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
                    ImGui::PopItemWidth();

                    if (scroll_to_off || scroll_to_pos)
                        enable_track = false;

                    ImGuiStyle& style = ImGui::GetStyle();
                    float child_w = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
                    if (child_w < 1.0f)
                        child_w = 1.0f;
                    ImGui::PushID("##VerticalScrolling");
                    for (int i = 0; i < 5; i++)
                    {
                        if (i > 0) ImGui::SameLine();
                        ImGui::BeginGroup();
                        const char* names[] = { "Top", "25%", "Center", "75%", "Bottom" };
                        ImGui::TextUnformatted(names[i]);

                        const ImGuiWindowFlags child_flags = enable_extra_decorations ? ImGuiWindowFlags_MenuBar : 0;
                        const ImGuiID child_id = ImGui::GetID((void*)(intptr_t)i);
                        const bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, 200.0f), true, child_flags);
                        if (ImGui::BeginMenuBar())
                        {
                            ImGui::TextUnformatted("abc");
                            ImGui::EndMenuBar();
                        }
                        if (scroll_to_off)
                            ImGui::SetScrollY(scroll_to_off_px);
                        if (scroll_to_pos)
                            ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
                        if (child_is_visible) // Avoid calling SetScrollHereY when running with culled items
                        {
                            for (int item = 0; item < 100; item++)
                            {
                                if (enable_track && item == track_item)
                                {
                                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
                                    ImGui::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                                }
                                else
                                {
                                    ImGui::Text("Item %d", item);
                                }
                            }
                        }
                        float scroll_y = ImGui::GetScrollY();
                        float scroll_max_y = ImGui::GetScrollMaxY();
                        ImGui::EndChild();
                        ImGui::Text("%.0f/%.0f", scroll_y, scroll_max_y);
                        ImGui::EndGroup();
                    }
                    ImGui::PopID();

                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene"))
            {
                static bool selected[3] = { false, false, false };
                ImGui::Selectable("Scene One", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                ImGui::Selectable("Scene Two", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
                ImGui::Selectable("Scene Three", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
                if (&selected[1])
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

}


void drawRightPanel(ImGuiIO& io) {
    static float far_plane = 0.0f;
    static float near_plane = 0.0f;
    static int pov = 10;
    static int p1 = 10;
    static int p2 = 10;
    static int p3 = 10;
    static int p4 = 10;


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