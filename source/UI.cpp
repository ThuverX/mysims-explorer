#include "UI.hpp"

#include "imgui.h"

void UI::DrawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Game Root...", "Ctrl+O")) {
                // TODO: Open dialog to open game root directory
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
