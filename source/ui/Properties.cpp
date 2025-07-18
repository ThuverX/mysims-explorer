#include "Properties.hpp"

#include "imgui.h"
#include "Context.hpp"

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UI::Properties::DrawModel(Loader &loader) {
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
                        if (ImGui::ImageButton(materialButtonLabel.c_str(), texture, ImVec2(128, 128))) {
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

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UI::Properties::DrawMaterial(Loader &loader) {
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
                            if (ImGui::CollapsingHeader(resourceKeyLabel.c_str())) {
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

void UI::Properties::Draw() {
    ImGui::Begin("Properties");

    auto &loader = Context::Get().GetLoader();

    switch (Context::Get().GetContextType()) {
        case ContextType::MODEL:
            DrawModel(loader);
            break;
        case ContextType::MATERIAL:
            DrawMaterial(loader);
            break;
        default:
            break;
    }

    ImGui::End();
}

