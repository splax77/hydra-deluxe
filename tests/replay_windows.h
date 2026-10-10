// Made-up replay windows over a real song, shared by tests/test_replay.cpp
// and hydra_bench's --replay timing (tools/bench.cpp).

#ifndef HYDRA_TESTS_REPLAY_WINDOWS_H
#define HYDRA_TESTS_REPLAY_WINDOWS_H

#include <algorithm>
#include <cstdint>
#include <vector>

#include "core/replay.h"
#include "parse/song.h"

namespace testreplay {

// A small deterministic generator, so the synthetic windows are the same on
// every run and every machine.
struct Lcg {
    uint64_t state;
    uint32_t next() {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<uint32_t>(state >> 33);
    }
};

// `count` made-up windows over `song`: random activation chords, windows of
// 1 to 60 chords that overlap freely, deactivation nodes that are sometimes a
// few ticks off a chord, and a squeeze-out chord on about a quarter of them
// (anywhere from 3 chords before the SP end to 3 after, so some sit before D).
inline std::vector<hydra::ReplayWindow> synthetic_windows(const hydra::Song& song, size_t count,
                                                          uint64_t seed) {
    std::vector<hydra::ReplayWindow> out;
    const size_t n = song.sequence.size();
    if (n == 0) return out;
    Lcg rng{seed};
    out.reserve(count);
    for (size_t j = 0; j < count; ++j) {
        const size_t a = rng.next() % n;
        const size_t len = 1 + rng.next() % 60;
        const size_t d = std::min(a + len, n - 1);
        hydra::ReplayWindow w;
        w.act_tick = song.sequence[a].timecode.ticks();
        w.deact_tick = song.sequence[d].timecode.ticks() +
                       (rng.next() % 3 == 0 ? static_cast<int64_t>(rng.next() % 10) : 0);
        if (rng.next() % 4 == 0) {
            const size_t lo = d >= 3 ? d - 3 : 0;
            const size_t q = std::min(lo + rng.next() % 7, n - 1);
            w.sqout_tick = song.sequence[q].timecode.ticks();
        }
        out.push_back(w);
    }
    return out;
}

}  // namespace testreplay

#endif  // HYDRA_TESTS_REPLAY_WINDOWS_H
