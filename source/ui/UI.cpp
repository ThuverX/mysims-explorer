#include "UI.hpp"

#include "Context.hpp"
#include <SDL3/SDL_misc.h>
#include <essencio/GameType.hpp>
#include <essencio/model/VertexKey.hpp>
#include <essencio/model/WindowsModel.hpp>
#include <filesystem>
#include <string>
#include <tinyxml2.h>
#include <unordered_map>

#include "util/log.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "nfd.h"
#include "version.h"

#include "IconsLucide.h"

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
            if (ImGui::BeginMenu(ICON_LC_FILE "Open Data Root")) {

#if defined(_WIN32) || defined(_WIN64)
                static std::vector<std::pair<std::string, std::string>> paths = {
                    {"MySims Kingdom (Steam)", KINGDOM_STEAM_PATH},
                    {"MySims (Steam)", MYSIMS_STEAM_PATH},
                    {"MySims Kingdom (EA)", KINGDOM_EA_PATH},
                    {"MySims (EA)", MYSIMS_EA_PATH},
                };

                for (const auto &pair : paths) {
                    bool isSelected = Context::Get().GetDataRoot() == pair.second;

                    if (ImGui::MenuItem(pair.first.c_str(), nullptr, isSelected)) {
                        auto dataRoot = Context::FindDataRoot(KINGDOM_STEAM_PATH);
                        if (!dataRoot) {
                            LOG_ERROR("Failed to automatically find data root for MySims Kingdom (Steam) at %s", KINGDOM_STEAM_PATH);
                        } else {
                            Context::Get().ChangeDataRoot(dataRoot);
                        }
                    }
                }
#else
                // Default selection options are disabled on Linux
                ImGui::MenuItem("MySims Kingdom (Steam)", nullptr, false, false);
                ImGui::MenuItem("MySims (Steam)", nullptr, false, false);
                ImGui::MenuItem("MySims Kingdom (EA)", nullptr, false, false);
                ImGui::MenuItem("MySims (EA)", nullptr, false, false);
#endif
                if (ImGui::MenuItem("Custom...")) {
                    std::string defaultPath = ".";
                    // If data root is set, use it as the default path
                    auto dataRoot = Context::Get().GetDataRoot();
                    if (dataRoot) {
                        defaultPath = (*dataRoot).string();
                    }

                    fs::path realDefaultPath = fs::absolute(defaultPath);
                    if (!fs::exists(realDefaultPath)) {
                        realDefaultPath = fs::current_path();
                    }

                    // Initialize NFD
                    nfdchar_t *outPath = nullptr;
                    nfdresult_t result = NFD_PickFolder(realDefaultPath.string().c_str(), &outPath);

                    if (result == NFD_OKAY) {
                        std::string selectedPath(outPath);
                        free(outPath);

                        auto dataRoot = Context::FindDataRoot(selectedPath);
                        if (!dataRoot) {
                            LOG_WARN("The selected directory is not a MySims data directory!");
                        } else {
                            if (*dataRoot != selectedPath) {
                                LOG_INFO("Automatically detected data root at %s", dataRoot->string().c_str());
                            }
                            Context::Get().ChangeDataRoot(dataRoot);
                        }

                        Context::Get().ChangeDataRoot(selectedPath);
                    }
                    else if (result == NFD_CANCEL) {
                        LOG_TRACE("File dialog cancelled by user");
                    }
                    else {
                        LOG_ERROR("NFD error: %s", NFD_GetError());
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(ICON_LC_GAMEPAD_2 "Force Game Type")) {
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

            if (ImGui::MenuItem(ICON_LC_DOOR_OPEN "Quit")) {
                Context::Quit();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {

            if (ImGui::MenuItem(ICON_LC_FOLDER_TREE "Explorer", nullptr, 
                Context::Get().mShowExplorer)) {
                    Context::Get().mShowExplorer = !Context::Get().mShowExplorer;
                }

            if (ImGui::MenuItem(ICON_LC_TABLE_PROPERTIES "Properties", nullptr, 
                Context::Get().mShowProperties)) {
                    Context::Get().mShowProperties = !Context::Get().mShowProperties;
                }

            if (ImGui::MenuItem(ICON_LC_SQUARE_TERMINAL "Console", nullptr, 
                Context::Get().mShowConsole)) {
                    Context::Get().mShowConsole = !Context::Get().mShowConsole;
                }

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_LC_WAYPOINTS "Wireframe Mode", nullptr, 
                Context::Get().mWireframeMode)) {
                    Context::Get().mWireframeMode = !Context::Get().mWireframeMode;
                }

            if (ImGui::MenuItem(ICON_LC_SCAN "Show Bounds", nullptr, 
                Context::Get().mShowBounds)) {
                    Context::Get().mShowBounds = !Context::Get().mShowBounds;
                }

            ImGui::Separator();

            bool isDarkTheme = Context::Get().GetIsDarkTheme();
            if (ImGui::MenuItem(ICON_LC_MOON "Dark Theme", nullptr, isDarkTheme)) {
                Context::Get().SetIsDarkTheme(!isDarkTheme);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::BeginMenu(ICON_LC_INFO "About")) {

                std::string titleLabel = std::string("MySims Explorer v") + VERSION_STRING;
                ImGui::Text("%s", titleLabel.c_str());
                ImGui::Separator();
                ImGui::TextWrapped("A tool to easily browse, inspect and view game assets from MySims and MySims Kingdom (Cozy Bundle edition).");
                ImGui::Separator();
                ImGui::Text("Created by bottledlactose");
                
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem(ICON_LC_GITHUB "GitHub")) {
                SDL_OpenURL("https://github.com/bottledlactose/mysims-explorer");
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
