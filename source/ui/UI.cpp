#include "UI.hpp"

#include "Context.hpp"
#include <essencio/GameType.hpp>
#include <essencio/model/VertexKey.hpp>
#include <essencio/model/WindowsModel.hpp>
#include <string>
#include <tinyxml2.h>

#include "essencio/material/MaterialParameter.hpp"
#include "util/log.hpp"
#include "gfx/Renderer.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileDialog.h"

#include "io/Logger.hpp"

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

void UI::DrawDirectoryEntry(const DirectoryEntry &entry) {
    if (entry.isDirectory) {
        if (ImGui::TreeNodeEx(entry.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
            for (const auto& child : entry.children) {
                DrawDirectoryEntry(child);
            }
            ImGui::TreePop();
        }
    } else {
        const auto& path = entry.path;
        const auto& assetMap = Context::Get().GetAssetMap();

        const ContextType type = Context::GetExtensionContextType(path.extension().string());

        if (type == ContextType::NONE) {
            ImGui::BeginDisabled(true);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        std::string displayName = entry.name;
        // Currently, we're only translating asset map names for Kingdom
        // MySims seems to have some weirdness going on in terms of uniqueness
        if (assetMap && Context::Get().GetGameType() == essencio::GameType::KINGDOM) {
            auto mapping = assetMap->Get(path.stem().string());
            if (mapping) {
                displayName = *mapping + path.extension().string();
            }
        }

        bool isCurrentFile = Context::Get().GetCurrentFile() == path;
        if (ImGui::Selectable(displayName.c_str(), isCurrentFile)) {
            Context::Get().SetNextFile(path.string());
        }

        if (type == ContextType::NONE) {
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }
    }
}

void UI::Explorer() {
    ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    const auto& context = Context::Get();
    const auto& root = context.GetRootDirectory();

    if (!root.path.empty()) {
        for (const auto& child : root.children) {
            DrawDirectoryEntry(child);
        }
    } else {
        ImGui::TextUnformatted("No valid game root selected.");
    }

    ImGui::End();
}

void UI::DrawModelProperties(Loader &loader) {
    for (auto &pair : loader.GetModels()) {
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
            // Native mesh data loaded through Essencio
            const auto &mesh = model.meshes[i];
            // Custom loaded data from Loader
            auto &meshData = pair.second.meshes[i];

            std::string meshLabel = "Mesh #" + std::to_string(i + 1);
            if (ImGui::CollapsingHeader(meshLabel.c_str())) {
                ImGui::Indent();

                std::string isVisibleLabel = "Is Visible##" + std::to_string(i);
                ImGui::Checkbox(isVisibleLabel.c_str(), &meshData.isVisible);

                std::string materialLabel = "Material##" + std::to_string(i);
                if (ImGui::CollapsingHeader(materialLabel.c_str())) {
                    ImGui::Indent();

                    GLuint texture = meshData.handle.texture;

                    if (texture != 0) {
                        std::string materialButtonLabel = "MaterialButton##" + std::to_string(i);
                        if (ImGui::ImageButton(materialButtonLabel.c_str(), (void*)(intptr_t)texture, ImVec2(128, 128))) {
                            Context::Get().SetNextFile(meshData.material.path.c_str());
                        }
                    }

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
    const auto &gameType = Context::Get().GetGameType();

    for (const auto &material : loader.GetMaterials()) {
        const auto &data = material.second.data;

        if (gameType == essencio::GameType::MYSIMS) {
            ImGui::Text("Header Size: %d", data.headerSize);
            ImGui::Text("Total Size: %d", data.totalSize);
            ImGui::Text("Version: %d", data.version);

            ImGui::Text("Material Hash: 0x%X", data.materialHash);
            ImGui::Text("Shader Hash: 0x%X", data.shaderHash);
        }

        ImGui::Text("Data Size: %d", data.dataSize);

        ImGui::Text("Parameter Size: %d", data.paramSize);
        ImGui::Text("# of Parameters: %d", data.paramCount);

        for (uint32_t i = 0; i < data.params.size(); ++i) {
            const auto &param = data.params[i];
            const std::string paramLabel = "Parameter #" + std::to_string(i + 1);

            if (ImGui::CollapsingHeader(paramLabel.c_str())) {
                ImGui::Indent();
                std::string valueType;

                switch (param.valueType) {
                    case essencio::MaterialParameterType::COLOR: valueType = "COLOR"; break;
                    case essencio::MaterialParameterType::VALUE: valueType = "VALUE"; break;
                    case essencio::MaterialParameterType::RESOURCE_KEY: valueType = "RESOURCE_KEY"; break;
                }

                ImGui::Text("Type: 0x%X", param.type);

                ImGui::Text("Value Type: %s", valueType.c_str());
                ImGui::Text("# of Value Fields: %d", param.valueFieldCount);
                ImGui::Text("Offset: %d", param.offset);

                switch (param.valueType) {
                    case essencio::MaterialParameterType::COLOR:
                        {
                            std::string colorLabel = "Color##" + std::to_string(i);
                            if (ImGui::CollapsingHeader(colorLabel.c_str())) {
                                ImGui::Indent();

                                for (const auto &channel : param.color) {
                                    ImGui::Text("Channel: %.f", channel);
                                }

                                ImGui::Unindent();
                            }
                        }
                        break;
                    case essencio::MaterialParameterType::VALUE:
                        {
                            std::string valueLabel = "Value##" + std::to_string(i);
                            if (ImGui::CollapsingHeader(valueLabel.c_str())) {
                                ImGui::Indent();
                                ImGui::Text("Value: %d", param.value);
                                ImGui::Unindent();
                            }
                        }
                        break;
                    case essencio::MaterialParameterType::RESOURCE_KEY:
                        {
                            std::string resourceKeyLabel = "ResourceKey##" + std::to_string(i);
                            if (ImGui::CollapsingHeader("ResourceKey")) {
                                ImGui::Indent();

                                ImGui::Text("Type: 0x%X", param.mapKey.type);
                                ImGui::Text("Group: 0x%X", param.mapKey.group);
                                ImGui::Text("Instance: 0x%llX", param.mapKey.instance);

                                ImGui::Unindent();
                            }
                        }
                        break;
                }

                ImGui::Unindent();
            }
        }
    }
}

void UI::Properties() {
    ImGui::Begin("Properties");

    auto &loader = Context::Get().GetLoader();

    switch (Context::Get().GetContextType()) {
        case ContextType::MODEL:
            DrawModelProperties(loader);
            break;
        case ContextType::MATERIAL:
            DrawMaterialProperties(loader);
            break;
        default:
            break;
    }

    ImGui::End();
}

void UI::Console() {
    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    const auto &history = Logger::Get().GetHistory();
    static uint64_t lastEntryId = 0;

    for (const auto& entry : history) {
        ImVec4 color;

        switch (entry.level) {
            case LogLevel::ERROR:
                color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // red
                break;
            case LogLevel::SUCCESS:
                color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // green
                break;
            case LogLevel::WARN:
                color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f); // orange/yellow
                break;
            case LogLevel::INFO:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // white
                break;
            default:
                color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // gray
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (!history.empty()) {
        uint64_t latestId = history.back().id;
        if (latestId != lastEntryId) {
            ImGui::SetScrollHereY(1.0f);
            lastEntryId = latestId;
        }
    }

    ImGui::End();
}

void UI::DrawSceneViewport(FramebufferHandle &framebuffer, const ImVec2 &size) {
    auto &camera = Context::Get().GetCamera();

    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    if (width > 0 && height > 0 && (width != framebuffer.width || height != framebuffer.height)) {
        // TODO: Handle framebuffer resizing through Context
        Renderer::DestroyFramebuffer(framebuffer);
        framebuffer = Renderer::CreateFramebuffer(width, height);
    }

    ImGui::Image((ImTextureID)(intptr_t)framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));

    if (ImGui::IsWindowHovered()) {
        ImGuiIO& io = ImGui::GetIO();

        float deltaX = io.MouseDelta.x;
        float deltaY = io.MouseDelta.y;

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            camera.Orbit(deltaX * 0.3f,  deltaY * 0.3f);
        }

        if (io.MouseWheel != 0.0f) {
            camera.Zoom(io.MouseWheel * 0.6f);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            camera.Pan(deltaX * 0.005f, deltaY * 0.005f);
        }
    }
}

void UI::DrawXmlElement(const XmlNode &node, int &nodeIndex) {
    std::string nodeLabel = node.name;
    if (!node.text.empty()) {
        nodeLabel += " = " + node.text;
    }
    nodeLabel += "##" + std::to_string(nodeIndex);

    if (ImGui::TreeNode(nodeLabel.c_str())) {

        for (const auto &attr : node.attributes) {
            ImGui::Text("%s = %s", attr.first.c_str(), attr.second.c_str());
        }

        for (const auto &child : node.children) {
            DrawXmlElement(child, ++nodeIndex);
        }

        ImGui::TreePop();
    }
}

void UI::Viewport(ContextType type, FramebufferHandle &framebuffer) {
    ImGui::Begin("Viewport");

    auto &loader = Context::Get().GetLoader();
    ImVec2 size = ImGui::GetContentRegionAvail();

    switch (type) {
        case ContextType::MODEL:
            UI::DrawSceneViewport(framebuffer, size);
            break;
        case ContextType::MATERIAL:
            {
                const auto &materials = loader.GetMaterials();

                if (materials.size() > 0) {
                    TextureHandle texture = materials.begin()->second.texture;
                    ImGui::Image(texture, ImVec2(256, 256));
                }
            }
            break;
        case ContextType::MATERIALSET:
            {
                const auto &materials = loader.GetMaterials();
                size_t count = 0;

                for (const auto &material : materials) {
                    std::string materialLabel = "Material##" + std::to_string(count++);
                    TextureHandle texture = material.second.texture;
                    if (ImGui::ImageButton(materialLabel.c_str(), texture, ImVec2(256, 256))) {
                        Context::Get().SetNextFile(material.second.path);
                    }
                }
            }
            break;
        case ContextType::XML:
            {
                const auto &xml = loader.GetXml();

                if (xml.size() > 0) {
                    const auto &root = xml.begin()->second.root;
                    int nodeIndex = 0;
                    DrawXmlElement(root, nodeIndex);
                }
            }
            break;
        default:
            // Nothing to do here
            break;
    }

    ImGui::End();
}

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
                Context::Get().Quit();
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
