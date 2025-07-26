#include "File.hpp"
#include "Context.hpp"

#include <filesystem>
#include <sstream>
#include <iomanip>

std::string File::GetResourceKeyPath(const essencio::ResourceKey &key, const std::string &extension) {
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(8) << std::hex << key.group;
    stream << "!";
    stream << "0x" << std::setfill('0') << std::setw(16) << std::hex << key.instance;

    if (!extension.empty()) {
        stream << ".";
        stream << extension;
    }

    return stream.str();
}

FileEntry File::BuildFileTree(const fs::path &directory) {
    FileEntry node;
    node.path = directory;
    node.name = directory.filename().string();
    node.isDirectory = fs::is_directory(directory);

    if (node.isDirectory) {
        for (const auto& entry : fs::directory_iterator(directory)) {
            node.children.push_back(BuildFileTree(entry.path()));
        }
    } else {
        node.type = Context::GetExtensionContextType(directory.extension().string());
    }

    return node;
}
