#pragma once

#include "Context.hpp"
#include "gfx/Renderer.hpp"
#include "io/Loader.hpp"
#include "io/File.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

namespace UI {

void DockSpace();
void DrawDirectoryEntry(const DirectoryEntry &entry);
void Explorer();

void DrawModelProperties(Loader &loader);
void DrawMaterialProperties(Loader &loader);
void Properties();

void Console();

void DrawXmlElement(const XmlNode &node, int &nodeIndex);

void DrawSceneViewport(FramebufferHandle &framebuffer, const ImVec2 &size);
void Viewport(ContextType type, FramebufferHandle &framebuffer);

void MainMenuBar();

} // namespace UI
