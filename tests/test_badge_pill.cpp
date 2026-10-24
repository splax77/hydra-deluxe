// The badge pill's size and text place (ui/badge_pill.h) as the
// activation-row layout uses them (ui/activation_row_layout.h): every pill on
// every row is sized for the longest badge, side by side and stacked, and its
// text sits centred in it (D262).
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

// "early fill 0 ms", "squeeze in -224 ms" and the longest wording a badge can have.
std::vector<float> mixed_badges() { return {mono(15), mono(18), longest_w()}; }

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
            const ActivationRowLayout shortest = row({mono(15)}, row_w, scale);
            const ActivationRowLayout longest = row({longest_w()}, row_w, scale);
            const ActivationRowLayout mixed = row(mixed_badges(), row_w, scale);
            CHECK(shortest.pill_w == doctest::Approx(longest.pill_w));
            CHECK(mixed.pill_w == doctest::Approx(longest.pill_w));
            // Literals from one run: 182.4 px of text and 8 px each side, doubled at scale 2.
            CHECK(longest.pill_w == doctest::Approx(198.4f * scale));
            for (const PillRect& p : pills(mixed, scale))
                CHECK(p.x1 - p.x0 == doctest::Approx(longest.pill_w));
        }
    }
    // A literal from one run: the 19-character longest badge, 182.4 px, and 8 px each side.
    CHECK(row({mono(15)}, 1400.0f).pill_w == doctest::Approx(198.4f));
}

TEST_CASE("badge pill: a badge wider than the pill text width grows the pill") {
    const float wide = longest_w() + mono(3);
    const ActivationRowLayout l = row({mono(15), wide}, 1400.0f);
    CHECK(l.pill_w == doctest::Approx(227.2f));  // 211.2 px of text and 8 px each side: a literal from one run
}

TEST_CASE("badge pill: text sits centred in its pill") {
    for (float scale : {1.0f, 2.0f}) {
        CAPTURE(scale);
        const ActivationRowLayout l = row(mixed_badges(), 1400.0f, scale);
        const std::vector<PillRect> p = pills(l, scale);
        // The shortest badge's middle is the pill's middle.
        const float short_w = mono(15) * scale;
        const float x = centred_text_x(p[0], short_w);
        CHECK(x + short_w * 0.5f == doctest::Approx((p[0].x0 + p[0].x1) * 0.5f));
        // The longest badge sits one pad in from each side.
        const float long_w = longest_w() * scale;
        const float lx = centred_text_x(p[2], long_w);
        CHECK(lx - p[2].x0 == doctest::Approx(hydra::ui::kRowBadgePad * scale));
        CHECK(p[2].x1 - (lx + long_w) == doctest::Approx(hydra::ui::kRowBadgePad * scale));
    }
    // A literal from one run: "early fill 0 ms" in the last row's first pill, scale 1.
    const std::vector<PillRect> p = pills(row(mixed_badges(), 1400.0f), 1.0f);
    CHECK(centred_text_x(p[0], mono(15)) == doctest::Approx(810.0f));
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

TEST_CASE("badge pill: one badge alone sits flush right as it always has") {
    // Burnout-sized pieces at the narrowest column with the longest badge:
    // literals from one run, scale 1.
    const ActivationRowLayout l = row({longest_w()}, kNarrowRow, 1.0f, mono(7), kBars3);
    REQUIRE(l.badge_lines == 0);
    const PillRect p = pill_rect(l.pill_x[0], l.badge_y[0], l.pill_w, kLine, 1.0f);
    CHECK(p.x0 == doctest::Approx(279.6f));
    CHECK(p.x1 == doctest::Approx(478.0f));
    CHECK(p.y0 == doctest::Approx(7.0f));
    CHECK(p.y1 == doctest::Approx(27.0f));
}
