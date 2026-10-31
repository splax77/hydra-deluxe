// The Preview's whole-song timeline: its pure layout and hit test
// (ui/preview_timeline). No ImGui context is needed; the on-screen click is
// the Path Builder's GUI test.
#include "doctest.h"

#include "core/timing.h"
#include "fixed_pitch.h"  // ten_per_char
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
    // With no ghost the ghost row is 0 tall, so the Activations row is 40 to
    // 50 px down; its band stands 2 px in.
    REQUIRE(l.bands.size() == 1);
    CHECK(l.bands[0].rect.y0 == doctest::Approx(42.0f));
    CHECK(l.bands[0].rect.y1 == doctest::Approx(48.0f));
    // The Fills row starts at 50 px; a fill's 10 px box is centred 6 px down.
    REQUIRE(l.fills.size() == 3);
    CHECK(l.fills[0].hit.y0 == doctest::Approx(51.0f));
    CHECK(l.fills[0].hit.y1 == doctest::Approx(61.0f));
    // The saved row is 30 to 40 px down: none at 39, the cap at 31.
    REQUIRE(l.saved_line.size() == 2);
    CHECK(l.saved_line[0].y == doctest::Approx(39.0f));
    CHECK(l.saved_line[1].y == doctest::Approx(31.0f));
    CHECK(l.activate_y == doctest::Approx(35.0f));
}

TEST_CASE("preview timeline: the ghost path's windows are bands in their own row, which is 0 tall with no ghost") {
    app::PreviewScene s = scene_with_fills();
    ui::TimelineMetrics m = metrics();
    m.band_inset = 2.0f;

    // No ghost: the row has no height and no bands, and the rows below it
    // move up.
    const ui::TimelineLayout bare = *ui::timeline_layout(s, kLengthMs, kWidth, m);
    CHECK(bare.ghost_bands.empty());
    CHECK(bare.row(ui::TimelineRow::Ghost).y0 == doctest::Approx(40.0f));
    CHECK(bare.row(ui::TimelineRow::Ghost).y1 == doctest::Approx(40.0f));
    CHECK(bare.row(ui::TimelineRow::Activations).y0 == doctest::Approx(40.0f));
    CHECK(bare.height() == doctest::Approx(60.0f));

    // Two ghost windows. Pinned from one run.
    s.ghost_windows = {{1000.0, 2500.0}, {6000.0, 9000.0}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, kWidth, m);
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
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, kWidth, metrics());
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

// Offered fills ending at each of `end_ms`, chord tick = end, each with a
// four-character label 40 px wide.
struct LabelledFills {
    app::PreviewScene scene;
    std::vector<ui::TimelineFillLabel> labels;
};
LabelledFills labelled_fills(const std::vector<double>& end_ms) {
    LabelledFills out{fx::timed_scene(), {}};
    for (double e : end_ms) {
        app::PreviewFill f = fx::fill(fx::span(e - 500.0, e), PreviewFillState::Offered);
        f.chord_tick = static_cast<int64_t>(e);
        out.scene.fills.push_back(f);
        out.labels.push_back({f.chord_tick, "-1.0", 0});
    }
    return out;
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

TEST_CASE("preview timeline: a cost label that would touch the one before it drops to a second line") {
    const ui::TimelineMetrics m = label_metrics();

    // Fills at 200, 240 and 300 px; each label is 40 px, centred. The second
    // starts at 220 px, where the first ends, so it touches and drops to line
    // 1. The third starts at 280 px, clear of the first, so it stays on line
    // 0. Pinned from one run.
    const LabelledFills touch = labelled_fills({2000.0, 2400.0, 3000.0});
    const ui::TimelineLayout l = *ui::timeline_layout(touch.scene, kLengthMs, kWidth, m, touch.labels);
    REQUIRE(l.cost_labels.size() == 3);
    CHECK(l.cost_labels[0].line == 0);
    CHECK(l.cost_labels[0].rect.x0 == doctest::Approx(180.0f));
    CHECK(l.cost_labels[0].rect.x1 == doctest::Approx(220.0f));
    CHECK(l.cost_labels[1].line == 1);
    CHECK(l.cost_labels[1].rect.x0 == doctest::Approx(220.0f));
    CHECK(l.cost_labels[2].line == 0);
    CHECK(l.cost_clashes == 0);
    // The Fills row starts at 50 px; line 0 is 3 px down, line 1 one 5 px
    // line under it, and the row is that one line taller.
    CHECK(l.cost_labels[0].rect.y0 == doctest::Approx(53.0f));
    CHECK(l.cost_labels[1].rect.y0 == doctest::Approx(58.0f));
    CHECK(l.cost_labels[1].rect.y1 == doctest::Approx(63.0f));
    CHECK(l.row(ui::TimelineRow::Fills).y1 == doctest::Approx(65.0f));
    CHECK(l.height() == doctest::Approx(65.0f));

    // One pixel further, the second label starts at 221 px and clears the
    // first: every label on line 0, and the row keeps its height.
    const LabelledFills clear = labelled_fills({2000.0, 2410.0, 3000.0});
    const ui::TimelineLayout c = *ui::timeline_layout(clear.scene, kLengthMs, kWidth, m, clear.labels);
    REQUIRE(c.cost_labels.size() == 3);
    for (const ui::TimelineCostLabel& lab : c.cost_labels) CHECK(lab.line == 0);
    CHECK(c.height() == doctest::Approx(60.0f));
}

TEST_CASE("preview timeline: the stagger measures a label where it is kept inside the timeline, not where it would be centred") {
    // Fills at 950 and 995 px, 40 px labels. Centred, the second would start
    // at 975 px, clear of the first's end at 970. Kept inside the 1,000 px
    // timeline it starts at 960 px and touches it, so it drops to line 1.
    // Pinned from one run.
    const LabelledFills f = labelled_fills({9500.0, 9950.0});
    const ui::TimelineLayout l = *ui::timeline_layout(f.scene, kLengthMs, kWidth, label_metrics(), f.labels);
    REQUIRE(l.cost_labels.size() == 2);
    CHECK(l.cost_labels[0].rect.x1 == doctest::Approx(970.0f));
    CHECK(l.cost_labels[1].rect.x0 == doctest::Approx(960.0f));
    CHECK(l.cost_labels[1].rect.x1 == doctest::Approx(1000.0f));
    CHECK(l.cost_labels[0].line == 0);
    CHECK(l.cost_labels[1].line == 1);
}

TEST_CASE("preview timeline: a cost label that touches the last label on both lines goes where the last one ends first, and is counted") {
    // Fills at 200, 210 and 220 px: line 0 ends at 220, line 1 at 230, and
    // the third starts at 200, touching both. Pinned from one run.
    const LabelledFills f = labelled_fills({2000.0, 2100.0, 2200.0});
    const ui::TimelineLayout l = *ui::timeline_layout(f.scene, kLengthMs, kWidth, label_metrics(), f.labels);
    REQUIRE(l.cost_labels.size() == 3);
    CHECK(l.cost_labels[0].line == 0);
    CHECK(l.cost_labels[1].line == 1);
    CHECK(l.cost_labels[2].line == 0);
    CHECK(l.cost_clashes == 1);
}

TEST_CASE("preview timeline: a section name too long for its section ends in an ellipsis") {
    app::PreviewScene s = fx::timed_scene();
    s.sections ={{0, 0.0, "Intro"}, {400, 400.0, "Verse 1b"}, {1000, 1000.0, "Chorus"}};
    const ui::TimelineLayout l = *ui::timeline_layout(s, kLengthMs, kWidth, label_metrics());
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
    // A 61 s song on 1,000 px: labels at 0, 30 and 60 s, each 40 px. Pinned
    // from one run.
    const ui::TimelineLayout l = *ui::timeline_layout(s, 61000.0, kWidth, label_metrics());
    REQUIRE(l.axis_ticks.size() == 3);
    // 0:00 would start 20 px left of the timeline, in the label column; it
    // starts at the timeline's left edge instead.
    CHECK(l.axis_ticks[0].text == "0:00");
    CHECK(l.axis_ticks[0].x == doctest::Approx(0.0f));
    CHECK(l.axis_ticks[0].text_x == doctest::Approx(0.0f));
    // 0:30 is centred on its moment.
    CHECK(l.axis_ticks[1].text_x == doctest::Approx(471.8033f));
    // 1:00 at 983.6 px would end past the right edge; it ends on it.
    CHECK(l.axis_ticks[2].text == "1:00");
    CHECK(l.axis_ticks[2].text_x == doctest::Approx(960.0f));
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
