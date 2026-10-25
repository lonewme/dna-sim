#include "dna/Nucleotide.hpp"

namespace dna {

char to_char(Nucleotide n) {
    static constexpr char kSymbols[] = {'A', 'C', 'G', 'T'};
    return kSymbols[static_cast<std::uint8_t>(n)];
}

std::optional<Nucleotide> from_char(char c) {
    switch (c) {
        case 'A': return Nucleotide::A;
        case 'C': return Nucleotide::C;
        case 'G': return Nucleotide::G;
        case 'T': return Nucleotide::T;
        default:  return std::nullopt;
    }
}

}  // namespace dna