// The badge pill: the rounded box a timing badge sits in on a Paths-tab
// activation row. This header owns its look (fill, border, corner radius,
// padding) and its size: a pill is its text's width plus the padding, or a
// width it is given, so every pill in a row can share the widest one (D257
// call 6). The rectangle arithmetic is pure, so tests/test_badge_pill.cpp
// checks it without a frame; draw_badge_pill is the one ImGui call that
// paints a pill. activation_row_layout.h decides where the pills go.

#ifndef HYDRA_UI_BADGE_PILL_H
#define HYDRA_UI_BADGE_PILL_H

#include "imgui.h"

namespace hydra::ui {

// Unscaled (multiply by px(1)).
inline constexpr float kRowBadgePad = 8.0f;     // the padding each side of the widest text
                                                // (pill_width); also the gap between two pills
inline constexpr float kRowPillReachY = 2.0f;   // a pill reaches this far above and below its text
inline constexpr float kPillRadius = 9.0f;      // the pill's corner radius

// The pill's colours, opaque. squeeze_box (paths_tab.cpp) paints its box in
// the same two.
struct PillRgb {
    unsigned char r, g, b;
};
inline constexpr PillRgb kPillFill{51, 38, 26};
inline constexpr PillRgb kPillBorder{106, 69, 32};

inline ImU32 pill_u32(PillRgb c) { return IM_COL32(c.r, c.g, c.b, 255); }
inline ImVec4 pill_vec4(PillRgb c) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);
}

// A pill's corners, in the same space as the text position it was made from.
struct PillRect {
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;
};

// The width of a pill around text `text_w` wide; `scale` is px(1).
inline float pill_width(float text_w, float scale) {
    return text_w + 2.0f * kRowBadgePad * scale;
}

// The pill around text whose top-left is (text_x, text_y), `pill_w` wide and
// one text line `line` tall. The text sits one pad in from the pill's left
// edge, whatever the width.
inline PillRect pill_rect(float text_x, float text_y, float pill_w, float line, float scale) {
    const float pad = kRowBadgePad * scale;
    const float reach = kRowPillReachY * scale;
    return {text_x - pad, text_y - reach, text_x - pad + pill_w, text_y + line + reach};
}

// Paint pill `r` (screen space) on `dl`: the fill, then the border.
inline void draw_badge_pill(ImDrawList* dl, const PillRect& r, float scale) {
    const ImVec2 lo(r.x0, r.y0), hi(r.x1, r.y1);
    dl->AddRectFilled(lo, hi, pill_u32(kPillFill), kPillRadius * scale);
    dl->AddRect(lo, hi, pill_u32(kPillBorder), kPillRadius * scale);
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_BADGE_PILL_H
