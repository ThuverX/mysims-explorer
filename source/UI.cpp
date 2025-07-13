#include "UI.hpp"

#include "Context.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

void UI::DrawDockSpace() {
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
        ImGuiID dock_id_right  = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
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

void UI::DrawDirectory(const fs::path &directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        const auto& path = entry.path();
        std::string name = path.filename().string();

        if (entry.is_directory()) {
            if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
                DrawDirectory(path);
                ImGui::TreePop();
            }
        } else {
            // Only display models for now
            if (path.extension() == ".0xb359c791") {
                if (ImGui::Selectable(name.c_str())) {
                    // Hacky way to quickly reload the model...
                    // TODO: Add checks to detemine how to load the currently selected file
                    Context::Get().GetLoader().UnloadAll();
                    Context::Get().GetLoader().LoadModel(path.string());
                }
            }
        }
    }
}

void UI::DrawFileExplorer() {
    ImGui::Begin("Explorer");

    auto gameRoot = Context::Get().GetGameRoot();
    if (gameRoot && fs::exists(*gameRoot)) {
        DrawDirectory(*gameRoot);
    } else {
        ImGui::TextUnformatted("No valid game root selected.");
    }

    ImGui::End();
}

void UI::DrawProperties() {
    ImGui::Begin("Properties");


    ImGui::End();
}

void UI::DrawConsole() {
    ImGui::Begin("Console");


    ImGui::End();
}

void UI::DrawViewport(FramebufferHandle &framebuffer) {
    ImGui::Begin("Viewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    if (width > 0 && height > 0 && (width != framebuffer.width || height != framebuffer.height)) {
        Renderer::DestroyFramebuffer(framebuffer);
        framebuffer = Renderer::CreateFramebuffer(width, height);
    }

    ImGui::Image((ImTextureID)(intptr_t)framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));

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
