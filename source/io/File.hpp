#pragma once

#include <vector>
#include <string>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "essencio/ResourceKey.hpp"
#include "ContextType.hpp"

struct FileEntry {
    fs::path path;
    std::string name;
    std::optional<std::string> displayName; // The cached unhashed file name
    bool isDirectory;
    // Used by the explorer to hide entries if they don't match the search query
    bool isVisible = true;
    std::vector<FileEntry> children;
    ContextType type;
};

namespace File {

std::string GetResourceKeyPath(const essencio::ResourceKey &key, const std::string &extension);
FileEntry BuildFileTree(const fs::path &directory);

} // namespace File
