// The badge pill's size and text place (ui/badge_pill.h) as the
// activation-row layout uses them (ui/activation_row_layout.h): every pill on
// every row is sized for the longest badge, side by side and stacked, and its
// text sits centred in it (D262). Also the one shared pill width
// (shared_pill_width), the Dynamic Hit Window tag's colours (its text, fill
// and outline) and the shared pill colours every other badge keeps.
//
// Widths are in the GUI harness's mono font, 9.6 px a character, on a 16 px
// text line, as in test_activation_row_layout.cpp.

#include "doctest.h"

#include <vector>

#include "app/path_view.h"  // longest_activation_badge
#include "ui/activation_row_layout.h"
#include "ui/badge_pill.h"
#include "ui/details_view.h"

using hydra::ui::activation_row_layout;
using hydra::ui::ActivationRowLayout;
using hydra::ui::centred_text_x;
using hydra::ui::pill_rect;
using hydra::ui::PillRect;

namespace {

constexpr float kMonoChar = 9.6f;
constexpr float kLine = 16.0f;

float mono(int chars) { return kMonoChar * static_cast<float>(chars); }

float longest_w() {
    return mono(static_cast<int>(hydra::app::longest_activation_badge().size()));
}

const float kBars12 = mono(7);  // "12 bars"
const float kBars3 = mono(6);   // "3 bars"
const float kNarrowRow = hydra::ui::kMinPathDetailsW - 16.0f;

// "early fill 0.0 ms", "squeeze in -224.0 ms" and the longest wording a badge can have.
std::vector<float> mixed_badges() { return {mono(17), mono(20), longest_w()}; }

// The row's layout with every pill sized for the longest badge, as the Paths tab does.
ActivationRowLayout row(const std::vector<float>& ws, float row_w, float scale = 1.0f,
                        float measure_w = mono(11), float bars_w = kBars12) {
    std::vector<float> scaled = ws;
    for (float& w : scaled) w *= scale;
    return activation_row_layout(measure_w * scale, bars_w * scale, scaled, longest_w() * scale,
                                 row_w * scale, kLine * scale, scale);
}

std::vector<PillRect> pills(const ActivationRowLayout& l, float scale) {
    std::vector<PillRect> out;
    for (size_t k = 0; k < l.pill_x.size(); ++k)
        out.push_back(pill_rect(l.pill_x[k], l.badge_y[k], l.pill_w, kLine * scale, scale));
    return out;
}

}  // namespace

TEST_CASE("badge pill: a row of the shortest badge and a row of the longest share one width") {
    for (float scale : {1.0f, 2.0f}) {
        for (float row_w : {kNarrowRow, 640.0f, 1400.0f}) {
            CAPTURE(scale);
            CAPTURE(row_w);
            const ActivationRowLayout shortest = row({mono(17)}, row_w, scale);
            const ActivationRowLayout longest = row({longest_w()}, row_w, scale);
            const ActivationRowLayout mixed = row(mixed_badges(), row_w, scale);
            CHECK(shortest.pill_w == doctest::Approx(longest.pill_w));
            CHECK(mixed.pill_w == doctest::Approx(longest.pill_w));
            // Literals from one run: 201.6 px of text and 8 px each side, doubled at scale 2.
            CHECK(longest.pill_w == doctest::Approx(217.6f * scale));
            for (const PillRect& p : pills(mixed, scale))
                CHECK(p.x1 - p.x0 == doctest::Approx(longest.pill_w));
        }
    }
    // A literal from one run: the 21-character longest badge, 201.6 px, and 8 px each side.
    CHECK(row({mono(17)}, 1400.0f).pill_w == doctest::Approx(217.6f));
}

TEST_CASE("badge pill: a badge wider than the pill text width grows the pill") {
    const float wide = longest_w() + mono(3);
    const ActivationRowLayout l = row({mono(17), wide}, 1400.0f);
    CHECK(l.pill_w == doctest::Approx(246.4f));  // 230.4 px of text and 8 px each side: a literal from one run
}

TEST_CASE("badge pill: text sits centred in its pill") {
    for (float scale : {1.0f, 2.0f}) {
        CAPTURE(scale);
        const ActivationRowLayout l = row(mixed_badges(), 1400.0f, scale);
        const std::vector<PillRect> p = pills(l, scale);
        // The shortest badge's middle is the pill's middle.
        const float short_w = mono(17) * scale;
        const float x = centred_text_x(p[0], short_w);
        CHECK(x + short_w * 0.5f == doctest::Approx((p[0].x0 + p[0].x1) * 0.5f));
        // The longest badge sits one pad in from each side.
        const float long_w = longest_w() * scale;
        const float lx = centred_text_x(p[2], long_w);
        CHECK(lx - p[2].x0 == doctest::Approx(hydra::ui::kRowBadgePad * scale));
        CHECK(p[2].x1 - (lx + long_w) == doctest::Approx(hydra::ui::kRowBadgePad * scale));
    }
    // A literal from one run: "early fill 0.0 ms" in the last row's first pill, scale 1.
    const std::vector<PillRect> p = pills(row(mixed_badges(), 1400.0f), 1.0f);
    CHECK(centred_text_x(p[0], mono(17)) == doctest::Approx(752.4f));
}

TEST_CASE("badge pill: pills side by side match and keep one gap between them") {
    const ActivationRowLayout l = row(mixed_badges(), 1400.0f);
    REQUIRE(l.badge_lines == 0);
    const std::vector<PillRect> p = pills(l, 1.0f);
    for (size_t k = 1; k < p.size(); ++k) {
        CAPTURE(k);
        CHECK(p[k].x0 - p[k - 1].x1 == doctest::Approx(hydra::ui::kRowBadgePad));
        CHECK(p[k].y0 == doctest::Approx(p[0].y0));
    }
    // The last pill ends one pad past kRowBadgeRight in from the row's edge: a literal from one run.
    CHECK(p.back().x1 == doctest::Approx(1394.0f));
}

TEST_CASE("badge pill: stacked pills share both edges") {
    // At the narrowest column the three badges take a line each under the measure.
    const ActivationRowLayout l = row(mixed_badges(), kNarrowRow);
    REQUIRE(l.badge_lines == 3);
    const std::vector<PillRect> p = pills(l, 1.0f);
    for (size_t k = 1; k < p.size(); ++k) {
        CAPTURE(k);
        CHECK(p[k].x0 == doctest::Approx(p[0].x0));
        CHECK(p[k].x1 == doctest::Approx(p[0].x1));
        CHECK(p[k].y0 > p[k - 1].y1);
    }
}

namespace {

// `c` is the 8-bit colour (r, g, b), opaque.
void check_rgb(const ImVec4& c, int r, int g, int b) {
    CHECK(c.x * 255.0f == doctest::Approx(r));
    CHECK(c.y * 255.0f == doctest::Approx(g));
    CHECK(c.z * 255.0f == doctest::Approx(b));
    CHECK(c.w == doctest::Approx(1.0f));
}

}  // namespace

TEST_CASE("window tag: safe and short take the mockup's text colours") {
    // The approved mockup's .dhw .spare ink values
    // (docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html (removed; git show 45dd7799:docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html)):
    // #f2c300 for safe, #ff7f00 for short. Fails if window_tag_ink returns another colour.
    check_rgb(hydra::ui::window_tag_ink(false), 0xf2, 0xc3, 0x00);
    check_rgb(hydra::ui::window_tag_ink(true), 0xff, 0x7f, 0x00);
}

TEST_CASE("window tag: its pill is painted in its own fill, outlined in its text colour") {
    // The mockup's .dhw .spare fills: #2a2410 for safe, #2e1a08 for short.
    // Fails if window_tag_fill returns another colour.
    check_rgb(hydra::ui::window_tag_fill(false), 0x2a, 0x24, 0x10);
    check_rgb(hydra::ui::window_tag_fill(true), 0x2e, 0x1a, 0x08);
    // The tag's pill: its fill inside, its text colour as the outline. Fails
    // if window_tag_pill paints the shared pill's fill or border, or outlines
    // the tag in anything but its ink.
    for (bool is_short : {false, true}) {
        CAPTURE(is_short);
        const hydra::ui::PillColors tag = hydra::ui::window_tag_pill(is_short);
        const ImU32 fill =
            is_short ? IM_COL32(0x2e, 0x1a, 0x08, 255) : IM_COL32(0x2a, 0x24, 0x10, 255);
        const ImU32 ink =
            is_short ? IM_COL32(0xff, 0x7f, 0x00, 255) : IM_COL32(0xf2, 0xc3, 0x00, 255);
        CHECK(tag.fill == fill);
        CHECK(tag.border == ink);
        CHECK(tag.fill != hydra::ui::pill_u32(hydra::ui::kPillFill));
        CHECK(tag.border != hydra::ui::pill_u32(hydra::ui::kPillBorder));
    }
}

TEST_CASE("badge pill: every other badge keeps the shared pill colours") {
    // draw_badge's PillColors left at its defaults, as the activation rows
    // call it. Fails if the default fill or border stops being the shared
    // pill's kPillFill and kPillBorder.
    const hydra::ui::PillColors shared;
    CHECK(shared.fill == hydra::ui::pill_u32(hydra::ui::kPillFill));
    CHECK(shared.border == hydra::ui::pill_u32(hydra::ui::kPillBorder));
}

TEST_CASE("badge pill: the shared width grows only for a longer text") {
    // Literals: 201.6 px is the 21-character longest badge in the mono font,
    // and a pill is 8 px wider each side at scale 1, 16 px at scale 2.
    CHECK(hydra::ui::shared_pill_width(201.6f, mono(13), 1.0f) == doctest::Approx(217.6f));
    CHECK(hydra::ui::shared_pill_width(201.6f, 201.6f, 1.0f) == doctest::Approx(217.6f));
    CHECK(hydra::ui::shared_pill_width(201.6f, mono(24), 1.0f) == doctest::Approx(246.4f));
    CHECK(hydra::ui::shared_pill_width(403.2f, mono(13) * 2.0f, 2.0f) == doctest::Approx(435.2f));
}

TEST_CASE("badge pill: one badge alone sits flush right") {
    // Burnout-sized pieces at the narrowest column with the longest badge:
    // literals from one run, scale 1.
    const ActivationRowLayout l = row({longest_w()}, kNarrowRow, 1.0f, mono(7), kBars3);
    REQUIRE(l.badge_lines == 0);
    const PillRect p = pill_rect(l.pill_x[0], l.badge_y[0], l.pill_w, kLine, 1.0f);
    CHECK(p.x0 == doctest::Approx(260.4f));
    CHECK(p.x1 == doctest::Approx(478.0f));
    CHECK(p.y0 == doctest::Approx(7.0f));
    CHECK(p.y1 == doctest::Approx(27.0f));
}
