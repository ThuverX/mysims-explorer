#include "UI.hpp"

#include "Context.hpp"
#include <essencio/GameType.hpp>
#include <essencio/model/VertexKey.hpp>
#include <essencio/model/WindowsModel.hpp>
#include <string>
#include <tinyxml2.h>

#include "util/log.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

void UI::DockSpace() {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    if (opt_fullscreen) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("DockSpace", nullptr, window_flags);

    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    static bool initialized = false;
    if (!initialized) {
        initialized = true;

        ImGui::DockBuilderRemoveNode(dockspace_id); // clear previous
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main_id = dockspace_id;

        // Step 1: Split horizontally
        ImGuiID dock_id_left   = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
        ImGuiID dock_id_right  = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
        ImGuiID dock_id_center = dock_main_id;

        // Step 2: Split center vertically for Viewport (top) and Console (bottom)
        ImGuiID dock_id_console = ImGui::DockBuilderSplitNode(dock_id_center, ImGuiDir_Down, 0.25f, nullptr, &dock_id_center);

        // Step 3: Dock windows
        ImGui::DockBuilderDockWindow("Explorer", dock_id_left);
        ImGui::DockBuilderDockWindow("Properties", dock_id_right);
        ImGui::DockBuilderDockWindow("Viewport", dock_id_center);
        ImGui::DockBuilderDockWindow("Console", dock_id_console);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::End();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UI::MainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Data Root...")) {
                IGFD::FileDialogConfig config;
                config.path = ".";

                auto dataRoot = Context::Get().GetDataRoot();
                if (dataRoot) {
                    config.path = (*dataRoot).string();
                }

                ImGuiFileDialog::Instance()->OpenDialog("ChooseDataRoot", "Choose Data Root", nullptr, config);
            }

            if (ImGui::BeginMenu("Select Game Type")) {
                auto gameType = Context::Get().GetGameType();

                if (ImGui::MenuItem("MySims", nullptr, 
                    gameType == essencio::GameType::MYSIMS)) {
                        Context::Get().ChangeGameType(essencio::GameType::MYSIMS);
                    }

                if (ImGui::MenuItem("MySims Kingdom", nullptr,
                    gameType == essencio::GameType::KINGDOM)) {
                        Context::Get().ChangeGameType(essencio::GameType::KINGDOM);
                    }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) {
                Context::Quit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {

            if (ImGui::MenuItem("Explorer", nullptr, 
                Context::Get().mShowExplorer)) {
                    Context::Get().mShowExplorer = !Context::Get().mShowExplorer;
                }

            if (ImGui::MenuItem("Properties", nullptr, 
                Context::Get().mShowProperties)) {
                    Context::Get().mShowProperties = !Context::Get().mShowProperties;
                }

            if (ImGui::MenuItem("Console", nullptr, 
                Context::Get().mShowConsole)) {
                    Context::Get().mShowConsole = !Context::Get().mShowConsole;
                }

            ImGui::Separator();

            if (ImGui::MenuItem("Wireframe Mode", nullptr, 
                Context::Get().mWireframeMode)) {
                    Context::Get().mWireframeMode = !Context::Get().mWireframeMode;
                }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseDataRoot", ImGuiWindowFlags_NoCollapse, ImVec2(500, 250))) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::string directoryPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            auto dataRoot = Context::FindDataRoot(directoryPath);

            if (!dataRoot) {
                LOG_WARN("The selected directory is not a MySims data directory!");
            } else {
                if (*dataRoot != directoryPath) {
                    LOG_INFO("Automatically detected data root at %s", dataRoot->string().c_str());
                }
                Context::Get().ChangeDataRoot(dataRoot);
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }
}
