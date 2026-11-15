// Tests for app/preview_view's SP meter: the gauge curve the Preview tab
// draws, read back with sp_meter_bars_at and sp_meter_readout, plus
// has_sp_gauge and the activation's sp_window. Hand-built songs pin the bank,
// the drain and the cap exactly; the corpus case holds every stored path's
// gauge to its record. The scene has its own file (test_preview_scene.cpp),
// and so do the overlay boxes (test_preview_boxes.cpp).

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "core/backend_value.h"  // core::after_sp_end
#include "core/model.h"
#include "core/replay.h"
#include "core/squeeze_rating.h"
#include "core/timing.h"  // sp_bars_to_measures
#include "corpus_util.h"
#include "parse/song.h"
#include "preview_song_fixtures.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

using namespace hydra;
using namespace hydra::app;
using namespace hydra::preview_fixtures;

namespace {

// The curve must tile the timeline with no gap and no overlap, and never leave
// the 0..cap band (every fixture here spends no more than the cap).
void check_curve_well_formed(const SpMeterCurve& curve) {
    for (size_t i = 1; i < curve.segments.size(); ++i)
        CHECK(curve.segments[i].start_ms == doctest::Approx(curve.segments[i - 1].end_ms));
    for (const SpMeterSegment& s : curve.segments) {
        CHECK(s.end_ms >= s.start_ms);
        CHECK(s.start_bars >= 0.0);
        CHECK(s.end_bars >= 0.0);
        CHECK(s.start_bars <= static_cast<double>(curve.cap) + 1e-9);
        CHECK(s.end_bars <= static_cast<double>(curve.cap) + 1e-9);
    }
}

// The stored path of `rec` whose notation is `notation`, or null.
const Path* path_named(const HydraRecord& rec, const std::string& notation) {
    for (const Path* p : rec.all_paths())
        if (p->pathstring() == notation) return p;
    return nullptr;
}

// The record's early squeeze-out: the activation squeezed out a chord that
// core::after_sp_end (the search's own SqueezeChoice::late question) says
// is not after its stored end. Asked of the two stored ticks.
bool early_sqout(const Activation& a) {
    const std::optional<int64_t> d = a.deact_tick();
    return d && a.sqout_tick && !core::after_sp_end(*a.sqout_tick, *d);
}

}  // namespace

TEST_CASE("sp meter curve: each phrase's last note banks one bar") {
    // Phrases ending at ticks 960 and 2880, i.e. 1000 ms and 3000 ms.
    Song song = make_sp_song({960, 2880}, /*last_tick=*/5760);
    PreviewScene scene = build_preview_scene(song, nullptr);
    const SpMeterCurve& c = scene.sp_meter;

    REQUIRE_FALSE(c.segments.empty());
    check_curve_well_formed(c);
    CHECK(c.cap == 4);

    CHECK(sp_meter_bars_at(c, 0.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 999.0) == doctest::Approx(0.0));
    // The step lands ON the phrase's last note: the later segment owns the
    // shared boundary, so the bar is already banked at exactly 1000 ms.
    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 1000.001) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 2999.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.0));
}

TEST_CASE("sp meter curve: the bank between windows is the record's, not a phrase count") {
    // The chart has one phrase, at 960 (1000 ms). The record says bars
    // arrived at 960 and at 1920 (2000 ms), and one more after the window at
    // 12480 (13000 ms). Neither of the last two has a phrase. The gauge
    // follows the record.
    Song song = make_sp_song({960}, /*last_tick=*/15360);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 1920};            // two bars; no phrase at 1920
    path.activations = {act};
    path.trailing_bank_ticks = {12480};           // a bar after the window; no phrase there

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    // Two bars is four measures: 8000 ms at 120 BPM 4/4, ending at tick 11520.
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 1999.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    // One bar's worth of drain is two measures = 4000 ms.
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 13000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 13000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: a phrase collected mid-activation jumps the meter a bar") {
    // Phrases at 1000 ms and 6000 ms; the second lands inside the activation's
    // window. The record's second SP-end step is what says it was collected
    // during SP: at 5760 the end moves two measures later, so the window runs
    // six measures instead of four and the meter steps up a bar there.
    Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 0.0});
    // 4 measures banked, then 2 more for the collection at 5760.
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    std::optional<int64_t> deact = path.activations[0].deact_tick();
    REQUIRE(deact.has_value());
    CHECK(*deact == 15360);  // 3840 + 6 measures
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(16000.0));

    // One bar per two measures = 0.25 bars per second here, before the jump...
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.75));
    // Exactly one bar more at the phrase's last note than just before it.
    CHECK(sp_meter_bars_at(c, 6000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.5));
    // ...and the same rate after it, all the way to the extended deact node.
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 16000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 16000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a squeezed-out phrase does not bank mid-drain") {
    // The phrase at tick 9600 (10000 ms) has its last note inside the
    // activation's window, but the deact node sits at the plain act + 4
    // measures: the engine records no extension, so nothing was collected
    // during SP. That phrase is the squeezed-out one -- hit late, just after
    // SP ends, and banked for the next activation. The drain must stay on its
    // plain line across it, and the bar must arrive when the window closes.
    Song song = make_sp_song({960, 9600}, /*last_tick=*/15360);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    path.trailing_bank_ticks = {11520};  // its bar arrives at the deact node

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    // Straight through the phrase at one bar per two measures: no step.
    CHECK(sp_meter_bars_at(c, 10000.0 - 1e-6) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 10000.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(0.25));
    // Empty at the deact node, then the squeezed-out phrase's bar lands.
    CHECK(sp_meter_bars_at(c, 12000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 13000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: a full bank that collects a phrase and stores no row") {
    // Regression, from a gameplay video the user checked against the Preview.
    //
    // A four-bar activation collects one SP phrase while Star Power is
    // running, and no note lands within the engine's 500 ms squeeze window
    // after the deactivation node. So the record stores no backend row at all.
    // Two measures per banked bar cannot see the collection: that count would
    // empty the meter two measures early and then show a bar never banked.
    //
    // The search stamps the node and the Preview just reads it. Eight measures
    // for the four banked bars plus two for the collection: ten.
    //
    // The phrase's last note sits 3 and 15/16 measures into the window, off
    // the quarter-note grid, so the drain is caught mid-measure on both sides
    // of the step.
    const int64_t act_tick = 3840;                 // 4000 ms
    const int64_t phrase_tick = act_tick + 7560;   // 11400 -> 11875 ms
    const int64_t deact = act_tick + 10 * 1920;    // 23040 -> 24000 ms
    Song song = make_sp_song({phrase_tick}, /*last_tick=*/28800, /*extra_bpm=*/{},
                             /*step=*/120);
    Path path;
    Activation act = sp_act_at(song, act_tick, /*sp_meter=*/4, /*end_tick=*/19200);
    act.sp_end_steps = {{act_tick, act_tick + 8 * 1920, SpEndKind::Activation},
                        {phrase_tick, deact, SpEndKind::Collected}};
    REQUIRE(act.backends.empty());
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(c.cap == 4);

    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == deact);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(24000.0));

    // Four bars at the activation, then eight measures of drain to burn them.
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(4.0));
    // Just before the phrase's last note: 3.9375 measures gone of eight.
    CHECK(sp_meter_bars_at(c, 11875.0 - 1e-6) == doctest::Approx(2.03125));
    // The collection hands back a whole bar on the spot.
    CHECK(sp_meter_bars_at(c, 11875.0) == doctest::Approx(3.03125));
    // The remaining 6.0625 bars' worth of measures runs out exactly at D.
    CHECK(sp_meter_bars_at(c, 24000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 24000.0) == doctest::Approx(0.0));
    // Still 0 after D. The phrase was collected during SP, not squeezed out,
    // so there is no bar waiting at the window's close.
    CHECK(sp_meter_bars_at(c, 26000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: two clamped collections refill twice and empty at the deact node") {
    // Cap 2. Two phrases bank 2 bars; the activation at tick 2304 (6000 ms)
    // spends them. The phrases at 3072 (8000 ms) and 3840 (10000 ms) are both
    // collected during SP, and each one clamps at the cap. The engine puts
    // the deact node at 6912 (18000 ms). The curve draws both refills, and no
    // bar shows after SP ends.
    Song song = make_overfill_song();
    ScoreGraph graph(song, 2);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    REQUIRE(act.timecode.ticks() == 2304);
    REQUIRE(act.deact_tick() == std::optional<int64_t>(6912));
    // Extra parentheses: the braced list's comma would split the macro.
    REQUIRE((act.collected_phrase_ticks() == std::vector<int64_t>{3072, 3840}));

    PreviewScene scene = build_preview_scene(song, &paths.front(), /*sp_cap=*/2);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(c.cap == 2);

    // The window starts at the 2 bars the record's first SP end leaves.
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(2.0));
    // One measure of drain, then the first collection tops back up to the cap.
    CHECK(sp_meter_bars_at(c, 8000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    // Another measure of drain, then the second collection does the same.
    CHECK(sp_meter_bars_at(c, 10000.0 - 1e-6) == doctest::Approx(1.5));
    CHECK(sp_meter_bars_at(c, 10000.0) == doctest::Approx(2.0));
    // Four measures from 10000 ms burn the 2 bars exactly at the deact node.
    CHECK(sp_meter_bars_at(c, 14000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 18000.0) == doctest::Approx(0.0));
    // Both window phrases were collected, so nothing banks when SP ends.
    CHECK(sp_meter_bars_at(c, 19000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a bar that arrives on the activation note is spent there, not counted twice") {
    // The second phrase's last note sits exactly on the activation tick, and
    // the record says its bar arrived there. That arrival and the spend are
    // at the same instant: the window starts at the record's 2 bars, not 3,
    // and the bar is not left in the bank once the window closes.
    Song song = make_sp_song({960, 3840}, /*last_tick=*/13440);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 3840};  // the second bar arrives on the activation note
    path.activations = {act};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    // Two bars is four measures: 8000 ms at 120 BPM 4/4, ending at tick 11520.
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(12000.0));

    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));         // the first bar
    CHECK(sp_meter_bars_at(c, 4000.0 - 1e-6) == doctest::Approx(1.0));  // the second not yet in
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));         // 2, not 3
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));   // one bar's drain later
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));  // the deact node
    CHECK(sp_meter_bars_at(c, 14000.0) == doctest::Approx(0.0));  // the bar was spent, not banked
}

TEST_CASE("sp meter curve: the drain is linear in measures across a tempo change") {
    // 120 BPM until tick 5760 (6000 ms), then 60 BPM. A measure costs 2000 ms
    // before the change and 4000 ms after, so the ms slope halves there.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.tempos.size() == 2);
    CHECK(scene.tempos[1].tick == 5760);
    CHECK(scene.tempos[1].ms == doctest::Approx(6000.0));
    // Four measures of SP still, but they now stretch to tick 11520 = 18000 ms.
    CHECK(scene.activations[0].sp_end_tick == 11520);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(18000.0));

    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.75));  // 0.25 bars per second
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(1.5));   // the tempo change
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.375)); // 0.125 bars per second
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.75));
    CHECK(sp_meter_bars_at(c, 18000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: an activation the record stamped nothing on draws nothing") {
    // A hand-built activation with a timecode and nothing else: no bar
    // arrivals, no SP end steps. The gauge reads the record and counts no
    // phrase, so there is nothing to draw. A fresh record always stamps both
    // (the search tests prove it).
    Song song = make_sp_song({960, 1920, 5760}, /*last_tick=*/9600);
    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};

    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);

    REQUIRE(scene.activations.size() == 1);
    CHECK_FALSE(scene.activations[0].has_sp_end);

    CHECK(sp_meter_bars_at(c, 3999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 5999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: the bank stops at the cap") {
    // Six phrases, at 1000 ms through 6000 ms.
    Song song = make_sp_song({960, 1920, 2880, 3840, 4800, 5760}, /*last_tick=*/7680);

    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.cap == 4);
    CHECK(sp_meter_bars_at(scene.sp_meter, 3000.0) == doctest::Approx(3.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 4000.0) == doctest::Approx(4.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 5000.0) == doctest::Approx(4.0));
    CHECK(sp_meter_bars_at(scene.sp_meter, 8000.0) == doctest::Approx(4.0));

    // The same chart on a record analyzed at a cap of 2.
    PreviewScene capped = build_preview_scene(song, nullptr, /*sp_cap=*/2);
    CHECK(capped.sp_meter.cap == 2);
    check_curve_well_formed(capped.sp_meter);
    CHECK(sp_meter_bars_at(capped.sp_meter, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 6000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(capped.sp_meter, 8000.0) == doctest::Approx(2.0));
}

// The smallest cap is typed once, in the settings key table; both meters
// floor through Settings::clamp (finding 139). A cap of 0 reads as the
// owner's floor with a path and without one.
TEST_CASE("sp meter curve: the cap floor is the settings owner's") {
    const int floor_cap = Settings::clamp(&Settings::sp_cap, 0);
    CHECK(floor_cap == 1);
    Song song = make_sp_song({960, 1920, 5760}, /*last_tick=*/9600);

    const PreviewScene unanalyzed = build_preview_scene(song, nullptr, /*sp_cap=*/0);
    CHECK(unanalyzed.sp_meter.cap == floor_cap);

    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};
    const PreviewScene with_path = build_preview_scene(song, &path, /*sp_cap=*/0);
    CHECK(with_path.sp_meter.cap == floor_cap);
}

TEST_CASE("sp meter curve: without a path the meter fills and never drains") {
    Song song = make_sp_song({960, 1920, 2880, 3840, 4800}, /*last_tick=*/7680);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.activations.empty());
    check_curve_well_formed(scene.sp_meter);

    // Never decreasing anywhere: nothing spends the bank.
    for (const SpMeterSegment& s : scene.sp_meter.segments) CHECK(s.end_bars >= s.start_bars);
    double prev = 0.0;
    for (double ms = 0.0; ms <= 9000.0; ms += 250.0) {
        const double v = sp_meter_bars_at(scene.sp_meter, ms);
        CHECK(v >= prev - 1e-9);
        prev = v;
    }
    CHECK(sp_meter_bars_at(scene.sp_meter, 9000.0) == doctest::Approx(4.0));
}

TEST_CASE("sp meter curve: a chart with no SP and no path has no curve at all") {
    Song song = make_fill_song();  // fills but no SP phrases
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.segments.empty());
    CHECK(sp_meter_bars_at(scene.sp_meter, 1000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a late squeeze-in refills on the squeezed chord") {
    // Finding 5's chart. The engine's record puts the squeeze-in's step on
    // the late phrase's own chord, 13920 (7250 ms), past the end in force,
    // 13440 (7000 ms); the step's end is 17280 (9000 ms). The gauge reaches
    // empty on the old end, waits there, and gains the bar on the chord.
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Path& best = paths.front();
    REQUIRE(best.activations.size() == 1);
    REQUIRE((best.activations.front().sp_end_steps ==
             std::vector<SpEndStep>{{5760, 13440, SpEndKind::Activation},
                                    {13920, 17280, SpEndKind::SqIn}}));

    PreviewScene scene = build_preview_scene(song, &best);
    REQUIRE(scene.activations.size() == 1);
    // The change sits on the stored step's tick, the squeezed chord.
    CHECK(scene.activations.front().sp_end_changes.at(1).at_tick ==
          best.activations.front().sp_end_steps[1].tick);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(2.0));   // the activation
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.0));   // a bar per 2000 ms
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(0.0));   // empty on the old end
    CHECK(sp_meter_bars_at(c, 7125.0) == doctest::Approx(0.0));   // and waits there
    CHECK(sp_meter_readout(c, 7200.0) == "0.0/4");
    CHECK(sp_meter_bars_at(c, 7250.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7250.0) == doctest::Approx(0.875)); // the squeezed chord
    CHECK(sp_meter_readout(c, 7250.0) == "0.9/4");
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(0.0));   // the window's end
    CHECK(sp_meter_bars_at(c, 9500.0) == doctest::Approx(0.0));   // collected: nothing banks after
}

TEST_CASE("sp meter curve: a squeezed-out bar arrives on the squeezed chord") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    // The best path collects 12960 instead and keeps SP running over the
    // second fill, so search wide and pick the path that squeezed out.
    EngineOptions opts = test::wide_search();
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    std::vector<Path> paths = run_search(graph, opts);
    const Path* found = nullptr;
    for (const Path& p : paths)
        if (!p.activations.empty() && p.activations.front().sqout_tick) found = &p;
    REQUIRE(found != nullptr);
    REQUIRE(found->activations.size() == 2);
    // The engine's record: the squeezed-out chord is 12960 (6750 ms), before
    // the stored end 13440 (7000 ms), and the search stamped the bar on the
    // stored end.
    REQUIRE(found->activations[0].sqout_tick == std::optional<int64_t>(12960));
    REQUIRE(found->activations[0].deact_tick() == std::optional<int64_t>(13440));
    REQUIRE((found->activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
    PreviewScene scene = build_preview_scene(song, found);
    // Star Power is over on the chord: the window ends there.
    REQUIRE(scene.activations.size() == 2);
    REQUIRE(scene.activations[0].sp_window().has_value());
    CHECK(scene.activations[0].sp_window()->second ==
          doctest::Approx(song.timecode(*found->activations[0].sqout_tick).ms()));
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    // The activation drains toward the chord: (12960 - 5760) / 3840 bars.
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(1.875));
    CHECK(sp_meter_bars_at(c, 6750.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 6750.0) == doctest::Approx(1.0));   // its bar, on the chord
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // nothing at the stored end
    CHECK(sp_meter_bars_at(c, 7500.0) == doctest::Approx(2.0));   // the next phrase
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(2.0));   // the second activation
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: the drain follows the stored steps, not the collected list") {
    // The record moves the end at 7680 (8000 ms) from 11520 to 15360, and no
    // phrase ends there. The gauge still steps there.
    Song song = make_sp_song({960}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.bank_rise_ticks = {960, 1920};
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation},
                        {7680, 15360, SpEndKind::Collected}};  // no phrase ends at 7680
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 8000.0 - 1e-6) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 16000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp_meter_bars_at: before the curve, after it, and on a shared boundary") {
    CHECK(sp_meter_bars_at(SpMeterCurve{}, 123.0) == doctest::Approx(0.0));

    SpMeterCurve c;
    c.segments = {{100.0, 200.0, 0.0, 0.0}, {200.0, 400.0, 1.0, 0.0}};

    CHECK(sp_meter_bars_at(c, 50.0) == doctest::Approx(0.0));   // before the first
    CHECK(sp_meter_bars_at(c, 100.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 150.0) == doctest::Approx(0.0));
    // 200 ms is the boundary: the later segment's 1.0, not the earlier's 0.0.
    CHECK(sp_meter_bars_at(c, 200.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 300.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 400.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 99999.0) == doctest::Approx(0.0));  // holds the last value

    // A zero-width final segment is how a curve carries its end value when the
    // last step lands exactly at the end of the chart.
    SpMeterCurve z;
    z.segments = {{0.0, 100.0, 0.0, 1.0}, {100.0, 100.0, 3.0, 3.0}};
    CHECK(sp_meter_bars_at(z, 50.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(z, 100.0) == doctest::Approx(3.0));
    CHECK(sp_meter_bars_at(z, 500.0) == doctest::Approx(3.0));
}

TEST_CASE("has_sp_gauge: every scene built from a song, Star Power or not") {
    CHECK_FALSE(PreviewScene{}.has_sp_gauge());

    // Timing but no curve: a chart with no SP phrase still has its gauge,
    // the scene of "drain box: idle on a chart with no Star Power".
    PreviewScene no_curve = build_preview_scene(make_sp_song({}, /*last_tick=*/3840), nullptr);
    REQUIRE(no_curve.timing.has_value());
    REQUIRE(no_curve.sp_meter.segments.empty());
    CHECK(no_curve.has_sp_gauge());

    // A curve but no timing.
    PreviewScene no_timing;
    no_timing.sp_meter.segments.push_back(SpMeterSegment{});
    CHECK_FALSE(no_timing.has_sp_gauge());

    // The scene of "drain box: active inside the stored SP window, idle outside it".
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    CHECK(build_preview_scene(song, &path).has_sp_gauge());
}

TEST_CASE("sp_window: the stored window, none without an end, none with no length") {
    // The numbers of test_track_state's "active SP window ends exactly at the
    // deact node".
    PreviewActivation a;
    a.ms = 1000.0;
    a.has_sp_end = true;
    a.sp_end_ms = 3000.0;
    const std::optional<std::pair<double, double>> w = a.sp_window();
    REQUIRE(w.has_value());
    CHECK(w->first == 1000.0);
    CHECK(w->second == 3000.0);

    PreviewActivation no_end = a;
    no_end.has_sp_end = false;
    CHECK_FALSE(no_end.sp_window().has_value());

    PreviewActivation no_length = a;
    no_length.sp_end_ms = 1000.0;
    CHECK_FALSE(no_length.sp_window().has_value());
}

TEST_CASE("sp_running_at: on from the strike, off at the window's end, the next window runs at the join") {
    // Two back-to-back windows, [1000, 2000) and [2000, 3000) ms. The scene of
    // test_track_state's "the sp_active sweep agrees with sp_running_at".
    PreviewActivation a;
    a.ms = 1000.0;
    a.has_sp_end = true;
    a.sp_end_ms = 2000.0;
    PreviewActivation b = a;
    b.ms = 2000.0;
    b.sp_end_ms = 3000.0;

    CHECK_FALSE(a.sp_running_at(500.0));   // before the start
    CHECK(a.sp_running_at(1000.0));        // at the start: the chord is struck
    CHECK(a.sp_running_at(1500.0));        // inside
    CHECK_FALSE(a.sp_running_at(2000.0));  // at the end: not running
    CHECK(b.sp_running_at(2000.0));        // the join: the second runs
    CHECK_FALSE(b.sp_running_at(3000.0));  // at the second's end
    CHECK_FALSE(b.sp_running_at(4000.0));  // after

    // No window, no running: without a stored end, or with no length.
    PreviewActivation no_end = a;
    no_end.has_sp_end = false;
    CHECK_FALSE(no_end.sp_running_at(1500.0));
    PreviewActivation no_length = a;
    no_length.sp_end_ms = 1000.0;
    CHECK_FALSE(no_length.sp_running_at(1000.0));
}

TEST_CASE("sp meter curve: a meter change inside the window changes the drain rate") {
    // 4/4 to tick 7680 (8000 ms, a barline), then 3/4: 1440 ticks, 1500 ms a
    // measure. Two bars from tick 3840 (measure 2) are four measures: two of
    // 4/4 to tick 7680, then two of 3/4 to tick 10560 (11000 ms). One bar is
    // 4000 ms of 4/4 and 3000 ms of 3/4.
    Song song = make_sp_song({960, 1920}, /*last_tick=*/13440, {}, 480, {{7680, {3, 4}}});
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/10560);
    act.bank_rise_ticks = {960, 1920};
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    REQUIRE(scene.activations.size() == 1);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(11000.0));

    CHECK(sp_meter_bars_at(c, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 4000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 6000.0) == doctest::Approx(1.5));   // 2000 ms of 4/4
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.25));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(1.0));   // the meter change
    CHECK(sp_meter_bars_at(c, 8750.0) == doctest::Approx(0.75));  // 750 ms of 3/4
    CHECK(sp_meter_bars_at(c, 9500.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, std::nextafter(11000.0, -1e300)) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));
}

// D15 and D42. The engine stores each SP end as a whole tick, dropping the
// fraction of a measure (SongTiming::plusmeasure). The gauge drains at
// exactly one bar per two measures to that stored end, so inside the window
// it may sit up to one tick's worth of drain per stored SP-end step below a
// gauge that starts at exactly the banked bars. Between windows it is exact.
TEST_CASE("sp meter curve: a stored SP end that drops a fraction of a tick") {
    // 120 BPM, 480 ticks a beat; 4/4 to tick 7680 (8000 ms), then 3/4 (1440
    // ticks a measure). The activation is two ticks into measure 3: measure
    // 2 + 2/1920. Two bars later is measure 6 + 2/1920, which in 3/4 is tick
    // 7680 + (2 + 2/1920) x 1440 = 10561.5. The engine stores 10561.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    apply_timesig(song, 7680, 3, 4);
    song.build_timing();
    for (int64_t t : {0, 960, 1920, 3842, 4800, 7680, 9000, 12000, 14000}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(NoteColor::Red);
        if (t == 960 || t == 1920) test::mark_phrase_end(ts, t, 480);
        song.sequence.push_back(std::move(ts));
    }
    CHECK(song.timing().plusmeasure(song.timecode(3842), 4).ticks() == 10561);

    Path path;
    Activation act = sp_act_at(song, 3842, /*sp_meter=*/2, /*end_tick=*/10561);
    act.bank_rise_ticks = {960, 1920};
    path.activations = {act};
    path.trailing_bank_ticks = {12000};  // a bar after the window (12500 ms)
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    REQUIRE(scene.activations.size() == 1);
    // Tick 3842 is 4002.083 ms; tick 10561 is 8000 + 2881 x 500/480 =
    // 11001.042 ms.
    const double act_ms = scene.activations[0].ms;
    const double end_ms = scene.activations[0].sp_end_ms;
    CHECK(act_ms == doctest::Approx(4002.0833333));
    CHECK(end_ms == doctest::Approx(11001.0416667));

    // One tick of drain at the smallest measure here (1440 ticks): a bar is
    // two measures, so 1 / 2880 bars. The window has one stored step.
    const double one_tick = 1.0 / 2880.0;
    // At the activation: the stored end is measure 4 + 2881/1440 = 6.0006944
    // and the activation 2 + 2/1920 = 2.0010417, so 3.9996528 measures are
    // left, 1.9998264 bars. Exactly 2 bars would start at 2.
    const double at_act = sp_meter_bars_at(c, act_ms);
    CHECK(at_act == doctest::Approx(1.99982638889).epsilon(1e-10));
    CHECK(std::fabs(at_act - 2.0) <= one_tick);
    // At the meter change (tick 7680, 8000 ms): 2881/1440 measures left =
    // 1.0003472 bars. Exactly 2 bars less the 1.9989583 measures gone would
    // be 1.0005208.
    const double at_meter = sp_meter_bars_at(c, 8000.0);
    CHECK(at_meter == doctest::Approx(1.00034722222).epsilon(1e-10));
    CHECK(std::fabs(at_meter - 1.0005208) <= one_tick);
    // Empty at the stored end, as both would be.
    CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) == doctest::Approx(0.0));
    // Between windows, exact whole bars: two before, none after, then the
    // trailing bar.
    CHECK(sp_meter_bars_at(c, 3000.0) == 2.0);
    CHECK(sp_meter_bars_at(c, end_ms) == 0.0);
    CHECK(sp_meter_bars_at(c, 12000.0) == 0.0);
    CHECK(sp_meter_bars_at(c, 12500.0) == 1.0);
}

// The gauge on every stored path of every corpus chart, held to what the
// record says, through the record's own facts. Each window ends on
// Activation::window_end_tick, and the gauge is empty there. At each
// activation it is never above the bars the activation spent
// (Activation::sp_meter); when the window ends on the stored end it is at
// most one tick's worth of drain lower (D15: the stored end drops a fraction
// of a tick), and an early squeezed-out chord ends it sooner. Between windows
// it holds whole bars, never over the cap. Without a path it holds whole bars
// that never fall.
TEST_CASE("sp meter curve: every corpus gauge starts at its spent bars and empties at its window's end") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int charts = 0, windows = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);

        const PreviewScene bare = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules);
        check_curve_well_formed(bare.sp_meter);
        double prev = 0.0;
        for (const SpMeterSegment& s : bare.sp_meter.segments) {
            CHECK(s.start_bars == s.end_bars);
            CHECK(s.start_bars == std::floor(s.start_bars));
            CHECK(s.start_bars >= prev);
            prev = s.start_bars;
        }

        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            const SpMeterCurve& c = scene.sp_meter;
            check_curve_well_formed(c);
            int64_t min_tpm = std::numeric_limits<int64_t>::max();
            for (const PreviewMeasureSpan& m : scene.measure_spans)
                min_tpm = std::min(min_tpm, m.tpm);
            REQUIRE(min_tpm > 0);
            const double one_tick = 1.0 / (static_cast<double>(kMeasuresPerSpBar) *
                                           static_cast<double>(min_tpm));
            size_t i = 0;
            for (const Activation& a : p->walk_activations()) {
                const PreviewActivation& pa = scene.activations.at(i++);
                REQUIRE(pa.has_sp_end);
                ++windows;
                const std::optional<int64_t> end = a.window_end_tick();
                REQUIRE(end.has_value());
                CHECK(pa.sp_end_tick == *end);
                const double end_ms = song.timing().ms_index().at(*end);
                CHECK(pa.sp_end_ms == doctest::Approx(end_ms));
                const double at_act = sp_meter_bars_at(c, pa.ms);
                CHECK(at_act <= a.sp_meter() + 1e-9);
                if (end == a.deact_tick()) CHECK(at_act >= a.sp_meter() - one_tick - 1e-9);
                CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) < 1e-6);
            }
            for (const SpMeterSegment& s : c.segments) {
                bool in_window = false;
                for (const PreviewActivation& pa : scene.activations)
                    if (s.end_ms > pa.ms && s.start_ms < pa.sp_end_ms) in_window = true;
                if (in_window) continue;
                CHECK(s.start_bars == s.end_bars);
                CHECK(s.start_bars == std::floor(s.start_bars));
            }
        }
    }
    CHECK(charts > 0);
    CHECK(windows > charts);
}

TEST_CASE("sp meter readout: bars banked over the cap") {
    TwoActs t;
    // The phrase at 1000 ms banks a bar; the activation at 2000 ms drains it
    // to empty at its deact node, two measures later (6000 ms).
    CHECK(sp_meter_readout(t.scene.sp_meter, 1500.0) == "1.0/4");
    CHECK(sp_meter_readout(t.scene.sp_meter, 4000.0) == "0.5/4");
    // No curve at all (a chart with no Star Power): an empty gauge.
    CHECK(sp_meter_readout(SpMeterCurve{}, 0.0) == "0.0/4");
}

// The Preview against the engine's record, squeeze by squeeze, on every
// stored path of every corpus chart. The expected side is always a stored
// tick: sqout_tick, the stored end (deact_tick) or a stored step's tick.
// Which side of the stored end a squeezed chord sits on is asked of those
// stored ticks with core::after_sp_end, the function the search asks.
//   An early squeeze-out (the chord is not after the stored end): the
//   window, drawn and ghost, ends on the chord, and the squeezed-out bar
//   arrives there, one whole bar on the gauge.
//   A late squeeze-out: the window ends on the stored end, and the bar
//   arrives on the chord.
//   A squeeze-in step: the gauge's change sits on the step's stored tick.
//   For a late one (the chord is after the end in force before it) the
//   gauge is empty just before the chord and gains the bar on it.
// Each of the four kinds must occur at least once, so a corpus that loses
// one fails here instead of passing with nothing checked.
TEST_CASE("Preview windows end on the squeezed chord on every corpus path") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int early_out = 0, late_out = 0, early_in = 0, late_in = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        auto ms_at = [&](int64_t tick) { return song.timecode(tick).ms(); };

        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            // The same path as the ghost too: its ghost windows are checked
            // against the same record.
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules,
                                                           std::nullopt, /*ghost_path=*/p);
            const SpMeterCurve& c = scene.sp_meter;
            const ActivationWalk acts = p->walk_activations();
            REQUIRE(scene.activations.size() == acts.size());
            REQUIRE(scene.ghost_windows.size() == acts.size());
            for (size_t i = 0; i < acts.size(); ++i) {
                CAPTURE(i);
                const Activation& a = acts[i];
                const PreviewActivation& pa = scene.activations[i];
                const std::optional<int64_t> deact = a.deact_tick();
                REQUIRE(deact.has_value());

                // Where the record says this window's Star Power is over.
                int64_t over_tick = *deact;
                if (a.sqout_tick) {
                    const int64_t chord = *a.sqout_tick;
                    // The squeezed-out bar is the first the next window (or
                    // the trailing bank) holds, and it arrives on the chord.
                    const std::vector<int64_t>& next = i + 1 < acts.size()
                                                           ? scene.activations[i + 1].bank_rise_ticks
                                                           : scene.trailing_bank_ticks;
                    REQUIRE_FALSE(next.empty());
                    CHECK(next.front() == chord);
                    if (early_sqout(a)) {
                        ++early_out;
                        over_tick = chord;
                        // One whole bar lands on the chord, unless the next
                        // activation spends it on that same note.
                        const bool spent_there =
                            i + 1 < acts.size() && acts[i + 1].timecode.ticks() == chord;
                        if (!spent_there) {
                            const double m = ms_at(chord);
                            CHECK(sp_meter_bars_at(c, m) ==
                                  doctest::Approx(
                                      sp_meter_bars_at(c, std::nextafter(m, -1e300)) + 1.0));
                        }
                    } else {
                        ++late_out;
                    }
                }
                const std::optional<std::pair<double, double>> w = pa.sp_window();
                REQUIRE(w.has_value());
                CHECK(pa.sp_end_tick == over_tick);
                // Both sides come from the song's one tick-to-ms map; the
                // tight epsilon keeps a squeezed chord a few ticks before its
                // stored end from passing as the end.
                CHECK(w->second == doctest::Approx(ms_at(over_tick)).epsilon(1e-12));
                CHECK(scene.ghost_windows[i].second ==
                      doctest::Approx(ms_at(over_tick)).epsilon(1e-12));

                // Each squeeze-in step, against the gauge's change.
                REQUIRE(pa.sp_end_changes.size() == a.sp_end_steps.size());
                for (size_t k = 1; k < a.sp_end_steps.size(); ++k) {
                    const SpEndStep& step = a.sp_end_steps[k];
                    if (!is_sqin_kind(step.kind)) continue;
                    CAPTURE(step.tick);
                    CHECK(pa.sp_end_changes[k].at_tick == step.tick);
                    if (core::after_sp_end(step.tick, a.sp_end_steps[k - 1].end_tick)) {
                        ++late_in;
                        const double m = ms_at(step.tick);
                        CHECK(sp_meter_bars_at(c, std::nextafter(m, -1e300)) < 1e-6);
                        CHECK(sp_meter_bars_at(c, m) > 1e-6);
                    } else {
                        ++early_in;
                    }
                }
            }
        }
    }
    CHECK(early_out > 0);
    CHECK(late_out > 0);
    CHECK(late_in > 0);
    CHECK(early_in > 0);
}

// Chord by chord, on every stored path of every corpus chart: for each
// activation that squeezed out early (early_sqout, asked of the stored
// ticks), the Preview has Star Power running on every chord from the
// activation up to the squeezed-out chord, and not running on that chord or
// any chord after it before the next activation. The record says Star Power
// is over there: Activation::set_sqout drops every row past the chord as hit
// after SP ended, and the replay pays them nothing from this window, which
// the case checks too. The squeezed chord itself is the window's end, and
// sp_running_at reads every window's end as not running.
TEST_CASE("Preview lights each chord the record says is before the end") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    ReplayOptions opts;
    opts.scores_only = true;
    int lit = 0, unlit = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);

        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            const ActivationWalk acts = p->walk_activations();
            bool any = false;
            for (size_t i = 0; i < acts.size(); ++i) any = any || early_sqout(acts[i]);
            if (!any) continue;
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            REQUIRE(scene.activations.size() == acts.size());
            const PathReplay replay = replay_stored_path(song, *p, cfg.rules, opts);
            for (size_t i = 0; i < acts.size(); ++i) {
                const Activation& a = acts[i];
                if (!early_sqout(a)) continue;
                CAPTURE(i);
                const PreviewActivation& pa = scene.activations[i];
                const int64_t act_tick = a.timecode.ticks();
                const int64_t chord_out = *a.sqout_tick;
                const int64_t next_act = i + 1 < acts.size()
                                             ? acts[i + 1].timecode.ticks()
                                             : std::numeric_limits<int64_t>::max();
                for (const ReplayChord& rc : replay.result.chords) {
                    if (rc.tick < act_tick || rc.tick >= next_act) continue;
                    CAPTURE(rc.tick);
                    const double ms = song.timecode(rc.tick).ms();
                    if (rc.tick < chord_out) {
                        CHECK(pa.sp_running_at(ms));
                        ++lit;
                        continue;
                    }
                    CHECK_FALSE(pa.sp_running_at(ms));
                    ++unlit;
                    if (rc.tick > chord_out)
                        for (const ReplayPayment& pay : rc.paid_by)
                            CHECK(pay.act_tick != act_tick);
                }
            }
        }
    }
    CHECK(lit > 0);
    CHECK(unlit > 0);
}

// Numbers from the engine's record on two corpus charts, saved from the main
// session's hydra_replay run on 2026-11-14 (PS0b). The user's words this
// pins: "blight - starpower ends when the green cymbal + kick is squeezed
// out, not when the measure ends" and "pitch dark - the red snare is where
// the squeeze in happens, yet the bar refills one note before". Blight is
// not in the corpus; its moments are read on screen instead (PS4).
TEST_CASE("Blight, Pitch Dark and Burnout: literals") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();

    SUBCASE("Burnout: the window ends on the squeezed-out chord, tick 70800") {
        const std::string chart = corpus::chart_named("Green Day - Burnout");
        REQUIRE_FALSE(chart.empty());
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        const Path* p = path_named(corpus::analyzed(chart, cfg), "3- 1 2");
        REQUIRE(p != nullptr);
        const Activation& a = p->walk_activations()[0];
        // The record, as PS0b saved it: activation 59520, stored end 71040
        // (48,783.075 ms), squeezed-out chord 70800 (48,620.0315 ms).
        REQUIRE(a.timecode.ticks() == 59520);
        REQUIRE(a.deact_tick() == std::optional<int64_t>(71040));
        REQUIRE(a.sqout_tick == std::optional<int64_t>(70800));
        REQUIRE(song.timecode(71040).ms() == doctest::Approx(48783.075).epsilon(1e-7));

        const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
        REQUIRE_FALSE(scene.activations.empty());
        const PreviewActivation& pa = scene.activations[0];
        CHECK(pa.sp_end_tick == 70800);
        REQUIRE(pa.sp_window().has_value());
        CHECK(pa.sp_window()->second == doctest::Approx(48620.0315).epsilon(1e-7));
    }

    SUBCASE("Pitch Dark: the bar refills on the red snare, tick 128520") {
        const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
        REQUIRE_FALSE(chart.empty());
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        const Path* p = path_named(corpus::analyzed(chart, cfg), "3+ 2+ 1");
        REQUIRE(p != nullptr);
        const Activation& a = p->walk_activations()[0];
        // The record, as PS0b saved it: the activation, a clamp, and the
        // squeeze-in on chord 243, the red snare, at 100,975.37 ms.
        REQUIRE((a.sp_end_steps ==
                 std::vector<SpEndStep>{{109440, 124800, SpEndKind::Activation},
                                        {113040, 128400, SpEndKind::Clamped},
                                        {128520, 132240, SpEndKind::SqIn}}));
        REQUIRE(song.timecode(128520).ms() == doctest::Approx(100975.37).epsilon(1e-7));

        const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
        REQUIRE_FALSE(scene.activations.empty());
        REQUIRE(scene.activations[0].sp_end_changes.size() == 3);
        CHECK(scene.activations[0].sp_end_changes[2].at_tick == 128520);
        // 45 ms before the snare the gauge is empty, waiting for it; 25 ms
        // after it the bar is back.
        CHECK(sp_meter_readout(scene.sp_meter, 100930.0) == "0.0/4");
        CHECK(sp_meter_readout(scene.sp_meter, 101000.0) == "1.0/4");
    }
}
