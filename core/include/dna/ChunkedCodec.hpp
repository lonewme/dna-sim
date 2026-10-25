#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dna/DnaSequence.hpp"
#include "dna/FixedLengthCodec.hpp"

namespace dna {

struct ChunkDecodeReport {
    std::string text;                        // lost chunks appear as '?'
    std::size_t total_chunks = 0;            // 0 means nothing could be recovered
    std::vector<std::size_t> lost_chunks;    // indices of chunks with no valid strand
    std::size_t rejected_strands = 0;        // strands that failed validation

    bool complete() const { return total_chunks > 0 && lost_chunks.empty(); }
};

// Splits data into chunks; each chunk becomes its own DNA strand with an
// index, a chunk count and a CRC-16, so damage is detected and stays local.
class ChunkedCodec {
public:
    // payload_bytes must be between 1 and 255. Throws std::invalid_argument otherwise.
    explicit ChunkedCodec(std::size_t payload_bytes = 8);

    // Throws std::length_error if the data needs more than 65535 chunks.
    std::vector<DnaSequence> encode(std::string_view data) const;

    // Never throws on damaged input: bad strands are rejected and counted.
    ChunkDecodeReport decode(const std::vector<DnaSequence>& strands) const;

private:
    struct ParsedChunk {
        std::size_t index = 0;
        std::size_t total = 0;
        std::string payload;
    };

    std::optional<ParsedChunk> parse_strand(const DnaSequence& strand) const;

    static constexpr std::size_t kHeaderBytes = 5;  // index(2) + total(2) + length(1)
    static constexpr std::size_t kCrcBytes = 2;

    std::size_t payload_bytes_;
    FixedLengthCodec base_;
};

}  // namespace dna