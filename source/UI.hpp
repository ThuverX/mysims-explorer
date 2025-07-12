#pragma once

#include "Renderer.hpp"

#include <filesystem>
namespace fs = std::filesystem;

namespace UI {

void DrawDockSpace();
void DrawDirectory(const fs::path &directory);
void DrawFileExplorer();
void DrawViewport(FramebufferHandle &framebuffer);
void DrawMainMenuBar();

} // namespace UI
