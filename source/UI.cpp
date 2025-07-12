#include "UI.hpp"

#include "Context.hpp"

#include "imgui.h"
#include "ImGuiFileDialog.h"

void UI::DrawDockSpace() {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // Important: Begin a full-screen window with no decoration
    ImGui::Begin("DockSpace", nullptr, window_flags);

    // Create the actual dockspace node
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    ImGui::End();
}

void UI::DrawFileExplorer() {
    ImGui::Begin("Window A");
    ImGui::Text("Hello from A!");
    ImGui::End();
}

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
