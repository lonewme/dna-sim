#pragma once

#include <cstdint>
#include <optional>

namespace dna {

// The numeric values are the 2-bit codes used by the fixed-length encoding.
enum class Nucleotide : std::uint8_t { A = 0, C = 1, G = 2, T = 3 };

char to_char(Nucleotide n);
std::optional<Nucleotide> from_char(char c);  // nullopt if c is not A/C/G/T

}  // namespace dna