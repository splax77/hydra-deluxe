// hydra_replay schedule: the press schedule the auto-player sends.
//
// The player reads one file and works no Star Power rule out itself. This
// module writes that file from the engine's own facts: each activation's
// SP-end steps and their anchors (Activation, core/model.h), each squeeze's
// stored end, anchor and ratios, the steady spans (core/squeeze_rating.h),
// and which windows paid each chord (ReplayChord::paid_by, core/replay.h).
// The game's per-note hit window at 100% speed and the player's timing
// constants live in replay_schedule.cpp, each with its source.
//
// Only hydra_replay and its tests read this. Nothing under src/ includes it,
// so no Hydra screen reads the per-note window or the constants: the screens
// keep kHitWindowMs and kSqueezeWindowMs (core/model.h). The plan that set
// this up is docs/superpowers/plans/2026-11-08-replay-press-schedule.md.

#ifndef HYDRA_TOOLS_REPLAY_SCHEDULE_H
#define HYDRA_TOOLS_REPLAY_SCHEDULE_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "json.hpp"

#include "core/model.h"
#include "core/replay.h"
#include "parse/song.h"
#include "search/pather.h"

namespace hydra::schedule {

// ---- the game's per-note window ------------------------------------------

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

// ---- the decline mode ------------------------------------------------------

// What a declined chord sends. The mode names' one home: decline_mode_from_name
// reads them, decline_mode_name writes them.
enum class DeclineMode { KickOnly, Nothing, AllButTopPad };
inline constexpr DeclineMode kDeclineModes[] = {DeclineMode::KickOnly, DeclineMode::Nothing,
                                                DeclineMode::AllButTopPad};
const char* decline_mode_name(DeclineMode mode);
std::optional<DeclineMode> decline_mode_from_name(std::string_view name);
// The names, comma-separated, for a refusal line.
std::string decline_mode_list();

// The gems of a declined chord the mode sends, by their place in `notes`
// (Chord::note_list(true) order, as ReplayChord::notes). `refusal` is set,
// and `places` empty, when the mode cannot say which gem to leave out.
struct KeptGems {
    std::vector<size_t> places;
    std::optional<std::string> refusal;
};
KeptGems kept_gems(DeclineMode mode, const std::vector<ReplayNote>& notes);

// ---- the file --------------------------------------------------------------

// One gem as the schedule and `hydra_replay score` both write it.
nlohmann::json replay_note_json(const ReplayNote& note);

// The chart's chord index on each tick, for naming chords in the file.
struct ChordIndex {
    std::vector<int64_t> ticks;  // song.sequence's ticks, in order
    // The chord on `tick`, or unset when no chord sits there.
    std::optional<int> at(int64_t tick) const;
};
ChordIndex chord_index(const Song& song);

// One activation's geometry: its chord, nominal and final ends, every
// SP-end step with its anchor (Activation::end_anchor_tick), the chord the
// player reads the end after (read_end_after, the final end's anchor) and
// every squeeze with its own end, anchor, stored ratios and steady spans.
// A fact the record does not hold is written null and its refusal line is
// added to `refusals`; nothing is worked out again.
nlohmann::json activation_json(const Activation& act, const ChordIndex& chords,
                               const SongTiming& timing, std::vector<std::string>& refusals);

// The whole schedule for one path: "constants", "activations", "chords",
// "squeezes" inside each activation, "presses" and "refusals". `replay` is
// replay_path over the same path's windows.
nlohmann::json schedule_json(const Song& song, const std::vector<const Activation*>& acts,
                             const ReplayResult& replay, DeclineMode mode);

// ---- the command's refusals ------------------------------------------------

// The line `hydra_replay schedule` refuses with when it was given no --path,
// or unset when it was.
std::optional<std::string> path_flag_refusal(const std::string& path_file);

// The line it refuses with when the dump `doc` (read from `path_file`) holds
// no path to plan, or unset when it holds one.
std::optional<std::string> no_path_refusal(const nlohmann::json& doc, const std::string& path_file);

// The line it refuses with when --decline was not given or names no mode.
std::optional<std::string> decline_flag_refusal(const std::optional<std::string>& name);

// The schedule of the path `windows` names (read off a dump, as `score --path`
// reads it): the windows are pinned through search_target so the engine's
// own Activation records come back, then replayed. Throws std::runtime_error
// with a plain line when the engine cannot realize the windows.
nlohmann::json schedule_for_windows(const Song& song, const SearchSettings& settings,
                                    const std::vector<ReplayWindow>& windows,
                                    DeclineMode mode);

}  // namespace hydra::schedule

#endif  // HYDRA_TOOLS_REPLAY_SCHEDULE_H
