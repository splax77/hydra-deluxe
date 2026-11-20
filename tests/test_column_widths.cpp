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

// Pins for `count` columns: column `c` dragged to `width`, the others not.
std::vector<std::optional<float>> dragged(std::size_t count, std::size_t c, float width) {
    std::vector<std::optional<float>> pins(count);
    pins[c] = width;
    return pins;
}

// The widths place_columns gives kSpecs when Num is `num` wide and Song and
// Artist split the rest of `available` by their shares, each at or above its
// header. The pixels lost to rounding go to Artist, the last text column.
std::vector<float> kspecs_split(float available, float spacing, float num) {
    const float leftover = available - spacing - num;
    const int total = kSpecs[1].share + kSpecs[2].share;
    const float song = split_px(leftover, kSpecs[1].share, total);
    const float artist = split_px(leftover, kSpecs[2].share, total);
    REQUIRE(song >= ten_px(kSpecs[1].header));
    REQUIRE(artist >= ten_px(kSpecs[2].header));
    return {num, song, artist + (leftover - (song + artist))};
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
    const float widest_text = ten_px(rows[widest][0]);
    CHECK(m.widths == std::vector<float>{widest_text + padding});
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
    const float song_header = ten_px(kSpecs[1].header);
    const float artist_header = ten_px(kSpecs[2].header);
    CHECK(l.min_inner_width == num + song_header + artist_header + spacing);
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
    const float room = 1000.0f;
    SUBCASE("Folder dragged wider than its header holds the drag") {
        // Panel closed. Folder holds its drag; the other four split what is
        // left by their own shares, and the lost pixels go to Best path.
        const float drag = 200.0f;
        REQUIRE(drag >= header_px(kFolder));
        const float left = room - drag;
        const int rest = shown_shares({}) - share_of(kFolder);
        std::vector<float> expected;
        float given = 0.0f;
        for (std::size_t c = 0; c < kLibrarySpecs.size(); ++c) {
            const float w = c == kFolder ? drag : split_px(left, share_of(c), rest);
            REQUIRE(w >= header_px(c));  // no column stops at its header
            if (c != kFolder) given += w;
            expected.push_back(w);
        }
        expected[kBestPath] += left - given;
        const ColumnLayout l = hydra::ui::place_columns(
            m, kLibrarySpecs, room_of(room, {}, dragged(kLibrarySpecs.size(), kFolder, drag)));
        CHECK(l.widths == expected);
        CHECK(l.inner_width == room);
    }
    SUBCASE("Artist dragged below its header stays at its header") {
        // Panel open. Artist sits at its header; Title and Best path split
        // what is left by their own shares, and the lost pixels go to Best
        // path. The hidden columns keep their measured width, their header's.
        const float drag = 20.0f;
        REQUIRE(drag < header_px(kArtist));
        const float left = room - header_px(kArtist);
        const int rest = share_of(kTitle) + share_of(kBestPath);
        const float title = split_px(left, share_of(kTitle), rest);
        const float best_path = split_px(left, share_of(kBestPath), rest);
        REQUIRE(title >= header_px(kTitle));
        REQUIRE(best_path >= header_px(kBestPath));
        const float lost = left - (title + best_path);
        const ColumnLayout l = hydra::ui::place_columns(
            m, kLibrarySpecs,
            room_of(room, kPanelOpen, dragged(kLibrarySpecs.size(), kArtist, drag)));
        CHECK(l.widths == std::vector<float>{title, header_px(kArtist), header_px(kCharter),
                                             header_px(kFolder), best_path + lost});
        CHECK(l.inner_width == room);
    }
}

TEST_CASE("column_widths: a dragged number column never goes below its widest") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 260.0f;
    const float spacing = 10.0f;
    const float num = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    SUBCASE("dragged narrower: it stays at its widest") {
        const float drag = 20.0f;
        REQUIRE(drag < num);
        const ColumnLayout l = hydra::ui::place_columns(
            m, kSpecs, TableRoom{available, spacing, {}, {}, dragged(kSpecs.size(), 0, drag)});
        CHECK(l.widths == kspecs_split(available, spacing, num));
        CHECK(l.inner_width == available);
    }
    SUBCASE("dragged wider: it keeps the drag") {
        const float drag = 90.0f;
        REQUIRE(drag > num);
        const ColumnLayout l = hydra::ui::place_columns(
            m, kSpecs, TableRoom{available, spacing, {}, {}, dragged(kSpecs.size(), 0, drag)});
        CHECK(l.widths == kspecs_split(available, spacing, drag));
        CHECK(l.inner_width == available);
    }
}

TEST_CASE("column_widths: when the headers don't fit, the inner width is their sum") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 100.0f;
    const float spacing = 10.0f;
    const float num = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    const float song_header = ten_px(kSpecs[1].header);
    const float artist_header = ten_px(kSpecs[2].header);
    // Both text columns' splits of the leftover fall below their headers.
    const float leftover = available - spacing - num;
    const int total = kSpecs[1].share + kSpecs[2].share;
    REQUIRE(split_px(leftover, kSpecs[1].share, total) < song_header);
    REQUIRE(split_px(leftover, kSpecs[2].share, total) < artist_header);
    const ColumnLayout l = hydra::ui::place_columns(m, kSpecs, TableRoom{available, spacing, {}});
    CHECK(l.widths == std::vector<float>{num, song_header, artist_header});
    CHECK(l.inner_width == num + song_header + artist_header + spacing);
    CHECK(l.inner_width > available);  // the table scrolls
}

TEST_CASE("column_widths: a header drawn wider than its text (the sort arrow) lifts the floor") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 240.0f;
    const float spacing = 10.0f;
    const float arrow = 20.0f;  // the room the sort arrow takes beside the text
    const float num = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    const float song_header = ten_px(kSpecs[1].header);
    const float artist_header = ten_px(kSpecs[2].header);
    const float artist_drawn = artist_header + arrow;
    // Artist's split clears its text but not its drawn header, so only the
    // drawn width stops it.
    const float leftover = available - spacing - num;
    const int total = kSpecs[1].share + kSpecs[2].share;
    const float artist_split = split_px(leftover, kSpecs[2].share, total);
    REQUIRE(artist_split >= artist_header);
    REQUIRE(artist_split < artist_drawn);
    // Artist sits at its drawn header, and Song, the one text column left,
    // takes the rest.
    const float song = leftover - artist_drawn;
    REQUIRE(song >= song_header);
    const ColumnLayout l = hydra::ui::place_columns(
        m, kSpecs, TableRoom{available, spacing, {}, {0.0f, 0.0f, artist_drawn}});
    CHECK(l.widths == std::vector<float>{num, song, artist_drawn});
    CHECK(l.inner_width == available);
    // The narrowest the table gets counts Artist at its drawn header too.
    CHECK(l.min_inner_width == num + song_header + artist_drawn + spacing);
}

TEST_CASE("column_widths: a column that never cuts is at least as wide as its drawn header") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 1000.0f;
    const float spacing = 10.0f;
    const float num_widest = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    const float song_header = ten_px(kSpecs[1].header);
    const float artist_header = ten_px(kSpecs[2].header);
    // Num is drawn wider than its widest cell; Song is drawn narrower than its
    // header's text, which must change nothing.
    const float num_drawn = num_widest + 20.0f;
    const float song_drawn = song_header - 20.0f;
    REQUIRE(num_drawn > num_widest);
    REQUIRE(song_drawn < song_header);
    const ColumnLayout l = hydra::ui::place_columns(
        m, kSpecs, TableRoom{available, spacing, {}, {num_drawn, song_drawn, 0.0f}});
    CHECK(l.widths == kspecs_split(available, spacing, num_drawn));
    CHECK(l.inner_width == available);
    CHECK(l.min_inner_width == num_drawn + song_header + artist_header + spacing);
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
    // Two number columns, each with a cell wider than its header.
    const std::vector<ColumnSpec> specs = {spec("Points", false), spec("Ms", false)};
    const Rows rows = {{"1234567", "12345"}};
    const float spacing = 10.0f;
    const float points = ten_px(rows[0][0]);
    const float ms = ten_px(rows[0][1]);
    REQUIRE(points > ten_px(specs[0].header));
    REQUIRE(ms > ten_px(specs[1].header));
    const float columns = points + ms + spacing;
    const MeasuredWidths m = hydra::ui::measure_widths(specs, rows.size(), cells(rows));
    // Less room than the columns, exactly their room, and far more.
    for (const float available : {columns / 2.0f, columns, columns * 4.0f}) {
        CAPTURE(available);
        const ColumnLayout l = hydra::ui::place_columns(m, specs, TableRoom{available, spacing, {}});
        CHECK(l.widths == std::vector<float>{points, ms});
        CHECK(l.inner_width == columns);
        CHECK(l.min_inner_width == columns);
    }
}

TEST_CASE("column_widths: a hidden column takes no room") {
    const MeasuredWidths m = hydra::ui::measure_widths(kSpecs, kRows.size(), cells(kRows));
    const float available = 260.0f;
    const float spacing = 10.0f;
    const float num = ten_px(kRows[0][0]);  // "12345", Num's widest cell
    const float song_header = ten_px(kSpecs[1].header);
    constexpr std::size_t kArtistColumn = 2;
    // Artist hidden: Song, the one shown text column, takes all the leftover.
    const float leftover = available - spacing - num;
    REQUIRE(leftover >= song_header);
    const ColumnLayout l =
        hydra::ui::place_columns(m, kSpecs, TableRoom{available, spacing, {true, true, false}});
    // Artist keeps its measured width, for when it is shown again.
    CHECK(l.widths == std::vector<float>{num, leftover, m.widths[kArtistColumn]});
    CHECK(l.inner_width == num + leftover + spacing);
    CHECK(l.min_inner_width == num + song_header + spacing);
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
    // Lists in the pretend font: 60 + 40 is well formed, 60 + 30 is not, and
    // a list of number columns alone has nothing to sum. The Library's real
    // list is checked in tests/ui/uitest_library.cpp, which reads the specs
    // the drawn table holds; library_column_specs itself is not public.
    CHECK(hydra::ui::text_shares_sum_to_100({spec("Num", false), text_spec("Song", 60), text_spec("Artist", 40)}));
    CHECK_FALSE(hydra::ui::text_shares_sum_to_100({text_spec("Song", 60), text_spec("Artist", 30)}));
    CHECK(hydra::ui::text_shares_sum_to_100({spec("Num", false), spec("Ms", false)}));
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
