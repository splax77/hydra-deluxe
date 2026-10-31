// The report windows (D103): the path report and the dmleaderboards
// comparison, each in its own OS window beside Hydra. Both are one frame
// (the header, the out-of-date strip, tiles, controls, count line, table, footer
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
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "app/dm_report.h"
#include "app/report.h"
#include "app/report_view.h"
#include "core/model.h"  // group_thousands
#include "imgui.h"
#include "ui/report_state.h"  // ReportBuild, ReportOutOfDate
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/column_widths.h"
#include "ui/list_scroll.h"
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
    // The path report's Analyze, while its window asks whether to analyze.
    std::function<void()> analyze;
    std::function<void()> try_again;
    // The comparison's "Compare another player..." (both its header and its
    // failed state).
    std::function<void()> compare_another;
    // The window was closed: its title bar's X, its Close button, Esc or
    // Ctrl+W.
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
    // The path report only: why analysis is off (AppState::analysis_off_lines,
    // the toolbar's own lines), shown when no report is in memory. Empty when
    // analysis is on.
    std::vector<AnalysisOffLine> analysis_off;
    // The path report only: `result` was built because the window opened
    // with nothing in memory (ReportSlot::built_on_open), so the window keeps
    // its search, timing, Optimal paths only and sort when the rows arrive.
    bool keep_filters = false;
    // The path report only: the batch its build waits on (AppState::
    // path_report_batch), as that batch's snapshot gives it, so the window's
    // numbers are the strip's. Unset when it waits on no batch.
    struct WaitedBatch {
        bool preparing = true;  // BatchJob::Snapshot::preparing
        int completed = 0;      // BatchJob::Snapshot::completed
        int total = 0;          // BatchJob::Snapshot::total
    };
    std::optional<WaitedBatch> batch;
    // The path report only: its build waits for the user to answer whether
    // to analyze (AppState::path_report_awaits_confirm). The question is
    // AppState::batch_question's; empty when the window asks nothing.
    std::string confirm_question;
    // Whether the question's Analyze can start the batch now
    // (AppState::can_analyze).
    bool can_analyze = false;
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

// The input's fields that come straight from a report's slot in AppState
// (ReportSlot, ui/app_state.h) and its build state. Both window inputs start
// here; each adds its own progress, player and callbacks.
template <template <class> class Slot, class Result>
ReportWindowInput<Result> input_from_slot(const Slot<Result>& slot, ReportBuild state) {
    ReportWindowInput<Result> in;
    in.result = slot.result;
    in.state = state;
    if (slot.result) in.built = slot.built_at;
    in.out_of_date = slot.out_of_date;
    in.batch_finished = slot.batch_finished;
    in.failure_message = slot.message;
    in.failure_error = slot.error;
    return in;
}

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
    // Building: the lines above the bar, the first one plain and the rest
    // dim. With `progress` (done, total) the bar fills to that share;
    // without it, progress_bar draws a moving bar and false draws none (the
    // path report's build, which only reads the store).
    std::vector<std::string> building_lines;
    std::optional<std::pair<int, int>> progress;
    bool progress_bar = true;
    // Building: grey rows where the table will be (mock board 2a).
    bool placeholder_rows = false;
    // Building: an Analyze button beside Cancel (the path report's
    // question), greyed unless analyze_enabled.
    bool analyze_button = false;
    bool analyze_enabled = false;
    // Never built: why no build can start (empty: the header alone), as the
    // input gives them.
    std::vector<AnalysisOffLine> analysis_off;
    // Failed: the report's own sentence, then the job's words.
    const char* failure_sentence = "";
    std::string failure_message;
    std::string failure_error;
    // Ready with no rows: what the report says instead.
    std::string empty_text;
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

// Each table cell's text as it was last fitted to its column, so a frame that
// draws a cell at the width it had before cuts nothing again. Fitting every
// cell on every frame was a large share of the open window's redraw. A cell
// is its row's index into the result's rows plus its column's index, so a
// sort or a filter keeps every entry. The cut itself is the `fit` handed in;
// the window hands the one cut rule (render::ellipsize), so no rule lives here.
class FittedTextCache {
public:
    using Fit = std::function<std::string(const std::string& text, float width)>;

    // What cell (row, column) shows for `text` at `width`: the stored fit when
    // the cell was last fitted from the same text at the same width, otherwise
    // fit(text, width), kept for the next frame.
    const std::string& fitted(size_t row, size_t column, const std::string& text, float width,
                              const Fit& fit) {
        Entry& e = entries_[key(row, column)];
        if (!e.fitted || e.width != width || e.text != text) {
            e.shown = fit(text, width);
            e.text = text;
            e.width = width;
            e.fitted = true;
        }
        return e.shown;
    }
    void clear() { entries_.clear(); }
    size_t size() const { return entries_.size(); }

private:
    struct Entry {
        bool fitted = false;
        float width = 0.0f;
        std::string text;
        std::string shown;
    };
    static uint64_t key(size_t row, size_t column) {
        return (static_cast<uint64_t>(row) << 32) | static_cast<uint64_t>(column);
    }
    std::unordered_map<uint64_t, Entry> entries_;
};

// What a window keeps between frames.
struct Memory {
    char search[256] = {};
    std::optional<size_t> selected;  // index into the rows
    bool sort_pushed = false;        // the view's sort was handed to the table
    bool visible = false;            // Begin drew the window this frame
    bool focused = false;
    bool was_open = false;
    // Measured this frame for the next one's minimum size (D103 item 14).
    float controls_w = 0.0f;
    float columns_w = 0.0f;
    float table_top = 0.0f;
    // The table's cells as last fitted; a new result or a new measure drops them.
    FittedTextCache fitted;
    // The table's reset key for the width rule (table_room in
    // ui/column_widths.h): it moves on with each new result (reset) and each
    // changed search (search_changed), so a dragged column holds until then.
    std::uint64_t rows_key = 0;
    // The search text changed: the rows will, so a drag lets go. search_box
    // calls this for a typed change; a window that empties the search on
    // "Clear filters" calls clear_search.
    void search_changed() { ++rows_key; }
    void clear_search() {
        search[0] = '\0';
        search_changed();
    }
    // Forget everything a built report set up: a new result starts fresh.
    void reset() {
        const bool open = was_open;
        const std::uint64_t key = rows_key;
        *this = Memory{};
        was_open = open;
        rows_key = key + 1;
    }
};

// Begins the window and draws its header, strips and any state other than
// Ready with rows. True when the caller should draw the tiles, controls and
// table. Always pair with end().
bool begin(bool* open, const Frame& frame, Memory& memory);
void end(bool* open, const Frame& frame, Memory& memory);

// The tiles, wrapping onto another line in a narrow window.
void tiles(const std::vector<app::report::Tile>& tiles);

// The search box. True when its text changed, which also moves the table's
// reset key on (Memory::search_changed).
bool search_box(Memory& memory, const char* hint);
// A dropdown of `labels`, at the width of its widest. True when `chosen`
// changed.
bool dropdown(const char* id, const std::vector<std::string>& labels, int& chosen);
// The count line, right after the controls row's last item. Call it last on
// that row.
void count_line(Memory& memory, const std::string& text);
// The empty state: kNothingMatches (app/html_page.h) and Clear filters. True
// on a click.
bool nothing_matches();
// The height the footer note takes under the table.
float footer_height(const Frame& frame);

// One cell's text in its look, and an outlined chip in its token's colour.
// The cell is (row, column) in `fits`: its row's index into the result's
// rows and its column's index.
void cell(const std::string& text, const app::report_view::CellLook& look,
          app::report_view::Tone tone, FittedTextCache& fits, size_t row, size_t column);
void chip(const std::string& text, app::report::ChipToken token);

// ---- The columns, as the width rule (ui/column_widths.h) sees them ---------

// The "#" column's text for the row drawn `k`th (from 0).
std::string row_number(size_t k);
// The "#" column, and one of the view's columns drawn in `look`.
ColumnSpec row_number_spec();
ColumnSpec column_spec(const std::string& title, const app::report_view::CellLook& look);

// The table's columns for the rule: the "#" column, then the view's.
template <class Row>
std::vector<ColumnSpec> column_specs(const std::vector<app::report_view::Column<Row>>& columns) {
    std::vector<ColumnSpec> specs{row_number_spec()};
    for (const app::report_view::Column<Row>& c : columns) specs.push_back(column_spec(c.title, c.look));
    return specs;
}

// Every row of `view` measured for the rule, the filtered-out rows too, so
// the columns keep their widths while the user searches. Call it inside a
// frame, when the view is made and when the measure goes stale.
template <class Row>
MeasuredWidths measure(const app::report_view::TableView<Row>& view) {
    const std::vector<app::report_view::Column<Row>>& columns = view.columns();
    const std::vector<Row>& rows = view.rows();
    return measure_widths(column_specs(columns), rows.size(), [&](size_t r, size_t c) {
        return c == 0 ? row_number(r) : columns[c - 1].cell(rows[r]);
    });
}

// The parts of the table that need ImGui's internals.
// Hands the table a sort (table column index and direction, first key
// first), and reads back a header click's: nullopt when the headers didn't
// change it. A key a Shift+click adds past kMaxSortKeys (app/report_view.h)
// is dropped.
using HeaderSort = std::vector<std::pair<int, app::report_view::SortDir>>;
void push_sort(const HeaderSort& sort);
std::optional<HeaderSort> header_sort();
// Records the first five columns' width, each at the narrowest the width rule
// gives it in `room`, and where the table starts, for the next frame's
// minimum size (D103 item 14). Call inside the table, after its headers.
void note_min_size(Memory& memory, const MeasuredWidths& widths,
                   const std::vector<ColumnSpec>& specs, const TableRoom& room);

// How each report marks its rows.
template <class Row>
struct RowLook {
    std::function<app::report::ChipToken(const Row&)> chip;     // the chip column's token
    std::function<bool(const Row&)> clickable;                  // empty: every row
    const char* unclickable_hint = "";
};

// The table: the "#" column, then the view's columns, its rows in the view's
// order, each column as wide as the width rule (ui/column_widths.h) says.
// `widths` is the view's measure, kept beside it by the window; it is taken
// again here when it has gone stale. Header clicks sort the view; a row click
// or an arrow key selects a row and calls row_click.
template <class Row>
void table(const char* id, Memory& memory, app::report_view::TableView<Row>& view,
           MeasuredWidths& widths, const RowLook<Row>& look, const ReportCallbacks& callbacks,
           float height) {
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

    const std::vector<ColumnSpec> specs = column_specs(columns);
    // A stale measure can mean the UI scale changed, which leaves every fit
    // in old pixels, so the fits go with it.
    if (widths.stale()) {
        widths = measure(view);
        memory.fitted.clear();
    }
    // A dragged column holds until a new result or a changed search
    // (Memory::rows_key).
    const TableRoom room = table_room(id, 0.0f, specs.size(), memory.rows_key);
    const ColumnLayout layout = place_columns(widths, specs, room);
    // SortMulti for the Shift+click second sort (D103 item 1).
    if (!ImGui::BeginTable(id, static_cast<int>(specs.size()),
                           table_flags() | ImGuiTableFlags_SortMulti, ImVec2(0.0f, height),
                           layout.inner_width))
        return;
    ImGui::TableSetupScrollFreeze(1, 1);  // the "#" column and the header row
    setup_column(specs[0], layout, 0, ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide);
    for (size_t c = 0; c < columns.size(); ++c) {
        const bool down = view.first_direction(columns[c].id) == SortDir::Descending;
        setup_column(specs[c + 1], layout, static_cast<int>(c) + 1,
                     down ? ImGuiTableColumnFlags_PreferSortDescending
                          : ImGuiTableColumnFlags_PreferSortAscending);
    }
    keep_table_column_order();  // imgui#9519, as in the Library table
    apply_column_widths(layout);

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
        std::vector<SortSpec> sort_specs;
        for (const auto& [index, dir] : *sort)
            if (index >= 1) sort_specs.push_back({columns[static_cast<size_t>(index) - 1].id, dir});
        view.set_sort(std::move(sort_specs));
    }

    // The headers, each with its column's definition on hover.
    draw_table_headers(specs, [&](size_t c) {
        if (c > 0 && !columns[c - 1].definition.empty())
            overflow_tooltip(columns[c - 1].definition.c_str());
    });
    note_min_size(memory, widths, specs, room);

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
            // (dim), so the whole row is one click target. The Selectable
            // keeps its label (its ID and the text log) but draws it
            // invisibly, so its highlight stays on the row, and the number
            // is drawn over it on the row's baseline (baseline_shift).
            ImGui::TableSetColumnIndex(0);
            const std::string number = row_number(static_cast<size_t>(k));
            bool clicked = false;
            {
                const float shift = baseline_shift(TextRole::Figure);
                const ImVec2 label_pos = ImGui::GetCursorScreenPos();
                ScopedFont figure(TextRole::Figure);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                clicked = ImGui::Selectable(
                    number.c_str(), memory.selected == index,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                ImGui::PopStyleColor();
                ImGui::GetWindowDrawList()->AddText(ImVec2(label_pos.x, label_pos.y + shift),
                                                    ImGui::GetColorU32(kDimTextColor),
                                                    number.c_str());
            }
            // Whether the list moves to the row is selection_scroll's to say
            // (ui/list_scroll.h), asked while the Selectable is the last item.
            if (clicked && pick(index)) scroll_last_row(selection_scroll(SelectionSource::Click));
            if (scroll_to && *scroll_to == static_cast<size_t>(k))
                scroll_last_row(selection_scroll(SelectionSource::Step));
            if (!can_click(row)) hint(look.unclickable_hint);

            for (size_t c = 0; c < columns.size(); ++c) {
                if (!ImGui::TableSetColumnIndex(static_cast<int>(c) + 1)) continue;
                const Column<Row>& col = columns[c];
                const std::string text = col.cell(row);
                if (col.look.chip && look.chip)
                    chip(text, look.chip(row));
                else
                    cell(text, col.look, col.tone ? col.tone(row) : Tone::Normal, memory.fitted,
                         index, c);
            }
            ImGui::PopID();
        }
    }
    ImGui::EndTable();
}

}  // namespace report_frame

}  // namespace hydra::ui

#endif  // HYDRA_UI_REPORT_WINDOW_H
