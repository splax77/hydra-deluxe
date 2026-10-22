// Where the pieces of one Paths-tab activation row go: the measure, the bars
// after it and the timing badges flush right. The bars start after the widest
// measure in the path's list (so a long measure like "m1024.1.120" never runs
// into "3 bars"), and never before kRowMinBarsX. Every pill in a row is as
// wide as the row's widest badge (D257 call 6; badge_pill.h owns the pill's
// size). When the badges would reach back into the bars, they all move under
// the measure and wrap onto as many lines as they need (D257). Pure
// arithmetic, so the rule is unit-tested (tests/test_activation_row_layout.cpp).

#ifndef HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
#define HYDRA_UI_ACTIVATION_ROW_LAYOUT_H

#include <algorithm>
#include <cstddef>
#include <vector>

#include "ui/badge_pill.h"

namespace hydra::ui {

// Offsets from the row's left edge, unscaled (multiply by px(1)).
inline constexpr float kRowMeasureX = 104.0f;   // where the measure starts
inline constexpr float kRowMinBarsX = 200.0f;   // the bars never start before this
inline constexpr float kRowBarsGap = 16.0f;     // room between the widest measure and the bars
inline constexpr float kRowBadgeRight = 14.0f;  // a flush-right pill ends kRowBadgePad past this,
                                                // in from the row's right edge
// Heights, unscaled.
inline constexpr float kRowLineH = 34.0f;       // the row's first line

// Pixel positions of one row's pieces, relative to the row's top-left corner.
struct ActivationRowLayout {
    float measure_x = 0.0f;
    float bars_x = 0.0f;
    float text_y = 0.0f;          // the first line's text top
    std::vector<float> badge_x;   // each badge text's left edge, in badge order
    std::vector<float> badge_y;   // each badge text's top
    std::vector<int> badge_line;  // 0: beside the bars; 1 and on: the lines under the measure
    int badge_lines = 0;          // how many lines under the measure the badges take
    float pill_w = 0.0f;          // every pill's width in this row (0: no badge)
    float badge_pill_min = 0.0f;  // the first pill's left edge (badge_x[0] - pad)
    float row_h = 0.0f;           // the whole row's height
};

// `widest_measure_w` is the widest measure text in this path's list, so every
// row's bars line up; `bars_w` this row's bars text width; `badge_ws` this
// row's badge text widths in display order (empty: no badge); `row_w` the
// row's width; `line` the text line height; `scale` is px(1).
inline ActivationRowLayout activation_row_layout(float widest_measure_w, float bars_w,
                                                 const std::vector<float>& badge_ws,
                                                 float row_w, float line, float scale) {
    ActivationRowLayout l;
    l.measure_x = kRowMeasureX * scale;
    // (std::max): windows.h's max macro reaches the GUI tests that include this.
    l.bars_x =
        (std::max)(kRowMinBarsX * scale, l.measure_x + widest_measure_w + kRowBarsGap * scale);
    const float pad = kRowBadgePad * scale;
    const float line_h = kRowLineH * scale;
    const float reach = kRowPillReachY * scale;
    // The first line's text sits in the middle of its band; that top padding is
    // also the gap above, between and below the lines under the measure.
    const float pad_y = (line_h - line) * 0.5f;
    l.text_y = pad_y;
    l.row_h = line_h;
    const size_t n = badge_ws.size();
    float widest = 0.0f;
    for (const float w : badge_ws) widest = (std::max)(widest, w);
    l.pill_w = n == 0 ? 0.0f : pill_width(widest, scale);
    // One pill's step to the next: its width and the gap.
    const float step = l.pill_w + pad;
    l.badge_x.resize(n);
    l.badge_y.assign(n, l.text_y);
    l.badge_line.assign(n, 0);
    // Beside the bars, laid from the right: the last pill ends where a pill
    // around the widest text ending kRowBadgeRight in would end, and each
    // earlier pill one gap before the next.
    const float text_right = row_w - kRowBadgeRight * scale;
    const float last_x = text_right - widest;
    for (size_t k = 0; k < n; ++k) l.badge_x[k] = last_x - static_cast<float>(n - 1 - k) * step;
    // A pill that only touches the bars' end counts as reaching them.
    if (n > 0 && l.badge_x[0] - pad <= l.bars_x + bars_w) {
        // Under the measure, left to right with the same spacing; a pill that
        // would end past the flush-right edge starts the next line.
        float x = l.measure_x;
        bool line_empty = true;
        l.badge_lines = 1;
        for (size_t k = 0; k < n; ++k) {
            if (!line_empty && x + widest > text_right) {
                ++l.badge_lines;
                x = l.measure_x;
            }
            l.badge_x[k] = x;
            l.badge_line[k] = l.badge_lines;
            x += step;
            line_empty = false;
        }
        // Each line's pill starts one pad_y below the previous line's bottom
        // (the first line's text bottom, then the previous pill's bottom).
        const float pill_h = line + reach + reach;
        for (size_t k = 0; k < n; ++k)
            l.badge_y[k] = line_h + static_cast<float>(l.badge_line[k] - 1) * (pill_h + pad_y) + reach;
        l.row_h = line_h + static_cast<float>(l.badge_lines) * (pill_h + pad_y);
    }
    l.badge_pill_min = n == 0 ? text_right : l.badge_x[0] - pad;
    return l;
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
