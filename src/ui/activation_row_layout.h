// Where the pieces of one Paths-tab activation row go: the measure, the bars
// after it and the timing badges flush right. The bars start after the widest
// measure in the path's list (so a long measure like "m1024.1.120" never runs
// into "3 bars"), and never before kRowMinBarsX. Every pill is one width, set
// by the pill text width the caller passes in (D262; badge_pill.h owns the
// pill's size and draws each badge in it). When the badges would reach back
// into the bars, they all move under the measure and wrap onto as many lines
// as they need (D257). A badge with a tag (its Dynamic Hit Window row's) has
// the tag's pill stacked under its own, and its line grows to hold it. Pure
// arithmetic, so the rule is unit-tested (tests/test_activation_row_layout.cpp).

#ifndef HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
#define HYDRA_UI_ACTIVATION_ROW_LAYOUT_H

#include <algorithm>
#include <cstddef>
#include <optional>
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

// The row's top padding, scaled: the first line's text sits in the middle of
// its band, so this is half of what a `line`-tall text line leaves of
// kRowLineH. It is also the gap above, between and below the lines under the
// measure, and between a badge's pill and its tag's. `line` is the scaled
// text line height; `scale` is px(1).
inline float activation_row_top_pad(float line, float scale) {
    return (kRowLineH * scale - line) * 0.5f;
}

// Pixel positions of one row's pieces, relative to the row's top-left corner.
struct ActivationRowLayout {
    float measure_x = 0.0f;
    float bars_x = 0.0f;
    float text_y = 0.0f;          // the first line's text top
    std::vector<float> pill_x;    // each badge pill's left edge, in badge order
    std::vector<float> badge_y;   // each badge text's top (the pill reaches past it, badge_pill.h)
    std::vector<int> badge_line;  // 0: beside the bars; 1 and on: the lines under the measure
    // Each badge's tag text top, its pill as wide as the badge's and under
    // it; unset when the badge has no tag.
    std::vector<std::optional<float>> tag_y;
    int badge_lines = 0;          // how many lines under the measure the badges take
    float pill_w = 0.0f;          // every pill's width in this row (0: no badge)
    float badge_pill_min = 0.0f;  // the first pill's left edge (pill_x[0])
    float row_h = 0.0f;           // the whole row's height
};

// `widest_measure_w` is the widest measure text in this path's list, so every
// row's bars line up; `bars_w` this row's bars text width; `badge_ws` this
// row's badge text widths in display order (empty: no badge); `pill_text_w`
// the text width every pill is sized for (the caller measures
// app::longest_activation_badge), which shared_pill_width grows to the widest
// of `badge_ws` and `tag_ws` (badge_pill.h); `row_w` the row's width;
// `line` the text line height; `scale` is px(1). `tag_ws` runs entry for
// entry with `badge_ws`: each badge's tag text width, unset (or past the
// end) when that badge has no tag.
inline ActivationRowLayout activation_row_layout(float widest_measure_w, float bars_w,
                                                 const std::vector<float>& badge_ws,
                                                 float pill_text_w, float row_w, float line,
                                                 float scale,
                                                 const std::vector<std::optional<float>>& tag_ws = {}) {
    ActivationRowLayout l;
    l.measure_x = kRowMeasureX * scale;
    // (std::max): windows.h's max macro reaches the GUI tests that include this.
    l.bars_x =
        (std::max)(kRowMinBarsX * scale, l.measure_x + widest_measure_w + kRowBarsGap * scale);
    const float pad = kRowBadgePad * scale;
    const float line_h = kRowLineH * scale;
    const float reach = kRowPillReachY * scale;
    const float pad_y = activation_row_top_pad(line, scale);
    l.text_y = pad_y;
    l.row_h = line_h;
    const size_t n = badge_ws.size();
    const auto has_tag = [&tag_ws](size_t k) { return k < tag_ws.size() && tag_ws[k].has_value(); };
    // A tag's pill is as wide as its badge's, so the tags count towards the
    // shared width too.
    float widest = 0.0f;
    for (size_t k = 0; k < n; ++k) {
        widest = (std::max)(widest, badge_ws[k]);
        if (has_tag(k)) widest = (std::max)(widest, *tag_ws[k]);
    }
    l.pill_w = n == 0 ? 0.0f : shared_pill_width(pill_text_w, widest, scale);
    // One pill's step to the next: its width and the gap.
    const float step = l.pill_w + pad;
    l.pill_x.resize(n);
    l.badge_y.assign(n, l.text_y);
    l.badge_line.assign(n, 0);
    l.tag_y.assign(n, std::nullopt);
    const float pill_h = line + reach + reach;
    // A tag's pill starts one pad_y below its badge's, the same gap as
    // between two lines of pills under the measure.
    const float tag_step = pill_h + pad_y;
    // Whether any badge on badge line `ln` has a tag; and how tall that
    // line stands: one pill, or a pill and a tag under it.
    const auto line_has_tag = [&](int ln) {
        for (size_t k = 0; k < n; ++k)
            if (l.badge_line[k] == ln && has_tag(k)) return true;
        return false;
    };
    const auto line_pills_h = [&](int ln) { return line_has_tag(ln) ? pill_h + tag_step : pill_h; };
    // Beside the bars, laid from the right: the last pill ends one pad past
    // kRowBadgeRight in from the row's edge, and each earlier pill one gap
    // before the next.
    const float pill_right = row_w - kRowBadgeRight * scale + pad;
    const float last_x = pill_right - l.pill_w;
    for (size_t k = 0; k < n; ++k) l.pill_x[k] = last_x - static_cast<float>(n - 1 - k) * step;
    // A pill that only touches the bars' end counts as reaching them.
    if (n > 0 && l.pill_x[0] <= l.bars_x + bars_w) {
        // Under the measure, left to right with the same spacing, the first
        // pill one pad before the measure; a pill that would end past the
        // flush-right edge starts the next line.
        const float first_x = l.measure_x - pad;
        float x = first_x;
        bool line_empty = true;
        l.badge_lines = 1;
        for (size_t k = 0; k < n; ++k) {
            if (!line_empty && x + l.pill_w > pill_right) {
                ++l.badge_lines;
                x = first_x;
            }
            l.pill_x[k] = x;
            l.badge_line[k] = l.badge_lines;
            x += step;
            line_empty = false;
        }
        // Each line's pills start one pad_y below the previous line's bottom
        // (the first line's text bottom, then the previous line's lowest
        // pill bottom), and the row ends one pad_y below the last line.
        float bottom = l.text_y + line;
        for (int ln = 1; ln <= l.badge_lines; ++ln) {
            const float pill_top = bottom + pad_y;
            for (size_t k = 0; k < n; ++k)
                if (l.badge_line[k] == ln) l.badge_y[k] = pill_top + reach;
            bottom = pill_top + line_pills_h(ln);
        }
        l.row_h = bottom + pad_y;
    } else if (line_has_tag(0)) {
        // Beside the bars, a tag reaches below the row's first line, and the
        // row ends one pad_y below the tag's pill.
        l.row_h = l.text_y - reach + line_pills_h(0) + pad_y;
    }
    for (size_t k = 0; k < n; ++k)
        if (has_tag(k)) l.tag_y[k] = l.badge_y[k] + tag_step;
    l.badge_pill_min = n == 0 ? pill_right - pad : l.pill_x[0];
    return l;
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_ACTIVATION_ROW_LAYOUT_H
