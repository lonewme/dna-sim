#include "dna/StrandSimulation.hpp"

#include <stdexcept>
#include <utility>

namespace dna {

void StrandSimulation::encode(std::string_view text, std::size_t payload_bytes,
                              std::size_t copies) {
    if (copies == 0) {
        throw std::invalid_argument("copies must be at least 1");
    }
    // Build everything in locals first, so a failure leaves our state unchanged.
    ChunkedCodec codec(payload_bytes);
    const std::vector<DnaSequence> unique = codec.encode(text);

    std::vector<DnaSequence> pool;
    pool.reserve(unique.size() * copies);
    for (const DnaSequence& strand : unique) {
        for (std::size_t c = 0; c < copies; ++c) {
            pool.push_back(strand);
        }
    }

    codec_ = codec;
    input_ = std::string(text);
    unique_strands_ = unique.size();
    copies_ = copies;
    pool_ = std::move(pool);
    damaged_.reset();
    totals_.reset();
    report_.reset();
    encoded_ = true;
}

void StrandSimulation::mutate(MutationRates rates, std::uint32_t seed) {
    if (!encoded_) {
        throw std::logic_error("StrandSimulation::mutate called before encode");
    }

    // One Mutator for the whole pool: it keeps drawing new random numbers,
    // so every copy gets different, independent damage.
    Mutator mutator(rates, seed);

    std::vector<DnaSequence> damaged;
    damaged.reserve(pool_.size());
    MutationTotals totals;

    for (const DnaSequence& strand : pool_) {
        MutationResult result = mutator.mutate(strand);
        for (const MutationEvent& event : result.events) {
            switch (event.type) {
                case MutationType::Substitution: ++totals.substitutions; break;
                case MutationType::Insertion:    ++totals.insertions;    break;
                case MutationType::Deletion:     ++totals.deletions;     break;
            }
        }
        damaged.push_back(std::move(result.sequence));
    }

    damaged_ = std::move(damaged);
    totals_ = totals;
    report_.reset();
}

void StrandSimulation::decode() {
    if (!encoded_) {
        throw std::logic_error("StrandSimulation::decode called before encode");
    }
    const std::vector<DnaSequence>& source = damaged_ ? *damaged_ : pool_;
    report_ = codec_.decode(source);
}

bool StrandSimulation::recovered_perfectly() const {
    return report_ && report_->complete() && report_->text == input_;
}

}  // namespace dna