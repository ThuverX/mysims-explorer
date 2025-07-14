#include "Loader.hpp"

#include <filesystem>

#include "Renderer.hpp"
#include "essencio/BinReader.hpp"
#include "essencio/model/WindowsModel.hpp"
#include "essencio/material/Material.hpp"

#include "Context.hpp"
#include "File.hpp"

#include <gli/load.hpp>

#include "macros/log.hpp"

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
        LOG_WARN("No FLOAT3 position key found in vertexKeys!");
        return vertices;
    }
    if (uvOffset == 0xFFFFFFFF) {
        LOG_WARN("No FLOAT2 UV key found in vertexKeys!");
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

ModelData *Loader::LoadModel(const std::string &path, const essencio::GameType &gameType) {

    auto loaded = models.find(path);
    if (loaded != models.end()) {
        return &loaded->second;
    }
    
    ModelData modelData;
    modelData.path = path;

    auto file = File::ReadFile(path);
    if (!file) {
        LOG_ERROR("Failed to open/read file: %s", path.c_str());
        return nullptr;
    }

    essencio::BinReader reader(file.value().data(), file.value().size());
    essencio::WindowsModel::Read(modelData.data, reader);

    for (const auto &mesh : modelData.data.meshes) {
        std::vector<float> vertices = GetMeshVertices(mesh);
        std::vector<uint32_t> indices = GetMeshIndices(mesh);

        MeshData meshData;
        meshData.handle = Renderer::CreateMesh({
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
                meshData.material = *materialData;
                // Attach this material to the current mesh
                meshData.handle.texture = materialData->texture;
            }
        }

        modelData.meshes.emplace_back(meshData);
    }

    auto result = models.insert({modelData.path, std::move(modelData)});
    LOG_INFO("Loaded model at path %s", path.c_str());
    return &result.first->second;
}

MaterialData *Loader::LoadMaterial(const std::string &path, const essencio::GameType &gameType) {

    auto loaded = materials.find(path);
    if (loaded != materials.end()) {
        return &loaded->second;
    }
    
    MaterialData materialData;
    materialData.path = path;

    auto file = File::ReadFile(path);
    if (!file) {
        LOG_ERROR("Failed to open/read file: %s", path.c_str());
        return nullptr;
    }

    essencio::BinReader reader(file.value().data(), file.value().size());
    essencio::Material::Read(materialData.data, reader, gameType);

    // Read material
    for (const auto &param : materialData.data.data.params) {
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
                            LOG_WARN("Failed to load texture: %s", (*texturePath).c_str());
                            return nullptr;
                        }

                        gli::gl GL(gli::gl::PROFILE_GL33);
                        gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
                        GLsizei const levels = static_cast<GLsizei>(texture.levels());

                        materialData.texture = Renderer::CreateTexture({
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

    auto result = materials.insert({materialData.path, std::move(materialData)});
    LOG_INFO("Loaded material at path %s", path.c_str());
    return &result.first->second;
}

void Loader::UnloadAll() {
    for (auto &material : materials) {
        Renderer::DestroyTexture(material.second.texture);
    }
    materials.clear();

    for (auto &model : models) {
        for (auto &mesh : model.second.meshes) {
            Renderer::DestroyMesh(mesh.handle);
        }
    }
    models.clear();
}
