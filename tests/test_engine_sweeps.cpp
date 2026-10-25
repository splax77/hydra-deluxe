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

}  // namespace

TEST_CASE("search invariants hold across the corpus and config knobs") {
    int charts = 0;
    SweepTally tally;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        ++charts;

        sweep_chart(tally, path, [&]() -> std::string {
            std::string d;
            // Depth asks for more alternate paths and must not move the
            // optimum; the ms filter is a constraint, so its best can only
            // be at or below the unconstrained best.
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_mode = DepthMode::Scores;
            cfg.depth_value = 0;
            cfg.ms_filter = std::nullopt;
            const HydraRecord& shallow = corpus::analyzed(path, cfg);
            cfg.depth_value = 200;
            const HydraRecord& deep = corpus::analyzed(path, cfg);
            cfg.ms_filter = 20.0;
            const HydraRecord& filtered = corpus::analyzed(path, cfg);

            if (shallow.paths.empty()) {
                d = "no paths";
            } else {
                const int64_t best = shallow.paths.front().totalscore();
                if (deep.paths.front().totalscore() != best)
                    d = "depth changed the best score";
                else if (!filtered.paths.empty() &&
                         filtered.paths.front().totalscore() > best)
                    d = "ms filter scored above the unconstrained best";
                else if (deep.paths.size() < shallow.paths.size())
                    d = "deeper search returned fewer paths";

                // Paths come out best-first, and every path in a record
                // reports the same chart notecount.
                int64_t prev = INT64_MAX;
                const int notecount = deep.paths.front().notecount;
                for (const Path& p : deep.paths) {
                    if (p.totalscore() > prev) {
                        d = "paths not sorted by score";
                        break;
                    }
                    prev = p.totalscore();
                    if (p.notecount != notecount) {
                        d = "notecount varies between paths";
                        break;
                    }
                    if (p.tied_pathcount() < 1) {
                        d = "tied_pathcount below 1";
                        break;
                    }
                }
            }
            return d;
        });
    }

    check_sweep_tally(tally);
    REQUIRE(charts > 0);
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
    constexpr double kLimitMs = 20.0;  // the invariants case's ms_filter
    int charts = 0, kept_checked = 0, model_rejected = 0;
    SweepTally tally;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;

        sweep_chart(tally, path, [&]() -> std::string {
            std::string d;
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_mode = DepthMode::Scores;
            cfg.depth_value = 200;
            cfg.ms_filter = std::nullopt;
            const HydraRecord& deep = corpus::analyzed(path, cfg);
            cfg.ms_filter = kLimitMs;
            const HydraRecord& filtered = corpus::analyzed(path, cfg);
            if (deep.paths.empty()) return d;
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
    REQUIRE(charts > 0);
    // Both walks must have had something to say no to and yes to, or the
    // case proves nothing.
    CHECK(kept_checked > 0);
    CHECK(model_rejected > 0);
    MESSAGE("checked " << charts << " charts, " << kept_checked << " kept paths, "
                       << model_rejected << " deep paths over the limit");
}

namespace {

// The invariants case's deep settings above (depth 200, cap 4), so the
// 2x Bass on runs below reuse its cached records, and a chart has many paths.
SearchSettings note_total_settings() {
    SearchSettings cfg;
    cfg.sp_cap = 4;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 200;
    cfg.ms_filter = std::nullopt;
    return cfg;
}

// Every stored path of `rec`, variants included, in walk order.
void each_stored_path(const std::vector<Path>& paths, const std::function<void(const Path&)>& f) {
    for (const Path& p : paths) {
        f(p);
        each_stored_path(p.variants, f);
    }
}

}  // namespace

TEST_CASE("note total: every path of How You Remind Me stores the Song's note count") {
    // Finding 255: the Notes column is the chart's note total, one fact about
    // the chart, so every path and variant stores the same number.
    const std::string chart = corpus::root() +
                              "/common/Summer Blast _25 Setlist/Tier 1/Nickelback - How You Remind Me/"
                              "notes.chart";
    const Song& song = corpus::song(chart, ChartReading{});
    CHECK(song.note_count() == 905);
    const HydraRecord& rec = corpus::analyzed(chart, note_total_settings());
    int paths = 0;
    each_stored_path(rec.paths, [&](const Path& p) {
        ++paths;
        CHECK(p.notecount == 905);
    });
    CHECK(paths > 1);
}

TEST_CASE("note total: the engine, the Song and the Dynamics tab agree on every corpus chart") {
    // Finding 255. Under each 2x Bass setting: the Song's note count, every
    // stored path's notecount, and the Dynamics tab's Totals (counted from its
    // own 2x-kept parse, then played_total for the setting) are one number.
    int checked = 0, rejected = 0;
    for (const std::string& path : corpus::chart_paths()) {
        for (const bool bass2x : {false, true}) {
            CAPTURE(path);
            CAPTURE(bass2x);
            const Song& song = corpus::song(path, ChartReading{Difficulty::Expert, true, bass2x});
            if (song.is_empty()) continue;
            const int total = song.note_count();

            const app::DynamicsBreakdown bd =
                app::count_dynamics(corpus::song(
                    path, ChartReading{Difficulty::Expert, true, app::kDynamicsParseBass2x}));
            CHECK(bd.played_total(bass2x).all() == total);

            app::AnalysisSettings a;
            static_cast<SearchSettings&>(a) = note_total_settings();
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
            ++checked;
        }
    }
    CHECK(rejected == 0);
    REQUIRE(checked > 0);
    MESSAGE("checked " << checked << " chart and 2x Bass pairs");
}

// A multiplier squeeze depends on the combo alone, and a full-combo path
// never breaks combo, so the list is one fact about the chart. The graph
// finds it once; analyze_chart hands that one list to the record.
TEST_CASE("the graph finds the chart's multiplier squeezes once, in chart order") {
    int charts = 0, with_squeezes = 0, analyzed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        ++charts;

        // Every chord, at the combo before it, read off chord_score_table's
        // row for that chord.
        const ChordScoreTable table = chord_score_table(song, core::SqOutRule::FirstNote);
        std::vector<MultSqueeze> want;
        for (size_t i = 0; i < song.sequence.size(); ++i) {
            try {
                want.push_back(MultSqueeze(song.sequence[i].chord, table.rows[i].combo_before));
            } catch (const std::invalid_argument&) {
            }
        }

        ScoreGraph graph(song, 4);
        CHECK_MESSAGE(graph.multsqueezes() == want, path);
        if (want.empty()) continue;
        ++with_squeezes;

        if (analyzed < 3) {
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_value = 0;
            CHECK_MESSAGE(corpus::analyzed(path, cfg).multsqueezes == want, path);
            ++analyzed;
        }
    }
    REQUIRE(charts > 0);
    CHECK(with_squeezes > 0);
}

// The legacy Clone Hero 1.0 fill rule is a whole different spawn deadline, so
// it reshapes which activations exist at all. That must still produce a normal,
// complete record -- the score itself is not pinned here (it is a different
// game's answer, and tests/test_fill_deadline.cpp pins the math instead).
TEST_CASE("legacy fill deadline analyzes a chart end to end") {
    int analyzed = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;

        std::optional<HydraRecord> legacy;
        try {
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_value = 0;
            cfg.legacy_fills = true;
            legacy = analyze_chart(song, cfg);
        } catch (const ChartFileError&) {
            continue;
        }

        // A record came back, and its paths are real ones the engine scored.
        REQUIRE(legacy.has_value());
        CHECK(legacy->sp_cap == 4);
        if (!legacy->paths.empty()) {
            CHECK(legacy->best_path().totalscore() > 0);
            ++analyzed;
        }
        if (analyzed >= 3) break;  // three charts is enough to prove the path
    }

    CHECK(analyzed > 0);
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
        if (song.is_empty()) continue;

        const HydraRecord* record = nullptr;
        try {
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_mode = DepthMode::Scores;
            cfg.depth_value = 4;
            cfg.ms_filter = std::nullopt;
            record = &corpus::analyzed(path, cfg);
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

// The engine stamps each activation with its frontend transfer scales at
// copy-out, through frontend_transfer_scales, from the SP-end steps it just
// stored. This pins the stored values against a live recompute from those
// steps across the corpus -- and with them the ratios the squeeze detail
// lines and eff. figures show.
TEST_CASE("stored transfer scales match the display-layer recomputation") {
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

            auto scales = frontend_transfer_scales(act, song.timing());
            if (!scales) return "display recomputation returned no scales";
            auto differs = [](const TransferScale& a, const TransferScale& b) {
                return std::abs(a.early - b.early) > 1e-9 || std::abs(a.late - b.late) > 1e-9;
            };
            if (differs(scales->post, *act.transfer_post))
                return "stored scales diverge from recomputation";
            // A non-positive scale is caught by unknown_scale_reason, above.
            // Each SqIn's stored scale, in SqIn order as
            // stored_transfer_scales pairs them, against the recomputed one.
            const std::optional<ActTransferScales> stored = stored_transfer_scales(act);
            if (!stored || !std::equal(stored->sqins.begin(), stored->sqins.end(),
                                       scales->sqins.begin(), scales->sqins.end(),
                                       [&](const TransferScale& a, const TransferScale& b) {
                                           return !differs(a, b);
                                       }))
                return "a SqIn's stored scale diverges from recomputation";

            // Copy-out stamps deact_tick on every activation it produces
            // (blob v4), so a record fresh off the engine should never be
            // missing it.
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
    REQUIRE(run.charts > 0);
    REQUIRE(acts > 0);
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

            // The engine stamps the squeezed-out chord's tick (record
            // v6); everything is compared by tick, never by ms.
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
    REQUIRE(run.charts > 0);
    MESSAGE("checked " << sqout_acts << " squeeze-out activations on " << run.charts
                       << " charts");
}

// The all-0 pass is a second, constrained search. Its whole contract is that
// every activation it reports fits the all-0 rule (no skips, or an E1: D102),
// that the 0 ms limit
// (Path::within_ms_limit) is a requirement rather than a preference, and that
// it never scores above the unconstrained optimum.
TEST_CASE("search_allzero returns only all-0 paths inside the 0 ms limit") {
    int checks = 0, mismatches = 0, found = 0;
    // Charts whose all-0 list keeps a path with a timing of exactly 0 ms: a
    // squeeze-out dead on the SP end or a zero-slack early fill. E3 dropped
    // these (D60); D85 restored them, so the corpus must still have some.
    std::set<std::string> zero_ms_charts;
    // Charts whose all-0 list keeps an E1: an activation that passed over
    // only its early fill (D102). The corpus must still have some.
    std::set<std::string> e1_charts;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;

        ScoreGraph graph(song, 4);
        std::vector<Path> allzero = search_allzero(graph);
        ++checks;
        if (allzero.empty()) continue;
        ++found;

        HydraRecord holder;
        holder.allzero_paths = allzero;
        const int64_t optimum =
            run_search(graph, EngineOptions{})
                .front()
                .totalscore();

        std::string d;
        for (const Path* p : holder.all_allzero_paths()) {
            if (!p->is_allzero()) {
                d = "not all-0: " + p->pathstring();
                break;
            }
            if (!p->within_ms_limit(0.0)) {
                d = "over the 0 ms limit: " + p->pathstring();
                break;
            }
            // A timing of exactly 0 ms needs hitting yet passes (D85).
            if (p->needs_timing()) zero_ms_charts.insert(path);
            for (const Activation& a : p->walk_activations())
                if (a.skips() == 1) e1_charts.insert(path);
            if (p->totalscore() > optimum) {
                d = "scores above the optimum: " + p->pathstring();
                break;
            }
        }
        if (!d.empty() && ++mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }

    CHECK(mismatches == 0);
    CHECK(found > 0);
    CHECK_FALSE(zero_ms_charts.empty());
    CHECK_FALSE(e1_charts.empty());
    MESSAGE("checked " << checks << " charts, " << found << " with an all-0 path, "
                       << zero_ms_charts.size() << " of them with a 0 ms timing, "
                       << e1_charts.size() << " with an E1");
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
    // The number is the default rules' fingerprint, which moved at D115 B2-3
    // when fill_length_measures was retired (test_rules.cpp pins it in hex).
    CHECK(key == "4|0|4|none|0|12303881770575177486|");

    std::vector<SearchSettings> changed(6, plain);
    changed[0].sp_cap = 1;
    changed[1].depth_mode = DepthMode::Points;
    changed[2].depth_value = 5;
    changed[3].ms_filter = 10.0;
    changed[4].legacy_fills = true;
    changed[5].rules.max_tied_paths += 1;
    for (size_t i = 0; i < changed.size(); ++i) {
        INFO("changed field " << i);
        CHECK(corpus::settings_key(changed[i]) != key);
    }
}

// analyze_chart used to run Clone Hero's 4 bars down its own branch, with the
// graph built a flat 4 bars tall. Every other fixed cap builds the graph only
// as tall as the song has phrases (graph_build_cap). Both give the same
// answer: a song with p phrases never holds more than p bars, so a p-bar
// ceiling clamps nothing a 4-bar ceiling would not. This pins it field by
// field (digest::record_hash) before the branches fold into one.
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
