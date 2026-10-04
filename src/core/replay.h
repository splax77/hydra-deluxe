// Replay — score an arbitrary Star Power path chord by chord, the way the
// engine scores it.
//
// The search never walks a chart note by note; it walks a graph whose edges
// carry pre-summed scores, so nothing in it can answer "what was this one
// chord worth on this path?". This module answers exactly that, by replaying
// the chart against a list of Star Power windows and applying the same three
// rules the graph applies:
//
//   1. One combo counter, never touched by Star Power. So base, combo,
//      accent and ghost are fixed per chord, and the only path-dependent
//      term is whether the chord's doubling (CategoryScores::sp) is paid.
//   2. The Star Power window is inclusive at both ends: the activation chord
//      earns its doubling, and so does the chord sitting on the deactivation
//      node. A chord landing within Rules::backend_leeway_ms after the deactivation
//      earns it too (core/backend_value.h, the same function the search calls).
//   3. A solo pays 100 per note on both tracks and is never doubled.
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
    // chord squeezed out (Activation::sqout_tick). That chord is hit after
    // Star Power has ended, so it and everything after it lose their
    // doubling, and the chord itself keeps only what its non-first hits are
    // worth (CategoryScores::sqout_reduction is the first hit's share).
    std::optional<int64_t> sqout_tick;

    // The same squeeze-out as an ms offset from D, for display, and as typed
    // by hand in `--acts`. replay_path reads only the tick; a window with an
    // offset and no tick must go through resolve_sqout_note first.
    std::optional<double> sqout_offset_ms;

    // The phrase chords this window squeezed in, when known: a stored path's
    // SqIn steps (windows_for_path). A typed window carries none. A phrase
    // is squeezed in only once (D34), so resolve_sqout_note and
    // ambiguous_window_warnings skip these when they name the engine's chord.
    // replay_path does not read it.
    std::vector<int64_t> sqin_ticks;
};

// The phrase chord a typed SqOut offset means.
struct SqOutNote {
    int64_t tick = 0;
    double offset_ms = 0.0;  // its real offset from D
};

// Resolve w.sqout_offset_ms to the phrase chord nearest D + offset, among
// the phrase chords strictly within kSqueezeWindowMs of D on either side.
// The engine only ever squeezes out one of those (core::sqout_chord): the
// first that the window did not bank before its activation and did not
// already squeeze in (w.sqin_ticks; D34). When the nearest one is any other
// chord this refuses (plan decision 20 of 2026-09-24). Throws
// std::runtime_error, with a message naming both chords, in that case; also
// when there is no candidate, or when w has no offset.
SqOutNote resolve_sqout_note(const Song& song, const ReplayWindow& w);

// The six score categories a Path stores, in the same order.
struct ReplayScore {
    int64_t base = 0;
    int64_t combo = 0;
    int64_t sp = 0;
    int64_t solo = 0;
    int64_t accent = 0;
    int64_t ghost = 0;

    int64_t total() const { return base + combo + sp + solo + accent + ghost; }
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
    // it. Notes of one chord differ when the chord straddles 10, 20 or 30.
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

    int combo_before = 0;
    // The combo once this chord is hit: combo_before plus the chord's notes.
    // What the game's combo counter shows after the chord.
    int combo_after = 0;
    // What category_scores applied to the chord's first note (base-sorted).
    int multiplier = 1;
    // What category_scores applied to the chord's last note: the multiplier
    // the game's disc shows once this chord is hit.
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
// std::invalid_argument for a window with a SqOut offset but no SqOut tick.
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

// The Star Power windows a stored path describes: one per activation, with
// its deactivation node read straight off the record (Activation::deact_tick,
// stamped by the search) and, when the activation squeezed out, the SqOut
// tick and offset read from Activation::sqout_row (the squeeze-out's one
// stored form). An activation with no SP-end steps, which only a hand-built
// one can be, is skipped, so a path that yields fewer windows than it has
// activations cannot be replayed faithfully.
// replay_stored_path below checks that before a score is trusted.
std::vector<ReplayWindow> windows_for_path(const Path& path);

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
// The chord it can be about is the one the engine would squeeze out there
// (core::sqout_chord): the first phrase chord strictly within
// kSqueezeWindowMs of the deactivation node that the window did not bank or
// already squeeze in. A window with such a chord is ambiguous: the score is
// right if the player did not squeeze, and high by that chord's first-hit
// share if they
// did, and nothing in the window list says which. So this reports the doubt
// and nothing else: it never changes a score and never invents an offset.
//
// It warns only when the window paid that chord (`result` must be the replay
// of these same windows): a chord past the leeway after D was never doubled,
// so squeezing it out changes nothing. A window that already carries a
// squeeze-out offset or chord is settled and is never reported.
std::vector<std::string> ambiguous_window_warnings(
    const Song& song, const ReplayResult& result,
    const std::vector<ReplayWindow>& windows);

// Not offered: a simulated SP meter and skip count. A straightforward
// simulation (one bar per phrase completed outside Star Power, capped at 4,
// two bars to activate, a passed fill with two bars banked is a skip) was
// measured against every corpus path and does not reproduce the engine: 37
// of 1486 activations came out with too few bars, and 224 with the wrong
// skip count, 201 of them too many. The meter misses bars a squeeze-out
// leaves behind, and the skip count over-counts fills the engine knew could
// not actually be summoned in time. Read both off the record instead
// (Activation::sp_meter / ::skips), which is what `hydra_replay dump` does.

}  // namespace hydra

#endif  // HYDRA_CORE_REPLAY_H
