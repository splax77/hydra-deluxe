// Replay — score an arbitrary Star Power path chord by chord, the way the
// engine scores it.
//
// The search never walks a chart note by note; it walks a graph whose edges
// carry pre-summed scores, so nothing in it can answer "what was this one
// chord worth on this path?". This module answers exactly that, by replaying
// the chart against a list of Star Power windows. It owns no scoring rule:
//
//   - A chord's path-free points (base, combo, accent, ghost, solo, its SP
//     value and squeezed-out SP value, its multipliers and combo) are rows
//     of chord_score_table (core/scoring.h), the table ScoreGraph::build
//     sums its edges from.
//   - Whether a window pays a chord's doubling, and how much, is
//     core::paid_by_sp and core::backend_row_value (core/backend_value.h),
//     the functions the search calls.
//   - Which side of the SP end and of a squeeze-out a chord sits on is
//     core::after_sp_end and core::sqout_position, in the same header.
//
// The one question answered here is which windows can still pay a chord,
// and hydra_replay's selfcheck is the test that it agrees with the graph.
//
// Display and tooling only, like core/squeeze_rating.h: nothing in the
// search, the store, or a stored record reads any of this. It exists so the
// hydra_replay CLI and its self-check can price a user's own path and prove
// the pricing against the engine's own numbers.

#ifndef HYDRA_CORE_REPLAY_H
#define HYDRA_CORE_REPLAY_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/model.h"
#include "parse/song.h"

namespace hydra {

// One activation's Star Power window, in ticks. `deact_tick` is the
// deactivation node D — for a stored activation, the tick the search
// stamped onto it; it need not be a chord tick.
struct ReplayWindow {
    int64_t act_tick = 0;
    int64_t deact_tick = 0;

    // Set when the activation ends on a squeeze-out: the tick of the phrase
    // chord squeezed out (Activation::sqout_tick). What that chord and the
    // ones after it are paid is core::backend_row_value's answer, and what
    // the chord loses is CategoryScores::sqout_reduction under the rules'
    // sqout_rule.
    std::optional<int64_t> sqout_tick;

    // The same squeeze-out as an ms offset from D, for display, and as typed
    // by hand in `--acts`. replay_path reads only the tick; see
    // sqout_needs_resolving and resolve_window_sqout below.
    std::optional<double> sqout_offset_ms;

    // The phrase chords this window squeezed in, when known: a stored path's
    // SqIn steps (windows_for_path). A typed window carries none. A phrase
    // is squeezed in only once (D34), so resolve_sqout_note and
    // ambiguous_window_warnings skip these when they name the engine's chord.
    // replay_path does not read it.
    std::vector<int64_t> sqin_ticks;

    // The window comes off a stored record (windows_for_path, or a dump of
    // one), so its record says how it ended: with the squeeze-out above, or
    // plainly at D. A plainly ended window's end was D, so the only chord
    // the engine could have squeezed out there is a late one (D36). A typed
    // window has no history and may mean either side (core::sqout_chords).
    bool from_record = false;
};

// The phrase chord a typed SqOut offset means.
struct SqOutNote {
    int64_t tick = 0;
    double offset_ms = 0.0;  // its real offset from D
};

// Resolve w.sqout_offset_ms to the phrase chord nearest D + offset, among
// the phrase chords strictly within kSqueezeWindowMs of D on either side.
// The engine only ever squeezes out its newest phrase at or before D or the
// first one after D that it did not already squeeze in (core::sqout_chords;
// w.sqin_ticks, D34; D36); a typed window may mean either. When the nearest
// one is any other chord this refuses (plan decision 20 of 2026-09-24). Throws
// std::runtime_error, with a message naming both chords, in that case; also
// when no single chord is nearest (D83), when there is no candidate, or when
// w has no offset.
SqOutNote resolve_sqout_note(const Song& song, const ReplayWindow& w);

// Whether `w` names its squeeze-out by an offset alone, so replay_path
// cannot read it until resolve_window_sqout has run. The one statement of
// that rule: replay_path's refusal and resolve_window_sqout both ask it.
bool sqout_needs_resolving(const ReplayWindow& w);

// Resolve `w` in place when sqout_needs_resolving says so: set its
// sqout_tick to resolve_sqout_note's chord and return that note, so a
// caller can say which chord it used. Returns nothing, and leaves `w` as it
// is, for any other window. Throws what resolve_sqout_note throws. Every
// reader of a typed or dumped window list goes through this (hydra_replay
// score, and pinned_windows for target).
std::optional<SqOutNote> resolve_window_sqout(const Song& song, ReplayWindow& w);

// The six score categories a Path stores, in the same order.
struct ReplayScore {
    int64_t base = 0;
    int64_t combo = 0;
    int64_t sp = 0;
    int64_t solo = 0;
    int64_t accent = 0;
    int64_t ghost = 0;

    int64_t total() const { return score_total(base, combo, sp, solo, accent, ghost); }
    void add(const ReplayScore& o) {
        base += o.base;
        combo += o.combo;
        sp += o.sp;
        solo += o.solo;
        accent += o.accent;
        ghost += o.ghost;
    }
    bool operator==(const ReplayScore& o) const {
        return base == o.base && combo == o.combo && sp == o.sp &&
               solo == o.solo && accent == o.accent && ghost == o.ghost;
    }
};

// The six score categories, in the order hydra_replay's JSON
// (tools/replay_json.h) and its check output print them. One list, so the
// names and the fields cannot drift.
struct ReplayScoreField {
    const char* name;
    int64_t ReplayScore::*member;
};
inline constexpr ReplayScoreField kReplayScoreFields[] = {
    {"base", &ReplayScore::base},     {"combo", &ReplayScore::combo},
    {"sp", &ReplayScore::sp},         {"solo", &ReplayScore::solo},
    {"accent", &ReplayScore::accent}, {"ghost", &ReplayScore::ghost},
};

// One note of one chord, in the base-sorted order category_scores reads
// (Chord::notes(true)). `sp_points` is what this note contributes to the
// chord's doubling; it is reported whether or not the chord is under Star
// Power, so a caller can see what a window would be worth.
struct ReplayNote {
    NoteColor color = NoteColor::Kick;
    bool cymbal = false;
    int sp_points = 0;
    // The combo multiplier this note was paid at, as category_scores applied
    // it. Notes of one chord can differ when the chord straddles a step of
    // to_multiplier (core/timing.h).
    int multiplier = 1;
    // What this note's ghost or accent earned, multiplier included
    // (CategoryScores::dynamics_bonus); 0 for a plain note. A mis-hit dynamic
    // note pays sp_points minus this.
    int dynamics_bonus = 0;
    // Ghost, accent or neither, copied from ChordNote::dynamictype.
    NoteDynamicType dynamic = NoteDynamicType::Normal;
};

// One scored chord.
struct ReplayChord {
    int index = 0;
    int64_t tick = 0;
    double ms = 0.0;
    int64_t measure = 0;
    int64_t beat = 0;
    int64_t measure_tick = 0;
    double measures_decimal = 0.0;
    std::string chord_code;
    std::vector<ReplayNote> notes;

    bool is_fill = false;           // the chord carries an activation fill
    bool is_solo = false;
    bool is_sp_phrase_end = false;  // the chord ends an SP phrase

    // The chord's row of chord_score_table: the combo before and after it
    // (what the game's combo counter shows once it is hit) and the
    // multipliers CategoryScores::multiplier and ::multiplier_after name.
    int combo_before = 0;
    int combo_after = 0;
    int multiplier = 1;
    int multiplier_after = 1;
    // Star Power paid this chord something: at least one window's
    // core::paid_by_sp is true for it (decision D2). A squeezed-out chord
    // whose SP points are 0 reads false; a partly paid one reads true.
    bool in_sp = false;
    // What the game's disc shows once this chord is hit: multiplier_after,
    // doubled when in_sp (shown_multiplier), so only when SP paid something.
    int multiplier_shown = 1;

    ReplayScore points;
    ReplayScore cum;

    // The running total as the game's counter shows it: a solo's bonus is
    // withheld until the last chord of that solo run, then paid in full.
    int64_t cum_onscreen_total = 0;
};

struct ReplayResult {
    std::vector<ReplayChord> chords;
    ReplayScore final;
};

// How much of each row replay_path fills in.
struct ReplayOptions {
    // Leave each row's chord_code and notes empty, and skip the work that
    // builds them. Every score field (points, cum, cum_onscreen_total,
    // multipliers, combo, in_sp) and ReplayResult::final are the same as a
    // full replay's. Only the Preview sets it: its score box reads ms,
    // cum_onscreen_total, multiplier_shown, multiplier_after and combo_after
    // off each row, and
    // PathReplay::faithful() reads final (app/preview_view.cpp build_score).
    // Set it field by field (`ReplayOptions o; o.scores_only = true;`): the
    // project is C++17, which has no designated initializers.
    bool scores_only = false;
};

// Score `song` under `windows` (any order; they are sorted here). An empty
// list scores the chart with no Star Power anywhere. Throws
// std::invalid_argument for a window sqout_needs_resolving names.
//
// One pass over the chords. Only the windows that can still pay the current
// chord are checked (replay.cpp explains why a window that leaves can never
// pay again), so many activations cost little more than a few.
ReplayResult replay_path(const Song& song, std::vector<ReplayWindow> windows,
                         const core::Rules& rules = core::default_rules(),
                         const ReplayOptions& options = {});

// The six score categories off a stored Path, in ReplayScore's shape, so a
// caller can compare it against a ReplayResult::final with operator==
// instead of listing all six fields at each comparison site.
ReplayScore score_of(const Path& path);

// Writes a ReplayScore's six categories into a Path's score fields. It is
// score_of's reverse: the two spell the Path-to-ReplayScore field pairing,
// and nothing else does.
void assign_score(Path& path, const ReplayScore& score);

// The Star Power windows a stored path describes: one per activation, with
// its deactivation node read straight off the record (Activation::deact_tick,
// stamped by the search) and, when the activation squeezed out, the SqOut
// tick and offset read from Activation::sqout_row (the squeeze-out's one
// stored form). An activation with no SP-end steps, which only a hand-built
// one can be, is skipped, so a path that yields fewer windows than it has
// activations cannot be replayed faithfully.
// replay_stored_path below checks that before a score is trusted.
std::vector<ReplayWindow> windows_for_path(const Path& path);

// The phrases a stored activation squeezed in: its SqIn steps. The one
// statement of that rule; windows_for_path and hydra_replay's dump
// (paths_json) both ask it, so a window read back from a dump knows them too.
std::vector<int64_t> sqin_phrase_ticks(const Activation& act);

// A stored path replayed, with the two checks that say whether the replay
// stands for it. The one place those checks live: hydra_replay's selfcheck,
// the Preview's score and the corpus tests all read them from here.
struct PathReplay {
    std::vector<ReplayWindow> windows;  // windows_for_path
    ReplayResult result;
    ReplayScore stored;                 // score_of(path)
    size_t activations = 0;             // path.walk_activations().size()

    // Every activation became a window.
    bool all_windows() const { return windows.size() == activations; }
    // The replay's six totals equal the stored ones.
    bool totals_match() const { return result.final == stored; }
    // Both: the replay provably stands for the path.
    bool faithful() const { return all_windows() && totals_match(); }
};

// Replay `path` through its own windows. Replays even when some activation
// yields no window, so a caller can report what did not match.
PathReplay replay_stored_path(const Song& song, const Path& path,
                              const core::Rules& rules = core::default_rules(),
                              const ReplayOptions& options = {});

// The windows a hand-typed window list may have priced too high, one
// plain-English line each.
//
// A squeeze-out lives on the note that ends a Star Power phrase: the player
// delays that note until after Star Power has run out, so it is not doubled.
// The chords it can be about are the ones the engine could squeeze out there
// (core::sqout_chords, D36): for a stored window that ended plainly, the
// first phrase chord after the deactivation node that it did not squeeze
// in; for a typed window, that one or its newest phrase chord at or before
// the node. A window with such a chord is ambiguous: the score is right if
// the player did not squeeze, and high by what the squeeze-out costs if
// they did, and nothing in the window list says which. So this reports the doubt
// and nothing else: it never changes a score and never invents an offset.
// The line quotes that cost in points: `result`'s total minus the total of
// the same windows replayed with this one squeezed out on that chord, under
// `rules`. That is the chord's own sqout_reduction under `rules.sqout_rule`
// plus the full doubling of every later chord the window paid, since Star
// Power ends before the squeezed-out chord. `rules` must be the rules
// `result` was replayed under.
//
// It warns only when the window paid that chord (`result` must be the replay
// of these same windows): a chord past the leeway after D was never doubled,
// so squeezing it out changes nothing. A window that already carries a
// squeeze-out offset or chord is settled and is never reported.
std::vector<std::string> ambiguous_window_warnings(
    const Song& song, const ReplayResult& result,
    const std::vector<ReplayWindow>& windows, const core::Rules& rules);

// Not offered: a simulated SP meter and skip count. A straightforward
// simulation (one bar per phrase completed outside Star Power, capped at 4,
// kSpActivationBars to activate, a passed fill with that many banked is a
// skip) was
// measured against every corpus path and does not reproduce the engine: 37
// of 1486 activations came out with too few bars, and 224 with the wrong
// skip count, 201 of them too many. The meter misses bars a squeeze-out
// leaves behind, and the skip count over-counts fills the engine knew could
// not actually be summoned in time. Read both off the record instead
// (Activation::sp_meter / ::skips), which is what `hydra_replay dump` does.

}  // namespace hydra

#endif  // HYDRA_CORE_REPLAY_H
