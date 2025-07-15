#include "AssetMap.hpp"

#include "macros/log.hpp"
#include "tinyxml2.h"
#include <optional>
#include <sstream>

using namespace tinyxml2;

std::optional<AssetMap> AssetMap::Read(const std::string &path) {
    XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        LOG_ERROR("Failed to XML file: %s", path.c_str());
        return std::nullopt;
    }

    AssetMap assetMap;

    XMLElement *root = doc.FirstChildElement("mappings");
    if (root) {
        XMLElement *type = root->FirstChildElement();
        while (type) {
            XMLElement *mapping = type->FirstChildElement("mapping");
            while (mapping) {
                const char *key = mapping->Attribute("key");
                const char *name = mapping->Attribute("name");

                // XMLElement *info = mapping->FirstChildElement("info");
                // if (info) {
                //     const char *sourcePath = info->Attribute("sourcePath");
                //     LOG_TRACE("%s", sourcePath);
                // }

                std::stringstream ss(key);
                std::string type, group, instance;

                std::getline(ss, type, ':');
                std::getline(ss, group, ':');
                std::getline(ss, instance, ':');

                std::string mappingKey = "0x" + group + "!0x" + instance;

                assetMap.mappings.insert({mappingKey, name});
                mapping = mapping->NextSiblingElement("mapping");
            }

            type = type->NextSiblingElement();
        }
    }

    return assetMap;
}

std::optional<std::string> AssetMap::Get(const std::string &key) const {
    auto it = mappings.find(key);

    if (it != mappings.end()) {
        return std::make_optional(it->second);
    }

    return std::nullopt;
}
