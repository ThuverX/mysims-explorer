#pragma once

#include "Renderer.hpp"
#include "Loader.hpp"

#include <filesystem>
namespace fs = std::filesystem;

namespace UI {

void DockSpace();
void DrawDirectory(const fs::path &directory);
void Explorer();

void DrawModelProperties(Loader &loader);
void DrawMaterialProperties(Loader &loader);
void Properties();

void Console();
void Viewport(FramebufferHandle &framebuffer);
void MainMenuBar();

} // namespace UI
