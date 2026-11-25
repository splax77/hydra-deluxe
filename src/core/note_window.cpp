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
    out.chords.reserve(song.sequence.size());
    for (const SongTimestamp& ts : song.sequence) {
        out.ticks.push_back(ts.timecode.ticks());
        out.ms.push_back(ts.timecode.ms());
        out.chords.push_back(ts.chord);
    }
    out.sides_ms = chord_sides_ms(out.ms);
    return out;
}

// ---- the Dynamic Hit Window row --------------------------------------------

double scaled_anchor_ms(double anchor_side_ms, double ratio) { return anchor_side_ms * ratio; }

double squeeze_safe_ms(double anchor_side_ms, double ratio, double squeeze_side_ms, double needed_ms) {
    return scaled_anchor_ms(anchor_side_ms, ratio) + squeeze_side_ms - needed_ms;
}

double early_fill_safe_ms(double ready_side_ms, double early_fill_ms) { return ready_side_ms - early_fill_ms; }

bool safe_is_short(double safe_ms) { return safe_ms < 0.0; }

// ---- which way each squeeze note is pressed ---------------------------------

// The squeeze sentence (app::squeeze_sentences) prints its "early" or "late"
// from this answer, and squeeze_multiplier (core/squeeze_rating.h) hands out
// the anchor's press from it.
SqueezePresses squeeze_presses(const SPSqueeze& q) {
    const bool squeezed_late = (q.kind == SqueezeKind::SqOut) != q.needs_no_timing();
    const Press squeezed = squeezed_late ? Press::Late : Press::Early;
    const Press anchor = squeezed_late ? Press::Early : Press::Late;
    return {squeezed, anchor};
}

double transfer_side(const TransferScale& scale, Press press) {
    return press == Press::Early ? scale.early : scale.late;
}

// ---- the row's word for each note ------------------------------------------

const char* role_word(EndRole role) {
    switch (role) {
        case EndRole::Frontend: return "frontend";
        case EndRole::Backend: return "backend";
        case EndRole::SpReady: return "";
    }
    return "";
}

namespace {

// The side of the chord on `tick`, or unset when no chord sits there.
std::optional<double> side_at(const ChordTimes& chords, std::optional<int64_t> tick) {
    if (!tick) return std::nullopt;
    const std::optional<int> c = chords.at(*tick);
    if (!c) return std::nullopt;
    return chords.sides_ms[static_cast<size_t>(*c)];
}

}  // namespace

std::optional<WindowEnds> squeeze_window_ends(const Activation& act, size_t k, const ChordTimes& chords) {
    if (k >= act.sqinouts.size()) return std::nullopt;
    const SPSqueeze& q = act.sqinouts[k];
    const std::optional<PricedScale> priced = squeeze_multiplier(act, k);
    if (!priced || !act.squeeze_end_tick(k)) return std::nullopt;
    const std::optional<int64_t> anchor_tick = act.squeeze_anchor_tick(k);
    const std::optional<int64_t> squeezed_tick = act.squeeze_chord_tick(k);
    const std::optional<double> anchor = side_at(chords, anchor_tick);
    const std::optional<double> squeezed = side_at(chords, squeezed_tick);
    if (!anchor || !squeezed) return std::nullopt;
    WindowEnds out;
    out.ends.push_back(WindowEnd{EndRole::Frontend, *anchor_tick, *anchor});
    out.ends.push_back(WindowEnd{EndRole::Backend, *squeezed_tick, *squeezed});
    out.priced = priced;
    out.frontend_effective_ms = scaled_anchor_ms(*anchor, priced->ratio);
    out.safe_ms = squeeze_safe_ms(*anchor, priced->ratio, *squeezed, q.difficulty());
    return out;
}

std::optional<WindowEnds> early_fill_window_end(const Activation& act, const ChordTimes& chords) {
    const std::optional<int64_t> tick = act.sp_ready_tick();
    const std::optional<double> ready = side_at(chords, tick);
    if (!ready) return std::nullopt;
    WindowEnds out;
    out.ends.push_back(WindowEnd{EndRole::SpReady, *tick, *ready});
    out.safe_ms = early_fill_safe_ms(*ready, *act.e_difficulty(/*verbose=*/true));
    return out;
}

std::optional<WindowEnds> early_fill_row(const Activation& act, const ChordTimes& chords) {
    if (!act.is_e_critical()) return std::nullopt;
    return early_fill_window_end(act, chords);
}

}  // namespace hydra
