// Where Star Power ends, and what moves it: SP that outlasts the chart, the
// squeeze choices at an SP end, a phrase collected mid-SP (overfill and the
// collected-phrase list), the SP end history, the transfer scales anchored on
// it, the squeeze-out, and the bars banked at the deact node. The corpus-wide
// engine sweeps live in test_engine_sweeps.cpp.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
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
#include "app/dynamics_breakdown.h"
#include "bank_check.h"
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

using namespace hydra;

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
// A tick's ms, and the two SP-past-the-end songs (record_fixtures.h).
using test::tick_ms;
using test::past_last_note_song;
using test::mid_sp_phrase_song;

// The chart for cap 2 where two mid-SP phrases, at 3072 and 3840, each clamp
// the end. Several cases below share these notes.
std::vector<TailNote> double_clamp_notes() {
    return {{0, true, false},    {768, true, false},  {2304, false, true},
            {3072, true, false}, {3840, true, false}, {4608},
            {5376},              {6000},              {6768},
            {7500}};
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
    Song song = past_last_note_song();

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
          doctest::Approx(offset_from_sp_end(tick_ms(song, 5280), end_ms)).epsilon(1e-9));

    // And the rows put the SP end back exactly where the engine had it.
    auto deact = act.deact_tick();
    REQUIRE(deact.has_value());
    CHECK(*deact == end_tick);
}

TEST_CASE("SP past the last note: a mid-activation phrase extends the end") {
    // Same shape, but the note at 3840 completes an SP phrase during the
    // activation. That pushes the pending deactivation two measures out, from
    // tick 5376 to 6912 -- an extension the measure-count fallback (2 measures
    // per SP bar, from a meter still recorded as 2) cannot see.
    Song song = mid_sp_phrase_song();

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

    auto deact = act.deact_tick();
    REQUIRE(deact.has_value());
    CHECK(*deact == extended_tick);
    CHECK(*deact != plain_tick);
    // The activation records no SqIn, so the old fallback would have landed
    // on the plain end. Pin that the rows, not the reconstruction, answered.
    CHECK(act.sqinouts.empty());
    CHECK(song.timing().plusmeasure(act.timecode, 4).ticks() == plain_tick);
}

// D5: a tail row is kept on the same window as every other row, measured
// from this activation's own SP end. The graph keeps 5136 (375 ms before the
// last note), but it sits 625 ms before the SP end at 5376, so the engine
// must not copy it into the activation.
TEST_CASE("SP past the last note: tail rows use the 500 ms window from the SP end") {
    Song song = past_last_note_song();
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

// The graph lists, for every deactivation edge, exactly the phrase chords
// core::squeeze_window_phrases names, in chart order, so the engine can offer
// a window the one phrase core::offered_phrase picks (D36): its newest phrase
// before the end, or the first after it not yet squeezed in. This guards
// against drift.
TEST_CASE("graph: every deactivation edge lists the chords squeeze_window_phrases names") {
    int edges = 0, claimed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        const ScoreGraph graph(song, 4);

        std::set<const ScoreGraphEdge*> seen;
        for (const ScoreGraphNode* sp = test::sp_track_start(graph); sp;
             sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
            const ScoreGraphEdge* e = sp->branch_edge;
            if (!e || !seen.insert(e).second) continue;
            ++edges;
            const Timecode& end = e->dest->timecode;
            const std::vector<const SongTimestamp*> window =
                core::squeeze_window_phrases(song, end);
            REQUIRE(e->squeeze_choices.size() == window.size());
            if (window.empty()) continue;
            ++claimed;
            for (size_t k = 0; k < window.size(); ++k)
                CHECK(e->squeeze_choices[k].chord.ticks() == window[k]->timecode.ticks());
        }
    }
    CHECK(edges > 0);
    CHECK(claimed > 0);
}

using test::deact_edge_at;
using test::extension_of;

// A squeeze choice's facts, pinned on two hand-built songs at 240 BPM (a
// measure is 1920 ticks and 1000 ms). Both activate at 5760 with two bars,
// so SP ends at 13440 (7000 ms), and one bar more is 17280 (9000 ms).
TEST_CASE("graph: a squeeze choice before or after the SP end, pinned") {
    SUBCASE("a phrase 250 ms before the end: an early squeeze, both ends move") {
        const Song song = test::make_early_sqout_song();
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, 13440);
        REQUIRE(e != nullptr);
        REQUIRE(e->squeeze_choices.size() == 1);
        const SqueezeChoice& c = e->squeeze_choices[0];
        CHECK(c.chord.ticks() == 12960);
        CHECK(c.timing == doctest::Approx(-250.0).epsilon(1e-9));
        CHECK_FALSE(c.late);
        // An early chord's moved end is its own step's, not the choice's.
        CHECK_FALSE(c.sqin_time.has_value());
        // The phrase's own step moves 13440 to 17280, and 13440 can give it
        // back: the step offers the squeeze there (D36).
        const std::optional<SpExtension> x = extension_of(graph, 12960, 13440);
        REQUIRE(x.has_value());
        CHECK(x->to_tick == 17280);
        CHECK_FALSE(x->clamped);
        CHECK(x->sqout_node);
    }
    SUBCASE("a phrase 250 ms after the end: a late squeeze-in only") {
        const Song song = test::make_late_sqin_song();
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, 13440);
        REQUIRE(e != nullptr);
        REQUIRE(e->squeeze_choices.size() == 1);
        const SqueezeChoice& c = e->squeeze_choices[0];
        CHECK(c.chord.ticks() == 13920);
        REQUIRE(c.sqin_time.has_value());
        CHECK(c.sqin_time->ticks() == 17280);
        CHECK(c.timing == doctest::Approx(250.0).epsilon(1e-9));
        CHECK(c.late);
    }
    SUBCASE("no phrase in the window: the path just ends at the SP end") {
        // Two bars from the fill at 5760, and the next phrase is past the
        // window.
        const Song song = test::beat_song({480, 1920, 15360}, {5760}, 21120);
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, 13440);
        REQUIRE(e != nullptr);
        CHECK(e->squeeze_choices.empty());
    }
}

// The tick test (core::after_sp_end) and the ms test (core::paid_by_sp_walk)
// give opposite answers for every chord near an SP end, even when the tempo
// changes on the end tick itself. The engine's squeeze-out bank reads the
// graph's tick answer (SqueezeChoice::late); stored squeezes keep only the
// ms offset; this pins that the two agree. Two phrases bank two bars, the
// fill at 2304 activates, so SP ends 4 measures later at 5376, where the
// tempo halves from 120 to 60 BPM. Phrase chords sit 250 ms before the end,
// on it, and 250 ms after it.
TEST_CASE("graph: the tick and ms tests of 'after the SP end' agree across a tempo change on the end") {
    const int64_t end_tick = 5376;
    const Song song = test::build_tempo_song(
        {{0, true}, {768, true}, {1536}, {2304, false, true}, {3072}, {3840}, {4608},
         {5280, true}, {5376, true}, {5424, true}, {6144}, {6912}, {7680}},
        {{0, 120.0}, {end_tick, 60.0}});
    const double end_ms = tick_ms(song, end_tick);

    // Tick by tick around the end: one tick before and after reads each tempo.
    for (int64_t t = end_tick - 2; t <= end_tick + 2; ++t) {
        CAPTURE(t);
        const double offset = offset_from_sp_end(tick_ms(song, t), end_ms);
        CHECK(core::after_sp_end(t, end_tick) == !core::paid_by_sp_walk(offset));
    }

    // Every squeeze choice the graph lists: its tick answer and its offset agree.
    const ScoreGraph graph(song, 4);
    const ScoreGraphEdge* at_end = deact_edge_at(graph, end_tick);
    REQUIRE(at_end != nullptr);
    std::vector<std::pair<int64_t, bool>> at_end_choices;
    for (const SqueezeChoice& c : at_end->squeeze_choices)
        at_end_choices.push_back({c.chord.ticks(), c.late});
    CHECK(at_end_choices ==
          std::vector<std::pair<int64_t, bool>>{{5280, false}, {5376, false}, {5424, true}});
    int choices = 0;
    for (const ScoreGraphNode* sp = test::sp_track_start(graph); sp;
         sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
        const ScoreGraphEdge* e = sp->branch_edge;
        if (!e) continue;
        for (const SqueezeChoice& c : e->squeeze_choices) {
            CAPTURE(c.chord.ticks());
            CHECK(c.late == !core::paid_by_sp_walk(c.timing));
            ++choices;
        }
    }
    CHECK(choices >= 3);
}

// ---- SP cap overfill: a phrase collected mid-SP can clamp the end ------
//
// The meter holds a fixed number of bars. Once it's full, a phrase you
// collect while SP is active can't push the end out by a plain 2 measures
// any more -- it gets capped at 2*cap measures past that phrase's own note.
// When the cap wins, the note that pinned the end (not the activation) is
// what the timing warning has to point at, so the search stamps that note's
// tick onto the activation as clamp_tick.

// Every row below shares one activation. Cap 2 bars. Two SP phrases fill the
// meter before the activation at tick 2304 (meter 2), whose plain end is 4
// measures later at 5376. Each row then adds mid-SP phrases and pins where
// the end lands and which note, if any, clamped it. A row with steps also
// pins the end's history and the note each step's end is anchored on.
struct OverfillCase {
    const char* name;
    const char* why;
    std::vector<TailNote> notes;
    int64_t deact;
    std::optional<int64_t> clamp;  // unset: no step clamped the end
    std::vector<SpEndStep> steps;  // empty: this row does not pin the history
    std::vector<int64_t> anchors;  // end_anchor_tick of each step in `steps`
};

TEST_CASE("SP cap overfill: where the end lands and which note clamped it") {
    const std::vector<OverfillCase> cases = {
        {"a mid-SP phrase that clamps records the collecting note",
         "The phrase collected at 3072, mid-SP, moves the pending end to "
         "min(prev_end + 2 measures, 3072 + 2*cap measures). prev_end + 2 "
         "measures is 5376 + 1536 = 6912; the cap ceiling is 3072 + 4*768 = "
         "6144. The ceiling is smaller, so it wins: the end is pinned to 6144, "
         "and clamp_tick records the note that pinned it, 3072.",
         {{0, true, false}, {768, true, false}, {2304, false, true},
          {3072, true, false}, {3840}, {4608}, {5376}, {6000}},
         6144, 3072, {}, {}},
        {"a mid-SP phrase that only ties the cap does not clamp",
         "Same cap and activation as above, but the mid-SP phrase lands at 3840 "
         "instead of 3072. Now both options land on the same tick: prev_end + 2 "
         "measures is 5376 + 1536 = 6912, and the cap ceiling is 3840 + 4*768 = "
         "6912 too. A tie means the plain extension wins, not the cap -- so this "
         "is not a clamp, and clamp_tick stays unset.",
         {{0, true, false}, {768, true, false}, {2304, false, true},
          {3840, true, false}, {4608}, {5376}, {6000}, {6500}},
         6912, std::nullopt, {}, {}},
        {"a later unclamped extension keeps the earlier clamp_tick",
         "Same cap and activation, but SP is extended twice. The first mid-SP "
         "phrase, at 3072, clamps exactly as in the first case above: the end is "
         "pinned to 6144, with clamp_tick 3072. The second phrase, at 5760, "
         "wants to move the end again -- the cap ceiling from THAT note would be "
         "5760 + 4*768 = 8832, but the plain +2-measure step from the current "
         "end (6144 + 1536 = 7680) is smaller and wins instead. Because this "
         "second extension is not itself a clamp, the note that pinned the "
         "window stays the first one: clamp_tick is still 3072, even though the "
         "end has moved again. The chart ends at 7500, before the SP end at "
         "7680, so this is the same \"SP outlasts the chart\" case as the tests "
         "above: the tail rows are synthesized against the tracked end rather "
         "than a real deactivation.",
         {{0, true, false}, {768, true, false}, {2304, false, true},
          {3072, true, false}, {3840}, {4608}, {5376}, {5760, true, false},
          {6000}, {6768}, {7500}},
         7680, 3072, {}, {}},
        {"a second clamp in the same window replaces clamp_tick, and each "
         "clamp is a step the anchor follows",
         "Same cap and activation. The phrase at 3072 clamps as in the first "
         "case: the end is pinned to 6144. The phrase at 3840 then clamps again: "
         "the plain step from the current end is 6144 + 1536 = 7680, but the "
         "cap ceiling from 3840 is 3840 + 4*768 = 6912, which is smaller. The "
         "end is now pinned by the later note, so clamp_tick moves to 3840.",
         double_clamp_notes(),
         6912, 3840,
         {{2304, 5376, SpEndKind::Activation},
          {3072, 6144, SpEndKind::Clamped},
          {3840, 6912, SpEndKind::Clamped}},
         {2304, 3072, 3840}},
    };

    for (const OverfillCase& c : cases) {
        SUBCASE(c.name) {
            INFO(c.why);
            Song song = build_tail_song(c.notes);
            ScoreGraph graph(song, 2);
            const std::vector<Path> paths = run_search(graph, EngineOptions{});

            const Activation& act = last_act(paths);
            CHECK(act.sp_meter() == 2);
            CHECK(act.timecode.ticks() == 2304);

            const std::optional<int64_t> deact = act.deact_tick();
            REQUIRE(deact.has_value());
            CHECK(*deact == c.deact);

            if (c.clamp) {
                REQUIRE(act.clamp_tick().has_value());
                CHECK(*act.clamp_tick() == *c.clamp);
            } else {
                CHECK_FALSE(act.clamp_tick().has_value());
            }

            if (!c.steps.empty()) {
                CHECK(act.sp_end_steps == c.steps);
                for (size_t i = 0; i < c.anchors.size(); ++i)
                    CHECK(act.end_anchor_tick(i) == c.anchors[i]);
            }
        }
    }
}

TEST_CASE("collected phrases: none when no phrase lands during the activation") {
    Song song = past_last_note_song();
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
    Song song = mid_sp_phrase_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);
    CHECK(act.collected_phrase_ticks() == std::vector<int64_t>{3840});
}

TEST_CASE("collected phrases: two phrases under a full meter are both recorded, in order") {
    // The cap-2 clamp fixture: both phrases are collected while active, and
    // the second is the note the cap pinned the end to. A clamped phrase
    // counts as collected.
    Song song = build_tail_song(double_clamp_notes());
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
    cases.push_back({"SP past the last note", past_last_note_song(), 4});
    cases.push_back({"mid-SP phrase, SP past the end", mid_sp_phrase_song(), 4});
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

// A path that banked the phrase may not be folded into one that collected it:
// after the fold the variant would take the leader's squeeze-out, a choice it
// never had, and store the leader's score under its own windows.
TEST_CASE("SP end history: a variant never takes a squeeze on a phrase it banked") {
    Song song = test::banked_phrase_fold_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths;
    REQUIRE_NOTHROW(paths = run_search(graph, test::wide_search()));
    const std::vector<const Path*> all = flatten_paths(paths);
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
                // Appendix B's first guarantee and D34: one SqIn step per
                // SqIn, never repeated or squeezed out (tests/bank_check.h).
                bank_check::check_one_step_per_sqin(act);
            }
        }
    }
    CHECK(acts > 1000);  // a floor the user approved (D43)
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
        CAPTURE(chart);
        acts += bank_check::check_record_banks(song, corpus::analyzed(chart, cfg));
    }
    CHECK(acts > 1000);  // a floor the user approved (D43)
}

// A deactivation edge keeps no rows of its own: its rows are a range of the
// graph's one row list, read through ScoreGraph::edge_backends. The three
// numbers were pinned from one run on c251abd, before the change: one row per
// chart timestamp, the SP track's deactivation edges, and the rows those
// edges held as their own copies then.
TEST_CASE("graph: a deactivation edge's rows are a range of the graph's rows") {
    const Song& song = corpus::song(corpus::first_chart_with_notes(), true, true);
    const ScoreGraph graph(song, 4);
    const size_t all = graph.all_backends().size();
    CHECK(all == 622);

    std::set<const ScoreGraphEdge*> seen;
    size_t rows = 0;
    for (const ScoreGraphNode* sp = test::sp_track_start(graph); sp;
         sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
        const ScoreGraphEdge* e = sp->branch_edge;
        if (!e || !seen.insert(e).second) continue;
        CAPTURE(e->dest->timecode.ticks());
        CHECK(e->backend_begin <= e->backend_end);
        CHECK(e->backend_end <= static_cast<int32_t>(all));
        rows += graph.edge_backends(*e).size();
    }
    CHECK(seen.size() == 28);
    CHECK(rows == 134);
}
