#include "core/note_window.h"

#include <algorithm>

namespace hydra {

// ---- the game's per-note window ------------------------------------------

// TM-19: half the gap, held between the front and the back; a missing
// neighbour, or one at the same time, gives the back.
double half_gap_ms(std::optional<double> gap_ms) {
    if (!gap_ms || *gap_ms == 0.0) return kWindowBackMs;
    return std::min(std::max(*gap_ms / 2.0, kWindowFrontMs), kWindowBackMs);
}

// TM-21. The function has no clamp of its own; its input is clamped by
// half_gap_ms on each side (TM-22).
double whole_window_ms(double x) {
    return (x * kFormulaLinear - x * x * kFormulaSquare) * kFormulaScale - kFormulaOffsetMs;
}

// TM-20: the note gets half the formula's output on each side.
double window_side_ms(std::optional<double> gap_before_ms, std::optional<double> gap_after_ms) {
    return whole_window_ms(half_gap_ms(gap_before_ms) + half_gap_ms(gap_after_ms)) / 2.0;
}

// TM-51: the neighbours are the nearest notes at another tick, so the
// chords either side.
std::vector<double> chord_sides_ms(const std::vector<double>& chord_ms) {
    std::vector<double> sides;
    sides.reserve(chord_ms.size());
    for (size_t i = 0; i < chord_ms.size(); ++i) {
        const std::optional<double> before =
            i > 0 ? std::optional<double>(chord_ms[i] - chord_ms[i - 1]) : std::nullopt;
        const std::optional<double> after = i + 1 < chord_ms.size()
                                                ? std::optional<double>(chord_ms[i + 1] - chord_ms[i])
                                                : std::nullopt;
        sides.push_back(window_side_ms(before, after));
    }
    return sides;
}

// ---- a song's chords -------------------------------------------------------

std::optional<int> ChordTimes::at(int64_t tick) const {
    const auto it = std::lower_bound(ticks.begin(), ticks.end(), tick);
    if (it == ticks.end() || *it != tick) return std::nullopt;
    return static_cast<int>(it - ticks.begin());
}

ChordTimes chord_times(const Song& song) {
    ChordTimes out;
    out.ticks.reserve(song.sequence.size());
    out.ms.reserve(song.sequence.size());
    for (const SongTimestamp& ts : song.sequence) {
        out.ticks.push_back(ts.timecode.ticks());
        out.ms.push_back(ts.timecode.ms());
    }
    out.sides_ms = chord_sides_ms(out.ms);
    return out;
}

// ---- the Dynamic Hit Window row --------------------------------------------

double squeeze_spare_ms(double anchor_side_ms, double ratio, double squeeze_side_ms, double needed_ms) {
    return anchor_side_ms * ratio + squeeze_side_ms - needed_ms;
}

double early_fill_spare_ms(double ready_side_ms, double early_fill_ms) { return ready_side_ms - early_fill_ms; }

bool spare_is_short(double spare_ms) { return spare_ms < 0.0; }

namespace {

// The side of the chord on `tick`, or unset when no chord sits there.
std::optional<double> side_at(const ChordTimes& chords, std::optional<int64_t> tick) {
    if (!tick) return std::nullopt;
    const std::optional<int> c = chords.at(*tick);
    if (!c) return std::nullopt;
    return chords.sides_ms[static_cast<size_t>(*c)];
}

// Squeeze `k`'s row at the stored scale side `ratio` picks, when the squeeze
// is of `kind`.
template <class PickRatio>
std::optional<WindowRow> squeeze_window(const Activation& act, size_t k, const ChordTimes& chords,
                                        SqueezeKind kind, PickRatio pick_ratio) {
    if (k >= act.sqinouts.size() || act.sqinouts[k].kind != kind) return std::nullopt;
    const SPSqueeze& q = act.sqinouts[k];
    const std::optional<double> ratio = pick_ratio(q);
    if (!ratio || !act.squeeze_end_tick(k)) return std::nullopt;
    const std::optional<double> anchor = side_at(chords, act.squeeze_anchor_tick(k));
    const std::optional<double> squeezed = side_at(chords, act.squeeze_chord_tick(k));
    if (!anchor || !squeezed) return std::nullopt;
    WindowRow row;
    row.first_side_ms = *anchor;
    row.second_side_ms = *squeezed;
    row.spare_ms = squeeze_spare_ms(*anchor, *ratio, *squeezed, q.difficulty());
    return row;
}

}  // namespace

std::optional<WindowRow> squeeze_out_window(const Activation& act, size_t k, const ChordTimes& chords) {
    return squeeze_window(act, k, chords, SqueezeKind::SqOut, [&](const SPSqueeze&) -> std::optional<double> {
        if (!act.transfer_post) return std::nullopt;
        return act.transfer_post->early;
    });
}

std::optional<WindowRow> squeeze_in_window(const Activation& act, size_t k, const ChordTimes& chords) {
    return squeeze_window(act, k, chords, SqueezeKind::SqIn, [](const SPSqueeze& q) -> std::optional<double> {
        if (!q.transfer) return std::nullopt;
        return q.transfer->late;
    });
}

std::optional<WindowRow> early_fill_window(const Activation& act, const ChordTimes& chords) {
    const std::optional<double> ready = side_at(chords, act.sp_ready_tick());
    if (!ready) return std::nullopt;
    WindowRow row;
    row.first_side_ms = *ready;
    row.spare_ms = early_fill_spare_ms(*ready, *act.e_difficulty(/*verbose=*/true));
    return row;
}

}  // namespace hydra
