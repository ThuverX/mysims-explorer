#pragma once

#include "Context.hpp"

namespace UI::Explorer {

void DrawEntry(const FileEntry &entry);
void DrawDirectoryChildren(const std::vector<FileEntry>& children);
void Draw();

} // namespace UI::Properties
