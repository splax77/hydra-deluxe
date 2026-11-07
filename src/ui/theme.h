// Color constants. The slots the app doesn't colour for itself (WindowBg,
// FrameBg, Border/Separator, Scrollbar*) keep the default colors of DearPyGui,
// the toolkit the Python Hydra drew with, so they keep that look. Dear ImGui's
// own StyleColorsDark() defaults are meaningfully different (bluer, darker),
// so apply_theme() sets those slots rather than relying on them.

#ifndef HYDRA_UI_THEME_H
#define HYDRA_UI_THEME_H

#include "imgui.h"

namespace hydra::app::report {
enum class ChipToken;  // app/report.h
}

namespace hydra::ui {

inline const ImVec4 kBestPathColor{250 / 255.0f, 210 / 255.0f, 0 / 255.0f, 1.0f};
// The Star Power gold. A different gold from kBestPathColor.
inline const ImVec4 kStarPowerColor{255 / 255.0f, 204 / 255.0f, 51 / 255.0f, 1.0f};
// The Preview's SP gauge fills with that gold a touch see-through: alpha 230
// of 255, the look as shipped.
inline constexpr float kStarPowerFillAlpha = 230.0f / 255.0f;
inline const ImVec4 kWarningColor{255 / 255.0f, 127 / 255.0f, 0 / 255.0f, 1.0f};
// Light enough for the 4.5:1 WCAG AA minimum for text: (145,145,145) reads as
// ~4.9:1 against the window background while still clearly "dimmed".
inline const ImVec4 kNewSongColor{145 / 255.0f, 145 / 255.0f, 145 / 255.0f, 1.0f};
inline const ImVec4 kDefaultTextColor{250 / 255.0f, 250 / 255.0f, 250 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonColor{180 / 255.0f, 5 / 255.0f, 5 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonHoveredColor{250 / 255.0f, 50 / 255.0f, 50 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonActiveColor{250 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};
inline const ImVec4 kAccentColor{0 / 255.0f, 180 / 255.0f, 180 / 255.0f, 1.0f};
// Button faces. These keep (250,250,250) text at 4.95:1, 6.33:1 and 7.93:1,
// over WCAG's 4.5:1; hover and press go darker, since a lighter teal can't
// hold white text above 4.5:1.
inline const ImVec4 kButtonColor{0 / 255.0f, 122 / 255.0f, 122 / 255.0f, 1.0f};
inline const ImVec4 kButtonHoveredColor{0 / 255.0f, 104 / 255.0f, 104 / 255.0f, 1.0f};
inline const ImVec4 kButtonActiveColor{0 / 255.0f, 88 / 255.0f, 88 / 255.0f, 1.0f};
// A selected row in any list or table (ImGuiCol_Header). A dark teal, so it
// stands out from the grey row stripes while every text colour a row uses
// still reads at 4.5:1 on it (test_theme.cpp checks them); the user chose this
// teal.
inline const ImVec4 kSelectedRowColor{0 / 255.0f, 48 / 255.0f, 48 / 255.0f, 1.0f};
// The hover face of an input frame and of a list or table header. It is the
// button hover teal (D74), so it names that one rather than spelling a second.
inline const ImVec4& kFrameHoveredColor = kButtonHoveredColor;

// Surfaces. The window is the baseline gray from the top of this file; the
// song panel sits a shade lighter so it reads as its own area, as in the
// approved mockup.
inline const ImVec4 kWindowBgColor{37 / 255.0f, 37 / 255.0f, 38 / 255.0f, 1.0f};
inline const ImVec4 kPanelBg{40 / 255.0f, 40 / 255.0f, 42 / 255.0f, 1.0f};

// Dimmed text: hints, "(?)" markers, secondary lines, ImGui's TextDisabled,
// and disabled labels. (160,160,160) is 5.86:1 on the window and 4.81:1 on
// an input face.
inline const ImVec4 kDimTextColor{160 / 255.0f, 160 / 255.0f, 160 / 255.0f, 1.0f};
// The byline under the song title: brighter than dimmed, quieter than text.
inline const ImVec4 kSubtleTextColor{200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 1.0f};
inline const ImVec4 kFolderListBg{50 / 255.0f, 50 / 255.0f, 50 / 255.0f, 1.0f};

// Disabled controls. A disabled control is a flat, fully-opaque gray -- not
// ImGui's default DisabledAlpha fade -- so apply_theme() sets
// style.DisabledAlpha = 1 and begin_disabled_button()/begin_disabled_input()
// push these explicitly.
// (235,235,235) on the (100,100,100) disabled button reaches ~5.0:1, over
// WCAG AA's 4.5:1, and the gray button face still reads as disabled.
inline const ImVec4 kDisabledButtonTextColor{235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f};
inline const ImVec4 kDisabledButtonColor{100 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};
// The dimmed grey is 5.64:1 on the (40,40,40) input face; the flat dark face
// still says "off". It is the same grey as dimmed text, so it names that one
// rather than spelling it again.
inline const ImVec4& kDisabledInputTextColor = kDimTextColor;
inline const ImVec4 kDisabledInputBgColor{40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.0f};

// The strips' backgrounds: the batch's finished strip and the report
// windows' out-of-date strip (done), and the finished strip after a failed
// report (problem). (250,250,250) text is 12.94:1 on the done strip and
// 13.73:1 on the problem strip; orange (kWarningColor) on the problem strip is
// 5.65:1.
inline const ImVec4 kDoneStripBg{31 / 255.0f, 51 / 255.0f, 34 / 255.0f, 1.0f};
inline const ImVec4 kProblemStripBg{58 / 255.0f, 38 / 255.0f, 18 / 255.0f, 1.0f};

// The version bar across the top of the window (src/ui/version_bar.cpp): its
// background, a shade darker than the window, and the dot before its status.
// Green says this Hydra is up to date, amber that a newer release is known,
// grey every other state. tests/test_theme.cpp holds the contrast floors.
inline const ImVec4 kVersionBarBg{30 / 255.0f, 30 / 255.0f, 31 / 255.0f, 1.0f};
inline const ImVec4 kUpToDateDotColor{80 / 255.0f, 190 / 255.0f, 120 / 255.0f, 1.0f};
inline const ImVec4 kUpdateAvailableDotColor{235 / 255.0f, 180 / 255.0f, 60 / 255.0f, 1.0f};
inline const ImVec4 kNeutralDotColor{128 / 255.0f, 128 / 255.0f, 128 / 255.0f, 1.0f};

// The colour a report window's chip is drawn in, its outline and its words
// both: the path report's Timing chips and the comparison's Status chips
// (report::tier_token and dm_report::status_token pick the token). The one
// home of which colour each token takes; the tier colours themselves are
// app/html_page.h's kChipRgb*, shared with the fill page.
ImVec4 chip_color(app::report::ChipToken token);

// Applies the app-wide accent (teal buttons/headers) plus the baseline colors
// from the top of this file. Call once after ImGui::StyleColorsDark().
void apply_theme();

// Wrap a Button (or ArrowButton/etc) that becomes non-interactive under
// `disabled`, giving it the flat gray disabled look -- use instead of a bare
// ImGui::BeginDisabled()/EndDisabled() pair, which looks enabled (the
// invariant in apply_theme).
void begin_disabled_button(bool disabled);
void end_disabled_button(bool disabled);

// Same, for an InputInt.
void begin_disabled_input(bool disabled);
void end_disabled_input(bool disabled);

// Same, for a Checkbox: the box takes the disabled input's flat dark face and
// the label goes dim. The label uses kDimTextColor because a dimmed label
// still has to be legible enough to say what is switched off.
void begin_disabled_checkbox(bool disabled);
void end_disabled_checkbox(bool disabled);

}  // namespace hydra::ui

#endif  // HYDRA_UI_THEME_H
