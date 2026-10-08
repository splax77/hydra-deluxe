// Color constants, ported from hydra_app.py's build_main_ui theme blocks
// (default_theme, bestpath_theme, warning_theme, newsong_theme,
// songfolder_theme, delete_theme, disabled_text), plus DearPyGui's own
// baseline "Default Theme" colors -- verified against the real values via
// DPG's built-in style editor (dpg.show_style_editor()), not guessed --
// for every slot hydra_app.py leaves at DPG's default rather than
// overriding (WindowBg, FrameBg, Header, Border/Separator, Scrollbar*).
// Dear ImGui's own StyleColorsDark() defaults are meaningfully different
// (bluer, darker) from DPG's, so relying on them for un-overridden slots
// was the source of several color mismatches.

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
// Deliberately lighter than the Python app's (100,100,100): that gray sat at
// ~2.9:1 against the window background, under the 4.5:1 WCAG AA minimum for
// text. (145,145,145) reads as ~4.8:1 while still clearly "dimmed".
inline const ImVec4 kNewSongColor{145 / 255.0f, 145 / 255.0f, 145 / 255.0f, 1.0f};
inline const ImVec4 kDefaultTextColor{250 / 255.0f, 250 / 255.0f, 250 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonColor{180 / 255.0f, 5 / 255.0f, 5 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonHoveredColor{250 / 255.0f, 50 / 255.0f, 50 / 255.0f, 1.0f};
inline const ImVec4 kDeleteButtonActiveColor{250 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};
inline const ImVec4 kAccentColor{0 / 255.0f, 180 / 255.0f, 180 / 255.0f, 1.0f};
// Button faces. White text on today's (0,150,150) was 3.5:1 and on its
// (0,180,180) hover 2.4:1, under WCAG's 4.5:1. These keep (250,250,250) text
// at 4.95:1, 6.33:1 and 7.93:1; hover and press go darker, since a lighter
// teal can't hold white text above 4.5:1.
inline const ImVec4 kButtonColor{0 / 255.0f, 122 / 255.0f, 122 / 255.0f, 1.0f};
inline const ImVec4 kButtonHoveredColor{0 / 255.0f, 104 / 255.0f, 104 / 255.0f, 1.0f};
inline const ImVec4 kButtonActiveColor{0 / 255.0f, 88 / 255.0f, 88 / 255.0f, 1.0f};
// The hover face of an input frame and of a list or table header. It is the
// button hover teal (D74), so it names that one rather than spelling a second.
inline const ImVec4& kFrameHoveredColor = kButtonHoveredColor;

// Surfaces. The window is DPG's baseline (it used to be a local in
// apply_theme); the settings bar and the song panel sit a shade lighter so
// they read as their own areas, as in the approved mockup.
inline const ImVec4 kWindowBgColor{37 / 255.0f, 37 / 255.0f, 38 / 255.0f, 1.0f};
inline const ImVec4 kSettingsBarBg{43 / 255.0f, 43 / 255.0f, 46 / 255.0f, 1.0f};
inline const ImVec4 kPanelBg{40 / 255.0f, 40 / 255.0f, 42 / 255.0f, 1.0f};

// Dimmed text: hints, "(?)" markers, secondary lines, ImGui's TextDisabled,
// and disabled labels. (160,160,160) is 5.86:1 on the window, 5.40:1 on the
// settings bar and 4.81:1 on an input face. kNewSongColor (145) would be
// 4.48:1 on the settings bar, just under the line.
inline const ImVec4 kDimTextColor{160 / 255.0f, 160 / 255.0f, 160 / 255.0f, 1.0f};
// The byline under the song title: brighter than dimmed, quieter than text.
inline const ImVec4 kSubtleTextColor{200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 1.0f};
inline const ImVec4 kFolderListBg{50 / 255.0f, 50 / 255.0f, 50 / 255.0f, 1.0f};

// disabled_text theme + mvButton/mvInputInt(enabled_state=False) components.
// DPG's disabled state is a flat, fully-opaque gray -- not ImGui's default
// DisabledAlpha fade -- so apply_theme() sets style.DisabledAlpha = 1 and
// begin_disabled_button()/begin_disabled_input() push these explicitly.
// Lighter than the Python app's (200,200,200): that pairing against the
// (100,100,100) disabled button was ~3.5:1, under WCAG AA; (235,235,235)
// reaches ~4.9:1 and the gray button face still reads as disabled.
inline const ImVec4 kDisabledButtonTextColor{235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f};
inline const ImVec4 kDisabledButtonColor{100 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};
// Was (50,50,50): 1.15:1 on its (40,40,40) face, unreadable. The dimmed grey
// is 5.64:1 there; the flat dark face still says "off". It is the same grey
// as dimmed text, so it names that one rather than spelling it again.
inline const ImVec4& kDisabledInputTextColor = kDimTextColor;
inline const ImVec4 kDisabledInputBgColor{40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.0f};

// The colour a report window's chip is drawn in, its outline and its words
// both: the path report's Timing chips and the comparison's Status chips
// (report::tier_token and dm_report::status_token pick the token). The one
// home of the chip colours.
ImVec4 chip_color(app::report::ChipToken token);

// Applies the app-wide accent (teal buttons/headers, matching
// build_main_ui's "default_theme") plus DPG's own baseline colors for
// slots hydra_app.py never overrides. Call once after
// ImGui::StyleColorsDark().
void apply_theme();

// Wrap a Button (or ArrowButton/etc) that becomes non-interactive under
// `disabled`, matching mvButton(enabled_state=False)'s flat gray -- use
// instead of a bare ImGui::BeginDisabled()/EndDisabled() pair whenever the
// disabled visual should match hydra_app.py exactly.
void begin_disabled_button(bool disabled);
void end_disabled_button(bool disabled);

// Same, for an InputInt matching mvInputInt(enabled_state=False).
void begin_disabled_input(bool disabled);
void end_disabled_input(bool disabled);

// Same, for a Checkbox: the box takes the disabled input's flat dark face and
// the label goes dim. The label uses kDimTextColor rather than the old
// (50,50,50) disabled_text gray, which on the window background was all but
// unreadable -- a dimmed label still has to be legible enough to say what is
// switched off.
void begin_disabled_checkbox(bool disabled);
void end_disabled_checkbox(bool disabled);

}  // namespace hydra::ui

#endif  // HYDRA_UI_THEME_H
