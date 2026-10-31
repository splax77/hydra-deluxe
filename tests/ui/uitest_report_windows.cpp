// The report windows' GUI tests. The first five (report windows plan, task 5)
// open the path report or the comparison window on the sample rows
// (tests/report_samples.h) through ReportWindowInput, the struct AppState
// fills in the app, and drive the window by its labels. Their own GUI
// function draws the windows; only report-window-states' analysis-off step
// fills the input from AppState (path_report_input). More work the same way:
// the column widths and the window's fit to the screen (one column-width rule
// plan, Part 3), and the cells' and header's alignment (2026-10-14
// column-shares plan). The rest (task 6)
// run end to end: the app draws both windows itself from run_frame while a
// slot's window_open is set, so they have no GUI function. Under hydra_uitest
// there is no platform backend, so each report window draws inside the main
// viewport, over the main window's rectangle (D103 item 14): close it before
// clicking the toolbar or the finished strip.

#include "uitest_harness.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <ctime>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../report_samples.h"  // tests/ is not on the runner's include path
#include "../scoped_hook.h"
#include "../wait_util.h"
#include "imgui_internal.h"
#include "app/dm_report.h"
#include "app/path_report_view.h"  // timing_choices
#include "app/report.h"
#include "core/error_kind.h"
#include "core/model.h"  // kCloneHeroSpCap
#include "net/dmbot_client.h"
#include "parse/song.h"  // NoNotesError
#include "store/record_store.h"  // failure_record
#include "ui/library_jobs.h"  // set_report_hold_for_test
#include "ui/report_window.h"

namespace uitest {

// Defined in uitest_batch_reports.cpp.
bool batch_search(ImGuiTestContext* ctx, const std::string& search);
void dismiss_done(ImGuiTestContext* ctx);
void start_library_batch(ImGuiTestContext* ctx);

namespace {

using hydra::app::dm_report::GeneratedDmReport;
using hydra::app::report::GeneratedReport;
using hydra::ui::DmReportInput;
using hydra::ui::PathReportInput;
using hydra::ui::ReportOutOfDate;
using hydra::ui::ReportBuild;

// What the windows were handed, and what they handed back.
struct Fixture {
    bool path_open = false;
    bool dm_open = false;
    PathReportInput path;
    DmReportInput dm;
    std::vector<size_t> path_clicks;
    std::vector<size_t> dm_clicks;
    int refreshes = 0;
    int cancels = 0;
    int retries = 0;
    int compares = 0;
    int closes = 0;
    std::string text;  // what the windows drew last frame
    // report-window-column-widths: when set, each frame notes the path
    // table's widths as the rule gives them (note_rule_widths).
    bool want_rule_widths = false;
    std::vector<float> rule_widths;
};

Fixture& fixture() {
    static Fixture f;
    return f;
}

std::shared_ptr<GeneratedReport> sample_path_result() {
    auto r = std::make_shared<GeneratedReport>();
    // The sample rows counted against the sample library, as generate_report
    // hands them over: each row carries its chart's copies, and
    // library_charts is the library's own count.
    const hydra::store::ChartIndex library = report_samples::sample_index();
    r->paths = report_samples::counted_against(report_samples::sample_path_rows(), library);
    r->library_charts = library.count(library.song_ids());
    r->rows = static_cast<int64_t>(r->paths.size());
    r->subtitle = "sample path subtitle";
    r->footer = "sample path footer";
    return r;
}

std::shared_ptr<GeneratedDmReport> sample_dm_result() {
    auto r = std::make_shared<GeneratedDmReport>();
    r->rows = report_samples::sample_dm_rows();
    r->stats = hydra::app::dm_report::tally_dm_rows(r->rows);
    r->username = "SamplePlayer";
    r->subtitle = "sample dm subtitle";
    r->footer = "sample dm footer";
    return r;
}

hydra::ui::ReportCallbacks recording_callbacks(std::vector<size_t>& clicks) {
    hydra::ui::ReportCallbacks cb;
    cb.row_click = [&clicks](size_t row) { clicks.push_back(row); };
    cb.refresh = [] { ++fixture().refreshes; };
    cb.cancel = [] { ++fixture().cancels; };
    cb.try_again = [] { ++fixture().retries; };
    cb.compare_another = [] { ++fixture().compares; };
    cb.close = [] { ++fixture().closes; };
    return cb;
}

// Both windows closed, each holding a freshly built sample report.
void reset_fixture() {
    Fixture& f = fixture();
    f = Fixture{};
    f.path.result = sample_path_result();
    f.path.state = ReportBuild::Ready;
    f.path.built = std::chrono::system_clock::now();
    f.path.callbacks = recording_callbacks(f.path_clicks);
    f.dm.result = sample_dm_result();
    f.dm.state = ReportBuild::Ready;
    f.dm.built = std::chrono::system_clock::now();
    f.dm.callbacks = recording_callbacks(f.dm_clicks);
}

ImGuiWindow* path_window();

// The path table's widths this frame as the production rule gives them: the
// sample rows measured by the window's own measure, placed in the room
// table_room reads inside the path window. It appends to the path window,
// drawn earlier this frame, so table_room sees the room the table had.
void note_rule_widths() {
    Fixture& f = fixture();
    f.rule_widths.clear();
    ImGuiWindow* window = path_window();
    if (!f.path_open || !window || !f.path.result) return;
    namespace view_rules = hydra::app::path_report_view;
    const GeneratedReport& report = *f.path.result;
    std::vector<std::string> texts;
    for (const auto& r : report.paths) texts.push_back(view_rules::path_search_text(r));
    const hydra::app::report_view::TableView<hydra::app::report::ReportRow> view(
        hydra::app::report_view::shared_rows(f.path.result, &GeneratedReport::paths),
        std::move(texts), view_rules::path_columns());
    const std::vector<hydra::ui::ColumnSpec> specs =
        hydra::ui::report_frame::column_specs(view.columns());
    ImGui::Begin(window->Name);
    const hydra::ui::TableRoom room = hydra::ui::table_room("##pathtable", 0.0f, specs.size());
    f.rule_widths =
        hydra::ui::place_columns(hydra::ui::report_frame::measure(view), specs, room).widths;
    ImGui::End();
}

// The test's GUI function: both windows, every frame, with their text kept.
void draw_windows(ImGuiTestContext*) {
    Fixture& f = fixture();
    ImGui::LogToBuffer();
    hydra::ui::draw_path_report_window(&f.path_open, f.path);
    hydra::ui::draw_dm_report_window(&f.dm_open, f.dm);
    ImGuiContext& g = *ImGui::GetCurrentContext();
    f.text.assign(g.LogBuffer.begin(), g.LogBuffer.end());
    ImGui::LogFinish();
    if (f.want_rule_widths) note_rule_widths();
}

// Today at hour:minute, local time.
std::chrono::system_clock::time_point local_time(int hour, int minute) {
    const std::time_t now = std::time(nullptr);
    std::tm t{};
    localtime_s(&t, &now);
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = 0;
    t.tm_isdst = -1;
    return std::chrono::system_clock::from_time_t(std::mktime(&t));
}

bool shows(const std::string& s) { return fixture().text.find(s) != std::string::npos; }
size_t where(const std::string& s) { return fixture().text.find(s); }

ImGuiWindow* path_window() { return window_named("###pathreport"); }
ImGuiWindow* dm_window() { return window_named("###dmreport"); }

// Opens the path report and points the ref at it, with Optimal paths only
// unticked: none of the sample rows is an optimal path.
void open_path_all_rows(ImGuiTestContext* ctx) {
    fixture().path_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    if (ctx->IsError()) return;
    ctx->SetRef(path_window());
    ctx->ItemClick("Optimal paths only");
    ctx->Yield(2);
}

void open_dm(ImGuiTestContext* ctx) {
    fixture().dm_open = true;
    ctx->Yield(3);
    IM_CHECK(dm_window() != nullptr);
    if (ctx->IsError()) return;
    ctx->SetRef(dm_window());
}

// The first sort is Score, highest first; a header click flips it.
void test_sort(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    fixture().path_open = true;
    ctx->Yield(3);
    // Optimal paths only starts ticked, and no sample row is optimal.
    IM_CHECK(shows("Nothing matches."));
    IM_CHECK(shows("0 of 6 paths"));
    open_path_all_rows(ctx);
    IM_CHECK(shows("6 of 6 paths"));
    // Row 1 is Song B's best path (sample row 4, 250,000); after the flip it
    // is Song C (sample row 6, 77,000). The rows are read back through their
    // clicks: a cut cell's text never reaches the frame's text.
    Fixture& f = fixture();
    ctx->ItemClick("**/1");
    IM_CHECK(!f.path_clicks.empty() && f.path_clicks.back() == 3);
    ctx->ItemClick("**/Score");
    ctx->Yield(2);
    ctx->ItemClick("**/1");
    IM_CHECK(!f.path_clicks.empty() && f.path_clicks.back() == 5);

    // The comparison opens on Points left, most first.
    open_dm(ctx);
    IM_CHECK(shows("4 of 4 scores"));
    IM_CHECK_LT(where("3,456"), where("+1,000 over"));
}

// Each window's search box hint, then filter to nothing, then Clear filters.
void test_filters(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    open_path_all_rows(ctx);
    IM_CHECK(shows("Search song, artist, charter, or path notation"));
    ctx->ItemInputValue("##search", "zzz");
    ctx->Yield(2);
    IM_CHECK(shows("0 of 6 paths"));
    IM_CHECK(shows("Nothing matches."));
    IM_CHECK(!shows("Nothing matches those filters."));
    ctx->ItemClick("Clear filters");
    ctx->Yield(2);
    // The search is empty again and Optimal paths only stayed unticked.
    IM_CHECK(shows("6 of 6 paths"));

    open_dm(ctx);
    IM_CHECK(shows("Search song, artist, or charter"));
    ctx->ComboClick("##status/Not in your library");
    ctx->Yield(2);
    IM_CHECK(shows("1 of 4 scores"));
    ctx->ItemInputValue("##search", "zzz");
    ctx->Yield(2);
    IM_CHECK(shows("0 of 4 scores"));
    IM_CHECK(shows("Nothing matches."));
    ctx->ItemClick("Clear filters");
    ctx->Yield(2);
    IM_CHECK(shows("4 of 4 scores"));
}

// Every state the frame draws.
void test_states(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    f.path.state = ReportBuild::Building;
    f.path_open = true;
    ctx->Yield(3);
    // The subtitle while it builds (D103 item 21, board 2a), not the old
    // result's. The build only reads the store, so no bar counts anything.
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK(!shows("sample path subtitle"));
    IM_CHECK(!shows("Analyzing "));
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 1);

    f.path.state = ReportBuild::Cancelled;
    ctx->Yield(2);
    IM_CHECK(shows("Report cancelled."));
    ctx->ItemClick("Try again");
    IM_CHECK_EQ(f.retries, 1);

    f.path.state = ReportBuild::Failed;
    f.path.failure_message = "sample failure message";
    f.path.failure_error = "sample failure error";
    ctx->Yield(2);
    IM_CHECK(shows("The path report could not be built."));
    IM_CHECK(shows("sample failure message"));
    IM_CHECK(shows("sample failure error"));

    f.path.state = ReportBuild::Ready;
    f.path.out_of_date = ReportOutOfDate::Library;
    ctx->Yield(2);
    IM_CHECK(shows("Your library changed since this report was built."));
    IM_CHECK(shows("Built "));
    ctx->ItemClick("**/Refresh##outofdate");  // inside the strip's child
    IM_CHECK_EQ(f.refreshes, 1);
    // With a batch's finish time, the library line names it (D103 item 21,
    // board 2b), in the same HH:MM as "Built".
    f.path.built = local_time(13, 42);
    f.path.batch_finished = local_time(14, 5);
    ctx->Yield(2);
    IM_CHECK(shows("Your library changed since this report was built "
                   "(a batch finished at 14:05)."));
    IM_CHECK(shows("Built 13:42"));
    // The settings line has no time.
    f.path.out_of_date = ReportOutOfDate::Settings;
    ctx->Yield(2);
    IM_CHECK(shows("The settings changed since this report was built."));
    IM_CHECK(!shows("a batch finished"));
    f.path.out_of_date = ReportOutOfDate::None;
    f.path.batch_finished.reset();

    // Nothing to report: the report's empty reason.
    auto empty = std::make_shared<GeneratedReport>();
    empty->why_empty = "sample empty reason";
    f.path.result = empty;
    ctx->Yield(2);
    IM_CHECK(shows("sample empty reason"));
    f.path_open = false;

    f.dm.state = ReportBuild::Building;
    open_dm(ctx);
    IM_CHECK(shows("Fetching scores and building the report..."));
    IM_CHECK(!shows("Building the report from your library..."));
    IM_CHECK(!shows("sample dm subtitle"));
    IM_CHECK(shows("The leaderboard server can take a moment to wake up."));
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 2);
    f.dm.state = ReportBuild::Failed;
    ctx->Yield(2);
    IM_CHECK(shows("The comparison could not be built."));  // D114 item 14
    ctx->ItemClick("Compare another player...");
    IM_CHECK_EQ(f.compares, 1);
    f.dm_open = false;

    // Analysis off with no report in memory (D103 item 24): the toolbar's
    // sentence and the rules file's error, as AppState hands them over.
    Harness& h = harness(ctx);
    reset_app(h, "max_tied_paths = 0\n");
    IM_CHECK_RETV(h.app->analysis_blocked(), );
    f.path = hydra::ui::path_report_input(*h.app);
    IM_CHECK(f.path.state == ReportBuild::None);
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(shows("hydra_rules.ini has an error, so analysis is off until the file is "
                   "fixed and Hydra is restarted."));
    IM_CHECK(shows("max_tied_paths"));
    f.path_open = false;
    ctx->Yield(2);
}

// The path table as ImGui holds it, with the ref on the path window.
ImGuiTable* path_table(ImGuiTestContext* ctx) {
    ctx->SetRef(path_window());
    return ImGui::TableFindByID(ctx->GetID("##pathtable"));
}

// Display row `n`'s rect (its "#" Selectable, which spans the row), or none
// when the table did not draw it.
std::optional<ImRect> drawn_row(ImGuiTestContext* ctx, size_t n) {
    const ImGuiTestItemInfo info =
        ctx->ItemInfo(("**/" + std::to_string(n)).c_str(), ImGuiTestOpFlags_NoError);
    if (info.ID == 0) return std::nullopt;
    return info.RectFull;
}

// In a path window too short for the six sample rows: a row click leaves the
// list where it is (the user's rule, 2026-10-14: a click never scrolls);
// Down onto the first row below the view brings it in at the bottom edge, not
// the middle; Up onto a row already on screen moves nothing (answer 4).
void check_keys_scroll_just_enough(ImGuiTestContext* ctx) {
    ImGuiWindow* window = path_window();
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(window != nullptr && table != nullptr && table->InnerWindow != nullptr, );
    const ImVec2 size_before = window->Size;
    // Room for the header row and about two and a half of the six rows, in
    // rows as the table drew them. With more on screen, centring the next row
    // would stop at the end of the list, where it meets the bottom edge too.
    const std::optional<ImRect> first = drawn_row(ctx, 1);
    IM_CHECK_RETV(first.has_value(), );
    const ImGuiStyle& style = ImGui::GetStyle();
    const float above = table->OuterRect.Min.y - window->Pos.y;
    const float footer = ImGui::GetTextLineHeightWithSpacing();
    ctx->WindowResize("", ImVec2(size_before.x, above + footer + style.WindowPadding.y +
                                                    style.ScrollbarSize +
                                                    first->GetHeight() * 3.5f));
    ctx->Yield(3);
    ImGuiWindow* list = table->InnerWindow;
    IM_CHECK_RETV(list->ScrollMax.y > 0.0f, );
    ImGui::SetScrollY(list, 0.0f);
    ctx->Yield(3);

    // The last row wholly on screen, and the row after it, partly or not at
    // all on screen. Centring that next row must move the list further than
    // bringing it to the bottom edge, so it sits well above the last row.
    size_t last_shown = 0;
    for (size_t n = 1; n <= 6; ++n) {
        const std::optional<ImRect> r = drawn_row(ctx, n);
        if (r && fully_shown(list, *r)) last_shown = n;
    }
    ctx->LogInfo("last row wholly on screen: %zu", last_shown);
    IM_CHECK_RETV(last_shown >= 2 && last_shown <= 3, );

    // A click on that row keeps the list still.
    const float scroll = list->Scroll.y;
    ctx->ItemClick(("**/" + std::to_string(last_shown)).c_str());
    ctx->Yield(3);
    ctx->LogInfo("scroll before the click %.1f, after %.1f", scroll, list->Scroll.y);
    IM_CHECK_EQ(list->Scroll.y, scroll);

    // Down onto the next row: it comes in at the bottom edge.
    ctx->KeyPress(ImGuiKey_DownArrow);
    ctx->Yield(3);
    const std::optional<ImRect> next = drawn_row(ctx, last_shown + 1);
    IM_CHECK_RETV(next.has_value(), );
    ctx->LogInfo("row bottom %.1f, list bottom %.1f, spacing %.1f, scroll %.1f", next->Max.y,
                 list->InnerRect.Max.y, style.ItemSpacing.y, list->Scroll.y);
    IM_CHECK(fully_shown(list, *next));
    IM_CHECK_GE(next->Max.y, list->InnerRect.Max.y - style.ItemSpacing.y);

    // Up onto the row above, which is on screen: nothing moves.
    const float after_down = list->Scroll.y;
    ctx->KeyPress(ImGuiKey_UpArrow);
    ctx->Yield(3);
    IM_CHECK_EQ(list->Scroll.y, after_down);

    ctx->WindowResize("", size_before);
    ctx->Yield(2);
}

// Row clicks, the arrow keys, Esc and Ctrl+W.
void test_keys(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    open_path_all_rows(ctx);
    // Row 1 in the first sort is Song B's best path, the fourth sample row.
    ctx->ItemClick("**/1");
    IM_CHECK_EQ(f.path_clicks.size(), size_t{1});
    if (ctx->IsError()) return;
    IM_CHECK_EQ(f.path_clicks.back(), size_t{3});
    ctx->KeyPress(ImGuiKey_DownArrow);
    ctx->Yield(2);
    IM_CHECK_EQ(f.path_clicks.back(), size_t{4});
    check_keys_scroll_just_enough(ctx);
    if (ctx->IsError()) return;

    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 1);
    // Reopening, the window takes the rows up again. They come as AppState
    // hands rows built for an opening (keep_filters), so Optimal paths only
    // stays unticked.
    f.path.keep_filters = true;
    f.path_open = true;
    ctx->Yield(2);
    IM_CHECK(shows("6 of 6 paths"));
    ctx->SetRef(path_window());
    ctx->WindowFocus("");
    ctx->KeyPress(ImGuiMod_Ctrl | ImGuiKey_W);
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK_EQ(f.closes, 2);

    // Row 3 is Song C, not in the library: its click does nothing.
    open_dm(ctx);
    ctx->ItemClick("**/3");
    IM_CHECK(f.dm_clicks.empty());
    ctx->ItemClick("**/1");
    IM_CHECK_EQ(f.dm_clicks.size(), size_t{1});
}

// The header's Close shuts the window as Esc does, in every build state and
// in both windows.
void test_close_button(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    open_path_all_rows(ctx);
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 1);

    // Outside Ready the header has no Refresh, but Close is still there.
    for (ReportBuild state : {ReportBuild::None, ReportBuild::Cancelled, ReportBuild::Failed}) {
        f.path.state = state;
        f.path_open = true;
        ctx->Yield(3);
        IM_CHECK_RETV(path_window() != nullptr, );
        ctx->SetRef(path_window());
        IM_CHECK(ctx->ItemExists("Close"));
        IM_CHECK(!ctx->ItemExists("Refresh"));
    }
    // Shut by the fixture, not by the window, so no close callback runs.
    f.path_open = false;
    ctx->Yield(2);
    IM_CHECK_EQ(f.closes, 1);

    // Closing while the report builds.
    f.path.state = ReportBuild::Building;
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK_RETV(path_window() != nullptr, );
    ctx->SetRef(path_window());
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 2);
    IM_CHECK_EQ(f.cancels, 0);  // Close is not the build's Cancel

    // The comparison has it too.
    open_dm(ctx);
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!f.dm_open);
    IM_CHECK(dm_window() == nullptr);
    IM_CHECK_EQ(f.closes, 3);
}

// A new result that lands while a window is hidden frees the old one at
// once: the hidden window's view must not keep it alive until the reopen.
void test_hidden_new_result(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    f.path_open = true;
    f.dm_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(dm_window() != nullptr);
    f.path_open = false;
    f.dm_open = false;
    ctx->Yield(2);

    std::weak_ptr<const GeneratedReport> old_path = f.path.result;
    std::weak_ptr<const GeneratedDmReport> old_dm = f.dm.result;
    f.path.result = sample_path_result();
    f.dm.result = sample_dm_result();
    ctx->Yield(2);
    IM_CHECK(old_path.expired());
    IM_CHECK(old_dm.expired());

    f.path_open = true;
    f.dm_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(dm_window() != nullptr);
    IM_CHECK(shows("of 6 paths"));
}

// Both windows open at once.
void test_both(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    fixture().path_open = true;
    fixture().dm_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(dm_window() != nullptr);
    IM_CHECK(shows("of 6 paths"));
    IM_CHECK(shows("4 of 4 scores"));
}

// ---- Column widths and the window's size (one column-width rule) ----------

// In a narrow window the chips and numbers show in full and the table
// scrolls sideways. A cut cell draws past ImGui's text log, so a label in the
// frame's text was drawn whole.
void test_chip_label(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    ctx->WindowResize("", ImVec2(900.0f, 600.0f));
    ctx->Yield(3);
    Fixture& f = fixture();
    const std::vector<hydra::app::report::ReportRow>& rows = f.path.result->paths;

    // Every row's Timing chip, Song B's "Beyond" tier among them (171 ms).
    for (const auto& r : rows)
        IM_CHECK(shows(hydra::app::path_report_view::tier_label(r.tier)));

    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    IM_CHECK(table->InnerWindow->ScrollbarX);

    // Every number column's cells, read at both ends of the sideways scroll
    // (a column scrolled out of view draws nothing).
    const auto columns = hydra::app::path_report_view::path_columns();
    std::vector<bool> read(columns.size(), false);
    for (bool right_end : {false, true}) {
        ImGui::SetScrollX(table->InnerWindow, right_end ? table->InnerWindow->ScrollMax.x : 0.0f);
        ctx->Yield(3);
        for (size_t c = 0; c < columns.size(); ++c) {
            if (!columns[c].numeric || !table->Columns[static_cast<int>(c) + 1].IsRequestOutput)
                continue;
            read[c] = true;
            for (const auto& r : rows) IM_CHECK(shows(columns[c].cell(r)));
        }
    }
    for (size_t c = 0; c < columns.size(); ++c)
        if (columns[c].numeric) IM_CHECK(read[c]);
}

// With the window wide, every column is as wide as the rule says for the
// sample rows.
void test_column_widths(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    Fixture& f = fixture();
    f.want_rule_widths = true;
    ctx->Yield(3);
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr, );
    IM_CHECK_RETV(static_cast<size_t>(table->ColumnsCount) == f.rule_widths.size(), );
    for (int c = 0; c < table->ColumnsCount; ++c) {
        ctx->LogInfo("column %d (%s)", c, ImGui::TableGetColumnName(table, c));
        IM_CHECK_EQ(table->Columns[c].WidthGiven, f.rule_widths[static_cast<size_t>(c)]);
    }
    f.want_rule_widths = false;
}

// The leftmost x of the text glyphs drawn last frame in `color` inside
// column `c` of `table`, between `top` and `bottom`: where that cell's text
// starts. Glyphs are told from filled rects by their texture coordinates (a
// rect samples the font atlas's white pixel). None when no such glyph.
std::optional<float> text_start(const ImGuiTable* table, int c, ImU32 color, float top,
                                float bottom) {
    const ImGuiTableColumn& col = table->Columns[c];
    const ImVec2 white = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
    std::optional<float> left;
    for (const ImDrawVert& v : table->InnerWindow->DrawList->VtxBuffer) {
        if (v.col != color || (v.uv.x == white.x && v.uv.y == white.y)) continue;
        if (v.pos.y < top || v.pos.y > bottom || v.pos.x < col.MinX || v.pos.x > col.MaxX)
            continue;
        left = left ? (std::min)(*left, v.pos.x) : v.pos.x;
    }
    return left;
}

// Every cell's text starts at its column's left edge: a number in Score and
// the dim row number in "#" alike (the user's answer to Change 3 of the
// 2026-10-14 column-shares plan). The shorter scores show it: right-aligned,
// "77,000" would start a digit's width in from where "250,000" starts.
void test_cells_left(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    reset_fixture();
    open_path_all_rows(ctx);
    if (ctx->IsError()) return;
    // Wide enough that every column is on screen.
    ctx->WindowResize("", ImVec2(static_cast<float>(h.width), 600.0f));
    ctx->Yield(3);
    ImGuiTable* table = path_table(ctx);
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    const auto columns = hydra::app::path_report_view::path_columns();
    int score = -1;
    for (size_t c = 0; c < columns.size(); ++c)
        if (columns[c].title == "Score") score = static_cast<int>(c) + 1;
    IM_CHECK_RETV(score > 0 && table->Columns[score].IsVisibleX, );

    const float pad = ImGui::GetStyle().CellPadding.x;
    const ImU32 text = ImGui::GetColorU32(ImGuiCol_Text);
    const ImU32 dim = ImGui::GetColorU32(hydra::ui::kDimTextColor);
    const std::pair<int, ImU32> cells[] = {{0, dim}, {score, text}};
    // From the bottom up, so the shortest score is read first.
    for (size_t n = 6; n >= 1; --n) {
        const std::optional<ImRect> row = drawn_row(ctx, n);
        IM_CHECK_RETV(row.has_value(), );
        for (const auto& [c, color] : cells) {
            const std::optional<float> x = text_start(table, c, color, row->Min.y, row->Max.y);
            IM_CHECK_RETV(x.has_value(), );
            ctx->LogInfo("row %zu column %d: text at %.1f, column starts at %.1f", n, c, *x,
                         table->Columns[c].WorkMinX);
            IM_CHECK_LE(std::fabs(*x - table->Columns[c].WorkMinX), pad);
        }
    }
}

// A report too narrow for the header's buttons beside "Built HH:MM": the
// buttons start a line of their own, Close last at its right edge (the user's
// answer 7 and their choice of 2026-10-14). A report with no rows has no
// controls row or table to hold the window wide.
void test_header_narrow(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    Fixture& f = fixture();
    auto empty = std::make_shared<GeneratedReport>();
    empty->why_empty = "sample empty reason";
    f.path.result = empty;
    f.path.built = local_time(13, 42);
    f.path_open = true;
    ctx->Yield(3);
    ImGuiWindow* window = path_window();
    IM_CHECK_RETV(window != nullptr, );
    ctx->SetRef(window);
    ctx->WindowResize("", ImVec2(900.0f, 400.0f));
    ctx->Yield(3);
    const ImRect wide = ctx->ItemInfo("Refresh").RectFull;
    const ImRect wide_close = ctx->ItemInfo("Close").RectFull;
    IM_CHECK(shows("Built 13:42"));
    // Wide: Refresh and Close share the heading's line, Close at its right
    // edge with Refresh just before it.
    IM_CHECK_LE(std::fabs(wide_close.Max.x - window->WorkRect.Max.x), 1.0f);
    IM_CHECK_EQ(wide_close.Min.y, wide.Min.y);
    IM_CHECK_LT(wide.Max.x, wide_close.Min.x);

    // Room for "Hydra Path Index  Built 13:42", but not for Refresh beside it.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float header_w = ImGui::CalcTextSize("Hydra").x + ImGui::CalcTextSize("Path Index").x +
                           ImGui::CalcTextSize("Built 13:42").x + style.ItemSpacing.x * 3.0f;
    ctx->WindowResize("", ImVec2(header_w + style.WindowPadding.x * 2.0f + style.ScrollbarSize +
                                     wide.GetWidth() * 0.5f,
                                 400.0f));
    ctx->Yield(3);
    const ImRect narrow = ctx->ItemInfo("Refresh").RectFull;
    const ImRect narrow_close = ctx->ItemInfo("Close").RectFull;
    ctx->LogInfo("Refresh wide %.1f..%.1f at y %.1f, narrow %.1f..%.1f at y %.1f, right edge %.1f",
                 wide.Min.x, wide.Max.x, wide.Min.y, narrow.Min.x, narrow.Max.x, narrow.Min.y,
                 window->WorkRect.Max.x);
    ctx->LogInfo("Close wide %.1f..%.1f, narrow %.1f..%.1f at y %.1f", wide_close.Min.x,
                 wide_close.Max.x, narrow_close.Min.x, narrow_close.Max.x, narrow_close.Min.y);
    IM_CHECK_GE(narrow.Min.y, wide.Min.y + ImGui::GetTextLineHeightWithSpacing());
    IM_CHECK_EQ(narrow_close.Min.y, narrow.Min.y);
    IM_CHECK_LT(narrow.Max.x, narrow_close.Min.x);
    IM_CHECK_LE(std::fabs(narrow_close.Max.x - window->WorkRect.Max.x), 1.0f);
    f.path_open = false;
    ctx->Yield(2);
}

// A saved size bigger than the screen (the user's hydra_ui.ini) opens fitted
// to it; with the main window maximized the report opens at the whole work
// area. The runner's work area is its display, and it has no OS frame.
void test_fits_screen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    reset_fixture();
    const hydra::ui::WindowPlacement main_before = hydra::ui::window_placement();
    const ImVec2 origin = ImGui::GetMainViewport()->Pos;
    const ImVec2 display(static_cast<float>(h.width), static_cast<float>(h.height));
    Fixture& f = fixture();
    // Shut, so the next opening is placed afresh (an open window stays put).
    ctx->Yield(2);

    // The user's entry: a 2,560 x 1,440 monitor less a 23 px title band.
    const char* big_ini = "[Window][###pathreport]\nViewportPos=0,23\nSize=2560,1417\n";
    ImGui::LoadIniSettingsFromMemory(big_ini);
    f.path_open = true;
    ctx->Yield(3);
    ImGuiWindow* w = path_window();
    IM_CHECK_RETV(w != nullptr, );
    IM_CHECK_LE(w->SizeFull.x, display.x);
    IM_CHECK_LE(w->SizeFull.y, display.y);
    IM_CHECK_GE(w->Pos.x, origin.x);
    IM_CHECK_GE(w->Pos.y, origin.y);
    IM_CHECK_LE(w->Pos.x + w->SizeFull.x, origin.x + display.x);
    IM_CHECK_LE(w->Pos.y + w->SizeFull.y, origin.y + display.y);
    f.path_open = false;
    ctx->Yield(2);

    // A small saved size, but the main window is maximized: the work area.
    const char* small_ini = "[Window][###pathreport]\nPos=100,100\nSize=600,400\n";
    ImGui::LoadIniSettingsFromMemory(small_ini);
    hydra::ui::WindowPlacement maximized = main_before;
    maximized.valid = true;
    maximized.maximized = true;
    hydra::ui::remember_window_placement(maximized);
    f.path_open = true;
    ctx->Yield(3);
    w = path_window();
    IM_CHECK(w != nullptr);
    if (w) {
        IM_CHECK_EQ(w->Pos.x, origin.x);
        IM_CHECK_EQ(w->Pos.y, origin.y);
        IM_CHECK_EQ(w->SizeFull.x, display.x);
        IM_CHECK_EQ(w->SizeFull.y, display.y);
    }
    f.path_open = false;
    ctx->Yield(2);
    hydra::ui::remember_window_placement(main_before);
}

// ---- End to end, through AppState (task 6) --------------------------------

// Library row `row`'s title in quotes, so the word search matches it as one
// phrase. Read it before any search narrows the rows.
std::string quoted_title(Harness& h, size_t row) {
    return "\"" + h.app->library_row_at(row).title + "\"";
}

// The tail of the path window's count line for the report in memory,
// " of N path" (the "s" left off, so one path matches too).
std::string path_count_tail(Harness& h) {
    const size_t n = h.app->path_report.result ? h.app->path_report.result->paths.size() : 0;
    return " of " + std::to_string(n) + " path";
}

// Esc on a report window, as a user closes it.
void press_escape_on(ImGuiTestContext* ctx, ImGuiWindow* window) {
    IM_CHECK_RETV(window != nullptr, );
    ctx->SetRef(window);
    ctx->WindowFocus("");
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
}

// Holds every path report build before it reads the store until release(),
// so a test can look at the building state. The job copies the hold when it
// is made; the destructor lets any held build go and removes the seam.
std::atomic<bool> g_report_release{true};
class ReportHold {
public:
    ReportHold() {
        g_report_release = false;
        seam_.emplace(hydra::ui::set_report_hold_for_test, [] {
            testwait::wait_until([] { return g_report_release.load(); },
                                 "the test to release the path report's build");
        });
    }
    ~ReportHold() { release(); }
    ReportHold(const ReportHold&) = delete;
    ReportHold& operator=(const ReportHold&) = delete;
    void release() {
        g_report_release = true;
        seam_.reset();
    }

private:
    std::optional<ScopedHook> seam_;
};

// The dmleaderboards API, canned as reset_app cans it (alice, one score on
// `md5`), with an optional second score on a chart not in the library, and
// optionally holding the scores fetch until release(). The destructor lets a
// held fetch go; reset_app puts its own fetcher back for the next test.
constexpr const char* kNotInLibraryMd5 = "ffffffffffffffffffffffffffffffff";
std::atomic<bool> g_fetch_release{true};
class CannedDm {
public:
    CannedDm(const std::string& md5, bool not_in_library_score, bool hold) {
        g_fetch_release = !hold;
        std::vector<std::string> rows{canned_dm_score(md5, 100000)};
        if (not_in_library_score) rows.push_back(canned_dm_score(kNotInLibraryMd5, 90000));
        const std::string body = canned_dm_scores(rows);
        hydra::net::set_fetcher([body](const std::string& url, const std::atomic<bool>* cancel) {
            if (url.find("/all-users") != std::string::npos) return canned_dm_users();
            testwait::wait_until(
                [cancel] { return g_fetch_release.load() || (cancel && cancel->load()); },
                "the test to release or cancel the scores fetch");
            return body;
        });
    }
    ~CannedDm() { release(); }
    CannedDm(const CannedDm&) = delete;
    CannedDm& operator=(const CannedDm&) = delete;
    void release() { g_fetch_release = true; }
};

// The toolbar's "Compare with dmleaderboards...", then alice in the picker.
void compare_with_alice(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Compare with dmleaderboards...");
    IM_CHECK_RETV(wait_until(ctx, [&] { return !h.app->dm_users.empty(); }, 10), );
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemClick("**/###111");
    ctx->Yield(2);
}

// A path report build just started under `hold`: the window shows the
// building subtitle with no count, then, once released, the rows.
void held_build_then_rows(ImGuiTestContext* ctx, ReportHold& hold) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job != nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Building;
    }, 10), );
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Building the report from your library..."));
    IM_CHECK(!on_screen(h, "Analyzing "));
    hold.release();
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, path_count_tail(h)));
}

// The toolbar's "Open path report" (or, with `refresh`, the open window's
// Refresh) with charts that have no result: with the confirm threshold at 0
// the path window asks first, and its Cancel leaves the report to list only
// what is stored. Returns with the threshold put back and that report's build
// started; the caller waits for it.
void open_path_report_declining_batch(ImGuiTestContext* ctx, bool refresh = false) {
    Harness& h = harness(ctx);
    h.app->report_confirm_above = 0;
    if (refresh) {
        ctx->SetRef(path_window());
        ctx->ItemClick("Refresh");
    } else {
        ctx->SetRef("//Hydra");
        ctx->ItemClick("**/Open path report");
    }
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->path_report_awaits_confirm());
    IM_CHECK(window_named("Analyze library") == nullptr);  // no main-window popup
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(!h.app->path_report_awaits_confirm());
    IM_CHECK(h.app->path_report_batch() == nullptr);
    IM_CHECK(h.app->path_report_build() != hydra::ui::ReportBuild::Cancelled);
    h.app->report_confirm_above = hydra::ui::AppState::kReportConfirmAbove;
}

// More rows to run than the threshold (lowered to 0 here): the path window
// itself asks, in the batch confirm's words, and its Analyze runs the batch
// the report then waits on, its count over the bar. No "Analyze library"
// popup opens in the main window. The batch's charts wait at a gate, so the
// running state is read before it ends.
void test_open_asks(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    struct Threshold {
        Harness& h;
        ~Threshold() { h.app->report_confirm_above = hydra::ui::AppState::kReportConfirmAbove; }
    } threshold{h};
    BatchGate gate;
    h.app->report_confirm_above = 0;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK_RETV(h.app->path_report_awaits_confirm(), );
    IM_CHECK(h.app->batch_job == nullptr);
    IM_CHECK(on_screen(h, h.app->batch_question(false)));
    IM_CHECK(window_named("Analyze library") == nullptr);

    ctx->SetRef(path_window());
    ctx->ItemClick("Analyze");
    ctx->Yield(2);
    IM_CHECK_RETV(h.app->batch_job != nullptr, );
    IM_CHECK(h.app->path_report_batch() == h.app->batch_job.get());
    IM_CHECK(wait_until(ctx, [&] {
        const hydra::ui::BatchJob::Snapshot s = h.app->batch_job->snapshot();
        return !s.preparing && s.total > 0 && gate.started() >= 1;
    }, 10));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Analyzing "));
    IM_CHECK(window_named("Analyze library") == nullptr);
    IM_CHECK(!h.app->batch_confirm_pending);

    // The window's Cancel now stops the report's own batch, as before.
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(h.app->batch_job->is_cancelled());
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Cancelled);
    gate.allow(1000000);
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 60));
    h.app->close_path_report();
    ctx->Yield(2);
}

// Waits for the path report's build to finish, up to 60 s.
bool path_report_ready(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const bool ready = wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60);
    ctx->Yield(2);
    return ready;
}

// The path window opens from the finished strip's "Open report", from the
// toolbar's "Open path report", and by itself after a batch with "Open
// automatically" on. While it builds it shows the building state.
void test_open_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string first = quoted_title(h, 0);
    const std::string second = quoted_title(h, 1);
    // Opening with nothing analyzed runs a batch first: test_open_analyzes.

    // The finished strip's Open report.
    IM_CHECK(batch_search(ctx, first));
    IM_CHECK(path_window() == nullptr);  // auto-open is off
    ImGuiWindow* done = window_named("##batchdone");
    IM_CHECK_RETV(done != nullptr, );
    ctx->SetRef(done);
    ctx->ItemClick("Open report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, path_count_tail(h)));
    press_escape_on(ctx, path_window());
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(path_window() == nullptr);
    dismiss_done(ctx);

    // The toolbar's Open path report. The Esc above only hid the window
    // (D271 item 7), so it shows the same rows at once, with no build.
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(on_screen(h, path_count_tail(h)));
    // Its Refresh, the batch declined, so the second chart below still has
    // no result.
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK(on_screen(h, path_count_tail(h)));
    h.app->close_path_report();  // it covers the toolbar
    ctx->Yield(2);

    // Open automatically: the next batch's report opens its window by itself.
    h.app->settings.auto_open_report = true;
    h.app->commit_settings();
    IM_CHECK(batch_search(ctx, second));
    ctx->Yield(2);
    IM_CHECK(h.app->path_report.window_open);
    IM_CHECK_RETV(path_window() != nullptr, );

    // Refresh with every build held before it reads the store: the window
    // shows the building state. Its batch over the charts with no result is
    // declined, so the build alone is held.
    ReportHold hold;
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    held_build_then_rows(ctx, hold);
}

// Opening the report with nothing analyzed runs a batch over the corpus first
// (plan 2026-10-15, question 8): the window shows the batch's own count over
// its bar, then the rows once the batch's report lands. The batch's workers
// wait at a gate before each real analysis, so the count is read at 0.
void test_open_analyzes(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    static std::atomic<bool> go{false};
    go = false;
    struct Release {
        ~Release() { go = true; }  // a failed check never leaves the workers waiting
    } release;
    hydra::ui::set_app_batch_analyzer_for_test(
        [](const std::string& path, const hydra::app::AnalysisSettings& settings,
           const std::function<void(float)>& progress) {
            while (!go.load()) std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return hydra::app::analyze_chart_file(path, settings, progress);
        },
        hydra::app::batch_worker_count());
    ctx->SetRef("//Hydra");
    IM_CHECK(ctx->ItemExists("**/Open path report"));  // offered with nothing analyzed
    ctx->ItemClick("**/Open path report");
    hydra::ui::set_app_batch_analyzer_for_test({}, 1);
    IM_CHECK_RETV(h.app->batch_job != nullptr, );
    IM_CHECK(h.app->path_report_batch() == h.app->batch_job.get());
    IM_CHECK(wait_until(ctx, [&] {
        const hydra::ui::BatchJob::Snapshot s = h.app->batch_job->snapshot();
        return !s.preparing && s.total > 0;
    }, 10));
    const int total = h.app->batch_job->snapshot().total;
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, "Analyzing 0 of " + hydra::counted(total, "chart", "charts")));

    go = true;
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 300));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Charts shown"));
    IM_CHECK(on_screen(h, path_count_tail(h)));
    // The coverage lines: the batch gave every chart a result, so none is
    // left without one, and any chart it couldn't analyze is named by the
    // report's own line on screen.
    IM_CHECK_RETV(h.app->path_report.result != nullptr, );
    const hydra::app::report::GeneratedReport& page = *h.app->path_report.result;
    IM_CHECK_EQ(page.no_result_charts, int64_t{0});
    IM_CHECK(page.no_result_line.empty());
    IM_CHECK_EQ(page.songs + page.cant_analyze_charts, page.library_charts);
    if (!page.cant_analyze_line.empty()) IM_CHECK(on_screen(h, page.cant_analyze_line));
    h.app->close_path_report();
    ctx->Yield(2);
}

// A chart whose failure the store kept (failure_record, saved on the scratch
// db the way a batch saves one, as library-cant-analyze does) is named under
// the coverage line's Show list, beneath its reason. The confirm is declined,
// so the report lists only what is stored: no rows, the coverage lines, and
// "Nothing to show" (plan 2026-10-15, question 5).
void test_cant_analyze_list(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string md5 = h.app->library_row_at(0).entry.md5;
    const std::string title = h.app->library_row_at(0).title;
    const hydra::NoNotesError no_notes(hydra::Difficulty::Expert, true);
    h.app->store->save(hydra::store::failure_record(md5, h.app->settings.profile(),
                                                    hydra::ErrorKind::NoNotes, no_notes.what()));
    h.app->refresh_library_summaries();

    open_path_report_declining_batch(ctx);
    IM_CHECK_RETV(path_report_ready(ctx), );
    IM_CHECK_RETV(h.app->path_report.result != nullptr, );
    const hydra::app::report::GeneratedReport& page = *h.app->path_report.result;
    IM_CHECK(page.paths.empty());
    IM_CHECK(on_screen(h, hydra::app::report::kNothingToShow));
    IM_CHECK(!page.cant_analyze_line.empty());
    IM_CHECK(on_screen(h, page.cant_analyze_line));
    IM_CHECK(!on_screen(h, "No Expert Pro Drums notes in this chart."));

    ctx->SetRef(path_window());
    ctx->ItemClick("**/Show list##cantanalyze");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "No Expert Pro Drums notes in this chart."));
    IM_CHECK(on_screen(h, title));
    IM_CHECK(on_screen(h, "Hide list"));
    h.app->close_path_report();
    ctx->Yield(2);
}

// Picking a player closes the picker and opens the comparison; the window's
// "Compare another player..." reopens the picker; Cancel while it builds
// closes the window and reopens the picker (D103 items 8 and 10).
void test_dm_handover(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    CannedDm dm(h.app->library_row_at(0).entry.md5, /*not_in_library_score=*/false,
                /*hold=*/false);

    compare_with_alice(ctx);
    IM_CHECK(!h.app->dm_picker_open);
    IM_CHECK_RETV(dm_window() != nullptr, );
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->dm_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60), );
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "1 of 1 score"));

    // A ready comparison's "Compare another player...".
    ctx->SetRef(dm_window());
    ctx->ItemClick("Compare another player...");
    ctx->Yield(2);
    IM_CHECK(h.app->dm_picker_open);
    IM_CHECK(on_screen(h, "Pick a player"));

    // Pick alice again with the scores fetch held: the window builds, and
    // its Cancel closes it and goes back to the list.
    g_fetch_release = false;
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemClick("**/###111");
    ctx->Yield(2);
    IM_CHECK(!h.app->dm_picker_open);
    IM_CHECK(h.app->dm_report_build() == hydra::ui::ReportBuild::Building);
    IM_CHECK_RETV(dm_window() != nullptr, );
    IM_CHECK(on_screen(h, "Fetching scores and building the report..."));
    ctx->SetRef(dm_window());
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(!h.app->dm_report.window_open);
    IM_CHECK(dm_window() == nullptr);
    IM_CHECK(h.app->dm_picker_open);
    IM_CHECK(on_screen(h, "Pick a player"));
    dm.release();
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!h.app->dm_picker_open);
}

// The path report lists the current chart mode's records only, and a row
// click selects its chart without switching the chart mode; a comparison row
// not in the library selects nothing.
void test_row_click(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string title = quoted_title(h, 0);
    const std::string md5 = h.app->library_row_at(0).entry.md5;
    auto selected_md5 = [&] { return h.app->selected ? h.app->selected->md5 : std::string(); };

    // The chart stored under two chart modes; the report lists the second.
    const std::string mode_a = h.app->settings.chartmode_key();
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);
    h.app->settings.view_prodrums = !h.app->settings.view_prodrums;
    h.app->commit_settings();
    const std::string mode_b = h.app->settings.chartmode_key();
    IM_CHECK(mode_a != mode_b);
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);

    // Optimal paths only leaves the one row of the current mode.
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, "1" + path_count_tail(h)));
    ctx->SetRef(path_window());
    ctx->ItemClick("**/1");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(selected_md5().c_str(), md5.c_str());
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), mode_b.c_str());
    h.app->close_path_report();  // it covers the toolbar
    ctx->Yield(2);

    // A comparison with a second score on a chart not in the library. With
    // the Status filter on "Not in your library", that score is row 1.
    CannedDm dm(md5, /*not_in_library_score=*/true, /*hold=*/false);
    compare_with_alice(ctx);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->dm_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60), );
    IM_CHECK_RETV(dm_window() != nullptr, );
    ctx->SetRef(dm_window());
    ctx->ComboClick("##status/Not in your library");
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "1 of 2 scores"));
    const std::string mode_before = h.app->settings.chartmode_key();
    ctx->SetRef(dm_window());
    ctx->ItemClick("**/1");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(selected_md5().c_str(), md5.c_str());  // unchanged
    IM_CHECK(h.app->settings.chartmode_key() == mode_before);
}

// The path window's count line for the report in memory, "n of M paths", as
// drawn this frame; empty when it isn't on screen.
std::string shown_count(Harness& h) {
    const std::string text = visible_text(h);
    const std::string tail = path_count_tail(h);
    const size_t at = text.find(tail);
    if (at == std::string::npos) return {};
    size_t start = at;
    while (start > 0 && (std::isdigit(static_cast<unsigned char>(text[start - 1])) ||
                         text[start - 1] == ','))
        --start;
    return text.substr(start, at + tail.size() - start);
}

// The first chart batched, its report built while the window is shut, then
// the window opened on that report. False when a step failed.
bool open_batch_report(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return false;
    IM_CHECK_RETV(batch_search(ctx, quoted_title(h, 0)), false);
    dismiss_done(ctx);

    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, false);
    IM_CHECK_RETV(h.app->report_job == nullptr, false);  // the batch's report, kept while shut
    IM_CHECK_RETV(h.app->path_report.result != nullptr, false);
    return true;
}

// A batch's report, built while the window is shut, shows at once. Esc then
// only hides the window and its rows stay (D271 item 7), so opening it again
// shows the same rows at once, with no build and no building state.
void test_reopen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    const std::string count = shown_count(h);
    IM_CHECK_RETV(!count.empty(), );
    const GeneratedReport* rows = h.app->path_report.result.get();

    press_escape_on(ctx, path_window());
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK(h.app->path_report.result != nullptr);
    IM_CHECK(h.app->path_report.result.get() == rows);

    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(!on_screen(h, "Building the report from your library..."));
    IM_CHECK(h.app->path_report.result.get() == rows);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), count.c_str());  // the same rows as before
}

// The path table's sort as ImGui holds it: table column index and direction,
// first key first. The window hands the table its view's sort.
std::vector<std::pair<int, int>> table_sort(ImGuiTestContext* ctx) {
    std::vector<std::pair<int, int>> out;
    ctx->SetRef(path_window());
    const ImGuiTableSortSpecs* specs = ctx->TableGetSortSpecs("##pathtable");
    if (!specs) return out;
    for (int i = 0; i < specs->SpecsCount; ++i)
        out.emplace_back(specs->Specs[i].ColumnIndex, static_cast<int>(specs->Specs[i].SortDirection));
    return out;
}

// A text box's or a dropdown's shown text as the frame text logs it.
std::string boxed(const std::string& s) { return "{ " + s + " }"; }

bool best_only_ticked(ImGuiTestContext* ctx) {
    ctx->SetRef(path_window());
    return (ctx->ItemInfo("Optimal paths only").StatusFlags & ImGuiItemStatusFlags_Checked) != 0;
}

// Closing the window and opening it again keeps its search, timing,
// Optimal paths only and sort (the user's answer to D103 item 28's
// follow-up, kept by D271 item 7). Refresh still resets all four.
void test_reopen_keeps_filters(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    // Read only before the Refresh below replaces these rows.
    const GeneratedReport& report = *h.app->path_report.result;
    const std::string default_count = shown_count(h);
    const auto default_sort = table_sort(ctx);
    IM_CHECK_RETV(!default_count.empty() && !default_sort.empty(), );
    IM_CHECK(best_only_ticked(ctx));

    // Sort by Song, then flip it: Z to A.
    ctx->SetRef(path_window());
    ctx->ItemClick("**/Song");
    ctx->Yield(2);
    ctx->ItemClick("**/Song");
    ctx->Yield(2);
    const auto sort = table_sort(ctx);
    IM_CHECK_RETV(!sort.empty() && sort != default_sort, );
    IM_CHECK_EQ(sort.front().second, static_cast<int>(ImGuiSortDirection_Descending));

    // Every path, then a search that keeps them (the chart's own song).
    ctx->SetRef(path_window());
    ctx->ItemClick("Optimal paths only");
    ctx->Yield(2);
    IM_CHECK(!best_only_ticked(ctx));
    const std::string all_count = shown_count(h);
    const std::string search = report.paths.front().song;
    ctx->SetRef(path_window());
    ctx->ItemInputValue("##search", search.c_str());
    ctx->Yield(2);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), all_count.c_str());
    const std::string search_shown = boxed(search);
    IM_CHECK(on_screen(h, search_shown));

    // A timing tier a path has, so the table stays up to show its sort.
    const auto choices = hydra::app::path_report_view::timing_choices();
    std::string label;
    for (size_t i = 1; i < choices.size() && label.empty(); ++i)
        for (const auto& r : report.paths)
            if (r.tier == *choices[i].tier) label = choices[i].label;
    IM_CHECK_RETV(!label.empty(), );
    ctx->SetRef(path_window());
    ctx->ComboClick(("##timing/" + label).c_str());
    ctx->Yield(2);
    const std::string count = shown_count(h);
    IM_CHECK_RETV(!count.empty() && count != default_count, );
    const std::string timing_shown = boxed(label);
    const std::string default_timing = boxed(choices.front().label);
    IM_CHECK(on_screen(h, timing_shown));

    // Esc only hides the window; opening it again shows it as it was left.
    press_escape_on(ctx, path_window());
    IM_CHECK(path_window() == nullptr);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK_STR_EQ(shown_count(h).c_str(), count.c_str());
    IM_CHECK(on_screen(h, search_shown));
    IM_CHECK(on_screen(h, timing_shown));
    IM_CHECK(!best_only_ticked(ctx));
    IM_CHECK(table_sort(ctx) == sort);

    // Refresh starts all four over (the rule before this change). Its batch
    // is declined too, so the rows stay the same chart's.
    open_path_report_declining_batch(ctx, /*refresh=*/true);
    IM_CHECK_RETV(path_report_ready(ctx), );
    ctx->Yield(1);
    IM_CHECK_STR_EQ(shown_count(h).c_str(), default_count.c_str());
    IM_CHECK(!on_screen(h, search_shown));
    IM_CHECK(on_screen(h, default_timing));
    IM_CHECK(best_only_ticked(ctx));
    IM_CHECK(table_sort(ctx) == default_sort);
}

// After a batch the report goes out of date with the batch's time; a setting
// the report reads gives the settings sentence; Refresh rebuilds and clears
// the strip.
void test_out_of_date(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(batch_search(ctx, quoted_title(h, 0)));
    dismiss_done(ctx);
    IM_CHECK(!h.app->path_report.batch_finished.has_value());

    // A batch that is stopped builds no report, so the one in memory stays,
    // out of date. The gate holds the run so Stop lands while it runs.
    {
        BatchGate gate;
        ctx->SetRef("//Hydra");
        ctx->ItemInputValue("**/##search", "");
        IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->search.empty(); }, 5), );
        start_library_batch(ctx);
        IM_CHECK_RETV(wait_until(ctx, [&] { return gate.started() >= 1; }, 30), );
        h.app->batch_job->stop();
        IM_CHECK_RETV(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300), );
    }
    ctx->Yield(3);
    IM_CHECK(h.app->report_job == nullptr);
    IM_CHECK(h.app->path_report.batch_finished.has_value());
    dismiss_done(ctx);

    const std::string library_line = "Your library changed since this report was built";
    const std::string settings_line = "The settings changed since this report was built.";
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, library_line + " (a batch finished at "));

    // A setting the report reads.
    h.app->settings.sp_cap = 5;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));
    IM_CHECK(!on_screen(h, library_line));

    // Nothing is analyzed at cap 5, so a report built there would fail.
    // Changing the cap back still leaves the rows out of date: the report
    // was built before both changes.
    h.app->settings.sp_cap = hydra::kCloneHeroSpCap;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));

    ctx->SetRef(path_window());
    ctx->ItemClick("**/Refresh##outofdate");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, settings_line));
    IM_CHECK(!on_screen(h, library_line));

    // The report lists one chart mode, so changing it (here Pro Drums) puts
    // the rows out of date too.
    h.app->settings.view_prodrums = !h.app->settings.view_prodrums;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(on_screen(h, settings_line));
}

}  // namespace

// The end-to-end scripts, registered by uitest_tests.cpp with no GUI function:
// the app draws the windows itself.
const std::vector<TestEntry>& report_window_flow_tests() {
    static const std::vector<TestEntry> entries = {
        {"report-window-open-path", test_open_path},
        {"report-window-open-analyzes", test_open_analyzes},
        {"report-window-open-asks", test_open_asks},
        {"report-window-cant-analyze-list", test_cant_analyze_list},
        {"report-window-dm-handover", test_dm_handover},
        {"report-window-row-click", test_row_click},
        {"report-window-reopen", test_reopen},
        {"report-window-reopen-keeps-filters", test_reopen_keeps_filters},
        {"report-window-out-of-date", test_out_of_date},
    };
    return entries;
}

void register_report_window_tests(Harness& h) {
    static const TestEntry kTests[] = {
        {"report-window-sort", test_sort},     {"report-window-filters", test_filters},
        {"report-window-states", test_states}, {"report-window-keys", test_keys},
        {"report-window-close-button", test_close_button},
        {"report-window-hidden-new-result", test_hidden_new_result},
        {"report-windows-both", test_both},
        {"report-window-chip-label", test_chip_label},
        {"report-window-column-widths", test_column_widths},
        {"report-window-cells-left", test_cells_left},
        {"report-window-header-narrow", test_header_narrow},
        {"report-window-fits-screen", test_fits_screen},
    };
    for (const TestEntry& e : kTests) {
        ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", e.name);
        t->UserData = &h;
        t->GuiFunc = draw_windows;
        t->TestFunc = e.fn;
    }
}

}  // namespace uitest
