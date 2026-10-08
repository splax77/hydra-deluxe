#include "ui/theme.h"

#include <stdexcept>

#include "app/report.h"  // ChipToken

namespace hydra::ui {

ImVec4 chip_color(app::report::ChipToken token) {
    using app::report::ChipToken;
    // The tiers are the HTML pages' dark-scheme --t0 to --t5 and --tn
    // (html_page.cpp's kReportCss), which the windows keep (D103). The dim
    // chip is the theme's dimmed text.
    switch (token) {
        case ChipToken::t0: return ImVec4(0x4f / 255.0f, 0xbf / 255.0f, 0x94 / 255.0f, 1.0f);
        case ChipToken::t1: return ImVec4(0xe0 / 255.0f, 0xb1 / 255.0f, 0x3f / 255.0f, 1.0f);
        case ChipToken::t2: return ImVec4(0xf0 / 255.0f, 0x89 / 255.0f, 0x4e / 255.0f, 1.0f);
        case ChipToken::t3: return ImVec4(0xf2 / 255.0f, 0x68 / 255.0f, 0x6b / 255.0f, 1.0f);
        case ChipToken::t4: return ImVec4(0xe0 / 255.0f, 0x7a / 255.0f, 0xc0 / 255.0f, 1.0f);
        case ChipToken::t5: return ImVec4(0xa7 / 255.0f, 0x8b / 255.0f, 0xfa / 255.0f, 1.0f);
        case ChipToken::tn: return ImVec4(0x94 / 255.0f, 0x9a / 255.0f, 0xa6 / 255.0f, 1.0f);
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
    colors[ImGuiCol_Header] = dpg_surface;
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
