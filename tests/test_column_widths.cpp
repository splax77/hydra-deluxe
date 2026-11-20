// Unit tests for the one column-width rule (ui/column_widths). The rule's
// arithmetic has no ImGui in it, so these feed it a pretend font: every
// character is 10 pixels wide. Every expected number is worked from the
// test's own inputs.

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

// Five text columns shaped like the Library's, in the pretend font. The shares
// are this file's own: the Library's real columns and shares are
// library_column_specs in src/ui/library_table.cpp. Headers: "Title" 50,
// "Artist" 60, "Charter" 70, "Folder" 60, "Best path" 90.
const std::vector<ColumnSpec> kLibrarySpecs = {text_spec("Title", 17), text_spec("Artist", 10),
                                               text_spec("Charter", 9), text_spec("Folder", 39),
                                               text_spec("Best path", 25)};
// The song panel open, in these five columns: Charter and Folder hidden. What
// the real panel hides is render_table's choice, in src/ui/library_table.cpp.
const std::vector<bool> kPanelOpen = {true, true, false, false, true};

// The five columns' places in kLibrarySpecs.
constexpr std::size_t kTitle = 0;
constexpr std::size_t kArtist = 1;
constexpr std::size_t kCharter = 2;
constexpr std::size_t kFolder = 3;
constexpr std::size_t kBestPath = 4;

// A kLibrarySpecs column's header in the pretend font, and its share.
float header_px(std::size_t c) { return ten_px(kLibrarySpecs[c].header); }
int share_of(std::size_t c) { return kLibrarySpecs[c].share; }

// The shares of the kLibrarySpecs columns `shown` keeps (all when empty).
int shown_shares(const std::vector<bool>& shown) {
    int total = 0;
    for (std::size_t c = 0; c < kLibrarySpecs.size(); ++c)
        if (shown.empty() || shown[c]) total += kLibrarySpecs[c].share;
    return total;
}

// What a text column gets of `room` pixels for `share` of `total` shares,
// in whole pixels rounded down, the way place_columns
// (src/ui/column_widths.cpp) splits the leftover.
float split_px(float room, int share, int total) {
    return std::floor(room * static_cast<float>(share) / static_cast<float>(total));
}

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
    const std::string header = "Artist";
    const Rows rows = {{"Abc"}, {"Ab"}};
    for (const auto& row : rows) REQUIRE(ten_px(row[0]) < ten_px(header));
    const MeasuredWidths m = hydra::ui::measure_widths({spec(header, false)}, 2, cells(rows));
    CHECK(m.widths == std::vector<float>{ten_px(header)});
    CHECK(m.header_widths == std::vector<float>{ten_px(header)});
    CHECK_FALSE(m.widest_row[0].has_value());
}

TEST_CASE("column_widths: a chip's padding is added to its cells, not its header") {
    const float padding = 8.0f;
    const std::string header = "T";
    const Rows rows = {{"Beyond"}, {"Hit"}};
    const MeasuredWidths m = hydra::ui::measure_widths({spec(header, false, padding)}, 2, cells(rows));
    const std::size_t widest = 0;  // "Beyond", wider than "Hit" and the header
    CHECK(m.widths == std::vector<float>{ten_px(rows[widest][0]) + padding});
    CHECK(m.header_widths == std::vector<float>{ten_px(header)});
    CHECK(m.widest_row[0] == widest);
}

TEST_CASE("column_widths: header_width_of measures the header, width_of the cells") {
    // The header gets a second pretend font, half as wide. Row 0's cell reads
    // the same as the header, yet measures in the cells' font.
    const auto five_px = [](std::string_view t) { return 5.0f * static_cast<float>(t.size()); };
    const std::string header = "Points";
    ColumnSpec s = spec(header, false);
    s.header_width_of = five_px;
    const Rows rows = {{header}, {"1234"}};
    const std::size_t widest = 0;  // the cell that reads as the header
    const MeasuredWidths m = hydra::ui::measure_widths({s}, 2, cells(rows));
    CHECK(m.header_widths == std::vector<float>{five_px(header)});
    CHECK(m.widths == std::vector<float>{ten_px(rows[widest][0])});
    CHECK(m.widest_row[0] == widest);

    // Without header_width_of, width_of measures the header too.
    const MeasuredWidths plain = hydra::ui::measure_widths({spec(header, false)}, 2, cells(rows));
    CHECK(plain.header_widths == std::vector<float>{ten_px(header)});
}

TEST_CASE("column_widths: a measure is stale until taken, and again after a scale change") {
    CHECK(MeasuredWidths{}.stale());
    const float before = hydra::ui::g_ui_scale;
    const float scale_measured_at = 1.0f;
    const float a_later_scale = 1.25f;
    hydra::ui::g_ui_scale = scale_measured_at;
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    CHECK(m.scale == scale_measured_at);
    CHECK_FALSE(m.stale());
    hydra::ui::g_ui_scale = a_later_scale;
    CHECK(m.stale());
    hydra::ui::g_ui_scale = before;
}

TEST_CASE("column_widths: five text columns, all shown, split the room by their shares") {
    // No spacing and no number column, so the shares split the whole room.
    const float room = 1000.0f;
    const int total = shown_shares({});
    std::vector<float> expected;
    for (std::size_t c = 0; c < kLibrarySpecs.size(); ++c) {
        expected.push_back(split_px(room, share_of(c), total));
        REQUIRE(expected[c] > header_px(c));  // no column stops at its header
    }
    const ColumnLayout l = hydra::ui::place_columns(library_measure(), kLibrarySpecs, room_of(room));
    CHECK(l.widths == expected);
    CHECK(l.inner_width == room);
}

TEST_CASE("column_widths: hidden columns' shares go to the shown ones, lost pixels to the last") {
    // kPanelOpen hides Charter and Folder; the shown shares split the whole
    // room. Best path is the last shown text column.
    const float room = 1000.0f;
    const int total = shown_shares(kPanelOpen);
    const float title = split_px(room, share_of(kTitle), total);
    const float artist = split_px(room, share_of(kArtist), total);
    const float best_path = split_px(room, share_of(kBestPath), total);
    const float lost = room - (title + artist + best_path);
    REQUIRE(lost > 0.0f);  // rounding down loses pixels in this room
    const ColumnLayout l =
        hydra::ui::place_columns(library_measure(), kLibrarySpecs, room_of(room, kPanelOpen));
    // The hidden columns keep their measured width, their header's.
    CHECK(l.widths == std::vector<float>{title, artist, header_px(kCharter), header_px(kFolder),
                                         best_path + lost});
    CHECK(l.inner_width == room);
}

TEST_CASE("column_widths: a text column whose split is below its header stops there") {
    const MeasuredWidths m = library_measure();
    const int total = shown_shares(kPanelOpen);
    SUBCASE("300 px: Artist stops at its header, the others split the rest") {
        // Panel open. On the first split only Artist falls below its header.
        const float room = 300.0f;
        REQUIRE(split_px(room, share_of(kTitle), total) >= header_px(kTitle));
        REQUIRE(split_px(room, share_of(kArtist), total) < header_px(kArtist));
        REQUIRE(split_px(room, share_of(kBestPath), total) >= header_px(kBestPath));
        // Artist sits at its header; Title and Best path split what is left by
        // their own shares, and the lost pixels go to Best path.
        const float left = room - header_px(kArtist);
        const int rest = share_of(kTitle) + share_of(kBestPath);
        const float title = split_px(left, share_of(kTitle), rest);
        const float best_path = split_px(left, share_of(kBestPath), rest);
        REQUIRE(title >= header_px(kTitle));
        REQUIRE(best_path >= header_px(kBestPath));
        const float lost = left - (title + best_path);
        const ColumnLayout l =
            hydra::ui::place_columns(m, kLibrarySpecs, room_of(room, kPanelOpen));
        CHECK(l.widths == std::vector<float>{title, header_px(kArtist), header_px(kCharter),
                                             header_px(kFolder), best_path + lost});
        CHECK(l.inner_width == room);
    }
    SUBCASE("150 px: every column at its header, and the table scrolls") {
        const float room = 150.0f;
        for (std::size_t c : {kTitle, kArtist, kBestPath})
            REQUIRE(split_px(room, share_of(c), total) < header_px(c));
        const float headers = header_px(kTitle) + header_px(kArtist) + header_px(kBestPath);
        const ColumnLayout l =
            hydra::ui::place_columns(m, kLibrarySpecs, room_of(room, kPanelOpen));
        CHECK(l.widths == std::vector<float>{header_px(kTitle), header_px(kArtist), header_px(kCharter),
                                             header_px(kFolder), header_px(kBestPath)});
        CHECK(l.inner_width == headers);
        CHECK(l.inner_width > room);
        CHECK(l.min_inner_width == headers);
    }
}

TEST_CASE("column_widths: a number column takes its widest and the text columns split the rest") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 260.0f;
    const float spacing = 10.0f;
    const float num = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    const float leftover = available - spacing - num;
    const int total = kSpecs[1].share + kSpecs[2].share;
    const float song = split_px(leftover, kSpecs[1].share, total);
    const float artist = split_px(leftover, kSpecs[2].share, total);
    REQUIRE(song + artist == leftover);  // no pixels lost to rounding here
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{available, spacing, {}});
    CHECK(l.widths == std::vector<float>{num, song, artist});
    CHECK(l.inner_width == num + song + artist + spacing);
    // The narrowest the table can be: the number column whole, each text
    // column at its header.
    CHECK(l.min_inner_width == num + ten_px(kSpecs[1].header) + ten_px(kSpecs[2].header) + spacing);
}

TEST_CASE("column_widths: the text in a text column does not change the layout") {
    // The same table twice: once with short cells, once with every cell far
    // wider than the room.
    const float room = 1000.0f;
    const Rows long_rows = {std::vector<std::string>(kLibrarySpecs.size(), std::string(200, 'a'))};
    REQUIRE(ten_px(long_rows[0][0]) > room);
    const MeasuredWidths short_m = library_measure();
    const MeasuredWidths long_m = library_measure(long_rows);
    CHECK(short_m.widths == long_m.widths);
    const ColumnLayout a = hydra::ui::place_columns(short_m, kLibrarySpecs, room_of(room));
    const ColumnLayout b = hydra::ui::place_columns(long_m, kLibrarySpecs, room_of(room));
    CHECK(a.widths == b.widths);
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
    // before it shows (as hydra_ui.ini can do to a Library column). The
    // table's id comes from a frame that opens only the window.
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
        report_frame::column_specs(hydra::app::path_report_view::path_columns())));
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
