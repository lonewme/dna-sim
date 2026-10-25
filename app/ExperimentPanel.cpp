#include "ExperimentPanel.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

#include "imgui.h"

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
    ImGui::Begin("Experiment: recovery vs copies");

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
        ImGui::PlotLines("##curve", percent_.data(), static_cast<int>(percent_.size()), 0,
                         "1 copy  ->  max copies", 0.0f, 100.0f, ImVec2(-1, 150));

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