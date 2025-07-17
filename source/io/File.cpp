#include "File.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

std::optional<std::vector<uint8_t>> File::ReadFile(const std::string &path) {

    std::ifstream in_file(path, std::ios::binary | std::ios::ate);
    if (!in_file) {
        return std::nullopt;
    }

    std::streamsize in_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<uint8_t> in_buffer(in_size);
    if (!in_file.read(reinterpret_cast<char*>(in_buffer.data()), in_size)) {
        return std::nullopt;
    }

    return in_buffer;
}

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

DirectoryEntry File::BuildDirectoryTree(const fs::path &directory) {
    DirectoryEntry node;
    node.path = directory;
    node.name = directory.filename().string();
    node.isDirectory = fs::is_directory(directory);

    if (node.isDirectory) {
        for (const auto& entry : fs::directory_iterator(directory)) {
            node.children.push_back(BuildDirectoryTree(entry.path()));
        }
    }

    return node;
}
