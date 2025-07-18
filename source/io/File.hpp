#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "essencio/ResourceKey.hpp"

struct FileEntry {
    fs::path path;
    std::string name;
    bool isDirectory;
    std::vector<FileEntry> children;
};

namespace File {

std::optional<std::vector<uint8_t>> ReadFile(const std::string &path);
std::string GetResourceKeyPath(const essencio::ResourceKey &key, const std::string &extension);
FileEntry BuildFileTree(const fs::path &directory);

} // namespace File
