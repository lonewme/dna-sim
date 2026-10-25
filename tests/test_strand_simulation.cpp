#include <doctest/doctest.h>

#include <cstdint>
#include <stdexcept>

#include "dna/StrandSimulation.hpp"

using namespace dna;

TEST_CASE("without mutation the text is recovered perfectly") {
    StrandSimulation sim;
    sim.encode("Hello, DNA!", 4, 1);
    sim.decode();

    REQUIRE(sim.report().has_value());
    CHECK(sim.report()->total_chunks == 3);
    CHECK(sim.recovered_perfectly());
}

TEST_CASE("the pool holds every copy of every strand") {
    StrandSimulation sim;
    sim.encode("Hello, DNA!", 4, 2);
    CHECK(sim.unique_strands() == 3);
    CHECK(sim.copies() == 2);
    CHECK(sim.pool().size() == 6);
}

TEST_CASE("mutate and decode before encode are rejected") {
    StrandSimulation sim;
    CHECK_THROWS_AS(sim.mutate({}, 1), std::logic_error);
    CHECK_THROWS_AS(sim.decode(), std::logic_error);
}

TEST_CASE("invalid settings are rejected and the old state is kept") {
    StrandSimulation sim;
    sim.encode("first", 4, 2);

    CHECK_THROWS_AS(sim.encode("second", 0, 2), std::invalid_argument);
    CHECK_THROWS_AS(sim.encode("second", 4, 0), std::invalid_argument);
    CHECK(sim.input() == "first");
    CHECK(sim.copies() == 2);
}

TEST_CASE("deleting every base destroys every strand") {
    StrandSimulation sim;
    sim.encode("Hello, DNA!", 4, 2);
    sim.mutate({0.0, 0.0, 1.0}, 3);
    sim.decode();

    REQUIRE(sim.report().has_value());
    CHECK(sim.report()->rejected_strands == sim.pool().size());
    CHECK_FALSE(sim.recovered_perfectly());
}

TEST_CASE("encoding again clears the old damage and results") {
    StrandSimulation sim;
    sim.encode("first", 4, 1);
    sim.mutate({0.5, 0.0, 0.0}, 1);
    sim.decode();

    sim.encode("second", 4, 1);
    CHECK_FALSE(sim.damaged_pool().has_value());
    CHECK_FALSE(sim.totals().has_value());
    CHECK_FALSE(sim.report().has_value());
}

TEST_CASE("more copies recover the text far more often") {
    const MutationRates rates{0.02, 0.0, 0.0};

    auto successes = [&](std::size_t copies) {
        int ok = 0;
        for (std::uint32_t seed = 1; seed <= 50; ++seed) {
            StrandSimulation sim;
            sim.encode("Hello, DNA!", 4, copies);
            sim.mutate(rates, seed);
            sim.decode();
            if (sim.recovered_perfectly()) {
                ++ok;
            }
        }
        return ok;
    };

    const int with_one_copy = successes(1);
    const int with_eight_copies = successes(8);
    CHECK(with_eight_copies > with_one_copy);
    CHECK(with_eight_copies >= 35);
}