// The Paths tab's activation-row rule (ui/activation_row_layout.h): the bars
// start after the widest measure with a gap, never before kRowMinBarsX; the
// badges sit flush right, and when they would reach back into the bars they
// all move under the measure, wrapping onto more lines when one is too short.
//
// Every piece of the row is drawn in the mono font, Courier Prime, which the
// GUI harness measures at 9.6 px a character ("m1024.1.120" is 105.6 px), on
// a 16 px text line. The GUI test paths-row-layout checks the same with the
// real fonts.
//
// The sweeps below walk every row width from the narrowest details column up
// to kWideRow, a pixel apart, so no rule rests on a few hand-picked widths.

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "app/path_view.h"  // longest_activation_badge
#include "ui/activation_row_layout.h"
#include "ui/details_view.h"

using hydra::ui::activation_row_layout;
using hydra::ui::ActivationRowLayout;
using hydra::ui::kRowBadgePad;
using hydra::ui::kRowBadgeRight;
using hydra::ui::kRowLineH;
using hydra::ui::kRowMeasureX;
using hydra::ui::kRowMinBarsX;
using hydra::ui::pill_rect;
using hydra::ui::PillRect;

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

// The widest row the sweeps reach: three of the longest badge sit beside the
// longest pieces there (the beside-the-bars test checks they do).
const float kWideRow = 3.0f * kNarrowRow;

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

// Calls fn(row_w) for every row width from kNarrowRow to kWideRow, a pixel apart.
template <typename Fn>
void for_each_row_w(Fn&& fn) {
    for (float row_w = kNarrowRow; row_w <= kWideRow; row_w += 1.0f) fn(row_w);
}

// Calls fn(count, row_w) for one to three badges at every swept row width.
template <typename Fn>
void for_each_row(Fn&& fn) {
    for (size_t count = 1; count <= 3; ++count)
        for_each_row_w([&](float row_w) { fn(count, row_w); });
}

// The row's top padding: the first line's text sits in the middle of the
// row's first line (kRowLineH), so the padding is half of what the text line
// leaves of it.
float top_pad(float scale) { return (kRowLineH - kLine) * 0.5f * scale; }

// Every position and size in `two` is twice the same one in `one`; the badge
// lines are counts, so they match.
void check_twice(const ActivationRowLayout& one, const ActivationRowLayout& two) {
    CHECK(two.measure_x == doctest::Approx(2.0f * one.measure_x));
    CHECK(two.bars_x == doctest::Approx(2.0f * one.bars_x));
    CHECK(two.text_y == doctest::Approx(2.0f * one.text_y));
    CHECK(two.pill_w == doctest::Approx(2.0f * one.pill_w));
    CHECK(two.badge_pill_min == doctest::Approx(2.0f * one.badge_pill_min));
    CHECK(two.row_h == doctest::Approx(2.0f * one.row_h));
    CHECK(two.badge_lines == one.badge_lines);
    REQUIRE(two.pill_x.size() == one.pill_x.size());
    REQUIRE(two.badge_y.size() == one.badge_y.size());
    REQUIRE(two.badge_line.size() == one.badge_line.size());
    for (size_t k = 0; k < one.pill_x.size(); ++k) {
        CAPTURE(k);
        CHECK(two.pill_x[k] == doctest::Approx(2.0f * one.pill_x[k]));
        CHECK(two.badge_y[k] == doctest::Approx(2.0f * one.badge_y[k]));
        CHECK(two.badge_line[k] == one.badge_line[k]);
    }
}

}  // namespace

TEST_CASE("activation row: a short measure keeps the bars at kRowMinBarsX") {
    // "m32.1.0" is 7 characters: it ends well before kRowMinBarsX.
    const ActivationRowLayout l = layout(mono(7), kBars12, {}, 600.0f);
    CHECK(l.measure_x == doctest::Approx(kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(kRowMinBarsX));
}

TEST_CASE("activation row: an 11-character measure pushes the bars past it") {
    // "m1024.1.120" ends past kRowMinBarsX, so bars at that floor would overlap it.
    const float measure_w = mono(11);
    CHECK(kRowMeasureX + measure_w > kRowMinBarsX);  // the measure ends past the floor
    const ActivationRowLayout l = layout(measure_w, kBars12, {}, 600.0f);
    CHECK(l.bars_x >= l.measure_x + measure_w + hydra::ui::kRowBarsGap - 0.001f);
    CHECK(l.bars_x > kRowMinBarsX);
}

TEST_CASE("activation row: the layout scales with the DPI") {
    // At twice the scale every position and height is twice as big, for short
    // and long measures, with no badge, badges beside the bars and badges
    // under the measure, at every swept row width.
    const float w = longest_badge_w();
    for (const float measure_w : {mono(7), mono(11)}) {
        for (size_t count = 0; count <= 3; ++count) {
            for_each_row_w([&](float row_w) {
                CAPTURE(measure_w);
                CAPTURE(count);
                CAPTURE(row_w);
                const std::vector<float> badges(count, w);
                const ActivationRowLayout one = layout(measure_w, kBars12, badges, row_w);
                const ActivationRowLayout two =
                    layout(measure_w, kBars12, scaled(badges, 2.0f), row_w, 2.0f);
                check_twice(one, two);
            });
        }
    }
}

TEST_CASE("activation row: no badge leaves nothing to place and one line") {
    const ActivationRowLayout l = layout(mono(11), kBars12, {}, kNarrowRow);
    CHECK(l.pill_x.empty());
    CHECK(l.badge_lines == 0);
    CHECK(l.row_h == doctest::Approx(kRowLineH));
}

TEST_CASE("activation row: one badge that fits sits flush right on the row's own line") {
    // Burnout-sized pieces at the narrowest column: "m32.1.0" and "3 bars".
    const ActivationRowLayout l = layout(mono(7), kBars3, {longest_badge_w()}, kNarrowRow);
    REQUIRE(l.pill_x.size() == 1);
    CHECK(l.badge_lines == 0);
    CHECK(l.badge_line[0] == 0);
    // Flush right: the pill ends one pad past kRowBadgeRight in from the edge.
    CHECK(l.pill_x[0] + l.pill_w == doctest::Approx(kNarrowRow - kRowBadgeRight + kRowBadgePad));
    CHECK(l.badge_pill_min == doctest::Approx(l.pill_x[0]));
    // On the row's own text line, in a row one line tall.
    CHECK(l.badge_y[0] == doctest::Approx(l.text_y));
    CHECK(l.row_h == doctest::Approx(kRowLineH));
}

TEST_CASE("activation row: badges beside the bars never overlap them") {
    // At every swept width, badges that stay beside the bars start past the
    // bars' end, and otherwise every badge is under the measure. Each count
    // meets both outcomes somewhere in the sweep.
    const float w = longest_badge_w();
    bool beside[4] = {false, false, false, false};
    bool under[4] = {false, false, false, false};
    for_each_row([&](size_t count, float row_w) {
        CAPTURE(count);
        CAPTURE(row_w);
        const ActivationRowLayout l =
            layout(mono(11), kBars12, std::vector<float>(count, w), row_w);
        if (l.badge_lines == 0) {
            beside[count] = true;
            for (size_t k = 0; k < count; ++k) {
                CAPTURE(k);
                CHECK(l.badge_line[k] == 0);
                CHECK(l.pill_x[k] > l.bars_x + kBars12);
            }
        } else {
            under[count] = true;
            for (size_t k = 0; k < count; ++k) {
                CAPTURE(k);
                CHECK(l.badge_line[k] >= 1);
            }
        }
    });
    for (size_t count = 1; count <= 3; ++count) {
        CAPTURE(count);
        CHECK(beside[count]);
        CHECK(under[count]);
    }
}

TEST_CASE("activation row: a pill that just touches the bars drops under the measure") {
    // One of the longest badge on a 640 px row beside "m32.1.0", whose bars
    // start at kRowMinBarsX. The pill ends one pad past kRowBadgeRight in from
    // the row's edge, so bars this wide end exactly where it starts.
    const float row_w = 640.0f;
    const float badge = longest_badge_w();
    const float pill_w = hydra::ui::shared_pill_width(badge, badge, 1.0f);
    const float pill_left = row_w - kRowBadgeRight + kRowBadgePad - pill_w;
    const float touching_bars_w = pill_left - kRowMinBarsX;
    const ActivationRowLayout touch = layout(mono(7), touching_bars_w, {badge}, row_w);
    REQUIRE(touch.bars_x == doctest::Approx(kRowMinBarsX));
    CHECK(touch.badge_lines == 1);
    // One pixel shorter bars clear the pill, and it stays beside them.
    const ActivationRowLayout clear = layout(mono(7), touching_bars_w - 1.0f, {badge}, row_w);
    CHECK(clear.badge_lines == 0);
}

TEST_CASE("activation row: a short badge drops when the longest badge's pill would not fit") {
    // "early fill 0.0 ms" (17 characters) beside "m1024.1.120" and "12 bars".
    // At every swept width where a pill sized for this badge alone would
    // clear the bars but a pill sized for the longest badge would reach them,
    // the badge drops, because every pill is sized for the longest.
    const float short_w = mono(17);
    const float longest_pill_w = hydra::ui::shared_pill_width(longest_badge_w(), short_w, 1.0f);
    size_t in_range = 0;
    for_each_row_w([&](float row_w) {
        CAPTURE(row_w);
        const ActivationRowLayout own =
            layout(mono(11), kBars12, {short_w}, row_w, 1.0f, short_w);
        const float bars_end = own.bars_x + kBars12;
        const float longest_left = row_w - kRowBadgeRight + kRowBadgePad - longest_pill_w;
        if (own.pill_x[0] <= bars_end || longest_left > bars_end) return;
        ++in_range;
        const ActivationRowLayout l = layout(mono(11), kBars12, {short_w}, row_w);
        CHECK(l.badge_lines == 1);
    });
    CHECK(in_range > 0);
}

TEST_CASE("activation row: badges under the measure clear the bars by the row's top padding") {
    // The first line under the measure starts one top padding below the bars'
    // line, each later line one top padding below the line above, and the
    // row ends one top padding below the last line.
    const float w = longest_badge_w();
    for (const float scale : {1.0f, 2.0f}) {
        size_t dropped = 0;
        for_each_row([&](size_t count, float row_w) {
            CAPTURE(scale);
            CAPTURE(count);
            CAPTURE(row_w);
            const ActivationRowLayout l =
                layout(mono(11), kBars12, scaled(std::vector<float>(count, w), scale), row_w,
                       scale);
            if (l.badge_lines == 0) return;
            ++dropped;
            const float line = kLine * scale;
            const float pad = top_pad(scale);
            CHECK(l.text_y == doctest::Approx(pad));
            float above = l.text_y + line;  // the bars' line bottom, then each pill line's
            float line_top = 0.0f;
            for (size_t k = 0; k < count; ++k) {
                CAPTURE(k);
                CHECK(l.badge_line[k] >= 1);
                const PillRect r = pill_rect(l.pill_x[k], l.badge_y[k], l.pill_w, line, scale);
                const bool new_line = k == 0 || l.badge_line[k] != l.badge_line[k - 1];
                if (new_line) {
                    CHECK(r.y0 - above == doctest::Approx(pad));
                    line_top = r.y0;
                    above = r.y1;
                } else {
                    CHECK(r.y0 == doctest::Approx(line_top));
                }
            }
            CHECK(l.row_h - above == doctest::Approx(pad));
        });
        CAPTURE(scale);
        CHECK(dropped > 0);
    }
}

TEST_CASE("activation row: no pill runs past the flush-right edge") {
    // A flush-right pill on the row's own line ends kRowBadgeRight - kRowBadgePad
    // in from the row's edge (badge_pill.h sizes the pill); no pill, beside the
    // bars or under the measure, ends further right.
    const float w = longest_badge_w();
    for_each_row([&](size_t count, float row_w) {
        CAPTURE(count);
        CAPTURE(row_w);
        const ActivationRowLayout l =
            layout(mono(11), kBars12, std::vector<float>(count, w), row_w);
        const float edge = row_w - kRowBadgeRight + kRowBadgePad;
        for (size_t k = 0; k < count; ++k) {
            CAPTURE(k);
            CHECK(l.pill_x[k] + l.pill_w <= edge + 0.001f);
        }
    });
}

TEST_CASE("activation row: pills never overlap, on one line or between lines") {
    const std::vector<float> widths{mono(15), mono(17), longest_badge_w()};
    for (const float scale : {1.0f, 2.0f}) {
        for_each_row([&](size_t count, float row_w) {
            CAPTURE(scale);
            CAPTURE(count);
            CAPTURE(row_w);
            const std::vector<float> badges(widths.begin(),
                                            widths.begin() + static_cast<std::ptrdiff_t>(count));
            const ActivationRowLayout l =
                layout(mono(11), kBars12, scaled(badges, scale), row_w, scale);
            REQUIRE(l.pill_x.size() == count);
            const float line = kLine * scale;
            for (size_t a = 0; a < count; ++a) {
                for (size_t b = a + 1; b < count; ++b) {
                    CAPTURE(a);
                    CAPTURE(b);
                    const PillRect ra = pill_rect(l.pill_x[a], l.badge_y[a], l.pill_w, line, scale);
                    const PillRect rb = pill_rect(l.pill_x[b], l.badge_y[b], l.pill_w, line, scale);
                    if (l.badge_line[a] == l.badge_line[b]) {
                        // Left to right in badge order, one pad between the pills.
                        CHECK(rb.x0 >= ra.x1 + kRowBadgePad * scale - 0.001f);
                    } else {
                        // A later line's pill starts below an earlier line's.
                        CHECK(l.badge_line[a] < l.badge_line[b]);
                        CHECK(rb.y0 > ra.y1);
                    }
                }
            }
        });
    }
}

TEST_CASE("activation row: badges under the measure start one pad before the measure") {
    const float w = longest_badge_w();
    // At the narrowest column three of the longest badge take one line each.
    const ActivationRowLayout l = layout(mono(11), kBars12, std::vector<float>(3, w), kNarrowRow);
    REQUIRE(l.badge_lines == 3);
    // Each line is one pill tall (pill_rect) with one top padding above it;
    // the first starts at the bottom of the row's first line.
    const PillRect unit = pill_rect(0.0f, 0.0f, l.pill_w, kLine, 1.0f);
    const float step = (unit.y1 - unit.y0) + top_pad(1.0f);
    for (size_t k = 0; k < 3; ++k) {
        CAPTURE(k);
        CHECK(l.badge_line[k] == static_cast<int>(k) + 1);
        CHECK(l.pill_x[k] == doctest::Approx(kRowMeasureX - kRowBadgePad));
        const PillRect r = pill_rect(l.pill_x[k], l.badge_y[k], l.pill_w, kLine, 1.0f);
        CHECK(r.y0 == doctest::Approx(kRowLineH + static_cast<float>(k) * step));
    }
    CHECK(l.row_h == doctest::Approx(kRowLineH + 3.0f * step));
    // On a wider row two fit on the first line under the measure.
    const ActivationRowLayout two = layout(mono(11), kBars12, std::vector<float>(3, w), 640.0f);
    CHECK(two.badge_lines == 2);
    CHECK(two.badge_line[0] == 1);
    CHECK(two.badge_line[1] == 1);
    CHECK(two.badge_line[2] == 2);
}
