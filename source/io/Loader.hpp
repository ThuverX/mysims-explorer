#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include "essencio/GameType.hpp"
#include "essencio/model/WindowsModel.hpp"
#include "essencio/model/WindowsMesh.hpp"
#include "essencio/material/Material.hpp"

#include "gfx/Renderer.hpp"

#include "tinyxml2.h"

struct MaterialData {
    std::string path;
    TextureHandle texture;
    essencio::Material data;
};

struct MeshData {
    MeshHandle handle;
    std::vector<MaterialData *> materials;
    essencio::WindowsMesh data;
    // Determines whether the item should be drawn in the viewport
    bool isVisible = true;
    // Determines which material should be displayed in the scene viewport
    uint32_t materialIndex = 0;
};

struct ModelData {
    std::string path;
    std::vector<MeshData> meshes;
    essencio::WindowsModel data;
};

struct XmlNode {
    std::string name;
    std::unordered_map<std::string, std::string> attributes;
    std::vector<XmlNode> children;
    std::string text;
};

struct XmlData {
    std::string path;
    XmlNode root;
};

class Loader {
private:
    std::unordered_map<std::string, ModelData> models;
    std::unordered_map<std::string, MaterialData> materials;
    std::unordered_map<std::string, XmlData> xml;

    // Mesh data helpers
    static std::vector<float> GetMeshVertices(const essencio::WindowsMesh &mesh);
    static std::vector<uint32_t> GetMeshIndices(const essencio::WindowsMesh &mesh);

    // Path helpers
    static std::optional<std::string> FindMaterialPath(const std::string &fileName, const std::string &modelPath = "");
    static std::optional<std::string> FindTexturePath(const std::string &fileName);

    // XML loading helper
    static XmlNode BuildXmlNodeTree(const tinyxml2::XMLElement *element);

public:
    ModelData *LoadModel(const std::string &path, const essencio::GameType &gameType);
    MaterialData *LoadMaterial(const std::string &path, const essencio::GameType &gameType);
    std::vector<MaterialData *> LoadMaterialSet(const std::string &path, const essencio::GameType &gameType);
    XmlData *LoadXml(const std::string &path);

    void UnloadAll();

    inline std::unordered_map<std::string, ModelData> &GetModels() {
        return models;
    }

    inline std::unordered_map<std::string, MaterialData> &GetMaterials() {
        return materials;
    }

    inline std::unordered_map<std::string, XmlData> &GetXml() {
        return xml;
    }
};
