// Unit tests for the one column-width rule (ui/column_widths). The rule's
// arithmetic has no ImGui in it, so these feed it a pretend font: every
// character is 10 pixels wide. Every expected number is worked by hand from
// the inputs in the comment beside it.

#include "doctest.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include "imgui.h"

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
    SUBCASE("the widest got narrower: the column is measured again") {
        // Row 1 becomes "aa" = 20. Row 0's 30 is now the widest. One call for
        // the changed cell, then the header and three rows: 5 calls.
        rows[1][0] = "aa";
        calls = 0;
        hydra::ui::update_cells(m, specs, 0, {1}, rows.size(), cells(rows));
        CHECK(calls == 5);
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

    hydra::ui::ImGuiSetupOptions opts;
    opts.dpi_scale = 1.0f;
    opts.ini_file = "-";
    opts.resource_dir = std::string(HYDRA_TESTDATA_DIR) + "/../resource";
    hydra::ui::setup_imgui(opts);
    ImFont* font = ImGui::GetIO().Fonts->Fonts.Size > 0 ? ImGui::GetIO().Fonts->Fonts[0] : nullptr;
    const bool real = font != nullptr;
    const hydra::ui::WidthOf width_of =
        real ? hydra::ui::text_width(font, font->LegacySize) : hydra::ui::WidthOf(ten_px);
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

    // One column: the widest row gets narrower, so the column is measured again.
    REQUIRE(m.widest_row[column].has_value());
    rows[*m.widest_row[column]][column] = "short";
    const Clock::time_point t2 = Clock::now();
    hydra::ui::update_cells(m, specs, column, {*m.widest_row[column]}, kRowsCount, text);
    const double column_ms = ms(Clock::now() - t2);

    std::printf(
        "column_widths timing (%s, %zu rows x %zu columns):\n"
        "  full measure %.2f ms, one-column measure %.2f ms, one-cell update %.4f ms\n",
        real ? "real font, ImFont::CalcTextSizeA" : "no font loaded: 10 px fake", kRowsCount,
        kColumns, full_ms, column_ms, cell_ms);
    hydra::ui::shutdown_imgui();
}
