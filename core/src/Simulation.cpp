#include "dna/Simulation.hpp"

#include <algorithm>
#include <stdexcept>

namespace dna {

namespace {

std::size_t count_wrong_bytes(const std::string& expected, const std::string& actual) {
    const std::size_t common = std::min(expected.size(), actual.size());
    // Missing or extra bytes at the end count as wrong too.
    std::size_t wrong = std::max(expected.size(), actual.size()) - common;
    for (std::size_t i = 0; i < common; ++i) {
        if (expected[i] != actual[i]) {
            ++wrong;
        }
    }
    return wrong;
}

}  // namespace

void Simulation::encode(std::string_view text) {
    input_ = std::string(text);
    original_ = codec_.encode(input_);
    mutation_.reset();
    decoded_.reset();
    encoded_ = true;
}

void Simulation::mutate(MutationRates rates, std::uint32_t seed) {
    if (!encoded_) {
        throw std::logic_error("Simulation::mutate called before encode");
    }
    Mutator mutator(rates, seed);
    mutation_ = mutator.mutate(original_);
    decoded_.reset();
}

void Simulation::decode() {
    if (!encoded_) {
        throw std::logic_error("Simulation::decode called before encode");
    }
    const DnaSequence& target = mutation_ ? mutation_->sequence : original_;

    DecodeOutcome outcome;
    outcome.text = codec_.decode(target);
    if (outcome.text) {
        outcome.wrong_bytes = count_wrong_bytes(input_, *outcome.text);
    }
    decoded_ = std::move(outcome);
}

std::size_t Simulation::mutation_count(MutationType type) const {
    if (!mutation_) {
        return 0;
    }
    return static_cast<std::size_t>(
        std::count_if(mutation_->events.begin(), mutation_->events.end(),
                      [type](const MutationEvent& e) { return e.type == type; }));
}

}  // namespace dna