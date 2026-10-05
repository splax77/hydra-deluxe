// The Paths tab's activation-row rule (ui/activation_row_layout.h): the bars
// start after the widest measure with a gap, never before +200 px, and at the
// narrowest details column the badge on the right still clears them.
//
// Widths here are the app's fonts at scale 1, as the GUI harness measures
// them: Courier Prime (the measure) is 9.6 px a character ("m1024.1.120" is
// 105.6 px); the other text averages under 6 px a character, taken here at
// a generous 7. The GUI test
// paths-row-layout checks the same with the real fonts.

#include "doctest.h"

#include "app/path_view.h"  // longest_activation_badge
#include "ui/activation_row_layout.h"
#include "ui/details_view.h"

using hydra::ui::activation_row_layout;
using hydra::ui::ActivationRowLayout;
using hydra::ui::kRowMeasureX;
using hydra::ui::kRowMinBarsX;

namespace {

constexpr float kMonoChar = 9.6f;
constexpr float kTextChar = 7.0f;

float mono(int chars) { return kMonoChar * static_cast<float>(chars); }
float text(int chars) { return kTextChar * static_cast<float>(chars); }

}  // namespace

TEST_CASE("activation row: a short measure keeps the bars at +200 px") {
    // "m32.1.0" is 7 characters: it ends well before kRowMinBarsX.
    const ActivationRowLayout l = activation_row_layout(mono(7), 0.0f, 600.0f, 1.0f);
    CHECK(l.measure_x == doctest::Approx(kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(kRowMinBarsX));
}

TEST_CASE("activation row: an 11-character measure pushes the bars past it") {
    // "m1024.1.120" ran into "3 bars" when the bars sat at a fixed kRowMinBarsX.
    const float measure_w = mono(11);
    CHECK(kRowMeasureX + measure_w > kRowMinBarsX);  // the old overlap
    const ActivationRowLayout l = activation_row_layout(measure_w, 0.0f, 600.0f, 1.0f);
    CHECK(l.bars_x >= l.measure_x + measure_w + hydra::ui::kRowBarsGap - 0.001f);
    CHECK(l.bars_x > kRowMinBarsX);
}

TEST_CASE("activation row: the layout scales with the DPI") {
    const ActivationRowLayout l = activation_row_layout(mono(7) * 2.0f, 0.0f, 1200.0f, 2.0f);
    CHECK(l.measure_x == doctest::Approx(2.0f * kRowMeasureX));
    CHECK(l.bars_x == doctest::Approx(2.0f * kRowMinBarsX));
    const ActivationRowLayout w = activation_row_layout(mono(11) * 2.0f, 0.0f, 1200.0f, 2.0f);
    CHECK(w.bars_x == doctest::Approx(2.0f * (kRowMeasureX + mono(11) + hydra::ui::kRowBarsGap)));
}

TEST_CASE("activation row: the badge sits flush right, its pill clear of long bars") {
    // The narrowest details column, less a scrollbar.
    const float row_w = hydra::ui::kMinPathDetailsW - 16.0f;
    const float badge_w = text(static_cast<int>(hydra::app::longest_activation_badge().size()));
    const ActivationRowLayout l = activation_row_layout(mono(11), badge_w, row_w, 1.0f);
    CHECK(l.badge_x + badge_w == doctest::Approx(row_w - hydra::ui::kRowBadgeRight));
    CHECK(l.badge_pill_min == doctest::Approx(l.badge_x - hydra::ui::kRowBadgePad));
    const float bars_end = l.bars_x + text(7);  // "12 bars"
    CHECK(bars_end < l.badge_pill_min);
}

TEST_CASE("activation row: every row of a path puts its bars at the same x") {
    // The widest measure in the list decides, whatever this row's own is.
    const ActivationRowLayout a = activation_row_layout(mono(11), 0.0f, 600.0f, 1.0f);
    const ActivationRowLayout b = activation_row_layout(mono(11), text(10), 600.0f, 1.0f);
    CHECK(a.bars_x == doctest::Approx(b.bars_x));
}
