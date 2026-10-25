#pragma once

#include "dna/Simulation.hpp"

// Draws the simulation window. Contains no DNA logic: it only calls Simulation.
class SimulationPanel {
public:
    void draw();

private:
    dna::Simulation simulation_;

    char input_[1024] = "Hello, DNA!";
    float substitution_ = 0.02f;
    float insertion_ = 0.0f;
    float deletion_ = 0.0f;
    int seed_ = 1;
};