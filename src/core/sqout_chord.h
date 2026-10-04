// Which phrase chord a Star Power end can squeeze out: one rule, asked by
// the search graph (it claims this chord for its deactivation edge) and by
// hydra_replay (it resolves a typed offset and words its warning with it).

#ifndef HYDRA_CORE_SQOUT_CHORD_H
#define HYDRA_CORE_SQOUT_CHORD_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

#include "core/model.h"
#include "parse/song.h"

namespace hydra::core {

// The first SP phrase chord, in chart order, strictly inside the squeeze
// window around `sp_end`. nullptr when there is none. Chords are in tick
// order and time rises with tick, so the window is one run of chords: find
// the first chord at or after the end, step back to the run's start, then
// take the run's first phrase chord.
inline const SongTimestamp* sqout_chord(const Song& song, const Timecode& sp_end) {
    const std::vector<SongTimestamp>& seq = song.sequence;
    auto inside = [&](const SongTimestamp& ts) {
        return within_squeeze_window(offset_from_sp_end(ts.timecode.ms(), sp_end.ms()));
    };
    auto it = std::lower_bound(seq.begin(), seq.end(), sp_end.ticks(),
                               [](const SongTimestamp& ts, int64_t t) {
                                   return ts.timecode.ticks() < t;
                               });
    while (it != seq.begin() && inside(*std::prev(it))) --it;
    for (; it != seq.end() && inside(*it); ++it)
        if (it->flag_sp) return &*it;
    return nullptr;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_SQOUT_CHORD_H
