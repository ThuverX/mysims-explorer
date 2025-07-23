#include "Viewport.hpp"

void UI::Viewport::DrawScene(FramebufferHandle &framebuffer, const ImVec2 &size) {
    auto &camera = Context::Get().GetCamera();

    int width = static_cast<int>(size.x);
    int height = static_cast<int>(size.y);

    if (width > 0 && height > 0 && (width != framebuffer.width || height != framebuffer.height)) {
        // TODO: Handle framebuffer resizing through Context
        Renderer::DestroyFramebuffer(framebuffer);
        framebuffer = Renderer::CreateFramebuffer(width, height);
    }

    ImGui::Image((ImTextureID)(intptr_t)framebuffer.texture, size, ImVec2(0, 1), ImVec2(1, 0));

    if (ImGui::IsWindowHovered()) {
        ImGuiIO& io = ImGui::GetIO(); // NOLINT(readability-identifier-length)

        float deltaX = io.MouseDelta.x;
        float deltaY = io.MouseDelta.y;

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            camera.Orbit(deltaX * 0.3f,  deltaY * 0.3f);
        }

        if (io.MouseWheel != 0.0f) {
            camera.Zoom(io.MouseWheel * 0.6f);
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            camera.Pan(deltaX * 0.0075f, deltaY * 0.0075f);
        }
    }
}

void UI::Viewport::DrawXml(const XmlNode &node, int &nodeIndex) {
    std::string nodeLabel = node.name;
    if (!node.text.empty()) {
        nodeLabel += " = " + node.text;
    }
    nodeLabel += "##" + std::to_string(nodeIndex);

    if (ImGui::TreeNode(nodeLabel.c_str())) {

        for (const auto &attr : node.attributes) {
            ImGui::Text("%s = %s", attr.first.c_str(), attr.second.c_str());
        }

        for (const auto &child : node.children) {
            DrawXml(child, ++nodeIndex);
        }

        ImGui::TreePop();
    }
}

void UI::Viewport::Draw(ContextType type, FramebufferHandle &framebuffer) {
    ImGui::Begin("Viewport");

    auto &loader = Context::Get().GetLoader();
    ImVec2 size = ImGui::GetContentRegionAvail();

    switch (type) {
        case ContextType::MODEL:
            DrawScene(framebuffer, size);
            break;
        case ContextType::MATERIAL:
            {
                const auto &materials = loader.GetMaterials();

                if (materials.size() > 0) {
                    TextureHandle texture = materials.begin()->second.texture;
                    ImGui::Image(texture, ImVec2(256, 256));
                }
            }
            break;
        case ContextType::MATERIALSET:
            {
                const auto &materials = loader.GetMaterials();
                size_t count = 0;

                for (const auto &material : materials) {
                    std::string materialLabel = "Material##" + std::to_string(count++);
                    TextureHandle texture = material.second.texture;
                    if (ImGui::ImageButton(materialLabel.c_str(), texture, ImVec2(256, 256))) {
                        Context::Get().SetEnqueuedFile(material.second.path);
                    }
                }
            }
            break;
        case ContextType::XML:
            {
                const auto &xml = loader.GetXml();

                if (xml.size() > 0) {
                    const auto &root = xml.begin()->second.root;
                    int nodeIndex = 0;
                    DrawXml(root, nodeIndex);
                }
            }
            break;
        default:
            // Nothing to do here
            break;
    }

    ImGui::End();
}
