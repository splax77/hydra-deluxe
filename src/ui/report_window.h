// The report windows (D103): the path report and the dmleaderboards
// comparison, each in its own OS window beside Hydra. Both are one frame
// (the header, notice strips, tiles, controls, count line, table, footer
// note and every state) filled with each report's rows. The frame draws what
// it is handed: the rows' order, filtering and count line come from the
// shared TableView (app/report_view.h), the columns and keep-rules from each
// report's view file, the tiles from path_tiles and dm_tiles, and the chip
// colours from chip_color. The two thin files that fill it in are
// path_report_window.cpp and dm_report_window.cpp.

#ifndef HYDRA_UI_REPORT_WINDOW_H
#define HYDRA_UI_REPORT_WINDOW_H

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/dm_report.h"
#include "app/report.h"
#include "app/report_view.h"
#include "core/model.h"  // group_thousands
#include "imgui.h"
#include "ui/report_state.h"  // ReportBuild, ReportOutOfDate
#include "ui/theme.h"
#include "ui/widgets.h"  // hint, overflow_tooltip, keep_table_column_order

namespace hydra::ui {

// What the window's buttons and rows hand back. An empty function leaves
// that control doing nothing.
struct ReportCallbacks {
    // A row was clicked, or picked with the arrow keys: its index into the
    // result's rows. Never called for a row the report says has no chart.
    std::function<void(size_t row)> row_click;
    std::function<void()> refresh;
    std::function<void()> cancel;
    std::function<void()> try_again;
    // The comparison's "Compare another player..." (both its header and its
    // failed state).
    std::function<void()> compare_another;
    // The window was closed: its title bar's X, Esc or Ctrl+W.
    std::function<void()> close;
};

// Everything a report window is drawn from. AppState fills it each frame.
template <class Result>
struct ReportWindowInput {
    // The last report built, or null when none is in memory.
    std::shared_ptr<const Result> result;
    ReportBuild state = ReportBuild::None;
    // When `result` was built, for "Built HH:MM".
    std::optional<std::chrono::system_clock::time_point> built;
    ReportOutOfDate out_of_date = ReportOutOfDate::None;
    // When the batch that changed the library finished, for the library
    // out-of-date line's "(a batch finished at HH:MM)". Unset: the line
    // names no time (D103 item 21).
    std::optional<std::chrono::system_clock::time_point> batch_finished;
    // The path report's progress while it builds: charts analyzed of all.
    // The comparison has no count.
    int progress_done = 0;
    int progress_total = 0;
    // A failed build's message and error, as the job gives them.
    std::string failure_message;
    std::string failure_error;
    // The comparison's player, which the title names even while that
    // player's scores are still fetching. Unused by the path report.
    std::string player;
    ReportCallbacks callbacks;
};

using PathReportInput = ReportWindowInput<app::report::GeneratedReport>;
using DmReportInput = ReportWindowInput<app::dm_report::GeneratedDmReport>;

// Draw each window for one frame while *open; the window clears *open when
// it closes. Call every frame, open or not.
void draw_path_report_window(bool* open, const PathReportInput& input);
void draw_dm_report_window(bool* open, const DmReportInput& input);

// Each window's input as AppState holds its report this frame: the slot's
// result and out-of-date reason, the job's state and progress, and callbacks
// into AppState. path_report_window.cpp and dm_report_window.cpp fill them;
// this header leaves AppState forward-declared.
class AppState;
PathReportInput path_report_input(AppState& app);
DmReportInput dm_report_input(AppState& app);

// ---- The shared frame, used by the two window files -----------------------

namespace report_frame {

// One window's words and state for this frame.
struct Frame {
    // The ImGui window name: the OS title, then a "###" id that keeps the
    // window's saved placement when the title changes.
    std::string window_name;
    // The heading's accent words after "Hydra".
    const char* heading = "";
    std::string subtitle;
    // The subtitle while the report builds (empty: none).
    const char* building_subtitle = "";
    std::optional<std::chrono::system_clock::time_point> built;
    ReportBuild state = ReportBuild::None;
    ReportOutOfDate out_of_date = ReportOutOfDate::None;
    std::optional<std::chrono::system_clock::time_point> batch_finished;
    // Building: the bar's count, or no count for a moving bar, and the lines
    // above it.
    std::optional<std::pair<int, int>> progress;
    std::vector<const char*> building_lines;
    // Failed: the report's own sentence, then the job's words.
    const char* failure_sentence = "";
    std::string failure_message;
    std::string failure_error;
    // Ready with no rows: what the report says instead.
    std::string empty_text;
    // The notice strip's sentence (empty: no strip), and the files it can
    // fold open.
    std::string notice;
    std::vector<std::string> notice_files;
    std::string footer;
    // Whether the window offers "Compare another player...".
    bool compare_another = false;
    const ReportCallbacks* callbacks = nullptr;
};

// The frame's fields that come straight from the window's input and its
// result. Each window adds its own words and the fields only it has.
template <class Result>
Frame frame_from(const ReportWindowInput<Result>& input) {
    Frame f;
    f.built = input.built;
    f.state = input.state;
    f.out_of_date = input.out_of_date;
    f.batch_finished = input.batch_finished;
    f.failure_message = input.failure_message;
    f.failure_error = input.failure_error;
    f.callbacks = &input.callbacks;
    if (input.result) {
        f.subtitle = input.result->subtitle;
        f.footer = input.result->footer;
    }
    return f;
}

// Whether `w` still points at the result `p` owns (an expired one never
// matches a new one, even at the same address).
template <class Result>
bool same_result(const std::weak_ptr<const Result>& w, const std::shared_ptr<const Result>& p) {
    return !w.owner_before(p) && !p.owner_before(w);
}

// What a window keeps between frames.
struct Memory {
    char search[256] = {};
    bool show_files = false;
    std::optional<size_t> selected;  // index into the rows
    bool sort_pushed = false;        // the view's sort was handed to the table
    bool visible = false;            // Begin drew the window this frame
    bool focused = false;
    bool was_open = false;
    // Measured this frame for the next one's minimum size (D103 item 14).
    float controls_w = 0.0f;
    float columns_w = 0.0f;
    float table_top = 0.0f;
    // Forget everything a built report set up: a new result starts fresh.
    void reset() {
        const bool open = was_open;
        *this = Memory{};
        was_open = open;
    }
};

// Begins the window and draws its header, strips and any state other than
// Ready with rows. True when the caller should draw the tiles, controls and
// table. Always pair with end().
bool begin(bool* open, const Frame& frame, Memory& memory);
void end(bool* open, const Frame& frame, Memory& memory);

// The tiles, wrapping onto another line in a narrow window.
void tiles(const std::vector<app::report::Tile>& tiles);

// The search box. True when its text changed.
bool search_box(Memory& memory, const char* hint);
// A dropdown of `labels`, at the width of its widest. True when `chosen`
// changed.
bool dropdown(const char* id, const std::vector<std::string>& labels, int& chosen);
// The count line, at the right of the controls row. Call it last on that row.
void count_line(Memory& memory, const std::string& text);
// "Nothing matches those filters." and Clear filters. True on a click.
bool nothing_matches();
// The height the footer note takes under the table.
float footer_height(const Frame& frame);

// One cell's text in its look, and an outlined chip in its token's colour.
void cell(const std::string& text, const app::report_view::CellLook& look,
          app::report_view::Tone tone, bool numeric);
void chip(const std::string& text, app::report::ChipToken token);

// The parts of the table that need ImGui's internals.
ImGuiTableFlags table_flags();
// The "#" column's width for `rows` rows.
float row_number_width(size_t rows);
// The gold bar at a best path's left edge, beside the row just drawn.
void best_bar();
// Hands the table a sort (table column index and direction, first key
// first), and reads back a header click's: nullopt when the headers didn't
// change it. A key a Shift+click adds past kMaxSortKeys (app/report_view.h)
// is dropped.
using HeaderSort = std::vector<std::pair<int, app::report_view::SortDir>>;
// Whether the current table is past its first frame.
bool table_settled();
void push_sort(const HeaderSort& sort);
std::optional<HeaderSort> header_sort();
// Records the first five columns' width and where the table starts, for the
// next frame's minimum size. Call inside the table, after its headers.
void measure_columns(Memory& memory, const std::vector<std::string>& titles);

// How each report marks its rows.
template <class Row>
struct RowLook {
    std::function<bool(const Row&)> best;                       // the gold bar
    std::function<app::report::ChipToken(const Row&)> chip;     // the chip column's token
    std::function<bool(const Row&)> clickable;                  // empty: every row
    const char* unclickable_hint = "";
};

// The table: the "#" column, then the view's columns, its rows in the view's
// order. Header clicks sort the view; a row click or an arrow key selects a
// row and calls row_click.
template <class Row>
void table(const char* id, Memory& memory, app::report_view::TableView<Row>& view,
           const RowLook<Row>& look, const ReportCallbacks& callbacks, float height) {
    using app::report_view::Column;
    using app::report_view::SortDir;
    using app::report_view::SortSpec;
    using app::report_view::Tone;
    const std::vector<Column<Row>>& columns = view.columns();
    const std::vector<Row>& rows = view.rows();
    const std::vector<size_t>& shown = view.visible();
    auto can_click = [&](const Row& r) { return !look.clickable || look.clickable(r); };
    auto pick = [&](size_t index) {
        if (!can_click(rows[index])) return false;
        memory.selected = index;
        if (callbacks.row_click) callbacks.row_click(index);
        return true;
    };

    // Up and Down move the selection, like a click on the next row.
    std::optional<size_t> scroll_to;
    if (memory.focused && !ImGui::GetIO().WantTextInput && !shown.empty()) {
        const int step = ImGui::IsKeyPressed(ImGuiKey_DownArrow) ? 1
                         : ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1
                                                                 : 0;
        if (step != 0) {
            const long count = static_cast<long>(shown.size());
            long k = step > 0 ? -1 : count;  // nothing selected: start at an end
            for (long n = 0; n < count; ++n)
                if (memory.selected && shown[static_cast<size_t>(n)] == *memory.selected) k = n;
            // Rows with no chart to select are passed over.
            for (k += step; k >= 0 && k < count; k += step)
                if (pick(shown[static_cast<size_t>(k)])) {
                    scroll_to = static_cast<size_t>(k);
                    break;
                }
        }
    }

    if (!ImGui::BeginTable(id, static_cast<int>(columns.size()) + 1, table_flags(),
                           ImVec2(0.0f, height)))
        return;
    ImGui::TableSetupScrollFreeze(1, 1);  // the "#" column and the header row
    ImGui::TableSetupColumn("#",
                            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort |
                                ImGuiTableColumnFlags_NoHide,
                            row_number_width(rows.size()));
    std::vector<std::string> titles{"#"};
    for (size_t c = 0; c < columns.size(); ++c) {
        const bool down = view.first_direction(columns[c].id) == SortDir::Descending;
        ImGui::TableSetupColumn(columns[c].title.c_str(),
                                ImGuiTableColumnFlags_WidthStretch |
                                    (down ? ImGuiTableColumnFlags_PreferSortDescending
                                          : ImGuiTableColumnFlags_PreferSortAscending));
        titles.push_back(columns[c].title);
    }
    keep_table_column_order();  // imgui#9519, as in the Library table

    // The table opens in the view's sort, whatever an earlier run left. A
    // table's first frame sets its own first sort after this, so the view's
    // goes in once that frame is over, and until then the headers' sort is
    // not read back.
    if (!memory.sort_pushed && table_settled()) {
        HeaderSort sort;
        for (const SortSpec& s : view.sort())
            for (size_t c = 0; c < columns.size(); ++c)
                if (columns[c].id == s.column) sort.push_back({static_cast<int>(c) + 1, s.dir});
        push_sort(sort);
        memory.sort_pushed = true;
    }
    if (std::optional<HeaderSort> sort = memory.sort_pushed ? header_sort() : std::nullopt) {
        std::vector<SortSpec> specs;
        for (const auto& [index, dir] : *sort)
            if (index >= 1) specs.push_back({columns[static_cast<size_t>(index) - 1].id, dir});
        view.set_sort(std::move(specs));
    }

    // The headers, each with its column's definition on hover.
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (int c = 0; c <= static_cast<int>(columns.size()); ++c) {
        if (!ImGui::TableSetColumnIndex(c)) continue;
        ImGui::TableHeader(titles[static_cast<size_t>(c)].c_str());
        if (c > 0 && !columns[static_cast<size_t>(c) - 1].definition.empty())
            overflow_tooltip(columns[static_cast<size_t>(c) - 1].definition.c_str());
    }
    measure_columns(memory, titles);

    const std::vector<size_t>& order = view.visible();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(order.size()));
    if (scroll_to) clipper.IncludeItemByIndex(static_cast<int>(*scroll_to));
    while (clipper.Step()) {
        for (int k = clipper.DisplayStart; k < clipper.DisplayEnd; ++k) {
            const size_t index = order[static_cast<size_t>(k)];
            const Row& row = rows[index];
            ImGui::TableNextRow();
            ImGui::PushID(static_cast<int>(index));

            // One Selectable spans the row, labeled with its row number
            // (right-aligned, dim), so the whole row is one click target.
            ImGui::TableSetColumnIndex(0);
            const std::string number = group_thousands(static_cast<int64_t>(k) + 1);
            move_to_right_edge(ImGui::CalcTextSize(number.c_str()).x);
            ImGui::PushStyleColor(ImGuiCol_Text, kDimTextColor);
            const bool clicked = ImGui::Selectable(
                number.c_str(), memory.selected == index,
                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
            ImGui::PopStyleColor();
            if (clicked) pick(index);
            if (!can_click(row)) hint(look.unclickable_hint);
            if (look.best && look.best(row)) best_bar();
            if (scroll_to && *scroll_to == static_cast<size_t>(k)) ImGui::SetScrollHereY(0.5f);

            for (size_t c = 0; c < columns.size(); ++c) {
                if (!ImGui::TableSetColumnIndex(static_cast<int>(c) + 1)) continue;
                const Column<Row>& col = columns[c];
                const std::string text = col.cell(row);
                if (col.look.chip && look.chip)
                    chip(text, look.chip(row));
                else
                    cell(text, col.look, col.tone ? col.tone(row) : Tone::Normal, col.numeric);
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
}

}  // namespace report_frame

}  // namespace hydra::ui

#endif  // HYDRA_UI_REPORT_WINDOW_H
