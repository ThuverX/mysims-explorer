#pragma once

#include "Context.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"

#include <filesystem>
namespace fs = std::filesystem;

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

namespace UI {

void DockSpace();
void DrawDirectory(const fs::path &directory);
void Explorer();

void DrawModelProperties(Loader &loader);
void DrawMaterialProperties(Loader &loader);
void Properties();

void Console();

void DrawSceneViewport(FramebufferHandle &framebuffer, const ImVec2 &size);
void Viewport(ViewportType type, FramebufferHandle &framebuffer);

void MainMenuBar();

} // namespace UI
