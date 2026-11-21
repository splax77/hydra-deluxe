// Fills the search spawns, passes over or refuses: a later-ready path that an
// upcoming fill still spawns for, and the skipped fills each activation
// stores under the 1.0 and 1.1 deadline rules, and the fills a path passed
// over after its last activation. The deadline math itself is pinned in
// test_fill_deadline.cpp.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/preview_view.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/timing.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"
#include "search/pathstring_read.h"

using namespace hydra;

// The chords of a song that end a fill (SongTimestamp::activation_length is
// set), in chart order.
static std::vector<const SongTimestamp*> fill_chords(const Song& song) {
    std::vector<const SongTimestamp*> out;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) out.push_back(&ts);
    return out;
}

// The fill chord at `tick`, or nullptr when no fill ends there.
static const SongTimestamp* fill_chord_at(const Song& song, int64_t tick) {
    for (const SongTimestamp* ts : fill_chords(song))
        if (ts->timecode.ticks() == tick) return ts;
    return nullptr;
}

// Two paths with the same SP meter are not interchangeable when an upcoming
// fill spawns for one and refuses the other (Clone Hero's early-fill rule:
// SP must be ready by the fill's deadline). The later-ready path leads on
// score here; it may not knock out the earlier-ready one, whose activation at
// that fill is the best path. Path A activates at the chart's first fill;
// path B passes it over and is the one the chart's last fill spawns for.
TEST_CASE("search: a later-ready path never knocks out one an upcoming fill still spawns for") {
    Song song = test::ready_time_fold_song();
    const std::vector<const SongTimestamp*> fills = fill_chords(song);
    REQUIRE(fills.size() >= 2);
    const int64_t first_fill = fills.front()->timecode.ticks();
    const int64_t last_fill = fills.back()->timecode.ticks();
    ScoreGraph graph(song, 2);
    std::vector<Path> best;
    REQUIRE_NOTHROW(best = run_search(graph, EngineOptions{}));
    REQUIRE_FALSE(best.empty());

    EngineOptions target;
    target.target_act_ticks = std::vector<int64_t>{last_fill};
    std::vector<Path> late;
    REQUIRE_NOTHROW(late = run_search(graph, target));
    REQUIRE_FALSE(late.empty());

    // A, which activated at the first fill, cannot spawn the last one.
    EngineOptions a_then_late;
    a_then_late.target_act_ticks = std::vector<int64_t>{first_fill, last_fill};
    CHECK_THROWS(run_search(graph, a_then_late));

    CAPTURE(best.front().pathstring());
    CHECK(test::act_ticks(best.front()) == std::vector<int64_t>{last_fill});
    CHECK(best.front().totalscore() == late.front().totalscore());
}

// Under the 1.0 rule a fill that ends later can have the earlier deadline.
// make_ch10_fill_song banks exactly an activation's worth of SP, so SP comes
// ready at its last phrase before the activation at the chart's last fill.
// The activation stores every earlier fill the rule shows (its deadline, less
// the early-fill window, not before SP came ready) and none it refuses.
TEST_CASE("Skipped fills: the 1.0 rule's offered fill is the one stored") {
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    const std::vector<const SongTimestamp*> fills = fill_chords(song);
    REQUIRE(!fills.empty());
    const int64_t act_tick = fills.back()->timecode.ticks();

    double ready_ms = -1.0;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.flag_sp && ts.timecode.ticks() < act_tick) ready_ms = ts.timecode.ms();
    REQUIRE(ready_ms >= 0.0);

    std::vector<int64_t> shown, refused;
    for (const SongTimestamp* f : fills) {
        const int64_t tick = f->timecode.ticks();
        if (tick >= act_tick) continue;
        const double deadline =
            activation_fill_deadline_ms(song.timing(), tick, *f->activation_length, FillDeadlineRule::Ch10);
        const double e_offset = fill_e_offset(deadline, ready_ms);
        (fill_refuses(e_offset, graph.early_fill_window_ms()) ? refused : shown).push_back(tick);
    }
    // Finding 52's case: one fill shown and passed over, and a later-ending
    // fill, nearer the activation, that never spawned.
    REQUIRE(shown.size() == 1);
    REQUIRE(!refused.empty());
    REQUIRE(refused.back() > shown.front());

    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{act_tick};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK(act.skipped_fill_ticks == shown);
}

// ---- the early-fill fixture -----------------------------------------------
// 125 BPM at 480 ticks a beat makes a tick one millisecond. A note on every
// beat up to kEfChartEnd. The first phrase (kEfFirstPhrase) banks a bar; a
// second phrase at `ready` banks the other. Fill A ends at kEfFillA and is
// kEfFillLength long. With `second_fill`, fill C ends at kEfFillC and three
// more notes follow it inside that fill's Star Power, so a path that passes A
// over and activates at C scores more than one that activates at A.
constexpr int kEfResolution = 480;
constexpr double kEfBpm = 125.0;
constexpr int64_t kEfFirstPhrase = 480;
constexpr int64_t kEfFillA = 9600;
constexpr int64_t kEfFillC = 19200;
constexpr int64_t kEfFillLength = 480;
constexpr int64_t kEfChartEnd = 28800;

// How far from fill A's deadline SP comes ready in the window cases: between
// the 60 ms default and the 85 ms widest window, so one input splits them.
constexpr double kBetweenWindowsMs = 70.0;
static_assert(kDefaultEarlyFillWindowMs < kBetweenWindowsMs && kBetweenWindowsMs < kHitWindowMs,
              "the input must sit between the two windows");

static Song early_fill_song(int64_t ready, bool second_fill = false) {
    std::vector<test::FixtureNote> notes;
    for (int64_t t = 0; t <= kEfChartEnd; t += kEfResolution) {
        test::FixtureNote n{t};
        n.phrase = t == kEfFirstPhrase;
        if (t == kEfFillA || (second_fill && t == kEfFillC)) n.fill_length = kEfFillLength;
        notes.push_back(n);
    }
    if (second_fill)
        for (const int64_t t : {kEfFillC + 240, kEfFillC + 720, kEfFillC + 1200}) notes.push_back({t});
    notes.push_back({ready, true, 0});
    std::sort(notes.begin(), notes.end(),
              [](const test::FixtureNote& a, const test::FixtureNote& b) { return a.tick < b.tick; });
    Song song = test::build_fixture_song(kEfResolution, kEfBpm, notes);
    REQUIRE(song.timecode(ready).ms() == static_cast<double>(ready));
    return song;
}

// Fill A's 1.1 deadline, from the rule's owner (test_fill_deadline.cpp pins
// its math). It reads only the tempo map, which every early_fill_song shares.
static double fill_a_deadline_ms() {
    const Song timing_only = test::build_fixture_song(kEfResolution, kEfBpm, {});
    return activation_fill_deadline_ms(timing_only.timing(), kEfFillA, kEfFillLength,
                                       FillDeadlineRule::Ch11);
}

// The fixture's tick at `ms`: a tick is one millisecond, and the ms must land
// on a tick.
static int64_t ef_tick_at(double ms) {
    const int64_t tick = static_cast<int64_t>(ms);
    REQUIRE(static_cast<double>(tick) == ms);
    return tick;
}

// The 1.1 rule at the exact edge. A fill spawns when SP is ready no later
// than the early-fill window past its deadline (fill_refuses). Fill C is the
// one taken. With SP ready exactly the window late, A is shown and passed
// over; one tick later it never spawns. Run at the widest window and at the
// default, so a graph that dropped the window it was given fails one of them.
TEST_CASE("Skipped fills: under the 1.1 rule a fill exactly at the reach edge is stored") {
    auto passed_over = [](int64_t ready, double window_ms) {
        const Song song = early_fill_song(ready, /*second_fill=*/true);
        ScoreGraph graph(song, 4, FillDeadlineRule::Ch11, core::default_rules(), window_ms);
        EngineOptions opts;
        opts.target_act_ticks = std::vector<int64_t>{kEfFillC};
        const std::vector<Path> paths = run_search(graph, opts);
        REQUIRE(!paths.empty());
        REQUIRE(paths.front().activations.size() == 1);
        REQUIRE(paths.front().activations.front().timecode.ticks() == kEfFillC);
        return paths.front().activations.front().skipped_fill_ticks;
    };
    const double deadline = fill_a_deadline_ms();
    for (const double window : {kHitWindowMs, kDefaultEarlyFillWindowMs}) {
        CAPTURE(window);
        const int64_t edge = ef_tick_at(deadline + window);
        CHECK((passed_over(edge, window) == std::vector<int64_t>{kEfFillA}));
        CHECK(passed_over(edge + 1, window).empty());
    }
}

// The search settings these cases run under: the app's search defaults with
// the early-fill window set to `window_ms`.
static SearchSettings window_settings(double window_ms) {
    SearchSettings s;
    s.early_fill_window_ms = window_ms;
    return s;
}

// Every activation of a path and its tied variants carries the window its
// search ran under (Activation::e_window_ms).
static void check_window_carried(const Path& p, double window_ms) {
    for (const Activation& a : p.walk_activations()) CHECK(a.e_window_ms == window_ms);
    for (const Path& v : p.variants) check_window_carried(v, window_ms);
}

static void check_window_carried(const HydraRecord& record, double window_ms) {
    CHECK(record.early_fill_window_ms == window_ms);
    for (const Path& p : record.paths) check_window_carried(p, window_ms);
    for (const Path& p : record.allzero_paths) check_window_carried(p, window_ms);
}

// Does any path of the record, tied variants included, activate at `tick`?
static bool activates_at(const HydraRecord& record, int64_t tick) {
    for (const Path* p : record.all_paths())
        for (const Activation& a : p->walk_activations())
            if (a.timecode.ticks() == tick) return true;
    return false;
}

// An orange early fill (D250): SP comes ready kBetweenWindowsMs after fill A's
// deadline, so the activation needs an early hit that long. With the window
// at 85 ms the search takes it, and the record stores it as a required early
// fill (E0) whose difficulty is how late SP came ready
// (Activation::e_difficulty, which the stored hardest_efill reads). At the 60
// ms default the fill never spawns, so no path activates there. Every route
// to the engine is read: analyze_chart, search_target and
// paths_from_pathstring, each on the graph search_graph builds.
TEST_CASE("search: a fill that needs a 70 ms early hit is taken at an 85 ms window, not at 60") {
    const double deadline = fill_a_deadline_ms();
    const int64_t ready = ef_tick_at(deadline + kBetweenWindowsMs);
    const Song song = early_fill_song(ready);
    const double ready_ms = song.timecode(ready).ms();

    const HydraRecord wide = analyze_chart(song, window_settings(kHitWindowMs));
    REQUIRE(!wide.paths.empty());
    CAPTURE(wide.best_path().pathstring());
    REQUIRE(test::act_ticks(wide.best_path()) == std::vector<int64_t>{kEfFillA});
    const Activation& act = wide.best_path().activations.front();
    CHECK(act.is_E0());
    CHECK(act.e_difficulty() == std::optional<double>{ready_ms - deadline});
    check_window_carried(wide, kHitWindowMs);

    const HydraRecord narrow = analyze_chart(song, window_settings(kDefaultEarlyFillWindowMs));
    CHECK_FALSE(activates_at(narrow, kEfFillA));
    check_window_carried(narrow, kDefaultEarlyFillWindowMs);

    // search_target on the graph search_graph builds: the pinned activation
    // is realizable at 85 and not at 60, and the 60 search names fill A as
    // the activation it could not place.
    const ScoreGraph wide_graph = search_graph(song, window_settings(kHitWindowMs));
    const TargetResult wide_pin = search_target(wide_graph, activation_pins({kEfFillA}));
    REQUIRE(wide_pin.paths.size() >= 1);
    for (const Path& p : wide_pin.paths) {
        CHECK(test::act_ticks(p) == std::vector<int64_t>{kEfFillA});
        check_window_carried(p, kHitWindowMs);
    }
    const ScoreGraph narrow_graph = search_graph(song, window_settings(kDefaultEarlyFillWindowMs));
    const TargetResult narrow_pin = search_target(narrow_graph, activation_pins({kEfFillA}));
    CHECK(narrow_pin.paths.empty());
    CHECK(narrow_pin.failed_tick == std::optional<int64_t>{kEfFillA});

    // The path text read back: it places at 85, and at 60 the reader refuses
    // it as text that names no path on the chart.
    const std::string text = wide.best_path().pathstring();
    const Path read = paths_from_pathstring_on_graph(song, wide_graph, text);
    CHECK(test::act_ticks(read) == std::vector<int64_t>{kEfFillA});
    check_window_carried(read, kHitWindowMs);
    try {
        paths_from_pathstring_on_graph(song, narrow_graph, text);
        FAIL("the 60 ms graph placed the path text");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::PathTextRefused);
    }
}

// The white side of the same window: SP comes ready kBetweenWindowsMs before
// fill A's deadline. Under an 85 ms window the activation is E-critical and
// its early fill is listed with that much to spare; under the 60 ms default
// it is a plain activation with no early fill.
TEST_CASE("search: SP ready 70 ms before a fill's deadline marks the activation E at 85, not at 60") {
    const double deadline = fill_a_deadline_ms();
    const int64_t ready = ef_tick_at(deadline - kBetweenWindowsMs);
    const Song song = early_fill_song(ready);
    const double ready_ms = song.timecode(ready).ms();
    for (const double w : {kHitWindowMs, kDefaultEarlyFillWindowMs}) {
        CAPTURE(w);
        const HydraRecord record = analyze_chart(song, window_settings(w));
        REQUIRE(!record.paths.empty());
        CAPTURE(record.best_path().pathstring());
        REQUIRE(test::act_ticks(record.best_path()) == std::vector<int64_t>{kEfFillA});
        const Activation& act = record.best_path().activations.front();
        CHECK(act.e_offset == fill_e_offset(deadline, ready_ms));
        const bool wide = w == kHitWindowMs;
        CHECK(act.is_e_critical() == wide);
        CHECK(act.is_E0() == wide);
        CHECK(act.e_difficulty().has_value() == wide);
        // The figure is how late SP came ready, here negative: time to spare.
        if (wide) CHECK(act.e_difficulty() == std::optional<double>{ready_ms - deadline});
        check_window_carried(record, w);
    }
}

// The all-0 pass reads the window too. SP comes ready kBetweenWindowsMs
// before fill A's deadline, and fill C scores more. Passing A over keeps a
// path all-0 only when A was its early fill (an E1, allzero_activation): so
// at 85 the best all-0 path activates at C, and at 60, where A is a plain
// fill, it activates at A. The all-0 answer is read through its one reader,
// best_allzero_paths (search/pather.h).
TEST_CASE("all-0 pass: passing over a fill with 70 ms to spare is an E1 at 85, a skip at 60") {
    const int64_t ready = ef_tick_at(fill_a_deadline_ms() - kBetweenWindowsMs);
    const Song song = early_fill_song(ready, /*second_fill=*/true);

    // At 85 the best path passes A over as an E1, so it is the all-0 answer.
    const HydraRecord wide = analyze_chart(song, window_settings(kHitWindowMs));
    REQUIRE(!wide.paths.empty());
    CAPTURE(wide.best_path().pathstring());
    CHECK(test::act_ticks(wide.best_path()) == std::vector<int64_t>{kEfFillC});
    const std::vector<const Path*> wide_allzero = best_allzero_paths(wide);
    REQUIRE(!wide_allzero.empty());
    CHECK(wide_allzero.front() == &wide.best_path());
    check_window_carried(wide, kHitWindowMs);

    // At 60 passing A over is a plain skip, so the best path is not all-0 and
    // the all-0 answer is another path, one that activates at A.
    const HydraRecord narrow = analyze_chart(song, window_settings(kDefaultEarlyFillWindowMs));
    REQUIRE(!narrow.paths.empty());
    CAPTURE(narrow.best_path().pathstring());
    const std::vector<const Path*> narrow_allzero = best_allzero_paths(narrow);
    REQUIRE(!narrow_allzero.empty());
    CAPTURE(narrow_allzero.front()->pathstring());
    CHECK(narrow_allzero.front() != &narrow.best_path());
    CHECK(test::act_ticks(*narrow_allzero.front()) == std::vector<int64_t>{kEfFillA});
    check_window_carried(narrow, kDefaultEarlyFillWindowMs);
}

// The song of a corpus chart under `cfg`. Every chart the corpus scan lists
// has Expert drum notes, so a chart that loads empty fails here by name
// instead of being passed over.
static const Song& song_with_notes(const std::string& chart, const app::AnalysisSettings& cfg) {
    const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
    REQUIRE_MESSAGE(!song.is_empty(), chart << " loads with no notes");
    return song;
}

// The lasting order checks, on root paths. A tied variant's list is checked
// by lone pricing (test::lone_pricing in record_fixtures.h).
TEST_CASE("Skipped fills: every corpus root passes over real fills in order") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int skipped = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        for (const Path& root : corpus::analyzed(chart, cfg).paths) {
            const Activation* prev = nullptr;
            for (const Activation& act : root.walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                // A fill is stored as passed over only when it spawned: its
                // e_offset, which the activation keeps for the first fill it
                // passed over, is not refused under the window the search ran.
                if (!act.skipped_fill_ticks.empty()) {
                    CHECK(act.e_window_ms == cfg.early_fill_window_ms);
                    CHECK_FALSE(fill_refuses(act.e_offset, act.e_window_ms));
                }
                for (size_t k = 0; k < act.skipped_fill_ticks.size(); ++k) {
                    const int64_t t = act.skipped_fill_ticks[k];
                    ++skipped;
                    CHECK(t < act.timecode.ticks());
                    if (prev) CHECK(t > prev->timecode.ticks());
                    if (k > 0) CHECK(t > act.skipped_fill_ticks[k - 1]);
                    CHECK(fill_chord_at(song, t) != nullptr);
                }
                prev = &act;
            }
        }
    }
    // With no skipped fill the checks above ran on nothing, so the test fails.
    CHECK_MESSAGE(skipped > 0, "no corpus root path passed over a fill");
}

// ---- the fills a path passed over after its last activation -------------
// Path::trailing_skipped_fill_ticks. A best path tends to spend what it
// banks, so the corpus cases read the top 40 scores at cap 4
// (test::scores_settings), whose lower-scoring paths can leave fills behind.
// Each case fails when the corpus gives it nothing to check.

namespace {

// Each listed tick comes after the path's last activation, in chart order, and
// is a fill chord of the chart. The path had enough Star Power saved by then
// (Path::trailing_sp_ready_tick, the engine's stored arrival), and the fill
// spawned under the window `cfg` searched under (fill_refuses on that ready
// moment and the fill's deadline). The ready tick's ms is the engine's own:
// the search reads the same note's Timecode the bar's arrival tick comes from.
void check_trailing(const Song& song, const Path& p, const app::AnalysisSettings& cfg) {
    const ActivationWalk walk = p.walk_activations();
    const int64_t last_act = walk.size() > 0 ? walk[walk.size() - 1].timecode.ticks() : -1;
    const std::vector<int64_t>& listed = p.trailing_skipped_fill_ticks;
    const std::optional<int64_t> ready = p.trailing_sp_ready_tick();
    if (!listed.empty()) REQUIRE(ready.has_value());
    for (size_t k = 0; k < listed.size(); ++k) {
        const int64_t t = listed[k];
        CAPTURE(t);
        CHECK(t > last_act);
        if (k > 0) CHECK(t > listed[k - 1]);
        const SongTimestamp* chord = fill_chord_at(song, t);
        REQUIRE(chord != nullptr);
        CHECK(*ready <= t);
        const double deadline = activation_fill_deadline_ms(song.timing(), t, *chord->activation_length,
                                                            fill_rule_for(cfg.legacy_fills));
        const double ready_ms = song.timecode(*ready).ms();
        CHECK_FALSE(fill_refuses(fill_e_offset(deadline, ready_ms), cfg.early_fill_window_ms));
    }
}

}  // namespace

TEST_CASE("trailing fills: a corpus path lists the fills after its last activation that the meter allows") {
    const app::AnalysisSettings cfg = test::scores_settings(4);
    // Every corpus path, tied variants included.
    int listed = 0;
    for (const std::string& c : corpus::chart_paths()) {
        const Song& s = song_with_notes(c, cfg);
        for (const Path* p : corpus::analyzed(c, cfg).all_paths()) {
            CAPTURE(c);
            CAPTURE(p->pathstring());
            listed += static_cast<int>(p->trailing_skipped_fill_ticks.size());
            check_trailing(s, *p, cfg);
        }
    }
    // With no listed fill the checks above ran on nothing.
    CHECK_MESSAGE(listed > 0, "no corpus path at cap 4 lists a fill after its last activation");
}

TEST_CASE("trailing fills: a path whose last window outlasts the chart lists none") {
    // Its Star Power is still running at the chart's last note, so no fill
    // after its last activation was passed over. Every such corpus path.
    const app::AnalysisSettings cfg = test::scores_settings(4);
    int outlasting = 0, with_fills_after = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        const int64_t last_note = song.sequence.back().timecode.ticks();
        for (const Path* p : corpus::analyzed(chart, cfg).all_paths()) {
            const ActivationWalk walk = p->walk_activations();
            if (walk.size() == 0) continue;
            const Activation& last = walk[walk.size() - 1];
            if (last.deact_tick().value_or(0) <= last_note) continue;
            ++outlasting;
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            CHECK(p->trailing_skipped_fill_ticks.empty());
            for (const SongTimestamp* f : fill_chords(song))
                if (f->timecode.ticks() > last.timecode.ticks()) {
                    ++with_fills_after;
                    break;
                }
        }
    }
    // The empty-list check above ran only on outlasting paths; one whose
    // window runs over fills of the chart's own is the case it is for.
    CHECK_MESSAGE(outlasting > 0, "no corpus path at cap 4 has a window that outlasts its chart");
    CHECK_MESSAGE(with_fills_after > 0,
                  "no outlasting corpus path runs over a fill after its last activation");
}

// A pinned search of a path's own windows lists the same trailing fills as
// the free search did, and the pinned list meets the same rule.
TEST_CASE("trailing fills: a full pin of a corpus path agrees with the free search") {
    const app::AnalysisSettings cfg = test::scores_settings(4);
    int compared = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = song_with_notes(chart, cfg);
        for (const Path* p : corpus::analyzed(chart, cfg).all_paths()) {
            if (p->trailing_skipped_fill_ticks.empty()) continue;
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            const TargetResult t = search_target(song, cfg, pinned_windows(*p));
            REQUIRE(t.paths.size() == 1);
            ++compared;
            check_trailing(song, t.paths[0], cfg);
            CHECK(t.paths[0].trailing_skipped_fill_ticks == p->trailing_skipped_fill_ticks);
        }
    }
    CHECK_MESSAGE(compared > 0, "no corpus path at cap 4 lists a fill to compare");
}

// The empty path of the Path Builder's start: search_target over no windows
// declines every fill, so it lists exactly the fills the game shows. A fill
// is shown when the meter held an activation's worth by its deadline, late by
// no more than the early-fill window (fill_refuses).
TEST_CASE("trailing fills: the path with no activation lists every fill the meter allows") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int listed = 0, unlisted = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = song_with_notes(chart, cfg);
        const TargetResult t = search_target(song, cfg, std::vector<PinnedWindow>{});
        REQUIRE(t.paths.size() == 1);
        const Path& empty = t.paths[0];
        CHECK(empty.walk_activations().size() == 0);
        check_trailing(song, empty, cfg);
        const app::PreviewScene scene = app::build_preview_scene(song, &empty, cfg.sp_cap, cfg.rules);
        const std::vector<int64_t>& list = empty.trailing_skipped_fill_ticks;
        for (const app::PreviewFill& f : scene.fills) {
            CAPTURE(f.chord_tick);
            const bool shown = meter_spawns_fill(scene, song, f.chord_tick, cfg);
            const bool is_listed = std::find(list.begin(), list.end(), f.chord_tick) != list.end();
            CHECK(is_listed == shown);
            ++(is_listed ? listed : unlisted);
        }
    }
    // Both sides of the rule ran, or the test fails.
    CHECK_MESSAGE(listed > 0, "no empty corpus path lists a fill");
    CHECK_MESSAGE(unlisted > 0, "every fill of every empty corpus path is listed");
}
