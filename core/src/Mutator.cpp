#include "dna/Mutator.hpp"

#include <stdexcept>

namespace dna {

namespace {

bool is_probability(double p) { return p >= 0.0 && p <= 1.0; }

}  // namespace

Mutator::Mutator(MutationRates rates, std::uint32_t seed)
    : rates_(rates), rng_(seed) {
    if (!is_probability(rates.substitution) || !is_probability(rates.insertion) ||
        !is_probability(rates.deletion)) {
        throw std::invalid_argument("Mutation rates must be between 0.0 and 1.0");
    }
    // A base can't be both deleted and substituted, so these share one dice roll.
    if (rates.substitution + rates.deletion > 1.0) {
        throw std::invalid_argument("substitution + deletion rate must not exceed 1.0");
    }
}

MutationResult Mutator::mutate(const DnaSequence& original) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::uniform_int_distribution<int> any_base(0, 3);
    std::uniform_int_distribution<int> other_offset(1, 3);

    MutationResult result;
    result.sequence.reserve(original.size());

    for (std::size_t i = 0; i < original.size(); ++i) {
        const Nucleotide base = original[i];
        const double roll = chance(rng_);

        if (roll < rates_.deletion) {
            result.events.push_back({MutationType::Deletion, i});
        } else if (roll < rates_.deletion + rates_.substitution) {
            // Adding 1..3 (mod 4) always lands on a DIFFERENT base.
            const int changed = (static_cast<int>(base) + other_offset(rng_)) % 4;
            result.sequence.push_back(static_cast<Nucleotide>(changed));
            result.events.push_back({MutationType::Substitution, i});
        } else {
            result.sequence.push_back(base);
        }

        // Insertion is an independent roll: an extra random base after position i.
        if (chance(rng_) < rates_.insertion) {
            result.sequence.push_back(static_cast<Nucleotide>(any_base(rng_)));
            result.events.push_back({MutationType::Insertion, i});
        }
    }
    return result;
}

}  // namespace dna