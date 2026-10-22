// The badge pill's size (ui/badge_pill.h) as the activation-row layout uses
// it (ui/activation_row_layout.h): every pill in one row is as wide as the
// row's widest badge plus the padding, side by side and stacked, and one
// badge alone gets the pill it always had (D257 call 6).
//
// Widths are in the GUI harness's mono font, 9.6 px a character, on a 16 px
// text line, as in test_activation_row_layout.cpp.

#include "doctest.h"

#include <algorithm>
#include <vector>

#include "app/path_view.h"  // longest_activation_badge
#include "ui/activation_row_layout.h"
#include "ui/badge_pill.h"
#include "ui/details_view.h"

using hydra::ui::activation_row_layout;
using hydra::ui::ActivationRowLayout;
using hydra::ui::pill_rect;
using hydra::ui::pill_width;
using hydra::ui::PillRect;

namespace {

constexpr float kMonoChar = 9.6f;
constexpr float kLine = 16.0f;

float mono(int chars) { return kMonoChar * static_cast<float>(chars); }

const float kBars12 = mono(7);  // "12 bars"
const float kBars3 = mono(6);   // "3 bars"
const float kNarrowRow = hydra::ui::kMinPathDetailsW - 16.0f;

// "early fill 0 ms", "squeeze in -224 ms" and the longest wording a badge can have.
std::vector<float> mixed_badges() {
    return {mono(15), mono(18),
            mono(static_cast<int>(hydra::app::longest_activation_badge().size()))};
}

std::vector<PillRect> pills(const ActivationRowLayout& l, float scale) {
    std::vector<PillRect> out;
    for (size_t k = 0; k < l.badge_x.size(); ++k)
        out.push_back(pill_rect(l.badge_x[k], l.badge_y[k], l.pill_w, kLine * scale, scale));
    return out;
}

}  // namespace

TEST_CASE("badge pill: every pill in a row is the widest text plus the padding") {
    const std::vector<float> ws = mixed_badges();
    const float widest = *std::max_element(ws.begin(), ws.end());
    for (float scale : {1.0f, 2.0f}) {
        for (float row_w : {kNarrowRow, 640.0f, 1400.0f}) {
            CAPTURE(scale);
            CAPTURE(row_w);
            std::vector<float> scaled = ws;
            for (float& w : scaled) w *= scale;
            const ActivationRowLayout l = activation_row_layout(
                mono(11) * scale, kBars12 * scale, scaled, row_w * scale, kLine * scale, scale);
            CHECK(l.pill_w == doctest::Approx(pill_width(widest * scale, scale)));
            for (const PillRect& p : pills(l, scale)) CHECK(p.x1 - p.x0 == doctest::Approx(l.pill_w));
        }
    }
    // A literal from one run: the 19-character longest badge, 182.4 px, and 8 px each side.
    const ActivationRowLayout l = activation_row_layout(mono(11), kBars12, ws, 1400.0f, kLine, 1.0f);
    CHECK(l.badge_lines == 0);
    CHECK(l.pill_w == doctest::Approx(198.4f));
}

TEST_CASE("badge pill: pills side by side match and keep one gap between them") {
    const ActivationRowLayout l =
        activation_row_layout(mono(11), kBars12, mixed_badges(), 1400.0f, kLine, 1.0f);
    REQUIRE(l.badge_lines == 0);
    const std::vector<PillRect> p = pills(l, 1.0f);
    for (size_t k = 1; k < p.size(); ++k) {
        CAPTURE(k);
        CHECK(p[k].x0 - p[k - 1].x1 == doctest::Approx(hydra::ui::kRowBadgePad));
        CHECK(p[k].y0 == doctest::Approx(p[0].y0));
    }
    // The last pill ends where a lone pill around the widest text ends: a literal from one run.
    CHECK(p.back().x1 == doctest::Approx(1394.0f));
}

TEST_CASE("badge pill: stacked pills share both edges") {
    // At the narrowest column the three badges take a line each under the measure.
    const ActivationRowLayout l =
        activation_row_layout(mono(11), kBars12, mixed_badges(), kNarrowRow, kLine, 1.0f);
    REQUIRE(l.badge_lines == 3);
    const std::vector<PillRect> p = pills(l, 1.0f);
    for (size_t k = 1; k < p.size(); ++k) {
        CAPTURE(k);
        CHECK(p[k].x0 == doctest::Approx(p[0].x0));
        CHECK(p[k].x1 == doctest::Approx(p[0].x1));
        CHECK(p[k].y0 > p[k - 1].y1);
    }
    // Short text keeps the usual inset from the pill's left edge.
    CHECK(l.badge_x[0] - p[0].x0 == doctest::Approx(hydra::ui::kRowBadgePad));
}

TEST_CASE("badge pill: one badge alone is drawn as it always was") {
    // Burnout-sized pieces at the narrowest column with the longest badge:
    // literals from one run, scale 1.
    const float w = mono(static_cast<int>(hydra::app::longest_activation_badge().size()));
    const ActivationRowLayout l = activation_row_layout(mono(7), kBars3, {w}, kNarrowRow, kLine, 1.0f);
    REQUIRE(l.badge_lines == 0);
    const PillRect p = pill_rect(l.badge_x[0], l.badge_y[0], l.pill_w, kLine, 1.0f);
    CHECK(p.x0 == doctest::Approx(279.6f));
    CHECK(p.x1 == doctest::Approx(478.0f));
    CHECK(p.y0 == doctest::Approx(7.0f));
    CHECK(p.y1 == doctest::Approx(27.0f));
}
