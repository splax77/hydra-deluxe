// The one column-width rule. The interface and what each step promises are in
// column_widths.h; the design is Part 1 of
// docs/superpowers/plans/2026-10-10-one-column-width-rule.md.

#include "ui/column_widths.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "imgui_internal.h"  // ImGuiTable, TableFindByID, CalcItemSize

#include "ui/fonts.h"  // g_ui_scale

namespace hydra::ui {

namespace {

// One cell's width: its text plus the column's padding, in whole pixels so
// ImGui (which truncates a column's width) never cuts it.
float cell_width(const ColumnSpec& spec, const std::string& text) {
    return std::ceil(spec.width_of(text) + spec.padding);
}

// Measures one column over every row into `m`.
void measure_column(MeasuredWidths& m, const ColumnSpec& spec, std::size_t c,
                    std::size_t row_count, const CellText& text) {
    const float header = std::ceil(spec.width_of(spec.header));
    float widest = header;
    std::optional<std::size_t> widest_row;
    for (std::size_t r = 0; r < row_count; ++r) {
        const float w = cell_width(spec, text(r, c));
        if (w > widest) {
            widest = w;
            widest_row = r;
        }
    }
    m.widths[c] = widest;
    m.header_widths[c] = header;
    m.widest_row[c] = widest_row;
}

}  // namespace

bool MeasuredWidths::stale() const { return scale == 0.0f || scale != g_ui_scale; }

MeasuredWidths measure_widths(const std::vector<ColumnSpec>& specs, std::size_t row_count,
                              const CellText& text) {
    MeasuredWidths m;
    m.widths.resize(specs.size());
    m.header_widths.resize(specs.size());
    m.widest_row.resize(specs.size());
    for (std::size_t c = 0; c < specs.size(); ++c)
        measure_column(m, specs[c], c, row_count, text);
    m.scale = g_ui_scale;
    return m;
}

void update_cells(MeasuredWidths& m, const std::vector<ColumnSpec>& specs, std::size_t column,
                  const std::vector<std::size_t>& changed_rows, std::size_t row_count,
                  const CellText& text) {
    const ColumnSpec& spec = specs[column];
    // Set when the row holding the widest cell got narrower: then some
    // unchanged row may be the widest now, and only a measure of the whole
    // column can say which.
    bool remeasure = false;
    for (std::size_t r : changed_rows) {
        const float w = cell_width(spec, text(r, column));
        if (w > m.widths[column]) {
            // Wider than everything so far, including whatever was widest
            // before: the new widest, whatever else narrowed.
            m.widths[column] = w;
            m.widest_row[column] = r;
            remeasure = false;
        } else if (m.widest_row[column] == r && w < m.widths[column]) {
            remeasure = true;
        }
    }
    if (remeasure) measure_column(m, spec, column, row_count, text);
}

ColumnLayout place_columns(const MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                           const TableRoom& room) {
    const std::size_t n = specs.size();
    auto shown = [&](std::size_t c) { return room.shown.empty() || room.shown[c]; };

    // A header is as wide as its text, or as ImGui drew it last frame when
    // that is wider (the sort arrow and sort-order digit take room too). A
    // column is never narrower than its header.
    std::vector<float> header(n), wide(n);
    for (std::size_t c = 0; c < n; ++c) {
        header[c] = measured.header_widths[c];
        if (c < room.header_drawn.size()) header[c] = std::max(header[c], std::ceil(room.header_drawn[c]));
        wide[c] = std::max(measured.widths[c], header[c]);
    }

    ColumnLayout layout;
    layout.widths = wide;

    // What the columns that never cut take, and each cut column's floor.
    float fixed = 0.0f;
    float floors = 0.0f;
    std::vector<std::size_t> open;  // cut columns whose share is still open
    for (std::size_t c = 0; c < n; ++c) {
        if (!shown(c)) continue;
        if (specs[c].may_cut) {
            floors += header[c];
            open.push_back(c);
        } else {
            fixed += wide[c];
        }
    }

    // The cut columns share the leftover in proportion to their measured
    // widths. A column whose share falls below its header's width is pinned
    // there, which leaves less for the rest, so the share is worked out again
    // until no other column falls below its floor.
    float leftover = room.available - room.spacing - fixed;
    for (bool pinned = true; pinned && !open.empty();) {
        pinned = false;
        float total = 0.0f;
        for (std::size_t c : open) total += wide[c];
        // Every share in a pass comes from the same leftover and total.
        const float pass_leftover = leftover;
        for (auto it = open.begin(); it != open.end();) {
            const float share = total > 0.0f ? pass_leftover * wide[*it] / total : 0.0f;
            if (share < header[*it]) {
                layout.widths[*it] = header[*it];
                leftover -= header[*it];
                it = open.erase(it);
                pinned = true;
            } else {
                ++it;
            }
        }
        if (pinned) continue;
        for (std::size_t c : open)
            layout.widths[c] =
                total > 0.0f ? std::min(std::floor(leftover * wide[c] / total), wide[c]) : 0.0f;
    }

    float sum = 0.0f;
    for (std::size_t c = 0; c < n; ++c)
        if (shown(c)) sum += layout.widths[c];
    layout.inner_width = sum + room.spacing;
    layout.min_inner_width = fixed + floors + room.spacing;
    return layout;
}

// ---- The ImGui side -------------------------------------------------------

ImGuiTableFlags scroll_fixed_flags() {
    return ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit;
}

ImGuiTableFlags table_flags() {
    return scroll_fixed_flags() | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable |
           ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
           ImGuiTableFlags_BordersOuterH;
}

WidthOf measure_in_font(ImFont* font, float size) {
    return [font, size](std::string_view s) {
        if (s.empty()) return 0.0f;
        return font->CalcTextSizeA(size, FLT_MAX, 0.0f, s.data(), s.data() + s.size()).x;
    };
}

WidthOf measure_in_font(ImFont* font) {
    return measure_in_font(font ? font : ImGui::GetFont(), ImGui::GetFontSize());
}

TableRoom table_room(const char* str_id, float outer_width, std::size_t column_count) {
    TableRoom room;
    // BeginTable's own reading of an outer width of 0 or less.
    room.available =
        ImGui::CalcItemSize(ImVec2(outer_width, 0.0f), ImGui::GetContentRegionAvail().x, 0.0f).x;
    const ImGuiTable* table = ImGui::TableFindByID(ImGui::GetID(str_id));
    if (!table || table->ColumnsCount != static_cast<int>(column_count) || !table->InnerWindow)
        return room;

    // Last frame's vertical scrollbar takes its width from the columns.
    if (table->InnerWindow != table->OuterWindow)
        room.available -= table->InnerWindow->ScrollbarSizes.x;
    room.shown.resize(column_count);
    room.header_drawn.resize(column_count);
    int shown = 0;
    for (std::size_t c = 0; c < column_count; ++c) {
        const ImGuiTableColumn& column = table->Columns[static_cast<int>(c)];
        room.shown[c] = column.IsEnabled;
        if (room.shown[c]) ++shown;
        // The header's ideal width as TableHeader recorded it, text plus the
        // room for the sort arrow and digit; ImGui's own auto-fit reads the
        // same two numbers (TableGetColumnWidthAuto).
        room.header_drawn[c] = std::max(0.0f, column.ContentMaxXHeadersIdeal - column.WorkMinX);
    }
    // ImGui's spacing for that many columns, from its own numbers for this
    // table (TableUpdateLayout, imgui_tables.cpp).
    if (shown > 0)
        room.spacing = table->OuterPaddingX * 2.0f +
                       (table->CellSpacingX1 + table->CellSpacingX2) * static_cast<float>(shown - 1) +
                       table->CellPaddingX * 2.0f * static_cast<float>(shown);
    return room;
}

float table_outer_height(std::size_t rows, const ColumnLayout& layout, const TableRoom& room,
                        const std::vector<float>& cell_heights) {
    const ImGuiStyle& style = ImGui::GetStyle();
    const float line = ImGui::GetFontSize();
    const float padding = style.CellPadding.y * 2.0f;
    // The header row, then each row: a line, or its wrapped cell when taller
    // (TableGetHeaderRowHeight is a line plus the padding above and below).
    float height = line + padding;
    for (std::size_t r = 0; r < rows; ++r)
        height += (std::max)(line, r < cell_heights.size() ? cell_heights[r] : 0.0f) + padding;
    if (layout.inner_width > room.available) height += style.ScrollbarSize;
    return height;
}

void setup_column(const ColumnSpec& spec, const ColumnLayout& layout, int column,
                  ImGuiTableColumnFlags flags, ImGuiID user_id) {
    flags = (flags & ~ImGuiTableColumnFlags_WidthMask_) | ImGuiTableColumnFlags_WidthFixed;
    ImGui::TableSetupColumn(spec.header.c_str(), flags,
                            layout.widths[static_cast<std::size_t>(column)], user_id);
}

bool begin_small_table(const char* str_id, ImGuiTableFlags look,
                       const std::vector<ColumnSpec>& specs, std::size_t rows,
                       const CellText& text, const CellHeights& wrapped_heights) {
    const MeasuredWidths measured = measure_widths(specs, rows, text);
    const TableRoom room = table_room(str_id, 0.0f, specs.size());
    const ColumnLayout layout = place_columns(measured, specs, room);
    const std::vector<float> heights =
        wrapped_heights ? wrapped_heights(layout) : std::vector<float>{};
    if (!ImGui::BeginTable(str_id, static_cast<int>(specs.size()), look | scroll_fixed_flags(),
                           ImVec2(0.0f, table_outer_height(rows, layout, room, heights)),
                           layout.inner_width))
        return false;
    for (std::size_t c = 0; c < specs.size(); ++c)
        setup_column(specs[c], layout, static_cast<int>(c));
    apply_column_widths(layout);
    return true;
}

void apply_column_widths(const ColumnLayout& layout) {
    ImGuiTable* table = ImGui::GetCurrentTable();
    if (!table || table->IsInitializing) return;
    // The widths last set, kept in the table's own window state (never saved
    // to hydra_ui.ini), one key per column.
    ImGuiStorage& memory = table->InnerWindow->StateStorage;
    const ImGuiID seed = ImHashStr("column_widths", 0, table->ID);
    const int n = (std::min)(table->ColumnsCount, static_cast<int>(layout.widths.size()));
    for (int c = 0; c < n; ++c) {
        const float width = layout.widths[static_cast<std::size_t>(c)];
        const ImGuiID key = ImHashData(&c, sizeof(c), seed);
        if (memory.GetFloat(key, -1.0f) == width) continue;
        ImGui::TableSetColumnWidth(c, width);
        memory.SetFloat(key, width);
    }
}

}  // namespace hydra::ui
