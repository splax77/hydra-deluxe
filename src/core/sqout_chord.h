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
#include <optional>
#include <stdexcept>
#include <vector>

#include "core/backend_value.h"
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

// The phrase the SP end `sp_end` offers the running window to squeeze in or
// out (D36). A squeeze-out means Star Power runs out before the phrase's
// note is hit, and every later phrase is hit after that note. So:
//  - Early side: the only phrase at or before the end a window can squeeze
//    is its newest one, and only at the end that phrase's step moved its SP
//    end from, and only when the window has not squeezed it in already (a
//    phrase is squeezed in only once, D34). `newest_tick` is the window's
//    newest SP-end step's phrase; `newest_moved_from` is the end that step
//    moved, when that end holds the phrase in its squeeze window (nullopt
//    otherwise, and for an Activation step). Banked phrases never have a
//    step, so they are never offered (D18).
//  - Late side: when the window's end `path_end` is this end, the first
//    phrase after it that the window has not squeezed in (D34).
// Two SP ends one tick apart that move one bar on to the same tick each
// offer the phrase only to the window whose step moved from them.
//
// [first, last) is the end's window phrase chords in chart order; `tick_of`
// reads a chord's tick, `squeezed_in(tick)` says whether this window
// already squeezed that chord in. Returns `last` when nothing is offered.
// The search and the replay both pick through this one function. When the
// early side holds but the range lacks the newest phrase, the caller's
// window list and its step disagree, an impossible state: this throws.
template <class It, class TickOf, class SqueezedIn>
It offered_phrase(It first, It last, int64_t sp_end, std::optional<int64_t> path_end,
                  int64_t newest_tick, std::optional<int64_t> newest_moved_from, TickOf tick_of,
                  SqueezedIn squeezed_in) {
    using Ref = decltype(*first);
    if (newest_moved_from == sp_end && !squeezed_in(newest_tick)) {
        const It it = std::find_if(first, last,
                                   [&](Ref c) { return tick_of(c) == newest_tick; });
        if (it == last)
            throw std::logic_error("an SP end holds no choice for the phrase its step moved");
        return it;
    }
    if (path_end != sp_end) return last;
    return std::find_if(first, last, [&](Ref c) {
        const int64_t tick = tick_of(c);
        return after_sp_end(tick, sp_end) && !squeezed_in(tick);
    });
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

// The phrase chords the activation on `act_tick` could have squeezed out at
// `sp_end` (offered_phrase over the window's chords), in chart order: zero,
// one or two. `squeezed_in` lists the phrase chords this window already
// squeezed in, when the caller knows them (a stored path's SqIn steps).
// A window known to have ended plainly at `sp_end` (a stored record without
// a squeeze-out) had its end there, so only the late side applies. A window
// typed by hand carries no history, so both sides are possible: its newest
// step may be the last phrase at or before the end that it could collect
// (after the activation, not squeezed in), moved from this end; or its end
// may be this end.
inline std::vector<const SongTimestamp*> sqout_chords(const Song& song, const Timecode& sp_end,
                                                      int64_t act_tick,
                                                      const std::vector<int64_t>& squeezed_in,
                                                      bool ended_plainly) {
    const std::vector<const SongTimestamp*> window = squeeze_window_phrases(song, sp_end);
    const auto tick_of = [](const SongTimestamp* c) { return c->timecode.ticks(); };
    const auto is_in = [&squeezed_in](int64_t tick) {
        return std::find(squeezed_in.begin(), squeezed_in.end(), tick) != squeezed_in.end();
    };
    const int64_t d = sp_end.ticks();
    std::vector<const SongTimestamp*> out;
    if (!ended_plainly) {
        const SongTimestamp* newest = nullptr;
        for (const SongTimestamp* c : window)
            if (!after_sp_end(tick_of(c), d) && activation_can_squeeze(act_tick, tick_of(c)))
                newest = c;
        if (newest) {
            const auto it = offered_phrase(window.begin(), window.end(), d, std::nullopt,
                                           tick_of(newest), d, tick_of, is_in);
            if (it != window.end()) out.push_back(*it);
        }
    }
    const auto late = offered_phrase(window.begin(), window.end(), d, d, 0, std::nullopt,
                                     tick_of, is_in);
    if (late != window.end()) out.push_back(*late);
    return out;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_SQOUT_CHORD_H
