#include <doctest/doctest.h>

#include <stdexcept>

#include "dna/Experiment.hpp"

using namespace dna;

namespace {

ExperimentSettings hello_settings() {
    ExperimentSettings settings;
    settings.text = "Hello, DNA!";
    settings.payload_bytes = 4;
    return settings;
}

}  // namespace

TEST_CASE("without damage every run recovers perfectly") {
    auto settings = hello_settings();
    settings.rates = {0.0, 0.0, 0.0};
    settings.trials = 20;

    const auto points = run_copies_sweep(settings, 3);
    REQUIRE(points.size() == 3);
    for (const auto& point : points) {
        CHECK(point.perfect_runs == 20);
        CHECK(point.recovery_rate() == doctest::Approx(1.0));
    }
}

TEST_CASE("there is one point per copy count, in order") {
    auto settings = hello_settings();
    settings.trials = 5;

    const auto points = run_copies_sweep(settings, 4);
    REQUIRE(points.size() == 4);
    for (std::size_t i = 0; i < points.size(); ++i) {
        CHECK(points[i].copies == i + 1);
        CHECK(points[i].trials == 5);
    }
}

TEST_CASE("more copies give a much higher recovery rate") {
    auto settings = hello_settings();
    settings.rates = {0.02, 0.0, 0.0};
    settings.trials = 200;

    const auto points = run_copies_sweep(settings, 8);
    REQUIRE(points.size() == 8);

    // Theory says about 7% with 1 copy and about 96% with 8 copies;
    // the limits below leave a wide margin for random variation.
    CHECK(points[0].recovery_rate() <= 0.25);
    CHECK(points[7].recovery_rate() >= 0.85);
}

TEST_CASE("the same settings give the same results") {
    auto settings = hello_settings();
    settings.rates = {0.02, 0.01, 0.01};
    settings.trials = 30;

    const auto first = run_copies_sweep(settings, 3);
    const auto second = run_copies_sweep(settings, 3);

    REQUIRE(first.size() == second.size());
    for (std::size_t i = 0; i < first.size(); ++i) {
        CHECK(first[i].perfect_runs == second[i].perfect_runs);
    }
}

TEST_CASE("invalid settings are rejected") {
    auto settings = hello_settings();

    auto no_trials = settings;
    no_trials.trials = 0;
    CHECK_THROWS_AS(run_copies_sweep(no_trials, 3), std::invalid_argument);

    CHECK_THROWS_AS(run_copies_sweep(settings, 0), std::invalid_argument);

    auto bad_payload = settings;
    bad_payload.payload_bytes = 0;
    CHECK_THROWS_AS(run_copies_sweep(bad_payload, 3), std::invalid_argument);
}