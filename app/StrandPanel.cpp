#include "StrandPanel.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "DisplayText.hpp"
#include "imgui.h"

namespace {

constexpr std::size_t kMaxStrandsShown = 50;

// Shows every `step`-th strand in a scrollable box (limited, to keep the UI fast).
void draw_strands(const char* id, const std::vector<dna::DnaSequence>& strands,
                  std::size_t step) {
    ImGui::BeginChild(id, ImVec2(-1, 110), ImGuiChildFlags_Borders);
    std::size_t shown = 0;
    for (std::size_t i = 0; i < strands.size() && shown < kMaxStrandsShown;
         i += step, ++shown) {
        ImGui::TextWrapped("%s", dna::to_string(strands[i]).c_str());
    }
    ImGui::EndChild();

    const std::size_t visible = (strands.size() + step - 1) / step;
    if (visible > kMaxStrandsShown) {
        ImGui::TextDisabled("(first %zu of %zu shown)", kMaxStrandsShown, visible);
    }
}

}  // namespace

void StrandPanel::draw() {
    ImGui::SetNextWindowPos(ImVec2(520, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(470, 660), ImGuiCond_FirstUseEver);
    ImGui::Begin("Chunks + copies");

    // --- 1. Encode ---
    ImGui::SeparatorText("1. Input text");
    ImGui::InputTextMultiline("##strand_input", input_, sizeof(input_), ImVec2(-1, 70));
    ImGui::SliderInt("Payload bytes per strand", &payload_bytes_, 1, 32);
    ImGui::SliderInt("Copies per strand", &copies_, 1, 20);
    if (ImGui::Button("Encode")) {
        simulation_.encode(input_, static_cast<std::size_t>(payload_bytes_),
                           static_cast<std::size_t>(copies_));
    }

    if (simulation_.has_encoded()) {
        ImGui::Text("%zu strands x %zu copies = %zu strands in the pool",
                    simulation_.unique_strands(), simulation_.copies(),
                    simulation_.pool().size());
        draw_strands("##original", simulation_.pool(), simulation_.copies());

        // --- 2. Mutate ---
        ImGui::SeparatorText("2. Mutate (every copy independently)");
        ImGui::SliderFloat("Substitution", &substitution_, 0.0f, 0.2f, "%.3f");
        ImGui::SliderFloat("Insertion", &insertion_, 0.0f, 0.2f, "%.3f");
        ImGui::SliderFloat("Deletion", &deletion_, 0.0f, 0.2f, "%.3f");
                ImGui::Checkbox("Pure random (new seed every time)", &pure_random_);
        ImGui::BeginDisabled(pure_random_);
        ImGui::InputInt("Seed", &seed_);
        ImGui::EndDisabled();
        if (ImGui::Button("Mutate")) {
            if (pure_random_) {
                seed_ = static_cast<int>(dna::random_seed() & 0x7FFFFFFFu);
            }
            simulation_.mutate({substitution_, insertion_, deletion_},
                               static_cast<std::uint32_t>(seed_));
        }

        if (const auto& damaged = simulation_.damaged_pool(); damaged) {
            draw_strands("##damaged", *damaged, 1);
            if (const auto& totals = simulation_.totals(); totals) {
                ImGui::Text("Substitutions: %zu   Insertions: %zu   Deletions: %zu",
                            totals->substitutions, totals->insertions, totals->deletions);
            }
        }

        // --- 3. Decode ---
        ImGui::SeparatorText("3. Decode");
        if (ImGui::Button("Decode")) {
            simulation_.decode();
        }

        if (const auto& report = simulation_.report(); report) {
            if (report->total_chunks == 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                                   "No valid strand survived. Nothing can be recovered.");
            } else {
                ImGui::Text("Strands rejected: %zu of %zu", report->rejected_strands,
                            simulation_.pool().size());
                ImGui::Text("Chunks recovered: %zu of %zu",
                            report->total_chunks - report->lost_chunks.size(),
                            report->total_chunks);
                ImGui::TextWrapped("Result: %s", printable(report->text).c_str());

                if (simulation_.recovered_perfectly()) {
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Perfect recovery.");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f),
                                       "Incomplete: lost chunks are shown as '?'.");
                }
            }
        }
    }

    ImGui::End();
}