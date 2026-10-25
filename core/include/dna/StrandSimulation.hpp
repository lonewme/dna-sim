#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "dna/ChunkedCodec.hpp"
#include "dna/DnaSequence.hpp"
#include "dna/Mutator.hpp"

namespace dna {

struct MutationTotals {
    std::size_t substitutions = 0;
    std::size_t insertions = 0;
    std::size_t deletions = 0;
};

// Chunked pipeline: encode into strands, make copies, damage every copy
// independently (like a test tube), then decode the whole pool.
class StrandSimulation {
public:
    // Throws std::invalid_argument if payload_bytes is not 1..255 or copies is 0.
    // On failure the previous state is left untouched.
    void encode(std::string_view text, std::size_t payload_bytes, std::size_t copies);

    // Throws std::logic_error if encode() has not been called yet.
    void mutate(MutationRates rates, std::uint32_t seed);

    // Decodes the damaged pool if there is one, otherwise the original pool.
    // Throws std::logic_error if encode() has not been called yet.
    void decode();

    bool has_encoded() const { return encoded_; }
    const std::string& input() const { return input_; }
    std::size_t unique_strands() const { return unique_strands_; }
    std::size_t copies() const { return copies_; }

    // Pool order: all copies of strand 0, then all copies of strand 1, and so on.
    const std::vector<DnaSequence>& pool() const { return pool_; }
    const std::optional<std::vector<DnaSequence>>& damaged_pool() const { return damaged_; }
    const std::optional<MutationTotals>& totals() const { return totals_; }
    const std::optional<ChunkDecodeReport>& report() const { return report_; }

    bool recovered_perfectly() const;

private:
    ChunkedCodec codec_;
    bool encoded_ = false;
    std::string input_;
    std::size_t unique_strands_ = 0;
    std::size_t copies_ = 0;
    std::vector<DnaSequence> pool_;
    std::optional<std::vector<DnaSequence>> damaged_;
    std::optional<MutationTotals> totals_;
    std::optional<ChunkDecodeReport> report_;
};

}  // namespace dna