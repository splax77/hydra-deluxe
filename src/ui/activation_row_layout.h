// Where the pieces of one Paths-tab activation row go: the measure, the bars
// after it and the timing badges flush right. The bars used to sit at a fixed
// +200 px, so a long measure like "m1024.1.120" ran into "3 bars". Now they
// start after the widest measure in the path's list, never earlier than before.
// When the badges would reach back into the bars, they all move to a second
// line inside the row, at the same x positions. Pure arithmetic, so the rule
// is unit-tested (tests/test_activation_row_layout.cpp).

#ifndef HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
#define HYDRA_UI_ACTIVATION_ROW_LAYOUT_H

#include <algorithm>
#include <cstddef>
#include <vector>

namespace hydra::ui {

// Offsets from the row's left edge, unscaled (multiply by px(1)).
inline constexpr float kRowMeasureX = 104.0f;   // where the measure starts
inline constexpr float kRowMinBarsX = 200.0f;   // the bars never start before this
inline constexpr float kRowBarsGap = 16.0f;     // room between the widest measure and the bars
inline constexpr float kRowBadgeRight = 14.0f;  // last badge text's right edge, in from the row's
inline constexpr float kRowBadgePad = 8.0f;     // a pill reaches this far past its text; also
                                                // the gap between two pills

// Pixel positions of one row's pieces, relative to the row's left edge.
struct ActivationRowLayout {
    float measure_x = 0.0f;
    float bars_x = 0.0f;
    std::vector<float> badge_x;   // each badge text's left edge, in badge order
    float badge_pill_min = 0.0f;  // the first pill's left edge (badge_x[0] - pad)
    bool badges_below = false;    // the badges sit on a second line, under the bars
};

// `widest_measure_w` is the widest measure text in this path's list, so every
// row's bars line up; `bars_w` this row's bars text width; `badge_ws` this
// row's badge text widths in display order (empty: no badge); `row_w` the
// row's width; `scale` is px(1).
inline ActivationRowLayout activation_row_layout(float widest_measure_w, float bars_w,
                                                 const std::vector<float>& badge_ws,
                                                 float row_w, float scale) {
    ActivationRowLayout l;
    l.measure_x = kRowMeasureX * scale;
    // (std::max): windows.h's max macro reaches the GUI tests that include this.
    l.bars_x =
        (std::max)(kRowMinBarsX * scale, l.measure_x + widest_measure_w + kRowBarsGap * scale);
    const float pad = kRowBadgePad * scale;
    // Laid from the right: the last badge's text ends kRowBadgeRight in, and each
    // earlier pill ends one pad before the next pill starts.
    l.badge_x.resize(badge_ws.size());
    float text_end = row_w - kRowBadgeRight * scale;
    for (size_t k = badge_ws.size(); k-- > 0;) {
        l.badge_x[k] = text_end - badge_ws[k];
        text_end = l.badge_x[k] - pad - pad - pad;  // this pill's left, the gap, the next pill's right
    }
    l.badge_pill_min = badge_ws.empty() ? row_w - kRowBadgeRight * scale : l.badge_x[0] - pad;
    // A pill that only touches the bars' end counts as reaching them.
    l.badges_below = !badge_ws.empty() && l.badge_pill_min <= l.bars_x + bars_w;
    return l;
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
