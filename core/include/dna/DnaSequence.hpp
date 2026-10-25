#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dna/Nucleotide.hpp"

namespace dna {

using DnaSequence = std::vector<Nucleotide>;

std::string to_string(const DnaSequence& sequence);
std::optional<DnaSequence> parse_sequence(std::string_view text);

}  // namespace dna