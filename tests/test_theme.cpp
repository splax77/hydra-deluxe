// The theme's text colours against the surfaces they sit on, measured the way
// WCAG 2.2 defines contrast: (L1 + 0.05) / (L2 + 0.05) on relative luminance.
// 4.5:1 is the minimum for normal-size text.

#include "doctest.h"

#include "imgui.h"
#include "ui/theme.h"
#include "wcag_util.h"

using namespace hydra::ui;

namespace {

double luminance(const ImVec4& c) { return testwcag::relative_luminance(c.x, c.y, c.z); }

double contrast(const ImVec4& a, const ImVec4& b) {
    return testwcag::contrast_ratio(luminance(a), luminance(b));
}

}  // namespace

TEST_CASE("theme: button text reads at 4.5:1 in every button state") {
    CHECK(contrast(kDefaultTextColor, kButtonColor) >= 4.5);
    CHECK(contrast(kDefaultTextColor, kButtonHoveredColor) >= 4.5);
    CHECK(contrast(kDefaultTextColor, kButtonActiveColor) >= 4.5);
}

TEST_CASE("theme: dimmed and disabled text stays readable") {
    CHECK(contrast(kDimTextColor, kWindowBgColor) >= 4.5);
    CHECK(contrast(kDimTextColor, kSettingsBarBg) >= 4.5);
    CHECK(contrast(kDimTextColor, kPanelBg) >= 4.5);
    CHECK(contrast(kDisabledInputTextColor, kDisabledInputBgColor) >= 4.5);
    CHECK(contrast(kDisabledButtonTextColor, kDisabledButtonColor) >= 4.5);
}

TEST_CASE("theme: apply_theme uses the readable shades") {
    ImGui::CreateContext();
    apply_theme();
    const ImVec4* c = ImGui::GetStyle().Colors;
    CHECK(c[ImGuiCol_Button].y == kButtonColor.y);
    CHECK(c[ImGuiCol_ButtonHovered].y == kButtonHoveredColor.y);
    CHECK(c[ImGuiCol_ButtonActive].y == kButtonActiveColor.y);
    CHECK(c[ImGuiCol_TextDisabled].x == kDimTextColor.x);
    ImGui::DestroyContext();
}

// Disabled input text and dimmed text are one grey, written once: the
// disabled name refers to the dimmed one rather than spelling it again.
TEST_CASE("theme: disabled input text is the dimmed grey") {
    CHECK(&kDisabledInputTextColor == &kDimTextColor);
    CHECK(kDisabledInputTextColor.x == kDimTextColor.x);
    CHECK(kDisabledInputTextColor.y == kDimTextColor.y);
    CHECK(kDisabledInputTextColor.z == kDimTextColor.z);
    CHECK(kDisabledInputTextColor.w == kDimTextColor.w);
}

TEST_CASE("theme: frames and headers share one hover teal") {
    ImGui::CreateContext();
    apply_theme();
    const ImVec4* c = ImGui::GetStyle().Colors;
    for (const ImGuiCol slot : {ImGuiCol_FrameBgHovered, ImGuiCol_HeaderHovered}) {
        CHECK(c[slot].x == kFrameHoveredColor.x);
        CHECK(c[slot].y == kFrameHoveredColor.y);
        CHECK(c[slot].z == kFrameHoveredColor.z);
        CHECK(c[slot].w == kFrameHoveredColor.w);
    }
    ImGui::DestroyContext();
}

// The numbers D48, Q26 names for the Star Power gold.
TEST_CASE("theme: the Star Power gold is named") {
    CHECK(kStarPowerColor.x == 255 / 255.0f);
    CHECK(kStarPowerColor.y == 204 / 255.0f);
    CHECK(kStarPowerColor.z == 51 / 255.0f);
    CHECK(kStarPowerColor.w == 1.0f);
}
