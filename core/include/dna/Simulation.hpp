#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "dna/DnaSequence.hpp"
#include "dna/FixedLengthCodec.hpp"
#include "dna/Mutator.hpp"

namespace dna {

struct DecodeOutcome {
    // nullopt means the sequence could not be decoded at all.
    std::optional<std::string> text;
    // Bytes that differ from the input (only meaningful when text has a value).
    std::size_t wrong_bytes = 0;
};

// Holds the state of one encode -> mutate -> decode run.
class Simulation {
public:
    void encode(std::string_view text);

    // Throws std::logic_error if encode() has not been called yet.
    void mutate(MutationRates rates, std::uint32_t seed);

    // Decodes the damaged sequence if there is one, otherwise the original.
    // Throws std::logic_error if encode() has not been called yet.
    void decode();

    bool has_encoded() const { return encoded_; }
    const std::string& input() const { return input_; }
    const DnaSequence& original_dna() const { return original_; }
    const std::optional<MutationResult>& mutation() const { return mutation_; }
    const std::optional<DecodeOutcome>& decoded() const { return decoded_; }

    std::size_t mutation_count(MutationType type) const;

private:
    FixedLengthCodec codec_;
    bool encoded_ = false;
    std::string input_;
    DnaSequence original_;
    std::optional<MutationResult> mutation_;
    std::optional<DecodeOutcome> decoded_;
};

}  // namespace dna