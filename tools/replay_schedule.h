// hydra_replay schedule: the press schedule the auto-player sends.
//
// The player reads one file and works no Star Power rule out itself. This
// module writes that file from the engine's own facts: each activation's
// SP-end steps and their anchors (Activation, core/model.h), each squeeze's
// stored end, anchor and ratios, the steady spans (core/squeeze_rating.h),
// and which windows paid each chord (ReplayChord::paid_by, core/replay.h).
// The game's per-note hit window is core/note_window.h's. Every time in the
// file is played time at the song speed the file records as "speed_percent"
// (TM-43 and TM-56 on docs/clone-hero/03-timing.md). Every press time in the file is the time the press should be judged, with
// the band it may be judged in; when to send it so the game judges it then
// is the player's, from the frame step it measures. The schedule keeps no
// millisecond constant of its own.
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

// The end the player reads after an anchor chord: the end of the SP-end step
// on that chord (design point 5 of
// `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`)).
struct ReadEnd {
    int64_t tick = 0;     // that step's end tick
    TransferScale ratio;  // step_transfer_scale of that step
    SteadySpans steady;   // steady_spans(anchor, read end)
};

// One squeeze that needs timing, as its two presses see it: the anchor chord
// (Activation::squeeze_anchor_tick, the overfill chord when the cap clamped,
// else the activation) and the squeeze chord, each with its window side, and
// the end the squeeze is measured against as the engine stored it.
struct SqueezeGeometry {
    // The way the anchor is pressed, squeeze_presses(SPSqueeze).anchor
    // (core/note_window.h). Every way the presses go (early or late, before
    // or after the end) is read off it, never off the squeeze's kind.
    Press anchor_press = Press::Early;
    int anchor_chord = 0;
    double anchor_ms = 0.0;
    double anchor_side_ms = 0.0;
    int squeeze_chord = 0;
    double squeeze_ms = 0.0;
    double squeeze_side_ms = 0.0;
    double end_ms = 0.0;
    TransferScale ratio;  // as stored; the side the anchor moves is read off it
    SteadySpans steady;   // steady_spans(anchor, end)
    // The read end, when the activation has a step on the anchor's chord.
    // Unset, the read after the anchor is not planned and a refusal says so.
    std::optional<ReadEnd> read_end;
};

// The side of `scale` an anchor pressed `anchor_press` moves an end by:
// transfer_side (core/note_window.h), the same side the Dynamic Hit Window
// row's tag reads.
double moving_side(Press anchor_press, const TransferScale& scale);

// The ratio the anchor's press moves this squeeze's end by: moving_side of
// the squeeze's stored ratio.
double moving_ratio(const SqueezeGeometry& g);

// The lead the equal-room split gives the anchor of `g` alone, in ms (always
// how far from the chord's time; the direction is the anchor's press).
double equal_room_lead_ms(const SqueezeGeometry& g);

// ---- judge times and bands ---------------------------------------------------

// The earliest and latest song time a press of the chord at `chord_ms` with
// side `side_ms` may be judged at and still hit it (TM-27: equal is inside).
// The window is the same on both sides of the chord (chord_sides_ms).
double first_judge_ms(double chord_ms, double side_ms);
double last_judge_ms(double chord_ms, double side_ms);

// Where a press may be judged, in song ms. `to_ms` is always inside. A late
// gem must be judged after its end, not on it (SP-29 on
// docs/clone-hero/05-star-power.md), so its `from_ms` is strict.
struct Band {
    double from_ms = 0.0;
    double to_ms = 0.0;
    bool from_strict = false;
};

// The middle of `band`: where a banded press aims, the same room from each
// edge (the user's equal split).
double band_middle_ms(const Band& band);

// A press aimed at a point: its band is its chord's whole window.
Band window_band(double chord_ms, double side_ms);

// A press that must be judged by the Star Power end `end_ms` to be paid
// inside: from the window's front edge to that end. A hit in a frame whose
// clock equals the end is paid inside (SP-29), so the end is in the band. An
// end past the window's back edge gives the whole window.
Band end_band(double chord_ms, double side_ms, double end_ms);

// A squeeze-out's late gem: after its own end `end_ms`, up to its window's
// back edge.
Band late_gem_band(double chord_ms, double side_ms, double end_ms);

// A band as the file writes it: "from_ms", "to_ms", "from_strict".
nlohmann::json band_json(const Band& band);

// One squeeze's two presses at an anchor lead of `lead_ms`. Whether the
// squeeze fits is not the split's question: the file's "feasible" is Hydra's
// safe sum (squeeze_window_ends in core/note_window.h).
struct SqueezeSplit {
    double anchor_aim_ms = 0.0;    // judge target off the anchor chord's time, negative early
    double squeeze_aim_ms = 0.0;   // judge target off the squeeze chord's time: squeeze_band's middle
    Band squeeze_band;             // late_gem_band for a squeeze-out, end_band for a squeeze-in
    double anchor_room_ms = 0.0;   // press_room_ms of the anchor's aim
    double squeeze_room_ms = 0.0;  // press_room_ms at the end the squeeze press must clear
    double planned_end_ms = 0.0;   // the end once the anchor is judged at its aim
    double aim_end_ms = 0.0;       // the end the squeeze press's band assumed
};
SqueezeSplit split_at_lead(const SqueezeGeometry& g, double lead_ms);
SqueezeSplit split_squeeze(const SqueezeGeometry& g);  // at equal_room_lead_ms

// The one lead an anchor takes for every timed squeeze it moves: the largest
// any of them needs, so each other one keeps more room than the split gives
// it. Unset, with a plain-words line in `refusals`, when the squeezes need it
// pressed both early and late, when the split would press it the other way
// than its squeeze needs, or when the lead moves the anchor, the own end or
// a squeeze's read end across a steady span.
std::optional<double> anchor_lead_ms(const std::vector<SqueezeGeometry>& on_one_anchor,
                                     std::vector<std::string>& refusals);

// A press's room: how far inside the chord's side `side_ms` a judge time
// `aim_ms` off the chord's time stays.
double press_room_ms(double aim_ms, double side_ms);

// ---- the two ends of a timed press ----------------------------------------
// Design points 3 and 4 of
// `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`).
// A press timed from an end has two ends in play: the read end, the one the player
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
    // The reads over which the conversion holds: the read end as stored, less
    // its steady span before and plus its span after (steady_spans; design
    // point 5). A side with no change on it is unset.
    std::optional<double> read_low_ms;
    std::optional<double> read_high_ms;
};

// The own end's scale over the read end's, each side over the same side.
// When the two ends are one step the conversion is exactly 1 on both sides.
TransferScale end_conversion(const TransferScale& own, const TransferScale& read);

// The read at which the own end of `link` reaches `own_ms`: the late side of
// the conversion when `own_ms` is at or after the own stored end, else the
// early side.
double read_at_own_ms(const EndLink& link, double own_ms);

// The live fallbacks' thresholds, in own-end terms: the own end at or past
// which a squeeze-out's late gem has no band left in its window, and the own
// end before which a squeeze-in's band has none. Each is an edge of the
// chord's window and nothing else. The file writes each in read terms,
// through read_at_own_ms.
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

// One pad the player presses for a chord, and the range of song times it may
// be judged in (both the aim, for a press aimed at a point).
struct PadSend {
    Pad pad;
    double from_ms = 0.0;
    double to_ms = 0.0;
};

// One chord as the far-press check sees it: its pads in the chart, and each
// pad the player sends for it with its judge range (a declined chord's kept
// gems only; none for a chord the schedule does not press).
struct PadChord {
    int index = 0;
    double ms = 0.0;
    double side_ms = 0.0;
    std::vector<Pad> chart_pads;
    std::vector<PadSend> sends;
};

// The refusal for a press of `chords[place]` on `pads` that may be judged
// anywhere from `from_ms` to `to_ms`: when any of that range leaves its own
// chord's window, or when any of it falls inside the window of an earlier
// chord on one of its pads that may still be unhit then, because the player
// sends that chord no press on the pad or one that may be judged no earlier
// than this one's `from_ms`. The game gives such a press to the earlier note:
// TM-50 on docs/clone-hero/03-timing.md, followed as D322 item 1 rules. A
// later chord's window is no reason to refuse. Unset when the press is safe.
std::optional<std::string> far_press_refusal(const std::vector<PadChord>& chords, size_t place, double from_ms,
                                             double to_ms, const std::vector<Pad>& pads);

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
// "read_stored_end_ms", "own_stored_end_ms", "conversion", and
// "read_range_ms" as [read_low_ms, read_high_ms], null on an open side); its fallback
// thresholds are in read terms. Every press row's "aim_ms" is its judge target
// off its chord's time, "band" (band_json) where it may be judged,
// "window_from_ms" and "window_to_ms" its chord's window edges, "group" its
// place among its chord's groups and "follows" the group it goes down the
// frame after, or null. Each squeeze carries "safe_ms", Hydra's safe sum, and
// "feasible" from it. Every chord row carries "refused": true when the
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
// `settings.rules`, the ones the search and the replay price under. The file
// adds "speed_percent", the speed `song` is timed at (Song::speed_percent),
// and "safe_paths", `settings.safe_paths`.
nlohmann::json schedule_for_windows(const Song& song, const SearchSettings& settings,
                                    const std::vector<ReplayWindow>& windows);

}  // namespace hydra::schedule

#endif  // HYDRA_TOOLS_REPLAY_SCHEDULE_H
