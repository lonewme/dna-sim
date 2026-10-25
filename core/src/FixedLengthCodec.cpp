#include "dna/FixedLengthCodec.hpp"

#include <cstdint>

namespace dna {

DnaSequence FixedLengthCodec::encode(std::string_view data) const {
    DnaSequence sequence;
    sequence.reserve(data.size() * kBasesPerByte);

    for (char c : data) {
        const auto byte = static_cast<std::uint8_t>(c);
        for (int shift = 6; shift >= 0; shift -= 2) {
            sequence.push_back(static_cast<Nucleotide>((byte >> shift) & 0b11));
        }
    }
    return sequence;
}

std::optional<std::string> FixedLengthCodec::decode(const DnaSequence& sequence) const {
    if (sequence.size() % kBasesPerByte != 0) {
        return std::nullopt;
    }

    std::string data;
    data.reserve(sequence.size() / kBasesPerByte);

    for (std::size_t i = 0; i < sequence.size(); i += kBasesPerByte) {
        std::uint8_t byte = 0;
        for (std::size_t j = 0; j < kBasesPerByte; ++j) {
            byte = static_cast<std::uint8_t>(
                (byte << 2) | static_cast<std::uint8_t>(sequence[i + j]));
        }
        data.push_back(static_cast<char>(byte));
    }
    return data;
}

}  // namespace dna