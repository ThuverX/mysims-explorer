#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include "essencio/model/WindowsMesh.hpp"

#include "Renderer.hpp"

struct ModelData {
    std::string path;
    std::vector<MeshHandle> meshes;
};

struct MaterialData {
    std::string path;
    TextureHandle texture;
};

class Loader {
private:
    std::unordered_map<std::string, ModelData> models;
    std::unordered_map<std::string, MaterialData> materials;

    // Mesh data helpers
    static std::vector<float> GetMeshVertices(const essencio::WindowsMesh &mesh);
    static std::vector<uint32_t> GetMeshIndices(const essencio::WindowsMesh &mesh);

    // Path helpers
    static std::optional<std::string> FindMaterialPath(const std::string &fileName, const std::string &modelPath = "");
    static std::optional<std::string> FindTexturePath(const std::string &fileName);

public:
    std::optional<ModelData> LoadModel(const std::string &path);
    std::optional<MaterialData> LoadMaterial(const std::string &path);
};
