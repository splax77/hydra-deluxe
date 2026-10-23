// Unit tests for the one column-width rule (ui/column_widths). The rule's
// arithmetic has no ImGui in it, so these feed it a pretend font: every
// character is 10 pixels wide. Every expected number is worked by hand from
// the inputs in the comment beside it.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"  // ImGuiTable, for the apply_column_widths tests

#include "headless_imgui_fixture.h"
#include "app/dm_report_view.h"
#include "app/path_report_view.h"
#include "ui/column_widths.h"
#include "ui/fonts.h"
#include "ui/report_window.h"

using hydra::ui::CellText;
using hydra::ui::ColumnLayout;
using hydra::ui::ColumnSpec;
using hydra::ui::MeasuredWidths;
using hydra::ui::TableRoom;
using hydra::test::HeadlessImGui;

namespace {

// The pretend font: 10 pixels a character.
float ten_px(std::string_view s) { return 10.0f * static_cast<float>(s.size()); }

ColumnSpec spec(const std::string& header, bool may_cut, float padding = 0.0f, int share = 0) {
    return ColumnSpec{header, may_cut, padding, ten_px, share};
}

// A text column with its share.
ColumnSpec text_spec(const std::string& header, int share) {
    return spec(header, true, 0.0f, share);
}

// A table given as rows of cells.
using Rows = std::vector<std::vector<std::string>>;
CellText cells(const Rows& rows) {
    return [&rows](std::size_t r, std::size_t c) { return rows[r][c]; };
}

// The Library's five text columns with the user's shares (2026-10-14), in the
// pretend font. Headers: "Title" 50, "Artist" 60, "Charter" 70, "Folder" 60,
// "Best path" 90.
const std::vector<ColumnSpec> kLibrarySpecs = {text_spec("Title", 17), text_spec("Artist", 10),
                                               text_spec("Charter", 9), text_spec("Folder", 39),
                                               text_spec("Best path", 25)};
// The song panel open: Charter and Folder hidden.
const std::vector<bool> kPanelOpen = {true, true, false, false, true};

// Short Library rows. Text columns never read them; they are here so the
// measure has rows, as the app's does.
const Rows kLibraryRows = {{"Song", "Band", "Me", "pack", "1 2 3"},
                           {"Another song", "Group", "You", "pack", "2- 1"}};

MeasuredWidths library_measure(const Rows& rows = kLibraryRows) {
    return hydra::ui::measure_widths(kLibrarySpecs, rows.size(), cells(rows));
}

// The three-column table the number-beside-text tests share:
//   "Num"  never cuts. Header 3 chars = 30, widest cell "12345" = 50.
//   "Song" may cut, share 60.   Header 4 chars = 40.
//   "Artist" may cut, share 40. Header 6 chars = 60.
const std::vector<ColumnSpec> kSpecs = {spec("Num", false), text_spec("Song", 60),
                                        text_spec("Artist", 40)};
const Rows kRows = {{"12345", "abcdefghijklmnopqrst", "abcdefghij"}, {"1", "ab", "a"}};

// A room of `available` pixels with no spacing.
TableRoom room_of(float available, std::vector<bool> shown = {},
                  std::vector<std::optional<float>> pinned = {}) {
    TableRoom room;
    room.available = available;
    room.shown = std::move(shown);
    room.pinned = std::move(pinned);
    return room;
}

}  // namespace

TEST_CASE("column_widths: a header wider than every cell sets the width") {
    // Header "Artist" = 60; cells "Abc" = 30 and "Ab" = 20.
    const Rows rows = {{"Abc"}, {"Ab"}};
    const MeasuredWidths m = hydra::ui::measure_widths({spec("Artist", false)}, 2, cells(rows));
    CHECK(m.widths == std::vector<float>{60.0f});
    CHECK(m.header_widths == std::vector<float>{60.0f});
    CHECK_FALSE(m.widest_row[0].has_value());
}

TEST_CASE("column_widths: a chip's padding is added to its cells, not its header") {
    // Header "T" = 10. Cells "Beyond" = 60 and "Hit" = 30, each plus 8 of
    // padding: 68 and 38. The widest is row 0 at 68; the header stays 10.
    const Rows rows = {{"Beyond"}, {"Hit"}};
    const MeasuredWidths m = hydra::ui::measure_widths({spec("T", false, 8.0f)}, 2, cells(rows));
    CHECK(m.widths == std::vector<float>{68.0f});
    CHECK(m.header_widths == std::vector<float>{10.0f});
    CHECK(m.widest_row[0] == 0u);
}

TEST_CASE("column_widths: header_width_of measures the header, width_of the cells") {
    // Header "Points" in a pretend 5 px font = 30. Cells in the 10 px font:
    // "Points" = 60 and "1234" = 40. A cell that reads as the header still
    // measures in the cells' font, so row 0 is the widest at 60.
    ColumnSpec s = spec("Points", false);
    s.header_width_of = [](std::string_view t) { return 5.0f * static_cast<float>(t.size()); };
    const Rows rows = {{"Points"}, {"1234"}};
    const MeasuredWidths m = hydra::ui::measure_widths({s}, 2, cells(rows));
    CHECK(m.header_widths == std::vector<float>{30.0f});
    CHECK(m.widths == std::vector<float>{60.0f});
    CHECK(m.widest_row[0] == 0u);

    // Without header_width_of, width_of measures the header too: 60.
    const MeasuredWidths plain = hydra::ui::measure_widths({spec("Points", false)}, 2, cells(rows));
    CHECK(plain.header_widths == std::vector<float>{60.0f});
}

TEST_CASE("column_widths: a measure is stale until taken, and again after a scale change") {
    CHECK(MeasuredWidths{}.stale());
    const float before = hydra::ui::g_ui_scale;
    hydra::ui::g_ui_scale = 1.0f;
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    CHECK(m.scale == 1.0f);
    CHECK_FALSE(m.stale());
    hydra::ui::g_ui_scale = 1.25f;
    CHECK(m.stale());
    hydra::ui::g_ui_scale = before;
}

TEST_CASE("column_widths: the Library's text columns split the room by their shares") {
    // 1,000 px, no spacing, no number column: the shares apply to all 1,000.
    // Title 17% = 170, Artist 10% = 100, Charter 9% = 90, Folder 39% = 390,
    // Best path 25% = 250. Each is above its header. They sum to 1,000.
    const ColumnLayout l = hydra::ui::place_columns(library_measure(), kLibrarySpecs, room_of(1000.0f));
    CHECK(l.widths == std::vector<float>{170.0f, 100.0f, 90.0f, 390.0f, 250.0f});
    CHECK(l.inner_width == 1000.0f);
}

TEST_CASE("column_widths: hidden columns' shares go to the shown ones, lost pixels to the last") {
    // The song panel hides Charter and Folder. The shown shares 17, 10 and 25
    // sum to 52 and split the same 1,000:
    //   Title     1,000 x 17 / 52 = 326.9 -> 326
    //   Artist    1,000 x 10 / 52 = 192.3 -> 192
    //   Best path 1,000 x 25 / 52 = 480.8 -> 480
    // 326 + 192 + 480 = 998, so Best path, the last shown text column, takes
    // the 2 lost pixels: 482. The hidden columns keep their measured width,
    // their header's: Charter 70, Folder 60.
    const ColumnLayout l =
        hydra::ui::place_columns(library_measure(), kLibrarySpecs, room_of(1000.0f, kPanelOpen));
    CHECK(l.widths == std::vector<float>{326.0f, 192.0f, 70.0f, 60.0f, 482.0f});
    CHECK(l.inner_width == 1000.0f);
}

TEST_CASE("column_widths: a text column whose split is below its header stops there") {
    const MeasuredWidths m = library_measure();
    SUBCASE("300 px: Artist stops at its header, the others split the rest") {
        // Panel open, 300 px. First pass by 17, 10 and 25 of 52:
        //   Title 300 x 17 / 52 = 98.1, Artist 300 x 10 / 52 = 57.7 (below
        //   its header's 60), Best path 300 x 25 / 52 = 144.2.
        // Artist stops at 60. Title and Best path split the 240 left by 17
        // and 25 of 42: Title 97.1 -> 97, Best path 142.9 -> 142, and the
        // lost pixel goes to Best path: 143. 97 + 60 + 143 = 300.
        const ColumnLayout l =
            hydra::ui::place_columns(m, kLibrarySpecs, room_of(300.0f, kPanelOpen));
        CHECK(l.widths == std::vector<float>{97.0f, 60.0f, 70.0f, 60.0f, 143.0f});
        CHECK(l.inner_width == 300.0f);
    }
    SUBCASE("150 px: every column at its header, and the table scrolls") {
        // Every split is below its header: Title 150 x 17 / 52 = 49.0 < 50,
        // Artist 28.8 < 60, Best path 72.1 < 90. Each sits at its header:
        // 50, 60 and 90. The inner width 200 is more than the 150 of room.
        const ColumnLayout l =
            hydra::ui::place_columns(m, kLibrarySpecs, room_of(150.0f, kPanelOpen));
        CHECK(l.widths == std::vector<float>{50.0f, 60.0f, 70.0f, 60.0f, 90.0f});
        CHECK(l.inner_width == 200.0f);
        CHECK(l.min_inner_width == 200.0f);
    }
}

TEST_CASE("column_widths: a number column takes its widest and the text columns split the rest") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Available 260, spacing 10. Num takes its widest cell, 50. Leftover
    // 260 - 10 - 50 = 200: Song 60% = 120, Artist 40% = 80.
    // Inner width 50 + 120 + 80 + 10 = 260. Minimum: 50 + 40 + 60 + 10 = 160.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 120.0f, 80.0f});
    CHECK(l.inner_width == 260.0f);
    CHECK(l.min_inner_width == 160.0f);
}

TEST_CASE("column_widths: the text in a text column does not change the layout") {
    // The same Library table twice: once with short cells, once with every
    // cell 200 characters (2,000 px in the pretend font). Text columns
    // measure their headers alone, so both give the 1,000 px widths of the
    // shares: 170, 100, 90, 390, 250.
    const Rows long_rows = {std::vector<std::string>(5, std::string(200, 'a'))};
    const MeasuredWidths short_m = library_measure();
    const MeasuredWidths long_m = library_measure(long_rows);
    CHECK(short_m.widths == long_m.widths);
    const ColumnLayout a = hydra::ui::place_columns(short_m, kLibrarySpecs, room_of(1000.0f));
    const ColumnLayout b = hydra::ui::place_columns(long_m, kLibrarySpecs, room_of(1000.0f));
    CHECK(a.widths == b.widths);
    CHECK(b.widths == std::vector<float>{170.0f, 100.0f, 90.0f, 390.0f, 250.0f});
}

TEST_CASE("column_widths: a dragged text column keeps its width and the others re-split") {
    const MeasuredWidths m = library_measure();
    SUBCASE("Folder dragged to 200") {
        // 1,000 px, panel closed. Folder is fixed at 200, which leaves 800
        // for Title, Artist, Charter and Best path, shares 17, 10, 9 and 25
        // of 61:
        //   Title     800 x 17 / 61 = 222.95 -> 222
        //   Artist    800 x 10 / 61 = 131.15 -> 131
        //   Charter   800 x  9 / 61 = 118.03 -> 118
        //   Best path 800 x 25 / 61 = 327.87 -> 327
        // 222 + 131 + 118 + 327 = 798; Best path takes the 2 lost: 329.
        const ColumnLayout l = hydra::ui::place_columns(
            m, kLibrarySpecs,
            room_of(1000.0f, {}, {std::nullopt, std::nullopt, std::nullopt, 200.0f, std::nullopt}));
        CHECK(l.widths == std::vector<float>{222.0f, 131.0f, 118.0f, 200.0f, 329.0f});
        CHECK(l.inner_width == 1000.0f);
    }
    SUBCASE("Artist dragged below its header stays at its header") {
        // Panel open, 1,000 px. Artist dragged to 20 sits at its header, 60.
        // Title and Best path split the 940 left by 17 and 25 of 42:
        //   Title 940 x 17 / 42 = 380.5 -> 380, Best path 940 x 25 / 42 =
        //   559.5 -> 559, plus the 1 lost pixel: 560. 380 + 60 + 560 = 1,000.
        const ColumnLayout l = hydra::ui::place_columns(
            m, kLibrarySpecs,
            room_of(1000.0f, kPanelOpen, {std::nullopt, 20.0f, std::nullopt, std::nullopt, std::nullopt}));
        CHECK(l.widths == std::vector<float>{380.0f, 60.0f, 70.0f, 60.0f, 560.0f});
        CHECK(l.inner_width == 1000.0f);
    }
}

TEST_CASE("column_widths: a dragged number column never goes below its widest") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    SUBCASE("dragged narrower: it stays at its widest, 50") {
        // As with no drag: Num 50, leftover 200, Song 120, Artist 80.
        const ColumnLayout l =
            hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {}, {}, {20.0f, {}, {}}});
        CHECK(l.widths == std::vector<float>{50.0f, 120.0f, 80.0f});
    }
    SUBCASE("dragged wider: it keeps the drag, 90") {
        // Num 90. Leftover 260 - 10 - 90 = 160: Song 60% = 96, Artist 40% = 64.
        const ColumnLayout l =
            hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {}, {}, {90.0f, {}, {}}});
        CHECK(l.widths == std::vector<float>{90.0f, 96.0f, 64.0f});
        CHECK(l.inner_width == 260.0f);
    }
}

TEST_CASE("column_widths: when the headers don't fit, the inner width is their sum") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Available 100, spacing 10: leftover 100 - 10 - 50 = 40. Song 40 x 60%
    // = 24 is below its 40 and Artist 16 below its 60, so both sit at their
    // headers. Inner width 50 + 40 + 60 + 10 = 160, more than 100: the table
    // scrolls.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{100.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 40.0f, 60.0f});
    CHECK(l.inner_width == 160.0f);
    CHECK(l.min_inner_width == 160.0f);
}

TEST_CASE("column_widths: a header drawn wider than its text (the sort arrow) lifts the floor") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Artist's header is drawn 80 wide: its text 60 plus 20 for the sort
    // arrow. Available 170, spacing 10: leftover 110. Song 110 x 60% = 66,
    // Artist 110 x 40% = 44, below its floor of 80, so Artist sits at 80.
    // Song gets the 30 left, below its own header's 40, so Song sits at 40.
    // Inner width 50 + 40 + 80 + 10 = 180, more than 170: scrolls.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kSpecs, TableRoom{170.0f, 10.0f, {}, {0.0f, 0.0f, 80.0f}});
    CHECK(l.widths == std::vector<float>{50.0f, 40.0f, 80.0f});
    CHECK(l.inner_width == 180.0f);
    CHECK(l.min_inner_width == 180.0f);
}

TEST_CASE("column_widths: a column that never cuts is at least as wide as its drawn header") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Num's cells are 50 and its text header 30, but it is drawn 70 wide.
    // Available 1000, spacing 10: Num 70, leftover 1000 - 10 - 70 = 920,
    // Song 60% = 552, Artist 40% = 368. Inner width 70 + 552 + 368 + 10 =
    // 1000. Minimum: 70 + 40 + 60 + 10 = 180. A drawn width under the text's
    // (Song's 20 against 40) changes nothing.
    const ColumnLayout l = hydra::ui::place_columns(
        m, kSpecs, TableRoom{1000.0f, 10.0f, {}, {70.0f, 20.0f, 0.0f}});
    CHECK(l.widths == std::vector<float>{70.0f, 552.0f, 368.0f});
    CHECK(l.inner_width == 1000.0f);
    CHECK(l.min_inner_width == 180.0f);
}

TEST_CASE("column_widths: table_room reads a sorted header's drawn width from the last frame") {
    HeadlessImGui imgui;

    TableRoom room;
    float plain_text = 0.0f;
    float sorted_text = 0.0f;
    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
        ImGui::Begin("column_widths test");
        // The headers' text alone, as the rule's own measure would take it.
        plain_text = std::ceil(hydra::ui::measure_in_font()("Plain"));
        sorted_text = std::ceil(hydra::ui::measure_in_font()("Sorted"));
        room = hydra::ui::table_room("##t", 0.0f, 2, 0);
        if (ImGui::BeginTable("##t", 2, hydra::ui::table_flags(), ImVec2(0.0f, 100.0f))) {
            ImGui::TableSetupColumn("Plain", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Sorted", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 100.0f);
            ImGui::TableHeadersRow();
            ImGui::EndTable();
        }
        ImGui::End();
        ImGui::EndFrame();
    }
    REQUIRE(room.header_drawn.size() == 2);
    // A sortable column keeps room for the arrow, sorted or not, and the
    // column that is sorted shows it. Both are wider than their text.
    CHECK(room.header_drawn[0] > plain_text);
    CHECK(room.header_drawn[1] > sorted_text);
}

TEST_CASE("column_widths: a table with no cut columns is as wide as its columns") {
    // "Points" header 60, cell "1234567" 70. "Ms" header 20, cell "12345" 50.
    // Spacing 10. Inner width 70 + 50 + 10 = 130 whatever the room.
    const std::vector<ColumnSpec> specs = {spec("Points", false), spec("Ms", false)};
    const Rows rows = {{"1234567", "12345"}};
    const MeasuredWidths m = hydra::ui::measure_widths(specs, 1, cells(rows));
    const ColumnLayout l = hydra::ui::place_columns(m, specs, TableRoom{500.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{70.0f, 50.0f});
    CHECK(l.inner_width == 130.0f);
    CHECK(l.min_inner_width == 130.0f);
}

TEST_CASE("column_widths: a hidden column takes no room") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Artist hidden. Available 260, spacing 10: leftover 200, all for Song,
    // the one shown text column. Inner width 50 + 200 + 10 = 260. Artist
    // keeps its measured width, its header's 60, for when it is shown again.
    // Minimum: 50 + 40 + 10 = 100.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {true, true, false}});
    CHECK(l.widths == std::vector<float>{50.0f, 200.0f, 60.0f});
    CHECK(l.inner_width == 260.0f);
    CHECK(l.min_inner_width == 100.0f);
}

namespace {

const std::vector<ColumnSpec> kApplySpecs = {text_spec("A", 30), text_spec("B", 40),
                                             text_spec("C", 30)};

ColumnLayout layout_of(std::vector<float> widths) {
    ColumnLayout l;
    l.widths = std::move(widths);
    return l;
}

// One frame of table "##a" in window "apply test" with the rule's widths
// `layout`. It reads the table's room with `reset_key` into `room`, if given,
// as a table does before BeginTable. `before_apply`, if given, runs after the
// column setup and just before apply_column_widths. Returns the table.
ImGuiTable* apply_frame(const ColumnLayout& layout,
                        const std::function<void(ImGuiTable*)>& before_apply = {},
                        std::uint64_t reset_key = 0, TableRoom* room = nullptr) {
    ImGuiTable* table = nullptr;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
    ImGui::Begin("apply test");
    const TableRoom read = hydra::ui::table_room("##a", 0.0f, 3, reset_key);
    if (room) *room = read;
    if (ImGui::BeginTable("##a", 3, hydra::ui::table_flags(), ImVec2(0.0f, 100.0f))) {
        for (int c = 0; c < 3; ++c) hydra::ui::setup_column(kApplySpecs[c], layout, c);
        table = ImGui::GetCurrentTable();
        if (before_apply) before_apply(table);
        hydra::ui::apply_column_widths(layout);
        ImGui::TableHeadersRow();
        ImGui::EndTable();
    }
    ImGui::End();
    ImGui::EndFrame();
    return table;
}

}  // namespace

TEST_CASE("column_widths: a column hidden from the start gets the rule's width the frame it shows") {
    HeadlessImGui imgui;
    // The saved settings hide column B at 90 px, so ImGui never lays it out
    // before it shows (as hydra_ui.ini did to the Library's Charter and
    // Folder). The table's id comes from a frame that opens only the window.
    ImGui::NewFrame();
    ImGui::Begin("apply test");
    const ImGuiID id = ImGui::GetID("##a");
    ImGui::End();
    ImGui::EndFrame();
    char ini[96];
    std::snprintf(ini, sizeof ini, "[Table][0x%08X,3]\nColumn 1 Width=90 Visible=0\n", id);
    ImGui::LoadIniSettingsFromMemory(ini);

    const ColumnLayout rule = layout_of({100.0f, 150.0f, 80.0f});
    ImGuiTable* table = nullptr;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(rule);
    REQUIRE(table != nullptr);
    REQUIRE_FALSE(table->Columns[1].IsEnabled);
    // The user shows it: the header menu asks for it the frame before.
    table = apply_frame(rule, [](ImGuiTable*) { ImGui::TableSetColumnEnabled(1, true); });
    CHECK(table->Columns[1].IsEnabled);
    CHECK(table->Columns[1].WidthGiven == 150.0f);
}

TEST_CASE("column_widths: a width ImGui changed by itself is put back") {
    HeadlessImGui imgui;
    const ColumnLayout rule = layout_of({100.0f, 150.0f, 80.0f});
    ImGuiTable* table = nullptr;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(rule);
    REQUIRE(table != nullptr);
    REQUIRE(table->Columns[0].WidthGiven == 100.0f);
    // "Size column to fit" in the header menu: ImGui fits column A to its
    // header "A", narrower than the rule's 100.
    ImGui::TableSetColumnWidthAutoSingle(table, 0);
    table = apply_frame(rule);
    REQUIRE(table->Columns[0].WidthGiven < 100.0f);
    table = apply_frame(rule);
    CHECK(table->Columns[0].WidthGiven == 100.0f);
}

TEST_CASE("column_widths: a column list's shares sum to 100, or it has no text column") {
    // Fake lists in the pretend font: 60 + 40 is well formed, 60 + 30 is not,
    // and a list of number columns alone has nothing to sum.
    CHECK(hydra::ui::text_shares_sum_to_100({spec("Num", false), text_spec("Song", 60), text_spec("Artist", 40)}));
    CHECK_FALSE(hydra::ui::text_shares_sum_to_100({text_spec("Song", 60), text_spec("Artist", 30)}));
    CHECK(hydra::ui::text_shares_sum_to_100({spec("Num", false), spec("Ms", false)}));
    CHECK(hydra::ui::text_shares_sum_to_100(kLibrarySpecs));
}

TEST_CASE("column_widths: the report views' real column lists have shares that sum to 100") {
    // The test builds are release builds, where place_columns' assert is off,
    // so the real lists are checked here.
    HeadlessImGui imgui;
    namespace report_frame = hydra::ui::report_frame;
    CHECK(hydra::ui::text_shares_sum_to_100(
        report_frame::column_specs(hydra::app::path_report_view::path_columns(0.0))));
    CHECK(hydra::ui::text_shares_sum_to_100(
        report_frame::column_specs(hydra::app::dm_report_view::dm_columns())));
}

TEST_CASE("column_widths: a drag pins the column until the table's reset key changes") {
    HeadlessImGui imgui;
    const ColumnLayout rule = layout_of({100.0f, 150.0f, 80.0f});
    ImGuiTable* table = nullptr;
    TableRoom room;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(rule, {}, 7, &room);
    REQUIRE(table != nullptr);
    // Nothing dragged yet: no pins.
    for (const std::optional<float>& pin : room.pinned) CHECK_FALSE(pin.has_value());
    // The user drags B's border to 200 and lets go: ImGui queues the width
    // the frame the mouse is held, exactly as its own border does.
    table->ResizedColumn = 1;
    table->ResizedColumnNextWidth = 200.0f;
    table->InstanceInteracted = 0;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(rule, {}, 7, &room);
    // The same rows: the drag is pinned at 200, and only B is.
    REQUIRE(room.pinned.size() == 3);
    CHECK_FALSE(room.pinned[0].has_value());
    CHECK(room.pinned[1] == std::optional<float>(200.0f));
    CHECK_FALSE(room.pinned[2].has_value());
    // The rows reload (a new reset key): the pin is gone, and stays gone.
    apply_frame(rule, {}, 8, &room);
    for (const std::optional<float>& pin : room.pinned) CHECK_FALSE(pin.has_value());
    apply_frame(rule, {}, 8, &room);
    for (const std::optional<float>& pin : room.pinned) CHECK_FALSE(pin.has_value());
}
