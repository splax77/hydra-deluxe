// The Paths tab's activation-row rule (ui/activation_row_layout.h): the bars
// start after the widest measure with a gap, never before kRowMinBarsX; the
// badges sit flush right, and when they would reach back into the bars they
// all move to a second line at the same x positions.
//
// Widths here are the app's fonts at scale 1, as the GUI harness measures
// them: Courier Prime (the measure) is 9.6 px a character ("m1024.1.120" is
// 105.6 px); the other text averages under 6 px a character, taken here at
// a generous 7. The GUI test
// paths-row-layout checks the same with the real fonts.

#include "doctest.h"

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

namespace {

constexpr float kMonoChar = 9.6f;
constexpr float kTextChar = 7.0f;

float mono(int chars) { return kMonoChar * static_cast<float>(chars); }
float text(int chars) { return kTextChar * static_cast<float>(chars); }

// The widest wording a badge can have, in the app's words.
float longest_badge_w() {
    return text(static_cast<int>(hydra::app::longest_activation_badge().size()));
}

const float kBars12 = text(7);  // "12 bars"

// The narrowest details column, less a scrollbar.
const float kNarrowRow = hydra::ui::kMinPathDetailsW - 16.0f;

}  // namespace

TEST_CASE("activation row: a short measure keeps the bars at kRowMinBarsX") {
    // "m32.1.0" is 7 characters: it ends well before kRowMinBarsX.
    const ActivationRowLayout l = activation_row_layout(mono(7), kBars12, {}, 600.0f, 1.0f);
    CHECK(l.measure_x == doctest::Approx(kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(kRowMinBarsX));
}

TEST_CASE("activation row: an 11-character measure pushes the bars past it") {
    // "m1024.1.120" ran into "3 bars" when the bars sat at a fixed kRowMinBarsX.
    const float measure_w = mono(11);
    CHECK(kRowMeasureX + measure_w > kRowMinBarsX);  // the old overlap
    const ActivationRowLayout l = activation_row_layout(measure_w, kBars12, {}, 600.0f, 1.0f);
    CHECK(l.bars_x >= l.measure_x + measure_w + hydra::ui::kRowBarsGap - 0.001f);
    CHECK(l.bars_x > kRowMinBarsX);
}

TEST_CASE("activation row: the layout scales with the DPI") {
    const ActivationRowLayout l =
        activation_row_layout(mono(7) * 2.0f, kBars12 * 2.0f, {}, 1200.0f, 2.0f);
    CHECK(l.measure_x == doctest::Approx(2.0f * kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(2.0f * kRowMinBarsX));
    const ActivationRowLayout w =
        activation_row_layout(mono(11) * 2.0f, kBars12 * 2.0f, {}, 1200.0f, 2.0f);
    CHECK(w.bars_x == doctest::Approx(451.2f));  // a literal from one run
}

TEST_CASE("activation row: no badge leaves nothing to place and nothing below") {
    const ActivationRowLayout l = activation_row_layout(mono(11), kBars12, {}, kNarrowRow, 1.0f);
    CHECK(l.badge_x.empty());
    CHECK_FALSE(l.badges_below);
}

TEST_CASE("activation row: one badge sits where it always has (B1)") {
    const float badge_w = longest_badge_w();
    for (float scale : {1.0f, 2.0f}) {
        CAPTURE(scale);
        const float row_w = kNarrowRow * scale;
        const ActivationRowLayout l =
            activation_row_layout(mono(11) * scale, kBars12 * scale, {badge_w * scale}, row_w, scale);
        REQUIRE(l.badge_x.size() == 1);
        CHECK(l.badge_x[0] == doctest::Approx(row_w - badge_w * scale - kRowBadgeRight * scale));
        CHECK(l.badge_pill_min == doctest::Approx(l.badge_x[0] - kRowBadgePad * scale));
        CHECK_FALSE(l.badges_below);
    }
    // Literals from one run at the narrowest column, scale 1.
    const ActivationRowLayout l = activation_row_layout(mono(11), kBars12, {badge_w}, kNarrowRow, 1.0f);
    CHECK(l.badge_x[0] == doctest::Approx(337.0f));
    CHECK(l.badge_pill_min == doctest::Approx(329.0f));
}

TEST_CASE("activation row: the longest badge clears long bars at the narrowest column") {
    const ActivationRowLayout l =
        activation_row_layout(mono(11), kBars12, {longest_badge_w()}, kNarrowRow, 1.0f);
    CHECK(l.bars_x + kBars12 < l.badge_pill_min);
    CHECK_FALSE(l.badges_below);
}

TEST_CASE("activation row: two longest badges drop below on a narrow row only (B2)") {
    const std::vector<float> two(2, longest_badge_w());
    const ActivationRowLayout narrow = activation_row_layout(mono(11), kBars12, two, 470.0f, 1.0f);
    CHECK(narrow.badges_below);
    const ActivationRowLayout wide = activation_row_layout(mono(11), kBars12, two, 900.0f, 1.0f);
    CHECK_FALSE(wide.badges_below);
    CHECK(wide.badge_pill_min > wide.bars_x + kBars12);
}

TEST_CASE("activation row: badges on the row's line never overlap the bars (B3)") {
    const float w = longest_badge_w();
    struct Case {
        size_t count;
        float row_w;
    };
    for (const Case c : {Case{1, kNarrowRow}, Case{1, 900.0f}, Case{2, 900.0f}, Case{3, 900.0f}}) {
        CAPTURE(c.count);
        CAPTURE(c.row_w);
        const ActivationRowLayout l =
            activation_row_layout(mono(11), kBars12, std::vector<float>(c.count, w), c.row_w, 1.0f);
        CHECK_FALSE(l.badges_below);
        CHECK(l.badge_pill_min > l.bars_x + kBars12);
    }
}

TEST_CASE("activation row: a pill that just touches the bars drops below") {
    // Bars at kRowMinBarsX (200), 50 wide, so they end at 250. A 100 px badge's
    // pill starts at row_w - 122 at scale 1: 250 on a 372 px row.
    const ActivationRowLayout touch = activation_row_layout(mono(7), 50.0f, {100.0f}, 372.0f, 1.0f);
    CHECK(touch.badge_pill_min == doctest::Approx(250.0f));
    CHECK(touch.badges_below);
    const ActivationRowLayout clear = activation_row_layout(mono(7), 50.0f, {100.0f}, 373.0f, 1.0f);
    CHECK_FALSE(clear.badges_below);
}

TEST_CASE("activation row: badges run left to right and end flush right (B4)") {
    const std::vector<float> widths{text(15), text(17), longest_badge_w()};
    for (float row_w : {470.0f, 900.0f}) {  // on the row's line and dropped below alike
        CAPTURE(row_w);
        const ActivationRowLayout l = activation_row_layout(mono(11), kBars12, widths, row_w, 1.0f);
        REQUIRE(l.badge_x.size() == widths.size());
        for (size_t k = 1; k < widths.size(); ++k) {
            CHECK(l.badge_x[k] > l.badge_x[k - 1] + widths[k - 1]);
            // One pad between this pill's left edge and the previous pill's right edge.
            const float prev_pill_right = l.badge_x[k - 1] + widths[k - 1] + kRowBadgePad;
            const float pill_left = l.badge_x[k] - kRowBadgePad;
            CHECK(pill_left - prev_pill_right == doctest::Approx(kRowBadgePad));
        }
        CHECK(l.badge_x.back() + widths.back() == doctest::Approx(row_w - kRowBadgeRight));
        CHECK(l.badge_pill_min == doctest::Approx(l.badge_x.front() - kRowBadgePad));
    }
}

TEST_CASE("activation row: every row of a path puts its bars at the same x") {
    // The widest measure in the list decides, whatever this row's own is.
    const ActivationRowLayout a = activation_row_layout(mono(11), kBars12, {}, 600.0f, 1.0f);
    const ActivationRowLayout b = activation_row_layout(mono(11), kBars12, {text(10)}, 600.0f, 1.0f);
    CHECK(a.bars_x == doctest::Approx(b.bars_x));
}
