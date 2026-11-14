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
// ruled "just show it alongside."

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

// A song's chords in chart order (Song::sequence): each one's tick, time,
// window side and chord. Built once from the song by chord_times, so every
// reader sees the same list.
struct ChordTimes {
    std::vector<int64_t> ticks;
    std::vector<double> ms;
    std::vector<double> sides_ms;  // chord_sides_ms over `ms`
    std::vector<Chord> chords;     // SongTimestamp::chord at each place
    // The chord on `tick`, by its place in the lists, or unset when no chord
    // sits there.
    std::optional<int> at(int64_t tick) const;
};
ChordTimes chord_times(const Song& song);

// ---- the Dynamic Hit Window row --------------------------------------------
// Each sum is "what the game's windows give, minus what the timing needs".

// A squeeze's safe sum: the anchor's side moves the SP end by `ratio` per ms,
// and the squeezed note's side covers the rest of the `needed_ms` it must
// cross (SPSqueeze::difficulty). Squeeze in and squeeze out share it;
// squeeze_window_ends picks `ratio` (transfer_side).
double squeeze_safe_ms(double anchor_side_ms, double ratio, double squeeze_side_ms, double needed_ms);

// An early fill's safe sum: the side of the note that makes Star Power ready,
// less the early fill figure (early_fill_difficulty).
double early_fill_safe_ms(double ready_side_ms, double early_fill_ms);

// Whether a safe sum reads "short" rather than "safe". The user's ruling of
// 2026-11-13 sets the edge: "xx ms safe >=0 ms, xx ms short <0ms".
bool safe_is_short(double safe_ms);

// The old names of the three above, kept only until T2 of the Dynamic Hit
// Window plan moves their callers over and removes them.
inline double squeeze_spare_ms(double anchor_side_ms, double ratio, double squeeze_side_ms, double needed_ms) {
    return squeeze_safe_ms(anchor_side_ms, ratio, squeeze_side_ms, needed_ms);
}
inline double early_fill_spare_ms(double ready_side_ms, double early_fill_ms) {
    return early_fill_safe_ms(ready_side_ms, early_fill_ms);
}
inline bool spare_is_short(double spare_ms) { return safe_is_short(spare_ms); }

// ---- which end each note is on ---------------------------------------------
// The Dynamic Hit Window row names each note's end with the user's words:
// the frontend or the backend (the ruling of 2026-11-13). These are the one
// owner of that question; the Paths tab's squeeze sentence, the row and the
// schedule tool read them.

enum class NoteEnd { Frontend, Backend };

// The end of a squeeze each of its two notes is on: the squeezed note (the SP
// phrase's last note, Activation::squeeze_chord_tick) and the anchor (the
// note whose timing moves that SP end, Activation::squeeze_anchor_tick). The
// anchor is always on the other end from the squeezed note.
struct SqueezeEnds {
    NoteEnd squeezed;
    NoteEnd anchor;
};

// The squeezed note's end is the word the squeeze sentence prints for it:
// "late" is the backend, "early" the frontend. It follows the squeeze's kind
// and SPSqueeze::needs_no_timing together, never the kind alone.
SqueezeEnds squeeze_ends(const SPSqueeze& q);

// The end of the note that makes Star Power ready, from the sign the "Early
// fill:" line prints (early_fill_difficulty of `e_offset`, the same figure as
// Activation::e_difficulty): 0 or more is the frontend, below 0 the backend.
NoteEnd early_fill_end(double e_offset);

// Which side of a stored transfer scale a note on `end` counts through: the
// early side for the frontend, the late side for the backend. Which stored
// scale it is (Activation::transfer_post or SPSqueeze::transfer) is the
// caller's.
double transfer_side(const TransferScale& scale, NoteEnd end);

// One end of a row: which end it is, the tick of its note (a place in
// ChordTimes, which also names the chord there) and that note's window side.
struct WindowEnd {
    NoteEnd end = NoteEnd::Frontend;
    int64_t tick = 0;
    double side_ms = 0.0;
};

// A row's ends, frontend first (one end for an early fill, two for a
// squeeze), and the safe sum.
struct WindowEnds {
    std::vector<WindowEnd> ends;
    double safe_ms = 0.0;
    bool is_short() const { return safe_is_short(safe_ms); }
};

// Squeeze `k` of `act`, of either kind. Each note's end is squeeze_ends'. The
// anchor's side counts through transfer_side of the squeeze's stored scale at
// the anchor's end; the stored scale is Activation::transfer_post for a
// squeeze out and the squeeze's own SPSqueeze::transfer for a squeeze in.
// Unset when `act` has no squeeze `k`, that scale is unset, the record names
// no chord or no end for it, or no chord in `chords` sits on either tick.
std::optional<WindowEnds> squeeze_window_ends(const Activation& act, size_t k, const ChordTimes& chords);

// The early fill of `act`: the note that made Star Power ready
// (Activation::sp_ready_tick), at early_fill_end of Activation::e_offset.
// Unset when the record holds too few bar arrivals to name that note, or no
// chord sits on its tick (a bar that arrived at the previous SP end). Whether
// the row shows at all is the caller's question.
std::optional<WindowEnds> early_fill_window_end(const Activation& act, const ChordTimes& chords);

// ---- the old row functions ------------------------------------------------
// Kept only until T2 of the Dynamic Hit Window plan moves the Paths tab to the
// functions above and removes these.

// One row's numbers: the first window side, the second when the timing has
// two notes, and the safe sum. Its field keeps the old name until T2,
// because the Paths tab reads it and a data member cannot forward to a new
// name.
struct WindowRow {
    double first_side_ms = 0.0;
    std::optional<double> second_side_ms;
    double spare_ms = 0.0;
    bool is_short() const { return safe_is_short(spare_ms); }
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
