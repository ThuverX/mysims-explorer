#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <optional>

#include "essencio/ResourceKey.hpp"

namespace File {

std::optional<std::vector<uint8_t>> ReadFile(const std::string &path);
std::string GetResourceKeyPath(const essencio::ResourceKey &key, const std::string &extension);

} // namespace File
