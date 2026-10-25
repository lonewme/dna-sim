#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "dna/FixedLengthCodec.hpp"

using namespace dna;

TEST_CASE("the letter A encodes to CAAC") {
    FixedLengthCodec codec;
    CHECK(to_string(codec.encode("A")) == "CAAC");
}

TEST_CASE("empty text gives an empty sequence") {
    FixedLengthCodec codec;
    CHECK(codec.encode("").empty());
}

TEST_CASE("text survives an encode/decode round trip") {
    FixedLengthCodec codec;
    const std::string text = "Hello, DNA!";
    auto decoded = codec.decode(codec.encode(text));
    REQUIRE(decoded.has_value());
    CHECK(*decoded == text);
}

TEST_CASE("non-ASCII UTF-8 text survives a round trip") {
    FixedLengthCodec codec;
    const std::string text = "çğış";
    auto decoded = codec.decode(codec.encode(text));
    REQUIRE(decoded.has_value());
    CHECK(*decoded == text);
}

TEST_CASE("every possible byte value survives a round trip") {
    FixedLengthCodec codec;
    std::string all_bytes;
    for (int i = 0; i < 256; ++i) {
        all_bytes.push_back(static_cast<char>(i));
    }
    auto decoded = codec.decode(codec.encode(all_bytes));
    REQUIRE(decoded.has_value());
    CHECK(*decoded == all_bytes);
}

TEST_CASE("decoding rejects a length that is not a multiple of 4") {
    FixedLengthCodec codec;
    auto sequence = parse_sequence("CAA");
    REQUIRE(sequence.has_value());
    CHECK_FALSE(codec.decode(*sequence).has_value());
}

TEST_CASE("parsing rejects invalid characters") {
    CHECK_FALSE(parse_sequence("ACGX").has_value());
}