// Unit tests for the one column-width rule (ui/column_widths). The rule's
// arithmetic has no ImGui in it, so these feed it a pretend font: every
// character is 10 pixels wide. Every expected number is worked by hand from
// the inputs in the comment beside it.

#include "doctest.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"  // ImGuiTable, for the apply_column_widths tests

#include "testdata_dirs.h"  // resource_root
#include "ui/app_shell.h"
#include "ui/column_widths.h"
#include "ui/fonts.h"

using hydra::ui::CellText;
using hydra::ui::ColumnLayout;
using hydra::ui::ColumnSpec;
using hydra::ui::MeasuredWidths;
using hydra::ui::TableRoom;

namespace {

// The pretend font: 10 pixels a character.
float ten_px(std::string_view s) { return 10.0f * static_cast<float>(s.size()); }

ColumnSpec spec(const std::string& header, bool may_cut, float padding = 0.0f) {
    return ColumnSpec{header, may_cut, padding, ten_px};
}

// A table given as rows of cells.
using Rows = std::vector<std::vector<std::string>>;
CellText cells(const Rows& rows) {
    return [&rows](std::size_t r, std::size_t c) { return rows[r][c]; };
}

void check_same(const MeasuredWidths& a, const MeasuredWidths& b) {
    CHECK(a.widths == b.widths);
    CHECK(a.header_widths == b.header_widths);
    CHECK(a.widest_row == b.widest_row);
    CHECK(a.usual_widths == b.usual_widths);
    CHECK(a.cell_widths == b.cell_widths);
}

// A headless ImGui context with the app's fonts, for driving real tables with
// no window or renderer. It shuts ImGui down when the test ends.
struct HeadlessImGui {
    HeadlessImGui() {
        hydra::ui::ImGuiSetupOptions opts;
        opts.dpi_scale = 1.0f;
        opts.ini_file = "-";
        opts.resource_dir = corpus::resource_root();
        hydra::ui::setup_imgui(opts);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(800.0f, 600.0f);
        io.DeltaTime = 1.0f / 60.0f;
        // No renderer is attached, so say the fonts need no texture upload.
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    }
    ~HeadlessImGui() { hydra::ui::shutdown_imgui(); }
};

// A column of `rows` cells: every cell `usual` characters long except the
// last, which is `widest` characters long.
std::vector<std::string> one_long(std::size_t rows, std::size_t usual, std::size_t widest) {
    std::vector<std::string> column(rows, std::string(usual, 'a'));
    column.back() = std::string(widest, 'a');
    return column;
}

// Rows from columns given side by side.
Rows side_by_side(const std::vector<std::vector<std::string>>& columns) {
    Rows rows(columns.front().size());
    for (std::size_t r = 0; r < rows.size(); ++r)
        for (const auto& column : columns) rows[r].push_back(column[r]);
    return rows;
}

// The Library's case in miniature: 201 rows, three cut columns.
//   "Title"     header 50.  Every cell 15 chars = 150.
//   "Artist"    header 60.  Every cell 11 chars = 110.
//   "Best path" header 90.  200 cells of 10 chars = 100, and the last row's
//               cell is 1,200 chars = 12,000, the outlier.
// The usual width (usual_of, column_widths.cpp, skips the one outlier) is
// 150, 110 and 100; the widest is 150, 110 and 12,000.
const std::vector<ColumnSpec> kOutlierSpecs = {spec("Title", true), spec("Artist", true),
                                               spec("Best path", true)};
Rows outlier_rows() {
    return side_by_side({one_long(201, 15, 15), one_long(201, 11, 11), one_long(201, 10, 1200)});
}

// The three-column table the placing tests share:
//   "Num"  never cuts. Header 3 chars = 30, widest cell "12345" = 50.
//   "Song" may cut.    Header 4 chars = 40, widest cell 20 chars = 200.
//   "Artist" may cut.  Header 6 chars = 60, widest cell 10 chars = 100.
const std::vector<ColumnSpec> kSpecs = {spec("Num", false), spec("Song", true),
                                        spec("Artist", true)};
const Rows kRows = {{"12345", "abcdefghijklmnopqrst", "abcdefghij"}, {"1", "ab", "a"}};

}  // namespace

TEST_CASE("column_widths: a header wider than every cell sets the width") {
    // Header "Artist" = 60; cells "Abc" = 30 and "Ab" = 20.
    const Rows rows = {{"Abc"}, {"Ab"}};
    const MeasuredWidths m = hydra::ui::measure_widths({spec("Artist", true)}, 2, cells(rows));
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

TEST_CASE("column_widths: cut columns share the leftover in proportion") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    REQUIRE(m.widths == std::vector<float>{50.0f, 200.0f, 100.0f});
    // Available 260, spacing 10: leftover 260 - 10 - 50 = 200 for Song and
    // Artist, whose measured widths total 300.
    // Song 200 * 200 / 300 = 133.3 -> 133. Artist 200 * 100 / 300 = 66.7 -> 66,
    // above its header's 60. Inner width 50 + 133 + 66 + 10 = 259.
    // Minimum: 50 + 40 + 60 + 10 = 160.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 133.0f, 66.0f});
    CHECK(l.inner_width == 259.0f);
    CHECK(l.min_inner_width == 160.0f);
}

TEST_CASE("column_widths: a cut column stops at its header's width and the rest share") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Available 170, spacing 10: leftover 110. First pass: Song
    // 110 * 200 / 300 = 73.3, Artist 110 * 100 / 300 = 36.7, below its 60.
    // Artist is pinned at 60, leaving 50 for Song alone: 50 * 200 / 200 = 50,
    // above its header's 40. Inner width 50 + 50 + 60 + 10 = 170.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{170.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 50.0f, 60.0f});
    CHECK(l.inner_width == 170.0f);
}

TEST_CASE("column_widths: when the minimums don't fit, the inner width is their sum") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Available 100, spacing 10: leftover 40. Song 40 * 200 / 300 = 26.7 is
    // below 40 and Artist 13.3 below 60, so both sit at their headers.
    // Inner width 50 + 40 + 60 + 10 = 160, more than 100: the table scrolls.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{100.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 40.0f, 60.0f});
    CHECK(l.inner_width == 160.0f);
    CHECK(l.min_inner_width == 160.0f);
}

TEST_CASE("column_widths: a header drawn wider than its text (the sort arrow) lifts the floor") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Artist's header is drawn 80 wide: its text 60 plus 20 for the sort
    // arrow. Available 170, spacing 10: leftover 110. Song 110 * 200 / 300 =
    // 73.3, Artist 110 * 100 / 300 = 36.7, below its floor of 80, so Artist
    // sits at 80. Song gets the 30 left, below its own header's 40, so Song
    // sits at 40. Inner width 50 + 40 + 80 + 10 = 180, more than 170: scrolls.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kSpecs, TableRoom{170.0f, 10.0f, {}, {0.0f, 0.0f, 80.0f}});
    CHECK(l.widths == std::vector<float>{50.0f, 40.0f, 80.0f});
    CHECK(l.inner_width == 180.0f);
    CHECK(l.min_inner_width == 180.0f);
}

TEST_CASE("column_widths: a column that never cuts is at least as wide as its drawn header") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Num's cells are 50 and its text header 30, but it is drawn 70 wide.
    // Available 1000, spacing 10: Num 70, Song 200, Artist 100.
    // Inner width 70 + 200 + 100 + 10 = 380. Minimum: 70 + 40 + 60 + 10 = 180.
    // A drawn width under the text's (Song's 20 against 40) changes nothing.
    const ColumnLayout l = hydra::ui::place_columns(
        m, kSpecs, TableRoom{1000.0f, 10.0f, {}, {70.0f, 20.0f, 0.0f}});
    CHECK(l.widths == std::vector<float>{70.0f, 200.0f, 100.0f});
    CHECK(l.inner_width == 380.0f);
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
        room = hydra::ui::table_room("##t", 0.0f, 2);
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

TEST_CASE("column_widths: with room to spare, cut columns stop at their measured width") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    // Available 1000, spacing 10: leftover 940, more than Song's 200 plus
    // Artist's 100. Inner width 50 + 200 + 100 + 10 = 360.
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{1000.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{50.0f, 200.0f, 100.0f});
    CHECK(l.inner_width == 360.0f);
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
    // Artist hidden. Available 260, spacing 10: leftover 200 all for Song,
    // capped at its measured 200. Inner width 50 + 200 + 10 = 260. Artist
    // keeps its measured 100 for when it is shown again.
    // Minimum: 50 + 40 + 10 = 100.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kSpecs, TableRoom{260.0f, 10.0f, {true, true, false}});
    CHECK(l.widths == std::vector<float>{50.0f, 200.0f, 100.0f});
    CHECK(l.inner_width == 260.0f);
    CHECK(l.min_inner_width == 100.0f);
}

TEST_CASE("column_widths: a one-cell update equals a full measure") {
    // One column, header "H" = 10. Rows "aaa" = 30, "aaaaa" = 50, "a" = 10:
    // the widest is row 1 at 50.
    int calls = 0;
    const std::vector<ColumnSpec> specs = {ColumnSpec{"H", false, 0.0f, [&](std::string_view s) {
                                                          ++calls;
                                                          return ten_px(s);
                                                      }}};
    Rows rows = {{"aaa"}, {"aaaaa"}, {"a"}};
    MeasuredWidths m = hydra::ui::measure_widths(specs, rows.size(), cells(rows));
    REQUIRE(m.widths == std::vector<float>{50.0f});
    REQUIRE(m.widest_row[0] == 1u);

    SUBCASE("a wider text becomes the widest, measuring only its cell") {
        // Row 2 becomes 7 characters = 70, wider than 50.
        rows[2][0] = "aaaaaaa";
        calls = 0;
        hydra::ui::update_cells(m, specs, 0, {2}, rows.size(), cells(rows));
        CHECK(calls == 1);
        CHECK(m.widths == std::vector<float>{70.0f});
        CHECK(m.widest_row[0] == 2u);
        check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
    }
    SUBCASE("a narrower text that wasn't the widest changes nothing") {
        // Row 0 becomes "aa" = 20; row 1 still holds 50.
        rows[0][0] = "aa";
        calls = 0;
        hydra::ui::update_cells(m, specs, 0, {0}, rows.size(), cells(rows));
        CHECK(calls == 1);
        CHECK(m.widths == std::vector<float>{50.0f});
        check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
    }
    SUBCASE("the widest got narrower: the widest is found again without measuring") {
        // Row 1 becomes "aa" = 20. Row 0's 30 is now the widest. The measure
        // keeps every cell's width, so only the changed cell is measured: one
        // call (the old rule measured the whole column again here, 5 calls).
        rows[1][0] = "aa";
        calls = 0;
        hydra::ui::update_cells(m, specs, 0, {1}, rows.size(), cells(rows));
        CHECK(calls == 1);
        CHECK(m.widths == std::vector<float>{30.0f});
        CHECK(m.widest_row[0] == 0u);
        check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
    }
    SUBCASE("the widest got narrower but another changed row beat it") {
        // Row 1 becomes "aa" = 20 and row 2 becomes 6 characters = 60, wider
        // than the old 50: row 2 is the widest, with no second measure.
        rows[1][0] = "aa";
        rows[2][0] = "aaaaaa";
        calls = 0;
        hydra::ui::update_cells(m, specs, 0, {1, 2}, rows.size(), cells(rows));
        CHECK(calls == 2);
        CHECK(m.widths == std::vector<float>{60.0f});
        CHECK(m.widest_row[0] == 2u);
        check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
    }
    SUBCASE("everything narrower than the header: the header is the widest") {
        // Header "H" = 10. Row 1 becomes "" = 0 and the others stay 30 and 10,
        // so row 0 at 30 is the widest after the second measure.
        rows[1][0] = "";
        hydra::ui::update_cells(m, specs, 0, {1}, rows.size(), cells(rows));
        check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
        CHECK(m.widths == std::vector<float>{30.0f});
    }
}

TEST_CASE("column_widths: one outlier row does not take the room from the other columns") {
    const Rows rows = outlier_rows();
    const MeasuredWidths m = hydra::ui::measure_widths(kOutlierSpecs, rows.size(), cells(rows));
    CHECK(m.usual_widths == std::vector<float>{150.0f, 110.0f, 100.0f});
    CHECK(m.widths == std::vector<float>{150.0f, 110.0f, 12000.0f});
    // Available 1000, spacing 10, no column that never cuts: leftover 990.
    // The usual widths 150 + 110 + 100 = 360 fit, so each gets its usual
    // width, and the 630 left goes to the one column still short of its
    // widest: Best path, 100 + 630 = 730. Inner width 990 + 10 = 1000.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kOutlierSpecs, TableRoom{1000.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{150.0f, 110.0f, 730.0f});
    CHECK(l.inner_width == 1000.0f);
    CHECK(l.min_inner_width == 210.0f);
}

TEST_CASE("column_widths: the usual width is the widest below 100 rows, and skips one outlier at 100") {
    // Header "H" = 10. 99 rows: 98 cells of 3 chars = 30 and one of 9 = 90.
    // Too few rows to skip any: the usual width is the widest, 90.
    const std::vector<ColumnSpec> specs = {spec("H", true)};
    const Rows ninety_nine = side_by_side({one_long(99, 3, 9)});
    CHECK(hydra::ui::measure_widths(specs, 99, cells(ninety_nine)).usual_widths ==
          std::vector<float>{90.0f});
    // 100 rows: 99 of 30 and one of 90. The one long row is skipped: 30.
    const Rows hundred = side_by_side({one_long(100, 3, 9)});
    CHECK(hydra::ui::measure_widths(specs, 100, cells(hundred)).usual_widths ==
          std::vector<float>{30.0f});
    // A header wider than the usual cell sets it: "Header" = 60.
    CHECK(hydra::ui::measure_widths({spec("Header", true)}, 100, cells(hundred)).usual_widths ==
          std::vector<float>{60.0f});
    // No rows: the header.
    CHECK(hydra::ui::measure_widths(specs, 0, cells(hundred)).usual_widths ==
          std::vector<float>{10.0f});
}

TEST_CASE("column_widths: when the usual widths don't fit, they are shared in proportion") {
    const Rows rows = outlier_rows();
    const MeasuredWidths m = hydra::ui::measure_widths(kOutlierSpecs, rows.size(), cells(rows));
    // Available 330, spacing 10: leftover 320, less than the usual 360.
    // First pass, in proportion to 150, 110 and 100 (total 360): Title
    // 133.3, Artist 97.8, Best path 88.9, below its header's 90, so Best path
    // is pinned at 90. Second pass, 230 left between 150 and 110 (total
    // 260): Title 132.7 -> 132, Artist 97.3 -> 97, both above their headers.
    // Inner width 132 + 97 + 90 + 10 = 329.
    const ColumnLayout l =
        hydra::ui::place_columns(m, kOutlierSpecs, TableRoom{330.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{132.0f, 97.0f, 90.0f});
    CHECK(l.inner_width == 329.0f);
}

TEST_CASE("column_widths: a column that reaches its widest hands the rest on") {
    // Two cut columns of 100 rows, headers "A" and "B" = 10 each.
    //   A: 99 cells of 10 chars = 100, one of 12 chars = 120. Usual 100
    //      (the one long row is skipped), widest 120.
    //   B: 99 cells of 10 chars = 100, one of 100 chars = 1000. Usual 100,
    //      widest 1000.
    // Available 410, spacing 10: leftover 400, 200 past the usual widths. A's
    // gap (20) is the smaller, so A gets all of it, 120, and B the 180 left,
    // 280. No room is left over: inner width 120 + 280 + 10 = 410.
    const std::vector<ColumnSpec> specs = {spec("A", true), spec("B", true)};
    const Rows rows = side_by_side({one_long(100, 10, 12), one_long(100, 10, 100)});
    const MeasuredWidths m = hydra::ui::measure_widths(specs, rows.size(), cells(rows));
    const ColumnLayout l = hydra::ui::place_columns(m, specs, TableRoom{410.0f, 10.0f, {}});
    CHECK(l.widths == std::vector<float>{120.0f, 280.0f});
    CHECK(l.inner_width == 410.0f);
}

TEST_CASE("column_widths: spare room grows the smallest gap first, ties in column order") {
    // Three cut columns of 100 rows, headers "X", "Y", "Z" = 10 each, spacing 0.
    //   X: usual 10 chars = 100, widest 13 chars = 130, gap 30.
    //   Y: usual 100, widest 11 chars = 110, gap 10.
    //   Z: usual 100, widest 110, gap 10, tied with Y but after it.
    const std::vector<ColumnSpec> specs = {spec("X", true), spec("Y", true), spec("Z", true)};
    const Rows rows =
        side_by_side({one_long(100, 10, 13), one_long(100, 10, 11), one_long(100, 10, 11)});
    const MeasuredWidths m = hydra::ui::measure_widths(specs, rows.size(), cells(rows));
    SUBCASE("15 to spare: Y fills its gap, Z gets the 5 left, X nothing") {
        const ColumnLayout l = hydra::ui::place_columns(m, specs, TableRoom{315.0f, 0.0f, {}});
        CHECK(l.widths == std::vector<float>{100.0f, 110.0f, 105.0f});
        CHECK(l.inner_width == 315.0f);
    }
    SUBCASE("45 to spare: Y and Z fill theirs, X gets the 25 left") {
        const ColumnLayout l = hydra::ui::place_columns(m, specs, TableRoom{345.0f, 0.0f, {}});
        CHECK(l.widths == std::vector<float>{125.0f, 110.0f, 110.0f});
        CHECK(l.inner_width == 345.0f);
    }
}

TEST_CASE("column_widths: the outlier row shrinking is an update that equals a full measure") {
    int calls = 0;
    std::vector<ColumnSpec> specs = kOutlierSpecs;
    specs[2].width_of = [&](std::string_view s) {
        ++calls;
        return ten_px(s);
    };
    Rows rows = outlier_rows();
    MeasuredWidths m = hydra::ui::measure_widths(specs, rows.size(), cells(rows));
    REQUIRE(m.widths[2] == 12000.0f);
    REQUIRE(m.widest_row[2] == 200u);
    // The outlier's text becomes "aaaaa" = 50. Every other row is 100, so
    // the widest is 100, first held by row 0. Only the changed cell is
    // measured: one call.
    rows[200][2] = "aaaaa";
    calls = 0;
    hydra::ui::update_cells(m, specs, 2, {200}, rows.size(), cells(rows));
    CHECK(calls == 1);
    CHECK(m.widths[2] == 100.0f);
    CHECK(m.widest_row[2] == 0u);
    check_same(m, hydra::ui::measure_widths(specs, rows.size(), cells(rows)));
}

namespace {

const std::vector<ColumnSpec> kApplySpecs = {spec("A", true), spec("B", true), spec("C", true)};

ColumnLayout layout_of(std::vector<float> widths) {
    ColumnLayout l;
    l.widths = std::move(widths);
    return l;
}

// One frame of table "##a" in window "apply test" with the rule's widths
// `layout`. `before_apply`, if given, runs after the column setup and just
// before apply_column_widths. Returns the table.
ImGuiTable* apply_frame(const ColumnLayout& layout,
                        const std::function<void(ImGuiTable*)>& before_apply = {}) {
    ImGuiTable* table = nullptr;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
    ImGui::Begin("apply test");
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

TEST_CASE("column_widths: a dragged column keeps the drag until the rule's width changes") {
    HeadlessImGui imgui;
    ImGuiTable* table = nullptr;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(layout_of({100.0f, 150.0f, 80.0f}));
    REQUIRE(table != nullptr);
    // The user drags B's border to 200 and lets go: ImGui queues the width
    // the frame the mouse is held, exactly as its own border does.
    table->ResizedColumn = 1;
    table->ResizedColumnNextWidth = 200.0f;
    table->InstanceInteracted = 0;
    for (int frame = 0; frame < 3; ++frame) table = apply_frame(layout_of({100.0f, 150.0f, 80.0f}));
    CHECK(table->Columns[1].WidthGiven == 200.0f);
    // The rule's width for B changes: the rule's width wins again.
    for (int frame = 0; frame < 2; ++frame) table = apply_frame(layout_of({100.0f, 170.0f, 80.0f}));
    CHECK(table->Columns[1].WidthGiven == 170.0f);
}

// Prints how long the three measures take on a Library-sized table, with the
// app's real font when one loads. It never fails on the numbers. Skip it with
// -tse=column_widths_timing.
TEST_CASE("column_widths timing: 40,000 rows" * doctest::test_suite("column_widths_timing")) {
    constexpr std::size_t kRowsCount = 40000;
    constexpr std::size_t kColumns = 5;
    // About 30 characters a cell: 25 to 35, letters cycling by row and column.
    std::vector<std::vector<std::string>> rows(kRowsCount, std::vector<std::string>(kColumns));
    for (std::size_t r = 0; r < kRowsCount; ++r)
        for (std::size_t c = 0; c < kColumns; ++c) {
            const std::size_t len = 25 + (r * 7 + c * 3) % 11;
            std::string s(len, ' ');
            for (std::size_t k = 0; k < len; ++k)
                s[k] = static_cast<char>('a' + (r * 7 + c * 13 + k * 5) % 26);
            rows[r][c] = std::move(s);
        }
    const CellText text = [&rows](std::size_t r, std::size_t c) { return rows[r][c]; };

    HeadlessImGui imgui;
    ImFont* font = ImGui::GetIO().Fonts->Fonts.Size > 0 ? ImGui::GetIO().Fonts->Fonts[0] : nullptr;
    const bool real = font != nullptr;
    const hydra::ui::WidthOf width_of =
        real ? hydra::ui::measure_in_font(font, font->LegacySize) : hydra::ui::WidthOf(ten_px);
    if (real) std::printf("column_widths timing: 'Best path' measures %.1f px\n", width_of("Best path"));
    std::vector<ColumnSpec> specs;
    for (const char* h : {"Title", "Artist", "Charter", "Folder", "Best path"})
        specs.push_back(ColumnSpec{h, true, 0.0f, width_of});

    using Clock = std::chrono::steady_clock;
    auto ms = [](Clock::duration d) {
        return std::chrono::duration<double, std::milli>(d).count();
    };

    const Clock::time_point t0 = Clock::now();
    MeasuredWidths m = hydra::ui::measure_widths(specs, kRowsCount, text);
    const double full_ms = ms(Clock::now() - t0);

    // One cell: a row that isn't the widest gets a new, shorter text.
    const std::size_t column = 4;
    const std::size_t other = m.widest_row[column] == 0u ? 1u : 0u;
    rows[other][column] = "short";
    const Clock::time_point t1 = Clock::now();
    hydra::ui::update_cells(m, specs, column, {other}, kRowsCount, text);
    const double cell_ms = ms(Clock::now() - t1);

    // The widest row gets narrower: the widest and usual widths are worked
    // out again from the kept cell widths, with one cell measured.
    REQUIRE(m.widest_row[column].has_value());
    rows[*m.widest_row[column]][column] = "short";
    const Clock::time_point t2 = Clock::now();
    hydra::ui::update_cells(m, specs, column, {*m.widest_row[column]}, kRowsCount, text);
    const double column_ms = ms(Clock::now() - t2);

    std::printf(
        "column_widths timing (%s, %zu rows x %zu columns):\n"
        "  full measure %.2f ms, widest-row update %.2f ms, one-cell update %.4f ms\n",
        real ? "real font, ImFont::CalcTextSizeA" : "no font loaded: 10 px fake", kRowsCount,
        kColumns, full_ms, column_ms, cell_ms);
}
