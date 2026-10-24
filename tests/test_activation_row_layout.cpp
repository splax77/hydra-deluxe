// The Paths tab's activation-row rule (ui/activation_row_layout.h): the bars
// start after the widest measure with a gap, never before kRowMinBarsX; the
// badges sit flush right, and when they would reach back into the bars they
// all move under the measure, wrapping onto more lines when one is too short
// (D257).
//
// Every piece of the row is drawn in the mono font, Courier Prime, which the
// GUI harness measures at 9.6 px a character ("m1024.1.120" is 105.6 px), on
// a 16 px text line. The GUI test paths-row-layout checks the same with the
// real fonts.

#include "doctest.h"

#include <algorithm>
#include <iterator>
#include <vector>

#include "app/path_view.h"  // longest_activation_badge
#include "ui/activation_row_layout.h"
#include "ui/details_view.h"

using hydra::ui::activation_row_layout;
using hydra::ui::ActivationRowLayout;
using hydra::ui::kRowBadgePad;
using hydra::ui::kRowBadgeRight;
using hydra::ui::kRowMeasureX;
using hydra::ui::kRowMinBarsX;
using hydra::ui::kRowPillReachY;

namespace {

constexpr float kMonoChar = 9.6f;
constexpr float kLine = 16.0f;

float mono(int chars) { return kMonoChar * static_cast<float>(chars); }

// The widest wording a badge can have, in the app's words.
float longest_badge_w() {
    return mono(static_cast<int>(hydra::app::longest_activation_badge().size()));
}

const float kBars12 = mono(7);  // "12 bars"
const float kBars3 = mono(6);   // "3 bars"

// The narrowest details column, less a scrollbar.
const float kNarrowRow = hydra::ui::kMinPathDetailsW - 16.0f;

// Every pill sized for the longest badge, as the Paths tab sizes it, unless a
// case gives another pill text width (unscaled).
ActivationRowLayout layout(float measure_w, float bars_w, const std::vector<float>& badges,
                           float row_w, float scale = 1.0f, float pill_text_w = -1.0f) {
    if (pill_text_w < 0.0f) pill_text_w = longest_badge_w();
    return activation_row_layout(measure_w * scale, bars_w * scale, badges, pill_text_w * scale,
                                 row_w * scale, kLine * scale, scale);
}

std::vector<float> scaled(std::vector<float> ws, float scale) {
    for (float& w : ws) w *= scale;
    return ws;
}

// Rows whose badges go under the measure: two and three of the longest badge
// beside the longest pieces, on a narrow row and at the narrowest column.
struct BelowCase {
    size_t count;
    float row_w;
};
const BelowCase kBelowCases[] = {{1, kNarrowRow}, {2, 470.0f}, {2, kNarrowRow}, {3, 470.0f},
                                 {3, kNarrowRow}, {3, 640.0f}};

}  // namespace

TEST_CASE("activation row: a short measure keeps the bars at kRowMinBarsX") {
    // "m32.1.0" is 7 characters: it ends well before kRowMinBarsX.
    const ActivationRowLayout l = layout(mono(7), kBars12, {}, 600.0f);
    CHECK(l.measure_x == doctest::Approx(kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(kRowMinBarsX));
}

TEST_CASE("activation row: an 11-character measure pushes the bars past it") {
    // "m1024.1.120" ran into "3 bars" when the bars sat at a fixed kRowMinBarsX.
    const float measure_w = mono(11);
    CHECK(kRowMeasureX + measure_w > kRowMinBarsX);  // the old overlap
    const ActivationRowLayout l = layout(measure_w, kBars12, {}, 600.0f);
    CHECK(l.bars_x >= l.measure_x + measure_w + hydra::ui::kRowBarsGap - 0.001f);
    CHECK(l.bars_x > kRowMinBarsX);
}

TEST_CASE("activation row: the layout scales with the DPI") {
    const ActivationRowLayout l = layout(mono(7), kBars12, {}, 600.0f, 2.0f);
    CHECK(l.measure_x == doctest::Approx(2.0f * kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(2.0f * kRowMinBarsX));
    const ActivationRowLayout w = layout(mono(11), kBars12, {}, 600.0f, 2.0f);
    CHECK(w.bars_x == doctest::Approx(451.2f));  // a literal from one run
    CHECK(w.row_h == doctest::Approx(68.0f));    // a literal from one run
}

TEST_CASE("activation row: no badge leaves nothing to place and one line") {
    const ActivationRowLayout l = layout(mono(11), kBars12, {}, kNarrowRow);
    CHECK(l.pill_x.empty());
    CHECK(l.badge_lines == 0);
    CHECK(l.row_h == doctest::Approx(34.0f));  // a literal from one run
}

TEST_CASE("activation row: one badge that fits sits where it always has (B1)") {
    // Burnout-sized pieces at the narrowest column: "m32.1.0" and "3 bars".
    const ActivationRowLayout l = layout(mono(7), kBars3, {longest_badge_w()}, kNarrowRow);
    REQUIRE(l.pill_x.size() == 1);
    CHECK(l.badge_lines == 0);
    CHECK(l.badge_line[0] == 0);
    // Literals from one run, scale 1: flush right, on the row's own line, in a
    // row as tall as it was before any badge could drop.
    CHECK(l.pill_x[0] == doctest::Approx(279.6f));
    CHECK(l.badge_pill_min == doctest::Approx(279.6f));
    CHECK(l.badge_y[0] == doctest::Approx(9.0f));
    CHECK(l.text_y == doctest::Approx(9.0f));
    CHECK(l.row_h == doctest::Approx(34.0f));
}

TEST_CASE("activation row: badges beside the bars never overlap them (B3)") {
    const float w = longest_badge_w();
    for (const BelowCase c : {BelowCase{1, 640.0f}, BelowCase{2, 900.0f}, BelowCase{3, 1000.0f}}) {
        CAPTURE(c.count);
        CAPTURE(c.row_w);
        const ActivationRowLayout l =
            layout(mono(11), kBars12, std::vector<float>(c.count, w), c.row_w);
        CHECK(l.badge_lines == 0);
        CHECK(l.badge_pill_min > l.bars_x + kBars12);
    }
}

TEST_CASE("activation row: a pill that just touches the bars drops under the measure") {
    // Bars at kRowMinBarsX (200) on a 640 px row: bars that end exactly where
    // the flush-right pill starts touch it, and one pixel shorter clears it.
    const float pill_left = layout(mono(7), 0.0f, {100.0f}, 640.0f).badge_pill_min;
    const float bars_x = layout(mono(7), 0.0f, {100.0f}, 640.0f).bars_x;
    const ActivationRowLayout touch = layout(mono(7), pill_left - bars_x, {100.0f}, 640.0f);
    CHECK(touch.badge_lines == 1);
    const ActivationRowLayout clear = layout(mono(7), pill_left - bars_x - 1.0f, {100.0f}, 640.0f);
    CHECK(clear.badge_lines == 0);
    CHECK(clear.badge_pill_min == doctest::Approx(435.6f));  // a literal from one run
}

TEST_CASE("activation row: a short badge drops when the longest badge's pill would not fit") {
    // "early fill 0 ms" (15 characters) beside "m1024.1.120" and "12 bars" on a
    // 480 px row: its own-width pill would clear the bars, but every pill is
    // sized for the longest badge, and that one reaches them.
    const float short_w = mono(15);
    const ActivationRowLayout own = layout(mono(11), kBars12, {short_w}, 480.0f, 1.0f, short_w);
    CHECK(own.badge_lines == 0);
    const ActivationRowLayout l = layout(mono(11), kBars12, {short_w}, 480.0f);
    CHECK(l.badge_lines == 1);
    CHECK(l.pill_w == doctest::Approx(198.4f));  // a literal from one run
}

TEST_CASE("activation row: badges under the measure clear the bars by the row's top padding") {
    const float w = longest_badge_w();
    for (float scale : {1.0f, 2.0f}) {
        for (const BelowCase c : kBelowCases) {
            CAPTURE(scale);
            CAPTURE(c.count);
            CAPTURE(c.row_w);
            const ActivationRowLayout l =
                layout(mono(11), kBars12, scaled(std::vector<float>(c.count, w), scale), c.row_w,
                       scale);
            REQUIRE(l.badge_lines >= 1);
            const float line = kLine * scale;
            const float reach = kRowPillReachY * scale;
            const float top_pad = l.text_y;  // the row's top to its first line's text
            const float bars_bottom = l.text_y + line;
            float lowest_pill_bottom = 0.0f;
            for (size_t k = 0; k < c.count; ++k) {
                CAPTURE(k);
                CHECK(l.badge_line[k] >= 1);
                const float pill_top = l.badge_y[k] - reach;
                CHECK(pill_top - bars_bottom >= top_pad - 0.001f);
                lowest_pill_bottom = (std::max)(lowest_pill_bottom, l.badge_y[k] + line + reach);
            }
            // The same margin below the last line, and nothing below the row.
            CHECK(l.row_h - lowest_pill_bottom >= top_pad - 0.001f);
        }
    }
}

TEST_CASE("activation row: no pill runs past the flush-right edge") {
    // A flush-right pill on the row's own line ends kRowBadgeRight - kRowBadgePad
    // in from the row's edge (badge_pill.h sizes the pill); no pill, beside the
    // bars or under the measure, ends further right.
    const float w = longest_badge_w();
    std::vector<BelowCase> cases(std::begin(kBelowCases), std::end(kBelowCases));
    cases.push_back({1, 640.0f});
    cases.push_back({3, 900.0f});
    for (const BelowCase c : cases) {
        CAPTURE(c.count);
        CAPTURE(c.row_w);
        const ActivationRowLayout l =
            layout(mono(11), kBars12, std::vector<float>(c.count, w), c.row_w);
        for (size_t k = 0; k < c.count; ++k) {
            CAPTURE(k);
            const float pill_right = l.pill_x[k] + l.pill_w;
            CHECK(pill_right <= c.row_w - kRowBadgeRight + kRowBadgePad + 0.001f);
        }
    }
}

TEST_CASE("activation row: pills never overlap, on one line or between lines") {
    const std::vector<float> widths{mono(15), mono(17), longest_badge_w()};
    for (float row_w : {380.0f, 470.0f, kNarrowRow, 900.0f}) {
        CAPTURE(row_w);
        const ActivationRowLayout l = layout(mono(11), kBars12, widths, row_w);
        REQUIRE(l.pill_x.size() == widths.size());
        for (size_t a = 0; a < widths.size(); ++a) {
            for (size_t b = a + 1; b < widths.size(); ++b) {
                CAPTURE(a);
                CAPTURE(b);
                if (l.badge_line[a] == l.badge_line[b]) {
                    // Left to right in badge order, one pad between the pills.
                    const float a_right = l.pill_x[a] + l.pill_w;
                    const float b_left = l.pill_x[b];
                    CHECK(b_left >= a_right + kRowBadgePad - 0.001f);
                } else {
                    // A later line's pill starts below an earlier line's.
                    CHECK(l.badge_line[a] < l.badge_line[b]);
                    CHECK(l.badge_y[b] - kRowPillReachY >= l.badge_y[a] + kLine + kRowPillReachY);
                }
            }
        }
    }
}

TEST_CASE("activation row: badges under the measure start at the measure (D257)") {
    const float w = longest_badge_w();
    // Literals from one run at the narrowest column, scale 1: the 19-character
    // longest badge, three of them, take three lines under the measure.
    const ActivationRowLayout l = layout(mono(11), kBars12, std::vector<float>(3, w), kNarrowRow);
    CHECK(l.badge_lines == 3);
    for (size_t k = 0; k < 3; ++k) {
        CAPTURE(k);
        CHECK(l.pill_x[k] == doctest::Approx(96.0f));  // a literal from one run
    }
    CHECK(l.badge_y[0] == doctest::Approx(36.0f));
    CHECK(l.badge_y[1] == doctest::Approx(65.0f));
    CHECK(l.badge_y[2] == doctest::Approx(94.0f));
    CHECK(l.row_h == doctest::Approx(121.0f));
    // On a wider row two fit on the first line under the measure.
    const ActivationRowLayout two = layout(mono(11), kBars12, std::vector<float>(3, w), 640.0f);
    CHECK(two.badge_lines == 2);
    CHECK(two.badge_line[0] == 1);
    CHECK(two.badge_line[1] == 1);
    CHECK(two.badge_line[2] == 2);
}

TEST_CASE("activation row: every row of a path puts its bars at the same x") {
    // The widest measure in the list decides, whatever this row's own is.
    const ActivationRowLayout a = layout(mono(11), kBars12, {}, 600.0f);
    const ActivationRowLayout b = layout(mono(11), kBars12, {mono(10)}, 600.0f);
    CHECK(a.bars_x == doctest::Approx(b.bars_x));
}
