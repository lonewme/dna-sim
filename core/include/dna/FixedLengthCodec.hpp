#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "dna/DnaSequence.hpp"

namespace dna {

// Fixed-length encoding: 1 byte = 4 nucleotides (2 bits each).
class FixedLengthCodec {
public:
    static constexpr std::size_t kBasesPerByte = 4;

    DnaSequence encode(std::string_view data) const;

    // Returns nullopt if the sequence length is not a multiple of 4.
    std::optional<std::string> decode(const DnaSequence& sequence) const;
};

}  // namespace dna