#include "dna/DnaSequence.hpp"

namespace dna {

std::string to_string(const DnaSequence& sequence) {
    std::string text;
    text.reserve(sequence.size());
    for (Nucleotide n : sequence) {
        text.push_back(to_char(n));
    }
    return text;
}

std::optional<DnaSequence> parse_sequence(std::string_view text) {
    DnaSequence sequence;
    sequence.reserve(text.size());
    for (char c : text) {
        auto n = from_char(c);
        if (!n) {
            return std::nullopt;
        }
        sequence.push_back(*n);
    }
    return sequence;
}

}  // namespace dna