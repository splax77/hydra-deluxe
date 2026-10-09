// The report windows' GUI tests. The first five (report windows plan, task 5)
// open the path report or the comparison window on the sample rows
// (tests/report_samples.h) through ReportWindowInput, the struct AppState
// fills in the app, and drive the window by its labels. Their own GUI
// function draws the windows; only report-window-states' analysis-off step
// fills the input from AppState (path_report_input). The rest (task 6)
// run end to end: the app draws both windows itself from run_frame while a
// slot's window_open is set, so they have no GUI function. Under hydra_uitest
// there is no platform backend, so each report window draws inside the main
// viewport, over the main window's rectangle (D103 item 14): close it before
// clicking the toolbar or the finished strip.

#include "uitest_harness.h"

#include <atomic>
#include <cctype>
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../report_samples.h"  // tests/ is not on the runner's include path
#include "imgui_internal.h"
#include "app/analysis.h"
#include "app/dm_report.h"
#include "app/path_report_view.h"  // timing_choices
#include "app/report.h"
#include "core/model.h"  // kCloneHeroSpCap
#include "net/dmbot_client.h"
#include "ui/library_jobs.h"  // set_report_analyzer_for_test
#include "ui/report_window.h"

namespace uitest {

// Defined in uitest_batch_reports.cpp.
bool batch_search(ImGuiTestContext* ctx, const std::string& search);
void dismiss_done(ImGuiTestContext* ctx);

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
};

Fixture& fixture() {
    static Fixture f;
    return f;
}

std::shared_ptr<GeneratedReport> sample_path_result() {
    auto r = std::make_shared<GeneratedReport>();
    r->paths = report_samples::sample_path_rows();
    r->rows = static_cast<int64_t>(r->paths.size());
    r->hit_window_ms = report_samples::kSampleHitWindowMs;
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

// The test's GUI function: both windows, every frame, with their text kept.
void draw_windows(ImGuiTestContext*) {
    Fixture& f = fixture();
    ImGui::LogToBuffer();
    hydra::ui::draw_path_report_window(&f.path_open, f.path);
    hydra::ui::draw_dm_report_window(&f.dm_open, f.dm);
    ImGuiContext& g = *ImGui::GetCurrentContext();
    f.text.assign(g.LogBuffer.begin(), g.LogBuffer.end());
    ImGui::LogFinish();
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

// Opens the path report and points the ref at it, with Best path only
// unticked: none of the sample rows is an optimal path.
void open_path_all_rows(ImGuiTestContext* ctx) {
    fixture().path_open = true;
    ctx->Yield(3);
    IM_CHECK(path_window() != nullptr);
    if (ctx->IsError()) return;
    ctx->SetRef(path_window());
    ctx->ItemClick("Best path only");
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
    // Best path only starts ticked, and no sample row is optimal.
    IM_CHECK(shows("Nothing matches those filters."));
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

// Filter to nothing, then Clear filters.
void test_filters(ImGuiTestContext* ctx) {
    reset_app(harness(ctx));
    reset_fixture();
    open_path_all_rows(ctx);
    ctx->ItemInputValue("##search", "zzz");
    ctx->Yield(2);
    IM_CHECK(shows("0 of 6 paths"));
    IM_CHECK(shows("Nothing matches those filters."));
    ctx->ItemClick("Clear filters");
    ctx->Yield(2);
    // The search is empty again and Best path only stayed unticked.
    IM_CHECK(shows("6 of 6 paths"));

    open_dm(ctx);
    ctx->ComboClick("##status/Not in your library");
    ctx->Yield(2);
    IM_CHECK(shows("1 of 4 scores"));
    ctx->ItemInputValue("##search", "zzz");
    ctx->Yield(2);
    IM_CHECK(shows("0 of 4 scores"));
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
    f.path.progress_done = 3;
    f.path.progress_total = 10;
    f.path_open = true;
    ctx->Yield(3);
    // One per chart and mode still to analyze (D103 item 25).
    IM_CHECK(shows("Analyzing 3 of 10 records"));
    // The subtitle while it builds (D103 item 21, board 2a), not the old
    // result's.
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK(!shows("sample path subtitle"));
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 1);
    // Nothing left to analyze (D103 item 26): the bar moves with no count,
    // under the building subtitle, and Cancel stays.
    f.path.progress_done = 0;
    f.path.progress_total = 0;
    ctx->Yield(2);
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK(!shows("Analyzing "));
    IM_CHECK(!shows("0 of 0"));
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 2);

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

    // Left-out charts: the report's own sentence, the files under Show files.
    auto left_out = sample_path_result();
    left_out->failures.push_back({"D:\\Songs\\Song D\\notes.chart", "sample error"});
    f.path.result = left_out;
    ctx->Yield(2);
    IM_CHECK(shows(hydra::app::report::left_out_line(left_out->failures)));
    IM_CHECK(!shows("Song D\\notes.chart"));
    ctx->ItemClick("**/Show files");  // inside the strip's child
    ctx->Yield(2);
    IM_CHECK(shows("Song D\\notes.chart"));

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
    IM_CHECK_EQ(f.cancels, 3);
    f.dm.state = ReportBuild::Failed;
    ctx->Yield(2);
    IM_CHECK(shows("Could not build the report."));
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

    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
    IM_CHECK(!f.path_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK_EQ(f.closes, 1);
    // Reopening, the window takes the rows up again. They come as AppState
    // hands rows built for an opening (keep_filters), so Best path only
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

// Holds every path report build at its first chart until release(), so a
// test can look at the building state. The job copies the analyzer when it
// is made; the destructor lets any held build go and removes the seam.
std::atomic<bool> g_report_release{true};
class ReportHold {
public:
    ReportHold() {
        g_report_release = false;
        hydra::ui::set_report_analyzer_for_test(
            [](const std::string& path, const hydra::app::AnalysisSettings& s,
               const std::function<void(float)>& on_progress) {
                while (!g_report_release.load())
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return hydra::app::analyze_chart_file(path, s, on_progress);
            });
    }
    ~ReportHold() { release(); }
    ReportHold(const ReportHold&) = delete;
    ReportHold& operator=(const ReportHold&) = delete;
    void release() {
        g_report_release = true;
        hydra::ui::set_report_analyzer_for_test({});
    }
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
            while (!g_fetch_release.load() && !(cancel && cancel->load()))
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

// A path report build just started under `hold`: the window counts the
// records from 0 (and shows `also_while_building` when given), then, once
// released, shows the rows with no bar.
void held_build_then_rows(ImGuiTestContext* ctx, ReportHold& hold,
                          const char* also_while_building = nullptr) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job && h.app->report_job->progress().second > 0;
    }, 10), );
    const int total = h.app->report_job->progress().second;
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Analyzing 0 of " + std::to_string(total) + " record"));
    if (also_while_building) IM_CHECK(on_screen(h, also_while_building));
    hold.release();
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, "Analyzing "));
    IM_CHECK(on_screen(h, path_count_tail(h)));
}

// The path window opens from the finished strip's "Open report", from the
// toolbar's "Open path report", and by itself after a batch with "Open
// automatically" on. While it builds it counts the charts.
void test_open_path(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string first = quoted_title(h, 0);
    const std::string second = quoted_title(h, 1);

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

    // The toolbar's Open path report.
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    ctx->Yield(2);
    IM_CHECK_RETV(path_window() != nullptr, );
    // The Esc above let the rows go, so this opening builds them again.
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    ctx->Yield(2);
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

    // Refresh with every build held at its first chart: the window counts
    // the charts from 0.
    ReportHold hold;
    ctx->SetRef(path_window());
    ctx->ItemClick("Refresh");
    held_build_then_rows(ctx, hold);
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

// A row click selects its chart; a row of the other chart mode switches the
// analysis settings first; a comparison row not in the library selects nothing.
void test_row_click(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const std::string title = quoted_title(h, 0);
    const std::string md5 = h.app->library_row_at(0).entry.md5;
    auto selected_md5 = [&] { return h.app->selected ? h.app->selected->md5 : std::string(); };

    // The chart under two chart modes, so the report holds it twice.
    const std::string mode_a = h.app->settings.chartmode_key();
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);
    h.app->settings.view_prodrums = !h.app->settings.view_prodrums;
    h.app->commit_settings();
    const std::string mode_b = h.app->settings.chartmode_key();
    IM_CHECK(mode_a != mode_b);
    IM_CHECK(batch_search(ctx, title));
    dismiss_done(ctx);

    // Best path only leaves one row per mode.
    h.app->show_path_report();
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK(on_screen(h, "2" + path_count_tail(h)));
    ctx->SetRef(path_window());
    ctx->ItemClick("**/1");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(selected_md5().c_str(), md5.c_str());
    const std::string first_mode = h.app->settings.chartmode_key();
    IM_CHECK(first_mode == mode_a || first_mode == mode_b);
    ctx->SetRef(path_window());
    ctx->ItemClick("**/2");
    ctx->Yield(2);
    IM_CHECK_STR_EQ(selected_md5().c_str(), md5.c_str());
    IM_CHECK(h.app->settings.chartmode_key() != first_mode);
    IM_CHECK(h.app->settings.chartmode_key() == mode_a || h.app->settings.chartmode_key() == mode_b);
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
// closes the window and lets its rows go; opening it again builds the report
// again behind the building bar (D103 item 28).
void test_reopen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    const std::string count = path_count_tail(h);
    IM_CHECK(on_screen(h, count));

    press_escape_on(ctx, path_window());
    IM_CHECK(!h.app->path_report.window_open);
    IM_CHECK(path_window() == nullptr);
    IM_CHECK(h.app->path_report.result == nullptr);

    // Every build held at its first chart, so the bar stays up to be read.
    ReportHold hold;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    held_build_then_rows(ctx, hold, "Building the report from your library...");
    IM_CHECK(path_window() != nullptr);
    IM_CHECK(on_screen(h, count));  // the same rows as before the close
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
    return (ctx->ItemInfo("Best path only").StatusFlags & ImGuiItemStatusFlags_Checked) != 0;
}

// Closing the window and opening it again keeps its search, timing,
// Best path only and sort, though the rows are built again (the user's
// answer to D103 item 28's follow-up). Refresh still resets all four.
void test_reopen_keeps_filters(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_batch_report(ctx)) return;
    // Read only before the Esc below lets these rows go.
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
    ctx->ItemClick("Best path only");
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
    const auto choices = hydra::app::path_report_view::timing_choices(report.hit_window_ms);
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

    // Esc lets the rows go; opening again builds them, filters kept.
    press_escape_on(ctx, path_window());
    IM_CHECK_RETV(h.app->path_report.result == nullptr, );
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Open path report");
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60), );
    ctx->Yield(3);
    IM_CHECK_RETV(path_window() != nullptr, );
    IM_CHECK_STR_EQ(shown_count(h).c_str(), count.c_str());
    IM_CHECK(on_screen(h, search_shown));
    IM_CHECK(on_screen(h, timing_shown));
    IM_CHECK(!best_only_ticked(ctx));
    IM_CHECK(table_sort(ctx) == sort);

    // Refresh starts all four over (the rule before this change).
    ctx->SetRef(path_window());
    ctx->ItemClick("Refresh");
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->report_job == nullptr &&
               h.app->path_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60), );
    ctx->Yield(3);
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
        ctx->ItemClick("Analyze library...");
        ctx->SetRef("//Analyze library");
        ctx->ItemClick("Start analyzing");
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
}

}  // namespace

// The end-to-end scripts, registered by uitest_tests.cpp with no GUI function:
// the app draws the windows itself.
const std::vector<TestEntry>& report_window_flow_tests() {
    static const std::vector<TestEntry> entries = {
        {"report-window-open-path", test_open_path},
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
        {"report-windows-both", test_both},
    };
    for (const TestEntry& e : kTests) {
        ImGuiTest* t = IM_REGISTER_TEST(h.engine, "hydra", e.name);
        t->UserData = &h;
        t->GuiFunc = draw_windows;
        t->TestFunc = e.fn;
    }
}

}  // namespace uitest
