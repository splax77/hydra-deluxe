// Small UI idioms shared by the views.
//
// The text-overflow helpers exist because Dear ImGui's plain Text() hard-clips
// against the column/child edge with no visual indicator, so a long title just
// vanished mid-word with no way to read the rest. These render an ellipsis
// when the text doesn't fit and put the full text in a hover tooltip — the
// same affordance the HTML report gives truncated cells via title= attributes.
// The rest are the hover-hint / warning-color / progress-overlay patterns that
// used to be hand-rolled at every call site.
//
// Where text sits is apply_theme's rule (ui/theme.cpp): text starts at the
// left of its space. The helpers here that move the cursor right place
// controls, never text.

#ifndef HYDRA_UI_WIDGETS_H
#define HYDRA_UI_WIDGETS_H

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <string>

#include "core/model.h"  // group_thousands
#include "imgui.h"
#include "imgui_internal.h"  // GetCurrentWindow, for the window's draw list
#include "render/overlay_layout.h"  // ellipsize
#include "ui/theme.h"

namespace hydra::ui {

// Delayed tooltip on the last item — the "explain this control on hover"
// idiom. Sites that need other hover flags, format arguments, or extra
// conditions still call IsItemHovered/SetTooltip directly.
// The dimmed details line under an error's sentence, wrapped because it can
// carry a long file path. `detail` comes from app::plain_error_detail, which
// is empty when the sentence already quotes it; then nothing is drawn, so no
// screen shows a blank line or the sentence twice.
inline void error_detail_line(const std::string& detail, const ImVec4& color) {
    if (detail.empty()) return;
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextWrapped("%s", detail.c_str());
    ImGui::PopStyleColor();
}
inline void error_detail_line(const std::string& detail) {
    error_detail_line(detail, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
}

inline void hint(const char* text) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("%s", text);
}

// Draws `text` as a tooltip that wraps at a fixed width, so a long
// explanation stays a readable block instead of one line across the screen.
// Call it only once the item is known to be hovered.
inline void wrapped_tooltip(const char* text) {
    if (ImGui::BeginTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// The "(?)" marker after a setting's label: dimmed, with the explanation on
// hover. Hovers even inside a disabled (locked) group.
inline void help_marker(const char* text) {
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled))
        wrapped_tooltip(text);
}

// Warning-colored text for the current scope — RAII so the Pop can't drift
// away from its Push as lines get added between them.
struct WarnColor {
    WarnColor() { ImGui::PushStyleColor(ImGuiCol_Text, kWarningColor); }
    ~WarnColor() { ImGui::PopStyleColor(); }
    WarnColor(const WarnColor&) = delete;
    WarnColor& operator=(const WarnColor&) = delete;
};

// How full a progress bar is with `done` of `total` finished: held between 0
// and 1, and empty at a total of 0, which means nothing has been reported yet
// (D48, Q19). Every progress bar reads its fill from here.
inline float progress_fraction(double done, double total) {
    if (!(total > 0.0)) return 0.0f;
    return static_cast<float>(std::clamp(done / total, 0.0, 1.0));
}

// Full-width progress bar with a "done / total" overlay, digits grouped the
// way every count reads (group_thousands, D79).
inline void progress_bar_counted(int done, int total) {
    const float frac = progress_fraction(done, total);
    const std::string overlay = group_thousands(done) + " / " + group_thousands(total);
    ImGui::ProgressBar(frac, ImVec2(-1, 0), overlay.c_str());
}

// Full-width progress bar filled to `fraction` (0 to 1), with a whole-percent
// overlay such as "42%". The analyze bar and the Preview loader both draw it.
inline void progress_bar_percent(float fraction) {
    char overlay[16];
    std::snprintf(overlay, sizeof(overlay), "%.0f%%", fraction * 100.0f);
    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
}

// ---- Fixed slots: stop live numbers from moving their neighbours ----------
//
// The UI font is proportional, so "67.6 s" and "71.1 s" are not the same
// width. Any control laid out after a changing number walks left and right
// as the digits change, and a mouse held on it sees its value drift. The
// rule: a piece of text that changes while the user may be interacting
// nearby takes a fixed-width slot sized for the widest value it can show.

// A run of `count` copies of the widest digit in the current font, for
// building "widest this can get" sample strings.
inline std::string widest_digits(int count) {
    char widest = '0';
    float best = 0.0f;
    for (char d = '0'; d <= '9'; ++d) {
        char one[2] = {d, 0};
        float w = ImGui::CalcTextSize(one).x;
        if (w > best) { best = w; widest = d; }
    }
    return std::string(count > 0 ? (size_t)count : 1, widest);
}

// Number of decimal digits in n (1 for 0).
inline int digit_count(long long n) {
    int c = 1;
    for (n = n < 0 ? -n : n; n >= 10; n /= 10) ++c;
    return c;
}

inline float text_slot_width(const char* sample) { return ImGui::CalcTextSize(sample).x; }

// Draws `text` and leaves the cursor on the same line exactly `slot_w` past
// where the text began, so whatever follows never moves. The caller does
// NOT call SameLine() after this.
inline void text_in_slot(const char* text, float slot_w) {
    float x0 = ImGui::GetCursorPosX();
    ImGui::TextUnformatted(text);
    ImGui::SameLine(x0 + slot_w);
}

// A button whose label changes (a count, Play/Pause): give it the width of
// its widest label so the controls after it stay put.
inline float button_slot_width(const char* widest_label) {
    return ImGui::CalcTextSize(widest_label, nullptr, true).x +
           ImGui::GetStyle().FramePadding.x * 2.0f;
}
inline bool button_in_slot(const char* label, float slot_w) {
    return ImGui::Button(label, ImVec2(slot_w, 0.0f));
}

// ---- Wrapping a row: does the next item fit on this line? ----------------

// True when an item `w` wide, placed at screen x `x`, ends by screen x
// `right`. The bare rule, for a caller that places items itself.
inline bool fits_in_row(float x, float w, float right) { return x + w <= right; }

// True when an item `w` wide still fits after the last item, `spacing`
// apart, inside the window's work rect. Inside a table cell or a child that
// right edge is the same number as the cursor plus the available width.
inline bool fits_on_line(float w, float spacing) {
    return fits_in_row(ImGui::GetItemRectMax().x + spacing, w,
                       ImGui::GetCurrentWindow()->WorkRect.Max.x);
}

// Moves the cursor right so the next control, `w` wide, ends at the right
// edge of the space left: the table cell's, or the line's. A control wider
// than that space stays where it is. It places buttons, never text (the
// user's answer 7 in the 2026-10-14 column-shares plan); text follows
// apply_theme in ui/theme.cpp.
inline void place_control_at_right_edge(float w) {
    const float room = ImGui::GetContentRegionAvail().x - w;
    if (room > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + room);
}

// Puts the next buttons, `buttons_w` wide in all, at the right end of the
// current line when they fit beside what is already on it (fits_on_line).
// Otherwise they start a line of their own and sit at its right edge: the
// buttons stay at the right edge either way (the user's answer 7, and their
// choice of 2026-10-14 for the wrapped case).
inline void place_control_at_line_end(float buttons_w) {
    if (fits_on_line(buttons_w, ImGui::GetStyle().ItemSpacing.x)) ImGui::SameLine();
    place_control_at_right_edge(buttons_w);
}

// Keeps the current table's columns in the order they were set up. Call it
// right after the table's TableSetupColumn calls. Dear ImGui 1.93 WIP
// (ocornut/imgui#9519) loads a sort-only hydra_ui.ini entry into a table
// that can't reorder by moving the sorted column to the front: the columns
// with no saved line get position -1 and sort after it. The reset runs in the
// same layout pass, after the load, so ask for it whenever a load is pending
// or the order is already off (not every frame: each reset marks
// hydra_ui.ini as changed).
inline void keep_table_column_order() {
    ImGuiTable* table = ImGui::GetCurrentTable();
    if (!table) return;
    bool reset = table->IsSettingsRequestLoad;
    for (int n = 0; n < table->ColumnsCount && !reset; ++n)
        reset = table->Columns[n].DisplayOrder != n;
    if (reset) table->IsResetDisplayOrderRequest = true;
}

// A popup modal that keeps one width while its text changes. Pair with
// ImGuiWindowFlags_AlwaysAutoResize: height still follows the content,
// width is pinned to `width`.
inline void pin_next_modal_width(float width) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0.0f), ImVec2(width, FLT_MAX));
}

// Shows `text` in a (wrapped) tooltip when the last item is hovered. Callers
// use this directly for items that render their own text (Selectable rows);
// pair with a "does it actually overflow" check so short text stays quiet.
inline void overflow_tooltip(const char* text) {
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                              ImGuiHoveredFlags_AllowWhenDisabled))
        return;
    wrapped_tooltip(text);
}

// How wide `s` draws in the current font. The measurer every
// render::ellipsize call in the UI hands over.
inline float text_width(const std::string& s) { return ImGui::CalcTextSize(s.c_str()).x; }

// Draws `text` where it was cut to `shown` (render::ellipsize's answer for
// `avail` pixels): whole when nothing was cut, otherwise the cut text with the
// full text in a tooltip. The one place a cut label is drawn; text_ellipsized
// and the report window's cached cells (ui/report_window.cpp) both end here.
// The caller has already checked the window is not skipping items.
inline void draw_cut_text(const char* text, const std::string& shown, float avail) {
    if (shown == text) {
        ImGui::TextUnformatted(text);
        return;
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DrawList->AddText(window->DC.CursorPos, ImGui::GetColorU32(ImGuiCol_Text),
                              shown.c_str());
    // An item exactly as wide as the space the text was given, so layout
    // advances normally and the tooltip has a hover rect.
    ImGui::Dummy(ImVec2(avail, ImGui::CalcTextSize(text).y));
    overflow_tooltip(text);
}

// TextUnformatted that ellipsizes at the available width instead of clipping
// mid-glyph, with the full text in a tooltip when it didn't fit. The cut is
// render::ellipsize's, measured in the current font (text_width), so every cut label in
// the UI follows the one rule. Uses the current font and text color, so
// callers can Push either around it. `max_width` caps the space the text may
// take, for text that shares its line with right-aligned buttons (the song
// panel's title).
inline void text_ellipsized(const char* text, float max_width = FLT_MAX) {
    if (ImGui::GetCurrentWindow()->SkipItems) return;

    const float avail = (std::min)(ImGui::GetContentRegionAvail().x, max_width);
    draw_cut_text(text, render::ellipsize(text, avail, text_width), avail);
}

// A table row's Selectable spanning all columns, with the full text offered
// on hover when it overflows column 0 (only while actually over that column
// — the Selectable's hover rect spans the whole row). The width must be
// captured before the Selectable claims it. Returns the clicked bool.
inline bool row_selectable(const char* text, bool selected) {
    float avail = ImGui::GetContentRegionAvail().x;
    bool clicked =
        ImGui::Selectable(text, selected, ImGuiSelectableFlags_SpanAllColumns);
    if (ImGui::TableGetHoveredColumn() == 0 &&
        ImGui::CalcTextSize(text).x > avail)
        overflow_tooltip(text);
    return clicked;
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_WIDGETS_H
