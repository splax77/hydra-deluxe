// The theme's colours against the surfaces they sit on, measured with the
// contrast formula in wcag_util.h, and what apply_theme writes into ImGui's
// style.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "app/html_page.h"  // kChipRgb*
#include "app/report.h"     // ChipToken, tier_token
#include "core/squeeze_rating.h"
#include "imgui.h"
#include "ui/theme.h"
#include "wcag_util.h"

#include "headless_imgui_fixture.h"

using namespace hydra::ui;

namespace {

// The least contrast that counts as readable for text, and for a mark that
// is not text: wcag_util.h holds both floors.
constexpr double kReadableTextContrast = testwcag::kTextContrast;
constexpr double kReadableMarkContrast = testwcag::kLargeTextOrMarkContrast;

double luminance(const ImVec4& c) { return testwcag::relative_luminance(c.x, c.y, c.z); }

double contrast(const ImVec4& a, const ImVec4& b) {
    return testwcag::contrast_ratio(luminance(a), luminance(b));
}

// Every channel of `got` equals `want`'s, alpha included.
void check_same_color(const std::string& what, const ImVec4& got, const ImVec4& want) {
    CAPTURE(what);
    CHECK(got.x == want.x);
    CHECK(got.y == want.y);
    CHECK(got.z == want.z);
    CHECK(got.w == want.w);
}

// Every chip token a report window draws.
const hydra::app::report::ChipToken kEveryChip[] = {
    hydra::app::report::ChipToken::t0, hydra::app::report::ChipToken::t1,
    hydra::app::report::ChipToken::t2, hydra::app::report::ChipToken::t3,
    hydra::app::report::ChipToken::t4, hydra::app::report::ChipToken::t5,
    hydra::app::report::ChipToken::tn, hydra::app::report::ChipToken::muted,
};

// Where an item drew its text: the leftmost and rightmost x of the vertices
// `draw` added in the text colour, and the item's own left and right edges.
struct TextSpan {
    float text_left = 0.0f;
    float text_right = 0.0f;
    float item_left = 0.0f;
    float item_right = 0.0f;
    int vertices = 0;
};

template <class Draw>
TextSpan text_span(Draw draw) {
    const ImDrawList* list = ImGui::GetWindowDrawList();
    const int before = list->VtxBuffer.Size;
    draw();
    const ImU32 text = ImGui::GetColorU32(ImGuiCol_Text);
    TextSpan s;
    for (int i = before; i < list->VtxBuffer.Size; ++i) {
        const ImDrawVert& v = list->VtxBuffer[i];
        if (v.col != text) continue;
        s.text_left = s.vertices == 0 ? v.pos.x : (std::min)(s.text_left, v.pos.x);
        s.text_right = s.vertices == 0 ? v.pos.x : (std::max)(s.text_right, v.pos.x);
        ++s.vertices;
    }
    s.item_left = ImGui::GetItemRectMin().x;
    s.item_right = ImGui::GetItemRectMax().x;
    return s;
}

}  // namespace

TEST_CASE("theme: button text reads in every button state") {
    CHECK(contrast(kDefaultTextColor, kButtonColor) >= kReadableTextContrast);
    CHECK(contrast(kDefaultTextColor, kButtonHoveredColor) >= kReadableTextContrast);
    CHECK(contrast(kDefaultTextColor, kButtonActiveColor) >= kReadableTextContrast);
}

TEST_CASE("theme: dimmed and disabled text stays readable") {
    CHECK(contrast(kDimTextColor, kWindowBgColor) >= kReadableTextContrast);
    CHECK(contrast(kDimTextColor, kPanelBg) >= kReadableTextContrast);
    CHECK(contrast(kDisabledInputTextColor, kDisabledInputBgColor) >= kReadableTextContrast);
    CHECK(contrast(kDisabledButtonTextColor, kDisabledButtonColor) >= kReadableTextContrast);
}

TEST_CASE("theme: apply_theme uses the readable shades") {
    ImGui::CreateContext();
    apply_theme();
    const ImVec4* c = ImGui::GetStyle().Colors;
    check_same_color("Button", c[ImGuiCol_Button], kButtonColor);
    check_same_color("ButtonHovered", c[ImGuiCol_ButtonHovered], kButtonHoveredColor);
    check_same_color("ButtonActive", c[ImGuiCol_ButtonActive], kButtonActiveColor);
    check_same_color("TextDisabled", c[ImGuiCol_TextDisabled], kDimTextColor);
    ImGui::DestroyContext();
}

// A real button and a real selectable, each 240 px wide, drawn after
// apply_theme: the button's label sits in the middle of the button, and the
// selectable's text starts where the selectable starts. The style starts off
// the other way round so the test sees apply_theme's write.
TEST_CASE("theme: apply_theme centres button labels and starts selectable text at the left") {
    hydra::test::HeadlessImGui imgui;
    ImGuiStyle& style = ImGui::GetStyle();
    style.ButtonTextAlign = ImVec2(0.0f, 0.0f);
    style.SelectableTextAlign = ImVec2(1.0f, 1.0f);
    apply_theme();

    // A glyph's quad sits a pixel or so inside its advance width, and ImGui
    // rounds text positions to whole pixels; text placed the other way would
    // be some 100 px off.
    const float slack = 3.0f;
    const ImVec2 size(240.0f, 40.0f);

    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(480.0f, 160.0f));
    ImGui::Begin("###theme_alignment", nullptr, ImGuiWindowFlags_NoSavedSettings);

    const TextSpan button = text_span([&] { ImGui::Button("H##button", size); });
    REQUIRE(button.vertices > 0);
    const float left_gap = button.text_left - button.item_left;
    const float right_gap = button.item_right - button.text_right;
    CAPTURE(left_gap);
    CAPTURE(right_gap);
    CHECK(left_gap - right_gap <= slack);
    CHECK(right_gap - left_gap <= slack);

    const float selectable_start = ImGui::GetCursorScreenPos().x;
    const TextSpan selectable =
        text_span([&] { ImGui::Selectable("H##selectable", false, 0, size); });
    REQUIRE(selectable.vertices > 0);
    CAPTURE(selectable_start);
    CAPTURE(selectable.text_left);
    CHECK(selectable.text_left - selectable_start <= slack);
    CHECK(selectable.text_left >= selectable_start);

    ImGui::End();
    ImGui::EndFrame();
}

TEST_CASE("theme: a selected row is the dark teal and its text stays readable") {
    ImGui::CreateContext();
    apply_theme();
    const ImVec4& header = ImGui::GetStyle().Colors[ImGuiCol_Header];
    check_same_color("Header", header, kSelectedRowColor);
    ImGui::DestroyContext();
    // Every text colour a Library, report or Paths-tab row draws in: the
    // default and dimmed text, the gold of an optimal path or a Ready song,
    // the grey of a not-analyzed song, and the orange of a stale song or a
    // warned timing. The report chips' colours are checked below, beside
    // their check on the window.
    CHECK(contrast(kDefaultTextColor, kSelectedRowColor) >= kReadableTextContrast);
    CHECK(contrast(kBestPathColor, kSelectedRowColor) >= kReadableTextContrast);
    CHECK(contrast(kDimTextColor, kSelectedRowColor) >= kReadableTextContrast);
    CHECK(contrast(kNewSongColor, kSelectedRowColor) >= kReadableTextContrast);
    CHECK(contrast(kWarningColor, kSelectedRowColor) >= kReadableTextContrast);
}

TEST_CASE("theme: frames and headers share one hover teal") {
    ImGui::CreateContext();
    apply_theme();
    const ImVec4* c = ImGui::GetStyle().Colors;
    for (const ImGuiCol slot : {ImGuiCol_FrameBgHovered, ImGuiCol_HeaderHovered}) {
        check_same_color(ImGui::GetStyleColorName(slot), c[slot], kFrameHoveredColor);
    }
    ImGui::DestroyContext();
}

// Every report chip colour (chip_color) against the window it sits on. The
// chip's outline and its words are both drawn in the chip's colour, so the
// colour must read as text. A report row's chips and alert cells also sit on
// the selected row's face when the row is picked.
TEST_CASE("theme: report chip colours read on the window background") {
    for (const hydra::app::report::ChipToken token : kEveryChip) {
        CAPTURE(static_cast<int>(token));
        CHECK(contrast(chip_color(token), kWindowBgColor) >= kReadableTextContrast);
        CHECK(contrast(chip_color(token), kSelectedRowColor) >= kReadableTextContrast);
    }
}

// Each tier chip is drawn in the colour the fill page's dark scheme gives that
// tier (html_page.h's kChipRgb*, written 0xRRGGBB), fully opaque, and the dim
// chip is the theme's dimmed text.
TEST_CASE("theme: report chip colours are the pages' and the dimmed grey") {
    using hydra::app::report::ChipToken;
    namespace html = hydra::app::html;
    const std::pair<ChipToken, uint32_t> page_colors[] = {
        {ChipToken::t0, html::kChipRgbT0}, {ChipToken::t1, html::kChipRgbT1},
        {ChipToken::t2, html::kChipRgbT2}, {ChipToken::t3, html::kChipRgbT3},
        {ChipToken::t4, html::kChipRgbT4}, {ChipToken::t5, html::kChipRgbT5},
        {ChipToken::tn, html::kChipRgbTn},
    };
    for (const auto& [token, rgb] : page_colors) {
        CAPTURE(static_cast<int>(token));
        const ImVec4 chip = chip_color(token);
        CHECK(chip.x == ((rgb >> 16) & 0xffu) / 255.0f);
        CHECK(chip.y == ((rgb >> 8) & 0xffu) / 255.0f);
        CHECK(chip.z == (rgb & 0xffu) / 255.0f);
        CHECK(chip.w == 1.0f);
    }
    check_same_color("muted chip", chip_color(ChipToken::muted), kDimTextColor);
}

// A path row's tier carries tier_for's token name (ReportRow::tok), and
// tier_token turns it into the chip the row is drawn in: every tier of the
// ladder gets a chip of its own, in the ladder's order, a path with no
// squeeze gets the no-squeeze chip, and a tier's label is not read as a
// token.
TEST_CASE("theme: tier_token reads every token tier_for gives") {
    using hydra::app::report::ChipToken;
    using hydra::app::report::tier_token;
    const std::vector<hydra::TimingTier> tiers = hydra::timing_tiers();
    REQUIRE_FALSE(tiers.empty());
    std::set<int> chips;
    int previous = -1;
    for (const hydra::TimingTier& t : tiers) {
        CAPTURE(std::string(t.tok));
        REQUIRE_NOTHROW(tier_token(t.tok));
        const int chip = static_cast<int>(tier_token(t.tok));
        CHECK(chip > previous);
        previous = chip;
        chips.insert(chip);
        CHECK_THROWS(tier_token(t.name));
    }
    CHECK(chips.size() == tiers.size());
    CHECK(tier_token(hydra::none_tier(tiers).tok) == ChipToken::tn);
    CHECK(tier_token(hydra::tier_for(std::nullopt, tiers).second) == ChipToken::tn);
}

// The version bar: each dot is a mark and the bar's sentences are text, all
// on the bar's background, which sits darker than the window. The dots are
// not the orange of a problem line.
TEST_CASE("theme: the version bar's dots and text read on its background") {
    for (const ImVec4* dot : {&kUpToDateDotColor, &kUpdateAvailableDotColor, &kNeutralDotColor}) {
        CHECK(contrast(*dot, kVersionBarBg) >= kReadableMarkContrast);
        const bool is_warning =
            dot->x == kWarningColor.x && dot->y == kWarningColor.y && dot->z == kWarningColor.z;
        CHECK_FALSE(is_warning);
    }
    CHECK(contrast(kDefaultTextColor, kVersionBarBg) >= kReadableTextContrast);
    CHECK(contrast(kDimTextColor, kVersionBarBg) >= kReadableTextContrast);
    CHECK(kVersionBarBg.x < kWindowBgColor.x);
    CHECK(kVersionBarBg.y < kWindowBgColor.y);
    CHECK(kVersionBarBg.z < kWindowBgColor.z);
}
