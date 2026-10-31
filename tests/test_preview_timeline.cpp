// The Preview's whole-song timeline: its pure layout and hit test
// (ui/preview_timeline). No ImGui context is needed; the on-screen click is
// the Path Builder's GUI test.
#include "doctest.h"

#include "core/timing.h"
#include "preview_scene_fixtures.h"
#include "ui/preview_timeline.h"

using namespace hydra;
using hydra::app::PreviewFillState;
namespace fx = hydra::test_fixtures;

namespace {

// A 10 s song 1,000 px wide, so one pixel is 10 ms. Every row is 10 px tall
// and a fill's hit box 10 px square: sizes for the test only.
constexpr double kLengthMs = 10000.0;
constexpr float kWidth = 1000.0f;

ui::TimelineMetrics metrics() {
    ui::TimelineMetrics m;
    for (float& h : m.row_h) h = 10.0f;
    m.fill_size = 10.0f;
    return m;
}

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
    const std::optional<ui::TimelineLayout> l = ui::timeline_layout(s, kLengthMs, kWidth, metrics());
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
    // No usable song length, no timeline.
    CHECK_FALSE(ui::timeline_layout(s, 0.0, kWidth, metrics()).has_value());
}

TEST_CASE("preview timeline: a point on a fill names that fill and a point on the axis names its time") {
    const app::PreviewScene s = scene_with_fills();
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, kWidth, metrics());
    const ui::TimelineRect& fills = l.row(ui::TimelineRow::Fills);

    const ui::TimelineHit on_fill = ui::timeline_hit(l, 202.0f, fills.y0 + 3.0f);
    CHECK(on_fill.kind == ui::TimelineHit::Kind::Fill);
    CHECK(on_fill.index == 0);

    const ui::TimelineRect& acts = l.row(ui::TimelineRow::Activations);
    const ui::TimelineHit on_band = ui::timeline_hit(l, 600.0f, acts.y0 + 1.0f);
    CHECK(on_band.kind == ui::TimelineHit::Kind::Band);
    CHECK(on_band.index == 0);

    const ui::TimelineRect& axis = l.row(ui::TimelineRow::Axis);
    const ui::TimelineHit on_axis = ui::timeline_hit(l, 350.0f, axis.y0 + 1.0f);
    CHECK(on_axis.kind == ui::TimelineHit::Kind::Time);
    CHECK(on_axis.ms == doctest::Approx(3500.0));

    CHECK(ui::timeline_hit(l, 350.0f, l.height() + 1.0f).kind == ui::TimelineHit::Kind::None);
}

TEST_CASE("preview timeline: the saved line has a point at each segment start and end and a mark at kSpActivationBars") {
    app::PreviewScene s = fx::timed_scene();
    // Two bars by 3 s, then a phrase jumps it to three, then an activation
    // drains it to none by 8 s.
    s.sp_meter.cap = 4;
    s.sp_meter.segments = {{0.0, 3000.0, 2.0, 2.0}, {3000.0, 5000.0, 3.0, 3.0},
                           {5000.0, 8000.0, 3.0, 0.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, kWidth, metrics());
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
    // A 65 s song on the same 1,000 px, so the axis has three labels.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 65000.0, kWidth, m);
    // Pinned from one run.
    REQUIRE(l.axis_ticks.size() == 3);
    CHECK(l.axis_ticks[0].ms == doctest::Approx(0.0));
    CHECK(l.axis_ticks[1].ms == doctest::Approx(30000.0));
    CHECK(l.axis_ticks[2].ms == doctest::Approx(60000.0));
    CHECK(l.axis_ticks[2].x == doctest::Approx(923.0769f));
    // The Activations row is 50 to 60 px down; its band stands 2 px in.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(52.0f));
    CHECK(l.bands[0].rect.y1 == doctest::Approx(58.0f));
    // The Fills row starts at 60 px; a fill's 10 px box is centred 6 px down.
    REQUIRE(l.fills.size() == 3);
    CHECK(l.fills[0].hit.y0 == doctest::Approx(61.0f));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(71.0f));
    // The saved row is 30 to 40 px down: none at 39, the cap at 31.
    REQUIRE(l.saved_line.size() == 2);
    CHECK(l.saved_line[0].y == doctest::Approx(39.0f));
    CHECK(l.saved_line[1].y == doctest::Approx(31.0f));
    CHECK(l.activate_y == doctest::Approx(35.0f));
}

TEST_CASE("preview timeline: a huge song length gets labels by the width, not by the length") {
    const app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.axis_label_gap = 50.0f;
    // A 10-minute song on 1,000 px: 30 s is 50 px, exactly the gap, so every
    // step is labelled: 20 labels. Pinned from one run.
    const ui::TimelineLayout normal = *ui::timeline_layout(s, 600000.0, kWidth, m);
    CHECK(normal.axis_ticks.size() == 20);
    // Twice the length: 30 s is 25 px, so every second step: 20 labels, 60 s apart.
    const ui::TimelineLayout longer = *ui::timeline_layout(s, 1200000.0, kWidth, m);
    REQUIRE(longer.axis_ticks.size() == 20);
    CHECK(longer.axis_ticks[1].ms == doctest::Approx(60000.0));
    // A length a corrupt song.ini could state: still a handful of labels.
    const ui::TimelineLayout huge = *ui::timeline_layout(s, 1e300, kWidth, m);
    CHECK(huge.axis_ticks.size() <= 20);
}
