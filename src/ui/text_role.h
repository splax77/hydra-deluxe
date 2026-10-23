// Which font and size each kind of text draws in. This header is the one place
// that picks a font: draw and measure calls name the kind of text they show (a
// role) and ask font_for, ScopedFont, ScopedBaselineFont, text_size or
// measure_in_role. Where text of mixed roles sits on a shared line is
// baseline_shift's to say. Why each
// role takes its font is in
// docs/superpowers/plans/2026-10-22-font-role-single-owner.md ("The roles").

#ifndef HYDRA_UI_TEXT_ROLE_H
#define HYDRA_UI_TEXT_ROLE_H

#include <cstddef>

#include "imgui.h"
#include "imgui_internal.h"  // ImGui::GetDefaultFont

#include "ui/column_widths.h"  // WidthOf, measure_in_font
#include "ui/fonts.h"          // g_mono_font

namespace hydra::ui {

// The kind of text, not the font. Pick a role by asking "what is this text?".
enum class TextRole {
    Notation,        // a path string, an activation's notation, a chord name
    Figure,          // a number, timing, measure position, clock or count that lines up
    Label,           // a short word naming a thing: heading, column header, button
    Prose,           // a sentence or wrapped paragraph
    Title,           // a chart's title, artist, charter or folder
    Control,         // the text inside an input field, combo, slider or checkbox
    Overlay,         // the Preview highway's text boxes and SP gauge labels
    SongTitle,       // the song panel's title line
    HeadlineScore,   // the song panel's headline score
    HeadlinePath,    // the song panel's headline path
    DialogQuestion,  // the batch dialog's question
};

// The number of roles. DialogQuestion is the last enumerator; a new role goes
// before it or moves this anchor, and the static_assert below makes either
// change touch font_for and the test table in tests/test_text_role.cpp.
inline constexpr std::size_t kTextRoleCount =
    static_cast<std::size_t>(TextRole::DialogQuestion) + 1;
static_assert(kTextRoleCount == 11,
              "a new TextRole needs its case in font_for and its row in tests/test_text_role.cpp");

// A font and the base size to push it at. Size 0 keeps the current size.
struct FontPick {
    ImFont* font;
    float size;
};

// The one rule: which font and size a role draws in. A missing mono font falls
// back to the current font here and nowhere else, so the font is never null.
// Default-font roles name the default font, so they switch back inside a mono
// scope. Call it inside a frame. The switch has no default branch, so a role
// without a case fails to compile.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(error : 4062)  // an enumerator not handled in a switch
#endif
inline FontPick font_for(TextRole role) {
    ImFont* const mono = g_mono_font ? g_mono_font : ImGui::GetFont();
    ImFont* const deflt = ImGui::GetDefaultFont();
    switch (role) {
        case TextRole::Notation: return {mono, 0.0f};
        case TextRole::Figure: return {mono, 0.0f};
        case TextRole::Label: return {deflt, 0.0f};
        case TextRole::Prose: return {deflt, 0.0f};
        case TextRole::Title: return {deflt, 0.0f};
        case TextRole::Control: return {deflt, 0.0f};
        case TextRole::Overlay: return {mono, 0.0f};
        case TextRole::SongTitle: return {deflt, 26.0f};
        case TextRole::HeadlineScore: return {deflt, 40.0f};
        case TextRole::HeadlinePath: return {mono, 24.0f};
        case TextRole::DialogQuestion: return {deflt, 20.0f};
    }
    IM_ASSERT(false && "font_for: a value outside TextRole");
    return {deflt, 0.0f};
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Pushes the role's font for the scope and pops it on exit.
class ScopedFont {
public:
    explicit ScopedFont(TextRole role) {
        const FontPick pick = font_for(role);
        ImGui::PushFont(pick.font, pick.size);
    }
    ~ScopedFont() { ImGui::PopFont(); }
    ScopedFont(const ScopedFont&) = delete;
    ScopedFont& operator=(const ScopedFont&) = delete;
};

// The one rule for text of different roles sharing a line: every role's
// baseline sits on the anchor role's baseline. The anchor defaults to Label,
// the default font at the size in force, so on a table row the default-font
// cells stay put and the mono cells move. ImGui starts every item on a line at
// the same top edge, so this is how far down (or up, if negative) to start the
// role's text: the anchor font's ascent minus the role font's ascent, each at
// the size its role draws. Call it inside a frame.
inline float baseline_shift(TextRole role, TextRole anchor = TextRole::Label) {
    float anchor_ascent = 0.0f;
    {
        ScopedFont font(anchor);
        anchor_ascent = ImGui::GetFontBaked()->Ascent;
    }
    ScopedFont font(role);
    return anchor_ascent - ImGui::GetFontBaked()->Ascent;
}

// Pushes the role's font like ScopedFont and starts the next item
// baseline_shift(role) lower, so a table cell's text sits on the row's
// default-font baseline. The row keeps the height an unshifted item gives it:
// on exit the cursor and the cell's reach move back by the shift. Use it for
// one single-line item, inside a table cell or on its own line.
class ScopedBaselineFont {
public:
    explicit ScopedBaselineFont(TextRole role)
        : shift_(baseline_shift(role)),
          window_(ImGui::GetCurrentWindow()),
          max_y_(window_->DC.CursorMaxPos.y),
          font_(role) {
        if (shift_ != 0.0f) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + shift_);
    }
    ~ScopedBaselineFont() {
        if (shift_ == 0.0f) return;
        window_->DC.CursorPos.y -= shift_;
        window_->DC.CursorMaxPos.y = ImMax(max_y_, window_->DC.CursorMaxPos.y - shift_);
    }
    float shift() const { return shift_; }
    ScopedBaselineFont(const ScopedBaselineFont&) = delete;
    ScopedBaselineFont& operator=(const ScopedBaselineFont&) = delete;

private:
    float shift_;
    ImGuiWindow* window_;
    float max_y_;
    ScopedFont font_;
};

// The size of `text` drawn in the role's font, wrapped at `wrap_w` if it is
// above 0. Like ImGui::CalcTextSize, it hides anything after "##".
inline ImVec2 text_size(TextRole role, const char* text, float wrap_w = 0.0f) {
    ScopedFont font(role);
    return ImGui::CalcTextSize(text, nullptr, true, wrap_w);
}

// A WidthOf that measures in the role's font at the size it draws, so a column
// is measured in the font its text is drawn in. Keep it only for one measure,
// as measure_in_font says.
inline WidthOf measure_in_role(TextRole role) {
    ScopedFont font(role);
    return measure_in_font(ImGui::GetFont(), ImGui::GetFontSize());
}

// A WidthOf that measures in the role's font at `size` pixels, for text drawn
// in that font at a size of its own (ImDrawList::AddText with font_for(role)'s
// font). Keep it only for one measure, as measure_in_font says.
inline WidthOf measure_in_role(TextRole role, float size) {
    return measure_in_font(font_for(role).font, size);
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_TEXT_ROLE_H
