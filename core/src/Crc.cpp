#include "dna/Crc.hpp"

namespace dna {

std::uint16_t crc16(std::string_view data) {
    std::uint16_t crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<std::uint16_t>(static_cast<std::uint8_t>(c)) << 8;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000) ? static_cast<std::uint16_t>((crc << 1) ^ 0x1021)
                                 : static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}

}  // namespace dna