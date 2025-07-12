#include "UI.hpp"

#include "Context.hpp"

#include "imgui.h"
#include "ImGuiFileDialog.h"

// testing
#include <iostream>

void UI::DrawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Game Root...", "Ctrl+O")) {
                IGFD::FileDialogConfig config;
                config.path = ".";

                auto gameRoot = Context::Get().GetGameRoot();
                if (gameRoot) {
                    config.path = (*gameRoot).string();
                }

                ImGuiFileDialog::Instance()->OpenDialog("ChooseGameRoot", "Choose Game Directory", nullptr, config);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseGameRoot")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string directoryPath = ImGuiFileDialog::Instance()->GetCurrentPath();

            auto gameRoot = Context::FindGameRoot(directoryPath);

            if (!gameRoot) {
                // TODO: Warn the user; this is not a game directory
            }
            else {
                if (*gameRoot != directoryPath) {
                    // TODO: Ask the user to fix the game path automatically
                    // Just 
                }
                else {
                    Context::Get().SetGameRoot(gameRoot);
                }
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }
}
