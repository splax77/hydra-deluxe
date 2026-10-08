// The report windows' GUI tests (report windows plan, task 5). Each test
// opens the path report or the comparison window on the sample rows
// (tests/report_samples.h) through ReportWindowInput, the struct AppState
// fills in the app, and drives the window by its labels. The test's own GUI
// function draws the windows, so no app state is involved; the end-to-end
// tests through AppState come with task 6. Under hydra_uitest there is no
// platform backend, so each report window draws inside the main viewport.

#include "uitest_harness.h"

#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "../report_samples.h"  // tests/ is not on the runner's include path
#include "imgui_internal.h"
#include "app/dm_report.h"
#include "app/report.h"
#include "ui/report_window.h"

namespace uitest {

namespace {

using hydra::app::dm_report::GeneratedDmReport;
using hydra::app::report::GeneratedReport;
using hydra::ui::DmReportInput;
using hydra::ui::PathReportInput;
using hydra::ui::ReportOutOfDate;
using hydra::ui::ReportState;

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
    f.path.state = ReportState::Ready;
    f.path.built = std::chrono::system_clock::now();
    f.path.callbacks = recording_callbacks(f.path_clicks);
    f.dm.result = sample_dm_result();
    f.dm.state = ReportState::Ready;
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
    f.path.state = ReportState::Building;
    f.path.progress_done = 3;
    f.path.progress_total = 10;
    f.path_open = true;
    ctx->Yield(3);
    IM_CHECK(shows("Analyzing 3 of 10 charts"));
    // The subtitle while it builds (D103 item 21, board 2a), not the old
    // result's.
    IM_CHECK(shows("Building the report from your library..."));
    IM_CHECK(!shows("sample path subtitle"));
    ctx->SetRef(path_window());
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 1);

    f.path.state = ReportState::Cancelled;
    ctx->Yield(2);
    IM_CHECK(shows("Report cancelled."));
    ctx->ItemClick("Try again");
    IM_CHECK_EQ(f.retries, 1);

    f.path.state = ReportState::Failed;
    f.path.failure_message = "sample failure message";
    f.path.failure_error = "sample failure error";
    ctx->Yield(2);
    IM_CHECK(shows("The path report could not be built."));
    IM_CHECK(shows("sample failure message"));
    IM_CHECK(shows("sample failure error"));

    f.path.state = ReportState::Ready;
    f.path.out_of_date = ReportOutOfDate::Library;
    ctx->Yield(2);
    IM_CHECK(shows("Your library changed since this report was built."));
    IM_CHECK(shows("Built "));
    ctx->ItemClick("Refresh##outofdate");
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
    ctx->ItemClick("Show files");
    ctx->Yield(2);
    IM_CHECK(shows("Song D\\notes.chart"));

    // Nothing to report: the report's empty reason.
    auto empty = std::make_shared<GeneratedReport>();
    empty->why_empty = "sample empty reason";
    f.path.result = empty;
    ctx->Yield(2);
    IM_CHECK(shows("sample empty reason"));
    f.path_open = false;

    f.dm.state = ReportState::Building;
    open_dm(ctx);
    IM_CHECK(shows("Fetching scores and building the report..."));
    IM_CHECK(!shows("Building the report from your library..."));
    IM_CHECK(!shows("sample dm subtitle"));
    IM_CHECK(shows("The leaderboard server can take a moment to wake up."));
    ctx->ItemClick("Cancel");
    IM_CHECK_EQ(f.cancels, 2);
    f.dm.state = ReportState::Failed;
    ctx->Yield(2);
    IM_CHECK(shows("Could not build the report."));
    ctx->ItemClick("Compare another player...");
    IM_CHECK_EQ(f.compares, 1);
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
    // Reopening shows the rows at once: nothing is rebuilt.
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

}  // namespace

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
