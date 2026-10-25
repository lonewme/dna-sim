#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "dna/Mutator.hpp"

namespace dna {

struct ExperimentSettings {
    std::string text;
    std::size_t payload_bytes = 4;
    MutationRates rates;
    std::size_t trials = 200;       // simulations per copy count
    std::uint32_t base_seed = 1;    // trial t uses base_seed + t
};

struct ExperimentPoint {
    std::size_t copies = 0;
    std::size_t trials = 0;
    std::size_t perfect_runs = 0;   // runs where the text came back exactly

    double recovery_rate() const {
        return trials == 0 ? 0.0 : static_cast<double>(perfect_runs) / static_cast<double>(trials);
    }
};

// Measures how often the text is recovered perfectly with 1, 2, ..., max_copies
// copies per strand. Returns one point per copy count, in order.
// Throws std::invalid_argument if trials or max_copies is 0, or if the
// payload size is invalid.
std::vector<ExperimentPoint> run_copies_sweep(const ExperimentSettings& settings,
                                              std::size_t max_copies);

}  // namespace dna