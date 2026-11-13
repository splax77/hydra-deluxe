// hydra_replay schedule: the press schedule the auto-player sends.
//
// The player reads one file and works no Star Power rule out itself. This
// module writes that file from the engine's own facts: each activation's
// SP-end steps and their anchors (Activation, core/model.h), each squeeze's
// stored end, anchor and ratios, the steady spans (core/squeeze_rating.h),
// and which windows paid each chord (ReplayChord::paid_by, core/replay.h).
// The game's per-note hit window at 100% speed is core/note_window.h's; the
// player's timing constants live in replay_schedule.cpp, each with its
// source.
//
// Only hydra_replay and its tests read this. Nothing under src/ includes it,
// so no Hydra screen reads the player's constants. The plan that set this up
// is docs/superpowers/plans/2026-11-08-replay-press-schedule.md.

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
#include "core/note_window.h"  // the per-note window, ChordTimes
#include "core/replay.h"
#include "core/rules.h"
#include "store/record_store.h"  // ChartTimingMeta
#include "core/squeeze_rating.h"  // SteadySpans
#include "parse/song.h"
#include "search/pather.h"

namespace hydra::schedule {

// ---- a declined chord ------------------------------------------------------

// The gems of a declined chord (a skipped fill) the player sends, by their
// place in `notes` (`chord`'s Chord::note_list(true) order, as
// ReplayChord::notes): every gem but the one Chord::activation_note names.
// That is the one rule (D318 item 3).
struct KeptGems {
    std::vector<size_t> places;
};
KeptGems kept_gems(const Chord& chord, const std::vector<ReplayNote>& notes);

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

// The side of `scale` an anchor press for a squeeze of `kind` moves an end
// by: early for a squeeze-out (the anchor goes early), late for a squeeze-in.
double moving_side(SqueezeKind kind, const TransferScale& scale);

// The ratio the anchor's press moves this squeeze's end by: moving_side of
// the squeeze's stored ratio.
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

// ---- the two ends of a timed press ----------------------------------------
// Design points 3 and 4 of
// docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md. A press
// timed from an end has two ends in play: the read end, the one the player
// reads after the anchor chord, and the own end, the one the press must land
// before or after. Both move with the same anchor, each by its own step's
// scale (step_transfer_scale, core/squeeze_rating.h), so an error at the read
// end converts to the own end by the conversion. The player does the one sum
// with these numbers (retime_from_end in
// tools/ch_probe/experiments/play_chart.py); the schedule only runs it
// backwards, in read_at_own_ms.
struct EndLink {
    double read_stored_end_ms = 0.0;  // the read end as stored
    double aim_end_ms = 0.0;          // the read the press's aim assumed
    double own_stored_end_ms = 0.0;   // the own end as stored
    double own_end_ms = 0.0;          // the own end the press's aim assumed
    TransferScale conversion;         // end_conversion of the two step scales
};

// The own end's scale over the read end's, each side over the same side.
// When the two ends are one step the conversion is exactly 1 on both sides.
TransferScale end_conversion(const TransferScale& own, const TransferScale& read);

// The read at which the own end of `link` reaches `own_ms`: the late side of
// the conversion when `own_ms` is at or after the own stored end, else the
// early side.
double read_at_own_ms(const EndLink& link, double own_ms);

// The live fallbacks' thresholds, in own-end terms: the own end past which a
// squeeze-out's late press leaves its chord's side, and the own end before
// which a squeeze-in's press before the end lands ahead of it. The file
// writes each in read terms, through read_at_own_ms.
double sqout_fallback_after_ms(double chord_ms, double side_ms);
double sqin_fallback_before_ms(double chord_ms, double side_ms);

// The groups a chord's gems `places` (into its Chord::note_list(true) order)
// are pressed in, in firing order: `first` (an activation's activating note)
// alone first, then by `multipliers`, the lowest first. Empty `places` gives
// one empty group: a declined chord that sends nothing is still one press.
std::vector<std::vector<size_t>> press_groups(const std::vector<int>& multipliers,
                                              const std::vector<size_t>& places,
                                              std::optional<size_t> first);

// A squeeze-out chord's inside groups: the gems the player sends (`places`,
// every gem, or a declined chord's kept gems) but the late ones (`late`, the
// places sqout_late_notes in core/scoring.h names), lowest multiplier first.
// Empty when every sent gem is late. Unset, with a line in `refusals`, when a
// late gem pays less than an inside gem: the late gems go last, so no press
// gives that order.
std::optional<std::vector<std::vector<size_t>>> squeeze_out_inside_groups(
    int chord, const std::vector<int>& multipliers, const std::vector<size_t>& places,
    const std::vector<size_t>& late, std::vector<std::string>& refusals);

// A drum pad: a gem's colour and whether it is a cymbal.
using Pad = std::pair<NoteColor, bool>;

// One pad the player presses for a chord, and the song clock it is sent at.
struct PadSend {
    Pad pad;
    double send_ms = 0.0;
};

// One chord as the far-press check sees it: its pads in the chart, and each
// pad the player sends for it with its send time (a declined chord's kept
// gems only; none for a chord the schedule does not press).
struct PadChord {
    int index = 0;
    double ms = 0.0;
    double side_ms = 0.0;
    std::vector<Pad> chart_pads;
    std::vector<PadSend> sends;
};

// The refusal for a press of `chords[place]` sent at `send_ms` on `pads`,
// judged one frame after it is sent: when it leaves its own chord's window,
// or when it lands inside the window of an earlier chord on one of its pads
// that is still unhit then, because the player sends that chord no press on
// the pad or sends it no earlier than this one. The game gives such a press
// to the earlier note: TM-50 on docs/clone-hero/03-timing.md, followed as
// D322 item 1 rules. A later chord's window is no reason to refuse. Unset
// when the press is safe.
std::optional<std::string> far_press_refusal(const std::vector<PadChord>& chords, size_t place,
                                             double send_ms, const std::vector<Pad>& pads);

// ---- the file --------------------------------------------------------------

// One gem as the schedule and `hydra_replay score` both write it.
nlohmann::json replay_note_json(const ReplayNote& note);

// The build of the code that writes these files: the compile date and time of
// replay_schedule.cpp. CMake stamps no git hash today, so there is none to add.
// A build that changed only replay.cpp or replay_json.cpp keeps the older
// stamp: it tells builds apart by that file, not by every source.
// schedule_json and `hydra_replay dump` write it as "hydra_replay_build".
const char* replay_build();

// "song_length_ms" for a chart whose metadata is `meta`: Hydra's one song end,
// app::song_length_ms (D255), or null when it gives none.
nlohmann::json song_length_json(const store::ChartTimingMeta& meta, const Song& song);

// One activation's geometry: its chord, nominal and final ends, the point the
// meter reached the activation level (Activation::sp_ready_tick), every
// SP-end step with its anchor (Activation::end_anchor_tick), the chord the
// player reads the end after (read_end_after, the final end's anchor) and
// every squeeze with its own end, anchor, stored ratios and steady spans.
// A fact the record does not hold is written null and its refusal line is
// added to `refusals`; nothing is worked out again. `chords` names each
// chord in the file by its place (chord_times).
nlohmann::json activation_json(const Activation& act, const ChordTimes& chords,
                               const SongTiming& timing, std::vector<std::string>& refusals);

// The whole schedule for one path: "constants", "activations", "chords",
// "squeezes" inside each activation (each with its split and "feasible"),
// "presses" in firing order and "refusals". A chord the player reads the
// game's end after carries "reads", each with the step it sees, that step's
// scale and the end planned there; a press timed from an end names the
// activation ("timed_from_end"), the read it is retimed from
// ("end_read_after") and the EndLink fields ("aim_end_ms", "own_end_ms",
// "read_stored_end_ms", "own_stored_end_ms", "conversion"); its fallback
// thresholds are in read terms. Every chord row carries "refused": true when the
// schedule writes no press for it, and then a line in "refusals" says why.
// `replay` is replay_path over the same path's windows. `rules` are the
// rules the path was priced under: a squeeze-out's late press holds the gems
// they pay outside (sqout_late_notes).
nlohmann::json schedule_json(const Song& song, const std::vector<const Activation*>& acts,
                             const ReplayResult& replay, const core::Rules& rules);

// ---- the command's refusals ------------------------------------------------

// The line `hydra_replay schedule` refuses with when it was given no --path,
// or unset when it was.
std::optional<std::string> path_flag_refusal(const std::string& path_file);

// The line it refuses with when the dump `doc` (read from `path_file`) holds
// no path to plan, or unset when it holds one.
std::optional<std::string> no_path_refusal(const nlohmann::json& doc, const std::string& path_file);

// The schedule of the path `windows` names (read off a dump, as `score --path`
// reads it): the windows are pinned through search_target so the engine's
// own Activation records come back, then replayed. Throws std::runtime_error
// with a plain line when the engine cannot realize the windows. The rules are
// `settings.rules`, the ones the search and the replay price under.
nlohmann::json schedule_for_windows(const Song& song, const SearchSettings& settings,
                                    const std::vector<ReplayWindow>& windows);

}  // namespace hydra::schedule

#endif  // HYDRA_TOOLS_REPLAY_SCHEDULE_H
