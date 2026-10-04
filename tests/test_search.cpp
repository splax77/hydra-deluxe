// Invariant tests for search/ (ScoreGraph + engine + pather).
//
// The engine's best score must be stable across the config knobs that are not
// supposed to change it (search depth, the ms filter), every reported path
// must be internally consistent, and the all-0 pass must honor its contract.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"
#include "store/path_codec.h"
#include "store/serialize.h"

using namespace hydra;

TEST_CASE("search invariants hold across the corpus and config knobs") {
    int charts = 0, mismatches = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ++charts;

        std::string d;
        try {
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
        } catch (const ChartFileError&) {
            continue;  // charts the engine rejects are covered elsewhere
        }

        if (!d.empty() && ++mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }

    CHECK(mismatches == 0);
    REQUIRE(charts > 0);
    MESSAGE("checked " << charts << " charts");
}

// The legacy Clone Hero 1.0 fill rule is a whole different spawn deadline, so
// it reshapes which activations exist at all. That must still produce a normal,
// complete record -- the score itself is not pinned here (it is a different
// game's answer, and tests/test_fill_deadline.cpp pins the math instead).
// A multiplier squeeze depends on the combo alone, and a full-combo path
// never breaks combo, so the list is one fact about the chart. The graph
// finds it once; analyze_chart hands that one list to the record.
TEST_CASE("the graph finds the chart's multiplier squeezes once, in chart order") {
    int charts = 0, with_squeezes = 0, analyzed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ++charts;

        // An independent spelling: every chord, at the combo before it.
        std::vector<MultSqueeze> want;
        int combo = 0;
        for (const SongTimestamp& ts : song.sequence) {
            try {
                want.push_back(MultSqueeze(ts.chord, combo));
            } catch (const std::invalid_argument&) {
            }
            combo += ts.chord.count();
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

TEST_CASE("legacy fill deadline analyzes a chart end to end") {
    int analyzed = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;

        std::optional<HydraRecord> legacy;
        try {
            SearchSettings cfg;
            cfg.sp_cap = 4;
            cfg.depth_value = 0;
            cfg.legacy_fill_deadline = true;
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

// The engine stamps each activation with its frontend transfer scales at
// copy-out, through frontend_transfer_scales, from the SP-end steps it just
// stored. This pins the stored values against a live recompute from those
// steps across the corpus -- and with them the ratios the squeeze detail
// lines and eff. figures show.
TEST_CASE("stored transfer scales match the display-layer recomputation") {
    int charts = 0, acts = 0, nonflat = 0, mismatches = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
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
            continue;
        }
        ++charts;

        std::string d;
        for (const Path* p : record->all_paths()) {
            for (const Activation& act : p->all_activations()) {
                ++acts;
                if (!act.transfer_post) {
                    d = "transfer_post unknown";
                    break;
                }
                if (act.transfer_post->early != 1.0 || act.transfer_post->late != 1.0)
                    ++nonflat;

                auto scales = frontend_transfer_scales(act, song.timing());
                if (!scales) {
                    d = "display recomputation returned no scales";
                    break;
                }
                auto differs = [](const TransferScale& a, const TransferScale& b) {
                    return std::abs(a.early - b.early) > 1e-9 ||
                           std::abs(a.late - b.late) > 1e-9;
                };
                auto non_positive = [](const TransferScale& s) {
                    return s.early <= 0.0 || s.late <= 0.0;
                };
                if (differs(scales->post, *act.transfer_post)) {
                    d = "stored scales diverge from recomputation";
                    break;
                }
                if (non_positive(*act.transfer_post)) {
                    d = "non-positive transfer scale";
                    break;
                }
                // Each SqIn's stored scale is the j-th recomputed one.
                size_t j = 0;
                for (const SPSqueeze& sq : act.sqinouts) {
                    if (sq.kind != SqueezeKind::SqIn) continue;
                    if (!sq.transfer) {
                        d = "SqIn transfer unknown";
                        break;
                    }
                    if (j >= scales->sqins.size() || differs(scales->sqins[j], *sq.transfer)) {
                        d = "a SqIn's stored scale diverges from recomputation";
                        break;
                    }
                    if (non_positive(*sq.transfer)) {
                        d = "non-positive SqIn transfer scale";
                        break;
                    }
                    ++j;
                }
                if (!d.empty()) break;
                if (j != scales->sqins.size()) {
                    d = "recomputation has a scale for a SqIn the record lacks";
                    break;
                }

                // Copy-out stamps deact_tick on every activation it produces
                // (blob v4), so a record fresh off the engine should never be
                // missing it.
                if (!act.deact_tick().has_value()) {
                    d = "activation missing deact_tick";
                    break;
                }

                // A backend row's offset_ms is the gap from the deactivation
                // node to that row, in ms. Walking a row's offset back to a
                // tick has to land on the same node copy-out stamped -- if it
                // didn't, the backend rows and deact_tick would be describing
                // two different SP ends.
                for (const BackendSqueeze& b : act.backends) {
                    if (!b.offset_ms.has_value()) continue;
                    const int64_t implied =
                        *b.offset_ms == 0.0
                            ? b.timecode.ticks()
                            : std::llround(song.timing().ms_index().tick_at_ms(
                                  b.timecode.ms() - *b.offset_ms));
                    if (implied != *act.deact_tick()) {
                        d = "backend-implied deact tick disagrees with the stored one";
                        break;
                    }
                }
                if (!d.empty()) break;
            }
            if (!d.empty()) break;
        }
        if (!d.empty() && ++mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }

    CHECK(mismatches == 0);
    REQUIRE(charts > 0);
    REQUIRE(acts > 0);
    MESSAGE("checked " << acts << " activations on " << charts << " charts ("
                       << nonflat << " with a non-flat scale)");
}

// An activation that squeezes a note out of SP ends its SP on that note, so
// the note is hit after SP is gone -- and every note after it is hit later
// still. None of them can be a backend squeeze. The deactivation edge holds
// backends for every path that deactivates there, squeezed out or not, so the
// record build has to trim per activation; this pins that it does.
TEST_CASE("no activation keeps backends past its squeezed-out note") {
    int charts = 0, sqout_acts = 0, mismatches = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
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
            continue;
        }
        ++charts;

        std::string d;
        for (const Path* p : record->all_paths()) {
            for (const Activation& act : p->all_activations()) {
                bool has_sqout = false;
                for (const SPSqueeze& sq : act.sqinouts)
                    if (sq.kind == SqueezeKind::SqOut) has_sqout = true;
                if (!has_sqout) continue;
                ++sqout_acts;

                // The engine stamps the squeezed-out chord's tick (record
                // v6); everything is compared by tick, never by ms.
                if (!act.sqout_tick) {
                    d = "squeeze-out activation missing sqout_tick";
                    break;
                }
                int on_sqout = 0;
                for (const BackendSqueeze& b : act.backends) {
                    if (b.timecode.ticks() > *act.sqout_tick) {
                        d = "backend past the sqout note";
                        break;
                    }
                    if (b.timecode.ticks() == *act.sqout_tick) ++on_sqout;
                }
                if (d.empty() && on_sqout != 1)
                    d = "not exactly one backend row on the sqout tick";
                if (!d.empty()) break;
            }
            if (!d.empty()) break;
        }
        if (!d.empty() && ++mismatches <= 8)
            CHECK_MESSAGE(false, path << " " << d);
    }

    CHECK(mismatches == 0);
    REQUIRE(charts > 0);
    MESSAGE("checked " << sqout_acts << " squeeze-out activations on " << charts
                       << " charts");
}

// The all-0 pass is a second, constrained search. Its whole contract is that
// every activation it reports records skips == 0, that the 0 ms limit is a
// requirement rather than a preference, and that it never scores above the
// unconstrained optimum.
TEST_CASE("search_allzero returns only all-0 paths inside the 0 ms limit") {
    int checks = 0, mismatches = 0, found = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
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
            if (p->difficulty().value_or(0.0) > 0.0) {
                d = "over the 0 ms limit: " + p->pathstring() + " needs " +
                    std::to_string(*p->difficulty()) + " ms";
                break;
            }
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
    MESSAGE("checked " << checks << " charts, " << found << " with an all-0 path");
}

// ---- SP that outlasts the chart ----------------------------------------
//
// When the last activation's Star Power ends after the chart's final note,
// the graph never builds a deactivation edge, so no edge holds the trailing
// notes. The rebuild step synthesizes those rows against the SP end the
// engine tracked (Path::sp_end_time), which includes every mid-SP phrase
// extension the plain measure-count reconstruction cannot see.

namespace {

// A hand-built 4/4 120 BPM song. 192 ticks per beat, so a measure is 768
// ticks and 2000 ms; one tick is 2000/768 ms. Built directly rather than
// parsed so the note ticks in the assertions below are exactly these.
// record_fixtures.h owns the note type and the song builder; these names
// only bring that one copy into this file.
using test::TailNote;
using test::build_tail_song;

double tick_ms(const Song& song, int64_t tick) {
    return song.timing().ms_index().at(tick);
}

// The best path's final activation, which every case here expects to be one
// that never deactivated.
const Activation& last_act(const std::vector<Path>& paths) {
    REQUIRE(!paths.empty());
    REQUIRE(!paths.front().activations.empty());
    return paths.front().activations.back();
}

}  // namespace

TEST_CASE("SP past the last note: backends measured from the tracked SP end") {
    // Two SP phrases, then an activation at tick 2304 with a 2-bar meter, so
    // SP ends 4 measures later at tick 5376. The chart stops at 5280.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {1536},
                                 {2304, false, true},
                                 {3072},
                                 {3840},
                                 {4608},
                                 {5136},
                                 {5280}});

    ScoreGraph graph(song, 4);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.sp_meter == 2);
    CHECK(act.timecode.ticks() == 2304);

    const int64_t end_tick = 5376;
    const double end_ms = tick_ms(song, end_tick);

    // The rows exist at all -- the bug this pins showed "Backends: None."
    REQUIRE(!act.backends.empty());

    // Every trailing note is before the SP end, so every offset is negative.
    for (const BackendSqueeze& b : act.backends) {
        REQUIRE(b.offset_ms.has_value());
        CHECK(*b.offset_ms < 0.0);
    }

    // The chart's last note is one of the rows, at its true distance.
    const BackendSqueeze* last_note = nullptr;
    for (const BackendSqueeze& b : act.backends)
        if (b.timecode.ticks() == 5280) last_note = &b;
    REQUIRE(last_note != nullptr);
    CHECK(*last_note->offset_ms ==
          doctest::Approx(tick_ms(song, 5280) - end_ms).epsilon(1e-9));

    // And the rows put the SP end back exactly where the engine had it.
    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == end_tick);
}

TEST_CASE("SP past the last note: a mid-activation phrase extends the end") {
    // Same shape, but the note at 3840 completes an SP phrase during the
    // activation. That pushes the pending deactivation two measures out, from
    // tick 5376 to 6912 -- an extension the measure-count fallback (2 measures
    // per SP bar, from a meter still recorded as 2) cannot see.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {1536},
                                 {2304, false, true},
                                 {3072},
                                 {3840, true, false},
                                 {4608},
                                 {5376},
                                 {6144},
                                 {6720},
                                 {6816}});

    ScoreGraph graph(song, 4);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.sp_meter == 2);

    const int64_t extended_tick = 6912;
    const int64_t plain_tick = 5376;

    REQUIRE(!act.backends.empty());
    for (const BackendSqueeze& b : act.backends) {
        REQUIRE(b.offset_ms.has_value());
        CHECK(*b.offset_ms < 0.0);
    }

    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == extended_tick);
    CHECK(*deact != plain_tick);
    // The activation records no SqIn, so the old fallback would have landed
    // on the plain end. Pin that the rows, not the reconstruction, answered.
    CHECK(act.sqinouts.empty());
    CHECK(song.timing().plusmeasure(act.timecode, 4).ticks() == plain_tick);
}

TEST_CASE("SP past the last note: synthesized rows survive a store round-trip") {
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {1536},
                                 {2304, false, true},
                                 {3072},
                                 {3840},
                                 {4608},
                                 {5136},
                                 {5280}});

    ScoreGraph graph(song, 4);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);
    REQUIRE(!act.backends.empty());

    // Decoded timecodes are ticks-only until restore_timecodes resolves them
    // against the song's tempo map -- the same two steps a store load takes.
    HydraRecord back;
    back.paths.push_back(
        store::decode_path_node(store::encode_path_node(paths.front())));
    store::restore_timecodes(back, song.timing());
    REQUIRE(back.paths.front().activations.size() ==
            paths.front().activations.size());
    const Activation& ract = back.paths.front().activations.back();

    // The writer stores display_backends(), so what survives is the rows
    // inside the +/-500 ms display window -- the same trim a deactivating
    // activation's rows get. Here that is the last note, at -250 ms.
    std::vector<BackendSqueeze> want = act.display_backends();
    REQUIRE(!want.empty());
    REQUIRE(ract.backends.size() == want.size());
    for (size_t i = 0; i < want.size(); ++i) CHECK(ract.backends[i] == want[i]);

    CHECK(activation_deact_tick(ract) == activation_deact_tick(act));

    // deact_tick itself is stored data (blob v4), not something the reader
    // rederives -- so the round trip has to hand back the exact tick the
    // engine stamped, not just an equivalent one.
    REQUIRE(act.deact_tick().has_value());
    CHECK(ract.deact_tick() == act.deact_tick());
}

// D5: a tail row is kept on the same window as every other row, measured
// from this activation's own SP end. The graph keeps 5136 (375 ms before the
// last note), but it sits 625 ms before the SP end at 5376, so the engine
// must not copy it into the activation.
TEST_CASE("SP past the last note: tail rows use the 500 ms window from the SP end") {
    Song song = build_tail_song({{0, true, false}, {768, true, false}, {1536},
                                 {2304, false, true}, {3072}, {3840}, {4608},
                                 {5136}, {5280}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);

    bool graph_kept_5136 = false;
    for (const BackendSqueeze& b : graph.tail_backends())
        if (b.timecode.ticks() == 5136) graph_kept_5136 = true;
    REQUIRE(graph_kept_5136);

    REQUIRE(act.backends.size() == 1);
    CHECK(act.backends[0].timecode.ticks() == 5280);
    // Nothing is left for the display to drop.
    CHECK(act.display_backends() == act.backends);
}

// D5 moves where tail rows are filtered, not which rows survive. The old
// rule is written out here: every graph tail note, measured from the SP end,
// kept when it is strictly less than 500 ms away.
TEST_CASE("tail rows: every corpus activation keeps exactly the rows the old display kept") {
    int tails = 0, edges = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4);
        const int64_t last_tick = song.sequence.back().timecode.ticks();
        for (const Path& p : run_search(graph, EngineOptions{DepthMode::Scores, 4})) {
            for (const Activation& act : p.all_activations()) {
                // The SP end the engine stamped on this activation.
                const std::optional<int64_t> end_tick = act.deact_tick();
                if (!end_tick) continue;
                if (*end_tick <= last_tick) {
                    // A deactivation edge: its rows were gathered inside the window.
                    ++edges;
                    for (const BackendSqueeze& b : act.backends)
                        CHECK((std::fabs(*b.offset_ms) < 500.0 || act.is_sqout_backend(b)));
                    continue;
                }
                ++tails;
                const double end_ms = song.timing().ms_index().at(*end_tick);
                std::vector<BackendSqueeze> want;
                for (const BackendSqueeze& b : graph.tail_backends()) {
                    BackendSqueeze copy = b;
                    copy.offset_ms = b.timecode.ms() - end_ms;
                    if (std::fabs(*copy.offset_ms) < 500.0) want.push_back(copy);
                }
                CHECK(act.display_backends() == want);  // what is stored and shown: unchanged
                CHECK(act.backends == want);            // and now nothing extra in memory
            }
        }
    }
    CHECK(edges > 0);
    CHECK(tails > 0);
}

// The graph claims, for every deactivation edge, exactly the chord
// core::sqout_chord names. Run before the graph calls it, this proves the
// function is the graph's old rule; after, it guards against drift.
TEST_CASE("graph: every deactivation edge claims the chord sqout_chord names") {
    int edges = 0, claimed = 0, before = 0, after = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        const ScoreGraph graph(song, 4);

        // The SP track is one chain; it starts at the first activation's node.
        const ScoreGraphNode* sp = nullptr;
        for (const ScoreGraphNode* b = graph.start(); b && !sp;
             b = b->adv_edge ? b->adv_edge->dest : nullptr)
            if (b->branch_edge) sp = b->branch_edge->dest;

        std::set<const ScoreGraphEdge*> seen;
        for (; sp; sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
            const ScoreGraphEdge* e = sp->branch_edge;
            if (!e || !seen.insert(e).second) continue;
            ++edges;
            const Timecode& end = e->dest->timecode;
            const SongTimestamp* c = core::sqout_chord(song, end);
            CHECK(e->sqinout_time.has_value() == (c != nullptr));

            // The edge's other three facts, worked out here from the chord and
            // the song's own timing, not from the graph.
            REQUIRE(e->sqin_time.has_value());
            REQUIRE(e->sqout_time.has_value());
            if (!c) {
                // No chord to squeeze: both branches end at the SP end.
                CHECK(e->sqin_time->ticks() == end.ticks());
                CHECK(e->sqout_time->ticks() == end.ticks());
                CHECK(e->late_sqin_count == 0);
                continue;
            }
            if (!e->sqinout_time) continue;
            ++claimed;
            CHECK(e->sqinout_time->ticks() == c->timecode.ticks());
            CHECK(*e->sqinout_timing == c->timecode.ms() - end.ms());  // bit for bit

            // A chord moves the SqIn end one bar. A chord at or before the end
            // moves the SqOut end too; a chord after it is a late SqIn only.
            const int64_t one_bar =
                song.timing().plusmeasure(end, sp_bars_to_measures(1)).ticks();
            CHECK(one_bar > end.ticks());
            CHECK(e->sqin_time->ticks() == one_bar);
            const bool at_or_before = c->timecode.ticks() <= end.ticks();
            CHECK(e->sqout_time->ticks() == (at_or_before ? one_bar : end.ticks()));
            CHECK(e->late_sqin_count == (at_or_before ? 0 : 1));
            (at_or_before ? before : after)++;
        }
    }
    CHECK(edges > 0);
    CHECK(claimed > 0);
    CHECK(before > 0);  // both halves of the rule are exercised by the corpus
    CHECK(after > 0);
}

// run_search takes its knobs in one EngineOptions value, so no two flags can
// be swapped at a call site. Each knob must still do its own job.
TEST_CASE("run_search: EngineOptions carries each knob to the engine") {
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {1536},
                                 {2304, false, true},
                                 {3072},
                                 {3840},
                                 {4608},
                                 {5136},
                                 {5280}});
    ScoreGraph graph(song, 4);

    // The defaults are a plain best-path search: score depth 0, no limit.
    const std::vector<Path> best = run_search(graph, EngineOptions{});
    REQUIRE_FALSE(best.empty());
    REQUIRE_FALSE(best.front().activations.empty());

    // no_skips plus a hard 0 ms limit is exactly the all-0 search.
    EngineOptions allzero;
    allzero.ms_filter = 0.0;
    allzero.no_skips = true;
    allzero.hard_ms_filter = true;
    const std::vector<Path> z = run_search(graph, allzero);
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
    pinned.depth_value = 1'000'000'000;
    std::vector<int64_t> ticks;
    for (const Activation& a : best.front().activations) ticks.push_back(a.timecode.ticks());
    pinned.target_act_ticks = ticks;
    const std::vector<Path> again = run_search(graph, pinned);
    REQUIRE_FALSE(again.empty());
    CHECK(again.front().pathstring() == best.front().pathstring());
    CHECK(again.front().totalscore() == best.front().totalscore());
}

// analyze_chart used to run Clone Hero's 4 bars down its own branch, with the
// graph built a flat 4 bars tall. Every other fixed cap builds the graph only
// as tall as the song has phrases (graph_build_cap). Both give the same
// answer: a song with p phrases never holds more than p bars, so a p-bar
// ceiling clamps nothing a 4-bar ceiling would not. This pins it byte for
// byte through the store's own writer before the branches fold into one.
TEST_CASE("a 4-bar graph built at the song's phrase count stores the same paths") {
    std::vector<Song> songs;
    // Hand-built, three phrases, one of them collected mid-SP.
    songs.push_back(build_tail_song({{0, true, false},
                                     {768, true, false},
                                     {1536},
                                     {2304, false, true},
                                     {3072},
                                     {3840, true, false},
                                     {4608},
                                     {5376},
                                     {6144},
                                     {6720},
                                     {6816}}));
    for (const std::string& path : corpus::chart_paths()) {
        Song s = load_songpath(path, true, true);
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

        const store::FlatRecord a = store::flatten_record(tall);
        const store::FlatRecord b = store::flatten_record(built);
        bool same = a.structure == b.structure && a.nodes.size() == b.nodes.size();
        for (size_t i = 0; same && i < a.nodes.size(); ++i)
            same = a.nodes[i].payload == b.nodes[i].payload;
        CHECK_MESSAGE(same, "a song with " << song.sp_phrase_count() << " phrases");
        ++compared;
    }
    MESSAGE("compared " << compared << " songs with fewer than 4 phrases");
}

// ---- SP cap overfill: a phrase collected mid-SP can clamp the end ------
//
// The meter holds a fixed number of bars. Once it's full, a phrase you
// collect while SP is active can't push the end out by a plain 2 measures
// any more -- it gets capped at 2*cap measures past that phrase's own note.
// When the cap wins, the note that pinned the end (not the activation) is
// what the timing warning has to point at, so the search stamps that note's
// tick onto the activation as clamp_tick.

TEST_CASE("SP cap overfill: a mid-SP phrase that clamps records the "
          "collecting note") {
    // Cap 2 bars. Two SP phrases fill the meter before the activation at
    // tick 2304 (meter 2), whose plain end is 4 measures later at 5376.
    // The phrase collected at 3072, mid-SP, wants to push the end out by 2
    // more measures to 4608 + 1536 = ... no -- the pending end simply moves
    // to min(prev_end + 2 measures, 3072 + 2*cap measures). prev_end + 2
    // measures is 5376 + 1536 = 6912; the cap ceiling is 3072 + 4*768 =
    // 6144. The ceiling is smaller, so it wins: the end is pinned to 6144,
    // and clamp_tick records the note that pinned it, 3072.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {2304, false, true},
                                 {3072, true, false},
                                 {3840},
                                 {4608},
                                 {5376},
                                 {6000}});

    ScoreGraph graph(song, 2);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.sp_meter == 2);
    CHECK(act.timecode.ticks() == 2304);

    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == 6144);

    REQUIRE(act.clamp_tick().has_value());
    CHECK(*act.clamp_tick() == 3072);
}

TEST_CASE("SP cap overfill: a mid-SP phrase that only ties the cap does "
          "not clamp") {
    // Same cap and activation as above, but the mid-SP phrase lands at 3840
    // instead of 3072. Now both options land on the same tick: prev_end + 2
    // measures is 5376 + 1536 = 6912, and the cap ceiling is 3840 + 4*768 =
    // 6912 too. A tie means the plain extension wins, not the cap -- so this
    // is not a clamp, and clamp_tick stays unset.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {2304, false, true},
                                 {3840, true, false},
                                 {4608},
                                 {5376},
                                 {6000},
                                 {6500}});

    ScoreGraph graph(song, 2);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.sp_meter == 2);

    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == 6912);

    CHECK_FALSE(act.clamp_tick().has_value());
}

TEST_CASE("SP cap overfill: a later unclamped extension keeps the earlier "
          "clamp_tick") {
    // Same cap and activation, but SP is extended twice. The first mid-SP
    // phrase, at 3072, clamps exactly as in the first case above: the end is
    // pinned to 6144, with clamp_tick 3072. The second phrase, at 5760,
    // wants to move the end again -- the cap ceiling from THAT note would be
    // 5760 + 4*768 = 8832, but the plain +2-measure step from the current
    // end (6144 + 1536 = 7680) is smaller and wins instead. Because this
    // second extension is not itself a clamp, the note that pinned the
    // window stays the first one: clamp_tick is still 3072, even though the
    // end has moved again.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {2304, false, true},
                                 {3072, true, false},
                                 {3840},
                                 {4608},
                                 {5376},
                                 {5760, true, false},
                                 {6000},
                                 {6768},
                                 {7500}});

    ScoreGraph graph(song, 2);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.sp_meter == 2);

    // The chart ends at 7500, before the SP end at 7680, so this is the same
    // "SP outlasts the chart" case as the tests above: the tail rows are
    // synthesized against the tracked end rather than a real deactivation.
    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == 7680);

    REQUIRE(act.clamp_tick().has_value());
    CHECK(*act.clamp_tick() == 3072);
}

TEST_CASE("SP cap overfill: a second clamp in the same window replaces "
          "clamp_tick") {
    // Same cap and activation. The phrase at 3072 clamps as in the first
    // case: the end is pinned to 6144. The phrase at 3840 then clamps again:
    // the plain step from the current end is 6144 + 1536 = 7680, but the
    // cap ceiling from 3840 is 3840 + 4*768 = 6912, which is smaller. The
    // end is now pinned by the later note, so clamp_tick moves to 3840.
    Song song = build_tail_song({{0, true, false},
                                 {768, true, false},
                                 {2304, false, true},
                                 {3072, true, false},
                                 {3840, true, false},
                                 {4608},
                                 {5376},
                                 {6000},
                                 {6768},
                                 {7500}});

    ScoreGraph graph(song, 2);
    std::vector<Path> paths =
        run_search(graph, EngineOptions{});

    const Activation& act = last_act(paths);
    CHECK(act.timecode.ticks() == 2304);

    auto deact = activation_deact_tick(act);
    REQUIRE(deact.has_value());
    CHECK(*deact == 6912);

    REQUIRE(act.clamp_tick().has_value());
    CHECK(*act.clamp_tick() == 3840);
}

TEST_CASE("collected phrases: none when no phrase lands during the activation") {
    Song song = build_tail_song({{0, true, false}, {768, true, false}, {1536},
                                 {2304, false, true}, {3072}, {3840}, {4608},
                                 {5136}, {5280}});
    ScoreGraph graph(song, 4);
    // The paths are held in a named vector: last_act returns a reference into
    // it, so it has to outlive the checks below.
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);
    CHECK(act.collected_phrase_ticks().empty());
    CHECK_FALSE(act.sqout_tick.has_value());
}

TEST_CASE("collected phrases: one phrase mid-activation is recorded") {
    // The fixture of "SP past the last note: a mid-activation phrase extends
    // the end": the phrase ending at 3840 is collected while SP is active.
    Song song = build_tail_song({{0, true, false}, {768, true, false}, {1536},
                                 {2304, false, true}, {3072}, {3840, true, false},
                                 {4608}, {5376}, {6144}, {6720}, {6816}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);
    CHECK(act.collected_phrase_ticks() == std::vector<int64_t>{3840});
}

TEST_CASE("collected phrases: two phrases under a full meter are both recorded, in order") {
    // The cap-2 clamp fixture: both phrases are collected while active, and
    // the second is the note the cap pinned the end to. A clamped phrase
    // counts as collected.
    Song song = build_tail_song({{0, true, false}, {768, true, false},
                                 {2304, false, true}, {3072, true, false},
                                 {3840, true, false}, {4608}, {5376}, {6000},
                                 {6768}, {7500}});
    ScoreGraph graph(song, 2);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Activation& act = paths.front().activations.front();
    CHECK(act.collected_phrase_ticks() == std::vector<int64_t>{3072, 3840});
    REQUIRE(act.clamp_tick().has_value());
    CHECK(act.collected_phrase_ticks().back() == *act.clamp_tick());
}

TEST_CASE("collected phrases: the corpus agrees with the squeezes and the SP end") {
    int sqouts_seen = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4);
        for (const Path& p : run_search(graph, EngineOptions{DepthMode::Scores, 1})) {
            for (const Activation& act : p.all_activations()) {
                bool took_sqout = false;
                for (const SPSqueeze& sq : act.sqinouts)
                    if (sq.kind == SqueezeKind::SqOut) took_sqout = true;
                // sqout_tick is set exactly when the activation squeezed out.
                CHECK(act.sqout_tick.has_value() == took_sqout);
                if (act.sqout_tick) ++sqouts_seen;

                int64_t prev = -1;
                for (int64_t t : act.collected_phrase_ticks()) {
                    CHECK(t > prev);  // strictly ascending
                    CHECK(t >= act.timecode.ticks());
                    // A squeezed-out phrase, and anything after it, was
                    // never collected.
                    if (act.sqout_tick) CHECK(t < *act.sqout_tick);
                    prev = t;
                }
            }
        }
    }
    CHECK(sqouts_seen > 0);
}

TEST_CASE("SP end history: a late squeeze-in is a SqIn step on its phrase") {
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {5760, 13440, SpEndKind::Activation}, {13920, 17280, SpEndKind::SqIn}}));
    // The bar arrives at the old end: the player hits the phrase early.
    CHECK(act.refill_tick(1) == 13440);
    CHECK(act.deact_tick() == std::optional<int64_t>(17280));
    CHECK(act.nominal_end() == std::optional<int64_t>(13440));
    CHECK((act.collected_phrase_ticks() == std::vector<int64_t>{13920}));
    REQUIRE(act.sqinouts.size() == 1);
    CHECK(act.sqinouts[0].offset_ms == doctest::Approx(250.0));
    CHECK(act.squeeze_end_tick(0) == std::optional<int64_t>(13440));
}

TEST_CASE("SP end history: an early squeeze-in measures from the end before it") {
    // Part A's A2 case. The SqIn phrase at 5280 sits 250 ms before X = 5376.
    Song song = test::sqin_then_collect_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const Activation* act = test::find_act(paths, [](const Activation& a) {
        for (const SPSqueeze& s : a.sqinouts)
            if (s.kind == SqueezeKind::SqIn) return true;
        return false;
    });
    REQUIRE(act != nullptr);
    CHECK((act->sp_end_steps == std::vector<SpEndStep>{
               {2304, 5376, SpEndKind::Activation},
               {5280, 6912, SpEndKind::SqIn},
               {6144, 8448, SpEndKind::Collected}}));
    CHECK(act->deact_tick() == std::optional<int64_t>(8448));
    REQUIRE(act->sqinouts.front().kind == SqueezeKind::SqIn);
    CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(5376));  // X, not 8448 - 2 measures
    REQUIRE(act->squeeze_end_step(0).has_value());
    CHECK(act->end_anchor_tick(*act->squeeze_end_step(0)) == 2304);   // no clamp: the activation
    CHECK(act->sqinouts.front().offset_ms == doctest::Approx(-250.0).epsilon(1e-9));
}

TEST_CASE("SP end history: a squeeze-out measures from the deact node") {
    Song song = test::sqin_then_collect_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const Activation* act = test::find_act(paths, [](const Activation& a) {
        for (const SPSqueeze& s : a.sqinouts)
            if (s.kind == SqueezeKind::SqOut) return true;
        return false;
    });
    REQUIRE(act != nullptr);
    for (size_t i = 0; i < act->sqinouts.size(); ++i) {
        if (act->sqinouts[i].kind != SqueezeKind::SqOut) continue;
        CHECK(act->squeeze_end_tick(i) == act->deact_tick());
        CHECK(act->end_anchor_tick(*act->squeeze_end_step(i)) ==
              act->clamp_tick().value_or(act->timecode.ticks()));
    }
}

TEST_CASE("SP end history: clamps are steps, and the anchor follows them") {
    Song song = build_tail_song({{0, true, false}, {768, true, false},
                                 {2304, false, true}, {3072, true, false},
                                 {3840, true, false}, {4608}, {5376}, {6000},
                                 {6768}, {7500}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Activation& act = paths.front().activations.front();
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {2304, 5376, SpEndKind::Activation},
               {3072, 6144, SpEndKind::Clamped},
               {3840, 6912, SpEndKind::Clamped}}));
    CHECK(act.clamp_tick() == std::optional<int64_t>(3840));
    CHECK(act.end_anchor_tick(0) == 2304);
    CHECK(act.end_anchor_tick(1) == 3072);
    CHECK(act.end_anchor_tick(2) == 3840);
}

TEST_CASE("search: scales anchor on the collecting note and the SqIn's own end") {
    {
        Song song = test::clamp_song();
        ScoreGraph graph(song, 2);
        std::vector<Path> paths = run_search(graph, test::wide_search());
        const Activation* act = test::find_act(
            paths, [](const Activation& a) { return a.timecode.ticks() == 2304; });
        REQUIRE(act != nullptr);
        CHECK(act->clamp_tick() == std::optional<int64_t>(3072));
        CHECK(act->deact_tick() == std::optional<int64_t>(6144));
        // 3072 and 6144 are both in the 60 BPM section: x1.00, not x2.00.
        REQUIRE(act->transfer_post.has_value());
        CHECK(act->transfer_post->late == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(act->transfer_post->early == doctest::Approx(1.0).epsilon(1e-12));
    }
    {
        Song song = test::sqin_then_collect_song();
        ScoreGraph graph(song, 4);
        std::vector<Path> paths = run_search(graph, test::wide_search());
        const Activation* act = test::find_act(paths, [](const Activation& a) {
            return !a.sqinouts.empty() && a.sqinouts.front().kind == SqueezeKind::SqIn;
        });
        REQUIRE(act != nullptr);
        CHECK(act->deact_tick() == std::optional<int64_t>(8448));
        CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(5376));   // X
        CHECK(act->squeeze_anchor_tick(0) == std::optional<int64_t>(2304));
        // 2304 -> 5376, both 120 BPM.
        REQUIRE(act->sqinouts.front().transfer.has_value());
        CHECK(act->sqinouts.front().transfer->late == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(act->sqinouts.front().transfer->early == doctest::Approx(1.0).epsilon(1e-12));
        // 2304 (120) -> 8448 (60): measures last twice as long at D.
        REQUIRE(act->transfer_post.has_value());
        CHECK(act->transfer_post->late == doctest::Approx(2.0).epsilon(1e-12));
        // A SqOut stores no scale of its own: it reads transfer_post.
        const Activation* out = test::find_act(paths, [](const Activation& a) {
            for (const SPSqueeze& s : a.sqinouts)
                if (s.kind == SqueezeKind::SqOut) return true;
            return false;
        });
        REQUIRE(out != nullptr);
        for (const SPSqueeze& s : out->sqinouts) {
            if (s.kind != SqueezeKind::SqOut) continue;
            CHECK_FALSE(s.transfer.has_value());
        }
    }
}

TEST_CASE("SP end history: a squeezed-out phrase leaves no step") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    // The best path collects 12960 instead and keeps SP running over the
    // second fill, so search wide and pick the activation that squeezed out.
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    const Activation* found = test::find_act(
        paths, [](const Activation& a) { return a.sqout_tick.has_value(); });
    REQUIRE(found != nullptr);
    const Activation& first = *found;
    CHECK(first.timecode.ticks() == 5760);
    CHECK(first.sqout_tick == std::optional<int64_t>(12960));
    CHECK((first.sp_end_steps ==
           std::vector<SpEndStep>{{5760, 13440, SpEndKind::Activation}}));
    CHECK(first.collected_phrase_ticks().empty());
}

// The lasting check on every corpus activation's history (R1, D4).
TEST_CASE("SP end history: every corpus activation is consistent") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int acts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song =
            corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            for (const Activation& act : p->walk_activations()) {
                CAPTURE(chart);
                CAPTURE(p->pathstring());
                CAPTURE(act.timecode.ticks());
                ++acts;
                // D4's condition: no fresh record leaves the history out.
                REQUIRE_FALSE(act.sp_end_steps.empty());
                CHECK(act.sp_end_steps.front().kind == SpEndKind::Activation);
                CHECK(act.sp_end_steps.front().tick == act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k) {
                    const SpEndStep& prev = act.sp_end_steps[k - 1];
                    const SpEndStep& st = act.sp_end_steps[k];
                    CHECK(st.kind != SpEndKind::Activation);
                    CHECK(st.tick > prev.tick);
                    // SP never runs dry inside a window: every step takes
                    // effect at or before the end in force.
                    CHECK(act.refill_tick(k) <= prev.end_tick);
                    CHECK(act.refill_tick(k) <= st.tick);
                }
                CHECK(act.nominal_end() ==
                      song.timing().plusmeasure(act.timecode, sp_bars_to_measures(act.sp_meter)).ticks());
                // Appendix B's first guarantee: one SqIn step per SqIn, so
                // a relabel that found nothing cannot pass silently.
                size_t sqins = 0, sqin_steps = 0;
                for (const SPSqueeze& s : act.sqinouts)
                    if (s.kind == SqueezeKind::SqIn) ++sqins;
                for (const SpEndStep& s : act.sp_end_steps)
                    if (s.kind == SpEndKind::SqIn) ++sqin_steps;
                CHECK(sqin_steps == sqins);
            }
        }
    }
    CHECK(acts > 1000);
}

TEST_CASE("path codec: encode/decode a path node keeps clamp_tick()") {
    // A plain node round trip has to carry the clamp, which the history holds.
    Activation act;
    act.timecode = Timecode::raw(2304);
    test::set_clamped_window(act, 3072, 6144);

    Path path;
    path.activations.push_back(act);

    Path decoded = store::decode_path_node(store::encode_path_node(path));
    REQUIRE(decoded.activations.size() == 1);
    CHECK(decoded.activations.front().clamp_tick() == std::optional<int64_t>(3072));
}

TEST_CASE("path codec: each SqIn keeps its own scale; a SqOut stores none") {
    Activation act;
    act.timecode = Timecode::raw(2304);
    act.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -50.0}, SPSqueeze{SqueezeKind::SqIn, 20.0},
                    SPSqueeze{SqueezeKind::SqOut, -30.0}};
    test::set_sqin_transfers(act, {TransferScale{0.97, 1.5}, TransferScale{0.95, 2.5}},
                             TransferScale{1.25, 0.8});
    // A SqOut's own field is never written, so a value left on it is dropped.
    act.sqinouts[2].transfer = TransferScale{9.0, 9.0};

    Path path;
    path.activations.push_back(act);
    Path decoded = store::decode_path_node(store::encode_path_node(path));
    REQUIRE(decoded.activations.size() == 1);
    const Activation& got = decoded.activations.front();
    REQUIRE(got.sqinouts.size() == 3);
    REQUIRE(got.sqinouts[0].transfer.has_value());
    REQUIRE(got.sqinouts[1].transfer.has_value());
    CHECK(got.sqinouts[0].transfer->early == 0.97);
    CHECK(got.sqinouts[0].transfer->late == 1.5);
    CHECK(got.sqinouts[1].transfer->early == 0.95);
    CHECK(got.sqinouts[1].transfer->late == 2.5);
    CHECK_FALSE(got.sqinouts[2].transfer.has_value());
    REQUIRE(got.transfer_post.has_value());
    CHECK(got.transfer_post->early == 1.25);
    CHECK(got.transfer_post->late == 0.8);
}

TEST_CASE("graph_build_cap: never taller than the song's phrases, never below one") {
    CHECK(graph_build_cap(4, 10) == 4);   // the cap binds
    CHECK(graph_build_cap(32, 3) == 3);   // the song's phrases bind
    CHECK(graph_build_cap(8, 0) == 1);    // a phraseless song still builds one level
}
