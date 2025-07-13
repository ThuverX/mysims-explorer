#include "Loader.hpp"

#include <filesystem>

#include "Renderer.hpp"
#include "essencio/BinReader.hpp"
#include "essencio/model/WindowsModel.hpp"
#include "essencio/material/Material.hpp"

#include "Context.hpp"
#include "File.hpp"

#include <gli/load.hpp>

// TODO: Replace with logging system
#include <iostream>

namespace fs = std::filesystem;

std::vector<float> Loader::GetMeshVertices(const essencio::WindowsMesh &mesh) {
    std::vector<float> vertices;

    uint32_t positionOffset = 0xFFFFFFFF;
    uint32_t uvOffset = 0xFFFFFFFF;

    for (const auto& key : mesh.vertexKeys) {
        //if (key.index == 0 && key.type == essencio::VertexKeyType::FLOAT3) {
        if (key.type == essencio::VertexKeyType::FLOAT3 && positionOffset == 0xFFFFFFFF) {
            positionOffset = key.offset;
        } else if (key.type == essencio::VertexKeyType::FLOAT2 && uvOffset == 0xFFFFFFFF) {
            uvOffset = key.offset;
        }
    }

    // TODO: Throw exceptions instead?
    if (positionOffset == 0xFFFFFFFF) {
        std::cerr << "No FLOAT3 position key found in vertexKeys!" << std::endl;
        return vertices;
    }
    if (uvOffset == 0xFFFFFFFF) {
        std::cerr << "No FLOAT2 UV key found in vertexKeys!" << std::endl;
        return vertices;
    }

    size_t stride = mesh.vertexArraySize / mesh.numVertices;
    vertices.reserve(mesh.numVertices * 5); // 3 for pos + 2 for UV

    for (size_t i = 0; i < mesh.numVertices; ++i) {
        size_t base = i * stride;

        float x = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 0]);
        float y = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 4]);
        float z = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 8]);

        float u = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffset + 0]);
        float v = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffset + 4]);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(u);
        vertices.push_back(v);
    }

    return vertices;
}

std::vector<uint32_t> Loader::GetMeshIndices(const essencio::WindowsMesh &mesh) {
    std::vector<uint32_t> indices;
    indices.reserve(mesh.numFaces * 3);

    for (const auto& face : mesh.faces) {
        indices.push_back(face.a);
        indices.push_back(face.b);
        indices.push_back(face.c);
    }

    return indices;
}

std::optional<std::string> Loader::FindMaterialPath(const std::string &fileName, const std::string &modelPath) {

    if (!modelPath.empty()) {
        // Find the material file in the same directory as the model first
        fs::path modelParent = fs::path(modelPath).parent_path();
        fs::path materialPath = modelParent / fileName;

        if (fs::exists(materialPath) && !fs::is_directory(materialPath)) {
            return materialPath.string();
        }
    }

    // TODO: Add multiple possible material search paths here

    return std::nullopt;
}

std::optional<std::string> Loader::FindTexturePath(const std::string &fileName) {

    static std::vector<fs::path> searchPaths = {
        fs::path("GameData_Win64") / "Textures" / "Objects",
    };

    for (const auto &path : searchPaths) {
        const std::optional<fs::path> gameRoot = Context::Get().GetGameRoot();

        if (!gameRoot) {
            return std::nullopt;
        }

        const fs::path texturePath = *gameRoot / path / fileName;

        if (fs::exists(texturePath) && !fs::is_directory(texturePath)) {
            return texturePath.string();
        }
    }

    return std::nullopt;
}

std::optional<ModelData> Loader::LoadModel(const std::string &path, const essencio::GameType &gameType) {
    ModelData data;

    std::vector<uint8_t> file = File::ReadFile(path);
    essencio::BinReader reader(file.data(), file.size());

    essencio::WindowsModel model;
    essencio::WindowsModel::Read(model, reader);

    for (const auto &mesh : model.meshes) {
        std::vector<float> vertices = GetMeshVertices(mesh);
        std::vector<uint32_t> indices = GetMeshIndices(mesh);

        MeshHandle meshHandle = Renderer::CreateMesh({
            vertices,
            indices,
        });

        auto materialPath = FindMaterialPath(
            File::GetResourceKeyPath(mesh.material, "Material"),
            path
        );

        if (materialPath) {
            auto materialData = LoadMaterial(*materialPath, gameType);

            if (materialData) {
                // Attach this material to the current mesh
                meshHandle.texture = materialData->texture;
            }
        }

        data.path = path;
        data.meshes.emplace_back(meshHandle);
    }

    // TODO: Insert using base instance hash only?
    // Not sure how groups are being affected by this...

    models.insert({data.path, data});
    return data;
}

std::optional<MaterialData> Loader::LoadMaterial(const std::string &path, const essencio::GameType &gameType) {
    MaterialData data;

    std::vector<uint8_t> file = File::ReadFile(path);
    essencio::BinReader reader(file.data(), file.size());

    essencio::Material material;
    essencio::Material::Read(material, reader, gameType);

    // Read material
    for (const auto &param : material.data.params) {
        switch (param.valueType) {
            case essencio::MaterialParameterType::RESOURCE_KEY:
                {
                    auto texturePath = FindTexturePath(
                        File::GetResourceKeyPath(param.mapKey, "dds")
                    );

                    if (texturePath) {
                        // Load the DDS texture using gli
                        gli::texture texture = gli::load((*texturePath).c_str());
                        if (texture.empty()) {
                            std::cerr << "Failed to load texture: " << *texturePath << std::endl;
                            return std::nullopt;
                        }

                        gli::gl GL(gli::gl::PROFILE_GL33);
                        gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
                        GLsizei const levels = static_cast<GLsizei>(texture.levels());

                        data.texture = Renderer::CreateTexture({
                            texture,
                            format,
                            levels,
                        });
                    }
                }
                break;
            default:
                // Just do nothing for now
                break;
        }
    }

    materials.insert({data.path, data});
    return data;
}

void Loader::UnloadAll() {
    for (auto &material : materials) {
        Renderer::DestroyTexture(material.second.texture);
    }
    materials.clear();

    for (auto &model : models) {
        for (auto &mesh : model.second.meshes) {
            Renderer::DestroyMesh(mesh);
        }
    }
    models.clear();
}
