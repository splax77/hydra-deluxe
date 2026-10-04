// Invariant tests for search/ (ScoreGraph + engine + pather).
//
// The engine's best score must be stable across the config knobs that are not
// supposed to change it (search depth, the ms filter), every reported path
// must be internally consistent, and the all-0 pass must honor its contract.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "bank_check.h"
#include "core/model.h"
#include "core/replay.h"
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
                if (const std::string why = corpus::unknown_scale_reason(act); !why.empty()) {
                    d = why;
                    break;
                }
                // Non-flat counts the SP end's scale and every SqIn's scale.
                // unknown_scale_reason above proved each one known.
                auto flat = [](const TransferScale& s) {
                    return s.early == 1.0 && s.late == 1.0;
                };
                bool any_scaled = !flat(*act.transfer_post);
                for (const SPSqueeze& sq : act.sqinouts)
                    if (sq.kind == SqueezeKind::SqIn && !flat(*sq.transfer)) any_scaled = true;
                if (any_scaled) ++nonflat;

                auto scales = frontend_transfer_scales(act, song.timing());
                if (!scales) {
                    d = "display recomputation returned no scales";
                    break;
                }
                auto differs = [](const TransferScale& a, const TransferScale& b) {
                    return std::abs(a.early - b.early) > 1e-9 ||
                           std::abs(a.late - b.late) > 1e-9;
                };
                if (differs(scales->post, *act.transfer_post)) {
                    d = "stored scales diverge from recomputation";
                    break;
                }
                // A non-positive scale is caught by unknown_scale_reason, above.
                // Each SqIn's stored scale is the j-th recomputed one.
                size_t j = 0;
                for (const SPSqueeze& sq : act.sqinouts) {
                    if (sq.kind != SqueezeKind::SqIn) continue;
                    if (j >= scales->sqins.size() || differs(scales->sqins[j], *sq.transfer)) {
                        d = "a SqIn's stored scale diverges from recomputation";
                        break;
                    }
                    ++j;
                }
                if (!d.empty()) break;
                // No count check: the recomputation walks this same
                // activation's SqIns, so both sides always have j of them.

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

// The audit's constructed chart for finding 90 ("midsp"), 120 BPM 4/4, run at
// cap 2. '1' activates at 10752; '0 1' activates at 4608, then at 11520. The
// phrase at 11904 clamps both windows to end at 14976, on the same SP node
// with the same score, so the search folds '0 1' into '1' there. The phrase
// at 13056 then clamps the window again, to 16128: after the fold.
//
// with_squeeze adds four notes. The phrase at 11328 lands in '1''s SP but
// before '0 1' activates, so only the leader collects it; the note at 6144
// keeps the two tied. The note at 16032 and the phrase at 16224 sit 250 ms
// either side of the SP end at 16128, so the leader splits there into '1+'
// (SqIn) and '1-' (SqOut), and the variant hangs off both.
std::vector<TailNote> midsp_notes(bool with_squeeze) {
    std::vector<TailNote> n;
    for (int64_t t = 0; t < 3072; t += 96) n.push_back({t, t == 768 || t == 2304, false});
    n.push_back({4608, false, true});
    if (with_squeeze) n.push_back({6144});
    n.push_back({8448, true, false});
    n.push_back({9216, true, false});
    n.push_back({10752, false, true});
    if (with_squeeze) n.push_back({11328, true, false});
    n.push_back({11520, false, true});
    n.push_back({11904, true, false});
    n.push_back({12672, false, true});
    n.push_back({13056, true, false});
    n.push_back({15360});
    if (with_squeeze) {
        n.push_back({16032});
        n.push_back({16224, true, false});
    }
    n.push_back({16896});
    n.push_back({17664});
    n.push_back({18432});
    return n;
}

void collect_paths(const Path& p, std::vector<const Path*>& out) {
    out.push_back(&p);
    for (const Path& v : p.variants) collect_paths(v, out);
}
std::vector<const Path*> every_path(const std::vector<Path>& roots) {
    std::vector<const Path*> out;
    for (const Path& r : roots) collect_paths(r, out);
    return out;
}
const Path* root_named(const std::vector<Path>& roots, const std::string& s) {
    for (const Path& r : roots)
        if (r.pathstring() == s) return &r;
    return nullptr;
}
// The leader's variant whose own last activation is at `tick`.
const Path* variant_at(const Path& leader, int64_t tick) {
    for (const Path& v : leader.variants)
        if (!v.activations.empty() && v.activations.back().timecode.ticks() == tick)
            return &v;
    return nullptr;
}
using Step = std::tuple<int64_t, int64_t, SpEndKind>;
std::vector<Step> steps_of(const Activation& a) {
    std::vector<Step> out;
    for (const SpEndStep& s : a.sp_end_steps) out.emplace_back(s.tick, s.end_tick, s.kind);
    return out;
}

// Every window of a path written out: activation, SP end steps, squeezes,
// squeezed-out note and backend rows. Two paths with the same text store the
// same windows.
std::string windows_text(const Path& p) {
    std::ostringstream o;
    for (const Activation& a : p.walk_activations()) {
        o << a.timecode.ticks() << " [steps";
        for (const SpEndStep& s : a.sp_end_steps)
            o << ' ' << s.tick << '>' << s.end_tick << ':' << static_cast<int>(s.kind);
        o << " | sq";
        for (const SPSqueeze& q : a.sqinouts) o << ' ' << q.symbol() << q.offset_ms;
        o << " | out " << a.sqout_tick.value_or(-1) << " | backends";
        for (const BackendSqueeze& b : a.backends)
            o << ' ' << b.timecode.ticks() << '/' << b.points << '/' << b.sqout_points << '/'
              << (b.offset_ms ? *b.offset_ms : -1.0);
        o << "] ";
    }
    return o.str();
}

// D3's promise for a tied variant: it stores what the search stores when it
// prices that path's activations alone (search_target). Returns "" when one
// lone path has the variant's total and windows, else what differed.
std::string lone_pricing_mismatch(const Song& song, const SearchSettings& settings,
                                  const Path& variant) {
    std::vector<int64_t> ticks;
    for (const Activation& a : variant.walk_activations()) ticks.push_back(a.timecode.ticks());
    const std::string mine = windows_text(variant);
    std::string lone_same_total;
    for (const Path& t : search_target(song, settings, ticks)) {
        if (t.totalscore() != variant.totalscore()) continue;
        const std::string theirs = windows_text(t);
        if (theirs == mine) return "";
        lone_same_total += "\n  lone:    " + theirs;
    }
    return "'" + variant.pathstring() + "' " + std::to_string(variant.totalscore()) +
           "\n  variant: " + mine + (lone_same_total.empty() ? "\n  no lone path ties it" : lone_same_total);
}

void collect_variants(const Path& p, std::vector<const Path*>& out) {
    for (const Path& v : p.variants) {
        out.push_back(&v);
        collect_variants(v, out);
    }
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
    CHECK(act.sp_meter() == 2);
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
    CHECK(act.sp_meter() == 2);

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

// The graph lists, for every deactivation edge, exactly the phrase chords
// core::squeeze_window_phrases names, in chart order, so the engine can offer
// the first one a window can still squeeze (D34). This guards against drift.
TEST_CASE("graph: every deactivation edge lists the chords squeeze_window_phrases names") {
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
            const std::vector<const SongTimestamp*> window =
                core::squeeze_window_phrases(song, end);
            REQUIRE(e->squeeze_choices.size() == window.size());

            // The edge's other facts, worked out here from each chord and the
            // song's own timing, not from the graph.
            if (window.empty()) continue;  // no chord: the path just ends here
            ++claimed;
            // A chord moves the SqIn end one bar, or to the cap's ceiling (4
            // bars, 8 measures past the chord) when that is earlier (finding
            // 37). A chord at or before the end moves the SqOut end too; a
            // chord after it is a late SqIn only.
            const int64_t one_bar =
                song.timing().plusmeasure(end, sp_bars_to_measures(1)).ticks();
            CHECK(one_bar > end.ticks());
            for (size_t k = 0; k < window.size(); ++k) {
                const SongTimestamp* c = window[k];
                const SqueezeChoice& got = e->squeeze_choices[k];
                const int64_t ceiling =
                    song.timing().plusmeasure(c->timecode, sp_bars_to_measures(4)).ticks();
                const bool clamped = ceiling < one_bar;
                const int64_t moved = clamped ? ceiling : one_bar;
                CHECK(got.chord.ticks() == c->timecode.ticks());
                CHECK(got.timing == c->timecode.ms() - end.ms());  // bit for bit
                const bool at_or_before = c->timecode.ticks() <= end.ticks();
                CHECK(got.sqin_time.ticks() == moved);
                CHECK(got.clamped == clamped);
                CHECK(got.sqout_time.ticks() == (at_or_before ? moved : end.ticks()));
                CHECK(got.late == !at_or_before);
                (at_or_before ? before : after)++;
            }
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
    CHECK(act.sp_meter() == 2);
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
    CHECK(act.sp_meter() == 2);

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
    CHECK(act.sp_meter() == 2);

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
    // The early squeeze-out song: the fill at 5760 activates, SP would end at
    // 13440, and the phrase at 12960 is squeezed out. The history is trimmed
    // back to the activation step, so the end a SqOut is measured from is
    // that step's 13440 (not the 14400 phrase's extension), anchored on the
    // activation.
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    const Activation* act = test::find_act(
        paths, [](const Activation& a) { return a.sqout_tick.has_value(); });
    REQUIRE(act != nullptr);
    CHECK((act->sp_end_steps ==
           std::vector<SpEndStep>{{5760, 13440, SpEndKind::Activation}}));
    CHECK(act->deact_tick() == std::optional<int64_t>(13440));
    size_t sqouts = 0;
    for (size_t i = 0; i < act->sqinouts.size(); ++i) {
        if (act->sqinouts[i].kind != SqueezeKind::SqOut) continue;
        ++sqouts;
        CHECK(act->squeeze_end_tick(i) == std::optional<int64_t>(13440));
        REQUIRE(act->squeeze_end_step(i).has_value());
        CHECK(act->end_anchor_tick(*act->squeeze_end_step(i)) == 5760);
    }
    CHECK(sqouts == 1);
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

TEST_CASE("search: no fresh record stores an unknown transfer scale (constructed songs)") {
    struct Case {
        const char* name;
        Song song;
        int cap;
    };
    std::vector<Case> cases;
    cases.push_back({"SP past the last note",
                     test::build_tail_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                            {3072}, {3840}, {4608}, {5136}, {5280}}),
                     4});
    cases.push_back({"mid-SP phrase, SP past the end",
                     test::build_tail_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                            {3072}, {3840, true}, {4608}, {5376}, {6144},
                                            {6720}, {6816}}),
                     4});
    cases.push_back({"SqIn then a collection", test::sqin_then_collect_song(), 4});
    cases.push_back({"cap clamp", test::clamp_song(), 2});
    cases.push_back({"tempo change on the activation tick",
                     test::build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                             {3072}, {3840}, {4608}, {5376}, {6144}},
                                            {{0, 98.0}, {2304, 97.5}, {5376, 110.0}}),
                     4});

    int acts = 0, sqins = 0, sqouts = 0, clamps = 0;
    for (const Case& c : cases) {
        ScoreGraph graph(c.song, c.cap);
        for (const Path& p : run_search(graph, test::wide_search())) {
            for (const Activation& a : p.all_activations()) {
                ++acts;
                if (a.clamp_tick()) ++clamps;
                for (const SPSqueeze& s : a.sqinouts)
                    (s.kind == SqueezeKind::SqIn ? sqins : sqouts) += 1;
                const std::string why = corpus::unknown_scale_reason(a);
                CHECK_MESSAGE(why.empty(), c.name << ": activation at tick "
                                                  << a.timecode.ticks() << ": " << why);
            }
        }
    }
    // The cases really cover the shapes they claim.
    CHECK(acts > 0);
    CHECK(sqins > 0);
    CHECK(sqouts > 0);
    CHECK(clamps > 0);
    MESSAGE("checked " << acts << " activations (" << sqins << " SqIns, " << sqouts
                       << " SqOuts, " << clamps << " clamps)");
}

// The search ranks a squeeze-out by its own offset (its squeeze choice's
// timing on the deact edge); the record stores the row's. rebuild throws if the two
// differ, so run_search failing here is that check biting. The stored offset
// is then measured again, independently, from the deactivation node D.
TEST_CASE("squeeze-out: one SqOut, last, and its offset is the search's and D's, on every corpus path") {
    int sqouts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4);
        CAPTURE(path);
        std::vector<Path> paths;
        REQUIRE_NOTHROW(paths = run_search(graph, EngineOptions{DepthMode::Scores, 4}));
        for (const Path& p : paths) {
            for (const Activation& act : p.all_activations()) {
                int n = 0;
                for (size_t i = 0; i < act.sqinouts.size(); ++i) {
                    if (act.sqinouts[i].kind != SqueezeKind::SqOut) continue;
                    ++n;
                    CHECK(i + 1 == act.sqinouts.size());  // always the last squeeze
                }
                CHECK(n <= 1);
                CHECK(act.sqout_tick.has_value() == (n == 1));
                if (n != 1 || !act.sqout_tick) continue;
                ++sqouts;
                const BackendSqueeze* row = nullptr;
                for (const BackendSqueeze& b : act.backends)
                    if (b.timecode.ticks() == *act.sqout_tick) row = &b;
                REQUIRE(row != nullptr);
                REQUIRE(row->offset_ms.has_value());
                CHECK(*row->offset_ms == act.sqinouts.back().offset_ms);  // exact
                // Measured again from D, the SP end every row is measured
                // against, not read back off the row.
                const std::optional<int64_t> d = act.deact_tick();
                REQUIRE(d.has_value());
                const double from_d = offset_from_sp_end(
                    row->timecode.ms(), song.timing().timecode(*d).ms());
                CHECK(*row->offset_ms == from_d);  // exact
            }
        }
    }
    CHECK(sqouts > 0);
    MESSAGE("checked " << sqouts << " squeeze-outs");
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

// A squeeze window can reach back past the activation when 500 ms spans more
// than one SP bar. The phrases there were banked before SP started, so the
// activation cannot squeeze them in or out (and before the rule, the SqIn
// branch found no step to relabel and broke the search).
TEST_CASE("SP end history: no activation squeezes a phrase it banked before it started") {
    Song song = test::banked_phrase_window_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths;
    REQUIRE_NOTHROW(paths = run_search(graph, test::wide_search()));
    int acts = 0;
    for (const Path& p : paths) {
        for (const Activation& act : p.walk_activations()) {
            CAPTURE(p.pathstring());
            ++acts;
            CHECK(act.timecode.ticks() == 2304);
            CHECK_FALSE(act.sqout_tick.has_value());
            CHECK(act.sqinouts.empty());
            // D4: the history is never empty, and SP ends where the banked
            // bars put it.
            CHECK((act.sp_end_steps ==
                   std::vector<SpEndStep>{{2304, 6912, SpEndKind::Activation}}));
            CHECK(act.deact_tick() == std::optional<int64_t>(6912));
        }
    }
    CHECK(acts > 0);
}

namespace {
// Every path and nested variant, each before its variants.
void collect_paths(const std::vector<Path>& in, std::vector<const Path*>& out) {
    for (const Path& p : in) {
        out.push_back(&p);
        collect_paths(p.variants, out);
    }
}
}  // namespace

// A path that banked the phrase may not be folded into one that collected it:
// after the fold the variant would take the leader's squeeze-out, a choice it
// never had, and store the leader's score under its own windows.
TEST_CASE("SP end history: a variant never takes a squeeze on a phrase it banked") {
    Song song = test::banked_phrase_fold_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths;
    REQUIRE_NOTHROW(paths = run_search(graph, test::wide_search()));
    std::vector<const Path*> all;
    collect_paths(paths, all);
    // Same activations, same SP ends, same squeezes: same score.
    std::map<std::string, int64_t> by_windows;
    bool saw_late_variant = false;
    for (const Path* p : all) {
        std::string windows;
        for (const Activation& act : p->walk_activations()) {
            CAPTURE(p->pathstring());
            REQUIRE_FALSE(act.sp_end_steps.empty());
            if (act.sqout_tick) CHECK(*act.sqout_tick > act.timecode.ticks());
            windows += std::to_string(act.timecode.ticks()) + ":" +
                       std::to_string(*act.deact_tick());
            for (const SPSqueeze& s : act.sqinouts)
                windows += s.kind == SqueezeKind::SqOut ? "-" : "+";
            windows += " ";
            if (act.timecode.ticks() == 13824 && p->walk_activations().size() == 2) {
                saw_late_variant = true;
                CHECK(act.sqinouts.empty());
                CHECK(act.deact_tick() == std::optional<int64_t>(16896));
            }
        }
        CAPTURE(windows);
        auto [it, inserted] = by_windows.emplace(windows, p->totalscore());
        if (!inserted) CHECK(it->second == p->totalscore());
    }
    CHECK(saw_late_variant);
}

// Two paths with the same SP meter are not interchangeable when an upcoming
// fill spawns for one and refuses the other (Clone Hero's early-fill rule:
// SP must be ready by the fill's deadline). The later-ready path leads on
// score here; it may not knock out the earlier-ready one, whose activation at
// that fill is the best path.
TEST_CASE("search: a later-ready path never knocks out one an upcoming fill still spawns for") {
    Song song = test::ready_time_fold_song();
    ScoreGraph graph(song, 2);
    std::vector<Path> best;
    REQUIRE_NOTHROW(best = run_search(graph, EngineOptions{}));
    REQUIRE_FALSE(best.empty());

    EngineOptions target;
    target.target_act_ticks = std::vector<int64_t>{7680};
    std::vector<Path> late;
    REQUIRE_NOTHROW(late = run_search(graph, target));
    REQUIRE_FALSE(late.empty());

    // A, which activated at 2304, cannot spawn the fill at 7680.
    EngineOptions a_then_late;
    a_then_late.target_act_ticks = std::vector<int64_t>{2304, 7680};
    CHECK_THROWS(run_search(graph, a_then_late));

    std::vector<int64_t> ticks;
    for (const Activation& act : best.front().walk_activations())
        ticks.push_back(act.timecode.ticks());
    CAPTURE(best.front().pathstring());
    CHECK(ticks == std::vector<int64_t>{7680});
    CHECK(best.front().totalscore() == late.front().totalscore());
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
                // A phrase at or before the activation chord was banked
                // before SP started: no activation squeezes it out.
                if (act.sqout_tick) CHECK(*act.sqout_tick > act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k) {
                    const SpEndStep& prev = act.sp_end_steps[k - 1];
                    const SpEndStep& st = act.sp_end_steps[k];
                    CHECK(st.kind != SpEndKind::Activation);
                    CHECK(st.tick > prev.tick);
                    // SP never runs dry inside a window: every step takes
                    // effect at or before the end in force.
                    CHECK(act.refill_tick(k) <= prev.end_tick);
                    CHECK(act.refill_tick(k) >= prev.tick);
                }
                CHECK(act.nominal_end() ==
                      song.timing().plusmeasure(act.timecode, sp_bars_to_measures(act.sp_meter())).ticks());
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
    act.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -50.0}, SPSqueeze{SqueezeKind::SqIn, 20.0}};
    test::set_sqin_transfers(act, {TransferScale{0.97, 1.5}, TransferScale{0.95, 2.5}},
                             TransferScale{1.25, 0.8});
    // The squeeze-out goes in through its one writer, from its row.
    BackendSqueeze sqout_row;
    sqout_row.timecode = Timecode::raw(6000);
    sqout_row.offset_ms = -30.0;
    act.backends.push_back(sqout_row);
    act.set_sqout(6000);
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

TEST_CASE("Bank: a squeezed-out bar arrives at the deact node") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    // The best path collects 12960 instead and keeps SP running over the
    // second fill, so search wide and pick the path that squeezed out.
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    const Path* found = nullptr;
    for (const Path& p : paths)
        if (!p.activations.empty() && p.activations.front().sqout_tick) found = &p;
    REQUIRE(found != nullptr);
    const Path& path = *found;
    REQUIRE(path.activations.size() == 2);
    CHECK((path.activations[0].bank_rise_ticks == std::vector<int64_t>{480, 1920}));
    // The phrase at 12960 was hit late, just after SP ended at 13440: its bar
    // arrives there. Then the phrase at 14400.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
    CHECK(path.trailing_bank_ticks.empty());
}

// The lasting order checks (tests/bank_check.h), on every path, tied variants
// included (D3, finding 89). A variant's walk is its own windows, then its
// leader's from the fold on, so its banked bars must still fall between its
// own windows. The fixture case above pins the exact ticks.
TEST_CASE("Bank: every corpus path banks in order") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int acts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song =
            corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        const std::set<int64_t> phrase_ends = bank_check::phrase_ends(song);
        const int64_t chart_end = song.sequence.back().timecode.ticks();
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            acts += bank_check::check_path_banks(*p, phrase_ends, chart_end);
        }
    }
    CHECK(acts > 1000);
}

TEST_CASE("Skipped fills: the 1.0 rule's offered fill is the one stored") {
    // Fill A (19200) was shown and passed over; fill B (24960) has the earlier
    // 1.0 deadline and never spawned. The nearest-fill guess would name B.
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.skipped_fill_ticks == std::vector<int64_t>{19200}));
}

// The lasting order checks, on root paths. A tied variant still carries its
// leader's list until finding 97 gets its own plan (Q3).
TEST_CASE("Skipped fills: every corpus root passes over real fills in order") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int skipped = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song =
            corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        for (const Path& root : corpus::analyzed(chart, cfg).paths) {
            const Activation* prev = nullptr;
            for (const Activation& act : root.walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                for (size_t k = 0; k < act.skipped_fill_ticks.size(); ++k) {
                    const int64_t t = act.skipped_fill_ticks[k];
                    ++skipped;
                    CHECK(t < act.timecode.ticks());
                    if (prev) CHECK(t > prev->timecode.ticks());
                    if (k > 0) CHECK(t > act.skipped_fill_ticks[k - 1]);
                    CHECK(std::any_of(song.sequence.begin(), song.sequence.end(),
                                      [t](const SongTimestamp& ts) {
                                          return ts.timecode.ticks() == t &&
                                                 ts.activation_length.has_value();
                                      }));
                }
                prev = &act;
            }
        }
    }
    CHECK(skipped > 100);
}

// ---- Tied variants keep their own state (decision D3) ------------------

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's steps after the fold") {
    const Song song = build_tail_song(midsp_notes(false));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* leader = root_named(roots, "1");
    REQUIRE(leader != nullptr);
    const Path* variant = variant_at(*leader, 11520);
    REQUIRE(variant != nullptr);
    CHECK(variant->totalscore() == leader->totalscore());

    const Activation& mine = variant->activations.back();
    const Activation& lead = leader->activations.back();
    CHECK(steps_of(lead) == std::vector<Step>{{10752, 13824, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    // Its own activation step and its own clamp at the fold, then the
    // leader's clamp at 13056. Today the list stops at the fold: 14976.
    CHECK(steps_of(mine) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    CHECK(mine.deact_tick() == std::optional<int64_t>(16128));
    CHECK(mine.clamp_tick() == std::optional<int64_t>(13056));
    CHECK(mine.collected_phrase_ticks() == std::vector<int64_t>{11904, 13056});
    // The real deactivation's rows, not the song's last notes.
    CHECK(mine.backends == lead.backends);
    CHECK(mine.sqinouts.empty());
    CHECK(variant->pathstring() == "0 1");

    for (const Path* p : every_path(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's closing SqIn or SqOut") {
    const Song song = build_tail_song(midsp_notes(true));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* in_lead = root_named(roots, "1+");
    const Path* out_lead = root_named(roots, "1-");
    REQUIRE(in_lead != nullptr);
    REQUIRE(out_lead != nullptr);
    const Path* in_var = variant_at(*in_lead, 11520);
    const Path* out_var = variant_at(*out_lead, 11520);
    REQUIRE(in_var != nullptr);
    REQUIRE(out_var != nullptr);

    // The leader collected 11328 in SP. The variant did not: it banked that
    // phrase before activating at 11520, so the step is the leader's alone.
    CHECK(in_lead->activations.back().collected_phrase_ticks() ==
          std::vector<int64_t>{11328, 11904, 13056, 16224});

    // The SqIn side: a late SqIn on the phrase at 16224 moves the end to 17664.
    const Activation& a = in_var->activations.back();
    CHECK(steps_of(a) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped},
                                           {16224, 17664, SpEndKind::SqIn}});
    REQUIRE(a.sqinouts.size() == 1);
    CHECK(a.sqinouts[0].kind == SqueezeKind::SqIn);
    CHECK(in_var->pathstring() == "0 1+");

    // The SqOut side: SP ends at 16128 and the phrase at 16224 has no step.
    const Activation& b = out_var->activations.back();
    CHECK(steps_of(b) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped}});
    REQUIRE(b.sqinouts.size() == 1);
    CHECK(b.sqinouts[0].kind == SqueezeKind::SqOut);
    CHECK(b.sqout_tick == std::optional<int64_t>(16224));
    CHECK_FALSE(b.backends.empty());  // 16032 and the squeezed-out 16224
    CHECK(b.backends == out_lead->activations.back().backends);
    CHECK(out_var->pathstring() == "0 1-");

    for (const Path* p : every_path(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

// Review finding (Task 17): a variant that banked a phrase before activating
// can fold into a leader that collected the same phrase in its SP, and the
// leader can later squeeze that phrase in. The variant holds no step there.
// The chart is hand-made: 120 BPM, then 4000 BPM from tick 9600, so 500 ms
// spans several SP bars. '1' activates at 10752 and collects the phrase at
// 12288; '0 E0' activates at 4608, banks 8448 and 12288, and activates again
// at 13824. Both end at 16896 with the same score at 13824, and the leader
// later splits on 12288: '1+' (SqIn) and '1-' (SqOut). Before the activation
// rule they folded there, and the variant took each of the leader's squeezes.
//
// '0 E0' banked 12288 before activating at 13824, so it cannot squeeze that
// phrase in or out (core::activation_can_squeeze), while '1' can. The search
// groups the two apart (banked_phrase_in_reach), so '0 E0' is never folded
// into '1+' or '1-' and prices as its own root path with a plain SP end.
TEST_CASE("tied variants: a path that banked its leader's SqIn phrase is not folded into it") {
    const Song song = load_songpath(std::string(HYDRA_INPUT_DIR) +
                                        "/test_folded_sqin/folded_sqin.chart",
                                    true, true);
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = 4;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));

    const Path* in_lead = root_named(rec.paths, "1+");
    REQUIRE(in_lead != nullptr);
    // The leader's SqIn step sits on 12288, which it collected.
    CHECK(steps_of(in_lead->activations.back()) ==
          std::vector<Step>{{10752, 15360, SpEndKind::Activation}, {12288, 16896, SpEndKind::SqIn}});
    // D32: '1+' ends its SP at 16896, where 12288 is still in the window.
    // Before D32 that SP end was never taken (the window's chord made it
    // look like a squeeze choice), so '1+' ran on in SP and scored 7350.
    CHECK(in_lead->totalscore() == 6750);

    // '0 E0' is never folded into '1+' while SP runs (the group key keeps
    // them apart). Both SP ends fall on 16896 with the same score, so it
    // folds there, after SP, as a plain tied path with its own window.
    const Path* banked = variant_at(*in_lead, 13824);
    REQUIRE(banked != nullptr);
    CHECK(banked->pathstring() == "0 E0");
    const Activation& a = banked->activations.back();
    CHECK(steps_of(a) == std::vector<Step>{{13824, 16896, SpEndKind::Activation}});
    CHECK(a.sqinouts.empty());
    CHECK(banked->totalscore() == 6750);  // what hydra_replay prices 4608:7680,13824:16896 at
}

// The same family with the leader's SqIn phrase on the variant's own
// activation note: phrases at 8448, 9000 and 13824, none at 12288. The
// leader collects 13824 while its SP runs; the other path banks it on its
// activation chord. Before the activation rule, the two folded at 13824 and
// the leader's later SqIn on 13824 met the variant's Activation step there
// (the "not a collected step" throw). Both hand-made charts must analyze, and
// every variant on them must price as it does alone.
TEST_CASE("tied variants: the banked-phrase charts analyze and every variant prices as alone") {
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = 4;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    for (const char* name : {"folded_sqin.chart", "relabel_case.chart"}) {
        CAPTURE(name);
        const Song song = load_songpath(
            std::string(HYDRA_INPUT_DIR) + "/test_folded_sqin/" + name, true, true);
        HydraRecord rec;
        REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            for (const Activation& act : p->walk_activations()) {
                REQUIRE_FALSE(act.sp_end_steps.empty());
                if (act.sqout_tick) CHECK(*act.sqout_tick > act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k)
                    CHECK(act.sp_end_steps[k].tick > act.timecode.ticks());
            }
        }
        std::vector<const Path*> vs;
        for (const Path& root : rec.paths) collect_variants(root, vs);
        for (const Path* v : vs) {
            const std::string diff = lone_pricing_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}

// A folded variant's step on the leader's SqIn phrase can be Clamped: the
// variant's longer meter let the cap pin its end on that phrase, where the
// leader collected it. A lone search relabels a Clamped step SqIn too, so the
// fold must as well. Two fuzzed charts at 2,000 and 4,000 BPM (several SP bars
// inside 500 ms) reach this at cap 3; before the shared SqIn-step rule they
// threw "a folded variant's SqIn phrase is not a collected step". They do not
// check every variant against its lone pricing: at these tempos many variants
// already differ from it (the review's fuzz found such charts by the hundred),
// for reasons apart from this fold.
//
// Finding 37 (D29) changed what these charts produce. The pinned variant step
// ({16128, 20736, SqIn} on chart a, {15744, 20352, SqIn} on chart b) came from
// a squeeze the old graph offered by mistake: a deact node whose window held
// the clamped phrase offered it to any path whose end was one plain bar past
// the node, even when a later phrase, not the clamped one, had put the path's
// end there (on chart b, 15744 offered at node 20928 to a path whose end
// 22464 came from collecting 18048). Its squeeze-out then ended SP at a node
// the record never names. The squeeze choices now read extend_deacts, cap
// included, so those offers are gone. What these charts still check: they
// analyze, and every squeeze-out ends SP at the end its record names. The
// fold this test was written for (a variant's Clamped step on its leader's
// SqIn phrase, relabelled SqIn) no longer happens on them; test_s2_deact_
// extension.cpp's "folded variant's Clamped step" case covers it now.
TEST_CASE("clamped_sqin charts: every squeeze-out ends SP at the end its record names") {
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = 3;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    for (const std::string name : {"clamped_sqin_a.chart", "clamped_sqin_b.chart"}) {
        CAPTURE(name);
        const Song song = load_songpath(
            std::string(HYDRA_INPUT_DIR) + "/test_folded_sqin/" + name, true, true);
        HydraRecord rec;
        REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
        int sqouts = 0;
        std::vector<const Path*> every;
        for (const Path& root : rec.paths) {
            every.push_back(&root);
            collect_variants(root, every);
        }
        for (const Path* p : every) {
            CAPTURE(p->pathstring());
            for (const Activation& a : p->walk_activations()) {
                if (!a.sqout_tick) continue;
                ++sqouts;
                // The squeezed-out row's offset is measured from the node SP
                // really ended on; the record's end must be that node.
                REQUIRE(a.deact_tick().has_value());
                REQUIRE(a.sqout_row() != nullptr);
                CHECK(*a.sqout_row()->offset_ms ==
                      song.timecode(*a.sqout_tick).ms() - song.timecode(*a.deact_tick()).ms());
            }
        }
        CHECK(sqouts > 0);
    }
}

// The group key's banked phrase and the squeeze rule draw one line: a phrase
// is banked exactly when activation_can_squeeze says the activation cannot
// squeeze it. Here the expected answer comes from that rule by a plain scan,
// so a banked_phrase_in_reach with its own boundary would drift from it. The
// chart has a phrase on an activation chord (13824), the edge case.
TEST_CASE("banked_phrase_in_reach banks exactly what activation_can_squeeze refuses") {
    const Song song = load_songpath(
        std::string(HYDRA_INPUT_DIR) + "/test_folded_sqin/folded_sqin.chart", true, true);
    const std::vector<SongTimestamp>& seq = song.sequence;
    int on_activation_chord = 0;
    for (const SongTimestamp& act : seq) {
        const int64_t act_tick = act.timecode.ticks();
        CAPTURE(act_tick);
        const SongTimestamp* banked = nullptr;
        for (const SongTimestamp& c : seq)
            if (c.flag_sp && !core::activation_can_squeeze(act_tick, c.timecode.ticks()))
                banked = &c;
        // A reach at the activation itself: any banked phrase within 500 ms
        // before it counts.
        const SongTimestamp* want =
            banked && within_squeeze_window(offset_from_sp_end(banked->timecode.ms(),
                                                               act.timecode.ms()))
                ? banked
                : nullptr;
        CHECK(core::banked_phrase_in_reach(song, act_tick, act.timecode) == want);
        if (want && want->timecode.ticks() == act_tick) ++on_activation_chord;
    }
    CHECK(on_activation_chord > 0);
}

// D34 at a normal tempo. Two SP ends sit one tick apart, 30720 and 30721:
// '0' ends at 30720 (19200 plus two bars, plus one bar for the phrase on
// 23042), and '1' ends at 30721 (the cap pins it to 23042 plus two bars).
// plusmeasure rounds both one bar on to the same tick, 34560, so both ends
// offered the phrase on 30480 to the same path. It was squeezed in at the
// first and in or out again at the second: '0++' and '0+-', '1++' and '1+-'.
// Real charts do this (Thrice - Deadbolt, SoundHaven - Triad). A phrase is
// squeezed in once, so the second end offers the window's next phrase, and
// here there is none. The chart is hand-made (120 BPM, resolution 480).
TEST_CASE("squeeze rule: twin SP ends a tick apart squeeze one phrase in once (D34)") {
    const Song song = load_songpath(
        std::string(HYDRA_INPUT_DIR) + "/test_folded_sqin/twin_end_nodes.chart", true, true);
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = 2;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
    for (const Path* p : rec.all_paths()) {
        CAPTURE(p->pathstring());
        for (const Activation& a : p->walk_activations()) {
            std::vector<int64_t> sqin_steps;
            for (const SpEndStep& s : a.sp_end_steps)
                if (s.kind == SpEndKind::SqIn) sqin_steps.push_back(s.tick);
            const size_t sqins = static_cast<size_t>(
                std::count_if(a.sqinouts.begin(), a.sqinouts.end(),
                              [](const SPSqueeze& q) { return q.kind == SqueezeKind::SqIn; }));
            // One SqIn per phrase, and never out again once in.
            CHECK(sqins == sqin_steps.size());
            CHECK(std::set<int64_t>(sqin_steps.begin(), sqin_steps.end()).size() ==
                  sqin_steps.size());
            if (a.sqout_tick)
                CHECK(std::find(sqin_steps.begin(), sqin_steps.end(), *a.sqout_tick) ==
                      sqin_steps.end());
        }
    }
    // The best path keeps its score: the second "+" never moved anything.
    const Path* best = root_named(rec.paths, "0+");
    REQUIRE(best != nullptr);
    CHECK(best->totalscore() == 2950);
    CHECK(rec.best_path().totalscore() == 2950);
    CHECK(root_named(rec.paths, "0++") == nullptr);
    CHECK(root_named(rec.paths, "0+-") == nullptr);
}

// D3 on the corpus: every tied variant stores what the search stores when it
// prices that path alone, window by window (steps, squeezes, squeezed-out
// note, backend rows). The settings are ones where mid-SP folds happen; the
// defaults have none on this corpus.
TEST_CASE("tied variants: every corpus variant matches its lone pricing") {
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = 4;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = 10.0;
    int variants = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> vs;
        for (const Path& root : rec.paths) collect_variants(root, vs);
        for (const Path* v : vs) {
            ++variants;
            const std::string diff = lone_pricing_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), chart << " " << diff);
        }
    }
    CHECK(variants > 200);
}

TEST_CASE("tied variants: a variant that finished the song keeps its own bank") {
    // Cap 4. Two phrases fill 2 bars, then two fills. '0' activates at 2304;
    // its SP ends at 5376, so the phrase at 5760 is banked: 1 bar left. '1'
    // activates at 3072; its SP still runs at 5760, so that phrase extends it
    // instead: nothing left. Each doubles two notes, so both score 400, and
    // the search folds one into the other once both finish.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 3});

    auto check_banks = [](const std::vector<const Path*>& all) {
        int tied = 0;
        for (const Path* p : all) {
            if (p->totalscore() != 400) continue;
            ++tied;
            REQUIRE(p->walk_activations().size() == 1);
            const int64_t at = p->walk_activations().front().timecode.ticks();
            const std::vector<int64_t> want =
                at == 2304 ? std::vector<int64_t>{5760} : std::vector<int64_t>{};
            CHECK_MESSAGE(p->trailing_bank_ticks == want, "activation at " << at);
            CHECK(p->leftover_sp() == static_cast<int>(want.size()));
        }
        CHECK(tied == 2);
    };
    const std::vector<const Path*> all = every_path(roots);
    int variants = 0;
    for (const Path* p : all) variants += p->var_point.has_value() ? 1 : 0;
    REQUIRE(variants == 1);  // the two ties are one root and its variant
    check_banks(all);

    HydraRecord rec;
    rec.paths = roots;
    const HydraRecord back = store::rebuild_record(store::flatten_record(rec));
    check_banks(back.all_paths());
}

TEST_CASE("tied variants: a variant folded between windows keeps its own banked bars") {
    // Cap 2. '0' activates at 2304 (SP to 5376), then banks 5760 and 8448;
    // the phrase at 9216 finds the meter full. '1' activates at 3072 (SP to
    // 7680, extended by 5760), then banks 8448 and 9216. At the fill at 9984
    // both hold 2 bars with the same score, outside SP, so the search folds
    // '0' into '1' there. The leader then activates at 10752 ('1 0' and its
    // variant '0 0', 1100) or never again ('1' and its variant '0', 850).
    // Either way the variant's 2 bars are its own, banked at 5760 and 8448.
    //
    // The plan's draft had no fill at 9984. Without it the two windows meet
    // only on the SP node after both activate at 10752, a fold mid-SP, so the
    // draft's case passed before the fix and tested nothing new.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448, true, false},
                                       {9216, true, false}, {9984, false, true},
                                       {10752, false, true},
                                       {11520}, {12288}, {16128}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});

    const std::vector<int64_t> own_0{5760, 8448};
    const std::vector<int64_t> own_1{8448, 9216};
    int again = 0, never = 0, variants = 0;
    for (const Path* p : every_path(roots)) {
        const ActivationWalk acts = p->walk_activations();
        if (acts.empty()) continue;
        const int64_t first = acts[0].timecode.ticks();
        if (first != 2304 && first != 3072) continue;
        const std::vector<int64_t>& own = first == 2304 ? own_0 : own_1;
        CAPTURE(p->pathstring());
        CAPTURE(first);
        if (p->totalscore() == 1100) {
            ++again;
            REQUIRE(acts.size() == 2);
            CHECK(acts[1].timecode.ticks() == 10752);
            CHECK(p->pathstring() == (first == 2304 ? "0 0" : "1 0"));
            CHECK(acts[1].bank_rise_ticks == own);
            CHECK(acts[1].sp_meter() == 2);
            CHECK(p->trailing_bank_ticks.empty());
        } else if (p->totalscore() == 850) {
            ++never;
            REQUIRE(acts.size() == 1);
            CHECK(p->pathstring() == (first == 2304 ? "0" : "1"));
            CHECK(p->trailing_bank_ticks == own);
        } else {
            continue;
        }
        variants += p->var_point.has_value() ? 1 : 0;
        CHECK(replay_stored_path(song, *p).faithful());
    }
    CHECK(again == 2);
    CHECK(never == 2);
    CHECK(variants == 2);  // each pair is one root and its variant
}

TEST_CASE("tied variants: Don Broco - Actors at depth 40 shows each tied path's own bank") {
    // Audit finding 89: paths 29 '7' and 30 '3' tie at 233,100. '7' ends the
    // song with 1 bar of SP, '3' with 3. Today both read 1.
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Don Broco - Actors") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings s;
    s.sp_cap = 4;
    s.depth_mode = DepthMode::Scores;
    s.depth_value = 40;
    s.ms_filter = 10.0;
    const HydraRecord& rec = corpus::analyzed(chart, s);

    int seen = 0;
    for (const Path* p : rec.all_paths()) {
        if (p->totalscore() != 233100) continue;
        const int64_t first = p->walk_activations().front().timecode.ticks();
        if (first == 150720) { CHECK(p->leftover_sp() == 1); ++seen; }
        if (first == 82560) { CHECK(p->leftover_sp() == 3); ++seen; }
    }
    CHECK(seen == 2);
}
