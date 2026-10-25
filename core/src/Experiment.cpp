#include "dna/Experiment.hpp"

#include <stdexcept>

#include "dna/StrandSimulation.hpp"

namespace dna {

std::vector<ExperimentPoint> run_copies_sweep(const ExperimentSettings& settings,
                                              std::size_t max_copies) {
    if (settings.trials == 0) {
        throw std::invalid_argument("trials must be at least 1");
    }
    if (max_copies == 0) {
        throw std::invalid_argument("max_copies must be at least 1");
    }

    std::vector<ExperimentPoint> points;
    points.reserve(max_copies);

    for (std::size_t copies = 1; copies <= max_copies; ++copies) {
        // Encode once per copy count; every trial then damages a fresh copy of the pool.
        StrandSimulation simulation;
        simulation.encode(settings.text, settings.payload_bytes, copies);

        ExperimentPoint point;
        point.copies = copies;
        point.trials = settings.trials;

        for (std::size_t trial = 0; trial < settings.trials; ++trial) {
            simulation.mutate(settings.rates,
                              settings.base_seed + static_cast<std::uint32_t>(trial));
            simulation.decode();
            if (simulation.recovered_perfectly()) {
                ++point.perfect_runs;
            }
        }
        points.push_back(point);
    }
    return points;
}

}  // namespace dna