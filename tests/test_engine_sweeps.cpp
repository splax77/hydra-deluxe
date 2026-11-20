// Invariant tests for search/ (ScoreGraph + engine + pather).
//
// The engine's best score must be stable across the config knobs that are not
// supposed to change it (search depth, the ms filter), every reported path
// must be internally consistent, and the all-0 pass must honor its contract.
// Most cases here sweep the whole chart corpus; the rest pin run_search's
// options, settings_key and graph_build_cap. The SP end cases live in
// test_sp_end.cpp, the passed-over fills in test_fills_and_skips.cpp and the
// tied variants in test_tied_variants.cpp.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"
#include "song_digest.h"  // record_hash
#include "store/record_store.h"

using namespace hydra;

namespace {

// What a sweep over the corpus found, one chart at a time.
struct SweepTally {
    int mismatches = 0;
    int rejected = 0;  // charts the engine threw ChartFileError on
};

// Runs one chart's `check`, which returns "" when the chart passes, or what is
// wrong. A ChartFileError is counted as a reject, so the sweep cannot quietly
// shrink; the first eight bad charts fail by name.
template <class Check>
void sweep_chart(SweepTally& tally, const std::string& path, Check check) {
    std::string d;
    try {
        d = check();
    } catch (const ChartFileError&) {
        ++tally.rejected;
        return;
    }
    if (!d.empty() && ++tally.mismatches <= 8)
        CHECK_MESSAGE(false, path << " " << d);
}

// Every sweep that uses sweep_chart ends with no mismatches and no rejects.
void check_sweep_tally(const SweepTally& tally) {
    CHECK(tally.mismatches == 0);
    CHECK(tally.rejected == 0);
}

// The number of charts the corpus scan lists: every sweep below visits each
// one, so a sweep that checked fewer lost a chart on the way.
int corpus_chart_count() { return static_cast<int>(corpus::chart_paths().size()); }

// Does the corpus chart at `path` load with notes? Every chart the corpus scan
// lists has Expert drum notes, so one that loads empty fails here by name
// rather than being passed over.
bool loads_notes(const Song& song, const std::string& path) {
    CHECK_MESSAGE(!song.is_empty(), path << " loads with no notes");
    return !song.is_empty();
}

// A depth far past the app's default (kDefaultDepthValue), in scores below
// the best, so a sweep's record keeps many alternate paths for the checks
// below to walk.
constexpr int kSweepDepthScores = 200;

// The Path limit the sweeps search under, in ms: one player-pickable limit
// that every sweep shares, so the limit-walk case reuses the invariants
// case's cached records.
constexpr double kSweepLimitMs = 20.0;

// The settings every sweep searches with: Clone Hero's SP cap, a depth in
// scores below the best and an optional Path limit.
SearchSettings sweep_settings(int depth_scores, std::optional<double> limit_ms) {
    SearchSettings cfg;
    cfg.sp_cap = kCloneHeroSpCap;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = depth_scores;
    cfg.ms_filter = limit_ms;
    return cfg;
}

// Adds `why` to a list of what went wrong, so a chart reports every reason.
void add_reason(std::string& d, const std::string& why) {
    if (!d.empty()) d += "; ";
    d += why;
}

}  // namespace

TEST_CASE("search invariants hold across the corpus and config knobs") {
    int charts = 0;
    SweepTally tally;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        ++charts;
        if (!loads_notes(song, path)) continue;

        sweep_chart(tally, path, [&]() -> std::string {
            std::string d;
            // Depth asks for more alternate paths and must not move the
            // optimum; the ms filter is a constraint, so a path it keeps
            // can tie the unconstrained best but never score above it.
            const HydraRecord& shallow = corpus::analyzed(path, sweep_settings(0, std::nullopt));
            const HydraRecord& deep =
                corpus::analyzed(path, sweep_settings(kSweepDepthScores, std::nullopt));
            const HydraRecord& filtered =
                corpus::analyzed(path, sweep_settings(kSweepDepthScores, kSweepLimitMs));

            if (shallow.paths.empty()) add_reason(d, "no paths");
            if (deep.paths.empty()) add_reason(d, "no paths at the sweep depth");
            if (!d.empty()) return d;

            const int64_t best = shallow.paths.front().totalscore();
            if (deep.paths.front().totalscore() != best)
                add_reason(d, "depth changed the best score");
            if (!filtered.paths.empty() && filtered.paths.front().totalscore() > best)
                add_reason(d, "ms filter scored above the unconstrained best");
            if (deep.paths.size() < shallow.paths.size())
                add_reason(d, "deeper search returned fewer paths");

            // Paths come out best-first, and every path in a record
            // reports the same chart notecount.
            int64_t prev = INT64_MAX;
            const int notecount = deep.paths.front().notecount;
            for (const Path& p : deep.paths) {
                std::string bad;
                if (p.totalscore() > prev) add_reason(bad, "paths not sorted by score");
                prev = p.totalscore();
                if (p.notecount != notecount) add_reason(bad, "notecount varies between paths");
                if (p.tied_pathcount() < 1) add_reason(bad, "tied_pathcount below 1");
                if (!bad.empty()) {
                    add_reason(d, p.pathstring() + ": " + bad);
                    break;
                }
            }
            return d;
        });
    }

    check_sweep_tally(tally);
    CHECK(charts == corpus_chart_count());
    MESSAGE("checked " << charts << " charts");
}

// The engine walks an activation's timings on its own flat records
// (Engine::act_within_limit); the stored record walks them again
// (Path::within_ms_limit, through Activation::within_ms_limit). Both must give
// one answer. Two directions, on the invariants case's cached records:
// every path the engine kept under the limit passes the model's walk (a path
// that ties the unconstrained optimum is exempt on purpose, D51 call 2), and
// no path the model's walk passes outscores the engine's filtered best.
TEST_CASE("the engine's limit walk agrees with Activation::within_ms_limit") {
    constexpr double kLimitMs = kSweepLimitMs;
    int charts = 0, kept_checked = 0, model_rejected = 0;
    SweepTally tally;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (!loads_notes(song, path)) continue;

        sweep_chart(tally, path, [&]() -> std::string {
            std::string d;
            const HydraRecord& deep =
                corpus::analyzed(path, sweep_settings(kSweepDepthScores, std::nullopt));
            const HydraRecord& filtered =
                corpus::analyzed(path, sweep_settings(kSweepDepthScores, kLimitMs));
            if (deep.paths.empty()) return "no paths at the sweep depth";
            ++charts;
            const int64_t optimum = deep.paths.front().totalscore();

            for (const Path& p : filtered.paths) {
                if (p.totalscore() == optimum) continue;
                ++kept_checked;
                if (!p.within_ms_limit(kLimitMs)) {
                    d = "engine kept a path the model's walk rejects: " + p.pathstring();
                    break;
                }
            }

            std::optional<int64_t> model_best;
            for (const Path& p : deep.paths) {
                if (!p.within_ms_limit(kLimitMs)) {
                    ++model_rejected;
                    continue;
                }
                if (!model_best) model_best = p.totalscore();
            }
            if (d.empty() && model_best &&
                (filtered.paths.empty() || *model_best > filtered.paths.front().totalscore()))
                d = "engine dropped a path the model's walk keeps, scoring " +
                    std::to_string(*model_best);
            return d;
        });
    }

    check_sweep_tally(tally);
    CHECK(charts == corpus_chart_count());
    // Both walks must have had something to say no to and yes to, or the
    // case proves nothing.
    CHECK(kept_checked > 0);
    CHECK(model_rejected > 0);
    MESSAGE("checked " << charts << " charts, " << kept_checked << " kept paths, "
                       << model_rejected << " deep paths over the limit");
}

namespace {

// Every stored path of `rec`, variants included, in walk order.
void each_stored_path(const std::vector<Path>& paths, const std::function<void(const Path&)>& f) {
    for (const Path& p : paths) {
        f(p);
        each_stored_path(p.variants, f);
    }
}

}  // namespace

// The Notes column is the chart's note total, one fact about the chart, so
// every path and variant stores the same number.
TEST_CASE("note total: the engine, the Song and the Dynamics tab agree on every corpus chart") {
    // Under each 2x Bass setting: the Song's note count, every stored path's
    // notecount, and the Dynamics tab's Totals (counted from its own 2x-kept
    // parse, then played_total for the setting) are one number. The sweep
    // depth keeps many paths and variants for the check to walk.
    int checked_off = 0, checked_on = 0, rejected = 0;
    for (const std::string& path : corpus::chart_paths()) {
        for (const bool bass2x : {false, true}) {
            CAPTURE(path);
            CAPTURE(bass2x);
            const Song& song = corpus::song(path, ChartReading{Difficulty::Expert, true, bass2x});
            if (!loads_notes(song, path)) continue;
            const int total = song.note_count();

            const app::DynamicsBreakdown bd =
                app::count_dynamics(corpus::song(
                    path, ChartReading{Difficulty::Expert, true, app::kDynamicsParseBass2x}));
            CHECK(bd.played_total(bass2x).all() == total);

            app::AnalysisSettings a;
            static_cast<SearchSettings&>(a) = sweep_settings(kSweepDepthScores, std::nullopt);
            a.bass2x = bass2x;
            const HydraRecord* rec = nullptr;
            try {
                rec = &corpus::analyzed(path, a);
            } catch (const ChartFileError&) {
                ++rejected;  // counted, so the sweep cannot quietly shrink
                continue;
            }
            each_stored_path(rec->paths,
                             [&](const Path& p) { CHECK(p.notecount == total); });
            ++(bass2x ? checked_on : checked_off);
        }
    }
    CHECK(rejected == 0);
    // Each listed chart, once with 2x Bass off and once on.
    CHECK(checked_off == corpus_chart_count());
    CHECK(checked_on == corpus_chart_count());
    MESSAGE("checked " << checked_off << " charts with 2x Bass off and " << checked_on << " on");
}

// A multiplier squeeze depends on the combo alone, and a full-combo path
// never breaks combo, so the list is one fact about the chart. The graph
// finds it once; analyze_chart hands that one list to the record.
TEST_CASE("the graph finds the chart's multiplier squeezes once in chart order") {
    int charts = 0, with_squeezes = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        ++charts;
        if (!loads_notes(song, path)) continue;

        // Every chord, at the combo before it read off chord_score_table's
        // row for that chord, that MultSqueeze::applies (the rule's owner)
        // calls a multiplier squeeze.
        const ChordScoreTable table = chord_score_table(song, core::SqOutRule::FirstNote);
        std::vector<MultSqueeze> want;
        for (size_t i = 0; i < song.sequence.size(); ++i)
            if (MultSqueeze::applies(song.sequence[i].chord, table.rows[i].combo_before))
                want.emplace_back(song.sequence[i].chord, table.rows[i].combo_before);

        ScoreGraph graph(song, kCloneHeroSpCap);
        CHECK_MESSAGE(graph.multsqueezes() == want, path);
        // The record holds the graph's one list, on every chart. Depth 0 is
        // the invariants case's shallow record, so this reuses its cache.
        CHECK_MESSAGE(
            corpus::analyzed(path, sweep_settings(0, std::nullopt)).multsqueezes == want, path);
        if (!want.empty()) ++with_squeezes;
    }
    CHECK(charts == corpus_chart_count());
    // The corpus holds such chords: a reading of the .chart files' Expert
    // drums finds a chord of a cymbal and a tom or kick (notes worth different
    // points) whose first and last notes sit either side of a 10-note
    // multiplier step on several charts, Polyphia - 87 among them. So a list empty
    // on every chart means the rule stopped finding them.
    CHECK(with_squeezes > 0);
}

// The legacy Clone Hero 1.0 fill rule is a whole different spawn deadline, so
// it reshapes which activations exist at all. That must still produce a normal,
// complete record -- the score itself is not pinned here (it is a different
// game's answer, and tests/test_fill_deadline.cpp pins the math instead).
TEST_CASE("legacy fill deadline analyzes every corpus chart end to end") {
    int charts = 0, analyzed = 0;
    SweepTally tally;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        ++charts;
        if (!loads_notes(song, path)) continue;

        sweep_chart(tally, path, [&]() -> std::string {
            SearchSettings cfg = sweep_settings(0, std::nullopt);
            cfg.legacy_fills = true;
            const HydraRecord legacy = analyze_chart(song, cfg);
            std::string d;
            // The record names the rule it ran under (analyze_chart stamps
            // it from the settings) and the cap it was given. The search
            // always returns a path, one with no activations at the least.
            if (!legacy.legacy_fills) add_reason(d, "record does not name the 1.0 fill rule");
            if (legacy.sp_cap != cfg.sp_cap) add_reason(d, "record lost the SP cap");
            if (legacy.paths.empty()) add_reason(d, "no paths");
            if (d.empty()) ++analyzed;
            return d;
        });
    }

    check_sweep_tally(tally);
    CHECK(charts == corpus_chart_count());
    CHECK(analyzed == charts);
}

namespace {

// What a walk over every activation of every corpus chart found.
struct CorpusActivationRun {
    int charts = 0, mismatches = 0;
    int rejected = 0;  // charts the engine threw ChartFileError on
};

// Walks every activation of every corpus chart, analyzed with the settings
// below. `check` is called with the chart's song and one activation; it
// returns "" when the activation passes, or what is wrong. A chart stops at
// its first bad activation, and the first eight bad charts fail by name.
template <class Check>
CorpusActivationRun check_every_corpus_activation(Check check) {
    CorpusActivationRun run;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (!loads_notes(song, path)) continue;

        const HydraRecord* record = nullptr;
        try {
            record = &corpus::analyzed(path, sweep_settings(kDefaultDepthValue, std::nullopt));
        } catch (const ChartFileError&) {
            ++run.rejected;
            continue;
        }
        ++run.charts;

        std::string d;
        for (const Path* p : record->all_paths()) {
            for (const Activation& act : p->all_activations()) {
                d = check(song, act);
                if (!d.empty()) break;
            }
            if (!d.empty()) break;
        }
        if (!d.empty() && ++run.mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }
    return run;
}

}  // namespace

// The engine stamps each activation with its frontend transfer scales and its
// SP-end steps at copy-out. Across the corpus, every fresh activation holds a
// known, positive scale for its SP end and each SqIn (the ratios the squeeze
// detail lines and eff. figures show), a deactivation tick, and backend rows
// that all measure from that one SP end.
TEST_CASE("every fresh activation stores known transfer scales and backends from its SP end") {
    int acts = 0, nonflat = 0;

    const CorpusActivationRun run = check_every_corpus_activation(
        [&](const Song& song, const Activation& act) -> std::string {
            ++acts;
            if (const std::string why = corpus::unknown_scale_reason(act); !why.empty())
                return why;
            // Non-flat counts the SP end's scale and every SqIn's scale.
            // unknown_scale_reason above proved each one known.
            auto flat = [](const TransferScale& s) {
                return !is_scaled(s.early) && !is_scaled(s.late);
            };
            bool any_scaled = !flat(*act.transfer_post);
            for (const SPSqueeze& sq : act.sqinouts)
                if (sq.kind == SqueezeKind::SqIn && !flat(*sq.transfer)) any_scaled = true;
            if (any_scaled) ++nonflat;

            // Copy-out stamps deact_tick on every activation it produces, so
            // a record fresh off the engine should never be missing it.
            if (!act.deact_tick().has_value()) return "activation missing deact_tick";

            // A backend row's offset_ms is the gap from the deactivation
            // node to that row, in ms. Walking a row's offset back to a
            // tick has to land on the same node copy-out stamped -- if it
            // didn't, the backend rows and deact_tick would be describing
            // two different SP ends.
            for (const BackendSqueeze& b : act.backends) {
                if (!b.offset_ms.has_value()) continue;
                const int64_t implied =
                    *b.offset_ms == 0.0 ? b.timecode.ticks()
                                        : std::llround(song.timing().ms_index().tick_at_ms(
                                              b.timecode.ms() - *b.offset_ms));
                if (implied != *act.deact_tick())
                    return "backend-implied deact tick disagrees with the stored one";
            }
            return "";
        });

    CHECK(run.mismatches == 0);
    CHECK(run.rejected == 0);
    CHECK(run.charts == corpus_chart_count());
    // A walk that met no activation checked nothing, so it fails.
    CHECK(acts > 0);
    MESSAGE("checked " << acts << " activations on " << run.charts << " charts ("
                       << nonflat << " with a non-flat scale)");
}

// An activation that squeezes a note out of SP ends its SP on that note, so
// the note is hit after SP is gone -- and every note after it is hit later
// still. None of them can be a backend squeeze. The deactivation edge holds
// backends for every path that deactivates there, squeezed out or not, so the
// record build has to trim per activation; this pins that it does.
TEST_CASE("no activation keeps backends past its squeezed-out note") {
    int sqout_acts = 0;

    const CorpusActivationRun run = check_every_corpus_activation(
        [&](const Song&, const Activation& act) -> std::string {
            bool has_sqout = false;
            for (const SPSqueeze& sq : act.sqinouts)
                if (sq.kind == SqueezeKind::SqOut) has_sqout = true;
            if (!has_sqout) return "";
            ++sqout_acts;

            // The engine stamps the squeezed-out chord's tick; everything is
            // compared by tick, never by ms.
            if (!act.sqout_tick) return "squeeze-out activation missing sqout_tick";
            int on_sqout = 0;
            for (const BackendSqueeze& b : act.backends) {
                const core::SqOutPosition pos =
                    core::sqout_position(b.timecode.ticks(), act.sqout_tick);
                if (pos == core::SqOutPosition::After) return "backend past the sqout note";
                if (pos == core::SqOutPosition::Exact) ++on_sqout;
            }
            if (on_sqout != 1) return "not exactly one backend row on the sqout tick";
            return "";
        });

    CHECK(run.mismatches == 0);
    CHECK(run.rejected == 0);
    CHECK(run.charts == corpus_chart_count());
    MESSAGE("checked " << sqout_acts << " squeeze-out activations on " << run.charts
                       << " charts");
}

// The all-0 pass is a second, constrained search: the best path that skips no
// fill (or passes over only an E1, D102) and whose every timing is 0 ms or
// easier (D85). Two searches answer the same question from both sides. The
// all-0 pass never scores above the unconstrained optimum. And no path the
// full search keeps at the sweep depth that the all-0 rule and the 0 ms limit
// accept (a 0 ms timing or an E1 included) outscores the all-0 pass's best,
// nor exists on a chart where the all-0 pass found nothing.
TEST_CASE("search_allzero finds the best all-0 path the full search holds") {
    int charts = 0, mismatches = 0, found = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        ++charts;
        if (!loads_notes(song, path)) continue;

        ScoreGraph graph(song, kCloneHeroSpCap);
        const std::vector<Path> allzero = search_allzero(graph);
        std::optional<int64_t> allzero_best;
        for (const Path& p : allzero)
            if (!allzero_best || p.totalscore() > *allzero_best) allzero_best = p.totalscore();
        if (allzero_best) ++found;

        const HydraRecord& full =
            corpus::analyzed(path, sweep_settings(kSweepDepthScores, std::nullopt));
        std::string d;
        if (full.paths.empty()) add_reason(d, "no paths at the sweep depth");
        if (allzero_best && !full.paths.empty() &&
            *allzero_best > full.paths.front().totalscore())
            add_reason(d, "all-0 best scores above the optimum");
        for (const Path* p : full.all_paths()) {
            if (!p->is_allzero() || !p->within_ms_limit(0.0)) continue;
            if (!allzero_best)
                add_reason(d, "all-0 pass found nothing, full search holds " + p->pathstring());
            else if (p->totalscore() > *allzero_best)
                add_reason(d, "full search's all-0 path outscores the all-0 pass: " +
                                  p->pathstring());
            if (!d.empty()) break;
        }
        if (!d.empty() && ++mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }

    CHECK(mismatches == 0);
    CHECK(charts == corpus_chart_count());
    MESSAGE("checked " << charts << " charts, " << found << " with an all-0 path");
}

// A squeeze-in whose note sits exactly on the SP end needs 0.0 ms
// (SPSqueeze::needs_no_timing), and the library saves the engine's own figure
// (engine_hardest_ms) and shows it through shown_timing. Every count below is
// pinned from one run at the GUI's default settings, so a change to which
// timings count, to what the store saves or to the all-0 list shows here.
TEST_CASE("corpus: squeeze-ins at exactly 0 and what they change") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0;
    // Every kept path, the main search's and the all-0 pass's, as
    // test_pathstring_read.cpp walks them. An activation is counted once for
    // each path that walks it, so a shared variant tail counts again.
    int sqin_at_zero_acts = 0;      // activations with a squeeze-in at offset 0.0
    int sqin_at_zero_timed = 0;     // ... of them, whose difficulty() is 0.0
    int best_timed_by_sqin = 0;     // best paths at 0.0 through such a squeeze-in
    // Every path record_of keeps, read off the PathRow it builds.
    int saved_rows = 0;
    std::vector<std::string> shown_differs;  // shown_timing(saved) != difficulty()
    int saved_negative = 0;         // a figure below 0: saved, shown as none
    // sqin_count > 0 and difficulty() empty: the rows a fresh analysis leaves
    // with nothing to time (every squeeze-in inside Star Power). This is not
    // the count the refill's query takes from an old library. That query
    // (hardest_ms IS NULL AND sqin_count > 0) also takes the old rows whose
    // only timing is a squeeze-in at exactly 0.0, which were empty then and
    // show 0.0 now; counting those needs the old rule, which no longer exists
    // in the code, so this number is lower than what the query would take.
    int still_empty_with_sqin = 0;
    // The all-0 list, as it was on the code before the squeeze-in at 0.0 became
    // a timing; pinned so a change to it shows.
    int allzero_charts = 0, allzero_paths = 0;

    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        ++charts;

        std::vector<const Path*> kept = rec.all_paths();
        const std::vector<const Path*> allzero = rec.all_allzero_paths();
        if (!allzero.empty()) ++allzero_charts;
        allzero_paths += static_cast<int>(allzero.size());
        kept.insert(kept.end(), allzero.begin(), allzero.end());
        for (const Path* p : kept)
            for (const Activation& a : p->walk_activations()) {
                const bool at_zero = std::any_of(
                    a.sqinouts.begin(), a.sqinouts.end(), [](const SPSqueeze& sq) {
                        return sq.kind == SqueezeKind::SqIn && sq.offset_ms == 0.0;
                    });
                if (!at_zero) continue;
                ++sqin_at_zero_acts;
                if (a.difficulty() == std::optional<double>(0.0)) ++sqin_at_zero_timed;
            }

        if (rec.paths.empty()) continue;
        const Path& best = rec.best_path();
        if (best.difficulty() == std::optional<double>(0.0)) {
            const ActivationWalk acts = best.walk_activations();
            const HardestTiming at_zero{TimingPart::SqueezeIn, 0.0};
            if (std::any_of(acts.begin(), acts.end(),
                            [&](const Activation& a) { return a.hardest() == at_zero; }))
                ++best_timed_by_sqin;
        }

        const store::RecordRow row = store::record_of("", rec);
        const std::vector<const Path*> ranked = rec.all_paths();
        for (size_t i = 0; i < row.paths.size(); ++i) {
            const store::PathRow& saved = row.paths[i];
            const Path& path = *ranked[i];
            // record_of keeps all_paths() in order; a row paired with another
            // path would make the comparison below meaningless.
            REQUIRE(saved.path == path.pathstring());
            REQUIRE(saved.score == path.totalscore());
            ++saved_rows;
            if (shown_timing(saved.hardest_ms) != path.difficulty())
                shown_differs.push_back(chart + " " + saved.path);
            if (saved.hardest_ms && *saved.hardest_ms < 0.0) ++saved_negative;
            if (saved.sqin_count > 0 && !path.difficulty()) ++still_empty_with_sqin;
        }
    }

    for (const std::string& d : shown_differs) CHECK_MESSAGE(false, "shown differs: " << d);
    CHECK(charts == 98);
    CHECK(sqin_at_zero_acts == 56);
    CHECK(sqin_at_zero_timed == 56);
    CHECK(best_timed_by_sqin == 5);
    CHECK(saved_rows == 374);
    // A fresh row shows what Path::difficulty() shows: the check that saving
    // the engine's figure changed nothing on screen.
    CHECK(shown_differs.size() == 0);
    CHECK(saved_negative == 90);
    CHECK(still_empty_with_sqin == 79);
    CHECK(allzero_charts == 69);
    CHECK(allzero_paths == 70);
}

TEST_CASE("score graph: the backend rows are sized once, one per chord") {
    // The graph sizes its row list from the song before building, so it ends
    // with one row per chord and no spare room from growing a row at a time.
    int built = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        const ScoreGraph graph(song, 4);
        ++built;
        CHECK_MESSAGE(graph.all_backends().size() == song.sequence.size(), path);
        CHECK_MESSAGE(graph.all_backends().capacity() == song.sequence.size(), path);
    }
    CHECK(built > 0);
}

namespace {

// The two SP-past-the-end songs (record_fixtures.h), for the cases below.
using test::past_last_note_song;
using test::mid_sp_phrase_song;

}  // namespace

// run_search takes its knobs in one EngineOptions value, so no two flags can
// be swapped at a call site. Each knob must still do its own job.
TEST_CASE("run_search: EngineOptions carries each knob to the engine") {
    Song song = past_last_note_song();
    ScoreGraph graph(song, 4);

    // The defaults are a plain best-path search: score depth 0, no limit.
    const std::vector<Path> best = run_search(graph, EngineOptions{});
    REQUIRE_FALSE(best.empty());
    REQUIRE_FALSE(best.front().activations.empty());

    // no_skips plus a hard 0 ms limit is exactly the all-0 search.
    const std::vector<Path> z = run_search(graph, allzero_options());
    for (const Path& p : z) CHECK(p.within_ms_limit(0.0));
    const std::vector<Path> want_z = search_allzero(graph);
    REQUIRE_FALSE(want_z.empty());
    REQUIRE(z.size() == want_z.size());
    for (size_t i = 0; i < z.size(); ++i) {
        CHECK(z[i].pathstring() == want_z[i].pathstring());
        CHECK(z[i].totalscore() == want_z[i].totalscore());
    }

    // Pinning the best path's activation ticks hands that path back.
    EngineOptions pinned;
    pinned.depth_mode = DepthMode::Points;
    pinned.depth_value = kKeepEveryPathBand;
    const std::vector<int64_t> ticks = test::act_ticks(best.front());
    pinned.target_act_ticks = ticks;
    const std::vector<Path> again = run_search(graph, pinned);
    REQUIRE_FALSE(again.empty());
    CHECK(again.front().pathstring() == best.front().pathstring());
    CHECK(again.front().totalscore() == best.front().totalscore());
}

// The all-0 search's options have one owner, so search_allzero and the case
// above run the same search.
TEST_CASE("allzero_options: no skips, a hard 0 ms limit, everything else default") {
    const EngineOptions z = allzero_options();
    const EngineOptions plain{};
    CHECK(z.no_skips);
    CHECK(z.hard_ms_filter);
    REQUIRE(z.ms_filter.has_value());
    CHECK(*z.ms_filter == 0.0);  // D85: the 0 ms limit, whatever the Path limit
    CHECK(z.depth_mode == plain.depth_mode);
    CHECK(z.depth_value == plain.depth_value);
    CHECK_FALSE(z.target_act_ticks.has_value());
}

// The test cache keys a stored analysis by corpus::settings_key. The default's
// text is pinned from one run, so the key's shape cannot drift, and each
// SearchSettings field must move it, so a field left out of the key shows up
// here.
TEST_CASE("settings_key: the text for SearchSettings{} is pinned, and every field moves it") {
    const SearchSettings plain{};
    const std::string key = corpus::settings_key(plain);
    // The long number is the default rules' fingerprint (test_rules.cpp pins it in
    // hex).
    // The 60 is the default early-fill window (kDefaultEarlyFillWindowMs).
    CHECK(key == "4|0|4|none|0|60|12303881770575177486|");

    std::vector<SearchSettings> changed(7, plain);
    changed[0].sp_cap = 1;
    changed[1].depth_mode = DepthMode::Points;
    changed[2].depth_value = 5;
    changed[3].ms_filter = 10.0;
    changed[4].legacy_fills = true;
    changed[5].rules.max_tied_paths += 1;
    changed[6].early_fill_window_ms = 85.0;
    for (size_t i = 0; i < changed.size(); ++i) {
        INFO("changed field " << i);
        CHECK(corpus::settings_key(changed[i]) != key);
    }
}

// analyze_chart builds the graph for Clone Hero's 4 bars only as tall as the
// song has phrases (graph_build_cap). A graph built a flat 4 bars tall gives
// the same answer: a song with p phrases never holds more than p bars, so a
// p-bar ceiling clamps nothing a 4-bar ceiling would not. This pins it field by
// field (digest::record_hash).
TEST_CASE("a 4-bar graph built at the song's phrase count stores the same paths") {
    std::vector<Song> songs;
    // Hand-built, three phrases, one of them collected mid-SP.
    songs.push_back(mid_sp_phrase_song());
    for (const std::string& path : corpus::chart_paths()) {
        Song s = load_songpath(path, ChartReading{});
        if (!s.is_empty() && s.sp_phrase_count() < kCloneHeroSpCap)
            songs.push_back(std::move(s));
    }

    int compared = 0;
    for (const Song& song : songs) {
        const int build_cap = graph_build_cap(kCloneHeroSpCap, song.sp_phrase_count());
        REQUIRE(build_cap < kCloneHeroSpCap);

        ScoreGraph g_tall(song, kCloneHeroSpCap);
        ScoreGraph g_built(song, build_cap);
        EngineOptions options;
        options.depth_value = 4;
        HydraRecord tall, built;
        tall.sp_cap = kCloneHeroSpCap;
        built.sp_cap = kCloneHeroSpCap;
        tall.paths = run_search(g_tall, options);
        built.paths = run_search(g_built, options);
        tall.allzero_paths = search_allzero(g_tall);
        built.allzero_paths = search_allzero(g_built);

        CHECK_MESSAGE(digest::record_hash(tall) == digest::record_hash(built),
                      "a song with " << song.sp_phrase_count() << " phrases");
        ++compared;
    }
    MESSAGE("compared " << compared << " songs with fewer than 4 phrases");
}

TEST_CASE("graph_build_cap: never taller than the song's phrases, never below one") {
    CHECK(graph_build_cap(4, 10) == 4);   // the cap binds
    CHECK(graph_build_cap(32, 3) == 3);   // the song's phrases bind
    CHECK(graph_build_cap(8, 0) == 1);    // a phraseless song still builds one level
    // The meter's own height, which graph_build_cap reads: no floor of its own.
    CHECK(max_sp_bars(4, 10) == 4);
    CHECK(max_sp_bars(32, 3) == 3);
    CHECK(max_sp_bars(8, 0) == 0);
    CHECK(max_sp_bars(std::nullopt, 3) == 3);
}
