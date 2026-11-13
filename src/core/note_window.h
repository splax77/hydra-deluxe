// The game's own per-note hit window, at 100% speed in normal mode, and the
// Dynamic Hit Window row each timing badge shows beside Hydra's fixed one.
//
// Clone Hero sizes each note's window from the gaps to its two neighbours.
// The rule is TM-17 to TM-22 and TM-51 on docs/clone-hero/03-timing.md, and it
// is worked out here only. The hydra_replay schedule tool and the Paths tab's
// row both call it.
//
// The row is display only. Hydra's ratings, tiers, badges and search keep
// kHitWindowMs (core/model.h), a separate number (D270 call 8): the user
// ruled "just show it alongside." The plan is
// docs/superpowers/plans/2026-11-12-dynamic-hit-window.md.

#ifndef HYDRA_CORE_NOTE_WINDOW_H
#define HYDRA_CORE_NOTE_WINDOW_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "core/model.h"
#include "core/song.h"

namespace hydra {

// ---- the game's per-note window ------------------------------------------
// hydra_replay schedule writes each of these into its file's "constants"
// object, where its tests pin them.

// The game's normal-mode clamps on one half-gap: TM-18 on
// docs/clone-hero/03-timing.md (0.0375 s and 0.085 s).
inline constexpr double kWindowFrontMs = 37.5;
inline constexpr double kWindowBackMs = 85.0;

// The normal-mode window formula's constants: TM-21 on
// docs/clone-hero/03-timing.md.
inline constexpr double kFormulaLinear = 0.0110924369747899;
inline constexpr double kFormulaSquare = 2.628539e-05;
inline constexpr double kFormulaScale = 170.0;
inline constexpr double kFormulaOffsetMs = 20.0;

// One side's clamped half-gap to a neighbour `gap_ms` away; unset means no
// neighbour.
double half_gap_ms(std::optional<double> gap_ms);

// The whole window the formula gives for a summed gap of `summed_gap_ms`.
double whole_window_ms(double summed_gap_ms);

// How far the window reaches on each side of a note, given the gaps to the
// notes before and after it (unset where there is none).
double window_side_ms(std::optional<double> gap_before_ms, std::optional<double> gap_after_ms);

// Each chord's side, from the chords' times in chart order. A chord's
// neighbours are the chords either side of it; its own gems are never each
// other's neighbours.
std::vector<double> chord_sides_ms(const std::vector<double>& chord_ms);

// ---- a song's chords -------------------------------------------------------

// A song's chords in chart order (Song::sequence): each one's tick, time and
// window side. Built once from the song by chord_times, so every reader sees
// the same list.
struct ChordTimes {
    std::vector<int64_t> ticks;
    std::vector<double> ms;
    std::vector<double> sides_ms;  // chord_sides_ms over `ms`
    // The chord on `tick`, by its place in the lists, or unset when no chord
    // sits there.
    std::optional<int> at(int64_t tick) const;
};
ChordTimes chord_times(const Song& song);

// ---- the Dynamic Hit Window row --------------------------------------------
// Each sum is "what the game's windows give, minus what the timing needs".

// A squeeze's spare: the anchor's side moves the SP end by `ratio` per ms, and
// the squeezed note's side covers the rest of the `needed_ms` it must cross
// (SPSqueeze::difficulty). Squeeze in and squeeze out share it; only which
// side of which scale `ratio` is differs, and the row functions below pick it.
double squeeze_spare_ms(double anchor_side_ms, double ratio, double squeeze_side_ms, double needed_ms);

// An early fill's spare: the side of the note that makes Star Power ready,
// less the early fill figure (early_fill_difficulty).
double early_fill_spare_ms(double ready_side_ms, double early_fill_ms);

// Whether a spare reads "short". Exactly 0 reads "spare"; that edge is a
// wording choice, not a game fact.
bool spare_is_short(double spare_ms);

// One row's numbers: the first window side, the second when the timing has
// two notes, and the spare.
struct WindowRow {
    double first_side_ms = 0.0;
    std::optional<double> second_side_ms;
    double spare_ms = 0.0;
    bool is_short() const { return spare_is_short(spare_ms); }
};

// Squeeze `k` of `act`, when it is a squeeze out: the anchor's side first
// (Activation::squeeze_anchor_tick, hit early), then the squeezed note's
// (Activation::squeeze_chord_tick, hit late), at the early side of
// Activation::transfer_post. Unset when squeeze `k` is not a squeeze out, the
// scale is unset, the record names no chord or no end for it, or no chord in
// `chords` sits on either tick.
std::optional<WindowRow> squeeze_out_window(const Activation& act, size_t k, const ChordTimes& chords);

// Squeeze `k` of `act`, when it is a squeeze in: the anchor's side first (hit
// late), then the squeezed note's (hit early), at the late side of the
// squeeze's own SPSqueeze::transfer. Unset on the same gaps as
// squeeze_out_window.
std::optional<WindowRow> squeeze_in_window(const Activation& act, size_t k, const ChordTimes& chords);

// The early fill of `act`: the side of the note that made Star Power ready
// (Activation::sp_ready_tick), with no second side. Unset when the record
// holds too few bar arrivals to name that note, or no chord sits on its tick
// (a bar that arrived at the previous SP end). Whether the row shows at all
// is the caller's question.
std::optional<WindowRow> early_fill_window(const Activation& act, const ChordTimes& chords);

}  // namespace hydra

#endif  // HYDRA_CORE_NOTE_WINDOW_H
