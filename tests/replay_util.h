// Replay test helpers that tests/test_replay.cpp and
// tests/test_replay_windows.cpp both use: a hand-built squeeze chart and a
// row-by-row comparison of two replays. Made-up windows over a real song live
// in replay_windows.h, which hydra_bench shares.

#ifndef HYDRA_TESTS_REPLAY_UTIL_H
#define HYDRA_TESTS_REPLAY_UTIL_H

#include <cstdint>
#include <string>

#include "core/replay.h"
#include "parse/song.h"
#include "record_fixtures.h"

namespace hydra::test {

// Round and Round's shape on a hand-built chart: six R+Y chords, 192 ticks a
// beat, 120 BPM, 768 ticks (2000 ms) a measure. Ticks 0, 768, 1536, 2304,
// 3072 are 0, 2000, 4000, 6000, 8000 ms; the phrase chord at 3256 is
// 8479.2 ms.
inline Song squeeze_chart() {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3256}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        if (tick == 3256) mark_phrase_end(ts, tick, song.tick_resolution());
        song.sequence.push_back(ts);
    }
    return song;
}

// The name of the first field where two rows differ, or "" when every field
// of ReplayChord (and of each ReplayNote) is identical. With `scores_only`,
// chord_code and notes are skipped (a scores-only row leaves them empty).
inline std::string first_row_difference(const ReplayChord& a, const ReplayChord& b,
                                        bool scores_only = false) {
    if (a.index != b.index) return "index";
    if (a.tick != b.tick) return "tick";
    if (a.ms != b.ms) return "ms";
    if (a.measure != b.measure) return "measure";
    if (a.beat != b.beat) return "beat";
    if (a.measure_tick != b.measure_tick) return "measure_tick";
    if (a.measures_decimal != b.measures_decimal) return "measures_decimal";
    if (!scores_only) {
        if (a.chord_code != b.chord_code) return "chord_code";
        if (a.notes.size() != b.notes.size()) return "notes.size";
        for (size_t k = 0; k < a.notes.size(); ++k) {
            const ReplayNote& x = a.notes[k];
            const ReplayNote& y = b.notes[k];
            if (x.color != y.color) return "notes.color";
            if (x.cymbal != y.cymbal) return "notes.cymbal";
            if (x.sp_points != y.sp_points) return "notes.sp_points";
            if (x.multiplier != y.multiplier) return "notes.multiplier";
            if (x.dynamics_bonus != y.dynamics_bonus) return "notes.dynamics_bonus";
            if (x.dynamic != y.dynamic) return "notes.dynamic";
        }
    }
    if (a.is_fill != b.is_fill) return "is_fill";
    if (a.is_solo != b.is_solo) return "is_solo";
    if (a.is_sp_phrase_end != b.is_sp_phrase_end) return "is_sp_phrase_end";
    if (a.combo_before != b.combo_before) return "combo_before";
    if (a.combo_after != b.combo_after) return "combo_after";
    if (a.multiplier != b.multiplier) return "multiplier";
    if (a.multiplier_after != b.multiplier_after) return "multiplier_after";
    if (a.in_sp != b.in_sp) return "in_sp";
    if (a.multiplier_shown != b.multiplier_shown) return "multiplier_shown";
    if (!(a.points == b.points)) return "points";
    if (!(a.cum == b.cum)) return "cum";
    if (a.cum_onscreen_total != b.cum_onscreen_total) return "cum_onscreen_total";
    return "";
}

// "" when the two results are identical row for row, else where they differ.
inline std::string first_result_difference(const ReplayResult& a, const ReplayResult& b,
                                           bool scores_only = false) {
    if (a.chords.size() != b.chords.size()) return "row count";
    for (size_t i = 0; i < a.chords.size(); ++i) {
        const std::string d = first_row_difference(a.chords[i], b.chords[i], scores_only);
        if (!d.empty()) return "row " + std::to_string(i) + ": " + d;
    }
    if (!(a.final == b.final)) return "final";
    return "";
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_REPLAY_UTIL_H
