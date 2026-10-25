#include "dna/ChunkedCodec.hpp"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "dna/Crc.hpp"

namespace dna {

namespace {

void put_u16(std::string& out, std::size_t value) {
    out.push_back(static_cast<char>((value >> 8) & 0xFF));
    out.push_back(static_cast<char>(value & 0xFF));
}

std::uint16_t get_u16(const std::string& in, std::size_t pos) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint8_t>(in[pos]) << 8) | static_cast<std::uint8_t>(in[pos + 1]));
}

}  // namespace

ChunkedCodec::ChunkedCodec(std::size_t payload_bytes) : payload_bytes_(payload_bytes) {
    if (payload_bytes == 0 || payload_bytes > 255) {
        throw std::invalid_argument("payload_bytes must be between 1 and 255");
    }
}

std::vector<DnaSequence> ChunkedCodec::encode(std::string_view data) const {
    // Even empty data gets one chunk, so the decoder always has something to find.
    const std::size_t total =
        std::max<std::size_t>(1, (data.size() + payload_bytes_ - 1) / payload_bytes_);
    if (total > 0xFFFF) {
        throw std::length_error("data needs more than 65535 chunks");
    }

    std::vector<DnaSequence> strands;
    strands.reserve(total);

    for (std::size_t index = 0; index < total; ++index) {
        const std::size_t offset = index * payload_bytes_;
        const std::size_t length = std::min(payload_bytes_, data.size() - offset);

        std::string bytes;
        bytes.reserve(kHeaderBytes + payload_bytes_ + kCrcBytes);
        put_u16(bytes, index);
        put_u16(bytes, total);
        bytes.push_back(static_cast<char>(length));
        bytes.append(data.substr(offset, length));
        bytes.append(payload_bytes_ - length, '\0');  // padding
        put_u16(bytes, crc16(bytes));

        strands.push_back(base_.encode(bytes));
    }
    return strands;
}

std::optional<ChunkedCodec::ParsedChunk> ChunkedCodec::parse_strand(
    const DnaSequence& strand) const {
    const auto bytes = base_.decode(strand);
    if (!bytes || bytes->size() != kHeaderBytes + payload_bytes_ + kCrcBytes) {
        return std::nullopt;
    }

    const std::size_t body_size = bytes->size() - kCrcBytes;
    if (crc16(std::string_view(*bytes).substr(0, body_size)) != get_u16(*bytes, body_size)) {
        return std::nullopt;
    }

    ParsedChunk chunk;
    chunk.index = get_u16(*bytes, 0);
    chunk.total = get_u16(*bytes, 2);
    const std::size_t length = static_cast<std::uint8_t>((*bytes)[4]);

    if (chunk.total == 0 || chunk.index >= chunk.total || length > payload_bytes_) {
        return std::nullopt;
    }
    chunk.payload = bytes->substr(kHeaderBytes, length);
    return chunk;
}

ChunkDecodeReport ChunkedCodec::decode(const std::vector<DnaSequence>& strands) const {
    ChunkDecodeReport report;
    std::vector<std::optional<std::string>> chunks;

    for (const DnaSequence& strand : strands) {
        const auto parsed = parse_strand(strand);
        if (!parsed) {
            ++report.rejected_strands;
            continue;
        }
        if (report.total_chunks == 0) {
            // The first valid strand tells us how many chunks to expect.
            report.total_chunks = parsed->total;
            chunks.resize(parsed->total);
        } else if (parsed->total != report.total_chunks) {
            ++report.rejected_strands;
            continue;
        }
        // A second valid copy of the same chunk is simply ignored.
        if (!chunks[parsed->index]) {
            chunks[parsed->index] = parsed->payload;
        }
    }

    for (std::size_t i = 0; i < report.total_chunks; ++i) {
        if (chunks[i]) {
            report.text += *chunks[i];
        } else {
            report.lost_chunks.push_back(i);
            report.text.append(payload_bytes_, '?');
        }
    }
    return report;
}

}  // namespace dna