// The Preview's timeline: its pure layout and hit test (ui/preview_timeline).
// No ImGui context is needed; the on-screen click is the Path Builder's GUI
// test.
#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "analyzed_chart.h"  // corpus::analyzed_by_ten_scores, chart_paths, song, analyzed
#include "app/display_format.h"  // format_duration, the axis labels' text
#include "app/preview_view.h"
#include "app/song_length.h"  // song_length_ms, the real scenes' length
#include "core/model.h"  // sp_ready_tick, Path::trailing_sp_ready_tick
#include "core/rules.h"
#include "core/timing.h"
#include "fixed_pitch.h"  // ten_per_char
#include "preview_scene_fixtures.h"
#include "scratch_settings.h"  // the GUI tests' settings, as the Path Builder's tests use
#include "search/pather.h"  // search_target, search_graph: the Path Builder's empty path
#include "ui/preview_timeline.h"
#include "ui/timeline_view.h"

using namespace hydra;
using hydra::app::PreviewFillState;
namespace fx = hydra::test_fixtures;

namespace {

// A 10 s song 1,000 px wide, so one pixel is 10 ms over the whole song.
// Every row is 10 px tall: sizes for the test only.
constexpr double kLengthMs = 10000.0;
constexpr float kWidth = 1000.0f;

ui::TimelineMetrics metrics() {
    ui::TimelineMetrics m;
    for (float& h : m.row_h) h = 10.0f;
    return m;
}

// A window over the whole of a song `length_ms` long, which lays the song
// out as the timeline did before it had a window.
ui::TimelineView whole(double length_ms) { return ui::whole_song_view(length_ms); }

app::PreviewScene scene_with_fills() {
    app::PreviewScene s = fx::timed_scene();
    app::PreviewFill a = fx::fill(fx::span(1000.0, 2000.0), PreviewFillState::Offered);
    a.chord_tick = 2000;
    app::PreviewFill b = fx::fill(fx::span(4000.0, 5000.0), PreviewFillState::Taken);
    b.chord_tick = 5000;
    app::PreviewFill c = fx::fill(fx::span(7000.0, 8000.0), PreviewFillState::Hidden);
    c.chord_tick = 8000;
    s.fills = {a, b, c};
    app::PreviewActivation act;
    act.tick = 5000;
    act.ms = 5000.0;
    act.has_sp_end = true;
    act.sp_end_ms = 6500.0;
    s.activations = {act};
    return s;
}

// Where the layout's window puts `ms`, in its pixels: the window's share of
// the moment (ui::fraction, the owner every x is read from) times the width.
float x_at(const ui::TimelineLayout& l, double ms) {
    return static_cast<float>(ui::fraction(l.view, ms)) * l.width;
}

// The moment under `x` in the layout's window: ui::ms_at at x's share of the
// width.
double ms_under(const ui::TimelineLayout& l, float x) {
    return ui::ms_at(l.view, static_cast<double>(x) / static_cast<double>(l.width));
}

// The indexes of the scene's fills whose moment (ui::timeline_fill_ms) is
// inside `view` (ui::in_view), in the scene's order: the fills the layout
// should give an item.
std::vector<size_t> fills_in_view(const app::PreviewScene& s, ui::TimelineView view) {
    std::vector<size_t> out;
    for (size_t i = 0; i < s.fills.size(); ++i)
        if (ui::in_view(view, ui::timeline_fill_ms(s.fills[i]))) out.push_back(i);
    return out;
}

// The layout's fill items are exactly the scene's fills inside its window, in
// order, each at its moment's x.
void check_fills_in_view(const app::PreviewScene& s, const ui::TimelineLayout& l) {
    const std::vector<size_t> want = fills_in_view(s, l.view);
    REQUIRE(l.fills.size() == want.size());
    for (size_t k = 0; k < want.size(); ++k) {
        CAPTURE(k);
        CHECK(l.fills[k].fill == want[k]);
        CHECK(l.fills[k].x == doctest::Approx(x_at(l, ui::timeline_fill_ms(s.fills[want[k]]))));
    }
}

// The y the saved row gives `bars` of a meter capped at `cap`: no bars
// saved_inset up from the row's bottom, the cap saved_inset down from its top.
float saved_y(const ui::TimelineLayout& l, const ui::TimelineMetrics& m, int cap, double bars) {
    const ui::TimelineRect& row = l.row(ui::TimelineRow::Saved);
    const float zero = row.y1 - m.saved_inset;
    const float top = row.y0 + m.saved_inset;
    return zero - static_cast<float>(bars / static_cast<double>(cap)) * (zero - top);
}

// The y where row `r` starts: the heights in `m` of every row above it, the
// ghost row's only when the scene has a ghost path.
float row_top(const ui::TimelineMetrics& m, ui::TimelineRow r, bool ghost) {
    float y = 0.0f;
    for (size_t i = 0; i < static_cast<size_t>(r); ++i) {
        if (static_cast<ui::TimelineRow>(i) == ui::TimelineRow::Ghost && !ghost) continue;
        y += m.row_h[i];
    }
    return y;
}

// The axis labels: each sits on a whole kTimelineAxisStepMs after the song's
// start and before its end, inside the window, at its moment's x, reading its
// moment as format_duration spells it. The labels are the same whole number
// of steps apart, counted from the song's start, and no step at that spacing
// inside the window is left without a label.
void check_axis_steps(const ui::TimelineLayout& l) {
    const double step = ui::kTimelineAxisStepMs;
    REQUIRE(l.axis_ticks.size() >= 2);
    const double every = std::round((l.axis_ticks[1].ms - l.axis_ticks[0].ms) / step);
    REQUIRE(every >= 1.0);
    for (size_t i = 0; i < l.axis_ticks.size(); ++i) {
        const ui::TimelineTick& t = l.axis_ticks[i];
        CAPTURE(i);
        CAPTURE(t.ms);
        CHECK(t.ms > 0.0);
        CHECK(t.ms < l.length_ms);
        CHECK(ui::in_view(l.view, t.ms));
        const double steps = t.ms / step;
        CHECK(steps == doctest::Approx(std::round(steps)));
        CHECK(std::fmod(std::round(steps), every) == doctest::Approx(0.0));
        CHECK(t.x == doctest::Approx(x_at(l, t.ms)));
        CHECK(t.text == app::format_duration(t.ms / 1000.0));
        if (i > 0) CHECK(t.ms - l.axis_ticks[i - 1].ms == doctest::Approx(every * step));
    }
    // The labelled step before the first and after the last fall outside the
    // window, or at the song's start or end, which get none.
    const double before = l.axis_ticks.front().ms - every * step;
    CHECK((before <= 0.0 || !ui::in_view(l.view, before)));
    const double after = l.axis_ticks.back().ms + every * step;
    CHECK((after >= l.length_ms || !ui::in_view(l.view, after)));
}

// No two neighbouring axis labels are closer than `gap` pixels, and the
// labels take the fewest steps apart that keep that gap: one step fewer would
// put them closer than `gap`.
void check_axis_gap(const ui::TimelineLayout& l, float gap) {
    REQUIRE(l.axis_ticks.size() >= 2);
    for (size_t i = 1; i < l.axis_ticks.size(); ++i) {
        CAPTURE(i);
        const float dx = l.axis_ticks[i].x - l.axis_ticks[i - 1].x;
        CHECK((dx > gap || dx == doctest::Approx(gap)));
    }
    const double step = ui::kTimelineAxisStepMs;
    const double step_px = step / l.view.span_ms * static_cast<double>(l.width);
    const double every = (l.axis_ticks[1].ms - l.axis_ticks[0].ms) / step;
    // A step count too big for a double to tell from one less (a length a
    // corrupt song.ini states) has no smaller spacing to compare.
    const double fewer = every - 1.0;
    if (fewer >= 1.0 && fewer < every) CHECK(fewer * step_px < static_cast<double>(gap));
}

// The first corpus chart with paths (corpus::analyzed_by_ten_scores) as the
// Preview builds it, at the length app::song_length_ms gives it with no
// stated length (its last note's end, padded): with the engine's best path,
// and with no path, where the meter only fills and no activation drains it.
struct RealScenes {
    app::PreviewScene path;
    app::PreviewScene bare;
};

const RealScenes& real_scenes() {
    static const RealScenes scenes = [] {
        const app::AnalysisResult& a = corpus::analyzed_by_ten_scores();
        const std::optional<double> length = app::song_length_ms(store::ChartTimingMeta{}, a.song);
        RealScenes r;
        r.path = app::build_preview_scene(a.song, &a.record.best_path(), kCloneHeroSpCap,
                                          core::default_rules(), length);
        r.bare = app::build_preview_scene(a.song, nullptr, kCloneHeroSpCap, core::default_rules(), length);
        return r;
    }();
    return scenes;
}

// A real scene laid out over the whole song, as the timeline does before its
// window is moved.
ui::TimelineLayout whole_song_layout(const app::PreviewScene& s, const ui::TimelineMetrics& m) {
    REQUIRE_FALSE(s.sp_meter.segments.empty());
    const std::optional<ui::TimelineLayout> l =
        ui::timeline_layout(s, s.song_length_ms, whole(s.song_length_ms), kWidth, m);
    REQUIRE(l.has_value());
    return *l;
}

// The saved row's shaded stretches read against the meter itself
// (app::sp_meter_bars_at, the bars banked at a moment): each stretch spans the
// row, starts no earlier than the end of the last activation's band, and has
// kSpActivationBars or more banked at its middle; and every meter segment
// past that end with kSpActivationBars or more banked at its middle lies in a
// stretch.
void check_saved_enough(const app::PreviewScene& s, const ui::TimelineLayout& l) {
    double from_ms = 0.0;
    for (const app::PreviewActivation& a : s.activations)
        if (const std::optional<std::pair<double, double>> w = a.sp_window()) from_ms = (std::max)(from_ms, w->second);
    const float from_x = x_at(l, from_ms);
    const ui::TimelineRect& row = l.row(ui::TimelineRow::Saved);
    for (size_t i = 0; i < l.saved_enough.size(); ++i) {
        const ui::TimelineRect& r = l.saved_enough[i];
        CAPTURE(i);
        CHECK(r.x0 >= from_x);
        CHECK(r.y0 == doctest::Approx(row.y0));
        CHECK(r.y1 == doctest::Approx(row.y1));
        const double mid = ms_under(l, (r.x0 + r.x1) * 0.5f);
        CHECK(app::sp_meter_bars_at(s.sp_meter, mid) >= kSpActivationBars);
    }
    for (size_t k = 0; k < s.sp_meter.segments.size(); ++k) {
        const app::SpMeterSegment& seg = s.sp_meter.segments[k];
        const double mid = (seg.start_ms + seg.end_ms) * 0.5;
        if (!(mid > from_ms) || app::sp_meter_bars_at(s.sp_meter, mid) < kSpActivationBars) continue;
        CAPTURE(k);
        const float x = x_at(l, mid);
        const bool shaded = std::any_of(l.saved_enough.begin(), l.saved_enough.end(),
                                        [x](const ui::TimelineRect& r) { return r.x0 <= x && x <= r.x1; });
        CHECK(shaded);
    }
}

}  // namespace

TEST_CASE("preview timeline: every fill is an item named by its chord tick") {
    const app::PreviewScene s = scene_with_fills();
    const std::optional<ui::TimelineLayout> l = ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    REQUIRE(l.has_value());
    REQUIRE(l->fills.size() == 3);
    CHECK(l->fills[0].id == "##fill_2000");
    CHECK(l->fills[1].id == "##fill_5000");
    CHECK(l->fills[2].id == "##fill_8000");
    CHECK(l->fills[0].state == PreviewFillState::Offered);
    CHECK(l->fills[1].state == PreviewFillState::Taken);
    CHECK(l->fills[2].state == PreviewFillState::Hidden);
    // Each sits at its span's end: 2,000 ms of 10,000 on 1,000 px.
    CHECK(l->fills[0].x == doctest::Approx(200.0f));
    // The activation's band covers its sp_window, 5,000 to 6,500 ms.
    REQUIRE(l->bands.size() == 1);
    CHECK(l->bands[0].rect.x0 == doctest::Approx(500.0f));
    CHECK(l->bands[0].rect.x1 == doctest::Approx(650.0f));
    // No usable song length, or a window with no span: no timeline.
    CHECK_FALSE(ui::timeline_layout(s, 0.0, whole(kLengthMs), kWidth, metrics()).has_value());
    CHECK_FALSE(ui::timeline_layout(s, kLengthMs, whole(0.0), kWidth, metrics()).has_value());
}

TEST_CASE("preview timeline: a point in the Fills row names a band it is on, else the nearest fill, halfway to its neighbours") {
    const app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;
    // A 6 s window from the start: the fills at 2,000 and 5,000 ms are in
    // view, the one at 8,000 is not.
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{0.0, 6000.0}, kWidth, m);
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);
    check_fills_in_view(s, l);
    REQUIRE(l.fills.size() == 2);

    // The two boxes meet halfway between the fills' x's and span the row's
    // full height; the first starts at the left edge, the last ends at the
    // right.
    const float mid = (l.fills[0].x + l.fills[1].x) * 0.5f;
    CHECK(l.fills[0].hit.x0 == doctest::Approx(0.0f));
    CHECK(l.fills[0].hit.x1 == doctest::Approx(mid));
    CHECK(l.fills[1].hit.x0 == doctest::Approx(mid));
    CHECK(l.fills[1].hit.x1 == doctest::Approx(kWidth));
    CHECK(l.fills[0].hit.y0 == doctest::Approx(fills.y0));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(fills.y1));
    // The dots stay where they were drawn: fill_center_y down the row.
    CHECK(l.fills[0].y == doctest::Approx(fills.y0 + m.fill_center_y));

    // A pixel either side of the midpoint names the fill on that side; left
    // of the first fill, near the row's bottom, names the first; right of the
    // last, above the band there, names the last.
    const ui::TimelineHit before_mid = ui::timeline_hit(l, mid - 1.0f, fills.y0 + 1.0f);
    CHECK(before_mid.kind == ui::TimelineHit::Kind::Fill);
    CHECK(before_mid.index == 0);
    const ui::TimelineHit past_mid = ui::timeline_hit(l, mid + 1.0f, fills.y0 + 1.0f);
    CHECK(past_mid.kind == ui::TimelineHit::Kind::Fill);
    CHECK(past_mid.index == 1);
    const ui::TimelineHit left = ui::timeline_hit(l, l.fills[0].x * 0.5f, fills.y1 - 1.0f);
    CHECK(left.kind == ui::TimelineHit::Kind::Fill);
    CHECK(left.index == 0);
    REQUIRE(l.bands.size() == 1);
    const ui::TimelineRect& band = l.bands[0].rect;
    const float right_x = (l.fills[1].x + kWidth) * 0.5f;
    REQUIRE_FALSE(band.contains(right_x, fills.y0 + 1.0f));
    const ui::TimelineHit right = ui::timeline_hit(l, right_x, fills.y0 + 1.0f);
    CHECK(right.kind == ui::TimelineHit::Kind::Fill);
    CHECK(right.index == 1);

    // The band stands band_inset in from the row. A point on its visible
    // part names the band over the second fill's box; the same height off
    // the band, in the first fill's box, names that fill.
    CHECK(band.y0 == doctest::Approx(fills.y0 + m.band_inset));
    CHECK(band.y1 == doctest::Approx(fills.y1 - m.band_inset));
    const float band_y = (band.y0 + band.y1) * 0.5f;
    const float on_band_x = (band.x0 + (std::min)(band.x1, kWidth)) * 0.5f;
    REQUIRE(l.fills[1].hit.contains(on_band_x, band_y));
    const ui::TimelineHit on_band = ui::timeline_hit(l, on_band_x, band_y);
    CHECK(on_band.kind == ui::TimelineHit::Kind::Band);
    CHECK(on_band.index == 0);
    const float off_band_x = l.fills[0].x;
    REQUIRE_FALSE(band.contains(off_band_x, band_y));
    const ui::TimelineHit off_band = ui::timeline_hit(l, off_band_x, band_y);
    CHECK(off_band.kind == ui::TimelineHit::Kind::Fill);
    CHECK(off_band.index == 0);

    // Another row names the moment under the point.
    const ui::TimelineHit on_saved = ui::timeline_hit(l, 100.0f, l.row(ui::TimelineRow::Saved).y0 + 1.0f);
    CHECK(on_saved.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_saved.ms == doctest::Approx(ms_under(l, 100.0f)));

    const ui::TimelineRect& axis = l.row(ui::TimelineRow::Axis);
    const ui::TimelineHit on_axis = ui::timeline_hit(l, 350.0f, axis.y0 + 1.0f);
    CHECK(on_axis.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_axis.ms == doctest::Approx(ms_under(l, 350.0f)));

    CHECK(ui::timeline_hit(l, 350.0f, l.height() + 1.0f).kind == ui::TimelineHit::Kind::None);

    // A window with no fill in it, 8.5 to 10 s: the Fills row names nothing,
    // not even a moment.
    const ui::TimelineLayout empty =
        *ui::timeline_layout(s, kLengthMs, ui::TimelineView{8500.0, 1500.0}, kWidth, metrics());
    CHECK(empty.fills.empty());
    const ui::TimelineRect& empty_fills = empty.row(ui::TimelineRow::Fills);
    CHECK(ui::timeline_hit(empty, 500.0f, empty_fills.y0 + 5.0f).kind == ui::TimelineHit::Kind::None);
}

TEST_CASE("preview timeline: the moment under an x stays inside the window, past either edge too") {
    const app::PreviewScene s = scene_with_fills();
    // A 6 s window starting at 2 s.
    const ui::TimelineView view{2000.0, 6000.0};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, view, kWidth, metrics());
    const double end_ms = view.start_ms + view.span_ms;
    // Inside the track, the moment is the window's start plus the x's share
    // of its span.
    CHECK(ui::timeline_ms_at(l, 100.0f) == doctest::Approx(view.start_ms + 100.0 / kWidth * view.span_ms));
    CHECK(ui::timeline_ms_at(l, 0.0f) == doctest::Approx(view.start_ms));
    CHECK(ui::timeline_ms_at(l, kWidth) == doctest::Approx(end_ms));
    // Past either edge, the moment is held to that edge.
    CHECK(ui::timeline_ms_at(l, -50.0f) == doctest::Approx(view.start_ms));
    CHECK(ui::timeline_ms_at(l, kWidth + 50.0f) == doctest::Approx(end_ms));

    // A Time hit reads the same moment.
    const ui::TimelineHit hit = ui::timeline_hit(l, 100.0f, l.row(ui::TimelineRow::Saved).y0 + 1.0f);
    CHECK(hit.kind == ui::TimelineHit::Kind::Time);
    CHECK(hit.ms == doctest::Approx(ui::timeline_ms_at(l, 100.0f)));
}

TEST_CASE("preview timeline: a fill outside the window has no item and a band half outside keeps its visible half") {
    app::PreviewScene s = scene_with_fills();
    s.sections = {{0, 0.0, "Intro"}, {3000, 3000.0, "Verse"}, {7000, 7000.0, "Chorus"}};
    ui::TimelineFillMarks marks;
    marks.best_ticks = {2000, 8000};
    marks.best_label = "best";
    // A 4 s window from 6 s.
    const ui::TimelineLayout l =
        *ui::timeline_layout(s, kLengthMs, ui::TimelineView{6000.0, 4000.0}, kWidth, metrics(), marks);
    // Only the fill at 8,000 ms is inside; the word under the best fill at
    // 2,000 ms goes with its fill.
    check_fills_in_view(s, l);
    REQUIRE(l.fills.size() == 1);
    CHECK(l.fills[0].id == "##fill_8000");
    REQUIRE(l.best_labels.size() == 1);
    CHECK(l.best_labels[0].fill == l.fills[0].fill);
    // The band over the activation's sp_window starts left of the window and
    // ends inside it, each edge at its moment's x; the draw clips the rest.
    REQUIRE(l.bands.size() == 1);
    const std::optional<std::pair<double, double>> w = s.activations[0].sp_window();
    REQUIRE(w.has_value());
    CHECK(l.bands[0].rect.x0 == doctest::Approx(x_at(l, w->first)));
    CHECK(l.bands[0].rect.x1 == doctest::Approx(x_at(l, w->second)));
    CHECK(l.bands[0].rect.x0 < 0.0f);
    CHECK(l.bands[0].rect.x1 > 0.0f);
    CHECK(l.bands[0].rect.x1 < kWidth);
    // Intro ends before the window and is left out. Verse reaches in from the
    // left, runs to Chorus and keeps the shade it has from the song's start
    // (the second section shown); Chorus runs to the song's end.
    REQUIRE(l.sections.size() == 2);
    CHECK(l.sections[0].name == "Verse");
    CHECK(l.sections[0].rect.x0 == doctest::Approx(x_at(l, s.sections[1].ms)));
    CHECK(l.sections[0].rect.x0 < 0.0f);
    CHECK(l.sections[0].rect.x1 == doctest::Approx(x_at(l, s.sections[2].ms)));
    CHECK(l.sections[0].shaded);
    CHECK(l.sections[1].name == "Chorus");
    CHECK(l.sections[1].rect.x1 == doctest::Approx(x_at(l, kLengthMs)));
    CHECK_FALSE(l.sections[1].shaded);
}

TEST_CASE("preview timeline: the window moves every x by the view's start") {
    app::PreviewScene s = scene_with_fills();
    s.ghost_windows = {{1000.0, 2500.0}};
    s.sp_phrases = {fx::span(3000.0, 3500.0)};
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 0.0, 4.0}};
    // The same 5 s window, at the song's start and 2 s in. Every x in the
    // second is the same x in the first less `shift`: where the second
    // window's start falls in the first.
    const ui::TimelineLayout a = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{0.0, 5000.0}, kWidth, metrics());
    const ui::TimelineLayout b = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{2000.0, 5000.0}, kWidth, metrics());
    const float shift = x_at(a, b.view.start_ms);
    REQUIRE(shift > 0.0f);
    // Both windows hold the same fills, so the fills' x's pair up.
    check_fills_in_view(s, a);
    check_fills_in_view(s, b);
    REQUIRE_FALSE(a.fills.empty());
    REQUIRE(a.fills.size() == b.fills.size());
    for (size_t i = 0; i < a.fills.size(); ++i) {
        CAPTURE(i);
        REQUIRE(a.fills[i].fill == b.fills[i].fill);
        CHECK(b.fills[i].x == doctest::Approx(a.fills[i].x - shift));
    }
    REQUIRE_FALSE(a.bands.empty());
    REQUIRE(a.bands.size() == b.bands.size());
    for (size_t i = 0; i < a.bands.size(); ++i) {
        CAPTURE(i);
        CHECK(b.bands[i].rect.x0 == doctest::Approx(a.bands[i].rect.x0 - shift));
        CHECK(b.bands[i].rect.x1 == doctest::Approx(a.bands[i].rect.x1 - shift));
    }
    REQUIRE_FALSE(a.ghost_bands.empty());
    REQUIRE(a.ghost_bands.size() == b.ghost_bands.size());
    for (size_t i = 0; i < a.ghost_bands.size(); ++i) {
        CAPTURE(i);
        CHECK(b.ghost_bands[i].x0 == doctest::Approx(a.ghost_bands[i].x0 - shift));
        CHECK(b.ghost_bands[i].x1 == doctest::Approx(a.ghost_bands[i].x1 - shift));
    }
    REQUIRE_FALSE(a.phrase_x.empty());
    REQUIRE(a.phrase_x.size() == b.phrase_x.size());
    for (size_t i = 0; i < a.phrase_x.size(); ++i) {
        CAPTURE(i);
        CHECK(b.phrase_x[i] == doctest::Approx(a.phrase_x[i] - shift));
    }
    REQUIRE_FALSE(a.saved_line.empty());
    REQUIRE(a.saved_line.size() == b.saved_line.size());
    for (size_t i = 0; i < a.saved_line.size(); ++i) {
        CAPTURE(i);
        CHECK(b.saved_line[i].x == doctest::Approx(a.saved_line[i].x - shift));
    }
    // A point names the moment under it in the window.
    const ui::TimelineHit at_b = ui::timeline_hit(b, 100.0f, b.row(ui::TimelineRow::Axis).y0 + 1.0f);
    CHECK(at_b.kind == ui::TimelineHit::Kind::Time);
    CHECK(at_b.ms == doctest::Approx(ms_under(b, 100.0f)));
}

TEST_CASE("preview timeline: the axis labels the 30 s steps inside the window and thins them by its span") {
    const app::PreviewScene s = fx::timed_scene();
    // A 10-minute song, a 3-minute window from 0:45 on 1,000 px. With no
    // gap asked for, every step inside the window gets a label.
    const ui::TimelineView view{45000.0, 180000.0};
    const ui::TimelineLayout l = *ui::timeline_layout(s, 600000.0, view, kWidth, metrics());
    check_axis_steps(l);
    CHECK(l.axis_ticks[1].ms - l.axis_ticks[0].ms == doctest::Approx(ui::kTimelineAxisStepMs));
    // A gap wider than one step of this window takes every few steps,
    // counted from the song's start, never closer than the gap.
    ui::TimelineMetrics m = metrics();
    m.axis_label_gap = 200.0f;
    REQUIRE(ui::kTimelineAxisStepMs / view.span_ms * kWidth < m.axis_label_gap);
    const ui::TimelineLayout thin = *ui::timeline_layout(s, 600000.0, view, kWidth, m);
    check_axis_steps(thin);
    check_axis_gap(thin, m.axis_label_gap);
    CHECK(thin.axis_ticks[1].ms - thin.axis_ticks[0].ms > ui::kTimelineAxisStepMs);
}

TEST_CASE("preview timeline: the saved line has a point at each segment start and end and a mark at kSpActivationBars") {
    // The meter the engine's best path gives a real corpus chart, with the
    // saved line's ends standing 1 px in from the row.
    const app::PreviewScene& s = real_scenes().path;
    ui::TimelineMetrics m = metrics();
    m.saved_inset = 1.0f;
    const ui::TimelineLayout l = whole_song_layout(s, m);
    const app::SpMeterCurve& meter = s.sp_meter;
    // Each segment gives two points, its start and its end, at their moments'
    // x and their bars' height in the row.
    REQUIRE(l.saved_line.size() == 2 * meter.segments.size());
    for (size_t k = 0; k < meter.segments.size(); ++k) {
        const app::SpMeterSegment& seg = meter.segments[k];
        CAPTURE(k);
        const ui::TimelinePoint& a = l.saved_line[2 * k];
        const ui::TimelinePoint& b = l.saved_line[2 * k + 1];
        CHECK(a.x == doctest::Approx(x_at(l, seg.start_ms)));
        CHECK(a.y == doctest::Approx(saved_y(l, m, meter.cap, seg.start_bars)));
        CHECK(b.x == doctest::Approx(x_at(l, seg.end_ms)));
        CHECK(b.y == doctest::Approx(saved_y(l, m, meter.cap, seg.end_bars)));
        // The segments are contiguous, so where two meet the line steps
        // straight up or down.
        if (k + 1 < meter.segments.size()) CHECK(l.saved_line[2 * k + 2].x == doctest::Approx(b.x));
    }
    // The activate mark is kSpActivationBars of the meter's cap up the row.
    CHECK(l.activate_y == doctest::Approx(saved_y(l, m, meter.cap, kSpActivationBars)));
}

TEST_CASE("preview timeline: the insets move the bands, the fills and the saved line, and the axis labels every 30 s") {
    app::PreviewScene s = scene_with_fills();
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 0.0, 4.0}};
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;
    m.fill_center_y = 6.0f;
    m.saved_inset = 1.0f;
    // A 65 s song on the same 1,000 px, so the axis labels every step but
    // the start.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 65000.0, whole(65000.0), kWidth, m);
    check_axis_steps(l);
    CHECK(l.axis_ticks[1].ms - l.axis_ticks[0].ms == doctest::Approx(ui::kTimelineAxisStepMs));
    // With no ghost the ghost row is 0 tall, so the Fills row starts where
    // the saved row ends; the band sits in it and stands band_inset in.
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);
    CHECK(fills.y0 == doctest::Approx(row_top(m, ui::TimelineRow::Fills, false)));
    CHECK(fills.y0 == doctest::Approx(l.row(ui::TimelineRow::Saved).y1));
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(fills.y0 + m.band_inset));
    CHECK(l.bands[0].rect.y1 == doctest::Approx(fills.y1 - m.band_inset));
    // A fill's dot is centred fill_center_y down the same row, and its hit
    // box is the row's full height.
    REQUIRE_FALSE(l.fills.empty());
    CHECK(l.fills[0].y == doctest::Approx(fills.y0 + m.fill_center_y));
    CHECK(l.fills[0].hit.y0 == doctest::Approx(fills.y0));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(fills.y1));
    // The saved line's zero stands saved_inset up from the row's bottom, its
    // cap saved_inset down from the top, and the activate mark
    // kSpActivationBars of the cap up.
    const ui::TimelineRect& saved = l.row(ui::TimelineRow::Saved);
    REQUIRE(l.saved_line.size() == 2);
    CHECK(l.saved_line[0].y == doctest::Approx(saved.y1 - m.saved_inset));
    CHECK(l.saved_line[1].y == doctest::Approx(saved.y0 + m.saved_inset));
    CHECK(l.activate_y == doctest::Approx(saved_y(l, m, s.sp_meter.cap, kSpActivationBars)));
}

TEST_CASE("preview timeline: every corpus best path shades its saved row exactly when the engine says enough is saved") {
    // For each corpus chart's best path, under the GUI tests' settings: the
    // shaded stretch exists exactly when Path::trailing_sp_ready_tick gives a
    // tick, and starts there, held to the end of the last band. The shading
    // is also read against the drawn meter (check_saved_enough), so a best
    // path whose shaded row moved from where the line reaches the activate
    // line fails here.
    const app::AnalysisSettings cfg = scratch_settings().to_analysis_settings();
    int scenes = 0;
    int ready = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        CAPTURE(chart);
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        if (rec.paths.empty()) continue;
        const Path& best = rec.best_path();
        CAPTURE(best.pathstring());
        const app::PreviewScene s = app::build_preview_scene(
            song, &best, cfg.sp_cap, cfg.rules, app::song_length_ms(store::ChartTimingMeta{}, song));
        if (s.sp_meter.segments.empty()) continue;
        ++scenes;
        const ui::TimelineLayout l = whole_song_layout(s, metrics());
        const std::optional<int64_t> tick = best.trailing_sp_ready_tick();
        CHECK(l.saved_enough.empty() == !tick.has_value());
        check_saved_enough(s, l);
        if (!tick || l.saved_enough.empty()) continue;
        ++ready;
        REQUIRE(s.timing.has_value());
        float start = x_at(l, s.timing->ms_index().at(*tick));
        for (const ui::TimelineBand& b : l.bands) start = (std::max)(start, b.rect.x1);
        REQUIRE(l.saved_enough.size() == 1);
        CHECK(l.saved_enough[0].x0 == doctest::Approx(start));
        CHECK(l.saved_enough[0].x1 == doctest::Approx(l.saved_line.back().x));
    }
    REQUIRE(scenes > 0);
    // Premise P14 of the V2 plan: no corpus best path saves enough after its
    // last activation. A count above 0 is not a failure; it says which charts
    // the checks above covered.
    MESSAGE("corpus best paths with enough saved after their last activation: " << ready);
}

TEST_CASE("preview timeline: the saved row's shading starts at the engine's ready tick, not where the drawn line crosses") {
    // Ticks are ms on fx::timed_scene's timing. The meter drawn here sits at
    // no bars the whole song, so a layout that read the line would shade
    // nothing; the stored bars say enough is saved at the second arrival.
    app::PreviewScene s = scene_with_fills();  // its one band ends at 6,500 ms
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, kLengthMs, 0.0, 0.0}};
    const std::optional<std::pair<double, double>> band = s.activations.back().sp_window();
    REQUIRE(band.has_value());

    s.trailing_bank_ticks = {7000, 8000};
    REQUIRE(sp_ready_tick(s.trailing_bank_ticks) == int64_t{8000});
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    const ui::TimelineRect& row = l.row(ui::TimelineRow::Saved);
    REQUIRE(l.saved_enough.size() == 1);
    CHECK(l.saved_enough[0].x0 == doctest::Approx(x_at(l, s.timing->ms_index().at(8000))));
    CHECK(l.saved_enough[0].x1 == doctest::Approx(l.saved_line.back().x));
    CHECK(l.saved_enough[0].y0 == doctest::Approx(row.y0));
    CHECK(l.saved_enough[0].y1 == doctest::Approx(row.y1));

    // A ready tick inside the last band: the shading waits for the band's end.
    s.trailing_bank_ticks = {5500, 6000};
    REQUIRE(s.timing->ms_index().at(6000) < band->second);
    const ui::TimelineLayout held = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    REQUIRE(held.saved_enough.size() == 1);
    REQUIRE(held.bands.size() == 1);
    CHECK(held.saved_enough[0].x0 == doctest::Approx(held.bands[0].rect.x1));

    // One bar stored: the rule gives no tick, so nothing is shaded, even
    // where a drawn line stands well above the activate line.
    s.trailing_bank_ticks = {7000};
    s.sp_meter.segments = {{0.0, kLengthMs, 4.0, 4.0}};
    REQUIRE_FALSE(sp_ready_tick(s.trailing_bank_ticks).has_value());
    CHECK(ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics())->saved_enough.empty());
}

TEST_CASE("preview timeline: the Path Builder's path with no activation shades from the engine's ready tick") {
    // The path the Path Builder shows before the player picks a window:
    // search_target over no windows (pathbuilder/pb_state.cpp, empty_path).
    // Every bar it banks is trailing, so the engine's rule places the start.
    const app::AnalysisSettings cfg = scratch_settings().to_analysis_settings();
    bool found = false;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        if (song.is_empty()) continue;
        TargetResult none = search_target(search_graph(song, cfg), {});
        if (none.paths.empty()) continue;
        const Path& empty = none.paths.front();
        const std::optional<int64_t> tick = empty.trailing_sp_ready_tick();
        if (!tick) continue;
        CAPTURE(chart);
        found = true;
        REQUIRE(empty.walk_activations().empty());
        const app::PreviewScene s = app::build_preview_scene(
            song, &empty, cfg.sp_cap, cfg.rules, app::song_length_ms(store::ChartTimingMeta{}, song));
        REQUIRE(s.activations.empty());
        REQUIRE(s.trailing_bank_ticks == empty.trailing_bank_ticks);
        const ui::TimelineLayout l = whole_song_layout(s, metrics());
        REQUIRE(l.saved_enough.size() == 1);
        CHECK(l.saved_enough[0].x0 == doctest::Approx(x_at(l, s.timing->ms_index().at(*tick))));
        CHECK(l.saved_enough[0].x1 == doctest::Approx(l.saved_line.back().x));
        // Against the drawn meter: a failure here means the ready tick and
        // the point where the line reaches the activate line disagree, which
        // the main session shows the user (both positions) before merge.
        check_saved_enough(s, l);
        break;
    }
    REQUIRE(found);
}

TEST_CASE("preview timeline: with no path nothing on the saved row is shaded") {
    // The real chart with no path: the meter fills a bar at each phrase, but
    // there is no record, so the engine has no ready tick to shade from.
    const app::PreviewScene& s = real_scenes().bare;
    REQUIRE(s.trailing_bank_ticks.empty());
    const ui::TimelineLayout l = whole_song_layout(s, metrics());
    CHECK(l.saved_enough.empty());
}

TEST_CASE("preview timeline: the ghost path's windows are bands in their own row, which is 0 tall with no ghost") {
    app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;

    // No ghost: the row has no height and no bands, and the rows below it
    // move up.
    const ui::TimelineLayout bare = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, m);
    CHECK(bare.ghost_bands.empty());
    const ui::TimelineRect& bare_ghost = bare.row(ui::TimelineRow::Ghost);
    CHECK(bare_ghost.y0 == doctest::Approx(row_top(m, ui::TimelineRow::Ghost, false)));
    CHECK(bare_ghost.y1 == doctest::Approx(bare_ghost.y0));
    CHECK(bare.row(ui::TimelineRow::Saved).y1 == doctest::Approx(bare_ghost.y0));
    const ui::TimelineRect& bare_fills = bare.row(ui::TimelineRow::Fills);
    CHECK(bare_fills.y0 == doctest::Approx(row_top(m, ui::TimelineRow::Fills, false)));
    CHECK(bare_fills.y1 - bare_fills.y0 == doctest::Approx(m.row_h[static_cast<size_t>(ui::TimelineRow::Fills)]));
    CHECK(bare.height() == doctest::Approx(bare_fills.y1));

    // Two ghost windows: the row takes its height, and the Fills row moves
    // down by it.
    s.ghost_windows = {{1000.0, 2500.0}, {6000.0, 9000.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, m);
    const ui::TimelineRect& ghost = l.row(ui::TimelineRow::Ghost);
    const float ghost_h = m.row_h[static_cast<size_t>(ui::TimelineRow::Ghost)];
    REQUIRE(ghost_h > 0.0f);
    CHECK(ghost.y0 == doctest::Approx(row_top(m, ui::TimelineRow::Ghost, true)));
    CHECK(ghost.y1 == doctest::Approx(ghost.y0 + ghost_h));
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);
    CHECK(fills.y0 == doctest::Approx(row_top(m, ui::TimelineRow::Fills, true)));
    CHECK(fills.y0 == doctest::Approx(bare_fills.y0 + ghost_h));
    CHECK(l.height() == doctest::Approx(bare.height() + ghost_h));
    // Each ghost window is a band over its moments, standing band_inset in
    // from the ghost row.
    REQUIRE(l.ghost_bands.size() == s.ghost_windows.size());
    for (size_t i = 0; i < s.ghost_windows.size(); ++i) {
        CAPTURE(i);
        CHECK(l.ghost_bands[i].x0 == doctest::Approx(x_at(l, s.ghost_windows[i].first)));
        CHECK(l.ghost_bands[i].x1 == doctest::Approx(x_at(l, s.ghost_windows[i].second)));
        CHECK(l.ghost_bands[i].y0 == doctest::Approx(ghost.y0 + m.band_inset));
        CHECK(l.ghost_bands[i].y1 == doctest::Approx(ghost.y1 - m.band_inset));
    }
    // The drawn path's band stays in the Fills row, wherever it now is.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(fills.y0 + m.band_inset));

    // A ghost band takes no click: a point on one names the moment under it.
    const ui::TimelineRect& g = l.ghost_bands[1];
    const float gx = (g.x0 + g.x1) * 0.5f;
    const float gy = (g.y0 + g.y1) * 0.5f;
    const ui::TimelineHit on_ghost = ui::timeline_hit(l, gx, gy);
    CHECK(on_ghost.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_ghost.ms == doctest::Approx(ms_under(l, gx)));
}

TEST_CASE("preview timeline: the Section row leaves out a section named End") {
    app::PreviewScene s = fx::timed_scene();
    s.sections = {{0, 0.0, "Intro"}, {3000, 3000.0, "Verse"}, {9000, 9000.0, "End"},
                  {9500, 9500.0, "Ending"}, {9800, 9800.0, "end"}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    // The section before End still stops where End starts; a name that only
    // starts with End, or is end in lower case, stays. Pinned from one run.
    REQUIRE(l.sections.size() == 4);
    CHECK(l.sections[0].name == "Intro");
    CHECK(l.sections[1].name == "Verse");
    CHECK(l.sections[1].rect.x0 == doctest::Approx(300.0f));
    CHECK(l.sections[1].rect.x1 == doctest::Approx(900.0f));
    CHECK(l.sections[2].name == "Ending");
    CHECK(l.sections[2].rect.x0 == doctest::Approx(950.0f));
    CHECK(l.sections[3].name == "end");
    CHECK(l.sections[3].rect.x1 == doctest::Approx(1000.0f));
}

namespace {

// A fill ending at `end_ms` in `state`, its chord tick the end.
app::PreviewFill fill_at(double end_ms, PreviewFillState state) {
    app::PreviewFill f = fx::fill(fx::span(end_ms - 50.0, end_ms), state);
    f.chord_tick = static_cast<int64_t>(end_ms);
    return f;
}

ui::TimelineMetrics label_metrics() {
    ui::TimelineMetrics m = metrics();
    m.cost_top = 3.0f;
    m.cost_line_h = 5.0f;
    m.section_pad = 4.0f;
    m.cost_width = ten_per_char;
    m.section_width = ten_per_char;
    m.axis_width = ten_per_char;
    return m;
}

}  // namespace

TEST_CASE("preview timeline: a tinted fill reads its tint and an untinted offered fill reads none") {
    app::PreviewScene s = fx::timed_scene();
    s.fills = {fill_at(2000.0, PreviewFillState::Offered), fill_at(3000.0, PreviewFillState::Offered)};
    ui::TimelineFillMarks marks;
    marks.tints = {{2000, IM_COL32(1, 2, 3, 255)}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics(), marks);
    REQUIRE(l.fills.size() == 2);
    REQUIRE(l.fills[0].tint.has_value());
    CHECK(*l.fills[0].tint == IM_COL32(1, 2, 3, 255));
    CHECK_FALSE(l.fills[1].tint.has_value());
    // A tint changes the colour, not the look.
    CHECK(l.fills[0].look == ui::TimelineFillLook::Offered);
    CHECK(l.fills[1].look == ui::TimelineFillLook::Offered);
}

TEST_CASE("preview timeline: every fill in best_ticks has a word under it, kept inside the timeline") {
    // A best fill so near the start that its word centred on it would start
    // left of the timeline, a second best fill tied with it mid-song, and a
    // fill that is not best.
    app::PreviewScene s = fx::timed_scene();
    s.fills = {fill_at(100.0, PreviewFillState::Offered), fill_at(5000.0, PreviewFillState::Offered),
               fill_at(7000.0, PreviewFillState::Offered)};
    ui::TimelineFillMarks marks;
    marks.tints = {{100, IM_COL32(1, 2, 3, 255)}};
    marks.best_ticks = {100, 5000};
    marks.best_label = "best";
    const ui::TimelineMetrics m = label_metrics();
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, m, marks);
    const float w = m.cost_width(marks.best_label);
    REQUIRE(l.fills.size() == 3);
    REQUIRE(l.best_labels.size() == 2);
    // The first word, in its fill's tint, starts at the timeline's left edge.
    CHECK(l.best_labels[0].fill == 0);
    CHECK(l.best_labels[0].col == IM_COL32(1, 2, 3, 255));
    REQUIRE(l.fills[0].x - w * 0.5f < 0.0f);
    CHECK(l.best_labels[0].rect.x0 == doctest::Approx(0.0f));
    CHECK(l.best_labels[0].rect.x1 == doctest::Approx(w));
    // The second fits, so it is centred on its fill.
    CHECK(l.best_labels[1].fill == 1);
    CHECK(l.best_labels[1].rect.x0 == doctest::Approx(l.fills[1].x - w * 0.5f));
    CHECK(l.best_labels[1].rect.x1 == doctest::Approx(l.fills[1].x + w * 0.5f));
    // The word stands cost_top down the Fills row, one cost_line_h tall, and
    // the row keeps its height.
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);
    CHECK(l.best_labels[1].rect.y0 == doctest::Approx(fills.y0 + m.cost_top));
    CHECK(l.best_labels[1].rect.y1 == doctest::Approx(fills.y0 + m.cost_top + m.cost_line_h));
    CHECK(fills.y1 - fills.y0 == doctest::Approx(m.row_h[static_cast<size_t>(ui::TimelineRow::Fills)]));
}

TEST_CASE("preview timeline: looks are Taken, then Locked, then Offered, then Cant") {
    app::PreviewScene s = fx::timed_scene();
    s.fills = {
        fill_at(1000.0, PreviewFillState::Taken),    // Taken, though locked
        fill_at(2000.0, PreviewFillState::Hidden),   // Locked, though hidden
        fill_at(3000.0, PreviewFillState::Offered),  // Locked, though offered and can't
        fill_at(4000.0, PreviewFillState::Offered),  // Offered
        fill_at(5000.0, PreviewFillState::Offered),  // Cant: in cant_ticks
        fill_at(6000.0, PreviewFillState::Hidden),   // Cant: the game hides it
    };
    ui::TimelineFillMarks marks;
    marks.locked_ticks = {1000, 2000, 3000};
    marks.cant_ticks = {3000, 5000};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics(), marks);
    REQUIRE(l.fills.size() == 6);
    using Look = ui::TimelineFillLook;
    const Look want[6] = {Look::Taken, Look::Locked, Look::Locked, Look::Offered, Look::Cant, Look::Cant};
    for (size_t i = 0; i < 6; ++i) {
        CAPTURE(i);
        CHECK(l.fills[i].look == want[i]);
    }
}

TEST_CASE("preview timeline: a section name too long for its section ends in an ellipsis") {
    app::PreviewScene s = fx::timed_scene();
    s.sections ={{0, 0.0, "Intro"}, {400, 400.0, "Verse 1b"}, {1000, 1000.0, "Chorus"}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, label_metrics());
    // Intro is 40 px wide, 32 px inside its 4 px pads; Verse 1b 60 px, 52
    // inside; Chorus 900 px. Pinned from one run.
    REQUIRE(l.sections.size() == 3);
    CHECK(l.sections[0].shown == "In\xE2\x80\xA6");
    CHECK(l.sections[1].shown == "Vers\xE2\x80\xA6");
    CHECK(l.sections[1].name == "Verse 1b");
    CHECK(l.sections[2].shown == "Chorus");
}

TEST_CASE("preview timeline: the axis labels stay inside the timeline") {
    const app::PreviewScene s = fx::timed_scene();
    // A 61 s song on 1,000 px: the first label fits centred on its moment,
    // the last sits so near the right edge that centred it would end past it.
    const ui::TimelineMetrics m = label_metrics();
    const ui::TimelineLayout l = *ui::timeline_layout(s, 61000.0, whole(61000.0), kWidth, m);
    check_axis_steps(l);
    const ui::TimelineTick& first = l.axis_ticks.front();
    const float first_w = m.axis_width(first.text);
    REQUIRE(first.x - first_w * 0.5f >= 0.0f);
    REQUIRE(first.x + first_w * 0.5f <= kWidth);
    CHECK(first.text_x == doctest::Approx(first.x - first_w * 0.5f));
    // The last ends on the right edge instead.
    const ui::TimelineTick& last = l.axis_ticks.back();
    const float last_w = m.axis_width(last.text);
    REQUIRE(last.x + last_w * 0.5f > kWidth);
    CHECK(last.text_x == doctest::Approx(kWidth - last_w));
}

TEST_CASE("preview timeline: a label that would start left of the timeline starts at its left edge") {
    const app::PreviewScene s = fx::timed_scene();
    // A 50-minute song on 1,000 px: the first label sits so near the left
    // edge that centred on it, it would start left of the timeline.
    const ui::TimelineMetrics m = label_metrics();
    const ui::TimelineLayout l = *ui::timeline_layout(s, 3000000.0, whole(3000000.0), kWidth, m);
    check_axis_steps(l);
    const ui::TimelineTick& first = l.axis_ticks.front();
    REQUIRE(first.x - m.axis_width(first.text) * 0.5f < 0.0f);
    CHECK(first.text_x == doctest::Approx(0.0f));
}

TEST_CASE("preview timeline: a huge song length gets labels by the width, not by the length") {
    const app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.axis_label_gap = 50.0f;
    // A 10-minute song on 1,000 px, and twice that: the labels keep the gap,
    // a longer song taking more steps between labels, so the width alone
    // sets how many fit.
    const ui::TimelineLayout normal = *ui::timeline_layout(s, 600000.0, whole(600000.0), kWidth, m);
    check_axis_steps(normal);
    check_axis_gap(normal, m.axis_label_gap);
    const ui::TimelineLayout longer = *ui::timeline_layout(s, 1200000.0, whole(1200000.0), kWidth, m);
    check_axis_steps(longer);
    check_axis_gap(longer, m.axis_label_gap);
    CHECK(longer.axis_ticks[1].ms - longer.axis_ticks[0].ms >
          normal.axis_ticks[1].ms - normal.axis_ticks[0].ms);
    // A length a corrupt song.ini could state: the labels still keep the gap.
    const ui::TimelineLayout huge = *ui::timeline_layout(s, 1e300, whole(1e300), kWidth, m);
    check_axis_gap(huge, m.axis_label_gap);
}

TEST_CASE("preview timeline: the scrollbar's thumb and readout come from the view") {
    // The thumb is the window's shares of the song (ui::thumb) across the
    // bar, at least thumb_min_w wide and thumb_inset in from the bar's top
    // and bottom; the readout is the view's (ui::readout).
    constexpr float kBarW = 800.0f, kBarH = 16.0f, kThumbMinW = 19.0f, kThumbInset = 1.0f;
    auto check_bar = [&](ui::TimelineView view, double length_ms, float bar_w) {
        const ui::ScrollbarLayout bar = ui::scrollbar_layout(view, length_ms, bar_w, kBarH, kThumbMinW, kThumbInset);
        const ui::ThumbShares t = ui::thumb(view, length_ms);
        const float x0 = static_cast<float>(t.left) * bar_w;
        CHECK(bar.thumb.x0 == doctest::Approx(x0));
        CHECK(bar.thumb.x1 == doctest::Approx(x0 + (std::max)(static_cast<float>(t.width) * bar_w, kThumbMinW)));
        CHECK(bar.thumb.y0 == doctest::Approx(kThumbInset));
        CHECK(bar.thumb.y1 == doctest::Approx(kBarH - kThumbInset));
        CHECK(bar.readout == ui::readout(view, length_ms));
        return bar;
    };
    // A 3-minute window six minutes into a 40-minute song.
    check_bar(ui::TimelineView{360000.0, 180000.0}, 2400000.0, kBarW);

    // A song shorter than the window: the thumb fills the bar.
    const ui::ScrollbarLayout short_song = check_bar(ui::open_view(130088.0), 130088.0, kBarW);
    CHECK(short_song.thumb.x0 == doctest::Approx(0.0f));
    CHECK(short_song.thumb.x1 == doctest::Approx(kBarW));

    // A thumb narrower than the least width gets that width: 3 minutes of
    // 42:02 on a 100 px bar.
    const ui::TimelineView narrow_view{0.0, 180000.0};
    REQUIRE(ui::thumb(narrow_view, 2522500.0).width * 100.0 < kThumbMinW);
    const ui::ScrollbarLayout narrow = check_bar(narrow_view, 2522500.0, 100.0f);
    CHECK(narrow.thumb.x1 - narrow.thumb.x0 == doctest::Approx(kThumbMinW));
}
