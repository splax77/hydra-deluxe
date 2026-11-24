// The game's own per-note hit window, at 100% speed in normal mode, and the
// Dynamic Hit Window row each timing badge shows.
//
// Clone Hero sizes each note's window from the gaps to its two neighbours.
// The rule is TM-17 to TM-22 and TM-51 on docs/clone-hero/03-timing.md, and it
// is worked out here only. The hydra_replay schedule tool and the Paths tab's
// row both call it.
//
// Three readers ask the row: the Paths tab, the hydra_replay schedule tool
// and the safe-paths search (EngineOptions::safe_paths, search/engine.h).
// Hydra's ratings, tiers and badges still keep kTimingStepMs (core/model.h),
// a separate number (D270 call 8).

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

// ---- which way each squeeze note is pressed ---------------------------------
// A squeeze's two notes are each pressed off their chart time, one early and
// one late. This is the one owner of which way each goes.

enum class Press { Early, Late };

// The way each of a squeeze's two notes is pressed: the squeezed note (the SP
// phrase's last note, Activation::squeeze_chord_tick) and the anchor (the
// note whose timing moves that SP end, Activation::squeeze_anchor_tick). The
// anchor is always pressed the other way from the squeezed note.
struct SqueezePresses {
    Press squeezed;
    Press anchor;
};

// The squeezed note's way is the word the squeeze sentence prints for it,
// "early" or "late". It follows the squeeze's kind and
// SPSqueeze::needs_no_timing together, never the kind alone.
SqueezePresses squeeze_presses(const SPSqueeze& q);

// Which side of a stored transfer scale a note pressed `press` counts
// through: the early side for an early press, the late side for a late one.
// Which stored scale it is (Activation::transfer_post or SPSqueeze::transfer)
// is the caller's.
double transfer_side(const TransferScale& scale, Press press);

// ---- the row's word for each note ------------------------------------------
// The Dynamic Hit Window row names a squeeze's notes by the user's words,
// frontend and backend, in the sense CONTEXT.md's "Frontend squeeze" and
// "Backend squeeze" give them: the activation side and the SP-end side. An
// early fill row's one note gets no word (the user's ruling: "No word").

// The part a row's note plays: a squeeze's anchor (the frontend), its
// squeezed note (the backend), or an early fill's SP-ready note.
enum class EndRole { Frontend, Backend, SpReady };

// The word the row prints before a note's window: "frontend", "backend", or
// empty for the SP-ready note.
const char* role_word(EndRole role);

// One end of a row: the part its note plays, the tick of its note (a place in
// ChordTimes, which also names the chord there) and that note's window side.
struct WindowEnd {
    EndRole role = EndRole::Frontend;
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

// Squeeze `k` of `act`, of either kind: the anchor as the frontend, then the
// squeezed note as the backend. The anchor's side counts through
// transfer_side of the squeeze's stored scale, on the side of the anchor's
// press (squeeze_presses); the stored scale is Activation::transfer_post for a
// squeeze out and the squeeze's own SPSqueeze::transfer for a squeeze in.
// Unset when `act` has no squeeze `k`, that scale is unset, the record names
// no chord or no end for it, or no chord in `chords` sits on either tick.
std::optional<WindowEnds> squeeze_window_ends(const Activation& act, size_t k, const ChordTimes& chords);

// The early fill of `act`: the note that made Star Power ready
// (Activation::sp_ready_tick), as the SpReady end.
// Unset when the record holds too few bar arrivals to name that note, or no
// chord sits on its tick (a bar that arrived at the previous SP end). Whether
// the row shows at all is early_fill_row's question.
std::optional<WindowEnds> early_fill_window_end(const Activation& act, const ChordTimes& chords);

// The early fill row `act` shows: early_fill_window_end on an activation that
// shows the early fill badge and the "Early fill" line
// (Activation::is_e_critical, the test badge_timings uses), required,
// optional or with time to spare alike; unset on any other. The Paths tab and
// the safe-paths search both ask it.
std::optional<WindowEnds> early_fill_row(const Activation& act, const ChordTimes& chords);

}  // namespace hydra

#endif  // HYDRA_CORE_NOTE_WINDOW_H
