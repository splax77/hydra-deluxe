// The one pinned table of how each difficulty is spelled in a chart file, as
// literals from Clone Hero 1.1: 0x210D9C0 maps pitches 58-66 / 70-78 / 82-90 /
// 94-102 to Easy / Medium / Hard / Expert; 0x21555CD flags 59, 71, 83 and 95
// as DoubleKick; 0x210D990 maps a mix digit 0-3 to Easy..Expert.
// test_s2_difficulty_table.cpp checks the parser's table against it; other
// tests read their per-difficulty values from here instead of typing a second
// table.

#ifndef HYDRA_TESTS_DIFFICULTY_LITERALS_H
#define HYDRA_TESTS_DIFFICULTY_LITERALS_H

#include <cstdint>

#include "parse/song.h"

namespace testdiff {

struct Literals {
    hydra::Difficulty d;
    int kick;             // .mid kick pitch
    int kick2x;           // .mid 2x kick pitch
    uint8_t red;          // .mid red pad pitch
    char mix;             // disco marker digit
    const char* name;     // difficulty_name
    const char* section;  // .chart drum section
};

inline constexpr Literals kLiterals[] = {
    {hydra::Difficulty::Expert, 96, 95, 97, '3', "Expert", "ExpertDrums"},
    {hydra::Difficulty::Hard, 84, 83, 85, '2', "Hard", "HardDrums"},
    {hydra::Difficulty::Medium, 72, 71, 73, '1', "Medium", "MediumDrums"},
    {hydra::Difficulty::Easy, 60, 59, 61, '0', "Easy", "EasyDrums"},
};

}  // namespace testdiff

#endif  // HYDRA_TESTS_DIFFICULTY_LITERALS_H
