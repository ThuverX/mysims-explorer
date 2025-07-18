#include "Explorer.hpp"

#include "imgui.h"

void UI::Explorer::DrawEntry(const FileEntry &entry) {
    if (entry.isDirectory) {
        if (ImGui::TreeNodeEx(entry.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
            for (const auto& child : entry.children) {
                DrawEntry(child);
            }
            ImGui::TreePop();
        }
    } else {
        const auto& path = entry.path;
        const auto& assetMap = Context::Get().GetAssetMap();

        const ContextType type = Context::GetExtensionContextType(path.extension().string());

        if (type == ContextType::NONE) {
            ImGui::BeginDisabled(true);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        std::string displayName = entry.name;
        // Currently, we're only translating asset map names for Kingdom
        // MySims seems to have some weirdness going on in terms of uniqueness
        if (assetMap && Context::Get().GetGameType() == essencio::GameType::KINGDOM) {
            auto mapping = assetMap->Get(path.stem().string());
            if (mapping) {
                displayName = *mapping + path.extension().string();
            }
        }

        bool isCurrentFile = Context::Get().GetCurrentFile() == path;
        if (ImGui::Selectable(displayName.c_str(), isCurrentFile)) {
            Context::Get().SetNextFile(path.string());
        }

        if (type == ContextType::NONE) {
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }
    }
}

void UI::Explorer::Draw() {
    ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    const auto& context = Context::Get();
    const auto& root = context.GetRootDirectory();

    if (!root.path.empty()) {
        for (const auto& child : root.children) {
            DrawEntry(child);
        }
    } else {
        ImGui::TextUnformatted("No valid game root selected.");
    }

    ImGui::End();
}
