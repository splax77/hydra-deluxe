#include "core/replay.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "core/backend_value.h"
#include "core/scoring.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"
#include "core/timing.h"
#include "core/model.h"  // kSqueezeWindowMs, the engine's squeeze horizon

namespace hydra {

namespace {

struct Window {
    int64_t act_tick = 0;
    int64_t deact_tick = 0;
    double deact_ms = 0.0;
    std::optional<int64_t> sqout_tick;
};

// Every phrase chord strictly within kSqueezeWindowMs of deactivation node D,
// in chart order: the list the graph puts on D's edge
// (core::squeeze_window_phrases). The engine squeezes out only the ones
// core::sqout_chords names. A typed offset is matched to the chord
// nearest it, which is the replay's own question.
std::vector<const SongTimestamp*> sqout_candidates(const Song& song,
                                                   int64_t deact_tick) {
    return core::squeeze_window_phrases(song, song.timing().timecode(deact_tick));
}

}  // namespace

ReplayResult replay_path(const Song& song, std::vector<ReplayWindow> windows,
                         const core::Rules& rules, const ReplayOptions& options) {
    const SongTiming& timing = song.timing();

    std::vector<Window> wins;
    wins.reserve(windows.size());
    for (const ReplayWindow& w : windows) {
        Window win;
        win.act_tick = w.act_tick;
        win.deact_tick = w.deact_tick;
        win.deact_ms = timing.timecode(w.deact_tick).ms();
        if (w.sqout_offset_ms && !w.sqout_tick)
            throw std::invalid_argument(
                "window " + std::to_string(w.act_tick) + ":" +
                std::to_string(w.deact_tick) +
                " has a SqOut offset but no SqOut chord; resolve it with "
                "resolve_sqout_note first");
        win.sqout_tick = w.sqout_tick;
        wins.push_back(win);
    }
    std::sort(wins.begin(), wins.end(), [](const Window& a, const Window& b) {
        return a.act_tick < b.act_tick;
    });

    ReplayResult out;
    out.chords.reserve(song.sequence.size());

    // Every chord's path-free points, the same table ScoreGraph::build sums.
    const ChordScoreTable table = chord_score_table(
        song, rules.sqout_rule,
        options.scores_only ? ChordScoreDetail::ChordsOnly : ChordScoreDetail::WithNotes);

    ReplayScore cum;
    int64_t solo_pending = 0;
    size_t next_solo = 0;  // the first solo section not yet passed

    const size_t n = song.sequence.size();

    // The windows that can still pay a chord, as indexes into `wins`.
    //
    // A window joins when the walk reaches its activation chord. `wins` is
    // sorted by act_tick and the chords come in tick order, so `next_win`
    // walks `wins` once and a window, once in, stays past its activation.
    //
    // A window leaves for good when the chord is past its deactivation node
    // and the window does not pay it. Every reason a window pays a chord
    // nothing there holds for every later chord too, so it can never pay a
    // later one:
    //   - sqout_position(...) == After means the chord's tick is past the
    //     squeezed-out chord's tick, and ticks only grow.
    //   - Past the deactivation node the offset only grows, since ms never
    //     falls as ticks grow (positive tempos), and
    //     core::counted_without_squeeze is true up to a threshold and false
    //     above it, so once false it stays false.
    //   - The chord is the squeezed-out chord and its sqout_points are 0 (a
    //     one-note chord under the first-note rule). Every later chord is
    //     past it, so sqout_position(...) == After from then on.
    // So the sum below visits exactly the windows the old every-window loop
    // paid, and sums the same integers (order does not change an int sum).
    std::vector<size_t> open;
    size_t next_win = 0;

    for (size_t i = 0; i < n; ++i) {
        const SongTimestamp& ts = song.sequence[i];
        const ChordScoreRow& sg = table.rows[i];

        ReplayChord row;
        row.index = static_cast<int>(i);
        row.tick = ts.timecode.ticks();
        row.ms = ts.timecode.ms();
        const int64_t* mbt = ts.timecode.measure_beats_ticks();
        row.measure = mbt[0];
        row.beat = mbt[1];
        row.measure_tick = mbt[2];
        row.measures_decimal = ts.timecode.measures_decimal();
        if (!options.scores_only) row.chord_code = ts.chord.code();
        row.is_fill = ts.has_activation();
        row.is_solo = ts.flag_solo;
        row.is_sp_phrase_end = ts.flag_sp;
        row.combo_before = sg.combo_before;
        row.multiplier = sg.multiplier;
        row.multiplier_after = sg.multiplier_after;

        if (!options.scores_only) {
            const std::vector<ChordNote> ordering = ts.chord.notes(true);
            const size_t priced = sg.note_end - sg.note_begin;
            row.notes.reserve(ordering.size());
            for (size_t k = 0; k < ordering.size(); ++k) {
                const CategoryScores* own =
                    k < priced ? &table.notes[sg.note_begin + k] : nullptr;
                ReplayNote note;
                note.color = ordering[k].colortype;
                note.cymbal = ordering[k].is_cymbal();
                note.sp_points = own ? own->sp : 0;
                note.multiplier = own ? own->multiplier : 1;
                note.dynamics_bonus = own ? own->dynamics_bonus : 0;
                note.dynamic = ordering[k].dynamictype;
                row.notes.push_back(note);
            }
        }

        while (next_win < wins.size() && row.tick >= wins[next_win].act_tick)
            open.push_back(next_win++);

        // How many activations pay this chord's doubling, asked of the shared
        // rule core::paid_by_sp (decision D2). Normally 0 or 1;
        // summed rather than flagged because the engine sums too (a chord in
        // one activation's leeway that is also the next activation's frontend
        // is paid by both).
        int64_t sp_points = 0;
        int sp_paid = 0;
        size_t kept = 0;
        for (size_t k = 0; k < open.size(); ++k) {
            const Window& w = wins[open[k]];
            // The row's offset from the SP end, as the graph measures it. A
            // chord on or before the deactivation node is inside the window
            // whatever its ms says; time rises with tick (parse refuses
            // timing that does not, D6), so its offset is never above 0.
            // Whether the chord is after the SP end is asked of
            // core::after_sp_end, the rule the graph uses.
            const bool past_deact = core::after_sp_end(row.tick, w.deact_tick);
            const double offset = offset_from_sp_end(row.ms, w.deact_ms);
            const core::SqOutPosition pos =
                core::sqout_position(row.tick, w.sqout_tick);
            const bool paid = core::paid_by_sp(offset, sg.sp, sg.sqout_sp, pos,
                                               rules.backend_leeway_ms);
            if (paid) {
                ++sp_paid;
                sp_points += core::backend_row_value(offset, sg.sp, sg.sqout_sp, pos,
                                                     rules.backend_leeway_ms);
            }
            // Erase-remove in place: keep the window unless it is past its
            // deactivation node and paid nothing here (see `open` above).
            if (paid || !past_deact) open[kept++] = open[k];
        }
        open.resize(kept);
        row.in_sp = sp_paid > 0;
        row.multiplier_shown = shown_multiplier(row.multiplier_after, row.in_sp);

        row.points.base = sg.base;
        row.points.combo = sg.combo;
        row.points.sp = sp_points;
        row.points.solo = sg.solo;
        row.points.accent = sg.accent;
        row.points.ghost = sg.ghost;

        row.combo_after = sg.combo_after;

        cum.add(row.points);
        row.cum = cum;

        // The game pays a solo's bonus on the section's last chord
        // (Song::solo_sections says where each one ends).
        while (next_solo < song.solo_sections.size() && song.solo_sections[next_solo].last < i)
            ++next_solo;
        if (ts.flag_solo) {
            if (next_solo >= song.solo_sections.size() || song.solo_sections[next_solo].first > i)
                throw std::logic_error("replay: the solo chord at tick " +
                                       std::to_string(row.tick) +
                                       " is in no solo section");
            solo_pending += row.points.solo;
            if (song.solo_sections[next_solo].last == i) solo_pending = 0;
        }
        row.cum_onscreen_total = cum.total() - solo_pending;

        out.chords.push_back(std::move(row));
    }

    out.final = cum;
    return out;
}


ReplayScore score_of(const Path& path) {
    ReplayScore s;
    s.base = path.score_base;
    s.combo = path.score_combo;
    s.sp = path.score_sp;
    s.solo = path.score_solo;
    s.accent = path.score_accents;
    s.ghost = path.score_ghosts;
    return s;
}

void assign_score(Path& path, const ReplayScore& s) {
    path.score_base = s.base;
    path.score_combo = s.combo;
    path.score_sp = s.sp;
    path.score_solo = s.solo;
    path.score_accents = s.accent;
    path.score_ghosts = s.ghost;
}

std::vector<int64_t> sqin_phrase_ticks(const Activation& act) {
    std::vector<int64_t> out;
    for (const SpEndStep& s : act.sp_end_steps)
        if (is_sqin_kind(s.kind)) out.push_back(s.tick);
    return out;
}

// The deact node comes off the record, so the chart is not consulted.
std::vector<ReplayWindow> windows_for_path(const Path& path) {
    std::vector<ReplayWindow> out;
    for (const Activation& act : path.walk_activations()) {
        const std::optional<int64_t> deact = act.deact_tick();
        if (!deact) continue;

        ReplayWindow w;
        w.act_tick = act.timecode.ticks();
        w.deact_tick = *deact;
        // The squeeze-out's one stored form: its row's tick and offset.
        if (const BackendSqueeze* row = act.sqout_row()) {
            w.sqout_tick = row->timecode.ticks();
            w.sqout_offset_ms = row->offset_ms;
        }
        w.sqin_ticks = sqin_phrase_ticks(act);
        w.from_record = true;
        out.push_back(w);
    }
    return out;
}

PathReplay replay_stored_path(const Song& song, const Path& path,
                              const core::Rules& rules,
                              const ReplayOptions& options) {
    PathReplay out;
    out.windows = windows_for_path(path);
    out.result = replay_path(song, out.windows, rules, options);
    out.stored = score_of(path);
    out.activations = path.walk_activations().size();
    return out;
}

SqOutNote resolve_sqout_note(const Song& song, const ReplayWindow& w) {
    const std::string where = "window " + std::to_string(w.act_tick) + ":" +
                              std::to_string(w.deact_tick);
    if (!w.sqout_offset_ms)
        throw std::runtime_error(where + " has no SqOut offset to resolve");
    const double d_ms = song.timing().timecode(w.deact_tick).ms();

    // How far a candidate's offset from the SP end sits from the typed one.
    const auto miss = [&](const SongTimestamp* ts) {
        return std::fabs(offset_from_sp_end(ts->timecode.ms(), d_ms) -
                         *w.sqout_offset_ms);
    };
    const std::vector<const SongTimestamp*> cands =
        sqout_candidates(song, w.deact_tick);
    const SongTimestamp* best = nullptr;
    for (const SongTimestamp* ts : cands)
        if (!best || miss(ts) < miss(best)) best = ts;

    char buf[512];
    if (!best) {
        std::snprintf(buf, sizeof(buf),
                      "%s has a SqOut offset of %.2f ms but no Star Power "
                      "phrase chord within %.0f ms of its SP end",
                      where.c_str(), *w.sqout_offset_ms, kSqueezeWindowMs);
        throw std::runtime_error(buf);
    }
    // D83: an offset exactly as close to two or more chords names none of
    // them, so it is refused rather than priced on a guess.
    std::vector<const SongTimestamp*> tied;
    for (const SongTimestamp* ts : cands)
        if (miss(ts) == miss(best)) tied.push_back(ts);
    // The one way the refusals below name a chord in a list.
    const auto chord_text = [&](const SongTimestamp* ts) {
        char one[96];
        std::snprintf(one, sizeof(one), "tick %lld (%.2f ms)", (long long)ts->timecode.ticks(),
                      offset_from_sp_end(ts->timecode.ms(), d_ms));
        return std::string(one);
    };
    if (tied.size() > 1) {
        std::string named;
        for (size_t i = 0; i < tied.size(); ++i) {
            if (i > 0) named += i + 1 == tied.size() ? " and " : ", ";
            named += chord_text(tied[i]);
        }
        std::snprintf(buf, sizeof(buf),
                      "%s: the SqOut offset %.2f ms is equally close to the phrase chords at ",
                      where.c_str(), *w.sqout_offset_ms);
        throw std::runtime_error(std::string(buf) + named +
                                 ". Type an offset nearer the one you mean. Not priced.");
    }
    const SqOutNote typed{best->timecode.ticks(),
                          offset_from_sp_end(best->timecode.ms(), d_ms)};

    // The engine squeezes out only the chords core::sqout_chords names for
    // this window. Anything else is a squeeze-out the search can never produce:
    // refuse, never price it.
    const Timecode deact_tc = song.timing().timecode(w.deact_tick);
    const bool typed_squeezed_in =
        std::find(w.sqin_ticks.begin(), w.sqin_ticks.end(), typed.tick) != w.sqin_ticks.end();
    if (!core::activation_can_squeeze(w.act_tick, typed.tick)) {
        std::snprintf(buf, sizeof(buf),
                      "%s: the SqOut offset %.2f ms lands on the phrase chord at "
                      "tick %lld, at or before the activation at tick %lld. That "
                      "phrase was banked before Star Power started, so this "
                      "window cannot squeeze it out. Not priced.",
                      where.c_str(), *w.sqout_offset_ms, (long long)typed.tick,
                      (long long)w.act_tick);
        throw std::runtime_error(buf);
    }
    if (typed_squeezed_in) {
        // D34: a phrase is squeezed in only once, so never out after that.
        std::snprintf(buf, sizeof(buf),
                      "%s: the SqOut offset %.2f ms lands on the phrase chord at "
                      "tick %lld, which this window already squeezed in. A "
                      "phrase is squeezed in only once. Not priced.",
                      where.c_str(), *w.sqout_offset_ms, (long long)typed.tick);
        throw std::runtime_error(buf);
    }
    // D36: a window squeezes out only its newest phrase at or before the SP
    // end, or the first phrase after it. A typed window has no history, so
    // either is accepted (core::sqout_chords).
    const std::vector<const SongTimestamp*> offered =
        core::sqout_chords(song, deact_tc, w.act_tick, w.sqin_ticks, false);
    if (std::find(offered.begin(), offered.end(), best) == offered.end()) {
        std::string can;
        for (const SongTimestamp* c : offered) {
            if (!can.empty()) can += " or ";
            can += chord_text(c);
        }
        std::snprintf(
            buf, sizeof(buf),
            "%s: the SqOut offset %.2f ms lands on the phrase chord at tick "
            "%lld (%.2f ms from the SP end), which the engine never squeezes "
            "out. A window squeezes out only its newest phrase chord at or "
            "before the SP end or the first one after it%s%s. Not priced.",
            where.c_str(), *w.sqout_offset_ms, (long long)typed.tick, typed.offset_ms,
            can.empty() ? "; here neither is left" : ": here ", can.c_str());
        throw std::runtime_error(buf);
    }
    return typed;
}

std::vector<std::string> ambiguous_window_warnings(
    const Song& song, const ReplayResult& result,
    const std::vector<ReplayWindow>& windows, const core::Rules& rules) {
    const SongTiming& timing = song.timing();
    std::vector<std::string> out;

    for (const ReplayWindow& w : windows) {
        // A squeeze-out offset or chord settles the question.
        if (w.sqout_offset_ms || w.sqout_tick) continue;

        // The chords the engine could squeeze out at this D, for this window
        // (core::sqout_chords, D36): for a stored window that ended plainly,
        // only a late one; for a typed one, up to two.
        for (const SongTimestamp* engine :
             core::sqout_chords(song, timing.timecode(w.deact_tick), w.act_tick, w.sqin_ticks,
                                w.from_record)) {
        const int64_t tick = engine->timecode.ticks();

        // Only a chord the window paid can make the score too high: one at or
        // before D, or inside the leeway after it.
        const ReplayChord* chord = nullptr;
        for (const ReplayChord& c : result.chords)
            if (c.tick == tick) chord = &c;
        if (!chord || !chord->in_sp) continue;

        const double deact_ms = timing.timecode(w.deact_tick).ms();
        std::string where = "on the Star Power phrase note at tick " +
                            std::to_string(tick);
        char gap[32];
        if (core::after_sp_end(tick, w.deact_tick)) {
            std::snprintf(gap, sizeof(gap), "%.2f",
                          std::fabs(offset_from_sp_end(chord->ms, deact_ms)));
            where = "just before the Star Power phrase note at tick " +
                    std::to_string(tick) + " (" + gap + " ms later)";
        } else if (tick != w.deact_tick) {
            std::snprintf(gap, sizeof(gap), "%.2f",
                          std::fabs(offset_from_sp_end(chord->ms, deact_ms)));
            where = "just after the Star Power phrase note at tick " +
                    std::to_string(tick) + " (" + gap + " ms earlier)";
        }

        // What the squeeze-out would take off the total. It is more than the
        // chord's own sqout_reduction whenever a chord follows it inside the
        // window, because Star Power ends before the squeezed-out chord and
        // every later chord loses its doubling too. So ask replay_path, the
        // owner of that rule: replay the same windows with this one squeezed
        // out on this chord, under the same rules, and take the difference.
        std::vector<ReplayWindow> squeezed = windows;
        squeezed[static_cast<size_t>(&w - windows.data())].sqout_tick = tick;
        ReplayOptions scores_only;
        scores_only.scores_only = true;
        const int64_t cost = result.final.total() -
                             replay_path(song, squeezed, rules, scores_only).final.total();
        out.push_back("window " + std::to_string(w.act_tick) + ":" +
                      std::to_string(w.deact_tick) + " ends " + where +
                      " with no squeeze-out offset; if the player squeezed it "
                      "out, this score is " + std::to_string(cost) +
                      " points high (the same windows replayed with that "
                      "squeeze-out, under sqout_rule)");
        }
    }
    return out;
}

}  // namespace hydra
