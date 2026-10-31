// The Preview's timeline: its pure layout and hit test (ui/preview_timeline).
// No ImGui context is needed; the on-screen click is the Path Builder's GUI
// test.
#include "doctest.h"

#include "core/timing.h"
#include "fixed_pitch.h"  // ten_per_char
#include "preview_scene_fixtures.h"
#include "ui/preview_timeline.h"

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
ui::TimelineView whole(double length_ms) { return ui::TimelineView{0.0, length_ms}; }

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

TEST_CASE("preview timeline: a point anywhere in the Fills row names the nearest fill, halfway to its neighbours") {
    const app::PreviewScene s = scene_with_fills();
    // A 6 s window from the start, so 6 ms a pixel: the fills at 2,000 and
    // 5,000 ms are in view at 333.3 and 833.3 px, the one at 8,000 is not.
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{0.0, 6000.0}, kWidth, metrics());
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);
    REQUIRE(l.fills.size() == 2);

    // The two boxes meet halfway between the fills and span the row's full
    // height; the first starts at the left edge, the last ends at the right.
    // Pinned from one run.
    CHECK(l.fills[0].hit.x0 == doctest::Approx(0.0f));
    CHECK(l.fills[0].hit.x1 == doctest::Approx(583.3333f));
    CHECK(l.fills[1].hit.x0 == doctest::Approx(583.3333f));
    CHECK(l.fills[1].hit.x1 == doctest::Approx(1000.0f));
    CHECK(l.fills[0].hit.y0 == doctest::Approx(fills.y0));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(fills.y1));
    // The dots stay where they were drawn: fill_center_y down the row.
    CHECK(l.fills[0].y == doctest::Approx(fills.y0));

    // Just past the midpoint names the second fill; left of the first fill,
    // near the row's bottom, names the first; right of the last names the last.
    const ui::TimelineHit past_mid = ui::timeline_hit(l, 590.0f, fills.y0 + 1.0f);
    CHECK(past_mid.kind == ui::TimelineHit::Kind::Fill);
    CHECK(past_mid.index == 1);
    const ui::TimelineHit left = ui::timeline_hit(l, 10.0f, fills.y1 - 1.0f);
    CHECK(left.kind == ui::TimelineHit::Kind::Fill);
    CHECK(left.index == 0);
    const ui::TimelineHit right = ui::timeline_hit(l, 990.0f, fills.y0 + 5.0f);
    CHECK(right.kind == ui::TimelineHit::Kind::Fill);
    CHECK(right.index == 1);

    // The Activations row still names a band or a time. The band, 5,000 to
    // 6,500 ms, starts at 833.3 px.
    const ui::TimelineRect& acts = l.row(ui::TimelineRow::Activations);
    const ui::TimelineHit on_band = ui::timeline_hit(l, 900.0f, acts.y0 + 1.0f);
    CHECK(on_band.kind == ui::TimelineHit::Kind::Band);
    CHECK(on_band.index == 0);
    const ui::TimelineHit off_band = ui::timeline_hit(l, 100.0f, acts.y0 + 1.0f);
    CHECK(off_band.kind == ui::TimelineHit::Kind::Time);
    CHECK(off_band.ms == doctest::Approx(600.0));

    const ui::TimelineRect& axis = l.row(ui::TimelineRow::Axis);
    const ui::TimelineHit on_axis = ui::timeline_hit(l, 350.0f, axis.y0 + 1.0f);
    CHECK(on_axis.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_axis.ms == doctest::Approx(2100.0));

    CHECK(ui::timeline_hit(l, 350.0f, l.height() + 1.0f).kind == ui::TimelineHit::Kind::None);

    // A window with no fill in it, 8.5 to 10 s: the Fills row names nothing,
    // not even a moment.
    const ui::TimelineLayout empty =
        *ui::timeline_layout(s, kLengthMs, ui::TimelineView{8500.0, 1500.0}, kWidth, metrics());
    CHECK(empty.fills.empty());
    const ui::TimelineRect& empty_fills = empty.row(ui::TimelineRow::Fills);
    CHECK(ui::timeline_hit(empty, 500.0f, empty_fills.y0 + 5.0f).kind == ui::TimelineHit::Kind::None);
}

TEST_CASE("preview timeline: a fill outside the window has no item and a band half outside keeps its visible half") {
    app::PreviewScene s = scene_with_fills();
    s.sections = {{0, 0.0, "Intro"}, {3000, 3000.0, "Verse"}, {7000, 7000.0, "Chorus"}};
    ui::TimelineFillMarks marks;
    marks.best_ticks = {2000, 8000};
    marks.best_label = "best";
    // A 4 s window from 6 s, so 4 ms a pixel. Pinned from one run.
    const ui::TimelineLayout l =
        *ui::timeline_layout(s, kLengthMs, ui::TimelineView{6000.0, 4000.0}, kWidth, metrics(), marks);
    // Only the fill at 8,000 ms is inside; the word under the best fill at
    // 2,000 ms goes with its fill.
    REQUIRE(l.fills.size() == 1);
    CHECK(l.fills[0].id == "##fill_8000");
    CHECK(l.fills[0].fill == 2);
    CHECK(l.fills[0].x == doctest::Approx(500.0f));
    REQUIRE(l.best_labels.size() == 1);
    CHECK(l.best_labels[0].fill == 2);
    // The band, 5,000 to 6,500 ms, starts left of the window and ends 125 px
    // into it; the draw clips the rest.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.x0 == doctest::Approx(-250.0f));
    CHECK(l.bands[0].rect.x1 == doctest::Approx(125.0f));
    // Intro ends before the window and is left out. Verse reaches in from the
    // left and keeps the shade it has from the song's start (the second
    // section shown); Chorus runs to the song's end.
    REQUIRE(l.sections.size() == 2);
    CHECK(l.sections[0].name == "Verse");
    CHECK(l.sections[0].rect.x0 == doctest::Approx(-750.0f));
    CHECK(l.sections[0].rect.x1 == doctest::Approx(250.0f));
    CHECK(l.sections[0].shaded);
    CHECK(l.sections[1].name == "Chorus");
    CHECK(l.sections[1].rect.x1 == doctest::Approx(1000.0f));
    CHECK_FALSE(l.sections[1].shaded);
}

TEST_CASE("preview timeline: the window moves every x by the view's start") {
    app::PreviewScene s = scene_with_fills();
    s.ghost_windows = {{1000.0, 2500.0}};
    s.sp_phrases = {fx::span(3000.0, 3500.0)};
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 0.0, 4.0}};
    // The same 5 s window, at the song's start and 2 s in: 5 ms a pixel, so
    // every x moves 400 px left. Pinned from one run.
    const ui::TimelineLayout a = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{0.0, 5000.0}, kWidth, metrics());
    const ui::TimelineLayout b = *ui::timeline_layout(s, kLengthMs, ui::TimelineView{2000.0, 5000.0}, kWidth, metrics());
    REQUIRE(a.fills.size() == 2);
    REQUIRE(b.fills.size() == 2);
    CHECK(a.fills[0].x == doctest::Approx(400.0f));
    CHECK(b.fills[0].x == doctest::Approx(0.0f));
    CHECK(a.fills[1].x == doctest::Approx(1000.0f));
    CHECK(b.fills[1].x == doctest::Approx(600.0f));
    REQUIRE(a.bands.size() == 1);
    CHECK(a.bands[0].rect.x0 == doctest::Approx(1000.0f));
    CHECK(b.bands[0].rect.x0 == doctest::Approx(600.0f));
    CHECK(b.bands[0].rect.x1 == doctest::Approx(900.0f));
    REQUIRE(b.ghost_bands.size() == 1);
    CHECK(a.ghost_bands[0].x0 == doctest::Approx(200.0f));
    CHECK(b.ghost_bands[0].x0 == doctest::Approx(-200.0f));
    REQUIRE(b.phrase_x.size() == 1);
    CHECK(a.phrase_x[0] == doctest::Approx(600.0f));
    CHECK(b.phrase_x[0] == doctest::Approx(200.0f));
    REQUIRE(b.saved_line.size() == 2);
    CHECK(b.saved_line[0].x == doctest::Approx(-400.0f));
    CHECK(b.saved_line[1].x == doctest::Approx(200.0f));
    // A point names the moment under it in the window.
    const ui::TimelineHit at_b = ui::timeline_hit(b, 100.0f, b.row(ui::TimelineRow::Axis).y0 + 1.0f);
    CHECK(at_b.kind == ui::TimelineHit::Kind::Time);
    CHECK(at_b.ms == doctest::Approx(2500.0));
}

TEST_CASE("preview timeline: the axis labels the 30 s steps inside the window and thins them by its span") {
    const app::PreviewScene s = fx::timed_scene();
    // A 10-minute song, a 3-minute window from 0:45 on 1,000 px: the steps
    // from 1:00 to 3:30 are inside. Pinned from one run.
    const ui::TimelineView view{45000.0, 180000.0};
    const ui::TimelineLayout l = *ui::timeline_layout(s, 600000.0, view, kWidth, metrics());
    REQUIRE(l.axis_ticks.size() == 6);
    CHECK(l.axis_ticks[0].ms == doctest::Approx(60000.0));
    CHECK(l.axis_ticks[0].x == doctest::Approx(83.3333f));
    CHECK(l.axis_ticks[5].ms == doctest::Approx(210000.0));
    // 30 s is 166.7 px of this window; a gap of 200 px takes every second
    // step, counted from the song's start: 1:00, 2:00 and 3:00.
    ui::TimelineMetrics m = metrics();
    m.axis_label_gap = 200.0f;
    const ui::TimelineLayout thin = *ui::timeline_layout(s, 600000.0, view, kWidth, m);
    REQUIRE(thin.axis_ticks.size() == 3);
    CHECK(thin.axis_ticks[0].ms == doctest::Approx(60000.0));
    CHECK(thin.axis_ticks[1].ms == doctest::Approx(120000.0));
    CHECK(thin.axis_ticks[2].ms == doctest::Approx(180000.0));
}

TEST_CASE("preview timeline: the saved line has a point at each segment start and end and a mark at kSpActivationBars") {
    app::PreviewScene s = fx::timed_scene();
    // Two bars by 3 s, then a phrase jumps it to three, then an activation
    // drains it to none by 8 s.
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 2.0, 2.0}, {3000.0, 5000.0, 3.0, 3.0},
                           {5000.0, 8000.0, 3.0, 0.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    // The saved row is the fourth 10 px row, 30 to 40 px down: none at its
    // bottom, the cap of 4 at its top. Pinned from one run.
    REQUIRE(l.saved_line.size() == 6);
    const float xs[6] = {0.0f, 300.0f, 300.0f, 500.0f, 500.0f, 800.0f};
    const float ys[6] = {35.0f, 35.0f, 32.5f, 32.5f, 32.5f, 40.0f};
    for (size_t i = 0; i < 6; ++i) {
        CAPTURE(i);
        CHECK(l.saved_line[i].x == doctest::Approx(xs[i]));
        CHECK(l.saved_line[i].y == doctest::Approx(ys[i]));
    }
    // kSpActivationBars (2) of 4 is halfway up the row.
    CHECK(l.activate_y == doctest::Approx(35.0f));
}

TEST_CASE("preview timeline: the insets move the bands, the fills and the saved line, and the axis labels every 30 s") {
    app::PreviewScene s = scene_with_fills();
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 0.0, 4.0}};
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;
    m.fill_center_y = 6.0f;
    m.saved_inset = 1.0f;
    // A 65 s song on the same 1,000 px, so the axis has two labels: 0:30 and
    // 1:00, the start getting none.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 65000.0, whole(65000.0), kWidth, m);
    // Pinned from one run.
    REQUIRE(l.axis_ticks.size() == 2);
    CHECK(l.axis_ticks[0].ms == doctest::Approx(30000.0));
    CHECK(l.axis_ticks[1].ms == doctest::Approx(60000.0));
    CHECK(l.axis_ticks[1].x == doctest::Approx(923.0769f));
    // With no ghost the ghost row is 0 tall, so the Activations row is 40 to
    // 50 px down; its band stands 2 px in.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(42.0f));
    CHECK(l.bands[0].rect.y1 == doctest::Approx(48.0f));
    // The Fills row is 50 to 60 px down; a fill's dot is centred 6 px down
    // it, and its hit box is the row's full height.
    REQUIRE(l.fills.size() == 3);
    CHECK(l.fills[0].y == doctest::Approx(56.0f));
    CHECK(l.fills[0].hit.y0 == doctest::Approx(50.0f));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(60.0f));
    // The saved row is 30 to 40 px down: none at 39, the cap at 31.
    REQUIRE(l.saved_line.size() == 2);
    CHECK(l.saved_line[0].y == doctest::Approx(39.0f));
    CHECK(l.saved_line[1].y == doctest::Approx(31.0f));
    CHECK(l.activate_y == doctest::Approx(35.0f));
}

TEST_CASE("preview timeline: the saved row's enough stretches start after the last activation and split a segment at the line") {
    app::PreviewScene s = fx::timed_scene();
    app::PreviewActivation act;
    act.ms = 2000.0;
    act.has_sp_end = true;
    act.sp_end_ms = 3000.0;
    s.activations = {act};
    // Three bars before the activation, drained to none by its end, then a
    // climb from one bar to the cap that crosses the line at two.
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 2000.0, 3.0, 3.0},
                           {2000.0, 3000.0, 3.0, 0.0},
                           {3000.0, 5000.0, 0.0, 0.0},
                           {5000.0, 9000.0, 1.0, 4.0},
                           {9000.0, 10000.0, 4.0, 4.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    // The three bars before the window are not shaded; the climb is from its
    // crossing on, merged with the stretch at the cap. Pinned from one run.
    REQUIRE(l.saved_enough.size() == 1);
    CHECK(l.saved_enough[0].x0 == doctest::Approx(633.3333f));
    CHECK(l.saved_enough[0].x1 == doctest::Approx(1000.0f));
    // The saved row's full height, 30 to 40 px down.
    CHECK(l.saved_enough[0].y0 == doctest::Approx(30.0f));
    CHECK(l.saved_enough[0].y1 == doctest::Approx(40.0f));
}

TEST_CASE("preview timeline: a segment ending exactly on the line is enough") {
    app::PreviewScene s = fx::timed_scene();
    // A climb that ends on two bars, a stretch on the line, then a drain
    // that starts on it.
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 0.0, 2.0},
                           {3000.0, 6000.0, 2.0, 2.0},
                           {6000.0, 8000.0, 2.0, 0.0},
                           {8000.0, 10000.0, 0.0, 0.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    // Pinned from one run.
    REQUIRE(l.saved_enough.size() == 1);
    CHECK(l.saved_enough[0].x0 == doctest::Approx(300.0f));
    CHECK(l.saved_enough[0].x1 == doctest::Approx(600.0f));
}

TEST_CASE("preview timeline: with no activation every stretch at or above the line is shaded") {
    app::PreviewScene s = fx::timed_scene();
    // Above the line, down across it, below, up across it, above.
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 2000.0, 3.0, 3.0},
                           {2000.0, 4000.0, 3.0, 1.0},
                           {4000.0, 6000.0, 1.0, 1.0},
                           {6000.0, 8000.0, 1.0, 3.0},
                           {8000.0, 10000.0, 3.0, 3.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, metrics());
    // Pinned from one run.
    REQUIRE(l.saved_enough.size() == 2);
    CHECK(l.saved_enough[0].x0 == doctest::Approx(0.0f));
    CHECK(l.saved_enough[0].x1 == doctest::Approx(300.0f));
    CHECK(l.saved_enough[1].x0 == doctest::Approx(700.0f));
    CHECK(l.saved_enough[1].x1 == doctest::Approx(1000.0f));
}

TEST_CASE("preview timeline: the ghost path's windows are bands in their own row, which is 0 tall with no ghost") {
    app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;

    // No ghost: the row has no height and no bands, and the rows below it
    // move up.
    const ui::TimelineLayout bare = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, m);
    CHECK(bare.ghost_bands.empty());
    CHECK(bare.row(ui::TimelineRow::Ghost).y0 == doctest::Approx(40.0f));
    CHECK(bare.row(ui::TimelineRow::Ghost).y1 == doctest::Approx(40.0f));
    CHECK(bare.row(ui::TimelineRow::Activations).y0 == doctest::Approx(40.0f));
    CHECK(bare.height() == doctest::Approx(60.0f));

    // Two ghost windows. Pinned from one run.
    s.ghost_windows = {{1000.0, 2500.0}, {6000.0, 9000.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, m);
    const ui::TimelineRect& ghost = l.row(ui::TimelineRow::Ghost);
    CHECK(ghost.y0 == doctest::Approx(40.0f));
    CHECK(ghost.y1 == doctest::Approx(50.0f));
    CHECK(l.row(ui::TimelineRow::Activations).y0 == doctest::Approx(50.0f));
    CHECK(l.height() == doctest::Approx(70.0f));
    REQUIRE(l.ghost_bands.size() == 2);
    CHECK(l.ghost_bands[0].x0 == doctest::Approx(100.0f));
    CHECK(l.ghost_bands[0].x1 == doctest::Approx(250.0f));
    CHECK(l.ghost_bands[1].x0 == doctest::Approx(600.0f));
    CHECK(l.ghost_bands[1].x1 == doctest::Approx(900.0f));
    CHECK(l.ghost_bands[0].y0 == doctest::Approx(42.0f));
    CHECK(l.ghost_bands[0].y1 == doctest::Approx(48.0f));
    // The drawn path's band keeps its own row, now 50 to 60 px down.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(52.0f));

    // A ghost band takes no click: a point on one names the moment under it.
    const ui::TimelineHit on_ghost = ui::timeline_hit(l, 700.0f, 45.0f);
    CHECK(on_ghost.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_ghost.ms == doctest::Approx(7000.0));
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
    // A best fill at 10 px, where a centred 40 px word would start left of
    // the timeline, a second best fill tied with it at 500 px, and a fill
    // that is not best. Pinned from one run.
    app::PreviewScene s = fx::timed_scene();
    s.fills = {fill_at(100.0, PreviewFillState::Offered), fill_at(5000.0, PreviewFillState::Offered),
               fill_at(7000.0, PreviewFillState::Offered)};
    ui::TimelineFillMarks marks;
    marks.tints = {{100, IM_COL32(1, 2, 3, 255)}};
    marks.best_ticks = {100, 5000};
    marks.best_label = "best";
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, whole(kLengthMs), kWidth, label_metrics(), marks);
    REQUIRE(l.best_labels.size() == 2);
    CHECK(l.best_labels[0].fill == 0);
    CHECK(l.best_labels[0].col == IM_COL32(1, 2, 3, 255));
    CHECK(l.best_labels[0].rect.x0 == doctest::Approx(0.0f));
    CHECK(l.best_labels[0].rect.x1 == doctest::Approx(40.0f));
    CHECK(l.best_labels[1].fill == 1);
    CHECK(l.best_labels[1].rect.x0 == doctest::Approx(480.0f));
    CHECK(l.best_labels[1].rect.x1 == doctest::Approx(520.0f));
    // The Fills row starts at 50 px (no ghost row); the word is 3 px down,
    // one 5 px line tall, and the row keeps its height.
    CHECK(l.best_labels[1].rect.y0 == doctest::Approx(53.0f));
    CHECK(l.best_labels[1].rect.y1 == doctest::Approx(58.0f));
    CHECK(l.height() == doctest::Approx(60.0f));
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
    // A 61 s song on 1,000 px: labels at 30 and 60 s, each 40 px; the start
    // gets none. Pinned from one run.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 61000.0, whole(61000.0), kWidth, label_metrics());
    REQUIRE(l.axis_ticks.size() == 2);
    // 0:30 is centred on its moment.
    CHECK(l.axis_ticks[0].text == "0:30");
    CHECK(l.axis_ticks[0].text_x == doctest::Approx(471.8033f));
    // 1:00 at 983.6 px would end past the right edge; it ends on it.
    CHECK(l.axis_ticks[1].text == "1:00");
    CHECK(l.axis_ticks[1].text_x == doctest::Approx(960.0f));
}

TEST_CASE("preview timeline: a label that would start left of the timeline starts at its left edge") {
    const app::PreviewScene s = fx::timed_scene();
    // A 50-minute song on 1,000 px: 0:30 sits at 10 px, and its 40 px label
    // centred there would start 10 px left of the timeline. Pinned from one
    // run.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 3000000.0, whole(3000000.0), kWidth, label_metrics());
    REQUIRE_FALSE(l.axis_ticks.empty());
    CHECK(l.axis_ticks[0].text == "0:30");
    CHECK(l.axis_ticks[0].x == doctest::Approx(10.0f));
    CHECK(l.axis_ticks[0].text_x == doctest::Approx(0.0f));
}

TEST_CASE("preview timeline: a huge song length gets labels by the width, not by the length") {
    const app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.axis_label_gap = 50.0f;
    // A 10-minute song on 1,000 px: 30 s is 50 px, exactly the gap, so every
    // step but the start is labelled: 19 labels. Pinned from one run.
    const ui::TimelineLayout normal = *ui::timeline_layout(s, 600000.0, whole(600000.0), kWidth, m);
    CHECK(normal.axis_ticks.size() == 19);
    // Twice the length: 30 s is 25 px, so every second step: 19 labels, 60 s apart.
    const ui::TimelineLayout longer = *ui::timeline_layout(s, 1200000.0, whole(1200000.0), kWidth, m);
    REQUIRE(longer.axis_ticks.size() == 19);
    CHECK(longer.axis_ticks[0].ms == doctest::Approx(60000.0));
    CHECK(longer.axis_ticks[1].ms == doctest::Approx(120000.0));
    // A length a corrupt song.ini could state: still a handful of labels.
    const ui::TimelineLayout huge = *ui::timeline_layout(s, 1e300, whole(1e300), kWidth, m);
    CHECK(huge.axis_ticks.size() <= 20);
}
