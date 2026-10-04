// Which phrase chord a Star Power end can squeeze out: one rule, asked by
// the search graph (it claims this chord for its deactivation edge), by the
// engine (it asks whether the running activation can squeeze that chord),
// and by hydra_replay (it resolves a typed offset and words its warning
// with it).

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

// Whether the activation on `act_tick` can squeeze the phrase chord on
// `chord_tick` in or out. A squeeze moves a phrase the player collects while
// Star Power runs across its end. A phrase at or before the activation chord
// was banked before Star Power started, so it is not this activation's to
// squeeze, even when it sits inside the window (500 ms can span more than
// one SP bar at a fast tempo or a short measure).
inline bool activation_can_squeeze(int64_t act_tick, int64_t chord_tick) {
    return chord_tick > act_tick;
}

// The last phrase chord at or before `act_tick` that one of this
// activation's SP ends could still hold in its squeeze window, or nullptr.
// `earliest_end` is the first SP end at which the activation can squeeze
// anything: one SP bar after it (an SP end X decides a squeeze-out or a
// plain end only for a path whose end is X or X plus one bar, and the end is
// never under two bars past the activation). Two running activations whose
// answer here is the same face the same squeeze choices at every later SP
// end, because activation_can_squeeze then gives the same answer for every
// window chord. The search groups paths by it (engine.cpp). One tick of
// slack keeps the reach wide enough for plusmeasure's rounding.
inline const SongTimestamp* banked_phrase_in_reach(const Song& song, int64_t act_tick,
                                                   const Timecode& earliest_end) {
    const std::vector<SongTimestamp>& seq = song.sequence;
    auto it = std::upper_bound(seq.begin(), seq.end(), act_tick,
                               [](int64_t t, const SongTimestamp& ts) {
                                   return t < ts.timecode.ticks();
                               });
    while (it != seq.begin() && !std::prev(it)->flag_sp) --it;
    if (it == seq.begin()) return nullptr;
    const SongTimestamp& banked = *std::prev(it);
    const double reach_ms =
        song.timing().timecode(std::max<int64_t>(earliest_end.ticks() - 1, act_tick)).ms();
    return within_squeeze_window(offset_from_sp_end(banked.timecode.ms(), reach_ms))
               ? &banked
               : nullptr;
}

// The phrase chord the activation on `act_tick` can squeeze out at `sp_end`:
// the window's chord above, or nullptr when there is none or the activation
// cannot squeeze it. The graph names one chord per SP end, so when that one
// was banked before the activation, nothing is squeezed there.
inline const SongTimestamp* sqout_chord(const Song& song, const Timecode& sp_end,
                                        int64_t act_tick) {
    const SongTimestamp* c = sqout_chord(song, sp_end);
    return c && activation_can_squeeze(act_tick, c->timecode.ticks()) ? c : nullptr;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_SQOUT_CHORD_H
