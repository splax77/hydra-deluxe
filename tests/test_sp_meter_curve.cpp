// Tests for app/preview_view's SP meter: the gauge curve the Preview tab
// draws, read back with sp_meter_bars_at and sp_meter_readout, plus
// has_sp_gauge and the activation's sp_window. Hand-built songs pin a few
// cases exactly; inside a window every check reads the record's level
// (Activation::meter_bars_at). The corpus cases hold every stored path's
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

// The record's early squeeze-out: the activation squeezed out a chord that
// core::after_sp_end (the search's own SqueezeChoice::late question) says
// is not after its stored end. Asked of the two stored ticks.
bool early_sqout(const Activation& a) {
    const std::optional<int64_t> d = a.deact_tick();
    return d && a.sqout_tick && !core::after_sp_end(*a.sqout_tick, *d);
}

// The settings the corpus cases analyze and draw with: the app's defaults.
AnalysisSettings corpus_settings() { return Settings().to_analysis_settings(); }

// Every chart the corpus scan lists has a drum chart, so one that loads with
// no notes is a broken input: the case fails on it instead of passing over
// it. True when the song has notes to check.
bool loaded(const Song& song) {
    CHECK_FALSE(song.is_empty());
    return !song.is_empty();
}

// Every stored path of every corpus chart, each with the Preview scene the
// app builds for it under corpus_settings(). `visit(song, path, scene)`.
template <typename Visit>
void for_each_corpus_scene(Visit&& visit) {
    const AnalysisSettings cfg = corpus_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (!loaded(song)) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            const PreviewScene scene = build_preview_scene(song, p, cfg.sp_cap, cfg.rules);
            REQUIRE(scene.activations.size() == p->walk_activations().size());
            visit(song, *p, scene);
        }
    }
}

// The gauge's drain over one tick at the scene's shortest measure: the most
// a stored SP end can lose when the engine drops a fraction of a tick (D15).
double one_tick_of_drain(const PreviewScene& scene) {
    int64_t min_tpm = std::numeric_limits<int64_t>::max();
    for (const PreviewMeasureSpan& m : scene.measure_spans) min_tpm = std::min(min_tpm, m.tpm);
    REQUIRE(min_tpm > 0);
    return 1.0 / (static_cast<double>(kMeasuresPerSpBar) * static_cast<double>(min_tpm));
}

// How far the gauge steps at `ms`: its reading there less its reading just
// before.
double rise_at(const SpMeterCurve& c, double ms) {
    return sp_meter_bars_at(c, ms) - sp_meter_bars_at(c, std::nextafter(ms, -1e300));
}

// The record's level at `tick` (Activation::meter_bars_at): the expected side
// of every check of the gauge inside a window.
double record_level(const Activation& a, const Song& song, int64_t tick,
                    bool just_before = false) {
    const std::optional<double> v = a.meter_bars_at(tick, song.timing(), just_before);
    REQUIRE(v.has_value());
    return *v;
}

// The gauge on `tick` and just before it, against the record's level on each
// side. For a tick strictly inside a window: on the activation note the side
// before is the bank, and on the window's end the side after is.
void check_gauge_reads_record(const SpMeterCurve& c, const Song& song, const Activation& a,
                              int64_t tick) {
    CAPTURE(tick);
    const double ms = song.timecode(tick).ms();
    CHECK(sp_meter_bars_at(c, ms) == doctest::Approx(record_level(a, song, tick)));
    CHECK(sp_meter_bars_at(c, std::nextafter(ms, -1e300)) ==
          doctest::Approx(record_level(a, song, tick, /*just_before=*/true)));
}

// The gauge on the activation note: the window's first level, the record's.
void check_gauge_at_activation(const SpMeterCurve& c, const Song& song, const Activation& a) {
    const int64_t tick = a.timecode.ticks();
    CHECK(sp_meter_bars_at(c, song.timecode(tick).ms()) ==
          doctest::Approx(record_level(a, song, tick)));
}

// Does the next activation start by `tick`? It then draws its own window
// there, so window `i`'s own drain stops before it.
bool next_starts_by(const ActivationWalk& acts, size_t i, int64_t tick) {
    return i + 1 < acts.size() && acts[i + 1].timecode.ticks() <= tick;
}

// On window `i`'s end the gauge holds only the bars the record says arrived
// for the next window (Path::bank_arrivals) by then: nothing the window left
// waits after it. Not asked when the next activation starts by that end.
void check_nothing_waits_after(const SpMeterCurve& c, const Song& song, const Path& p,
                               const ActivationWalk& acts, size_t i) {
    const std::optional<int64_t> end = acts[i].window_end_tick();
    REQUIRE(end.has_value());
    if (next_starts_by(acts, i, *end)) return;
    const std::vector<int64_t> next = p.bank_arrivals(i + 1);
    const auto by_end =
        std::count_if(next.begin(), next.end(), [&](int64_t t) { return t <= *end; });
    CHECK(sp_meter_bars_at(c, test::tick_ms(song, *end)) ==
          doctest::Approx(static_cast<double>(by_end)));
}

// On every stored corpus path, each tick `cuts_of(scene)` gives that falls
// inside a window's own drain (after its activation note, before its end and
// before a next activation): the gauge on the tick, just before it, and
// halfway to the next cut or the drain's stop, against the record's level.
// Returns how many cuts it checked.
template <typename CutsOf>
int check_drain_across_cuts(CutsOf&& cuts_of) {
    int checked = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const std::vector<int64_t> cuts = cuts_of(scene);
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            const std::optional<int64_t> end = a.window_end_tick();
            REQUIRE(end.has_value());
            const int64_t start = a.timecode.ticks();
            const int64_t stop =
                i + 1 < acts.size() ? std::min(*end, acts[i + 1].timecode.ticks()) : *end;
            CAPTURE(i);
            for (size_t k = 0; k < cuts.size(); ++k) {
                const int64_t t = cuts[k];
                if (t <= start || t >= stop) continue;
                check_gauge_reads_record(c, song, a, t);
                const int64_t after = k + 1 < cuts.size() ? std::min(cuts[k + 1], stop) : stop;
                if (after - t > 1) check_gauge_reads_record(c, song, a, t + (after - t) / 2);
                ++checked;
            }
        }
    });
    return checked;
}

}  // namespace

TEST_CASE("sp meter curve: each phrase's last note banks one bar") {
    // Phrases ending at ticks 960 and 2880, i.e. 1000 ms and 3000 ms.
    Song song = make_sp_song({960, 2880}, /*last_tick=*/5760);
    PreviewScene scene = build_preview_scene(song, nullptr);
    const SpMeterCurve& c = scene.sp_meter;

    REQUIRE_FALSE(c.segments.empty());
    check_curve_well_formed(c);
    // No record: the gauge's ceiling is Clone Hero's (the default cap).
    CHECK(c.cap == kCloneHeroSpCap);

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

// Where the record says each bar a window spends arrived (Path::bank_arrivals,
// and after the last window the trailing bank), the gauge steps up by the bars
// that arrived on that tick, there and nowhere else. A bar that arrives on the
// activation note is spent on the spot (the case "a bar that arrives on the
// activation note" pins that), so it is not asked here, and nor is a bar saved
// on an early squeezed-out chord, where the window's leftover is dropped too
// (the case "a squeezed-out bar arrives on the squeezed chord" pins that).
// Inside a window the gauge reads the record's level
// (Activation::meter_bars_at), and just before the window's end the level
// the record gives there.
TEST_CASE("sp meter curve: the bank between windows is the record's, not a phrase count") {
    SUBCASE("every stored corpus path") {
        int arrivals = 0, drains = 0;
        for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
            const SpMeterCurve& c = scene.sp_meter;
            const ActivationWalk acts = p.walk_activations();
            for (size_t i = 0; i <= acts.size(); ++i) {
                const std::vector<int64_t> rises = p.bank_arrivals(i);
                // The window just before these arrivals, if it squeezed out early:
                // its end is the chord its saved bar arrives on.
                const std::optional<int64_t> early_end =
                    i > 0 && early_sqout(acts[i - 1]) ? acts[i - 1].window_end_tick()
                                                      : std::nullopt;
                for (size_t r = 0; r < rises.size(); ++r) {
                    const int64_t t = rises[r];
                    // A tick that holds several arrivals is asked once, at its first.
                    const auto here = rises.begin() + static_cast<std::ptrdiff_t>(r);
                    if (std::find(rises.begin(), here, t) != here) continue;
                    if (i < acts.size() && acts[i].timecode.ticks() == t) continue;
                    if (early_end && *early_end == t) continue;
                    CAPTURE(t);
                    const double bars = static_cast<double>(std::count(rises.begin(), rises.end(), t));
                    CHECK(rise_at(c, test::tick_ms(song, t)) == doctest::Approx(bars).epsilon(1e-6));
                    ++arrivals;
                }
            }
            for (size_t i = 0; i < acts.size(); ++i) {
                const Activation& a = acts[i];
                const std::optional<int64_t> end = a.window_end_tick();
                REQUIRE(end.has_value());
                CHECK(sp_meter_bars_at(c, std::nextafter(test::tick_ms(song, *end), -1e300)) ==
                      doctest::Approx(record_level(a, song, *end, /*just_before=*/true)));
                const int64_t mid = (a.timecode.ticks() + *end) / 2;
                // A next activation inside this window would draw its own drain.
                if (i + 1 < acts.size() && acts[i + 1].timecode.ticks() <= mid) continue;
                if (mid <= a.timecode.ticks()) continue;
                check_gauge_reads_record(c, song, a, mid);
                ++drains;
            }
        });
        CHECK(arrivals > 0);
        CHECK(drains > 0);
    }

    SUBCASE("a hand-built record") {
        // The chart has one phrase, at 960 (1000 ms). The record says bars
        // arrived at 960 and at 1920 (2000 ms), and one more after the window
        // at 12480 (13000 ms). Neither of the last two has a phrase. The gauge
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
        const double end_ms = test::tick_ms(song, 11520);
        CHECK(scene.activations[0].sp_end_ms == doctest::Approx(end_ms));

        // Each arrival's tick, as the song's own ms.
        const double first_ms = test::tick_ms(song, 960);
        const double second_ms = test::tick_ms(song, 1920);
        const double trailing_ms = test::tick_ms(song, 12480);
        CHECK(sp_meter_bars_at(c, std::nextafter(first_ms, -1e300)) == doctest::Approx(0.0));
        CHECK(sp_meter_bars_at(c, first_ms) == doctest::Approx(1.0));
        CHECK(sp_meter_bars_at(c, std::nextafter(second_ms, -1e300)) == doctest::Approx(1.0));
        CHECK(sp_meter_bars_at(c, second_ms) == doctest::Approx(2.0));
        // Inside the window the gauge reads the record's level.
        const Activation& a = path.activations[0];
        check_gauge_at_activation(c, song, a);
        for (int64_t t : {7680, 11520 - 1}) check_gauge_reads_record(c, song, a, t);
        // Empty at the window's end, and nothing banked until the next arrival.
        CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) == doctest::Approx(0.0));
        CHECK(sp_meter_bars_at(c, end_ms) == doctest::Approx(0.0));
        CHECK(sp_meter_bars_at(c, std::nextafter(trailing_ms, -1e300)) == doctest::Approx(0.0));
        CHECK(sp_meter_bars_at(c, trailing_ms) == doctest::Approx(1.0));
    }
}

// Each phrase the record says was collected while Star Power ran (an
// SpEndKind::Collected step, which moves the SP end one bar later) steps the
// gauge up on the step's tick by what the record's level gains there
// (Activation::meter_bars_at), and that gain is one bar. The stored end drops
// a fraction of a tick (D15), so each of the two ends either side of the step
// may sit up to one tick's drain off. Believer and Beg in the corpus hold
// Collected steps.
TEST_CASE("sp meter curve: a phrase collected mid-activation jumps the meter a bar") {
    SUBCASE("every stored corpus path") {
        int collected = 0;
        for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
            const SpMeterCurve& c = scene.sp_meter;
            const double one_tick = one_tick_of_drain(scene);
            const ActivationWalk acts = p.walk_activations();
            for (size_t i = 0; i < acts.size(); ++i) {
                const Activation& a = acts[i];
                const std::optional<int64_t> end = a.window_end_tick();
                REQUIRE(end.has_value());
                CAPTURE(i);
                for (size_t k = 1; k < a.sp_end_steps.size(); ++k) {
                    const SpEndStep& step = a.sp_end_steps[k];
                    if (step.kind != SpEndKind::Collected) continue;
                    // Nothing past the window is drawn.
                    if (step.tick >= *end) continue;
                    // A next activation that starts by the step draws its own window.
                    if (i + 1 < acts.size() && acts[i + 1].timecode.ticks() <= step.tick) continue;
                    // Two steps on one chord step the gauge once for both.
                    if (a.sp_end_steps[k - 1].tick == step.tick) continue;
                    if (k + 1 < a.sp_end_steps.size() && a.sp_end_steps[k + 1].tick == step.tick)
                        continue;
                    CAPTURE(step.tick);
                    const double gain = record_level(a, song, step.tick) -
                                        record_level(a, song, step.tick, /*just_before=*/true);
                    CHECK(rise_at(c, test::tick_ms(song, step.tick)) == doctest::Approx(gain));
                    CHECK(std::fabs(gain - 1.0) <= 2.0 * one_tick + 1e-9);
                    ++collected;
                }
            }
        });
        CHECK(collected > 0);
    }

    SUBCASE("a hand-built record") {
        // Phrases at 1000 ms and 6000 ms; the second lands inside the
        // activation's window. The record's second SP-end step is what says it
        // was collected during SP: at 5760 the end moves two measures later,
        // so the window runs six measures instead of four and the meter steps
        // up a bar there.
        Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
        Path path;
        Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
        // 4 measures banked, then 2 more for the collection at 5760.
        act.sp_end_steps = {{3840, 11520, SpEndKind::Activation},
                            {5760, 15360, SpEndKind::Collected}};
        path.activations = {act};

        PreviewScene scene = build_preview_scene(song, &path);
        const SpMeterCurve& c = scene.sp_meter;
        check_curve_well_formed(c);

        REQUIRE(scene.activations.size() == 1);
        CHECK(scene.activations[0].sp_end_ms == doctest::Approx(test::tick_ms(song, 15360)));

        // Before the jump, on both sides of the phrase's last note, and after
        // it all the way to the extended deact node, the gauge reads the record.
        const Activation& a = path.activations[0];
        check_gauge_at_activation(c, song, a);
        for (int64_t t : {4800, 5760, 7680, 11520, 15360 - 1}) check_gauge_reads_record(c, song, a, t);
        // The record's step hands back one bar on the phrase's last note.
        CHECK(record_level(a, song, 5760) - record_level(a, song, 5760, true) ==
              doctest::Approx(1.0));
        const double end_ms = test::tick_ms(song, 15360);
        CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) == doctest::Approx(0.0));
        CHECK(sp_meter_bars_at(c, end_ms) == doctest::Approx(0.0));
    }
}

// On every stored corpus path, the gauge inside a window steps up only on a
// tick where the record stored an SP-end step. A phrase the record holds no
// step for, such as a squeezed-out one, never banks mid-drain: an early
// squeezed-out chord ends the window instead, and its bar arrives there (the
// case "a squeezed-out bar arrives on the squeezed chord" pins that). Burnout
// in the corpus has an early one.
TEST_CASE("sp meter curve: a squeezed-out phrase does not bank mid-drain") {
    SUBCASE("every stored corpus path") {
        int windows = 0, early = 0;
        for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
            const SpMeterCurve& c = scene.sp_meter;
            // A rise no bigger than the drain a stored end may drop is no step.
            const double one_tick = one_tick_of_drain(scene);
            const ActivationWalk acts = p.walk_activations();
            for (size_t i = 0; i < acts.size(); ++i) {
                const Activation& a = acts[i];
                const PreviewActivation& pa = scene.activations[i];
                CAPTURE(i);
                ++windows;
                if (early_sqout(a)) ++early;
                // The window's own drain stops at its end, or at a next
                // activation that starts before it.
                const double stop = i + 1 < acts.size()
                                        ? std::min(pa.sp_end_ms, scene.activations[i + 1].ms)
                                        : pa.sp_end_ms;
                for (const SpMeterSegment& s : c.segments) {
                    const double m = s.start_ms;
                    if (!(m > pa.ms && m < stop)) continue;
                    if (rise_at(c, m) <= one_tick) continue;
                    CAPTURE(m);
                    bool on_step = false;
                    for (const SpEndStep& step : a.sp_end_steps)
                        if (test::tick_ms(song, step.tick) == m) on_step = true;
                    CHECK(on_step);
                }
            }
        });
        CHECK(windows > 0);
        CHECK(early > 0);
    }

    SUBCASE("a hand-built record") {
        // The phrase at tick 9600 (10000 ms) has its last note inside the
        // activation's window, and the record holds no SP-end step for it.
        // The drain must stay on its plain line across it.
        Song song = make_sp_song({960, 9600}, /*last_tick=*/15360);
        Path path;
        path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};

        PreviewScene scene = build_preview_scene(song, &path);
        const SpMeterCurve& c = scene.sp_meter;
        check_curve_well_formed(c);

        REQUIRE(scene.activations.size() == 1);
        CHECK(scene.activations[0].sp_end_tick == 11520);
        const double end_ms = test::tick_ms(song, 11520);
        CHECK(scene.activations[0].sp_end_ms == doctest::Approx(end_ms));

        // Straight through the phrase at 9600: the record has no step there,
        // so the gauge reads the same level on both sides of it.
        const Activation& a = path.activations[0];
        check_gauge_at_activation(c, song, a);
        for (int64_t t : {4800, 9600, 10560}) check_gauge_reads_record(c, song, a, t);
        CHECK(record_level(a, song, 9600) == record_level(a, song, 9600, true));
        // Empty at the deact node.
        CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) == doctest::Approx(0.0));
    }
}

// Regression, from a gameplay video the user checked against the Preview: a
// window that collects a phrase while Star Power runs keeps SP going past
// where its banked bars alone would run out (Activation::nominal_end, the
// first step's end). A gauge that counted measures per banked bar emptied
// there, two measures early. On every stored corpus path, for each window
// with a Collected step before that point, the drawn window ends on the
// record's stored end, later than the bars alone; at the bars-alone end the
// gauge still reads the record's level, above empty; it empties just before
// the stored end; and nothing waits after it. Believer and Beg in the corpus
// hold Collected steps (the case "a phrase collected mid-activation jumps the
// meter a bar" needs them too).
TEST_CASE("sp meter curve: a full bank that collects a phrase and stores no row") {
    int windows = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            const std::optional<int64_t> bars_end = a.nominal_end();
            REQUIRE(bars_end.has_value());
            bool collected = false;
            for (size_t k = 1; k < a.sp_end_steps.size(); ++k)
                if (a.sp_end_steps[k].kind == SpEndKind::Collected &&
                    a.sp_end_steps[k].tick < *bars_end)
                    collected = true;
            if (!collected || early_sqout(a) || next_starts_by(acts, i, *bars_end)) continue;
            CAPTURE(i);
            ++windows;
            const int64_t stored_end = *a.deact_tick();
            CHECK(scene.activations[i].sp_end_tick == stored_end);
            CHECK(stored_end > *bars_end);
            const double at_bars_end = sp_meter_bars_at(c, test::tick_ms(song, *bars_end));
            CHECK(at_bars_end == doctest::Approx(record_level(a, song, *bars_end)));
            CHECK(at_bars_end > 0.0);
            CHECK(sp_meter_bars_at(c, std::nextafter(test::tick_ms(song, stored_end), -1e300)) ==
                  doctest::Approx(record_level(a, song, stored_end, /*just_before=*/true)));
            CHECK(record_level(a, song, stored_end, /*just_before=*/true) == 0.0);
            check_nothing_waits_after(c, song, p, acts, i);
        }
    });
    CHECK(windows > 0);
}

// On every stored corpus path, each phrase the record says was collected at
// the cap (an SpEndKind::Clamped step, ADR 0013) tops the gauge back up on
// the step's tick: the gauge reads the record's level on both sides of it,
// and that level is the cap, less at most the tick the stored end drops
// (D15). Nothing waits after a window that clamped. Pitch Dark in the corpus
// holds Clamped steps.
TEST_CASE("sp meter curve: two clamped collections refill twice and empty at the deact node") {
    int clamps = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const double one_tick = one_tick_of_drain(scene);
        const double cap = static_cast<double>(c.cap);
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            const std::optional<int64_t> end = a.window_end_tick();
            REQUIRE(end.has_value());
            CAPTURE(i);
            bool clamped = false;
            for (size_t k = 1; k < a.sp_end_steps.size(); ++k) {
                const SpEndStep& step = a.sp_end_steps[k];
                if (step.kind != SpEndKind::Clamped) continue;
                if (step.tick >= *end || next_starts_by(acts, i, step.tick)) continue;
                // Two steps on one chord step the gauge once for both.
                if (a.sp_end_steps[k - 1].tick == step.tick) continue;
                if (k + 1 < a.sp_end_steps.size() && a.sp_end_steps[k + 1].tick == step.tick)
                    continue;
                CAPTURE(step.tick);
                check_gauge_reads_record(c, song, a, step.tick);
                const double level = record_level(a, song, step.tick);
                CHECK(level <= cap + 1e-9);
                CHECK(level >= cap - one_tick - 1e-9);
                clamped = true;
                ++clamps;
            }
            if (clamped) check_nothing_waits_after(c, song, p, acts, i);
        }
    });
    CHECK(clamps > 0);
}

// On every stored corpus path, the gauge on an activation note reads the
// record's level there, never more than the bars the record says the
// activation spent (Activation::sp_meter): a bar that arrived on the note
// itself is spent there, not counted on top. Just before the note it holds
// the bars that had arrived before it (Path::bank_arrivals), and nothing
// waits after the window.
TEST_CASE("sp meter curve: a bar that arrives on the activation note is spent there, not counted twice") {
    int windows = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            const int64_t act = a.timecode.ticks();
            CAPTURE(i);
            ++windows;
            check_gauge_at_activation(c, song, a);
            // A stored step on the note itself moves the level there.
            const bool step_on_note = a.sp_end_steps.size() > 1 && a.sp_end_steps[1].tick == act;
            const double act_ms = test::tick_ms(song, act);
            if (!step_on_note)
                CHECK(sp_meter_bars_at(c, act_ms) <= static_cast<double>(a.sp_meter()) + 1e-9);
            // The previous window may still run up to the note.
            const bool prev_runs = i > 0 && *acts[i - 1].window_end_tick() >= act;
            if (!prev_runs) {
                const std::vector<int64_t> arrived = p.bank_arrivals(i);
                const auto before = std::count_if(arrived.begin(), arrived.end(),
                                                  [&](int64_t t) { return t < act; });
                CHECK(sp_meter_bars_at(c, std::nextafter(act_ms, -1e300)) ==
                      doctest::Approx(static_cast<double>(before)));
            }
            check_nothing_waits_after(c, song, p, acts, i);
        }
    });
    CHECK(windows > 0);
}

// On every stored corpus path, the gauge across each tempo change inside a
// window reads the record's level (Activation::meter_bars_at), which counts
// measures, so its ms slope changes with the tempo. Pitch Dark in the corpus
// changes tempo inside its windows.
TEST_CASE("sp meter curve: the drain is linear in measures across a tempo change") {
    const int checked = check_drain_across_cuts([](const PreviewScene& scene) {
        std::vector<int64_t> ticks;
        for (const PreviewTempo& t : scene.tempos) ticks.push_back(t.tick);
        return ticks;
    });
    CHECK(checked > 0);
}

TEST_CASE("sp meter curve: the bank stops at the cap") {
    // Six phrases, at 1000 ms through 6000 ms: more than either cap below.
    const std::vector<int64_t> phrase_ticks = {960, 1920, 2880, 3840, 4800, 5760};
    Song song = make_sp_song(phrase_ticks, /*last_tick=*/7680);
    // Without a path, one bar per phrase ended by `ms`, never over `cap`.
    auto banked = [&](double ms, int cap) {
        int ended = 0;
        for (int64_t t : phrase_ticks)
            if (test::tick_ms(song, t) <= ms) ++ended;
        return static_cast<double>(std::min(ended, cap));
    };

    // No record: Clone Hero's cap.
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.cap == kCloneHeroSpCap);
    REQUIRE(static_cast<size_t>(kCloneHeroSpCap) < phrase_ticks.size());
    for (double ms : {3000.0, 4000.0, 5000.0, 8000.0}) {
        CAPTURE(ms);
        CHECK(sp_meter_bars_at(scene.sp_meter, ms) == doctest::Approx(banked(ms, kCloneHeroSpCap)));
    }

    // The same chart on a record analyzed at a cap of 2.
    const int small_cap = 2;
    PreviewScene capped = build_preview_scene(song, nullptr, small_cap);
    CHECK(capped.sp_meter.cap == small_cap);
    check_curve_well_formed(capped.sp_meter);
    for (double ms : {1000.0, 2000.0, 6000.0, 8000.0}) {
        CAPTURE(ms);
        CHECK(sp_meter_bars_at(capped.sp_meter, ms) == doctest::Approx(banked(ms, small_cap)));
    }
}

// The smallest cap is typed once, in the settings key table; both meters
// floor through Settings::clamp (finding 139). A cap of 0 reads as the
// owner's floor without a path and with a corpus chart's stored path.
TEST_CASE("sp meter curve: the cap floor is the settings owner's") {
    const int floor_cap = Settings::clamp(&Settings::sp_cap, 0);
    const AnalysisSettings cfg = corpus_settings();
    const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
    const std::vector<const Path*> paths = corpus::analyzed(chart, cfg).all_paths();
    REQUIRE_FALSE(paths.empty());

    const PreviewScene unanalyzed = build_preview_scene(song, nullptr, /*sp_cap=*/0, cfg.rules);
    CHECK(unanalyzed.sp_meter.cap == floor_cap);

    const PreviewScene with_path = build_preview_scene(song, paths.front(), /*sp_cap=*/0, cfg.rules);
    CHECK(with_path.sp_meter.cap == floor_cap);
}

TEST_CASE("sp meter curve: without a path the meter fills and never drains") {
    const std::vector<int64_t> phrase_ticks = {960, 1920, 2880, 3840, 4800};
    Song song = make_sp_song(phrase_ticks, /*last_tick=*/7680);
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
    // After the last phrase: a bar per phrase, up to Clone Hero's cap.
    const int phrases = static_cast<int>(phrase_ticks.size());
    CHECK(sp_meter_bars_at(scene.sp_meter, 9000.0) ==
          doctest::Approx(static_cast<double>(std::min(phrases, kCloneHeroSpCap))));
}

TEST_CASE("sp meter curve: a chart with no SP and no path has no curve at all") {
    Song song = make_fill_song();  // fills but no SP phrases
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.sp_meter.segments.empty());
    CHECK(sp_meter_bars_at(scene.sp_meter, 1000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: a late squeeze-in refills on the squeezed chord") {
    // Finding 5's chart: the best path squeezes a phrase in past the SP end
    // in force. Every tick below is the record's.
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Path& best = paths.front();
    const ActivationWalk acts = best.walk_activations();
    REQUIRE(acts.size() == 1);
    const Activation& a = acts[0];
    // The late squeeze-in: a squeeze-in step whose chord is past the end in
    // force before it, asked with core::after_sp_end, the search's question.
    size_t late = 0;
    for (size_t k = 1; k < a.sp_end_steps.size(); ++k)
        if (is_sqin_kind(a.sp_end_steps[k].kind) &&
            core::after_sp_end(a.sp_end_steps[k].tick, a.sp_end_steps[k - 1].end_tick))
            late = k;
    REQUIRE(late > 0);
    const int64_t chord = a.sp_end_steps[late].tick;
    const int64_t old_end = a.sp_end_steps[late - 1].end_tick;
    const int64_t new_end = a.sp_end_steps[late].end_tick;

    PreviewScene scene = build_preview_scene(song, &best);
    REQUIRE(scene.activations.size() == 1);
    // The change sits on the stored step's tick, the squeezed chord.
    CHECK(scene.activations[0].sp_end_changes.at(late).at_tick == chord);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    // From the activation to the old end, through the wait, on the chord and
    // on to the new end, the gauge reads the record's level.
    const int64_t waiting = (old_end + chord) / 2;
    check_gauge_at_activation(c, song, a);
    for (int64_t t : {(a.timecode.ticks() + old_end) / 2, old_end, waiting, chord,
                      (chord + new_end) / 2, new_end - 1})
        check_gauge_reads_record(c, song, a, t);
    // The record's level: empty from the old end until the chord, and back
    // above empty on the chord itself.
    CHECK(record_level(a, song, old_end) == 0.0);
    CHECK(record_level(a, song, waiting) == 0.0);
    CHECK(record_level(a, song, chord, /*just_before=*/true) == 0.0);
    CHECK(record_level(a, song, chord) > 0.0);
    // While it waits, the readout shows the empty gauge over the cap.
    CHECK(sp_meter_readout(c, test::tick_ms(song, waiting)) == "0.0/" + std::to_string(c.cap));
    check_nothing_waits_after(c, song, best, acts, 0);
}

// On every stored corpus path, an activation that squeezed out a chord
// strictly before its stored end (Activation::sqout_tick before deact_tick)
// ends its window on that chord. Just before the chord the gauge reads the
// record's level there (Activation::meter_bars_at). The squeezed-out bar is
// the first the next window holds (Path::bank_arrivals), and on the chord the
// gauge holds the bars saved there and nothing the window left. Across the
// stored end the gauge holds, unless the record stamps a bar or an activation
// there. Burnout in the corpus has one.
TEST_CASE("sp meter curve: a squeezed-out bar arrives on the squeezed chord") {
    int strict = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            if (!early_sqout(a)) continue;
            const int64_t chord = *a.sqout_tick;
            const int64_t stored_end = *a.deact_tick();
            if (chord >= stored_end) continue;
            CAPTURE(i);
            ++strict;
            CHECK(scene.activations[i].sp_end_tick == chord);
            const std::vector<int64_t> next = p.bank_arrivals(i + 1);
            REQUIRE_FALSE(next.empty());
            CHECK(next.front() == chord);
            const double chord_ms = test::tick_ms(song, chord);
            CHECK(sp_meter_bars_at(c, std::nextafter(chord_ms, -1e300)) ==
                  doctest::Approx(record_level(a, song, chord, /*just_before=*/true)));
            const bool next_act_by_end =
                i + 1 < acts.size() && acts[i + 1].timecode.ticks() <= stored_end;
            if (!(i + 1 < acts.size() && acts[i + 1].timecode.ticks() == chord))
                CHECK(sp_meter_bars_at(c, chord_ms) ==
                      doctest::Approx(static_cast<double>(std::count(next.begin(), next.end(), chord))));
            if (next_act_by_end || std::count(next.begin(), next.end(), stored_end) > 0) continue;
            CHECK(rise_at(c, test::tick_ms(song, stored_end)) == 0.0);
        }
    });
    CHECK(strict > 0);
}

TEST_CASE("sp meter curve: an early squeeze-out ends above empty and its bar arrives on the chord") {
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
    // the stored end 13440 (7000 ms), so SP ends on the chord.
    const Activation& a = found->activations[0];
    const int64_t chord = 12960;
    REQUIRE(a.sqout_tick == std::optional<int64_t>(chord));
    REQUIRE(a.deact_tick() == std::optional<int64_t>(13440));
    REQUIRE(a.window_end_tick() == a.sqout_tick);
    PreviewScene scene = build_preview_scene(song, found);
    // Star Power is over on the chord: the window ends there.
    REQUIRE(scene.activations.size() == 2);
    REQUIRE(scene.activations[0].sp_window().has_value());
    const double chord_ms = song.timecode(chord).ms();
    CHECK(scene.activations[0].sp_window()->second == doctest::Approx(chord_ms));
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    // Until the chord the meter drains at its own rate toward the stored
    // end, so the gauge reads the record's level all the way there, and on
    // the chord that level is above empty.
    check_gauge_at_activation(c, song, a);
    for (int64_t t : {int64_t{9600}, chord - 1}) check_gauge_reads_record(c, song, a, t);
    const double left_on_chord = record_level(a, song, chord, /*just_before=*/true);
    CHECK(left_on_chord > 0.0);
    CHECK(sp_meter_bars_at(c, std::nextafter(chord_ms, -1e300)) == doctest::Approx(left_on_chord));
    // SP ending there drops what is left (SP-25); the gauge then holds only
    // the bar saved on the chord, the first the next window holds.
    REQUIRE_FALSE(scene.activations[1].bank_rise_ticks.empty());
    CHECK(scene.activations[1].bank_rise_ticks.front() == chord);
    CHECK(sp_meter_bars_at(c, chord_ms) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // nothing at the stored end
    CHECK(sp_meter_bars_at(c, 7500.0) == doctest::Approx(2.0));   // the next phrase
    // The second window reads its own record.
    const Activation& b = found->activations[1];
    check_gauge_at_activation(c, song, b);
    check_gauge_reads_record(c, song, b, 21120);  // 11000 ms
}

// sp_meter_bars_at reads back every corpus path's gauge as its segments hold
// it: nothing before the first, the last value after the last, a straight line
// inside each, and on a boundary two segments share, the later one, so a step
// reads as a step.
TEST_CASE("sp_meter_bars_at: before the curve, after it, and on a shared boundary") {
    CHECK(sp_meter_bars_at(SpMeterCurve{}, 123.0) == doctest::Approx(0.0));

    int steps = 0;
    for_each_corpus_scene([&](const Song&, const Path&, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const std::vector<SpMeterSegment>& segs = c.segments;
        if (segs.empty()) return;
        CHECK(sp_meter_bars_at(c, std::nextafter(segs.front().start_ms, -1e300)) == 0.0);
        CHECK(sp_meter_bars_at(c, segs.back().end_ms + 1000.0) == segs.back().end_bars);
        for (size_t i = 0; i < segs.size(); ++i) {
            const SpMeterSegment& s = segs[i];
            // The last segment to start at a moment is the one that owns it.
            if (i + 1 < segs.size() && segs[i + 1].start_ms <= s.start_ms) continue;
            CAPTURE(s.start_ms);
            const double span = s.end_ms - s.start_ms;
            CHECK(sp_meter_bars_at(c, s.start_ms) == (span > 0.0 ? s.start_bars : s.end_bars));
            if (span > 0.0)
                CHECK(sp_meter_bars_at(c, s.start_ms + span / 2.0) ==
                      doctest::Approx((s.start_bars + s.end_bars) / 2.0));
            if (i > 0 && segs[i - 1].end_ms == s.start_ms && segs[i - 1].end_bars != s.start_bars)
                ++steps;
        }
    });
    CHECK(steps > 0);
}

TEST_CASE("has_sp_gauge: every scene built from a song, Star Power or not") {
    CHECK_FALSE(PreviewScene{}.has_sp_gauge());

    // Timing but no curve: a chart with no SP phrase still has its gauge,
    // the scene of "drain box: idle on a chart with no Star Power".
    PreviewScene no_curve = build_preview_scene(make_sp_song({}, /*last_tick=*/3840), nullptr);
    REQUIRE(no_curve.timing.has_value());
    REQUIRE(no_curve.sp_meter.segments.empty());
    CHECK(no_curve.has_sp_gauge());

    // Every stored path of every corpus chart.
    int scenes = 0;
    for_each_corpus_scene([&](const Song&, const Path&, const PreviewScene& scene) {
        CHECK(scene.has_sp_gauge());
        ++scenes;
    });
    CHECK(scenes > 0);
}

// On every stored corpus path, each activation's window runs from its note to
// the end the record gives it (Activation::window_end_tick).
TEST_CASE("sp_window: the stored window, from the activation note to the record's window end") {
    int windows = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            const Activation& a = acts[i];
            const std::optional<int64_t> end = a.window_end_tick();
            REQUIRE(end.has_value());
            const std::optional<std::pair<double, double>> w = scene.activations[i].sp_window();
            REQUIRE(w.has_value());
            CHECK(w->first == doctest::Approx(song.timecode(a.timecode.ticks()).ms()));
            CHECK(w->second == doctest::Approx(test::tick_ms(song, *end)));
            ++windows;
        }
    });
    CHECK(windows > 0);
}

// On every stored corpus path, Star Power runs from the moment the activation
// note is struck up to, and not at, the end the record gives the window.
TEST_CASE("sp_running_at: on from the strike, off at the window's end") {
    int windows = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            CAPTURE(i);
            const Activation& a = acts[i];
            const PreviewActivation& pa = scene.activations[i];
            const std::optional<int64_t> end = a.window_end_tick();
            REQUIRE(end.has_value());
            const double start_ms = song.timecode(a.timecode.ticks()).ms();
            const double end_ms = test::tick_ms(song, *end);
            CHECK_FALSE(pa.sp_running_at(std::nextafter(start_ms, -1e300)));
            CHECK(pa.sp_running_at(start_ms));
            CHECK(pa.sp_running_at(std::nextafter(end_ms, -1e300)));
            CHECK_FALSE(pa.sp_running_at(end_ms));
            ++windows;
        }
    });
    CHECK(windows > 0);
}

// On every stored corpus path, the gauge across each time-signature change
// inside a window (a span start of the continuous measure count) reads the
// record's level, which counts measures, so a bar still drains over two
// measures and its ms per bar changes with the measure's length. Pitch Dark
// in the corpus changes meter inside its windows.
TEST_CASE("sp meter curve: a meter change inside the window changes the drain rate") {
    const int checked = check_drain_across_cuts([](const PreviewScene& scene) {
        std::vector<int64_t> ticks;
        for (const PreviewMeasureSpan& m : scene.measure_spans) ticks.push_back(m.tick);
        return ticks;
    });
    CHECK(checked > 0);
}

// D15 and D42. The engine stores each SP end as a whole tick, dropping the
// fraction of a measure (SongTiming::plusmeasure). On every stored corpus
// path, the gauge on each activation note reads the measures from the note
// to the first stored end, counted by the song's continuous measure count,
// over kMeasuresPerSpBar. That is never above the bars the activation spent
// and at most one tick's worth of drain below them.
TEST_CASE("sp meter curve: a stored SP end that drops a fraction of a tick") {
    int windows = 0;
    for_each_corpus_scene([&](const Song& song, const Path& p, const PreviewScene& scene) {
        const SpMeterCurve& c = scene.sp_meter;
        const double one_tick = one_tick_of_drain(scene);
        const SongTiming& timing = song.timing();
        const ActivationWalk acts = p.walk_activations();
        for (size_t i = 0; i < acts.size(); ++i) {
            const Activation& a = acts[i];
            const int64_t act = a.timecode.ticks();
            const std::optional<int64_t> first_end = a.nominal_end();
            REQUIRE(first_end.has_value());
            // A stored step on the note itself moves the level there.
            if (a.sp_end_steps.size() > 1 && a.sp_end_steps[1].tick == act) continue;
            CAPTURE(i);
            ++windows;
            const double measures = timing.measures_at_tick_f(static_cast<double>(*first_end)) -
                                    timing.measures_at_tick_f(static_cast<double>(act));
            const double exact = measures / static_cast<double>(kMeasuresPerSpBar);
            const double spent = static_cast<double>(a.sp_meter());
            const double at_act = sp_meter_bars_at(c, test::tick_ms(song, act));
            CHECK(at_act == doctest::Approx(exact));
            CHECK(at_act <= spent + 1e-9);
            CHECK(spent - at_act <= one_tick + 1e-9);
        }
    });
    CHECK(windows > 0);
}

// The gauge on every stored path of every corpus chart, held to what the
// record says, through the record's own facts. Each window ends on
// Activation::window_end_tick. Just before that end the gauge reads the
// record's level there (Activation::meter_bars_at): empty when the window
// ends on the stored end (no squeeze-out, or a late one), and above empty on
// an early squeezed-out chord before the stored end, where SP ends with bars
// still in the meter. On every stored step's tick the gauge reads the
// record's level. At each activation it is never above the bars the
// activation spent (Activation::sp_meter); when the window ends on the stored
// end it is at most one tick's worth of drain lower (D15: the stored end
// drops a fraction of a tick). Between windows it holds whole bars, never
// over the cap. Without a path it holds whole bars that never fall.
TEST_CASE("sp meter curve: every corpus gauge reads the record's level up to its window's end") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int windows = 0, ends_empty = 0, ends_above = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (!loaded(song)) continue;
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
            const double one_tick = one_tick_of_drain(scene);
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

                // Just before the window's end: the record's level there.
                const double left = record_level(a, song, *end, /*just_before=*/true);
                CHECK(sp_meter_bars_at(c, std::nextafter(end_ms, -1e300)) ==
                      doctest::Approx(left));
                if (*end == *a.deact_tick()) {
                    ++ends_empty;
                    CHECK(left == 0.0);
                } else {
                    // An early squeeze-out's chord before the stored end.
                    ++ends_above;
                    CHECK(*end < *a.deact_tick());
                    CHECK(left > 0.0);
                }

                // On each stored step's tick inside the window, the record's
                // level for that tick.
                for (const SpEndStep& s : a.sp_end_steps) {
                    if (s.tick >= *end) continue;
                    CAPTURE(s.tick);
                    CHECK(sp_meter_bars_at(c, song.timing().ms_index().at(s.tick)) ==
                          doctest::Approx(record_level(a, song, s.tick)));
                }
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
    // The loop ran, and each of the two window ends above ran at least once:
    // a corpus that loses one fails here instead of passing with it unchecked.
    CHECK(windows > 0);
    CHECK(ends_empty > 0);
    CHECK(ends_above > 0);
}

// The readout shows the gauge's bars over the cap the record was analyzed at.
// On every stored corpus path, at each activation, mid-window and at each
// window's end, the number before the slash is the gauge's reading to the
// tenth it prints, and the number after it is the analysis cap.
TEST_CASE("sp meter readout: bars banked over the cap") {
    const std::string over_cap = "/" + std::to_string(corpus_settings().sp_cap);
    // The readout prints one decimal, so it sits within half a tenth.
    const double half_tenth = 0.05;
    int readouts = 0;
    for_each_corpus_scene([&](const Song&, const Path&, const PreviewScene& scene) {
        for (const PreviewActivation& pa : scene.activations) {
            for (double ms : {pa.ms, (pa.ms + pa.sp_end_ms) / 2.0, pa.sp_end_ms}) {
                const std::string r = sp_meter_readout(scene.sp_meter, ms);
                CAPTURE(r);
                const size_t slash = r.find('/');
                REQUIRE(slash != std::string::npos);
                CHECK(r.substr(slash) == over_cap);
                CHECK(std::fabs(std::stod(r.substr(0, slash)) -
                                sp_meter_bars_at(scene.sp_meter, ms)) <= half_tenth + 1e-9);
                ++readouts;
            }
        }
    });
    CHECK(readouts > 0);

    // No curve at all (a chart with no Star Power): an empty gauge over
    // Clone Hero's cap.
    const std::string empty = sp_meter_readout(SpMeterCurve{}, 0.0);
    const size_t slash = empty.find('/');
    REQUIRE(slash != std::string::npos);
    CHECK(std::stod(empty.substr(0, slash)) == 0.0);
    CHECK(empty.substr(slash) == "/" + std::to_string(kCloneHeroSpCap));
}

// The Preview against the engine's record, squeeze by squeeze, on every
// stored path of every corpus chart. The expected side is always a stored
// tick: sqout_tick, the stored end (deact_tick) or a stored step's tick.
// Which side of the stored end a squeezed chord sits on is asked of those
// stored ticks with core::after_sp_end, the function the search asks.
//   An early squeeze-out (the chord is not after the stored end): the
//   window, drawn and ghost, ends on the chord, and the squeezed-out bar
//   arrives there; on the chord the gauge holds the bars saved there and
//   nothing the window left.
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
        if (!loaded(song)) continue;
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
                        // SP ends on the chord and drops what the meter
                        // held, so on the chord the gauge holds exactly the
                        // bars saved there, unless the next activation
                        // spends them on that same note.
                        const bool spent_there =
                            i + 1 < acts.size() && acts[i + 1].timecode.ticks() == chord;
                        if (!spent_there) {
                            const auto saved = std::count(next.begin(), next.end(), chord);
                            CHECK(sp_meter_bars_at(c, ms_at(chord)) ==
                                  doctest::Approx(static_cast<double>(saved)));
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
        if (!loaded(song)) continue;
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
