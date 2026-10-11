// Tests for render/overlay_layout: where the highway lands on screen, and the
// scale that keeps the Preview's text boxes beside it. Device-free.

#include "doctest.h"

#include <algorithm>
#include <string>
#include <vector>

#include "render/highway_draw.h"
#include "render/overlay_layout.h"
#include "render/preview_config.h"
#include "preview_config_util.h"

using namespace hydra::render;

TEST_CASE("track_height: the width ratio, capped by the image height") {
    PreviewConfig cfg = shipped_preview_config();  // height_width_ratio 1.1666...
    CHECK(track_height(cfg, 1000, 300) == 300);  // height-limited: the usual case
    CHECK(track_height(cfg, 300, 1000) == 350);  // 300 * 1.1666 = 350
    CHECK(track_height(cfg, 0, 0) == 1);
}

// The values are the derivation audit's table for finding 221.
TEST_CASE("track_rect: the clamped size, the aspect and the top row") {
    const PreviewConfig cfg = shipped_preview_config();

    const TrackRect wide = track_rect(cfg, 1920, 1080);
    CHECK(wide.width == 1920);
    CHECK(wide.height == 1080);
    CHECK(wide.track_height == 1080);
    CHECK(wide.top == 0);
    CHECK(wide.aspect == 1920.0f / 1080.0f);
    CHECK(track_height(cfg, 1920, 1080) == wide.track_height);

    const TrackRect tall = track_rect(cfg, 600, 1080);
    CHECK(tall.track_height == 700);
    CHECK(tall.top == 380);
    CHECK(tall.aspect == 600.0f / 700.0f);
    CHECK(track_height(cfg, 600, 1080) == tall.track_height);

    const TrackRect empty = track_rect(cfg, 0, 0);
    CHECK(empty.width == 1);
    CHECK(empty.height == 1);
    CHECK(empty.track_height == 1);
    CHECK(empty.top == 0);
    CHECK(track_height(cfg, 0, 0) == empty.track_height);
}

TEST_CASE("highway_span_at: passes through the projected railing corners") {
    PreviewConfig cfg = shipped_preview_config();
    const PreviewConfig::Track& T = cfg.track;
    const int w = 1200, h = 400;
    // The railings' outer edges, from the highway's own railing_outer_x.
    const float xl = railing_outer_x(cfg, false);
    const float xr = railing_outer_x(cfg, true);
    CHECK(xl == doctest::Approx(-1.09f));
    CHECK(xr == doctest::Approx(1.09f));
    // The strike line lies between the railing's two ends, so both edges run
    // through its corners at their rows (or outside them, the wider line winning).
    const ImagePoint r = project_to_image(cfg, w, h, {xr, T.railing_y_top, T.z_now});
    const ImagePoint l = project_to_image(cfg, w, h, {xl, T.railing_y_top, T.z_now});
    CHECK(highway_span_at(cfg, w, h, r.y).right >= r.x - 0.01f);
    CHECK(highway_span_at(cfg, w, h, l.y).left <= l.x + 0.01f);
    // A point on a lane lies inside the span at its own row.
    const ImagePoint lane = project_to_image(cfg, w, h, {0.9f, T.y, -6.0f});
    const HighwaySpan s = highway_span_at(cfg, w, h, lane.y);
    CHECK(lane.x < s.right);
    CHECK(lane.x > s.left);
}

TEST_CASE("highway_span_at: symmetric, and wider nearer the camera") {
    PreviewConfig cfg = shipped_preview_config();
    const int w = 1200, h = 400;
    const HighwaySpan top = highway_span_at(cfg, w, h, 60.0f);
    const HighwaySpan bottom = highway_span_at(cfg, w, h, 390.0f);
    CHECK(top.left + top.right == doctest::Approx(static_cast<double>(w)).epsilon(0.001));
    CHECK(bottom.left + bottom.right == doctest::Approx(static_cast<double>(w)).epsilon(0.001));
    CHECK(bottom.right - bottom.left > top.right - top.left);
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

TEST_CASE("overlay_scale: full size with room, floored when narrow, clear in between") {
    PreviewConfig cfg = shipped_preview_config();
    const int h = 390;
    auto boxes = [](int w) {
        OverlayBoxes b;
        b.left_w = 265.0f;
        b.left_h = 150.0f;
        b.right_w = 200.0f;
        b.right_h = 70.0f;
        b.right_top = 10.0f;
        b.right_edge = static_cast<float>(w) - 30.0f;
        b.next_w = 250.0f;
        b.next_h = 90.0f;
        b.next_top = 86.0f;
        b.gap = 6.0f;
        return b;
    };
    CHECK(overlay_scale(cfg, 1200, h, boxes(1200)) == 1.0f);
    CHECK(overlay_scale(cfg, 150, h, boxes(150)) == doctest::Approx(kOverlayMinScale));

    float prev = 0.0f;
    bool saw_partial = false;
    for (int w = 150; w <= 1200; w += 10) {
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
    for (int w = 150; w <= 1200; w += 10) {
        OverlayBoxes b = boxes(w);
        const float with_next = overlay_scale(cfg, w, h, b);
        b.next_w = 0.0f;
        if (overlay_scale(cfg, w, h, b) > with_next) saw_next_limit = true;
    }
    CHECK(saw_next_limit);
    // No boxes: nothing to fit.
    CHECK(overlay_scale(cfg, 300, h, OverlayBoxes{}) == 1.0f);
}

TEST_CASE("overlay_tops: score under time, next under drain or in its place") {
    // A 70 px time box, a 90 px score box, a 70 px drain box, 6 px apart,
    // the right column 10 px down.
    const OverlayHeights s{70.0f, 90.0f, 70.0f, 6.0f, 10.0f};
    const OverlayTops all = overlay_tops(s, /*drain_drawn=*/true, /*score_shown=*/true);
    CHECK(all.time == 0.0f);
    CHECK(all.score == 76.0f);
    CHECK(all.left_bottom == 166.0f);
    CHECK(all.drain == 10.0f);
    CHECK(all.next == 86.0f);
    const OverlayTops bare = overlay_tops(s, false, false);
    CHECK(bare.left_bottom == 70.0f);
    CHECK(bare.next == 10.0f);
}

namespace {
// 10 px per character (not per byte), "…" included: a fixed-pitch stand-in
// for the monospace font the path picker draws with.
float ten_per_char(const std::string& s) {
    float w = 0.0f;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) w += 10.0f;
    return w;
}
}  // namespace

TEST_CASE("wrap_words: one line when it fits, otherwise broken at spaces") {
    const std::string detail = "at m58.1.0 \xC2\xB7 [Kick - YellowCym - GreenCym]";  // 42 characters
    const std::vector<std::string> whole = wrap_words(detail, 420.0f, ten_per_char);
    REQUIRE(whole.size() == 1);
    CHECK(whole[0] == detail);
    // 20 characters a line: each line as full as fits, the break's space gone.
    const std::vector<std::string> lines = wrap_words(detail, 200.0f, ten_per_char);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "at m58.1.0 \xC2\xB7 [Kick -");
    CHECK(lines[1] == "YellowCym -");
    CHECK(lines[2] == "GreenCym]");
    for (const std::string& l : lines) CHECK(ten_per_char(l) <= 200.0f);
    // A word wider than the line gets a line of its own and runs past it.
    const std::vector<std::string> narrow = wrap_words("a YellowCym b", 50.0f, ten_per_char);
    REQUIRE(narrow.size() == 3);
    CHECK(narrow[1] == "YellowCym");
    // The text's own spacing stays inside a line; spaces at either end go.
    const std::vector<std::string> spaced = wrap_words("  3- 1 2  (optimal)", 130.0f, ten_per_char);
    REQUIRE(spaced.size() == 2);
    CHECK(spaced[0] == "3- 1 2");
    CHECK(spaced[1] == "(optimal)");
    CHECK(wrap_words("3- 1 2  (optimal)", 170.0f, ten_per_char)[0] == "3- 1 2  (optimal)");
    CHECK(wrap_words("", 100.0f, ten_per_char) == std::vector<std::string>{""});
    // Trailing spaces neither end up in a line nor stop the text fitting on one.
    CHECK(wrap_words("a b  ", 30.0f, ten_per_char) == std::vector<std::string>{"a b"});
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
    PreviewConfig cfg = shipped_preview_config();
    const int w = 800, h = 450;
    const float room = right_room(cfg, w, h, 700.0f, 100.0f, 6.0f);
    CHECK(room > 0.0f);
    // The box's right edge and the gap move the room one for one.
    CHECK(right_room(cfg, w, h, 750.0f, 100.0f, 6.0f) == doctest::Approx(room + 50.0f));
    CHECK(right_room(cfg, w, h, 700.0f, 100.0f, 0.0f) == doctest::Approx(room + 6.0f));
    // The highway widens downward, so a lower row has less room.
    CHECK(right_room(cfg, w, h, 700.0f, 300.0f, 6.0f) < room);
    // A box whose right edge is on the highway's edge has none.
    const float edge = highway_span_at(cfg, w, h, 100.0f).right;
    CHECK(right_room(cfg, w, h, edge, 100.0f, 0.0f) == doctest::Approx(0.0f));
}

namespace {
// A next-activation box's three pieces at `scale`: the header, the notation
// at 17/12 of the body's size, the "at ..." line. The body text is 15 px
// monospace at about 9 px a character, its line 18.75 px.
std::vector<TextPiece> next_pieces(const std::string& header, const std::string& notation,
                                   const std::string& detail, float scale) {
    auto body = [scale](const std::string& s) { return 0.9f * scale * ten_per_char(s); };
    auto big = [scale](const std::string& s) { return 1.275f * scale * ten_per_char(s); };
    return {{header, body, 18.75f * scale, 3},
            {notation, big, 26.5625f * scale, kKeepWhole},
            {detail, body, 18.75f * scale}};
}

struct NextBox {
    std::string header, notation, detail;
};
// Burnout's three boxes (the second's chord is the long one); the third's
// notation is the User Guide's example, a long one.
const std::vector<NextBox> kBurnout = {
    {"Next: activation 1 of 3", "3- (163 ms)", "at m32.1.0 \xC2\xB7 [Kick - GreenCym]"},
    {"Next: activation 2 of 3", "1", "at m58.1.0 \xC2\xB7 [Kick - YellowCym - GreenCym]"},
    {"Next: activation 3 of 3", "5+ (100 ms)", "at m75.1.0 \xC2\xB7 [Kick - GreenCym]"},
};
}  // namespace

TEST_CASE("wrap_right_box: one line a piece when it fits") {
    PreviewConfig cfg = shipped_preview_config();
    const int w = 1600, h = 450;  // wide: lots of room beside the highway
    // The widest line is each box's "at ..." line: 30, 42 and 30 characters at
    // 9 px (read off a run).
    const float widest[] = {270.0f, 378.0f, 270.0f};
    for (size_t k = 0; k < kBurnout.size(); ++k) {
        const NextBox& b = kBurnout[k];
        const std::vector<TextPiece> pieces = next_pieces(b.header, b.notation, b.detail, 1.0f);
        const WrappedText t = wrap_right_box(cfg, w, h, w - 30.0f, 86.0f, 8.0f, 6.0f, pieces);
        REQUIRE(t.lines.size() == 3);
        for (size_t i = 0; i < 3; ++i)
            CHECK(t.lines[i] == std::vector<std::string>{pieces[i].text});
        CHECK(t.text_h == doctest::Approx(64.0625f));
        CHECK(t.text_w == doctest::Approx(widest[k]));
    }
}

TEST_CASE("wrap_right_box: off the lane at its lowest row, one scale for the whole path") {
    // The Preview at the song panel's narrowest (820 px at a 1,280 px window):
    // the drain box 70 px tall at row 10, the next box under it.
    PreviewConfig cfg = shipped_preview_config();
    const int h = 450;
    const float pad = 8.0f, gap = 6.0f, drain_top = 10.0f, drain_h = 70.0f;
    for (int w : {800, 620}) {
        CAPTURE(w);
        const float edge = static_cast<float>(w) - 30.0f;
        // The scale reserves the largest box the path shows, wrapped at scale 1.
        OverlayBoxes boxes;
        boxes.right_edge = edge;
        boxes.right_w = 150.0f;
        boxes.right_h = drain_h;
        boxes.right_top = drain_top;
        boxes.next_top = drain_top + drain_h + gap;
        boxes.gap = gap;
        for (const NextBox& b : kBurnout) {
            const WrappedText t = wrap_right_box(cfg, w, h, edge, boxes.next_top, pad, gap,
                                                 next_pieces(b.header, b.notation, b.detail, 1.0f));
            boxes.next_w = std::max(boxes.next_w, t.text_w + pad * 2.0f);
            boxes.next_h = std::max(boxes.next_h, t.text_h + pad * 2.0f);
        }
        const float s = overlay_scale(cfg, w, h, boxes);
        REQUIRE(s > kOverlayMinScale);
        // At that scale every box, wrapped to the room at its own lowest row,
        // keeps every line off the lane, and is no larger than reserved.
        const float top = drain_top + (drain_h + gap) * s;
        for (const NextBox& b : kBurnout) {
            const WrappedText t = wrap_right_box(cfg, w, h, edge, top, pad * s, gap,
                                                 next_pieces(b.header, b.notation, b.detail, s));
            const float box_h = t.text_h + pad * 2.0f * s;
            CHECK(t.text_w + pad * 2.0f * s <= right_room(cfg, w, h, edge, top + box_h, gap) + 0.01f);
            CHECK(box_h <= boxes.next_h * s + 0.01f);
            // The notation stays one line.
            CHECK(t.lines[1] == std::vector<std::string>{b.notation});
            // The "at ..." line wraps at both widths.
            CHECK(t.lines[2].size() > 1);
        }
        // Full size at the wider one; the narrower one shrinks the boxes.
        if (w == 800)
            CHECK(s == 1.0f);
        else
            CHECK(s < 1.0f);
    }
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
