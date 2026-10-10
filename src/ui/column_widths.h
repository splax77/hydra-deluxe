// The one column-width rule every table in the app uses (plan:
// docs/superpowers/plans/2026-10-10-one-column-width-rule.md, Part 1).
//
// A column is as wide as the widest thing it shows: its header or its widest
// cell, in the cell's own font, with any per-cell padding. Columns that may
// cut their text share what is left after the others, and give way first when
// the window is narrow. Once each of them is down to its header's width, the
// table scrolls sideways instead.
//
// The rule has three steps, and only the last one touches ImGui:
//
//   1. Measure, once per change of rows or of UI scale. measure_widths()
//      reads every row; update_cells() redoes one column for a few rows.
//      The result is a MeasuredWidths the table keeps beside its rows.
//   2. Place, every frame. place_columns() turns the measure and the room
//      the table has into one width per column and the table's inner width.
//      It is plain arithmetic.
//   3. Apply, every frame. table_room() before BeginTable, setup_column()
//      in place of TableSetupColumn, and apply_column_widths() right after.
//
// A table's frame, in order:
//
//   if (cache.stale()) cache = measure_widths(specs, rows.size(), text);
//   const TableRoom room = table_room("##id", outer_size.x, specs.size());
//   const ColumnLayout layout = place_columns(cache, specs, room);
//   if (ImGui::BeginTable("##id", n, table_flags(), outer_size, layout.inner_width)) {
//       for (int c = 0; c < n; ++c) setup_column(specs[c], layout, c, extra_flags[c]);
//       apply_column_widths(layout);
//       ...headers and rows...
//   }
//
// Every width here is in whole pixels of the current UI scale.

#ifndef HYDRA_UI_COLUMN_WIDTHS_H
#define HYDRA_UI_COLUMN_WIDTHS_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "imgui.h"

namespace hydra::ui {

// A text's width in pixels, in the font a column draws its cells with.
using WidthOf = std::function<float(std::string_view)>;

// One column, as the table describes it to the rule.
struct ColumnSpec {
    std::string header;
    // True for a text column that may shrink and cut its cells with "…"
    // (Title, Artist, Song, Path...). Numbers and chips never cut.
    bool may_cut = false;
    // Extra width every cell adds beside its text: a chip's outline padding
    // (report_frame::column_spec sets it, from the same chip_pad the chip is
    // drawn with). The header does not get it.
    float padding = 0.0f;
    // Measures this column's header and cells. In the app, measure_in_font() with
    // the column's font; in a unit test, a fake.
    WidthOf width_of;
};

// Column `column`'s text for row `row`.
using CellText = std::function<std::string(std::size_t row, std::size_t column)>;

// What one measure found, kept beside the rows it measured.
struct MeasuredWidths {
    // Per column: the header's width or the widest cell's (padding
    // included), whichever is wider. ImGui's CellPadding is not in it.
    std::vector<float> widths;
    // Per column: the header's width alone. A cut column never goes below it.
    std::vector<float> header_widths;
    // Per column: the row whose cell is the widest. Empty when the header is
    // at least as wide as every cell (or there are no rows).
    std::vector<std::optional<std::size_t>> widest_row;
    // g_ui_scale (ui/fonts.h) when the measure was taken. Zero: never measured.
    float scale = 0.0f;

    // True when this measure has to be taken again: it never was, or the UI
    // scale has changed since (every width is in pixels of the current font).
    bool stale() const;
};

// Measures every row of every column. This is the slow step: one width_of
// call per cell. Call it when the rows are replaced or stale() says so.
MeasuredWidths measure_widths(const std::vector<ColumnSpec>& specs, std::size_t row_count,
                              const CellText& text);

// Brings one column of `measured` up to date after the text of `changed_rows`
// changed, and nothing else did (the same rows, the same scale). The result
// equals a fresh measure_widths. It measures only the changed cells, unless
// the row that held the column's widest cell got narrower; then it measures
// that one column again.
void update_cells(MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                  std::size_t column, const std::vector<std::size_t>& changed_rows,
                  std::size_t row_count, const CellText& text);

// The room a table has this frame, as ImGui will lay it out.
struct TableRoom {
    // The table's visible width: what its columns and ImGui's spacing between
    // and around them must fit in to need no horizontal scrollbar.
    float available = 0.0f;
    // ImGui's own width around and between the shown columns: cell padding
    // and spacing, for the shown columns only.
    float spacing = 0.0f;
    // Per column, whether ImGui shows it (the user can hide columns). Empty:
    // every column is shown. Hidden columns take no room.
    std::vector<bool> shown;
    // Per column, how wide ImGui drew the column's header last frame, with
    // the room it keeps for the sort arrow and sort-order digit, which text
    // alone does not give. place_columns never goes below it, so a sorted
    // column is never cut in its header. Empty, or 0 for a column: not known
    // yet (the table's first frame); the header's text width stands alone.
    std::vector<float> header_drawn;
};

// This frame's widths.
struct ColumnLayout {
    // One width per column, whole pixels. A hidden column keeps its measured
    // width, for when it comes back.
    std::vector<float> widths;
    // The width the table lays its shown columns out in: their widths plus
    // the spacing. More than room.available means the table scrolls sideways.
    float inner_width = 0.0f;
    // The inner width with every cut column at its header's width: the
    // narrowest the table gets before it scrolls. This is the whole table's
    // minimum. A report window's own minimum width is only its first five
    // columns (D103 item 14), so it does not read this; see
    // report_frame::note_min_size.
    float min_inner_width = 0.0f;
};

// Columns that may not cut get their measured width. Cut columns share what
// is left in proportion to their measured widths, each between its header's
// width and its measured width; one pinned at its header's width leaves the
// rest to the others. When even the headers do not fit, every cut column is
// at its header's width and the table scrolls. A header's width is its text
// or, when wider, room.header_drawn; a column is never narrower than it.
ColumnLayout place_columns(const MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                           const TableRoom& room);

// ---- The ImGui side -------------------------------------------------------

// The flags the rule needs from any table that goes through it. ScrollX lets
// the table scroll sideways when its columns don't fit; the rule sets every
// column's width itself, so ImGui's own sizing policy has nothing left to
// decide. A table with its own look (borders, no sorting) adds its own flags
// to these.
ImGuiTableFlags scroll_fixed_flags();

// The flags the Library and the report tables start from: scroll_fixed_flags
// plus the list look they share (sorting, hiding, resizing, row stripes).
ImGuiTableFlags table_flags();

// A WidthOf that measures in `font` at `size`, as ImGui would draw it. The
// one-argument form uses the current font size; with no font, the current
// font. Call it inside a frame; keep the result only for one measure, since
// a UI scale change changes the size.
WidthOf measure_in_font(ImFont* font, float size);
WidthOf measure_in_font(ImFont* font = nullptr);

// The room table `str_id` has this frame, for place_columns. Call it before
// BeginTable, in the window the table goes in, with the outer width that
// BeginTable gets (0 or less: ImGui's usual "the rest of the line" rule). It
// reads what ImGui knew of the table last frame: its spacing, its hidden
// columns, the width each header was drawn at and whether it showed a
// vertical scrollbar. Before the table's first frame it assumes no spacing,
// every column shown and no header wider than its text.
TableRoom table_room(const char* str_id, float outer_width, std::size_t column_count);

// The width the first `count` columns take with no room to share: each at its
// narrowest (a cut column at its header's width) plus ImGui's cell padding on
// both sides, every one counted as shown. Only `room.header_drawn` is read.
// The report window's minimum width asks for its first five columns (D103
// item 14).
float first_columns_min_width(const MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                              const TableRoom& room, std::size_t count);

// The outer height of a small table that shows its header and all `rows`
// rows, so it never scrolls up and down. A table that scrolls sideways is a
// child window, and an outer height of 0 would stretch it to the bottom of
// its box; pass this to BeginTable instead. Each row is one line high, or
// taller where `cell_heights[row]` (the height of that row's tallest wrapped
// cell; rows past the end of it, and an empty list, are one line) says so.
// When `layout` is wider than `room`, the horizontal scrollbar's height is
// added. Call it with the font the cells draw in pushed.
float table_outer_height(std::size_t rows, const ColumnLayout& layout, const TableRoom& room,
                         const std::vector<float>& cell_heights = {});

// How tall each row's tallest wrapped cell is, given this frame's layout (for
// table_outer_height's `cell_heights`).
using CellHeights = std::function<std::vector<float>(const ColumnLayout&)>;

// The whole frame of a small table whose rows are all at hand, up to and
// including BeginTable and the column setup: it measures `rows` rows now
// (they are a handful, so there is no cache to keep), reads the room, places
// the columns, opens table `str_id` at the height table_outer_height gives
// with `look` added to scroll_fixed_flags(), sets every column up and applies
// the widths. A table of more rows keeps its measure and runs the frame in
// the header comment by hand. Returns what BeginTable returned; when true, the
// caller draws the headers and rows and calls EndTable. `wrapped_heights`, if
// given, is called with the layout and returns table_outer_height's
// `cell_heights`. Call it with the font the cells draw in pushed.
bool begin_small_table(const char* str_id, ImGuiTableFlags look,
                       const std::vector<ColumnSpec>& specs, std::size_t rows,
                       const CellText& text, const CellHeights& wrapped_heights = {});

// TableSetupColumn for column `column`, as a fixed-width column at the
// rule's width. `flags` adds the table's own column flags (sort, hide); any
// sizing flag in it is replaced.
void setup_column(const ColumnSpec& spec, const ColumnLayout& layout, int column,
                  ImGuiTableColumnFlags flags = 0, ImGuiID user_id = 0);

// Hands the rule's widths to the current table. Call it right after the
// setup_column calls, before the first row. It waits until the table has
// settled (its first frame loads saved widths from hydra_ui.ini over
// anything set earlier), then sets each column whose rule width differs from
// the one it last set for that table. A column the user dragged keeps the
// drag until the rule's width for it changes. What it last set lives in the
// table's window state, never in hydra_ui.ini.
void apply_column_widths(const ColumnLayout& layout);

}  // namespace hydra::ui

#endif  // HYDRA_UI_COLUMN_WIDTHS_H
