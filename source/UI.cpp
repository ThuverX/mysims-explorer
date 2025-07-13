#include "UI.hpp"

#include "Context.hpp"
#include <essencio/GameType.hpp>
#include <essencio/model/VertexKey.hpp>
#include <essencio/model/WindowsModel.hpp>
#include <string>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

// testing
#include <iostream>

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
            if (ImGui::Selectable(name.c_str())) {
                Context::Get().LoadViewportFile(path);
            }
        }
    }
}

void UI::Explorer() {
    ImGui::Begin("Explorer");

    auto gameRoot = Context::Get().GetGameRoot();
    if (gameRoot && fs::exists(*gameRoot)) {
        DrawDirectory(*gameRoot);
    } else {
        ImGui::TextUnformatted("No valid game root selected.");
    }

    ImGui::End();
}

void UI::DrawModelProperties(Loader &loader) {

    for (const auto &pair : loader.GetModels()) {

        const essencio::WindowsModel &model = pair.second.data;

        ImGui::Text("Version: %d.%d", model.majorVersion, model.minorVersion);
        ImGui::Text("Min. Bounds: %.3f,%.3f,%.3f",
            model.boundsMin.x, model.boundsMin.y, model.boundsMin.z);
        ImGui::Text("Max. Bounds: %.3f,%.3f,%.3f",
            model.boundsMax.x, model.boundsMax.y, model.boundsMax.z);

        // TODO: Add extra parameter info

        for (uint32_t i = 0; i < model.rigs.size(); ++i) {
            const auto &rig = model.rigs[i];
            std::string rigLabel = "Rig #" + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(rigLabel.c_str())) {
                ImGui::Indent();
                ImGui::Text("# of Bones: %d", rig.numBones);
                ImGui::Unindent();
            }
        }

        for (uint32_t i = 0; i < model.meshes.size(); ++i) {
            const auto &mesh = model.meshes[i];
            std::string meshLabel = "Mesh #" + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(meshLabel.c_str())) {
                ImGui::Indent();

                std::string materialLabel = "Material##" + std::to_string(i);
                if (ImGui::CollapsingHeader(materialLabel.c_str())) {
                    ImGui::Indent();
                    GLuint texture = pair.second.meshes[i].texture;

                    if (texture != 0) {
                        ImGui::Image((void*)(intptr_t)texture, ImVec2(128, 128));
                    }

                    // TODO: Add possibilty to directly open material

                    ImGui::Unindent();
                }

                ImGui::Text("Min. Bounds: %.3f,%.3f,%.3f",
                    mesh.boundsMin.x, mesh.boundsMin.y, mesh.boundsMin.z);
                ImGui::Text("Max. Bounds: %.3f,%.3f,%.3f",
                    mesh.boundsMax.x, mesh.boundsMax.y, mesh.boundsMax.z);

                ImGui::Text("# of Vertices: %d", mesh.numVertices);
                ImGui::Text("# of Faces: %d", mesh.numFaces);
                ImGui::Text("# of Vertex Keys: %d", mesh.numVertexKeys);

                for (uint32_t j = 0; j < mesh.vertexKeys.size(); ++j) {
                    const auto &key = mesh.vertexKeys[j];
                    std::string keyLabel = "Vertex Key #" + std::to_string(j + 1)
                        + "##mesh" + std::to_string(i) + "_key" + std::to_string(j);

                    if (ImGui::CollapsingHeader(keyLabel.c_str())) {
                        ImGui::Indent();
                        std::string type;

                        switch (key.type) {
                            case essencio::VertexKeyType::FLOAT2: type = "FLOAT2"; break;
                            case essencio::VertexKeyType::FLOAT3: type = "FLOAT3"; break;
                            case essencio::VertexKeyType::FLOAT: type = "FLOAT"; break;
                            case essencio::VertexKeyType::UNKNOWN: type = "(Unknown)"; break;
                        }

                        ImGui::Text("Offset: %d", key.offset);
                        ImGui::Text("Type: %s", type.c_str());
                        ImGui::Text("Index: %d", key.index);
                        ImGui::Text("Sub Index: %d", key.subIndex);

                        ImGui::Unindent();
                    }
                }

                ImGui::Unindent();
            }
        }
    }
}

void UI::DrawMaterialProperties(Loader &loader) {

}

void UI::Properties() {
    ImGui::Begin("Properties");

    auto &loader = Context::Get().GetLoader();

    switch (Context::Get().GetViewportType()) {
        case ViewportType::MODEL:
            DrawModelProperties(loader);
            break;
        case ViewportType::MATERIAL:
            DrawMaterialProperties(loader);
            break;
        default:
            // TODO: Show default text here?
            break;
    }

    ImGui::End();
}

void UI::Console() {
    ImGui::Begin("Console");


    ImGui::End();
}

void UI::Viewport(FramebufferHandle &framebuffer) {
    ImGui::Begin("Viewport");

    ImVec2 size = ImGui::GetContentRegionAvail();
    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    if (width > 0 && height > 0 && (width != framebuffer.width || height != framebuffer.height)) {
        Renderer::DestroyFramebuffer(framebuffer);
        framebuffer = Renderer::CreateFramebuffer(width, height);
    }

    ImGui::Image((ImTextureID)(intptr_t)framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));

    auto &camera = Context::Get().GetCamera();

    if (ImGui::IsWindowHovered()) {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            float deltaX = io.MouseDelta.x;
            float deltaY = io.MouseDelta.y;

            camera.Orbit(deltaX * 0.2f, deltaY * 0.2f);
        }

        if (io.MouseWheel != 0.0f) {
            camera.Zoom(io.MouseWheel * 0.5f);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            float deltaX = io.MouseDelta.x;
            float deltaY = io.MouseDelta.y;

            camera.Pan(deltaX * 0.0075f, deltaY * 0.0075f);
        }
    }

    ImGui::End();
}

void UI::MainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Game Root...", "Ctrl+O")) {
                IGFD::FileDialogConfig config;
                config.path = ".";

                auto gameRoot = Context::Get().GetGameRoot();
                if (gameRoot) {
                    config.path = (*gameRoot).string();
                }

                ImGuiFileDialog::Instance()->OpenDialog("ChooseGameRoot", "Choose Game Root", nullptr, config);
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

    if (ImGuiFileDialog::Instance()->Display("ChooseGameRoot", ImGuiWindowFlags_NoCollapse, ImVec2(500, 250))) {
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
                    Context::Get().ChangeGameRoot(gameRoot);
                }
            }
        }

        ImGuiFileDialog::Instance()->Close();
    }
}
