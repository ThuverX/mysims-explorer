#include "AssetMap.hpp"

#include "util/log.hpp"
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
    if (root != nullptr) {
        XMLElement *type = root->FirstChildElement();
        while (type != nullptr) {
            XMLElement *mapping = type->FirstChildElement("mapping");
            while (mapping != nullptr) {
                const char *key = mapping->Attribute("key");
                const char *name = mapping->Attribute("name");

                std::stringstream input(key);
                std::string type, group, instance;

                std::getline(input, type, ':');
                std::getline(input, group, ':');
                std::getline(input, instance, ':');

                std::stringstream output;
                output << "0x";
                output << group;
                output << "!0x";
                output << instance;

                assetMap.mappings.insert({output.str(), name});
                mapping = mapping->NextSiblingElement("mapping");
            }

            type = type->NextSiblingElement();
        }
    }

    return assetMap;
}

std::optional<std::string> AssetMap::Get(const std::string &key) const {
    auto iterator = mappings.find(key);

    if (iterator != mappings.end()) {
        return std::make_optional(iterator->second);
    }

    return std::nullopt;
}
