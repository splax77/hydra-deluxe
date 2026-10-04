// Which phrase chord a Star Power end can squeeze in or out: one rule, asked
// by the search graph (it lists the window's phrase chords on each
// deactivation edge), by the engine (it picks the one the running window is
// offered) and by hydra_replay (it resolves a typed offset and words its
// warning with it).

#ifndef HYDRA_CORE_SQOUT_CHORD_H
#define HYDRA_CORE_SQOUT_CHORD_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

#include "core/model.h"
#include "parse/song.h"

namespace hydra::core {

// Every SP phrase chord strictly inside the squeeze window around `sp_end`,
// in chart order. Empty when there is none. Chords are in tick order and
// time rises with tick, so the window is one run of chords: find the first
// chord at or after the end, step back to the run's start, then keep the
// run's phrase chords.
inline std::vector<const SongTimestamp*> squeeze_window_phrases(const Song& song,
                                                                const Timecode& sp_end) {
    const std::vector<SongTimestamp>& seq = song.sequence;
    auto inside = [&](const SongTimestamp& ts) {
        return within_squeeze_window(offset_from_sp_end(ts.timecode.ms(), sp_end.ms()));
    };
    auto it = std::lower_bound(seq.begin(), seq.end(), sp_end.ticks(),
                               [](const SongTimestamp& ts, int64_t t) {
                                   return ts.timecode.ticks() < t;
                               });
    while (it != seq.begin() && inside(*std::prev(it))) --it;
    std::vector<const SongTimestamp*> out;
    for (; it != seq.end() && inside(*it); ++it)
        if (it->flag_sp) out.push_back(&*it);
    return out;
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

// The phrase an SP end offers the running window to squeeze in or out (D34):
// the first phrase chord in its squeeze window, in chart order, that the
// window can still squeeze. A phrase banked before the activation is not the
// window's to squeeze (activation_can_squeeze, D18). A phrase the window
// already squeezed in is spent: a phrase can be squeezed in only once. Two
// SP ends one tick apart move one bar on to the same tick, so without this
// both would offer the same phrase to the same path.
//
// [first, last) is the window's phrase chords in chart order; `tick_of`
// reads a chord's tick, `squeezed_in(tick)` says whether this window
// already squeezed that chord in. Returns `last` when nothing is offered.
// The search and the replay both pick through this one function.
template <class It, class TickOf, class SqueezedIn>
It offered_phrase(It first, It last, int64_t act_tick, TickOf tick_of, SqueezedIn squeezed_in) {
    for (; first != last; ++first) {
        const int64_t tick = tick_of(*first);
        if (activation_can_squeeze(act_tick, tick) && !squeezed_in(tick)) return first;
    }
    return last;
}

// The last phrase chord the activation on `act_tick` cannot squeeze (the
// banked side of activation_can_squeeze) that one of its SP ends could still
// hold in its squeeze window, or nullptr.
// `earliest_end` is the first SP end at which the activation can squeeze
// anything: one SP bar after it (an SP end X decides a squeeze-out or a
// plain end only for a path whose end is X or X's moved end: one bar on, or
// the cap's ceiling when that comes first (ScoreGraph::extend_deacts). The
// moved end is never more than one bar past X, and the end is never under two
// bars past the activation). Two running activations whose
// answer here is the same face the same squeeze choices at every later SP
// end, because activation_can_squeeze then gives the same answer for every
// window chord. The search groups paths by it (engine.cpp). One tick of
// slack keeps the reach wide enough for plusmeasure's rounding (decision
// D40, recorded in ADR 0014).
inline const SongTimestamp* banked_phrase_in_reach(const Song& song, int64_t act_tick,
                                                   const Timecode& earliest_end) {
    const std::vector<SongTimestamp>& seq = song.sequence;
    // The chords this activation cannot squeeze come first in tick order;
    // activation_can_squeeze says where they stop.
    auto it = std::partition_point(seq.begin(), seq.end(), [act_tick](const SongTimestamp& ts) {
        return !activation_can_squeeze(act_tick, ts.timecode.ticks());
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

// The phrase chord the activation on `act_tick` can squeeze out at `sp_end`
// (offered_phrase over the window's chords), or nullptr. `squeezed_in` lists
// the phrase chords this window already squeezed in, when the caller knows
// them (a stored path's SqIn steps); none otherwise.
inline const SongTimestamp* sqout_chord(const Song& song, const Timecode& sp_end,
                                        int64_t act_tick,
                                        const std::vector<int64_t>& squeezed_in = {}) {
    const std::vector<const SongTimestamp*> window = squeeze_window_phrases(song, sp_end);
    const auto it = offered_phrase(
        window.begin(), window.end(), act_tick,
        [](const SongTimestamp* c) { return c->timecode.ticks(); },
        [&squeezed_in](int64_t tick) {
            return std::find(squeezed_in.begin(), squeezed_in.end(), tick) != squeezed_in.end();
        });
    return it == window.end() ? nullptr : *it;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_SQOUT_CHORD_H
