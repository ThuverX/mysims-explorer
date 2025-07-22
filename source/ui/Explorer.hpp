#pragma once

#include "Context.hpp"

namespace UI::Explorer {

void DrawEntry(FileEntry &entry);
void DrawDirectoryChildren(std::vector<FileEntry>& children);
void Draw(UIState &state);

} // namespace UI::Properties
