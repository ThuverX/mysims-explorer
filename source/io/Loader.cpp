#include "Loader.hpp"

#include <filesystem>

#include "gfx/Renderer.hpp"
#include "essencio/BinReader.hpp"
#include "essencio/model/WindowsModel.hpp"
#include "essencio/material/Material.hpp"
#include "essencio/material/MaterialSet.hpp"

#include "Context.hpp"
#include "File.hpp"

#include <gli/load.hpp>
#include <tinyxml2.h>

#include "io/File.hpp"
#include "util/log.hpp"

namespace fs = std::filesystem;

std::vector<float> Loader::GetMeshVertices(const essencio::WindowsMesh &mesh) {
    std::vector<float> vertices;

    uint32_t positionOffset = UINT32_MAX;
    std::vector<uint32_t> uvOffsets;

    for (const auto& key : mesh.vertexKeys) {
        //if (key.index == 0 && key.type == essencio::VertexKeyType::FLOAT3) {
        if (key.type == essencio::VertexKeyType::FLOAT3 && positionOffset == UINT32_MAX) {
            positionOffset = key.offset;
        } else if (key.type == essencio::VertexKeyType::FLOAT2) {
            uvOffsets.emplace_back(key.offset);
        }
    }

    if (positionOffset == UINT32_MAX) {
        LOG_WARN("No FLOAT3 position key found in vertexKeys!");
        return vertices;
    }
    if (uvOffsets.size() == 0) {
        LOG_WARN("No FLOAT2 UV key found in vertexKeys!");
        return vertices;
    }

    size_t stride = mesh.vertexArraySize / mesh.numVertices;
    size_t vertex_size = 3 + 2 + 2; // position - uv0 - uv1
    vertices.reserve(static_cast<size_t>(mesh.numVertices) * vertex_size);

    for (size_t i = 0; i < mesh.numVertices; ++i) {
        size_t base = i * stride;

        float x = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 0]);
        float y = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 4]);
        float z = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 8]);

        float u0 = 0;
        float v0 = 0;
        float u1 = 0;
        float v1 = 0;

        if (uvOffsets.size() > 0) {
            u0 = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffsets[0] + 0]);
            v0 = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffsets[0] + 4]);
        }

        if (uvOffsets.size() > 1) {
            u1 = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffsets[1] + 0]);
            v1 = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffsets[1] + 4]);
        }

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(u0);
        vertices.push_back(v0);
        vertices.push_back(u1);
        vertices.push_back(v1);
    }

    return vertices;
}

std::vector<uint32_t> Loader::GetMeshIndices(const essencio::WindowsMesh &mesh) {
    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(mesh.numFaces) * 3);

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

    static std::vector<fs::path> searchPaths = {
        fs::path("GameData") / "Characters",
        fs::path("GameData") / "Levels",
        fs::path("GameData") / "Objects",
        fs::path("GameData") / "Textures",
        fs::path("GameData") / "UI",
    };

    for (const auto &path : searchPaths) {
        const std::optional<fs::path> dataRoot = Context::Get().GetDataRoot();

        if (!dataRoot) {
            return std::nullopt;
        }

        const fs::path materialPath = *dataRoot / path / fileName;

        if (fs::exists(materialPath) && !fs::is_directory(materialPath)) {
            return materialPath.string();
        }
    }

    return std::nullopt;
}

std::optional<std::string> Loader::FindTexturePath(const std::string &fileName) {

    static std::vector<fs::path> searchPaths = {
        fs::path("GameData_Win64") / "Textures" / "Characters",
        fs::path("GameData_Win64") / "Textures" / "Levels",
        fs::path("GameData_Win64") / "Textures" / "Objects",
        fs::path("GameData_Win64") / "Textures" / "Textures",
        fs::path("GameData_Win64") / "Textures" / "UI",
    };

    for (const auto &path : searchPaths) {
        const std::optional<fs::path> dataRoot = Context::Get().GetDataRoot();

        if (!dataRoot) {
            return std::nullopt;
        }

        const fs::path texturePath = *dataRoot / path / fileName;

        if (fs::exists(texturePath) && !fs::is_directory(texturePath)) {
            return texturePath.string();
        }
    }

    return std::nullopt;
}

XmlNode Loader::BuildXmlNodeTree(const tinyxml2::XMLElement *element) {
    XmlNode node;
    node.name = element->Name();

    for (const tinyxml2::XMLAttribute* attr = element->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
        node.attributes[attr->Name()] = attr->Value();
    }

    for (const tinyxml2::XMLElement* child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        node.children.push_back(BuildXmlNodeTree(child));
    }

    if (element->GetText() != nullptr) {
        node.text = element->GetText();
    }

    return node;
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
        meshData.data = mesh;

        // Pre-calculating mesh bounds
        {
            meshData.boundsMin = glm::vec3(
                mesh.boundsMin.x,
                mesh.boundsMin.y,
                mesh.boundsMin.z
            );
            meshData.boundsMax = glm::vec3(
                mesh.boundsMax.x,
                mesh.boundsMax.y,
                mesh.boundsMax.z
            );

            meshData.boundsCenter = (meshData.boundsMin + meshData.boundsMax) * 0.5f;
            meshData.boundsSize = (meshData.boundsMax - meshData.boundsMin);
        }

        meshData.handle = Renderer::CreateMesh({
            vertices,
            indices,
        });

        const auto &resourcePath = File::GetResourceKeyPath(mesh.material, "Material");

        if (auto materialPath = FindMaterialPath(resourcePath, path)) {
            auto *materialData = LoadMaterial(*materialPath, gameType);

            if (materialData != nullptr) {
                meshData.materials.push_back(materialData);
            } else {
                LOG_WARN("Material data could not be loaded");
            }
        } else {
            const auto &resourcePathSet = File::GetResourceKeyPath(mesh.material, "MaterialSet");

            if (auto materialSetPath = FindMaterialPath(resourcePathSet)) {
                std::vector<MaterialData *> materialSet = LoadMaterialSet(*materialSetPath, gameType);

                for (const auto &material: materialSet) {
                    meshData.materials.push_back(material);
                }
            } else {
                LOG_WARN("Material path not found for resource %s", resourcePath.c_str());
            }
        }

        modelData.meshes.emplace_back(meshData);
    }

    auto result = models.insert({modelData.path, std::move(modelData)});
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

    materialData.shaderHandle = Context::Get().GetShaderHandle(static_cast<ShaderId>(materialData.data.shaderHash));

    for (const auto & param : materialData.data.params) {
        if (param.valueType == essencio::MaterialParameterType::RESOURCE_KEY) {
            const auto &resourcePath = File::GetResourceKeyPath(param.mapKey, "dds");
            auto texturePath = FindTexturePath(resourcePath);

            if (texturePath) {
                // Load the DDS texture using gli
                gli::texture texture = gli::load(texturePath->c_str());
                if (texture.empty()) {
                    LOG_WARN("Failed to load texture: %s", texturePath->c_str());
                    return nullptr;
                }

                gli::gl GL(gli::gl::PROFILE_GL33);
                gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
                auto const levels = static_cast<GLsizei>(texture.levels());

                materialData.textures[param.type] = Renderer::CreateTexture({
                    texture,
                    format,
                    levels,
                });
            }
        }
    }

    auto result = materials.insert({materialData.path, std::move(materialData)});
    return &result.first->second;
}

std::vector<MaterialData *> Loader::LoadMaterialSet(const std::string &path, const essencio::GameType &gameType) {

    std::vector<MaterialData *> materialSetData;
    materialSetData.clear();

    auto file = File::ReadFile(path);
    if (!file) {
        LOG_ERROR("Failed to open/read file: %s", path.c_str());
        return materialSetData;
    }

    essencio::MaterialSet materialSet;

    essencio::BinReader reader(file.value().data(), file.value().size());
    essencio::MaterialSet::Read(materialSet, reader, gameType);

    for (const auto &material : materialSet.materials) {

        auto resourceKey = File::GetResourceKeyPath(material, "Material");
        auto materialPath = FindMaterialPath(resourceKey);

        if (!materialPath) {
            LOG_WARN("Could not find mateial path %s in material set", resourceKey.c_str());
            continue;
        }

        MaterialData *data = LoadMaterial(*materialPath, gameType);
        materialSetData.push_back(data);
    }

    return materialSetData;
}

XmlData *Loader::LoadXml(const std::string &path) {

    tinyxml2::XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        LOG_ERROR("Failed to load XML file");
        return nullptr;
    }

    XmlData xmlData;
    xmlData.path = path;
    xmlData.root = BuildXmlNodeTree(doc.RootElement());

    auto result = xml.insert({xmlData.path, std::move(xmlData)});
    return &result.first->second;
}

void Loader::UnloadAll() {
    xml.clear();

    for (auto &material : materials) {
        for (auto &texture : material.second.textures) {
            Renderer::DestroyTexture(texture.second);
        }
    }
    materials.clear();

    for (auto &model : models) {
        for (auto &mesh : model.second.meshes) {
            Renderer::DestroyMesh(mesh);
        }
    }
    models.clear();
}
