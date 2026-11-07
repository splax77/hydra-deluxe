// A fixed-pitch text measure for tests that cut or place text without a font.
#ifndef HYDRA_TESTS_FIXED_PITCH_H
#define HYDRA_TESTS_FIXED_PITCH_H

#include <string>

// 10 px per character (not per byte), "…" included: a stand-in for the width
// of a monospace font. A UTF-8 sequence counts as one character.
inline float ten_per_char(const std::string& s) {
    float w = 0.0f;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) w += 10.0f;
    return w;
}

#endif  // HYDRA_TESTS_FIXED_PITCH_H
