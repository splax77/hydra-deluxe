// Tests for render/overlay_layout: where the highway lands on screen, and the
// scale that keeps the Preview's text boxes beside it. Device-free.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <initializer_list>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_view.h"
#include "core/model.h"
#include "corpus_default_cfg.h"
#include "corpus_util.h"
#include "render/highway_draw.h"
#include "render/overlay_layout.h"
#include "render/preview_config.h"
#include "fixed_pitch.h"  // ten_per_char
#include "preview_config_util.h"

using namespace hydra::render;

TEST_CASE("track_rect: the clamped size, the aspect and the top row") {
    const PreviewConfig& cfg = shipped_preview_config();

    // A wide image: its width times the config's ratio is taller than the
    // image, so the image's height sets the track.
    const int wide_w = 1920, wide_h = 1080;
    const TrackRect wide = track_rect(cfg, wide_w, wide_h);
    REQUIRE(wide_w * cfg.view.height_width_ratio > wide_h);
    CHECK(wide.width == wide_w);
    CHECK(wide.height == wide_h);
    CHECK(wide.track_height == wide.height);
    CHECK(wide.top == 0);
    CHECK(wide.aspect == static_cast<float>(wide.width) / static_cast<float>(wide.track_height));
    CHECK(track_height(cfg, wide_w, wide_h) == wide.track_height);

    // A tall image: its width times the ratio sets the track, which hugs the
    // image's bottom edge.
    const int tall_w = 600, tall_h = 1080;
    const TrackRect tall = track_rect(cfg, tall_w, tall_h);
    const long ratio_rows = std::lround(tall_w * cfg.view.height_width_ratio);
    REQUIRE(ratio_rows < tall_h);
    CHECK(tall.track_height == ratio_rows);
    CHECK(tall.top == tall_h - tall.track_height);
    CHECK(tall.aspect == static_cast<float>(tall_w) / static_cast<float>(tall.track_height));
    CHECK(track_height(cfg, tall_w, tall_h) == tall.track_height);

    // An empty image: a 1 x 1 track.
    const TrackRect empty = track_rect(cfg, 0, 0);
    CHECK(empty.width == 1);
    CHECK(empty.height == 1);
    CHECK(empty.track_height == 1);
    CHECK(empty.top == 0);
    CHECK(track_height(cfg, 0, 0) == empty.track_height);
}

TEST_CASE("highway_span_at: passes through the projected railing corners") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PreviewConfig::Track& T = cfg.track;
    const int w = 1200, h = 400;
    // The railings' outer edges, from the highway's own railing_outer_x.
    const float xl = railing_outer_x(cfg, false);
    const float xr = railing_outer_x(cfg, true);
    // The railing tops' corners on the strike line.
    const ImagePoint r = project_to_image(cfg, w, h, {xr, T.railing_y_top, T.z_now});
    const ImagePoint l = project_to_image(cfg, w, h, {xl, T.railing_y_top, T.z_now});
    // The corners lie below the far end's row, so the span there is read off
    // the railing's own line, not the far end's clamp.
    const float far_row = project_to_image(cfg, w, h, {xr, T.railing_y_top, T.z_future}).y;
    REQUIRE(r.y > far_row);
    CHECK_MESSAGE(highway_span_at(cfg, w, h, r.y).right >= r.x - 0.01f,
                  "the span's right edge, below the far end, misses the right railing's corner");
    CHECK_MESSAGE(highway_span_at(cfg, w, h, l.y).left <= l.x + 0.01f,
                  "the span's left edge, below the far end, misses the left railing's corner");
    // A point on a lane, halfway down the highway, lies inside the span at its
    // own row.
    const ImagePoint lane =
        project_to_image(cfg, w, h, {0.9f * xr, T.y, 0.5f * (T.z_now + T.z_future)});
    const HighwaySpan s = highway_span_at(cfg, w, h, lane.y);
    CHECK(lane.x < s.right);
    CHECK(lane.x > s.left);
}

TEST_CASE("highway_span_at: symmetric, and wider nearer the camera") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PreviewConfig::Track& T = cfg.track;
    const int w = 1200, h = 400;
    // The rows of the highway's far end and of its strike line, at the
    // floor's middle.
    const float far_row = project_to_image(cfg, w, h, {0.0f, T.y, T.z_future}).y;
    const float strike_row = project_to_image(cfg, w, h, {0.0f, T.y, T.z_now}).y;
    REQUIRE(far_row < strike_row);
    const HighwaySpan far_end = highway_span_at(cfg, w, h, far_row);
    const HighwaySpan strike = highway_span_at(cfg, w, h, strike_row);
    CHECK(far_end.left + far_end.right ==
          doctest::Approx(static_cast<double>(w)).epsilon(0.001));
    CHECK(strike.left + strike.right == doctest::Approx(static_cast<double>(w)).epsilon(0.001));
    CHECK(strike.right - strike.left > far_end.right - far_end.left);
}

TEST_CASE("highway_span_at: a narrower image leaves the highway's size alone") {
    // The track height follows the image height here, so the highway is the
    // same size; a narrower image only trims the empty sides.
    PreviewConfig cfg = shipped_preview_config();
    const HighwaySpan wide = highway_span_at(cfg, 1200, 400, 390.0f);
    const HighwaySpan narrow = highway_span_at(cfg, 600, 400, 390.0f);
    CHECK(narrow.right - narrow.left ==
          doctest::Approx(static_cast<double>(wide.right - wide.left)).epsilon(0.01));
}

namespace {

// A point on the highway's floor in lane-space x, at chart time t, through
// the highway's own time_to_z.
Vec3 floor_point(const PreviewConfig& cfg, float x, double now_s, double t, double speed) {
    return {x, cfg.track.y, static_cast<float>(time_to_z(cfg, now_s, t, speed))};
}

}  // namespace

// The round trip's 1 ms tolerance is the Path Builder plan's (RD-1).
TEST_CASE("image_to_floor_time inverts project_to_image on the floor") {
    const PreviewConfig cfg = shipped_preview_config();
    const double now_s = 12.0;
    for (const double speed : {1.0, 1.5}) {
        const double far_t = far_time(cfg, now_s, speed);
        // A wide image (the track fills its height) and a tall one (the track
        // hugs the bottom, below an empty band).
        for (const std::pair<int, int>& size : {std::pair{1200, 400}, std::pair{600, 1080}}) {
            const int w = size.first, h = size.second;
            for (const double f : {0.01, 0.25, 0.5, 0.75, 0.99}) {
                const double t = now_s + f * (far_t - now_s);
                for (const float x : {-0.9f, 0.0f, 0.6f}) {
                    CAPTURE(speed);
                    CAPTURE(w);
                    CAPTURE(f);
                    CAPTURE(x);
                    const ImagePoint p =
                        project_to_image(cfg, w, h, floor_point(cfg, x, now_s, t, speed));
                    const std::optional<double> back =
                        image_to_floor_time(cfg, w, h, p, now_s, speed);
                    REQUIRE(back.has_value());
                    CHECK(std::fabs(*back - t) <= 0.001);
                }
            }
        }
    }
}

TEST_CASE("image_to_floor_time refuses a point above the far end") {
    const PreviewConfig cfg = shipped_preview_config();
    const double now_s = 12.0, speed = 1.0;
    const int w = 1200, h = 400;
    const double past_far = far_time(cfg, now_s, speed) + 0.25;
    const ImagePoint p = project_to_image(cfg, w, h, floor_point(cfg, 0.0f, now_s, past_far, speed));
    CHECK_FALSE(image_to_floor_time(cfg, w, h, p, now_s, speed).has_value());
}

TEST_CASE("image_to_floor_time refuses a point below the strike line") {
    const PreviewConfig cfg = shipped_preview_config();
    const double now_s = 12.0, speed = 1.0;
    const int w = 1200, h = 400;
    const ImagePoint p =
        project_to_image(cfg, w, h, floor_point(cfg, 0.0f, now_s, now_s - 0.1, speed));
    CHECK_FALSE(image_to_floor_time(cfg, w, h, p, now_s, speed).has_value());
}

namespace {

// The Preview panel's text sizes at scale 1, as ui/preview_panel.cpp sets
// them. The body size and the inset are the config's time box. The rest are
// that file's own factors: a line is 1.25 times its text size (line_height),
// two panels sit 6 px apart, the next-activation box's notation is 17/12 of
// the body size (kNextActNotationScale), the large type is 1.8 times the body
// size with a line 1.2 times its own size, the header's last 3 words stay
// together (kHeaderCountWords), the right column starts 10 px down (v_margin)
// and ends 30 px from the image's right edge (the gauge's 10 px inset, its
// 14 px bar and the 6 px before it). They sit in that file's unnamed
// namespace, so a test cannot read them; they are typed here once.
struct PanelText {
    float size = 0.0f;
    float inset = 0.0f;
    float gap = 0.0f;
    float notation_size = 0.0f;
    float big_size = 0.0f;
    float big_line_h = 0.0f;
    size_t header_count_words = 0;
    float top_margin = 0.0f;
    float gauge_room = 0.0f;
};

float panel_line_h(float size) { return size * 1.25f; }

PanelText panel_text(const PreviewConfig& cfg) {
    PanelText p;
    p.size = cfg.text.time_box_size;
    p.inset = cfg.text.time_box_margin;
    p.gap = 6.0f;
    p.notation_size = p.size * 17.0f / 12.0f;
    p.big_size = p.size * 1.8f;
    p.big_line_h = p.big_size * 1.2f;
    p.header_count_words = 3;
    p.top_margin = 10.0f;
    p.gauge_room = 10.0f + 14.0f + 6.0f;
    return p;
}

// `text` at text size `size`: ten_per_char's 10 px a character at the body
// size, in proportion at any other size.
float fixed_width(const std::string& text, float size) {
    return ten_per_char(text) * size / shipped_preview_config().text.time_box_size;
}

// One line of `text` at `size`, measured by fixed_width, `line_h` tall.
BoxLine fixed_line(const std::string& text, float size, float line_h) {
    BoxLine l;
    l.text = text;
    l.size = size;
    l.width = fixed_width(text, size);
    l.line_h = line_h;
    return l;
}

// A text box of `texts` at the body size, one line each.
TextBox box_of(std::initializer_list<const char*> texts) {
    const float size = shipped_preview_config().text.time_box_size;
    TextBox box;
    for (const char* t : texts) box.lines.push_back(fixed_line(t, size, panel_line_h(size)));
    return box;
}

// A corpus chart's Preview text, from the app's own box builders: the first
// chart, in corpus order, whose best path has an activation and whose song
// has a solo. Its time box at the first activation (playback ending at the
// last note), its drain box at the start, its next-activation box at each
// activation, and its solo boxes at their widest (build_solo_box_samples).
struct RealOverlayText {
    std::string chart;
    hydra::app::PreviewTimeBox time;
    hydra::app::PreviewDrainBox drain;
    std::vector<hydra::app::PreviewNextActBox> next;
    std::vector<hydra::app::PreviewSoloBox> solos;
};

RealOverlayText find_real_overlay_text() {
    const hydra::app::AnalysisSettings& cfg = corpus::default_cfg();
    for (const std::string& chart : corpus::chart_paths()) {
        try {
            const hydra::HydraRecord& rec = corpus::analyzed(chart, cfg);
            if (rec.paths.empty()) continue;
            const hydra::Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
            const hydra::app::PreviewScene scene =
                hydra::app::build_preview_scene(song, &rec.best_path(), cfg.sp_cap, cfg.rules);
            if (scene.activations.empty() || scene.notes.empty()) continue;
            RealOverlayText t;
            t.solos = hydra::app::build_solo_box_samples(scene);
            if (t.solos.empty()) continue;
            const double end_ms = scene.notes.back().ms;
            t.chart = chart;
            t.time = hydra::app::build_time_box(scene, scene.activations.front().ms, end_ms);
            t.drain = hydra::app::build_drain_box(scene, 0.0, end_ms);
            for (const hydra::app::PreviewActivation& a : scene.activations)
                t.next.push_back(hydra::app::build_next_act_box(scene, a.ms, end_ms, cfg.prodrums));
            return t;
        } catch (const std::exception&) {
            continue;
        }
    }
    return RealOverlayText{};
}

const RealOverlayText& real_overlay_text() {
    static const RealOverlayText t = find_real_overlay_text();
    REQUIRE_MESSAGE(!t.chart.empty(),
                    "no corpus chart has a best path with an activation and a song with a solo");
    return t;
}

// The real boxes' lines, as the panel lines them.
TextBox time_box_of(const hydra::app::PreviewTimeBox& t, const PanelText& p) {
    TextBox box;
    for (const std::string& l : {t.position + "  of " + t.length, t.tempo, t.section_line})
        if (!l.empty()) box.lines.push_back(fixed_line(l, p.size, panel_line_h(p.size)));
    return box;
}

TextBox drain_box_of(const hydra::app::PreviewDrainBox& d, const PanelText& p) {
    TextBox box;
    for (const std::string& l : {d.header, d.rate, d.detail})
        box.lines.push_back(fixed_line(l, p.size, panel_line_h(p.size)));
    return box;
}

// The next-activation box unwrapped: the header, the notation in its larger
// type, then the "at ..." line.
TextBox next_box_of(const hydra::app::PreviewNextActBox& n, const PanelText& p) {
    TextBox box;
    box.lines.push_back(fixed_line(n.header, p.size, panel_line_h(p.size)));
    if (!n.notation.empty())
        box.lines.push_back(fixed_line(n.notation, p.notation_size, panel_line_h(p.notation_size)));
    box.lines.push_back(fixed_line(n.detail, p.size, panel_line_h(p.size)));
    return box;
}

// The solo box: its headline in the large type, then its detail.
TextBox solo_box_of(const hydra::app::PreviewSoloBox& s, const PanelText& p) {
    TextBox box;
    box.lines.push_back(fixed_line(s.headline, p.big_size, p.big_line_h));
    if (!s.detail.empty()) box.lines.push_back(fixed_line(s.detail, p.size, panel_line_h(p.size)));
    return box;
}

// The real boxes at scale 1, in an image `h` rows tall: the time box (the
// left column, with the score box left out), the drain box, and the widest
// and tallest of the path's next-activation boxes (unwrapped) and of the
// solo boxes, stacked by overlay_tops with the drain box drawn and no
// next-activation reserve.
struct RealBoxes {
    BoxExtent time, drain, next, solo;
    OverlayTops tops;
};

RealBoxes real_boxes(const PanelText& p, int h) {
    const RealOverlayText& real = real_overlay_text();
    RealBoxes b;
    b.time = measure_text_box(time_box_of(real.time, p), p.inset);
    b.drain = measure_text_box(drain_box_of(real.drain, p), p.inset);
    for (const hydra::app::PreviewNextActBox& n : real.next) {
        const BoxExtent e = measure_text_box(next_box_of(n, p), p.inset);
        b.next.w = (std::max)(b.next.w, e.w);
        b.next.h = (std::max)(b.next.h, e.h);
    }
    for (const hydra::app::PreviewSoloBox& s : real.solos) {
        const BoxExtent e = measure_text_box(solo_box_of(s, p), p.inset);
        b.solo.w = (std::max)(b.solo.w, e.w);
        b.solo.h = (std::max)(b.solo.h, e.h);
    }
    OverlayHeights heights;
    heights.time_h = b.time.h;
    heights.drain_h = b.drain.h;
    heights.gap = p.gap;
    heights.top_margin = p.top_margin;
    heights.mid_row = static_cast<float>(h) * 0.5f;
    b.tops = overlay_tops(heights, /*drain_drawn=*/true, /*score_shown=*/false);
    return b;
}

// The fit's right column in an image `w` wide: the drain box, against the
// edge left of the gauge.
OverlayBoxes right_column(const RealBoxes& b, const PanelText& p, int w) {
    OverlayBoxes out;
    out.right_edge = static_cast<float>(w) - p.gauge_room;
    out.right_w = b.drain.w;
    out.right_h = b.drain.h;
    out.right_top = b.tops.drain;
    out.gap = p.gap;
    return out;
}

// A next-activation box's three pieces at `scale`, as the panel wraps them:
// the header with its count kept together, the notation never broken, the
// "at ..." line.
std::vector<TextPiece> next_pieces(const hydra::app::PreviewNextActBox& n, const PanelText& p,
                                   float scale) {
    const float body = p.size * scale;
    const float notation = p.notation_size * scale;
    auto at = [](float size) {
        return [size](const std::string& s) { return fixed_width(s, size); };
    };
    std::vector<TextPiece> pieces(3);
    pieces[0] = {n.header, at(body), panel_line_h(body), p.header_count_words};
    pieces[1] = {n.notation, at(notation), n.notation.empty() ? 0.0f : panel_line_h(notation),
                 kKeepWhole};
    pieces[2] = {n.detail, at(body), panel_line_h(body), 1};
    return pieces;
}

// The widest wrapped next-activation box at scale 1 under the drain box, in
// an image `w` x `h`: what the panel's scale reserves for the path.
BoxExtent next_reserve(const PreviewConfig& cfg, const PanelText& p, const RealBoxes& b, int w,
                       int h) {
    const float edge = static_cast<float>(w) - p.gauge_room;
    BoxExtent out;
    for (const hydra::app::PreviewNextActBox& n : real_overlay_text().next) {
        const WrappedText t =
            wrap_right_box(cfg, w, h, edge, b.tops.next, p.inset, p.gap, next_pieces(n, p, 1.0f));
        out.w = (std::max)(out.w, t.text_w + p.inset * 2.0f);
        out.h = (std::max)(out.h, t.text_h + p.inset * 2.0f);
    }
    return out;
}

}  // namespace

TEST_CASE("overlay_scale: full size with room, floored when narrow, clear in between") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PanelText p = panel_text(cfg);
    CAPTURE(real_overlay_text().chart);
    const int h = 390;
    const RealBoxes real = real_boxes(p, h);
    // The real time, drain and next-activation boxes, unwrapped.
    auto boxes = [&](int w) {
        OverlayBoxes b = right_column(real, p, w);
        b.left_w = real.time.w;
        b.left_h = real.tops.left_bottom;
        b.next_w = real.next.w;
        b.next_h = real.next.h;
        b.next_top = real.tops.next;
        return b;
    };
    const int narrowest = 150, widest = 3000;
    CHECK(overlay_scale(cfg, widest, h, boxes(widest)) == 1.0f);
    CHECK(overlay_scale(cfg, narrowest, h, boxes(narrowest)) == doctest::Approx(kOverlayMinScale));

    float prev = 0.0f;
    bool saw_partial = false;
    for (int w = narrowest; w <= widest; w += 10) {
        CAPTURE(w);
        const OverlayBoxes b = boxes(w);
        const float s = overlay_scale(cfg, w, h, b);
        CHECK(s >= prev - 1e-6f);  // never falls as the image widens
        prev = s;
        if (s > kOverlayMinScale && s < 1.0f) {
            saw_partial = true;
            const float left_room = highway_span_at(cfg, w, h, b.left_h).left - b.gap;
            CHECK(b.left_w * s <= left_room + 0.01f);
            CHECK(b.right_w * s <=
                  right_room(cfg, w, h, b.right_edge, b.right_top + b.right_h, b.gap) + 0.01f);
            CHECK(b.next_w * s <=
                  right_room(cfg, w, h, b.right_edge, b.next_top + b.next_h, b.gap) + 0.01f);
        }
    }
    CHECK(saw_partial);
    // The next-activation box, lower down where the highway is wider, is the
    // box that limits the scale at some width.
    bool saw_next_limit = false;
    for (int w = narrowest; w <= widest; w += 10) {
        OverlayBoxes b = boxes(w);
        const float with_next = overlay_scale(cfg, w, h, b);
        b.next_w = 0.0f;
        if (overlay_scale(cfg, w, h, b) > with_next) saw_next_limit = true;
    }
    CHECK(saw_next_limit);
    // No boxes: nothing to fit.
    CHECK(overlay_scale(cfg, narrowest, h, OverlayBoxes{}) == 1.0f);
}

namespace {

// Box heights to stack, each named by its field.
OverlayHeights named_heights() {
    OverlayHeights s;
    s.time_h = 70.0f;
    s.score_h = 90.0f;
    s.drain_h = 70.0f;
    s.gap = 6.0f;
    s.top_margin = 10.0f;
    return s;
}

}  // namespace

TEST_CASE("overlay_tops: score under time, next under drain or in its place") {
    const OverlayHeights s = named_heights();
    const bool drain_drawn = true, score_shown = true;
    const OverlayTops all = overlay_tops(s, drain_drawn, score_shown);
    CHECK(all.time == 0.0f);  // the image's top edge
    CHECK(all.score == doctest::Approx(all.time + s.time_h + s.gap));
    CHECK(all.left_bottom == doctest::Approx(all.score + s.score_h));
    CHECK(all.drain == doctest::Approx(s.top_margin));
    CHECK(all.next == doctest::Approx(all.drain + s.drain_h + s.gap));
    const OverlayTops bare = overlay_tops(s, !drain_drawn, !score_shown);
    CHECK(bare.left_bottom == doctest::Approx(bare.time + s.time_h));
    CHECK(bare.next == doctest::Approx(bare.drain));
}

TEST_CASE("overlay_tops: the solo box from the middle row, under the next box's reserve") {
    OverlayHeights s = named_heights();
    s.mid_row = 195.0f;
    const float next_top = overlay_tops(s, true, true).next;
    // A next-activation box that ends above the middle row: the solo box
    // starts on the middle row.
    s.next_h = 90.0f;
    REQUIRE(next_top + s.next_h + s.gap < s.mid_row);
    CHECK(overlay_tops(s, true, true).solo == doctest::Approx(s.mid_row));
    // One that reaches past it: the solo box goes under it, a gap below.
    s.next_h = 150.0f;
    REQUIRE(next_top + s.next_h + s.gap > s.mid_row);
    CHECK(overlay_tops(s, true, true).solo == doctest::Approx(next_top + s.next_h + s.gap));
    // No next-activation box and a middle row above the right column's start:
    // the solo box starts where the next box would, with the drain box drawn
    // or not.
    s.next_h = 0.0f;
    s.mid_row = 5.0f;
    for (const bool drain_drawn : {false, true}) {
        CAPTURE(drain_drawn);
        const OverlayTops t = overlay_tops(s, drain_drawn, true);
        REQUIRE(t.next > s.mid_row);
        CHECK(t.solo == doctest::Approx(t.next));
    }
}

TEST_CASE("overlay_scale: the solo box shrinks the boxes with the rest") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PanelText p = panel_text(cfg);
    CAPTURE(real_overlay_text().chart);
    const int h = 390;
    const RealBoxes real = real_boxes(p, h);
    // The real drain box and the chart's widest solo box, from the middle row.
    auto boxes = [&](int w) {
        OverlayBoxes b = right_column(real, p, w);
        b.solo_w = real.solo.w;
        b.solo_h = real.solo.h;
        b.solo_top = real.tops.solo;
        return b;
    };
    // Lower down the highway is wider, so the solo box limits the scale at
    // some width, and wherever the scale is partial it clears the highway.
    bool saw_solo_limit = false;
    for (int w = 150; w <= 3000; w += 10) {
        CAPTURE(w);
        OverlayBoxes b = boxes(w);
        const float s = overlay_scale(cfg, w, h, b);
        if (s > kOverlayMinScale && s < 1.0f)
            CHECK(b.solo_w * s <=
                  right_room(cfg, w, h, b.right_edge, b.solo_top + b.solo_h, b.gap) + 0.01f);
        b.solo_w = 0.0f;
        if (overlay_scale(cfg, w, h, b) > s) saw_solo_limit = true;
    }
    CHECK(saw_solo_limit);
}

TEST_CASE("measure_text_box: the widest line and every line's height, with the inset all round") {
    const float inset = 10.0f;
    // The time box's lines, the first the wider.
    const TextBox time = box_of({"m12.3.240  of m96.3.240", "BPM 120.000 4/4"});
    const BoxExtent e = measure_text_box(time, inset);
    REQUIRE(time.lines[0].width > time.lines[1].width);
    CHECK(e.w == doctest::Approx(time.lines[0].width + 2.0f * inset));
    CHECK(e.h == doctest::Approx(time.lines[0].line_h + time.lines[1].line_h + 2.0f * inset));
    // A star row is one more line, as wide as the caller measured it: here
    // wider than the score above it.
    TextBox score = box_of({"12,345"});
    BoxLine stars;
    stars.kind = BoxLine::Kind::Stars;
    stars.width = 99.0f;
    stars.line_h = score.lines[0].line_h;
    REQUIRE(stars.width > score.lines[0].width);
    score.lines.push_back(stars);
    const BoxExtent s = measure_text_box(score, inset);
    CHECK(s.w == doctest::Approx(stars.width + 2.0f * inset));
    CHECK(s.h == doctest::Approx(score.lines[0].line_h + stars.line_h + 2.0f * inset));
    // No lines: the inset alone.
    const BoxExtent none = measure_text_box(TextBox{}, inset);
    CHECK(none.w == doctest::Approx(2.0f * inset));
    CHECK(none.h == doctest::Approx(2.0f * inset));
}

TEST_CASE("text_box_line_origins: the drain box's lines share one left x") {
    // The drain box's three lines are of three widths; every one starts at
    // the box's left edge plus the inset, one line below the last.
    const float inset = 10.0f;
    const ImagePoint min{500.0f, 10.0f};
    const TextBox drain = box_of({"SP drain (if activated)", "1 bar / 4.0 s", "full meter 16.0 s"});
    const std::vector<ImagePoint> at = text_box_line_origins(drain, min, inset);
    REQUIRE(at.size() == drain.lines.size());
    for (const ImagePoint& o : at) CHECK(o.x == doctest::Approx(min.x + inset));
    CHECK(at[0].y == doctest::Approx(min.y + inset));
    for (size_t i = 1; i < at.size(); ++i) {
        CAPTURE(i);
        CHECK(at[i].y == doctest::Approx(at[i - 1].y + drain.lines[i - 1].line_h));
    }
    // The last line ends the inset above the box's bottom edge.
    const BoxExtent e = measure_text_box(drain, inset);
    CHECK(at.back().y + drain.lines.back().line_h + inset == doctest::Approx(min.y + e.h));
}

namespace {

// The real next-activation box's "at ..." line with the most characters.
std::string longest_real_detail() {
    std::string out;
    for (const hydra::app::PreviewNextActBox& n : real_overlay_text().next)
        if (ten_per_char(n.detail) > ten_per_char(out)) out = n.detail;
    REQUIRE_FALSE(out.empty());
    return out;
}

// The words of `text`, split at its spaces.
std::vector<std::string> words_of(const std::string& text) {
    std::istringstream in(text);
    std::vector<std::string> out;
    for (std::string w; in >> w;) out.push_back(w);
    return out;
}

}  // namespace

TEST_CASE("wrap_words: a label that fits is one line, unchanged") {
    const std::string detail = longest_real_detail();
    CHECK(wrap_words(detail, ten_per_char(detail), ten_per_char) ==
          std::vector<std::string>{detail});
}

TEST_CASE("wrap_words: a label too wide breaks at spaces, each line as full as fits") {
    const std::string detail = longest_real_detail();
    CAPTURE(detail);
    REQUIRE(detail.find("  ") == std::string::npos);  // one space between words
    const float max_w = ten_per_char(detail) * 0.5f;
    const std::vector<std::string> lines = wrap_words(detail, max_w, ten_per_char);
    REQUIRE(lines.size() > 1);
    // The lines are the text's own words, the break's space gone.
    std::string joined;
    for (const std::string& l : lines) joined += (joined.empty() ? "" : " ") + l;
    CHECK(joined == detail);
    for (size_t i = 0; i < lines.size(); ++i) {
        CAPTURE(lines[i]);
        // Only a single word runs past the room.
        if (words_of(lines[i]).size() > 1) CHECK(ten_per_char(lines[i]) <= max_w);
        // The next line's first word would not have fitted on this one.
        if (i + 1 < lines.size())
            CHECK(ten_per_char(lines[i] + " " + words_of(lines[i + 1]).front()) > max_w);
    }
}

TEST_CASE("wrap_words: a word wider than the line has a line of its own") {
    const std::string detail = longest_real_detail();
    CAPTURE(detail);
    std::string longest;
    for (const std::string& w : words_of(detail))
        if (ten_per_char(w) > ten_per_char(longest)) longest = w;
    const float max_w = ten_per_char(longest) - 1.0f;
    const std::vector<std::string> lines = wrap_words(detail, max_w, ten_per_char);
    CHECK(std::find(lines.begin(), lines.end(), longest) != lines.end());
}

TEST_CASE("wrap_words: the text's own spacing stays inside a line, spaces at either end go") {
    const std::vector<std::string> spaced = wrap_words("  3- 1 2  (optimal)", 130.0f, ten_per_char);
    REQUIRE(spaced.size() == 2);
    CHECK(spaced[0] == "3- 1 2");
    CHECK(spaced[1] == "(optimal)");
    CHECK(wrap_words("3- 1 2  (optimal)", 170.0f, ten_per_char)[0] == "3- 1 2  (optimal)");
    // Trailing spaces neither end up in a line nor stop the text fitting on one.
    CHECK(wrap_words("a b  ", 30.0f, ten_per_char) == std::vector<std::string>{"a b"});
}

TEST_CASE("wrap_words: empty text is one empty line") {
    CHECK(wrap_words("", 100.0f, ten_per_char) == std::vector<std::string>{""});
}

TEST_CASE("wrap_words: the last words kept together") {
    const std::string header = "Next: activation 12 of 40";  // 25 characters
    // Word by word the count splits; kept together it moves down whole.
    CHECK(wrap_words(header, 210.0f, ten_per_char) ==
          std::vector<std::string>{"Next: activation 12", "of 40"});
    CHECK(wrap_words(header, 210.0f, ten_per_char, 3) ==
          std::vector<std::string>{"Next: activation", "12 of 40"});
    CHECK(wrap_words(header, 90.0f, ten_per_char, 3) ==
          std::vector<std::string>{"Next:", "activation", "12 of 40"});
    CHECK(wrap_words(header, 250.0f, ten_per_char, 3) == std::vector<std::string>{header});
    CHECK(wrap_words("2 of 3", 10.0f, ten_per_char, 3) == std::vector<std::string>{"2 of 3"});
}

TEST_CASE("right_room: from the highway's right edge to the box's, less the gap") {
    const PreviewConfig& cfg = shipped_preview_config();
    const int w = 800, h = 450;
    const float room = right_room(cfg, w, h, 700.0f, 100.0f, 6.0f);
    CHECK(room > 0.0f);
    // The highway widens downward, so a lower row has less room.
    CHECK(right_room(cfg, w, h, 700.0f, 300.0f, 6.0f) < room);
}

TEST_CASE("wrap_right_box: one line a piece when it fits") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PanelText p = panel_text(cfg);
    CAPTURE(real_overlay_text().chart);
    const int w = 3000, h = 450;  // wide: lots of room beside the highway
    const RealBoxes real = real_boxes(p, h);
    const float edge = static_cast<float>(w) - p.gauge_room;
    const float top = real.tops.next;
    for (const hydra::app::PreviewNextActBox& n : real_overlay_text().next) {
        CAPTURE(n.detail);
        const std::vector<TextPiece> pieces = next_pieces(n, p, 1.0f);
        // Each piece whole: the box is its pieces' lines stacked, as wide as
        // the widest piece.
        float text_h = 0.0f, text_w = 0.0f;
        for (const TextPiece& piece : pieces) {
            text_h += piece.line_h;
            text_w = (std::max)(text_w, piece.width_of(piece.text));
        }
        REQUIRE(text_w + p.inset * 2.0f <=
                right_room(cfg, w, h, edge, top + text_h + p.inset * 2.0f, p.gap));
        const WrappedText t = wrap_right_box(cfg, w, h, edge, top, p.inset, p.gap, pieces);
        REQUIRE(t.lines.size() == pieces.size());
        for (size_t i = 0; i < pieces.size(); ++i)
            CHECK(t.lines[i] == std::vector<std::string>{pieces[i].text});
        CHECK(t.text_h == doctest::Approx(text_h));
        CHECK(t.text_w == doctest::Approx(text_w));
    }
}

TEST_CASE("wrap_right_box: off the lane at its lowest row, one scale for the whole path") {
    // The Preview narrowed step by step under the real drain box, the path's
    // real next-activation boxes under it.
    const PreviewConfig& cfg = shipped_preview_config();
    const PanelText p = panel_text(cfg);
    CAPTURE(real_overlay_text().chart);
    const int h = 450;
    const RealBoxes real = real_boxes(p, h);
    bool saw_wrap = false, saw_shrink = false, saw_full = false;
    for (int w = 300; w <= 1600; w += 10) {
        CAPTURE(w);
        // The scale reserves the largest box the path shows, wrapped at scale 1.
        OverlayBoxes boxes = right_column(real, p, w);
        const BoxExtent reserve = next_reserve(cfg, p, real, w, h);
        boxes.next_w = reserve.w;
        boxes.next_h = reserve.h;
        boxes.next_top = real.tops.next;
        const float s = overlay_scale(cfg, w, h, boxes);
        // At the floor the boxes give up on the room and overlap the highway.
        if (s <= kOverlayMinScale) continue;
        // At that scale every box, wrapped to the room at its own lowest row,
        // keeps every line off the lane, and is no larger than reserved.
        const float top = real.tops.drain + (real.drain.h + p.gap) * s;
        for (const hydra::app::PreviewNextActBox& n : real_overlay_text().next) {
            CAPTURE(n.detail);
            const WrappedText t = wrap_right_box(cfg, w, h, boxes.right_edge, top, p.inset * s,
                                                 p.gap, next_pieces(n, p, s));
            const float box_h = t.text_h + p.inset * 2.0f * s;
            CHECK(t.text_w + p.inset * 2.0f * s <=
                  right_room(cfg, w, h, boxes.right_edge, top + box_h, p.gap) + 0.01f);
            CHECK(box_h <= boxes.next_h * s + 0.01f);
            REQUIRE(t.lines.size() == 3);
            // The notation stays one line.
            CHECK(t.lines[1] == std::vector<std::string>{n.notation});
            if (t.lines[2].size() > 1) saw_wrap = true;
        }
        if (s < 1.0f)
            saw_shrink = true;
        else
            saw_full = true;
    }
    // The widths cover a wrapped "at ..." line, the boxes at full size and
    // the boxes shrunk.
    CHECK(saw_wrap);
    CHECK(saw_full);
    CHECK(saw_shrink);
}

TEST_CASE("ellipsize: whole when it fits, cut and ended in an ellipsis when not") {
    const std::string label = "3- 1 2  (optimal)";  // 17 characters, 170 px
    CHECK(ellipsize(label, 170.0f, ten_per_char) == label);
    CHECK(ellipsize(label, 500.0f, ten_per_char) == label);
    // One px short: the end goes, an ellipsis takes its place, and the result fits.
    const std::string cut = ellipsize(label, 169.0f, ten_per_char);
    CHECK(cut == "3- 1 2  (optima\xE2\x80\xA6");
    CHECK(ten_per_char(cut) <= 169.0f);
    // A cut that lands on the two spaces drops them rather than end in "  …".
    CHECK(ellipsize(label, 80.0f, ten_per_char) == "3- 1 2\xE2\x80\xA6");
    // Not even one character and the ellipsis: the ellipsis alone.
    CHECK(ellipsize(label, 15.0f, ten_per_char) == "\xE2\x80\xA6");
    CHECK(ellipsize("", 0.0f, ten_per_char).empty());
}

TEST_CASE("ellipsize: a 40-activation path fits its line, and a cut never splits a character") {
    // The longest real paths: 40 activations and "  (optimal)", about 100 characters.
    std::string label;
    for (int i = 0; i < 40; ++i) label += (i % 3 == 0 ? "2- " : "1 ");
    label += " (optimal)";
    for (float w : {600.0f, 420.0f, 333.0f, 95.0f}) {
        const std::string shown = ellipsize(label, w, ten_per_char);
        CHECK(ten_per_char(shown) <= w);
        CHECK(shown.size() >= 3);
        CHECK(shown.substr(shown.size() - 3) == "\xE2\x80\xA6");
        CHECK(label.rfind(shown.substr(0, shown.size() - 3), 0) == 0);  // a prefix of it
    }
    // A multi-byte character is kept whole or dropped whole.
    const std::string accented = "Caf\xC3\xA9 \xC3\xA9t\xC3\xA9";  // "Café été", 8 characters
    CHECK(ellipsize(accented, 50.0f, ten_per_char) == "Caf\xC3\xA9\xE2\x80\xA6");
    CHECK(ellipsize(accented, 40.0f, ten_per_char) == "Caf\xE2\x80\xA6");
}

TEST_CASE("ellipsize: the kept width is the width of the text before the ellipsis") {
    const std::string label = "3- 1 2  (optimal)";  // 17 characters, 170 px
    float kept_w = -1.0f;
    CHECK(ellipsize(label, 170.0f, ten_per_char, kept_w) == label);
    CHECK(kept_w == 170.0f);  // it fits: the whole width
    CHECK(ellipsize(label, 80.0f, ten_per_char, kept_w) == "3- 1 2\xE2\x80\xA6");
    CHECK(kept_w == 60.0f);  // "3- 1 2", the dropped spaces not counted
    CHECK(ellipsize(label, 15.0f, ten_per_char, kept_w) == "\xE2\x80\xA6");
    CHECK(kept_w == 0.0f);  // only the ellipsis shows
    const std::string accented = "Caf\xC3\xA9 \xC3\xA9t\xC3\xA9";  // "Café été"
    CHECK(ellipsize(accented, 40.0f, ten_per_char, kept_w) == "Caf\xE2\x80\xA6");
    CHECK(kept_w == 30.0f);
}
