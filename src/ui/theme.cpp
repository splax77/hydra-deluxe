#include "ui/theme.h"

#include <cstdint>
#include <stdexcept>

#include "app/html_page.h"  // kChipRgb*
#include "app/report.h"  // ChipToken

namespace hydra::ui {

namespace {

ImVec4 opaque_rgb(uint32_t rgb) {
    return ImVec4(((rgb >> 16) & 0xff) / 255.0f, ((rgb >> 8) & 0xff) / 255.0f,
                  (rgb & 0xff) / 255.0f, 1.0f);
}

}  // namespace

ImVec4 chip_color(app::report::ChipToken token) {
    using app::report::ChipToken;
    // The tiers are html_page.h's kChipRgb* colours, which the fill page's
    // dark scheme also reads for its --t0, --t3 and --tn (D103 kept them for
    // the windows). The dim chip is the theme's dimmed text.
    switch (token) {
        case ChipToken::t0: return opaque_rgb(app::html::kChipRgbT0);
        case ChipToken::t1: return opaque_rgb(app::html::kChipRgbT1);
        case ChipToken::t2: return opaque_rgb(app::html::kChipRgbT2);
        case ChipToken::t3: return opaque_rgb(app::html::kChipRgbT3);
        case ChipToken::t4: return opaque_rgb(app::html::kChipRgbT4);
        case ChipToken::t5: return opaque_rgb(app::html::kChipRgbT5);
        case ChipToken::tn: return opaque_rgb(app::html::kChipRgbTn);
        case ChipToken::muted: return kDimTextColor;
    }
    throw std::invalid_argument("chip_color: unknown chip token");
}

void apply_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // ---- DPG's own "Default Theme" baseline, for slots hydra_app.py never
    // overrides. Values read directly from dpg.show_style_editor(), not
    // Dear ImGui's StyleColorsDark() (which is meaningfully bluer/darker).
    ImVec4 dpg_surface(51 / 255.0f, 51 / 255.0f, 55 / 255.0f, 1.0f);
    const ImVec4 dpg_window = kWindowBgColor;
    ImVec4 dpg_border(78 / 255.0f, 78 / 255.0f, 78 / 255.0f, 1.0f);
    colors[ImGuiCol_WindowBg] = dpg_window;
    colors[ImGuiCol_ChildBg] = dpg_window;
    colors[ImGuiCol_PopupBg] = dpg_window;
    colors[ImGuiCol_FrameBg] = dpg_surface;
    colors[ImGuiCol_Border] = dpg_border;
    colors[ImGuiCol_Separator] = dpg_border;
    colors[ImGuiCol_ScrollbarBg] = dpg_surface;
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(82 / 255.0f, 82 / 255.0f, 85 / 255.0f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(90 / 255.0f, 90 / 255.0f, 95 / 255.0f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(90 / 255.0f, 90 / 255.0f, 95 / 255.0f, 1.0f);

    // ---- hydra_app.py's default_theme overrides, applied on top. Full
    // opacity throughout -- DPG's RGB-tuple add_theme_color calls default to
    // alpha=255; a few of these were previously (and wrongly) given reduced
    // alpha here.
    colors[ImGuiCol_Button] = kButtonColor;
    colors[ImGuiCol_ButtonHovered] = kButtonHoveredColor;
    colors[ImGuiCol_ButtonActive] = kButtonActiveColor;
    colors[ImGuiCol_TextDisabled] = kDimTextColor;
    colors[ImGuiCol_CheckMark] = kAccentColor;
    colors[ImGuiCol_FrameBgHovered] = kFrameHoveredColor;
    colors[ImGuiCol_FrameBgActive] = ImVec4(0, 150 / 255.0f, 150 / 255.0f, 1.0f);
    colors[ImGuiCol_Header] = kSelectedRowColor;
    colors[ImGuiCol_HeaderHovered] = kFrameHoveredColor;
    colors[ImGuiCol_HeaderActive] = kAccentColor;
    colors[ImGuiCol_TitleBgActive] = ImVec4(100 / 255.0f, 0, 0, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0, 200 / 255.0f, 200 / 255.0f, 1.0f);
    colors[ImGuiCol_Text] = kDefaultTextColor;

    // DPG's disabled state is a flat opaque gray (verified: a disabled
    // button samples as exactly (100,100,100), no fade). ImGui's
    // BeginDisabled() instead multiplies style.Alpha by DisabledAlpha
    // (0.60 default), which would wash out the explicit colors
    // begin_disabled_button()/begin_disabled_input() push below -- so
    // disable that multiply entirely and rely on the explicit push instead.
    //
    // INVARIANT: because of this, a bare ImGui::BeginDisabled() renders
    // EXACTLY like an enabled control (it only swallows input) -- every
    // disabled control must go through begin_disabled_button()/
    // begin_disabled_input() so the user can see it's disabled.
    style.DisabledAlpha = 1.0f;

    // Text alignment, written here and nowhere else (the single-owner row in
    // tests/single_owner/rules/ui/text-alignment.json). Text starts at the
    // left edge of the space it is given. A button's label stays centred in
    // its button: the user's answer 6 in
    // docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md.
    // Both values are ImGui's own defaults, written out so the choice is seen
    // in one place.
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
}

void begin_disabled_button(bool disabled) {
    ImGui::BeginDisabled(disabled);
    if (disabled) {
        ImGui::PushStyleColor(ImGuiCol_Button, kDisabledButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kDisabledButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kDisabledButtonColor);
        ImGui::PushStyleColor(ImGuiCol_Text, kDisabledButtonTextColor);
    }
}

void end_disabled_button(bool disabled) {
    if (disabled) ImGui::PopStyleColor(4);
    ImGui::EndDisabled();
}

void begin_disabled_input(bool disabled) {
    ImGui::BeginDisabled(disabled);
    if (disabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, kDisabledInputTextColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kDisabledInputBgColor);
        // InputInt's +/- step buttons use Button colors, not FrameBg; Python's
        // mvInputInt(enabled_state=False) component covers this explicitly.
        ImGui::PushStyleColor(ImGuiCol_Button, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kDisabledInputBgColor);
    }
}

void end_disabled_input(bool disabled) {
    if (disabled) ImGui::PopStyleColor(7);
    ImGui::EndDisabled();
}

void begin_disabled_checkbox(bool disabled) {
    ImGui::BeginDisabled(disabled);
    if (disabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, kDimTextColor);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, kDimTextColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kDisabledInputBgColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kDisabledInputBgColor);
    }
}

void end_disabled_checkbox(bool disabled) {
    if (disabled) ImGui::PopStyleColor(5);
    ImGui::EndDisabled();
}

}  // namespace hydra::ui
