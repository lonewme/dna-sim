#pragma once

#include <string>
#include <vector>

#include "dna/Experiment.hpp"

// Window that runs the copies experiment and plots the result.
// Contains no DNA logic: it only calls dna::run_copies_sweep.
class ExperimentPanel {
public:
    void draw();

private:
    void run();

    char input_[1024] = "Hello, DNA!";
    int payload_bytes_ = 4;
    int max_copies_ = 10;
    int trials_ = 200;
    float substitution_ = 0.02f;
    float insertion_ = 0.0f;
    float deletion_ = 0.0f;
    int seed_ = 1;

    std::vector<dna::ExperimentPoint> points_;
    std::vector<float> percent_;  // recovery rate in %, for the plot
    std::string error_;
};