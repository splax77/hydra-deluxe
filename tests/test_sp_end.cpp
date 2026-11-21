// Where Star Power ends, and what moves it: SP that outlasts the chart, the
// squeeze choices at an SP end, a phrase collected mid-SP (overfill and the
// collected-phrase list), the SP end history, the transfer scales anchored on
// it, the squeeze-out, and the bar a squeeze-out banks. The corpus-wide
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
#include "core/backend_value.h"
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

// The tick `bars` SP bars after `from_tick` on the song's own measure count:
// the SP end that much SP in force at `from_tick` gives. SongTiming::plusmeasure
// and sp_bars_to_measures own the arithmetic.
int64_t end_after_bars(const Song& song, int64_t from_tick, int64_t bars) {
    return song.timing().plusmeasure(song.timecode(from_tick), sp_bars_to_measures(bars)).ticks();
}

// The Song for one corpus chart. A chart that fails to load throws
// (corpus::song), and every chart the corpus scan lists has notes at the
// reading these loops use, so one that loads empty fails here by name
// rather than being passed over.
const Song& corpus_song_with_notes(const std::string& path, const ChartReading& reading,
                                   const core::Rules& rules = core::default_rules()) {
    const Song& song = corpus::song(path, reading, rules);
    REQUIRE_MESSAGE(!song.is_empty(), path << " loads with no notes");
    return song;
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

    // The activation has rows even though its SP end is past the last note.
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
    // tick 5376 to 6912 -- an extension that counting 2 measures per SP bar,
    // from a meter still recorded as 2, cannot see.
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
    // The activation records no SqIn, and a measure count from its meter lands
    // on the plain end, so the extended end above came from no such count.
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
// before the end, or the first after it not yet squeezed in. The window
// rule is the owner's; this checks that the graph's copy into each edge
// drops, adds and reorders no chord.
TEST_CASE("graph: every deactivation edge lists the chords squeeze_window_phrases names") {
    int edges = 0, claimed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        CAPTURE(path);
        const Song& song = corpus_song_with_notes(path, ChartReading{});
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

// A squeeze choice's facts on two hand-built beat songs (record_fixtures.h).
// Both bank two bars and activate on the fill at kBeatActTick, so SP ends
// two bars later; the squeeze phrase sits one beat before or after that end.
namespace {
constexpr int64_t kBeatActTick = 5760;
constexpr int64_t kBeatBankedBars = 2;

// The SP end those songs' activation gives.
int64_t beat_song_end(const Song& song) {
    return end_after_bars(song, kBeatActTick, kBeatBankedBars);
}
}  // namespace

TEST_CASE("graph: a squeeze choice before or after the SP end, pinned") {
    SUBCASE("a phrase one beat before the end: an early squeeze, both ends move") {
        const Song song = test::make_early_sqout_song();
        const int64_t end = beat_song_end(song);
        const int64_t phrase = end - song.tick_resolution();
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, end);
        REQUIRE(e != nullptr);
        REQUIRE(e->squeeze_choices.size() == 1);
        const SqueezeChoice& c = e->squeeze_choices[0];
        CHECK(c.chord.ticks() == phrase);
        CHECK(c.timing == doctest::Approx(offset_from_sp_end(tick_ms(song, phrase),
                                                             tick_ms(song, end)))
                              .epsilon(1e-9));
        CHECK(c.timing < 0.0);
        CHECK_FALSE(c.late);
        // An early chord's moved end is its own step's, not the choice's.
        CHECK_FALSE(c.sqin_time.has_value());
        // The phrase's own step moves the end one bar on, and the old end can
        // give it back: the step offers the squeeze there (D36).
        const std::optional<SpExtension> x = extension_of(graph, phrase, end);
        REQUIRE(x.has_value());
        CHECK(x->to_tick == end_after_bars(song, end, 1));
        CHECK_FALSE(x->clamped);
        CHECK(x->sqout_node);
    }
    SUBCASE("a phrase one beat after the end: a late squeeze-in only") {
        const Song song = test::make_late_sqin_song();
        const int64_t end = beat_song_end(song);
        const int64_t phrase = end + song.tick_resolution();
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, end);
        REQUIRE(e != nullptr);
        REQUIRE(e->squeeze_choices.size() == 1);
        const SqueezeChoice& c = e->squeeze_choices[0];
        CHECK(c.chord.ticks() == phrase);
        REQUIRE(c.sqin_time.has_value());
        CHECK(c.sqin_time->ticks() == end_after_bars(song, end, 1));
        CHECK(c.timing == doctest::Approx(offset_from_sp_end(tick_ms(song, phrase),
                                                             tick_ms(song, end)))
                              .epsilon(1e-9));
        CHECK(c.timing > 0.0);
        CHECK(c.late);
    }
    SUBCASE("no phrase in the window: the path just ends at the SP end") {
        // The next phrase after the two banked ones is well past the window.
        const Song song = test::beat_song({480, 1920, 15360}, {kBeatActTick}, 21120);
        const ScoreGraph graph(song, 4);
        const ScoreGraphEdge* e = deact_edge_at(graph, beat_song_end(song));
        REQUIRE(e != nullptr);
        CHECK(e->squeeze_choices.empty());
    }
}

// "Is this chord after the SP end?" has a tick answer (core::after_sp_end) and
// an ms answer (core::paid_by_sp_walk on the chord's offset). Each is checked
// here against the chord's place: a chord after the end tick is after the
// end, one on it or before it is paid by the SP walk. The tempo changes on
// the end tick itself, so one tick either side reads a different tempo. Two
// phrases bank two bars and the fill at act_tick activates; phrase chords
// sit before the end, on it, and after it.
TEST_CASE("graph: the tick and ms tests of 'after the SP end' agree across a tempo change on the end") {
    const int64_t act_tick = 2304;
    const int64_t end_tick = 5376;
    const std::vector<int64_t> phrases_near_end = {5280, 5376, 5424};
    const Song song = test::build_tempo_song(
        {{0, true}, {768, true}, {1536}, {act_tick, false, true}, {3072}, {3840}, {4608},
         {phrases_near_end[0], true}, {phrases_near_end[1], true}, {phrases_near_end[2], true},
         {6144}, {6912}, {7680}},
        {{0, 120.0}, {end_tick, 60.0}});
    // The tempo change really sits on the SP end.
    REQUIRE(end_after_bars(song, act_tick, 2) == end_tick);
    const double end_ms = tick_ms(song, end_tick);

    // Tick by tick around the end.
    for (int64_t t = end_tick - 2; t <= end_tick + 2; ++t) {
        CAPTURE(t);
        const bool after = t > end_tick;
        CHECK(core::after_sp_end(t, end_tick) == after);
        CHECK(core::paid_by_sp_walk(offset_from_sp_end(tick_ms(song, t), end_ms)) == !after);
    }

    // The graph lists the three phrases at that end, each late exactly when
    // it is after the end tick, and its offset answers the same way.
    const ScoreGraph graph(song, 4);
    const ScoreGraphEdge* at_end = deact_edge_at(graph, end_tick);
    REQUIRE(at_end != nullptr);
    std::vector<int64_t> listed;
    for (const SqueezeChoice& c : at_end->squeeze_choices) {
        CAPTURE(c.chord.ticks());
        listed.push_back(c.chord.ticks());
        const bool after = c.chord.ticks() > end_tick;
        CHECK(c.late == after);
        CHECK(core::paid_by_sp_walk(c.timing) == !after);
    }
    CHECK(listed == phrases_near_end);
}

// ---- SP cap overfill: a phrase collected mid-SP can clamp the end ------
//
// The game's rule for a phrase collected under a full meter is SP-24 in
// docs/clone-hero/05-star-power.md; which note the end is then anchored on is
// SP-40. The game's cap is always the full bar, so the cap of 2 here is a
// what-if setting with no game counterpart (SP-24); it keeps the songs short.

namespace {

constexpr int kOverfillCap = 2;
constexpr int64_t kOverfillActTick = 2304;

// Every row below shares one activation: two SP phrases fill the meter
// before the fill at kOverfillActTick. Each row lists its mid-SP phrases and
// the step kind each one makes, which is the shape the row's name describes.
struct OverfillCase {
    const char* name;
    std::vector<TailNote> notes;
    std::vector<int64_t> mid_sp_phrases;
    std::vector<SpEndKind> kinds;  // the step each mid-SP phrase makes
};

// The SP-end history SP-24 gives for a full meter of `cap` bars: each mid-SP
// phrase moves the end to the earlier of one bar past the end in force and
// a full meter past the phrase itself. Only a strictly earlier ceiling is a
// clamp: on a tie the plain bar step is what moved the end (graph.cpp,
// extend_deacts).
std::vector<SpEndStep> overfill_steps(const Song& song, int cap,
                                      const std::vector<int64_t>& phrases) {
    int64_t end = end_after_bars(song, kOverfillActTick, cap);
    std::vector<SpEndStep> steps{{kOverfillActTick, end, SpEndKind::Activation}};
    for (int64_t p : phrases) {
        const int64_t plain = end_after_bars(song, end, 1);
        const int64_t ceiling = end_after_bars(song, p, cap);
        const bool clamped = ceiling < plain;
        end = clamped ? ceiling : plain;
        steps.push_back({p, end, clamped ? SpEndKind::Clamped : SpEndKind::Collected});
    }
    return steps;
}

}  // namespace

TEST_CASE("SP cap overfill: where the end lands and which note clamped it") {
    const std::vector<OverfillCase> cases = {
        {"a mid-SP phrase that clamps records the collecting note",
         {{0, true, false}, {768, true, false}, {kOverfillActTick, false, true},
          {3072, true, false}, {3840}, {4608}, {5376}, {6000}},
         {3072},
         {SpEndKind::Clamped}},
        {"a mid-SP phrase that only ties the cap does not clamp",
         {{0, true, false}, {768, true, false}, {kOverfillActTick, false, true},
          {3840, true, false}, {4608}, {5376}, {6000}, {6500}},
         {3840},
         {SpEndKind::Collected}},
        // The chart ends at 7500, before the SP end, so this is also the "SP
        // outlasts the chart" case of the tests above.
        {"a later unclamped extension keeps the earlier clamp_tick",
         {{0, true, false}, {768, true, false}, {kOverfillActTick, false, true},
          {3072, true, false}, {3840}, {4608}, {5376}, {5760, true, false},
          {6000}, {6768}, {7500}},
         {3072, 5760},
         {SpEndKind::Clamped, SpEndKind::Collected}},
        {"a second clamp in the same window replaces clamp_tick, and each "
         "clamp is a step the anchor follows",
         double_clamp_notes(),
         {3072, 3840},
         {SpEndKind::Clamped, SpEndKind::Clamped}},
    };

    for (const OverfillCase& c : cases) {
        SUBCASE(c.name) {
            Song song = build_tail_song(c.notes);
            const std::vector<SpEndStep> want = overfill_steps(song, kOverfillCap, c.mid_sp_phrases);
            // The row is the shape its name says.
            std::vector<SpEndKind> want_kinds;
            for (size_t i = 1; i < want.size(); ++i) want_kinds.push_back(want[i].kind);
            REQUIRE(want_kinds == c.kinds);

            ScoreGraph graph(song, kOverfillCap);
            const std::vector<Path> paths = run_search(graph, EngineOptions{});
            const Activation& act = last_act(paths);
            CHECK(act.sp_meter() == kOverfillCap);
            CHECK(act.timecode.ticks() == kOverfillActTick);
            CHECK(act.sp_end_steps == want);

            // The clamp note is the last phrase whose step clamped, and each
            // step's end is anchored on the last clamp at or before it, or on
            // the activation before any clamp (SP-40).
            std::optional<int64_t> last_clamp;
            for (size_t i = 0; i < want.size(); ++i) {
                CAPTURE(i);
                if (want[i].kind == SpEndKind::Clamped) last_clamp = want[i].tick;
                CHECK(act.end_anchor_tick(i) == last_clamp.value_or(kOverfillActTick));
            }
            CHECK(act.clamp_tick() == last_clamp);
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
        CAPTURE(path);
        const Song& song = corpus_song_with_notes(path, ChartReading{});
        ScoreGraph graph(song, 4);
        for (const Path& p : run_search(graph, EngineOptions{DepthMode::Scores, 1})) {
            for (const Activation& act : p.all_activations()) {
                bool took_sqout = false;
                for (const SPSqueeze& sq : act.sqinouts)
                    if (sq.kind == SqueezeKind::SqOut) took_sqout = true;
                // sqout_tick is set exactly when the activation squeezed out.
                CHECK(act.sqout_tick.has_value() == took_sqout);
                if (act.sqout_tick) ++sqouts_seen;

                // A squeezed-out phrase, and anything after it, was never
                // collected. (Ascending order from the activation is checked
                // by "SP end history: every corpus activation is consistent".)
                if (act.sqout_tick)
                    for (int64_t t : act.collected_phrase_ticks()) CHECK(t < *act.sqout_tick);
            }
        }
    }
    CHECK(sqouts_seen > 0);
}

TEST_CASE("SP end history: a late squeeze-in is a SqIn step on its phrase") {
    // The late squeeze-in song (record_fixtures.h): two banked bars, the fill
    // at act_tick activates, and the phrase at sqin_phrase sits just after
    // the end that gives. Squeezed in, it is a SqIn step on its own tick that
    // moves the end one bar on from the old end, and the squeeze is measured
    // from that old end.
    const int64_t act_tick = kBeatActTick;
    const int64_t sqin_phrase = 13920;
    Song song = test::make_late_sqin_song();
    const int64_t old_end = beat_song_end(song);
    const int64_t new_end = end_after_bars(song, old_end, 1);
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {act_tick, old_end, SpEndKind::Activation},
               {sqin_phrase, new_end, SpEndKind::SqIn}}));
    REQUIRE(act.sqinouts.size() == 1);
    CHECK(act.sqinouts[0].offset_ms ==
          doctest::Approx(offset_from_sp_end(tick_ms(song, sqin_phrase), tick_ms(song, old_end))));
    CHECK(act.sqinouts[0].offset_ms > 0.0);
    CHECK(act.squeeze_end_tick(0) == std::optional<int64_t>(old_end));
}

TEST_CASE("SP end history: an early squeeze-in measures from the end before it") {
    // The finding-27 song (record_fixtures.h): two banked bars, the fill at
    // act_tick activates and gives the first end X. The phrase at sqin_phrase
    // sits just before X and is squeezed in (one bar on from X), then the
    // phrase at collected_phrase is collected mid-SP (one bar on again). The
    // tempo halves between the two, so the final end minus one bar is not X.
    const int64_t act_tick = 2304;
    const int64_t banked_bars = 2;
    const int64_t sqin_phrase = 5280;
    const int64_t collected_phrase = 6144;
    Song song = test::sqin_then_collect_song();
    const int64_t x = end_after_bars(song, act_tick, banked_bars);
    const int64_t after_sqin = end_after_bars(song, x, 1);
    const int64_t after_collect = end_after_bars(song, after_sqin, 1);
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const Activation* act = test::find_act(paths, [](const Activation& a) {
        for (const SPSqueeze& s : a.sqinouts)
            if (s.kind == SqueezeKind::SqIn) return true;
        return false;
    });
    REQUIRE(act != nullptr);
    CHECK((act->sp_end_steps == std::vector<SpEndStep>{
               {act_tick, x, SpEndKind::Activation},
               {sqin_phrase, after_sqin, SpEndKind::SqIn},
               {collected_phrase, after_collect, SpEndKind::Collected}}));
    REQUIRE(act->sqinouts.front().kind == SqueezeKind::SqIn);
    CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(x));
    REQUIRE(act->squeeze_end_step(0).has_value());
    // No clamp, so the end is anchored on the activation.
    CHECK(act->end_anchor_tick(*act->squeeze_end_step(0)) == act_tick);
    CHECK(act->sqinouts.front().offset_ms ==
          doctest::Approx(offset_from_sp_end(tick_ms(song, sqin_phrase), tick_ms(song, x))));
    CHECK(act->sqinouts.front().offset_ms < 0.0);
}

TEST_CASE("SP end history: a squeeze-out measures from the deact node") {
    // The early squeeze-out song (record_fixtures.h): two banked bars, the
    // fill at act_tick activates, and the phrase at sqout_phrase, inside the
    // first window, is squeezed out. It leaves no step: the history keeps
    // only the activation's, so the end a SqOut is measured from is the
    // activation's own end (not the next phrase's extension), anchored on the
    // activation.
    const int64_t act_tick = kBeatActTick;
    const int64_t sqout_phrase = 12960;
    const int64_t second_fill = 17280;
    Song song = test::make_early_sqout_song();
    const int64_t act_end = beat_song_end(song);
    ScoreGraph graph(song, 4);
    // The best path collects the phrase instead and keeps SP running over the
    // second fill, so search wide and pick the activation that squeezed out.
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{act_tick, second_fill};
    const std::vector<Path> paths = run_search(graph, opts);
    const Activation* act = test::find_act(
        paths, [](const Activation& a) { return a.sqout_tick.has_value(); });
    REQUIRE(act != nullptr);
    CHECK(act->timecode.ticks() == act_tick);
    CHECK(act->sqout_tick == std::optional<int64_t>(sqout_phrase));
    CHECK((act->sp_end_steps ==
           std::vector<SpEndStep>{{act_tick, act_end, SpEndKind::Activation}}));
    size_t sqouts = 0;
    for (size_t i = 0; i < act->sqinouts.size(); ++i) {
        if (act->sqinouts[i].kind != SqueezeKind::SqOut) continue;
        ++sqouts;
        CHECK(act->squeeze_end_tick(i) == std::optional<int64_t>(act_end));
        REQUIRE(act->squeeze_end_step(i).has_value());
        CHECK(act->end_anchor_tick(*act->squeeze_end_step(i)) == act_tick);
    }
    CHECK(sqouts == 1);
}

// A transfer scale is how much longer a measure lasts at the SP end than at
// the note the end is anchored on. Every section in these songs is 4/4, so
// that is the anchor's tempo over the end's (SongTiming::tempo_at), with no
// rounding of its own: the 1e-12 relative gap leaves room only for the last
// bits of the engine's ms arithmetic.
namespace {
double tempo_scale(const Song& song, int64_t anchor_tick, int64_t end_tick) {
    return song.timing().tempo_at(anchor_tick) / song.timing().tempo_at(end_tick);
}
}  // namespace

TEST_CASE("search: scales anchor on the collecting note and the SqIn's own end") {
    SUBCASE("a cap clamp: the scale is measured from the collecting note") {
        // The finding-28 song at cap 2 (record_fixtures.h): the meter is full
        // when the fill at act_tick activates, so collecting the phrase at
        // clamp_phrase pins the end a full meter past that phrase. The tempo
        // changes between the activation and the phrase.
        const int cap = 2;
        const int64_t act_tick = 2304;
        const int64_t clamp_phrase = 3072;
        Song song = test::clamp_song();
        const int64_t end = end_after_bars(song, clamp_phrase, cap);
        ScoreGraph graph(song, cap);
        std::vector<Path> paths = run_search(graph, test::wide_search());
        const Activation* act = test::find_act(
            paths, [&](const Activation& a) { return a.timecode.ticks() == act_tick; });
        REQUIRE(act != nullptr);
        CHECK(act->clamp_tick() == std::optional<int64_t>(clamp_phrase));
        CHECK(act->deact_tick() == std::optional<int64_t>(end));
        // From the activation instead, the scale would differ.
        REQUIRE(tempo_scale(song, act_tick, end) != tempo_scale(song, clamp_phrase, end));
        REQUIRE(act->transfer_post.has_value());
        const double want = tempo_scale(song, clamp_phrase, end);
        CHECK(act->transfer_post->late == doctest::Approx(want).epsilon(1e-12));
        CHECK(act->transfer_post->early == doctest::Approx(want).epsilon(1e-12));
    }
    SUBCASE("a SqIn: its scale ends at the end before it, the activation's at D") {
        // The finding-27 song: the SqIn is measured from the activation to
        // the first end X, the whole activation from the activation to D,
        // and the tempo changes between X and D.
        const int64_t act_tick = 2304;
        const int64_t banked_bars = 2;
        Song song = test::sqin_then_collect_song();
        const int64_t x = end_after_bars(song, act_tick, banked_bars);
        ScoreGraph graph(song, 4);
        std::vector<Path> paths = run_search(graph, test::wide_search());
        const Activation* act = test::find_act(paths, [](const Activation& a) {
            return !a.sqinouts.empty() && a.sqinouts.front().kind == SqueezeKind::SqIn;
        });
        REQUIRE(act != nullptr);
        const std::optional<int64_t> d = act->deact_tick();
        REQUIRE(d.has_value());
        REQUIRE(tempo_scale(song, act_tick, x) != tempo_scale(song, act_tick, *d));
        CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(x));
        CHECK(act->squeeze_anchor_tick(0) == std::optional<int64_t>(act_tick));
        REQUIRE(act->sqinouts.front().transfer.has_value());
        const double sqin_want = tempo_scale(song, act_tick, x);
        CHECK(act->sqinouts.front().transfer->late == doctest::Approx(sqin_want).epsilon(1e-12));
        CHECK(act->sqinouts.front().transfer->early == doctest::Approx(sqin_want).epsilon(1e-12));
        REQUIRE(act->transfer_post.has_value());
        CHECK(act->transfer_post->late ==
              doctest::Approx(tempo_scale(song, act_tick, *d)).epsilon(1e-12));
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
        CAPTURE(path);
        const Song& song = corpus_song_with_notes(path, ChartReading{});
        ScoreGraph graph(song, 4);
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
                // The row's offset, measured again from D (the SP end every
                // row is measured against), not read back off the row. Both
                // sides are the same subtraction, so they match exactly.
                const std::optional<int64_t> d = act.deact_tick();
                REQUIRE(d.has_value());
                const double from_d = offset_from_sp_end(
                    row->timecode.ms(), song.timing().timecode(*d).ms());
                CHECK(*row->offset_ms == from_d);
            }
        }
    }
    CHECK(sqouts > 0);
    MESSAGE("checked " << sqouts << " squeeze-outs");
}

// A squeeze window can reach back past the activation when 500 ms spans more
// than one SP bar. The phrases there were banked before SP started, so the
// activation cannot squeeze them in or out.
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
            // The history is never empty, and SP ends where the banked bars
            // put it.
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

// The lasting check on every corpus activation's history
// (docs/adr/0021-the-sp-end-history-is-stored.md).
TEST_CASE("SP end history: every corpus activation is consistent") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int acts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        corpus_song_with_notes(chart, cfg.reading(), cfg.rules);
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            for (const Activation& act : p->walk_activations()) {
                CAPTURE(chart);
                CAPTURE(p->pathstring());
                CAPTURE(act.timecode.ticks());
                ++acts;
                // No fresh record leaves the history out.
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
                }
                // D34: one SqIn step per SqIn, never repeated or squeezed out
                // (tests/bank_check.h).
                bank_check::check_one_step_per_sqin(act);
            }
        }
    }
    // Every chart was visited (none passes over quietly above); this only
    // says the walk reached at least one activation.
    CHECK(acts > 0);
}

TEST_CASE("Bank: a squeezed-out bar arrives on the squeezed chord") {
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
    // The squeezed-out phrase sits before the deact node (an early
    // squeeze-out), and its bar arrives on its own chord, then the phrase at
    // 14400. Both ticks are read off the record.
    const Activation& first = path.activations[0];
    REQUIRE(first.sqout_tick.has_value());
    REQUIRE(first.deact_tick().has_value());
    REQUIRE(*first.sqout_tick < *first.deact_tick());
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{*first.sqout_tick, 14400}));
    CHECK(path.trailing_bank_ticks.empty());
}

// On every stored path of every corpus chart, tied variants and all-zero paths
// included: after a window that squeezed out, the next window's first stored
// bar (or the path's first trailing bar, after the last window) is that
// window's own squeezed chord, for an early squeeze-out and a late one alike
// (docs/clone-hero/05-star-power.md, SP-35). Early or late is core::after_sp_end
// against the deact node. Path::bank_arrivals hands the stored lists back as
// they are.
TEST_CASE("Bank: after every corpus squeeze-out the next bar is stored on the squeezed chord") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    // early: the chord is before the deact node, where a stamp on the node
    // would differ. on_node: the chord is the node itself. late: after it.
    int early = 0, on_node = 0, late = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            const ActivationWalk acts = p->walk_activations();
            for (size_t i = 0; i <= acts.size(); ++i) {
                CAPTURE(i);
                const std::vector<int64_t>& stored =
                    i == acts.size() ? p->trailing_bank_ticks : acts[i].bank_rise_ticks;
                CHECK(p->bank_arrivals(i) == stored);
                if (i == 0 || !acts[i - 1].sqout_tick) continue;
                const Activation& prev = acts[i - 1];
                CAPTURE(prev.timecode.ticks());
                const std::optional<int64_t> d = prev.deact_tick();
                REQUIRE(d.has_value());
                const int64_t chord = *prev.sqout_tick;
                CAPTURE(chord);
                CAPTURE(*d);
                if (core::after_sp_end(chord, *d)) ++late;
                else if (chord < *d) ++early;
                else ++on_node;
                REQUIRE_FALSE(stored.empty());
                CHECK(stored.front() == chord);
            }
        }
    }
    // An empty loop must not pass: the corpus holds both kinds.
    CHECK(early > 0);
    CHECK(late > 0);
    MESSAGE("checked " << early << " early, " << on_node << " on the node and " << late
                       << " late squeeze-outs");
}

// The lasting order checks (tests/bank_check.h), on every path, tied variants
// included (D3, finding 89). A variant's walk is its own windows, then its
// leader's from the fold on, so its banked bars must still fall between its
// own windows. The fixture case above pins the exact ticks.
TEST_CASE("Bank: every corpus path banks in order") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int acts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus_song_with_notes(chart, cfg.reading(), cfg.rules);
        acts += bank_check::check_record_banks(song, corpus::analyzed(chart, cfg));
    }
    // Every chart was visited (none passes over quietly above); this only
    // says the walk reached at least one activation.
    CHECK(acts > 0);
}

// A deactivation edge keeps no rows of its own: its rows are a range of the
// graph's one row list, read through ScoreGraph::edge_backends. The graph
// stores one row per chart timestamp (ScoreGraph::build), and an edge's range
// holds the rows inside the squeeze window of its SP end
// (core::within_squeeze_window, the one window check). On every corpus chart.
TEST_CASE("graph: a deactivation edge's rows are a range of the graph's rows") {
    size_t edges = 0;
    for (const std::string& path : corpus::chart_paths()) {
        CAPTURE(path);
        const Song& song = corpus_song_with_notes(path, ChartReading{});
        const ScoreGraph graph(song, 4);
        const size_t all = graph.all_backends().size();
        CHECK(all == song.sequence.size());

        std::set<const ScoreGraphEdge*> seen;
        for (const ScoreGraphNode* sp = test::sp_track_start(graph); sp;
             sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
            const ScoreGraphEdge* e = sp->branch_edge;
            if (!e || !seen.insert(e).second) continue;
            ++edges;
            CAPTURE(e->dest->timecode.ticks());
            CHECK(e->backend_begin <= e->backend_end);
            CHECK(e->backend_end <= static_cast<int32_t>(all));
            for (const BackendSqueeze& row : graph.edge_backends(*e)) {
                CAPTURE(row.timecode.ticks());
                REQUIRE(row.offset_ms.has_value());
                CHECK(within_squeeze_window(*row.offset_ms));
            }
        }
    }
    CHECK(edges > 0);
}
