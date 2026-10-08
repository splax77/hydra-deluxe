// The theme's text colours against the surfaces they sit on, measured the way
// WCAG 2.2 defines contrast: (L1 + 0.05) / (L2 + 0.05) on relative luminance.
// 4.5:1 is the minimum for normal-size text.

#include "doctest.h"

#include "app/report.h"  // ChipToken, tier_token
#include "core/squeeze_rating.h"
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

// Every report chip colour (chip_color) against the window it sits on. The
// chip's outline is a non-text mark (3:1) and its words are text (4.5:1),
// both drawn in the chip's colour.
TEST_CASE("theme: report chip colours read on the window background") {
    using hydra::app::report::ChipToken;
    for (const ChipToken token : {ChipToken::t0, ChipToken::t1, ChipToken::t2, ChipToken::t3,
                                  ChipToken::t4, ChipToken::t5, ChipToken::tn,
                                  ChipToken::muted}) {
        CAPTURE(static_cast<int>(token));
        CHECK(contrast(chip_color(token), kWindowBgColor) >= 3.0);
        CHECK(contrast(chip_color(token), kWindowBgColor) >= 4.5);
    }
}

// The pages' dark-scheme tier colours, as html_page.cpp's kReportCss writes
// them, and the dim chip as the theme's dimmed text.
TEST_CASE("theme: report chip colours are the pages' and the dimmed grey") {
    using hydra::app::report::ChipToken;
    const ImVec4 t0 = chip_color(ChipToken::t0);  // #4fbf94
    CHECK(t0.x == 0x4f / 255.0f);
    CHECK(t0.y == 0xbf / 255.0f);
    CHECK(t0.z == 0x94 / 255.0f);
    const ImVec4 tn = chip_color(ChipToken::tn);  // #949aa6
    CHECK(tn.x == 0x94 / 255.0f);
    CHECK(tn.y == 0x9a / 255.0f);
    CHECK(tn.z == 0xa6 / 255.0f);
    const ImVec4 muted = chip_color(ChipToken::muted);
    CHECK(muted.x == kDimTextColor.x);
    CHECK(muted.y == kDimTextColor.y);
    CHECK(muted.z == kDimTextColor.z);
}

// A path row's tier carries tier_for's token name (ReportRow::tok);
// tier_token reads it.
TEST_CASE("theme: tier_token reads every token tier_for gives") {
    using hydra::app::report::ChipToken;
    using hydra::app::report::tier_token;
    CHECK(tier_token("t0") == ChipToken::t0);
    CHECK(tier_token("t1") == ChipToken::t1);
    CHECK(tier_token("t2") == ChipToken::t2);
    CHECK(tier_token("t3") == ChipToken::t3);
    CHECK(tier_token("t4") == ChipToken::t4);
    CHECK(tier_token("t5") == ChipToken::t5);
    CHECK(tier_token("tn") == ChipToken::tn);
    CHECK_THROWS(tier_token("t9"));
    for (const hydra::TimingTier& t : hydra::timing_tiers()) CHECK_NOTHROW(tier_token(t.tok));
}

// The numbers D48, Q26 names for the Star Power gold.
TEST_CASE("theme: the Star Power gold is named") {
    CHECK(kStarPowerColor.x == 255 / 255.0f);
    CHECK(kStarPowerColor.y == 204 / 255.0f);
    CHECK(kStarPowerColor.z == 51 / 255.0f);
    CHECK(kStarPowerColor.w == 1.0f);
}
