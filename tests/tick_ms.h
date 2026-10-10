// A tick's time in ms on a hand-built song, read off the song's own tempo map.
// Shared by test_sp_end.cpp and test_tied_variants.cpp, whose assertions name
// note ticks and compare against their times.

#ifndef HYDRA_TESTS_TICK_MS_H
#define HYDRA_TESTS_TICK_MS_H

#include <cstdint>

#include "parse/song.h"

namespace hydra::test {

inline double tick_ms(const Song& song, int64_t tick) {
    return song.timing().ms_index().at(tick);
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_TICK_MS_H
