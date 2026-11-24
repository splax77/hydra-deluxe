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

#include "app/dm_report.h"       // DmReportRow, kStatusWords, status_token
#include "app/dm_report_view.h"  // dm_columns
#include "app/html_page.h"       // kChipRgb*
#include "app/path_report_view.h"  // path_columns
#include "app/report.h"          // ChipToken, tier_token
#include "app/report_view.h"     // Column, CellLook, Tone
#include "core/squeeze_rating.h"
#include "imgui.h"
#include "ui/report_window.h"    // the table's colours
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

// An ImGui context styled by apply_theme, as the report windows draw in.
struct ThemedContext {
    ThemedContext() {
        ImGui::CreateContext();
        apply_theme();
    }
    ~ThemedContext() { ImGui::DestroyContext(); }
    ThemedContext(const ThemedContext&) = delete;
    ThemedContext& operator=(const ThemedContext&) = delete;
};

// ImGui's own dark style, untouched by Hydra: where the table's header grey
// and row stripe come from, since apply_theme sets neither.
ImGuiStyle fresh_dark_style() {
    ImGuiStyle style;
    ImGui::StyleColorsDark(&style);
    return style;
}

void check_same_rgb(const std::string& what, report_frame::Rgb got, report_frame::Rgb want) {
    CAPTURE(what);
    CAPTURE(static_cast<int>(got.r));
    CAPTURE(static_cast<int>(got.g));
    CAPTURE(static_cast<int>(got.b));
    CAPTURE(static_cast<int>(want.r));
    CAPTURE(static_cast<int>(want.g));
    CAPTURE(static_cast<int>(want.b));
    CHECK(got == want);
}

// The column `id` of a report's real column list.
template <class Row>
const hydra::app::report_view::Column<Row>* column_with_id(
    const std::vector<hydra::app::report_view::Column<Row>>& columns, const std::string& id) {
    for (const hydra::app::report_view::Column<Row>& c : columns)
        if (c.id == id) return &c;
    return nullptr;
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

// ---- The report table's colours (report_frame in ui/report_window.h) --------

// A plain cell's words: the row's tone first, then the column's look. Fails
// if the alert stops winning over a dim column, or a dim tone over a plain one.
TEST_CASE("report table: a plain cell reads in its look's and its tone's colour") {
    using hydra::app::report::ChipToken;
    using hydra::app::report_view::CellLook;
    using hydra::app::report_view::Tone;
    ThemedContext context;
    CellLook plain;
    CellLook dim;
    dim.dim = true;
    check_same_color("plain, normal", report_frame::cell_color(plain, Tone::Normal),
                     kDefaultTextColor);
    check_same_color("dim, normal", report_frame::cell_color(dim, Tone::Normal), kDimTextColor);
    check_same_color("plain, dim", report_frame::cell_color(plain, Tone::Dim), kDimTextColor);
    check_same_color("dim, dim", report_frame::cell_color(dim, Tone::Dim), kDimTextColor);
    check_same_color("plain, alert", report_frame::cell_color(plain, Tone::Alert),
                     chip_color(ChipToken::t3));
    check_same_color("dim, alert", report_frame::cell_color(dim, Tone::Alert),
                     chip_color(ChipToken::t3));
}

// The path report's real columns, with the chip rule the path window hands
// the table (path_report_window.cpp: a row's Timing chip is tier_token of its
// tok). Fails if Song stops reading plain, a name column stops reading dim,
// or Timing stops being a chip in its own tier's colour.
TEST_CASE("report table: the path report's cells read in the window's colours") {
    using hydra::app::path_report_view::path_columns;
    using hydra::app::report::ReportRow;
    using hydra::app::report::tier_token;
    ThemedContext context;
    const auto columns = path_columns();
    const auto* song = column_with_id(columns, "song");
    const auto* artist = column_with_id(columns, "artist");
    const auto* charter = column_with_id(columns, "charter");
    const auto* timing = column_with_id(columns, "tier");
    REQUIRE(song);
    REQUIRE(artist);
    REQUIRE(charter);
    REQUIRE(timing);
    report_frame::RowLook<ReportRow> look;
    look.chip = [](const ReportRow& r) { return tier_token(r.tok); };

    const std::vector<hydra::TimingTier> tiers = hydra::timing_tiers();
    REQUIRE_FALSE(tiers.empty());
    for (const hydra::TimingTier& t : tiers) {
        ReportRow row;
        row.song = "A song";
        row.artist = "An artist";
        row.charter = "A charter";
        row.tier = t.name;
        row.tok = t.tok;
        CAPTURE(row.tok);

        const report_frame::CellPaint s = report_frame::cell_paint(*song, row, look);
        CHECK_FALSE(s.chip.has_value());
        check_same_color("Song", s.color, kDefaultTextColor);
        const report_frame::CellPaint a = report_frame::cell_paint(*artist, row, look);
        CHECK_FALSE(a.chip.has_value());
        check_same_color("Artist", a.color, kDimTextColor);
        const report_frame::CellPaint c = report_frame::cell_paint(*charter, row, look);
        CHECK_FALSE(c.chip.has_value());
        check_same_color("Charter", c.color, kDimTextColor);

        const report_frame::CellPaint chip = report_frame::cell_paint(*timing, row, look);
        REQUIRE(chip.chip.has_value());
        CHECK(*chip.chip == tier_token(row.tok));
        check_same_color("Timing", chip.color, chip_color(tier_token(row.tok)));
    }
}

// The comparison's real columns, with the chip rule the comparison window
// hands the table (dm_report_window.cpp: a row's Status chip is status_token
// of its status). Points left is the one column whose colour moves with the
// row. Fails if any of its four cases reads in the wrong colour, if Posted
// stops reading dim, or if a status's chip is not in that status's token.
TEST_CASE("report table: the comparison's cells read in the window's colours") {
    using hydra::app::dm_report::DmReportRow;
    using hydra::app::dm_report::status_token;
    using hydra::app::dm_report_view::dm_columns;
    using hydra::app::report::ChipToken;
    namespace dm = hydra::app::dm_report;
    ThemedContext context;
    const auto columns = dm_columns();
    const auto* posted = column_with_id(columns, "posted");
    const auto* left = column_with_id(columns, "delta");
    const auto* status = column_with_id(columns, "status");
    REQUIRE(posted);
    REQUIRE(left);
    REQUIRE(status);
    report_frame::RowLook<DmReportRow> look;
    look.chip = [](const DmReportRow& r) { return status_token(r.status); };

    DmReportRow plain;
    plain.posted = "2026-11-01T12:00:00Z";
    plain.status = dm::kStatusUnderOptimal;
    plain.optimal = 1000;
    plain.actual = 900;
    plain.delta = 100;
    check_same_color("Posted", report_frame::cell_paint(*posted, plain, look).color,
                     kDimTextColor);

    // Under optimal, with points left: the plain text colour.
    const report_frame::CellPaint under = report_frame::cell_paint(*left, plain, look);
    CHECK_FALSE(under.chip.has_value());
    check_same_color("Points left, under optimal", under.color, kDefaultTextColor);

    DmReportRow no_delta = plain;
    no_delta.optimal.reset();
    no_delta.delta.reset();
    check_same_color("Points left, no delta", report_frame::cell_paint(*left, no_delta, look).color,
                     kDimTextColor);

    DmReportRow other_speed = plain;
    other_speed.status = dm::kStatusOtherSpeed;
    check_same_color("Points left, other speed",
                     report_frame::cell_paint(*left, other_speed, look).color, kDimTextColor);

    DmReportRow above = plain;
    above.actual = 1100;
    above.delta = -100;
    above.above_optimal = true;
    above.status = dm::kStatusAboveOptimal;
    check_same_color("Points left, above optimal",
                     report_frame::cell_paint(*left, above, look).color,
                     chip_color(ChipToken::t3));

    for (const dm::StatusWords& w : dm::kStatusWords) {
        DmReportRow row = plain;
        row.status = w.status;
        CAPTURE(row.status);
        const report_frame::CellPaint chip = report_frame::cell_paint(*status, row, look);
        REQUIRE(chip.chip.has_value());
        CHECK(*chip.chip == status_token(row.status));
        check_same_color("Status", chip.color, chip_color(status_token(row.status)));
    }
}

// A window that hands the table no chip rule gets no chips: the column's
// cell reads as plain text in its look, as the table drew it before.
TEST_CASE("report table: with no chip rule a chip column is plain text") {
    using hydra::app::path_report_view::path_columns;
    using hydra::app::report::ReportRow;
    ThemedContext context;
    const auto columns = path_columns();
    const auto* timing = column_with_id(columns, "tier");
    REQUIRE(timing);
    REQUIRE(timing->look.chip);
    const report_frame::RowLook<ReportRow> no_rule;
    ReportRow row;
    const std::vector<hydra::TimingTier> tiers = hydra::timing_tiers();
    row.tier = hydra::none_tier(tiers).name;
    row.tok = hydra::none_tier(tiers).tok;
    const report_frame::CellPaint paint = report_frame::cell_paint(*timing, row, no_rule);
    CHECK_FALSE(paint.chip.has_value());
    check_same_color("Timing, no rule", paint.color, kDefaultTextColor);
}

// The table's surfaces, read from the live style. The header grey and the
// stripe are ImGui's own dark defaults, which apply_theme leaves alone; the
// header test fails if Hydra ever sets that slot, so this plan's copy of the
// window would need a look.
TEST_CASE("report table: the background, header and row fills are the window's") {
    ThemedContext context;
    const ImGuiStyle fresh = fresh_dark_style();
    check_same_color("background", report_frame::table_background(), kWindowBgColor);
    check_same_color("header text", report_frame::header_text(), kDefaultTextColor);
    check_same_color("header fill", report_frame::header_fill(),
                     fresh.Colors[ImGuiCol_TableHeaderBg]);

    const report_frame::Rgb background = report_frame::to_rgb(kWindowBgColor);
    const report_frame::Rgb stripe =
        report_frame::lay_over(fresh.Colors[ImGuiCol_TableRowBgAlt], background);
    check_same_rgb("place 0", report_frame::row_fill(0), background);
    check_same_rgb("place 1", report_frame::row_fill(1), stripe);
    check_same_rgb("place 2", report_frame::row_fill(2), background);
    check_same_rgb("place 3", report_frame::row_fill(3), stripe);
    CHECK(report_frame::row_fill(1) != report_frame::row_fill(0));
}

// The mixing helper on colours made up here, none of them Hydra's. Fails if
// it ignores the top colour's alpha, or rounds a share the wrong way.
TEST_CASE("report table: lay_over mixes a see-through colour over a solid one") {
    const report_frame::Rgb under{10, 20, 30};
    check_same_rgb("see-through", report_frame::lay_over(ImVec4(1.0f, 1.0f, 1.0f, 0.0f), under),
                   under);
    const ImVec4 solid(0.2f, 0.4f, 0.6f, 1.0f);
    check_same_rgb("solid", report_frame::lay_over(solid, under), report_frame::to_rgb(solid));
    // White at strength 51 of 255 over black is one fifth of the way up.
    check_same_rgb("one fifth",
                   report_frame::lay_over(ImVec4(1.0f, 1.0f, 1.0f, 51.0f / 255.0f),
                                          report_frame::Rgb{0, 0, 0}),
                   report_frame::Rgb{51, 51, 51});
}

// The converter gives each tier chip the bytes the page holds for it, so a
// colour turned to 0-255 for a file is the one the window draws.
TEST_CASE("report table: to_rgb gives the tier chips' own bytes") {
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
        const report_frame::Rgb want{static_cast<uint8_t>((rgb >> 16) & 0xffu),
                                     static_cast<uint8_t>((rgb >> 8) & 0xffu),
                                     static_cast<uint8_t>(rgb & 0xffu)};
        check_same_rgb("tier chip", report_frame::to_rgb(chip_color(token)), want);
    }
}
