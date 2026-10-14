// The one column-width rule. The interface and what each step promises are in
// column_widths.h; the design is Change 1 of
// docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md.

#include "ui/column_widths.h"

#include <algorithm>
#include <cassert>
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

// Measures one column into `m`: its header, and for a column that never cuts,
// every row. A text column's width never follows its text, so its cells are
// not read.
void measure_column(MeasuredWidths& m, const ColumnSpec& spec, std::size_t c,
                    std::size_t row_count, const CellText& text) {
    const float header = std::ceil(spec.width_of(spec.header));
    m.header_widths[c] = header;
    float widest = header;
    std::optional<std::size_t> widest_row;
    if (!spec.may_cut) {
        for (std::size_t r = 0; r < row_count; ++r) {
            const float w = cell_width(spec, text(r, c));
            if (w > widest) {
                widest = w;
                widest_row = r;
            }
        }
    }
    m.widths[c] = widest;
    m.widest_row[c] = widest_row;
}

// The storage keys a table's pins and reset key live under, in its inner
// window's state (never saved to hydra_ui.ini). table_room reads them and
// apply_column_widths writes the pins.
ImGuiID pin_key(const ImGuiTable* table, int column) {
    const ImGuiID seed = ImHashStr("column_widths pinned", 0, table->ID);
    return ImHashData(&column, sizeof(column), seed);
}
ImGuiID reset_key_id(const ImGuiTable* table) {
    return ImHashStr("column_widths reset key", 0, table->ID);
}
// Not pinned.
constexpr float kNoPin = -1.0f;

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

bool text_shares_sum_to_100(const std::vector<ColumnSpec>& specs) {
    int total = 0;
    bool any_text = false;
    for (const ColumnSpec& s : specs)
        if (s.may_cut) {
            total += s.share;
            any_text = true;
        }
    return !any_text || total == 100;
}

ColumnLayout place_columns(const MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                           const TableRoom& room) {
    const std::size_t n = specs.size();
    auto shown = [&](std::size_t c) { return room.shown.empty() || room.shown[c]; };
    auto pin = [&](std::size_t c) {
        return c < room.pinned.size() ? room.pinned[c] : std::optional<float>{};
    };

    // A typo in a column list fails here (in a debug build) instead of quietly
    // skewing the table.
    assert(text_shares_sum_to_100(specs));

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

    // What the number columns and the dragged text columns take, and each
    // text column's floor. A dragged number column only ever widens: a
    // number is never cut. A dragged text column holds its drag, down to its
    // header.
    float fixed = 0.0f;
    float floors = 0.0f;
    float numbers = 0.0f;
    std::vector<std::size_t> open;  // text columns that split the leftover
    for (std::size_t c = 0; c < n; ++c) {
        if (!shown(c)) continue;
        const std::optional<float> dragged = pin(c);
        if (!specs[c].may_cut) {
            if (dragged) layout.widths[c] = std::max(std::floor(*dragged), wide[c]);
            fixed += layout.widths[c];
            numbers += layout.widths[c];
            continue;
        }
        floors += header[c];
        if (dragged) {
            layout.widths[c] = std::max(std::floor(*dragged), header[c]);
            fixed += layout.widths[c];
        } else {
            open.push_back(c);
        }
    }

    // The open text columns split the leftover by their shares. One whose
    // split falls below its header's width stops there, which leaves less
    // for the rest, so the split is worked out again until no other column
    // falls below its floor.
    float leftover = room.available - room.spacing - fixed;
    for (bool stopped = true; stopped && !open.empty();) {
        stopped = false;
        float total = 0.0f;
        for (std::size_t c : open) total += static_cast<float>(specs[c].share);
        // Every split in a pass comes from the same leftover and total.
        const float pass_leftover = leftover;
        auto split_of = [&](std::size_t c) {
            return pass_leftover * static_cast<float>(specs[c].share) / total;
        };
        for (auto it = open.begin(); it != open.end();) {
            // No share left to split by (total 0): the column keeps its floor.
            if (total <= 0.0f || split_of(*it) < header[*it]) {
                layout.widths[*it] = header[*it];
                leftover -= header[*it];
                it = open.erase(it);
                stopped = true;
            } else {
                ++it;
            }
        }
        if (stopped) continue;
        // Whole pixels, each rounded down; the pixels lost to rounding go to
        // the last open column, so the columns fill the room exactly.
        float given = 0.0f;
        for (std::size_t c : open) {
            layout.widths[c] = std::floor(split_of(c));
            given += layout.widths[c];
        }
        layout.widths[open.back()] += std::max(0.0f, std::floor(leftover) - given);
    }

    float sum = 0.0f;
    for (std::size_t c = 0; c < n; ++c)
        if (shown(c)) sum += layout.widths[c];
    layout.inner_width = sum + room.spacing;
    layout.min_inner_width = numbers + floors + room.spacing;
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

TableRoom table_room(const char* str_id, float outer_width, std::size_t column_count,
                     std::uint64_t reset_key) {
    TableRoom room;
    // BeginTable's own reading of an outer width of 0 or less.
    room.available =
        ImGui::CalcItemSize(ImVec2(outer_width, 0.0f), ImGui::GetContentRegionAvail().x, 0.0f).x;
    const ImGuiTable* table = ImGui::TableFindByID(ImGui::GetID(str_id));
    if (!table || table->ColumnsCount != static_cast<int>(column_count) || !table->InnerWindow)
        return room;

    // A new reset key means new rows: every drag is let go of.
    ImGuiStorage& memory = table->InnerWindow->StateStorage;
    const int key = static_cast<int>(ImHashData(&reset_key, sizeof(reset_key)));
    const bool reset = memory.GetInt(reset_key_id(table), 0) != key;
    if (reset) memory.SetInt(reset_key_id(table), key);

    // Last frame's vertical scrollbar takes its width from the columns.
    if (table->InnerWindow != table->OuterWindow)
        room.available -= table->InnerWindow->ScrollbarSizes.x;
    room.shown.resize(column_count);
    room.header_drawn.resize(column_count);
    room.pinned.resize(column_count);
    int shown = 0;
    for (std::size_t c = 0; c < column_count; ++c) {
        const ImGuiTableColumn& column = table->Columns[static_cast<int>(c)];
        room.shown[c] = column.IsEnabled;
        if (room.shown[c]) ++shown;
        // The header's ideal width as TableHeader recorded it, text plus the
        // room for the sort arrow and digit; ImGui's own auto-fit reads the
        // same two numbers (TableGetColumnWidthAuto).
        room.header_drawn[c] = std::max(0.0f, column.ContentMaxXHeadersIdeal - column.WorkMinX);
        const ImGuiID pinned = pin_key(table, static_cast<int>(c));
        if (reset) memory.SetFloat(pinned, kNoPin);
        const float drag = memory.GetFloat(pinned, kNoPin);
        if (drag >= 0.0f) room.pinned[c] = drag;
    }
    // ImGui's spacing for that many columns, from its own numbers for this
    // table (TableUpdateLayout, imgui_tables.cpp).
    if (shown > 0)
        room.spacing = table->OuterPaddingX * 2.0f +
                       (table->CellSpacingX1 + table->CellSpacingX2) * static_cast<float>(shown - 1) +
                       table->CellPaddingX * 2.0f * static_cast<float>(shown);
    return room;
}

float first_columns_min_width(const MeasuredWidths& measured, const std::vector<ColumnSpec>& specs,
                              const TableRoom& room, std::size_t count) {
    TableRoom none;
    none.header_drawn = room.header_drawn;
    const ColumnLayout narrowest = place_columns(measured, specs, none);
    const float padding = ImGui::GetStyle().CellPadding.x * 2.0f;
    float width = 0.0f;
    for (std::size_t c = 0; c < narrowest.widths.size() && c < count; ++c)
        width += narrowest.widths[c] + padding;
    return width;
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

void draw_table_headers(const std::vector<ColumnSpec>& specs,
                        const std::function<void(std::size_t column)>& after_header) {
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (std::size_t c = 0; c < specs.size(); ++c) {
        if (!ImGui::TableSetColumnIndex(static_cast<int>(c))) continue;
        ImGui::TableHeader(specs[c].header.c_str());
        if (after_header) after_header(c);
    }
}

bool begin_small_table(const char* str_id, ImGuiTableFlags look,
                       const std::vector<ColumnSpec>& specs, std::size_t rows,
                       const CellText& text, const CellHeights& wrapped_heights) {
    const MeasuredWidths measured = measure_widths(specs, rows, text);
    // The rows are at hand, so the reset key is what they say: a drag holds
    // while the table shows the same text.
    std::uint64_t reset_key = rows;
    for (std::size_t r = 0; r < rows; ++r)
        for (std::size_t c = 0; c < specs.size(); ++c) {
            const std::string cell = text(r, c);
            reset_key = ImHashData(cell.data(), cell.size(), static_cast<ImGuiID>(reset_key));
        }
    const TableRoom room = table_room(str_id, 0.0f, specs.size(), reset_key);
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

bool table_settled() {
    const ImGuiTable* table = ImGui::GetCurrentTable();
    return table && !table->IsInitializing;
}

void apply_column_widths(const ColumnLayout& layout) {
    if (!table_settled()) return;
    ImGuiTable* table = ImGui::GetCurrentTable();
    ImGuiStorage& memory = table->InnerWindow->StateStorage;
    const int n = (std::min)(table->ColumnsCount, static_cast<int>(layout.widths.size()));
    for (int c = 0; c < n; ++c) {
        ImGuiTableColumn& column = table->Columns[c];
        // A drag in progress, or one let go of last frame. A drag in progress
        // has its width applied by ImGui inside the layout
        // (TableApplyQueuedRequests), after this call; a drag let go of is
        // already in the column's request. Either way the drag wins this
        // frame, and its width becomes the column's pin, which table_room
        // hands place_columns from the next frame on.
        if (table->ResizedColumn == c || table->LastResizedColumn == c) {
            memory.SetFloat(pin_key(table, c), column.WidthRequest);
            continue;
        }
        const float request = (std::max)(layout.widths[static_cast<std::size_t>(c)],
                                          table->MinColumnWidth);
        // Anything else that changed the width (ImGui's own auto-fit, a UI
        // scale change rescaling saved widths) is put back.
        if (column.WidthRequest == request) continue;
        // The request is set directly, with no clamp. ImGui's setter
        // (TableSetColumnWidth) clamps to the column's last laid-out maximum,
        // which a hidden column never refreshes (zero if it was hidden since
        // the table was made), and the layout clamps what it gives anyway. The
        // columns are all fixed width, so the setter does no more than this.
        column.WidthRequest = request;
        table->IsSettingsDirty = true;
    }
}

}  // namespace hydra::ui
