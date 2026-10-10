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

// The shape of a made-up window. Windows are 1 to kMaxWindowChords chords
// long and overlap freely. About one in kDeactOffsetOneIn has its deactivation
// node 0 to kDeactOffsetRange - 1 ticks off a chord. About one in kSqoutOneIn
// has a squeeze-out chord, kSqoutBefore chords before the SP end up to
// kSqoutBefore after it (so some sit before D).
constexpr size_t kMaxWindowChords = 60;
constexpr size_t kDeactOffsetOneIn = 3;
constexpr int64_t kDeactOffsetRange = 10;
constexpr size_t kSqoutOneIn = 4;
constexpr size_t kSqoutBefore = 3;
constexpr size_t kSqoutPicks = 2 * kSqoutBefore + 1;

// One made-up window over `song` (which must have chords) from already drawn
// numbers: the activation chord `a`, its length in chords, the deactivation
// node's offset in ticks, and the squeeze-out pick (0 to kSqoutPicks - 1, or
// -1 for none). Every generator of made-up windows, the Lcg one below and a
// property test's, draws its numbers and calls this, so the shape is here once.
inline hydra::ReplayWindow window_over(const hydra::Song& song, size_t a, size_t len,
                                       int64_t deact_offset, int sqout_pick) {
    const size_t n = song.sequence.size();
    const size_t d = std::min(a + len, n - 1);
    hydra::ReplayWindow w;
    w.act_tick = song.sequence[a].timecode.ticks();
    w.deact_tick = song.sequence[d].timecode.ticks() + deact_offset;
    if (sqout_pick >= 0) {
        const size_t lo = d >= kSqoutBefore ? d - kSqoutBefore : 0;
        const size_t q = std::min(lo + static_cast<size_t>(sqout_pick), n - 1);
        w.sqout_tick = song.sequence[q].timecode.ticks();
    }
    return w;
}

// `count` made-up windows over `song`, drawn from an Lcg seeded with `seed`.
inline std::vector<hydra::ReplayWindow> synthetic_windows(const hydra::Song& song, size_t count,
                                                          uint64_t seed) {
    std::vector<hydra::ReplayWindow> out;
    const size_t n = song.sequence.size();
    if (n == 0) return out;
    Lcg rng{seed};
    out.reserve(count);
    for (size_t j = 0; j < count; ++j) {
        const size_t a = rng.next() % n;
        const size_t len = 1 + rng.next() % kMaxWindowChords;
        const int64_t offset =
            rng.next() % kDeactOffsetOneIn == 0 ? static_cast<int64_t>(rng.next() % kDeactOffsetRange) : 0;
        const int pick =
            rng.next() % kSqoutOneIn == 0 ? static_cast<int>(rng.next() % kSqoutPicks) : -1;
        out.push_back(window_over(song, a, len, offset, pick));
    }
    return out;
}

}  // namespace testreplay

#endif  // HYDRA_TESTS_REPLAY_WINDOWS_H
