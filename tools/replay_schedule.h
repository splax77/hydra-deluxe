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
#include <utility>
#include <vector>

#include "json.hpp"

#include "core/model.h"
#include "core/replay.h"
#include "core/squeeze_rating.h"  // SteadySpans
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
// (`chord`'s Chord::note_list(true) order, as ReplayChord::notes).
struct KeptGems {
    std::vector<size_t> places;
};
KeptGems kept_gems(DeclineMode mode, const Chord& chord, const std::vector<ReplayNote>& notes);

// ---- the aims ----------------------------------------------------------------
// Design points 4 and 5 of the plan. Each rule is worked out once here, on
// plain numbers, so the tests can pin it on the plan's own figures;
// schedule_json reads the engine's records into these and writes the results.

// One squeeze that needs timing, as its two presses see it: the anchor chord
// (Activation::squeeze_anchor_tick, the overfill chord when the cap clamped,
// else the activation) and the squeeze chord, each with its window side, and
// the end the squeeze is measured against as the engine stored it.
struct SqueezeGeometry {
    SqueezeKind kind = SqueezeKind::SqOut;
    int anchor_chord = 0;
    double anchor_ms = 0.0;
    double anchor_side_ms = 0.0;
    int squeeze_chord = 0;
    double squeeze_ms = 0.0;
    double squeeze_side_ms = 0.0;
    double end_ms = 0.0;
    TransferScale ratio;  // as stored; the side the anchor moves is read off it
    SteadySpans steady;   // steady_spans(anchor, end)
};

// The ratio the anchor's press moves this squeeze's end by: end_scale_early
// for a squeeze-out (the anchor goes early), end_scale_late for a squeeze-in.
double moving_ratio(const SqueezeGeometry& g);

// The lead the equal-room split gives the anchor of `g` alone, in ms (always
// how far from the chord's time; the direction is the squeeze's).
double equal_room_lead_ms(const SqueezeGeometry& g);

// One squeeze's two presses at an anchor lead of `lead_ms`.
struct SqueezeSplit {
    double anchor_aim_ms = 0.0;    // off the anchor chord's time, negative early
    double squeeze_aim_ms = 0.0;   // off the squeeze chord's time
    double anchor_room_ms = 0.0;
    double squeeze_room_ms = 0.0;
    double planned_end_ms = 0.0;   // the end once the anchor is judged kJudgeEarlyMs after its aim
    double aim_end_ms = 0.0;       // the end the squeeze press's aim assumed
    bool feasible = false;         // both rooms clear kRoomMarginMs
};
SqueezeSplit split_at_lead(const SqueezeGeometry& g, double lead_ms);
SqueezeSplit split_squeeze(const SqueezeGeometry& g);  // at equal_room_lead_ms

// The one lead an anchor takes for every timed squeeze it moves: the largest
// any of them needs, so each other one keeps more room than the split gives
// it. Unset, with a plain-words line in `refusals`, when the squeezes need it
// pressed both early and late, when the split would press it the other way
// than its squeeze needs, or when the lead crosses a steady span.
std::optional<double> anchor_lead_ms(const std::vector<SqueezeGeometry>& on_one_anchor,
                                     std::vector<std::string>& refusals);

// A press's room: how far inside the chord's side `side_ms` it stays,
// measured from its aim or its latest judged position, whichever is farther
// from the chord.
double press_room_ms(double aim_ms, double side_ms);

// The latest song clock a press of the chord at `chord_ms` with side
// `side_ms` may still be sent at (TM-27: equal is inside).
double last_send_ms(double chord_ms, double side_ms);

// The live fallbacks' thresholds (design point 5): the read end past which a
// squeeze-out's late press leaves its chord's side, and the read end before
// which a squeeze-in's press before the end lands ahead of it.
double sqout_fallback_after_ms(double chord_ms, double side_ms);
double sqin_fallback_before_ms(double chord_ms, double side_ms);

// The groups a chord's gems `places` (into its Chord::note_list(true) order)
// are pressed in, in firing order: `first` (an activation's activating note)
// alone first, then by `multipliers`, the lowest first. Empty `places` gives
// one empty group: a declined chord that sends nothing is still one press.
std::vector<std::vector<size_t>> press_groups(const std::vector<int>& multipliers,
                                              const std::vector<size_t>& places,
                                              std::optional<size_t> first);

// A squeeze-out chord's inside groups, every gem but the late one (place 0),
// lowest multiplier first. Unset, with a line in `refusals`, when the late
// gem pays less than an inside gem: the late gem goes last, so no press gives
// that order.
std::optional<std::vector<std::vector<size_t>>> squeeze_out_inside_groups(
    int chord, const std::vector<int>& multipliers, std::vector<std::string>& refusals);

// A drum pad: a gem's colour and whether it is a cymbal.
using Pad = std::pair<NoteColor, bool>;

// One chord as the far-press check sees it: its pads in the chart, and the
// pads the player sends for it (a declined chord's kept gems only).
struct PadChord {
    int index = 0;
    double ms = 0.0;
    double side_ms = 0.0;
    std::vector<Pad> chart_pads;
    std::vector<Pad> sent_pads;
};

// The refusal for a press of `chords[place]` sent at `send_ms` on `pads`:
// when it leaves its own chord's window, lands inside the window of the
// nearest earlier chord on one of its pads, or inside the window of the
// nearest later chord on one of its pads that the player sends no press on
// that pad for. A later chord the player does press is let through, because
// the game gives a press inside two unhit same-pad windows to the earlier
// note (the matching rule on docs/clone-hero/03-timing.md). Unset when the
// press is safe.
std::optional<std::string> far_press_refusal(const std::vector<PadChord>& chords, size_t place,
                                             double send_ms, const std::vector<Pad>& pads);

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
// "squeezes" inside each activation (each with its split and "feasible"),
// "presses" in firing order and "refusals". A chord the player reads the
// game's end after carries "reads", each with the end planned there; a press
// timed from an end names the activation ("timed_from_end"), the read it is
// retimed from ("end_read_after") and the end its aim assumed in that read's
// terms ("aim_end_ms"). `replay` is replay_path over the same path's windows.
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
