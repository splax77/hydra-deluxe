// The badge pill: the rounded box a badge sits in, on a Paths-tab activation
// row and as the tag of a Dynamic Hit Window row. This header owns its look
// (fill, border, corner radius, padding), its size (shared_pill_width, one
// width for every pill, D262) and where the text sits in it (centred).
// draw_badge is the one function that draws a badge: the pill, then its text.
// The rectangle and centring arithmetic is pure, so tests/test_badge_pill.cpp
// checks it without a frame. activation_row_layout.h decides where an
// activation row's pills go.

#ifndef HYDRA_UI_BADGE_PILL_H
#define HYDRA_UI_BADGE_PILL_H

#include <algorithm>

#include "imgui.h"
#include "ui/text_role.h"
#include "ui/theme.h"  // kWarningColor, the short tag's ink

namespace hydra::ui {

// Unscaled (multiply by px(1)).
inline constexpr float kRowBadgePad = 8.0f;     // the padding each side of the widest text
                                                // (pill_width); also the gap between two pills
inline constexpr float kRowPillReachY = 2.0f;   // a pill reaches this far above and below its text
inline constexpr float kPillRadius = 9.0f;      // the pill's corner radius

// The pill's colours, opaque. timing_box (paths_tab.cpp) paints its box in
// the same two, apart from its warning border, which it owns.
struct PillRgb {
    unsigned char r, g, b;
};
inline constexpr PillRgb kPillFill{51, 38, 26};
inline constexpr PillRgb kPillBorder{106, 69, 32};

inline ImU32 pill_u32(PillRgb c) { return IM_COL32(c.r, c.g, c.b, 255); }
inline ImVec4 pill_vec4(PillRgb c) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);
}

// A pill's corners, in the same space as the position it was made from.
struct PillRect {
    float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;
};

// The width of a pill around text `text_w` wide; `scale` is px(1).
inline float pill_width(float text_w, float scale) {
    return text_w + 2.0f * kRowBadgePad * scale;
}

// The one width every badge pill is drawn at (D262): sized for text
// `pill_text_w` wide (the caller measures app::longest_activation_badge),
// grown to `text_w` when a badge's own text is wider, so a pill is never
// narrower than its text; `scale` is px(1).
inline float shared_pill_width(float pill_text_w, float text_w, float scale) {
    // (std::max): windows.h's max macro reaches the GUI tests that include this.
    return pill_width((std::max)(pill_text_w, text_w), scale);
}

// The pill whose left edge is `pill_x`, `pill_w` wide, around one text line
// `line` tall whose top is `text_y`.
inline PillRect pill_rect(float pill_x, float text_y, float pill_w, float line, float scale) {
    const float reach = kRowPillReachY * scale;
    return {pill_x, text_y - reach, pill_x + pill_w, text_y + line + reach};
}

// The left edge of text `text_w` wide centred in pill `r`.
inline float centred_text_x(const PillRect& r, float text_w) {
    return r.x0 + (r.x1 - r.x0 - text_w) * 0.5f;
}

// Draw one badge on `dl`, in screen space: its pill (left edge `pill_x`,
// `pill_w` wide, around the text line whose top is `text_y`), then `text`
// centred in it, in the Figure role and `color`. The caller has submitted the
// item the badge lies over.
inline void draw_badge(ImDrawList* dl, float pill_x, float text_y, float pill_w, const char* text,
                       const ImVec4& color, float scale) {
    ScopedFont font(TextRole::Figure);
    const PillRect r = pill_rect(pill_x, text_y, pill_w, ImGui::GetTextLineHeight(), scale);
    const ImVec2 lo(r.x0, r.y0), hi(r.x1, r.y1);
    dl->AddRectFilled(lo, hi, pill_u32(kPillFill), kPillRadius * scale);
    dl->AddRect(lo, hi, pill_u32(kPillBorder), kPillRadius * scale);
    ImGui::SetCursorScreenPos(ImVec2(centred_text_x(r, ImGui::CalcTextSize(text).x), text_y));
    ImGui::TextColored(color, "%s", text);
}

// The text colour of a Dynamic Hit Window row's tag ("5.4 ms safe", "65.9 ms
// short"), which is a badge drawn by draw_badge: the mockup's yellow when the
// tag is safe, the theme's warning orange (kWarningColor) when it is short.
// Whether a tag is short is app::WindowRowView's. The yellow is the mockup the
// user approved,
// docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html (removed; git show 45dd7799:docs/superpowers/specs/2026-11-12-dynamic-hit-window-mockup/index.html)
// (the .dhw .spare rules).
inline ImVec4 window_tag_ink(bool is_short) {
    return is_short ? kWarningColor : pill_vec4(PillRgb{242, 195, 0});
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_BADGE_PILL_H
