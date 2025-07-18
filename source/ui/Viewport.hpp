#pragma once

#include "Context.hpp"
#include "imgui.h"

namespace UI::Viewport {

void DrawXml(const XmlNode &node, int &nodeIndex);
void DrawScene(FramebufferHandle &framebuffer, const ImVec2 &size);

void Draw(ContextType type, FramebufferHandle &framebuffer);

} // UI::Viewport
