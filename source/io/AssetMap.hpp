#pragma once

#include <unordered_map>
#include <string>
#include <optional>

class AssetMap {
private:
    std::unordered_map<std::string, std::string> mappings;

public:
    static std::optional<AssetMap> Read(const std::string &path);
    std::optional<std::string> Get(const std::string &key) const;
};
