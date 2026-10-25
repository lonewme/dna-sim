#pragma once

#include <cstdint>
#include <string_view>

namespace dna {

// CRC-16/CCITT-FALSE: polynomial 0x1021, initial value 0xFFFF.
std::uint16_t crc16(std::string_view data);

}  // namespace dna