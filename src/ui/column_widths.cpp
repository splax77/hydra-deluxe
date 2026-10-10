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

    ColumnLayout layout;
    layout.widths = measured.widths;

    // What the columns that never cut take, and each cut column's floor.
    float fixed = 0.0f;
    float floors = 0.0f;
    std::vector<std::size_t> open;  // cut columns whose share is still open
    for (std::size_t c = 0; c < n; ++c) {
        if (!shown(c)) continue;
        if (specs[c].may_cut) {
            floors += measured.header_widths[c];
            open.push_back(c);
        } else {
            fixed += measured.widths[c];
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
        for (std::size_t c : open) total += measured.widths[c];
        // Every share in a pass comes from the same leftover and total.
        const float pass_leftover = leftover;
        for (auto it = open.begin(); it != open.end();) {
            const float share =
                total > 0.0f ? pass_leftover * measured.widths[*it] / total : 0.0f;
            if (share < measured.header_widths[*it]) {
                layout.widths[*it] = measured.header_widths[*it];
                leftover -= measured.header_widths[*it];
                it = open.erase(it);
                pinned = true;
            } else {
                ++it;
            }
        }
        if (pinned) continue;
        for (std::size_t c : open)
            layout.widths[c] = std::min(std::floor(leftover * measured.widths[c] / total),
                                        measured.widths[c]);
    }

    float sum = 0.0f;
    for (std::size_t c = 0; c < n; ++c)
        if (shown(c)) sum += layout.widths[c];
    layout.inner_width = sum + room.spacing;
    layout.min_inner_width = fixed + floors + room.spacing;
    return layout;
}

// ---- The ImGui side -------------------------------------------------------

ImGuiTableFlags table_flags() {
    return ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
           ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
           ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_SizingFixedFit;
}

WidthOf text_width(ImFont* font, float size) {
    return [font, size](std::string_view s) {
        if (s.empty()) return 0.0f;
        return font->CalcTextSizeA(size, FLT_MAX, 0.0f, s.data(), s.data() + s.size()).x;
    };
}

WidthOf text_width(ImFont* font) {
    return text_width(font ? font : ImGui::GetFont(), ImGui::GetFontSize());
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
    int shown = 0;
    for (std::size_t c = 0; c < column_count; ++c) {
        room.shown[c] = table->Columns[static_cast<int>(c)].IsEnabled;
        if (room.shown[c]) ++shown;
    }
    // ImGui's spacing for that many columns, from its own numbers for this
    // table (TableUpdateLayout, imgui_tables.cpp).
    if (shown > 0)
        room.spacing = table->OuterPaddingX * 2.0f +
                       (table->CellSpacingX1 + table->CellSpacingX2) * static_cast<float>(shown - 1) +
                       table->CellPaddingX * 2.0f * static_cast<float>(shown);
    return room;
}

void setup_column(const ColumnSpec& spec, const ColumnLayout& layout, int column,
                  ImGuiTableColumnFlags flags, ImGuiID user_id) {
    flags = (flags & ~ImGuiTableColumnFlags_WidthMask_) | ImGuiTableColumnFlags_WidthFixed;
    ImGui::TableSetupColumn(spec.header.c_str(), flags,
                            layout.widths[static_cast<std::size_t>(column)], user_id);
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
