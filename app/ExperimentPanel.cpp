#include "ExperimentPanel.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "imgui.h"
namespace {

// Line chart of the recovery rate, with the real copy count on the x axis.
void draw_recovery_plot(const std::vector<dna::ExperimentPoint>& points) {
    if (points.empty()) {
        return;
    }

    const float width = std::max(ImGui::GetContentRegionAvail().x, 120.0f);
    ImGui::InvisibleButton("##recovery_plot", ImVec2(width, 180.0f));

    const ImVec2 item_min = ImGui::GetItemRectMin();
    const ImVec2 item_max = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Plot area inside the item (room is left for the axis labels).
    const float x0 = item_min.x + 40.0f;
    const float x1 = item_max.x - 12.0f;
    const float y_top = item_min.y + 8.0f;      // 100 %
    const float y_bottom = item_max.y - 22.0f;  // 0 %

    const std::size_t count = points.size();
    auto x_of = [&](std::size_t i) {
        if (count == 1) {
            return (x0 + x1) / 2.0f;
        }
        return x0 + (x1 - x0) * static_cast<float>(i) / static_cast<float>(count - 1);
    };
    auto y_of = [&](double rate) {
        return y_bottom - (y_bottom - y_top) * static_cast<float>(rate);
    };

    const ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 grid_color = IM_COL32(255, 255, 255, 40);
    const ImU32 line_color = IM_COL32(125, 211, 252, 255);

    draw->AddRectFilled(item_min, item_max, ImGui::GetColorU32(ImGuiCol_FrameBg));

    // Horizontal grid lines with percent labels.
    for (int percent = 0; percent <= 100; percent += 25) {
        const float y = y_of(percent / 100.0);
        draw->AddLine(ImVec2(x0, y), ImVec2(x1, y), grid_color);
        char label[8];
        std::snprintf(label, sizeof(label), "%d%%", percent);
        draw->AddText(ImVec2(item_min.x + 4.0f, y - ImGui::GetTextLineHeight() / 2.0f),
                      text_color, label);
    }

    // Copy counts under the axis (some are skipped when they would overlap).
    const float spacing = count == 1 ? 1000.0f : (x1 - x0) / static_cast<float>(count - 1);
    const auto label_step = static_cast<std::size_t>(std::ceil(24.0f / spacing));
    for (std::size_t i = 0; i < count; i += label_step) {
        char label[16];
        std::snprintf(label, sizeof(label), "%zu", points[i].copies);
        const ImVec2 label_size = ImGui::CalcTextSize(label);
        draw->AddText(ImVec2(x_of(i) - label_size.x / 2.0f, y_bottom + 5.0f),
                      text_color, label);
    }

    // The curve.
    for (std::size_t i = 0; i + 1 < count; ++i) {
        draw->AddLine(ImVec2(x_of(i), y_of(points[i].recovery_rate())),
                      ImVec2(x_of(i + 1), y_of(points[i + 1].recovery_rate())),
                      line_color, 2.0f);
    }
    for (std::size_t i = 0; i < count; ++i) {
        draw->AddCircleFilled(ImVec2(x_of(i), y_of(points[i].recovery_rate())), 3.5f,
                              line_color);
    }

    // Hover: find the nearest point and show its real copy count.
    if (ImGui::IsItemHovered()) {
        const float mouse_x = ImGui::GetIO().MousePos.x;
        std::size_t nearest = 0;
        float best = std::abs(mouse_x - x_of(0));
        for (std::size_t i = 1; i < count; ++i) {
            const float distance = std::abs(mouse_x - x_of(i));
            if (distance < best) {
                best = distance;
                nearest = i;
            }
        }

        const dna::ExperimentPoint& point = points[nearest];
        draw->AddLine(ImVec2(x_of(nearest), y_top), ImVec2(x_of(nearest), y_bottom),
                      IM_COL32(255, 255, 255, 90));
        draw->AddCircle(ImVec2(x_of(nearest), y_of(point.recovery_rate())), 6.0f,
                        IM_COL32(255, 255, 255, 255), 0, 2.0f);
        ImGui::SetTooltip("%zu %s: %.1f %% (%zu of %zu runs)", point.copies,
                          point.copies == 1 ? "copy" : "copies",
                          point.recovery_rate() * 100.0, point.perfect_runs, point.trials);
    }
}

}  // namespace
void ExperimentPanel::run() {
    dna::ExperimentSettings settings;
    settings.text = input_;
    settings.payload_bytes = static_cast<std::size_t>(payload_bytes_);
    settings.rates = {substitution_, insertion_, deletion_};
    settings.trials = static_cast<std::size_t>(trials_);
    settings.base_seed = static_cast<std::uint32_t>(seed_);

    try {
        points_ = dna::run_copies_sweep(settings, static_cast<std::size_t>(max_copies_));
        error_.clear();
        percent_.clear();
        for (const auto& point : points_) {
            percent_.push_back(static_cast<float>(point.recovery_rate() * 100.0));
        }
    } catch (const std::exception& error) {
        error_ = error.what();
    }
}

void ExperimentPanel::draw() {
    ImGui::SetNextWindowPos(ImVec2(990, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(490, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Experiment Simulation");

    ImGui::SeparatorText("Settings");
    ImGui::InputTextMultiline("##experiment_input", input_, sizeof(input_), ImVec2(-1, 70));
    ImGui::SliderInt("Payload bytes per strand", &payload_bytes_, 1, 32);
    ImGui::SliderInt("Max copies", &max_copies_, 2, 20);
    ImGui::SliderInt("Trials per copy count", &trials_, 10, 1000);
    ImGui::SliderFloat("Substitution", &substitution_, 0.0f, 0.2f, "%.3f");
    ImGui::SliderFloat("Insertion", &insertion_, 0.0f, 0.2f, "%.3f");
    ImGui::SliderFloat("Deletion", &deletion_, 0.0f, 0.2f, "%.3f");
    ImGui::InputInt("Base seed", &seed_);

    if (ImGui::Button("Run experiment")) {
        run();
    }
    ImGui::TextDisabled("The window freezes while it runs. Keep the text short.");

    if (!error_.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error: %s", error_.c_str());
    }

    if (!points_.empty()) {
        ImGui::SeparatorText("Result: perfect recovery rate (%) by copies per strand");
                                draw_recovery_plot(points_);

        if (ImGui::BeginTable("##results", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Copies");
            ImGui::TableSetupColumn("Perfect runs");
            ImGui::TableSetupColumn("Rate");
            ImGui::TableHeadersRow();
            for (const auto& point : points_) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%zu", point.copies);
                ImGui::TableNextColumn();
                ImGui::Text("%zu / %zu", point.perfect_runs, point.trials);
                ImGui::TableNextColumn();
                ImGui::Text("%.1f %%", point.recovery_rate() * 100.0);
            }
            ImGui::EndTable();
        }
    }

    ImGui::End();
}