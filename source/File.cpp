#include "File.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

// TODO: Use better error handling system
std::vector<uint8_t> File::ReadFile(const char *path) {

    std::ifstream in_file(path, std::ios::binary | std::ios::ate);
    if (!in_file) {
        std::cerr << "Failed to open file " << path << std::endl;
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize in_size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);

    std::vector<uint8_t> in_buffer(in_size);
    if (!in_file.read(reinterpret_cast<char*>(in_buffer.data()), in_size)) {
        std::cerr << "Failed to read from file: " << path << std::endl;
        throw std::runtime_error("Failed to read from file");
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
