#include "Explorer.hpp"

#include "imgui.h"
#include "IconsLucide.h"

void UI::Explorer::DrawEntry(FileEntry &entry) {
    if (!entry.isVisible) {
        return;
    }

    if (entry.isDirectory) {
        if (ImGui::TreeNodeEx(entry.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
            DrawDirectoryChildren(entry.children);
            ImGui::TreePop();
        }
    } else {        
        const auto& path = entry.path;
        const auto& assetMap = Context::Get().GetAssetMap();

        std::string displayName = entry.name;
        if (assetMap) {
            displayName = Context::GetFileEntryDisplayName(entry, *assetMap);
        }

        if (entry.type == ContextType::NONE) {
            ImGui::BeginDisabled(true);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }

        const char *icon = nullptr;
        switch (entry.type) {
            case ContextType::MODEL: icon = ICON_LC_BOX; break;
            case ContextType::MATERIAL: icon = ICON_LC_BOXES; break;
            case ContextType::MATERIALSET: icon = ICON_LC_COMBINE; break;
            case ContextType::XML: icon = ICON_LC_FILE_CODE_2; break;
            default: icon = ICON_LC_FILE_QUESTION; break;
        }

        bool isCurrentFile = Context::Get().GetCurrentFile() == path;
        if (ImGui::Selectable((icon + displayName).c_str(), isCurrentFile)) {
            Context::Get().SetEnqueuedFile(path.string());
        }

        if (entry.type == ContextType::NONE) {
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }
    }
}

void UI::Explorer::DrawDirectoryChildren(std::vector<FileEntry>& children) {
    std::vector<std::reference_wrapper<FileEntry>> visibleChildren;
    visibleChildren.reserve(children.size());

    for (auto& child : children) {
        if (child.isVisible) {
            visibleChildren.emplace_back(std::ref(child));
        }
    }

    // If there are a lot of children to render, use a clipper to improve performance
    if (visibleChildren.size() > 200) {
        ImGuiListClipper clipper;
        clipper.Begin(visibleChildren.size());

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                DrawEntry(visibleChildren[i]);
            }
        }
    } else {
        for (auto& child : visibleChildren) {
            DrawEntry(child);
        }
    }
}

void UI::Explorer::Draw(UIState &state) {
    ImGui::Begin("Explorer", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    auto& context = Context::Get();
    auto& root = context.GetRootDirectory();

    if (!root.path.empty()) {
        ImGui::InputText("Search", state.mSearchQuery, 255);
        ImGui::Separator();

        for (auto& child : root.children) {
            if (child.isVisible) {
                DrawEntry(child);
            }
        }
    } else {
        ImGui::TextUnformatted("No valid game root selected.");
    }

    ImGui::End();
}
