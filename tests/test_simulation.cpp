#include <doctest/doctest.h>

#include <stdexcept>

#include "dna/Simulation.hpp"

using namespace dna;

TEST_CASE("decoding without mutation restores the input") {
    Simulation sim;
    sim.encode("Hello, DNA!");
    sim.decode();

    REQUIRE(sim.decoded().has_value());
    REQUIRE(sim.decoded()->text.has_value());
    CHECK(*sim.decoded()->text == "Hello, DNA!");
    CHECK(sim.decoded()->wrong_bytes == 0);
}

TEST_CASE("mutate and decode before encode are rejected") {
    Simulation sim;
    CHECK_THROWS_AS(sim.mutate({}, 1), std::logic_error);
    CHECK_THROWS_AS(sim.decode(), std::logic_error);
}

TEST_CASE("substituting every base makes every byte wrong") {
    Simulation sim;
    sim.encode("Hello, DNA!");
    sim.mutate({1.0, 0.0, 0.0}, 5);
    sim.decode();

    REQUIRE(sim.decoded()->text.has_value());
    CHECK(sim.decoded()->wrong_bytes == sim.input().size());
    CHECK(sim.mutation_count(MutationType::Substitution) == sim.input().size() * 4);
}

TEST_CASE("deleting every base decodes to empty text with all bytes wrong") {
    Simulation sim;
    sim.encode("Hello, DNA!");
    sim.mutate({0.0, 0.0, 1.0}, 5);
    sim.decode();

    REQUIRE(sim.decoded()->text.has_value());
    CHECK(sim.decoded()->text->empty());
    CHECK(sim.decoded()->wrong_bytes == sim.input().size());
}

TEST_CASE("encoding again clears the old damage and results") {
    Simulation sim;
    sim.encode("first");
    sim.mutate({0.5, 0.0, 0.0}, 1);
    sim.decode();

    sim.encode("second");
    CHECK_FALSE(sim.mutation().has_value());
    CHECK_FALSE(sim.decoded().has_value());
    CHECK(sim.input() == "second");
}