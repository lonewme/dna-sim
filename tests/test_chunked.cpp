#include <doctest/doctest.h>

#include <stdexcept>
#include <vector>

#include "dna/ChunkedCodec.hpp"
#include "dna/Crc.hpp"

using namespace dna;

namespace {

// Changes one base to a different one.
void flip_base(DnaSequence& strand, std::size_t position) {
    strand[position] = static_cast<Nucleotide>((static_cast<int>(strand[position]) + 1) % 4);
}

}  // namespace

TEST_CASE("crc16 matches the standard check value") {
    CHECK(crc16("123456789") == 0x29B1);
}

TEST_CASE("chunked text survives a round trip") {
    ChunkedCodec codec(4);
    const auto strands = codec.encode("Hello, DNA!");

    REQUIRE(strands.size() == 3);
    for (const auto& strand : strands) {
        CHECK(strand.size() == (5 + 4 + 2) * 4);
    }

    const auto report = codec.decode(strands);
    CHECK(report.complete());
    CHECK(report.text == "Hello, DNA!");
    CHECK(report.rejected_strands == 0);
}

TEST_CASE("empty text survives a round trip") {
    ChunkedCodec codec(4);
    const auto report = codec.decode(codec.encode(""));
    CHECK(report.complete());
    CHECK(report.total_chunks == 1);
    CHECK(report.text.empty());
}

TEST_CASE("a substitution is detected and only that chunk is lost") {
    ChunkedCodec codec(4);
    auto strands = codec.encode("Hello, DNA!");
    flip_base(strands[1], 10);

    const auto report = codec.decode(strands);
    CHECK_FALSE(report.complete());
    CHECK(report.rejected_strands == 1);
    REQUIRE(report.lost_chunks.size() == 1);
    CHECK(report.lost_chunks[0] == 1);
    CHECK(report.text == "Hell????NA!");
}

TEST_CASE("a deletion damages only its own strand") {
    ChunkedCodec codec(4);
    auto strands = codec.encode("Hello, DNA!");
    strands[0].erase(strands[0].begin() + 5);

    const auto report = codec.decode(strands);
    REQUIRE(report.lost_chunks.size() == 1);
    CHECK(report.lost_chunks[0] == 0);
    CHECK(report.text == "????o, DNA!");
}

TEST_CASE("a missing last strand is still noticed") {
    ChunkedCodec codec(4);
    auto strands = codec.encode("Hello, DNA!");
    strands.pop_back();

    const auto report = codec.decode(strands);
    CHECK(report.total_chunks == 3);
    REQUIRE(report.lost_chunks.size() == 1);
    CHECK(report.lost_chunks[0] == 2);
}

TEST_CASE("a second copy of every strand repairs a damaged one") {
    ChunkedCodec codec(4);
    const auto strands = codec.encode("Hello, DNA!");

    std::vector<DnaSequence> pool = strands;
    pool.insert(pool.end(), strands.begin(), strands.end());  // two copies each
    flip_base(pool[1], 10);                                    // damage one copy

    const auto report = codec.decode(pool);
    CHECK(report.complete());
    CHECK(report.text == "Hello, DNA!");
    CHECK(report.rejected_strands == 1);
}

TEST_CASE("invalid payload sizes are rejected") {
    CHECK_THROWS_AS(ChunkedCodec(0), std::invalid_argument);
    CHECK_THROWS_AS(ChunkedCodec(256), std::invalid_argument);
}