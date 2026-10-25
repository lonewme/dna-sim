#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "dna/DnaSequence.hpp"

namespace dna {

enum class MutationType { Substitution, Insertion, Deletion };

struct MutationEvent {
    MutationType type;
    std::size_t position;  // index in the ORIGINAL sequence
};

// Per-base probabilities, each between 0.0 and 1.0.
struct MutationRates {
    double substitution = 0.0;
    double insertion = 0.0;
    double deletion = 0.0;
};

struct MutationResult {
    DnaSequence sequence;
    std::vector<MutationEvent> events;
};

class Mutator {
public:
    // Throws std::invalid_argument if the rates are not valid probabilities.
    Mutator(MutationRates rates, std::uint32_t seed);

    MutationResult mutate(const DnaSequence& original);

private:
    MutationRates rates_;
    std::mt19937 rng_;
};

}  // namespace dna