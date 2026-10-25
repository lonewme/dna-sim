#pragma once

#include <string>

// Damaged text can contain control characters that would cut the display short.
inline std::string printable(std::string text) {
    for (char& c : text) {
        if (static_cast<unsigned char>(c) < 0x20) {
            c = '?';
        }
    }
    return text;
}