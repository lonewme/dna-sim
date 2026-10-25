#include <doctest/doctest.h>

#include <algorithm>
#include <stdexcept>

#include "dna/FixedLengthCodec.hpp"
#include "dna/Mutator.hpp"

using namespace dna;

namespace {

std::size_t count_events(const MutationResult& result, MutationType type) {
    return static_cast<std::size_t>(
        std::count_if(result.events.begin(), result.events.end(),
                      [type](const MutationEvent& e) { return e.type == type; }));
}

DnaSequence sample_sequence() {
    return FixedLengthCodec{}.encode("Hello, DNA!");
}

}  // namespace

TEST_CASE("zero rates leave the sequence untouched") {
    Mutator mutator({0.0, 0.0, 0.0}, 1);
    const auto original = sample_sequence();
    const auto result = mutator.mutate(original);
    CHECK(result.sequence == original);
    CHECK(result.events.empty());
}

TEST_CASE("the same seed gives the same damage") {
    const auto original = sample_sequence();
    Mutator first({0.1, 0.1, 0.1}, 123);
    Mutator second({0.1, 0.1, 0.1}, 123);
    CHECK(first.mutate(original).sequence == second.mutate(original).sequence);
}

TEST_CASE("substitution rate 1.0 changes every base") {
    const auto original = sample_sequence();
    Mutator mutator({1.0, 0.0, 0.0}, 7);
    const auto result = mutator.mutate(original);

    REQUIRE(result.sequence.size() == original.size());
    for (std::size_t i = 0; i < original.size(); ++i) {
        CHECK(result.sequence[i] != original[i]);
    }
    CHECK(count_events(result, MutationType::Substitution) == original.size());
}

TEST_CASE("deletion rate 1.0 removes everything") {
    Mutator mutator({0.0, 0.0, 1.0}, 7);
    const auto result = mutator.mutate(sample_sequence());
    CHECK(result.sequence.empty());
}

TEST_CASE("insertion rate 1.0 doubles the length") {
    const auto original = sample_sequence();
    Mutator mutator({0.0, 1.0, 0.0}, 7);
    const auto result = mutator.mutate(original);
    CHECK(result.sequence.size() == original.size() * 2);
}

TEST_CASE("length always equals original - deletions + insertions") {
    const auto original = sample_sequence();
    Mutator mutator({0.1, 0.1, 0.1}, 42);
    const auto result = mutator.mutate(original);

    const auto deletions = count_events(result, MutationType::Deletion);
    const auto insertions = count_events(result, MutationType::Insertion);
    CHECK(result.sequence.size() == original.size() - deletions + insertions);
}

TEST_CASE("invalid rates are rejected") {
    CHECK_THROWS_AS(Mutator({1.5, 0.0, 0.0}, 1), std::invalid_argument);
    CHECK_THROWS_AS(Mutator({-0.1, 0.0, 0.0}, 1), std::invalid_argument);
    CHECK_THROWS_AS(Mutator({0.6, 0.0, 0.6}, 1), std::invalid_argument);
}