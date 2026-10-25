#include "SimulationPanel.hpp"
#include "DisplayText.hpp"
#include <cstdint>
#include <string>

#include "imgui.h"

namespace {

// Damaged text can contain control characters that would cut the display short.
std::string printable(std::string text) {
    for (char& c : text) {
        if (static_cast<unsigned char>(c) < 0x20) {
            c = '?';
        }
    }
    return text;
}

}  // namespace

void SimulationPanel::draw() {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(490, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Byte Simulation");

    // --- 1. Encode ---
    ImGui::SeparatorText("1. Input text");
    ImGui::InputTextMultiline("##input", input_, sizeof(input_), ImVec2(-1, 70));
    if (ImGui::Button("Encode")) {
        simulation_.encode(input_);
    }

    if (simulation_.has_encoded()) {
        ImGui::Text("Original DNA (%zu bases)", simulation_.original_dna().size());
        ImGui::TextWrapped("%s", dna::to_string(simulation_.original_dna()).c_str());

        // --- 2. Mutate ---
        ImGui::SeparatorText("2. Mutate");
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

        if (const auto& mutation = simulation_.mutation(); mutation) {
            ImGui::Text("Damaged DNA (%zu bases)", mutation->sequence.size());
            ImGui::TextWrapped("%s", dna::to_string(mutation->sequence).c_str());
            ImGui::Text("Substitutions: %zu   Insertions: %zu   Deletions: %zu",
                        simulation_.mutation_count(dna::MutationType::Substitution),
                        simulation_.mutation_count(dna::MutationType::Insertion),
                        simulation_.mutation_count(dna::MutationType::Deletion));
        }

        // --- 3. Decode ---
        ImGui::SeparatorText("3. Decode");
        if (ImGui::Button("Decode")) {
            simulation_.decode();
        }

        if (const auto& decoded = simulation_.decoded(); decoded) {
            if (decoded->text) {
                ImGui::Text("Wrong bytes: %zu of %zu", decoded->wrong_bytes,
                            simulation_.input().size());
            } else {
                ImGui::TextColored(
                    ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                    "Decoding failed: the length is not a multiple of 4. "
                    "An insertion or deletion broke the reading frame.");
            }
        }
    }

    ImGui::End();
}