#pragma once

#include "dna/StrandSimulation.hpp"

// Window for the chunked pipeline. Contains no DNA logic: it only calls StrandSimulation.
class StrandPanel {
public:
    void draw();

private:
    dna::StrandSimulation simulation_;

    char input_[1024] = "Hello, DNA!";
    int payload_bytes_ = 4;
    int copies_ = 3;
    float substitution_ = 0.02f;
    float insertion_ = 0.0f;
    float deletion_ = 0.0f;
    int seed_ = 1;
        bool pure_random_ = false;
};