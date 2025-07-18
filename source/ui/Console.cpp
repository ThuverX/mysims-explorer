#include "Console.hpp"

#include "imgui.h"
#include "io/Logger.hpp"

void UI::Console::Draw() {
    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_HorizontalScrollbar);

    const auto &history = Logger::Get().GetHistory();
    static uint64_t lastEntryId = 0;

    for (const auto& entry : history) {
        ImVec4 color;

        switch (entry.level) {
            case LogLevel::ERROR:
                color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // red
                break;
            case LogLevel::SUCCESS:
                color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // green
                break;
            case LogLevel::WARN:
                color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f); // orange/yellow
                break;
            case LogLevel::INFO:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // white
                break;
            default:
                color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // gray
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (!history.empty()) {
        uint64_t latestId = history.back().id;
        if (latestId != lastEntryId) {
            ImGui::SetScrollHereY(1.0f);
            lastEntryId = latestId;
        }
    }

    ImGui::End();
}
