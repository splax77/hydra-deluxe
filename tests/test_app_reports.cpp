// Tests for ui/app_state's two reports in memory (D103): the path report and
// the leaderboards comparison, how they build, cancel, close and go out of
// date, and the report that follows a batch. Split from test_app_click.cpp;
// the shared fixtures are in app_state_fixtures.h.

#include "doctest.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app_state_fixtures.h"
#include "corpus_util.h"
#include "library_fixtures.h"  // library_entry
#include "net/dmbot_client.h"   // set_fetcher
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "ui/app_state.h"
#include "ui/report_window.h"  // path_report_input
#include "wait_util.h"

using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::ui::AppState;
using testwait::wait_until;

// ---- the two reports in memory (D103) ---------------------------------------

namespace {

using hydra::ui::ReportBuild;
using hydra::ui::ReportOutOfDate;

// Waits for the path report's job, then collects it as the app's frame does.
void finish_path_report(AppState& app) {
    REQUIRE(app.report_job != nullptr);
    wait_until([&] { return app.report_job->finished(); }, "the path report's job to finish");
    app.update_background_jobs();
    REQUIRE(app.report_job == nullptr);
}

// The same for the comparison's job.
void finish_dm_report(AppState& app) {
    REQUIRE(app.dm_report_job != nullptr);
    wait_until([&] { return app.dm_report_job->finished(); }, "the comparison's job to finish");
    app.update_background_jobs();
    REQUIRE(app.dm_report_job == nullptr);
}

// A report hold that waits for `release` before the job reads the store.
// Declare `release` BEFORE the AppState: the app joins the job's thread in
// its destructor, so a failed check before the release must not destroy the
// flag while the job still reads it.
std::function<void()> held_report(std::atomic<bool>& release) {
    return [&release] {
        wait_until([&release] { return release.load(); }, "the test to release the report");
    };
}

// An app whose one chart has a stored result, so its path report has rows.
std::unique_ptr<AppState> app_with_result(const ScratchPaths& paths, const char* tag) {
    const ChartLibraryEntry song = corpus_chart(tag);
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    REQUIRE(app->viewed.ready());
    return app;
}

}  // namespace

TEST_CASE("a path report request builds it in memory, one build at a time") {
    ScratchPaths paths("appstate_pathreq");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "pathreq");
    CHECK(app->path_report_offered());
    CHECK(app->path_report_build() == ReportBuild::None);

    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->request_path_report();
    }
    REQUIRE(app->report_job != nullptr);
    const hydra::ui::ReportJob* first = app->report_job.get();
    CHECK(app->path_report_build() == ReportBuild::Building);
    app->request_path_report();  // one is running: nothing new starts
    CHECK(app->report_job.get() == first);
    release = true;
    finish_path_report(*app);

    CHECK(app->path_report_build() == ReportBuild::Ready);
    REQUIRE(app->path_report.result != nullptr);
    CHECK_FALSE(app->path_report.result->paths.empty());
    CHECK(app->path_report.built_at.time_since_epoch().count() != 0);
    CHECK(app->path_report.out_of_date == ReportOutOfDate::None);
    CHECK_FALSE(app->path_report.window_open);  // a request opens no window
}

TEST_CASE("showing the path report opens its window and builds it only the first time") {
    ScratchPaths paths("appstate_pathshow");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathshow");
    app->show_path_report();
    CHECK(app->path_report.window_open);
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    app->show_path_report();  // shown again while open
    CHECK(app->path_report.window_open);
    CHECK(app->report_job == nullptr);  // the rows in memory show; nothing rebuilds
}

// D103 item 28: closing the window (its X, Esc or Ctrl+W) lets the rows go,
// and reopening builds the report again.
TEST_CASE("closing the path report frees its rows, and reopening builds it again") {
    ScratchPaths paths("appstate_pathclose");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathclose");
    app->show_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    const std::weak_ptr<const hydra::app::report::GeneratedReport> rows = app->path_report.result;

    app->close_path_report();
    CHECK_FALSE(app->path_report.window_open);
    CHECK(app->path_report.result == nullptr);
    CHECK(rows.expired());  // nothing in AppState still holds them
    CHECK(app->path_report_build() == ReportBuild::None);

    app->show_path_report();
    CHECK(app->path_report.window_open);
    CHECK(app->path_report_build() == ReportBuild::Building);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->path_report.result != nullptr);
}

// The user's answer to item 28's follow-up (2026-10-09): rows built because
// the window opened keep its filters; Refresh's start them over.
TEST_CASE("a path report built on opening its window says so; Refresh's does not") {
    ScratchPaths paths("appstate_pathopenkeep");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathopenkeep");
    app->show_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    CHECK(app->path_report.built_on_open);
    CHECK(hydra::ui::path_report_input(*app).keep_filters);

    app->request_path_report();  // the window's Refresh
    finish_path_report(*app);
    CHECK_FALSE(app->path_report.built_on_open);
    CHECK_FALSE(hydra::ui::path_report_input(*app).keep_filters);

    // Closed and opened again: built on opening once more.
    app->close_path_report();
    CHECK_FALSE(app->path_report.built_on_open);
    app->show_path_report();
    finish_path_report(*app);
    CHECK(app->path_report.built_on_open);
}

TEST_CASE("closing the path report mid-build cancels the build, and nothing lands") {
    ScratchPaths paths("appstate_pathclosebuild");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "pathclosebuild");
    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->show_path_report();
    }
    REQUIRE(app->path_report_build() == ReportBuild::Building);

    app->close_path_report();
    CHECK(app->path_report_build() == ReportBuild::None);
    CHECK(app->report_job == nullptr);
    REQUIRE(app->parked_reports.size() == 1);
    CHECK(app->parked_reports.front()->is_cancelled());

    // The stopped job finishes; it never lands in the slot.
    release = true;
    wait_until([&] { return app->parked_reports.front()->finished(); },
               "the stopped report job to finish");
    app->update_background_jobs();
    CHECK(app->parked_reports.empty());
    CHECK(app->path_report.result == nullptr);
    CHECK(app->path_report_build() == ReportBuild::None);

    app->show_path_report();
    CHECK(app->path_report_build() == ReportBuild::Building);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
}

TEST_CASE("reopening the path report after a cancelled build builds it again") {
    ScratchPaths paths("appstate_pathclosecancelled");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "pathclosecancelled");
    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->show_path_report();
    }
    app->cancel_path_report();  // the window's Cancel
    release = true;
    finish_path_report(*app);
    REQUIRE(app->path_report_build() == ReportBuild::Cancelled);
    REQUIRE(app->path_report.result == nullptr);

    app->close_path_report();
    app->show_path_report();
    CHECK(app->path_report_build() == ReportBuild::Building);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
}

TEST_CASE("the path report goes out of date on a batch or a setting it was built from") {
    ScratchPaths paths("appstate_pathstale");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathstale");
    const std::string title = app->library_row_at(0).title;
    app->request_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.out_of_date == ReportOutOfDate::None);

    // A setting the report doesn't read leaves it alone.
    app->settings.auto_open_report = !app->settings.auto_open_report;
    app->commit_settings();
    CHECK(app->path_report.out_of_date == ReportOutOfDate::None);
    app->settings.auto_open_report = false;
    app->commit_settings();

    // The chart mode is one it reads: the page lists one profile (D115,
    // the user's answer 2).
    app->settings.view_prodrums = !app->settings.view_prodrums;
    app->commit_settings();
    CHECK(app->path_report.out_of_date == ReportOutOfDate::Settings);
    app->settings.view_prodrums = !app->settings.view_prodrums;
    app->commit_settings();
    app->request_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.out_of_date == ReportOutOfDate::None);

    // The SP cap is one it reads.
    app->settings.sp_cap = 5;
    app->commit_settings();
    CHECK(app->path_report.out_of_date == ReportOutOfDate::Settings);
    app->settings.sp_cap = 4;
    app->commit_settings();

    // A batch finishing after it: the later event wins, and the batch's own
    // report starts. The slot keeps the moment the batch's end was seen, for
    // the window's "(a batch finished at HH:MM)" (D103 item 21).
    CHECK_FALSE(app->path_report.batch_finished.has_value());
    run_redo_batch_over(*app, title);
    const auto before_end = std::chrono::system_clock::now();
    app->update_background_jobs();
    const auto after_end = std::chrono::system_clock::now();
    CHECK(app->path_report.out_of_date == ReportOutOfDate::Library);
    REQUIRE(app->path_report.batch_finished.has_value());
    CHECK(*app->path_report.batch_finished >= before_end);
    CHECK(*app->path_report.batch_finished <= after_end);
    finish_path_report(*app);
    CHECK(app->path_report.out_of_date == ReportOutOfDate::None);  // the rebuild is current
    CHECK_FALSE(app->path_report.window_open);           // Open automatically is off

    // With Open automatically on, the batch's report opens its window.
    app->batch_job.reset();  // the finished strip's X
    app->settings.auto_open_report = true;
    app->commit_settings();
    run_redo_batch_over(*app, title);
    app->update_background_jobs();
    finish_path_report(*app);
    CHECK(app->path_report.window_open);
    CHECK(app->status_message.empty());  // the strip is up, so it shows the outcome
}

TEST_CASE("a setting changed during a build leaves that build out of date") {
    ScratchPaths paths("appstate_pathduring");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "pathduring");
    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->request_path_report();
    }
    app->settings.mslimit_enabled = !app->settings.mslimit_enabled;
    app->commit_settings();
    release = true;
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    CHECK(app->path_report.out_of_date == ReportOutOfDate::Settings);
}

TEST_CASE("Cancel ends a path report build as cancelled and keeps the last report") {
    ScratchPaths paths("appstate_pathcancel");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "pathcancel");
    app->request_path_report();
    finish_path_report(*app);
    const auto kept = app->path_report.result;
    REQUIRE(kept != nullptr);

    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->request_path_report();
    }
    app->cancel_path_report();
    release = true;
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Cancelled);
    CHECK(app->path_report.result == kept);
}

// D103 item 3: a report row's click selects its chart, switching the
// settings bar to the row's mode first.
TEST_CASE("select_chart switches the mode first, then selects the first copy") {
    ScratchPaths paths("appstate_selectchart");
    std::unique_ptr<AppState> app = app_on(paths);
    // A second copy of chart 7, in another folder, sorting after it by title.
    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < kChartCount; ++i) charts.push_back(library_entry(i));
    ChartLibraryEntry copy = library_entry(7);
    copy.title += " copy";
    copy.notespath = "C:\\other\\hash007\\notes.chart";
    charts.push_back(copy);
    app->store->rebuild_chart_library(charts);
    app->reload_library();

    Settings target;
    target.view_difficulty = "Hard";
    target.view_prodrums = false;
    target.view_bass2x = true;
    const std::string mode = target.chartmode_key();
    REQUIRE(mode != app->settings.chartmode_key());

    app->select_chart("hash007", mode);
    CHECK(app->settings.chartmode_key() == mode);
    CHECK(Settings::load_file(paths.ini).chartmode_key() == mode);  // committed
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == library_entry(7).notespath);

    // The first copy in the library's order: sorted the other way, the copy.
    app->library.set_sort(hydra::ui::LibrarySort::Title, false);
    app->select_chart("hash007", mode);
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == copy.notespath);

    // A hash the library doesn't list selects nothing new.
    app->select_chart("not-in-library", mode);
    CHECK(app->selected->notespath == copy.notespath);
}

namespace {

// The canned leaderboard: one player, whose one score is chart 0's.
const char* const kDmUsersJson =
    R"([{"id":"111","username":"alice","elo":1500,"stats":{"total_scores":1,"total_score":1}}])";
const char* const kDmScoresJson =
    R"({"scores":[{"identifier":"hash000","song_name":"x","artist":"y","charter_refs":["z"],)"
    R"("score":100000,"is_fc":0,"percent":95,"speed":100,"rank":1,"posted":"2026-01-01"}],)"
    R"("unknown_scores":[]})";

}  // namespace

// D103 items 8 to 11: picking a player opens the comparison window and
// builds it; Refresh builds it again for the same player; Cancel keeps the
// last comparison.
TEST_CASE("picking a player opens the comparison; Refresh reuses the player") {
    ScratchPaths paths("appstate_dmreport");
    // Before the app, which joins the fetch's thread: the fetcher reads them.
    std::mutex m;
    std::vector<std::string> score_urls;
    std::atomic<bool> hold{false};
    // After what it reads, before the app: the fetch's thread calls it.
    const ScopedHook fetcher(hydra::net::set_fetcher, [&](const std::string& url,
                                                          const std::atomic<bool>* cancel)
                                                          -> std::string {
        if (url.find("/all-users") != std::string::npos) return kDmUsersJson;
        {
            std::lock_guard<std::mutex> lock(m);
            score_urls.push_back(url);
        }
        wait_until([&] { return !hold.load() || cancel->load(); },
                   "the test to stop holding or cancel the fetch");
        if (cancel->load()) throw std::runtime_error("cancelled");
        return kDmScoresJson;
    });
    std::unique_ptr<AppState> app = app_on(paths);

    app->start_dm_report("111", "alice");
    CHECK(app->dm_report.window_open);
    CHECK(app->settings.dm_last_user == "111");
    finish_dm_report(*app);
    CHECK(app->dm_report_build() == ReportBuild::Ready);
    REQUIRE(app->dm_report.result != nullptr);
    CHECK(app->dm_report.result->username == "alice");
    CHECK(app->dm_report.result->stats.total == 1);

    // The chart mode is a setting the comparison reads.
    app->settings.view_prodrums = !app->settings.view_prodrums;
    app->commit_settings();
    CHECK(app->dm_report.out_of_date == ReportOutOfDate::Settings);

    // Refresh: the same player again, and the result is current.
    app->dm_report.window_open = false;
    app->request_dm_report();
    finish_dm_report(*app);
    REQUIRE(score_urls.size() == 2);
    CHECK(score_urls[1] == score_urls[0]);
    CHECK(score_urls[1].find("111") != std::string::npos);
    CHECK(app->dm_report.out_of_date == ReportOutOfDate::None);
    CHECK_FALSE(app->dm_report.window_open);  // Refresh opens no window

    // Cancel while fetching: cancelled, and the last comparison is kept.
    const auto kept = app->dm_report.result;
    hold = true;
    app->request_dm_report();
    REQUIRE(app->dm_report_build() == ReportBuild::Building);
    app->cancel_dm_report();
    CHECK(app->dm_report_build() == ReportBuild::Cancelled);
    CHECK(app->dm_report.result == kept);
    wait_until(
        [&] {
            app->update_background_jobs();
            return app->parked_dm_reports.empty();
        },
        "the cancelled comparison to be collected");

    // The window's "Compare another player..." opens the picker again.
    app->dm_picker_open = false;
    app->reopen_dm_picker();
    CHECK(app->dm_picker_open);
    CHECK(app->dm_picker_popup_pending);
    if (app->dm_fetch_job)
        wait_until([&] { return app->dm_fetch_job->finished(); }, "the player list's fetch to finish");
}

// ---- opening the report analyzes first (plan 2026-10-15, question 8) --------

namespace {

// A batch analyzer that waits for `release`, then fails the chart, so the
// batch stores nothing and the test reads its snapshot while it is held.
hydra::app::ChartAnalyzer held_failing_analyzer(std::atomic<bool>& release) {
    return [&release](const std::string&, const hydra::app::AnalysisSettings&,
                      const std::function<void(float)>&) -> hydra::app::AnalysisResult {
        wait_until([&release] { return release.load(); }, "the test to release the batch");
        throw std::runtime_error("no chart file in this test");
    };
}

// The charts beside the analyzed one that have no result: the rows an
// opening's batch runs.
constexpr int kUnanalyzed = 5;

// An app whose one corpus chart has a stored result, so its report has rows,
// and kUnanalyzed seeded charts with no file and no result.
std::unique_ptr<AppState> app_with_unanalyzed(const ScratchPaths& paths, const char* tag) {
    const ChartLibraryEntry song = corpus_chart(tag);
    std::vector<ChartLibraryEntry> charts{song};
    for (int i = 1; i <= kUnanalyzed; ++i) charts.push_back(library_entry(i));
    std::unique_ptr<AppState> app = app_with(paths, charts);
    click(*app, song);
    REQUIRE(app->viewed.ready());
    return app;
}

}  // namespace

TEST_CASE("opening the path report runs a batch over the charts with no result, then builds") {
    ScratchPaths paths("appstate_pathopenbatch");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopenbatch");
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_failing_analyzer(release), 1);
        app->show_path_report();
    }
    REQUIRE(app->batch_job != nullptr);
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report.window_open);
    CHECK(app->path_report_build() == ReportBuild::Building);
    CHECK(app->path_report_batch() == app->batch_job.get());
    CHECK_FALSE(app->report_started);

    // The window's numbers are the batch snapshot's, the strip's numbers.
    wait_until([&] { return app->batch_job->snapshot().total == kUnanalyzed; },
               "the batch's count");
    hydra::ui::PathReportInput held = hydra::ui::path_report_input(*app);
    const hydra::ui::BatchJob::Snapshot s = app->batch_job->snapshot();
    REQUIRE(held.batch.has_value());
    CHECK(held.state == ReportBuild::Building);
    CHECK(held.batch->completed == s.completed);
    CHECK(held.batch->total == s.total);
    CHECK(held.batch->total == kUnanalyzed);

    release = true;
    wait_batch_finished(*app);
    const hydra::ui::BatchJob::Snapshot done = app->batch_job->snapshot();
    const hydra::ui::PathReportInput finished = hydra::ui::path_report_input(*app);
    REQUIRE(finished.batch.has_value());
    CHECK(finished.batch->completed == done.completed);
    CHECK(finished.batch->completed == kUnanalyzed);

    // The batch's finish starts the report, with the window's cause.
    app->update_background_jobs();
    CHECK(app->path_report_batch() == nullptr);
    CHECK(app->report_started);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->path_report.built_on_open);
}

TEST_CASE("the window's Cancel stops the report's own batch, and no report follows") {
    ScratchPaths paths("appstate_pathopencancel");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopencancel");
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_failing_analyzer(release), 1);
        app->show_path_report();
    }
    REQUIRE(app->batch_job != nullptr);
    app->cancel_path_report();  // the window's Cancel
    CHECK(app->batch_job->is_cancelled());
    CHECK(app->path_report_build() == ReportBuild::Cancelled);
    release = true;
    wait_batch_finished(*app);
    app->update_background_jobs();
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report_build() == ReportBuild::Cancelled);
}

// The user's answer 8: more than kReportConfirmAbove rows to run asks first,
// in the report window itself. The test library has kUnanalyzed to run, so
// the test lowers the threshold the app reads.
TEST_CASE("more rows to run than the threshold open the batch confirm first") {
    CHECK(AppState::kReportConfirmAbove == 500);
    ScratchPaths paths("appstate_pathopenconfirm");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopenconfirm");

    // At the threshold it just runs.
    app->report_confirm_above = kUnanalyzed;
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_failing_analyzer(release), 1);
        app->show_path_report();
    }
    REQUIRE(app->batch_job != nullptr);
    CHECK_FALSE(app->batch_confirm_pending);
    release = true;
    wait_batch_finished(*app);
    app->update_background_jobs();
    finish_path_report(*app);
    app->close_path_report();
    app->batch_job.reset();  // the finished strip's X

    // Over it, the report window asks on the same plan and nothing runs yet.
    // The question is the report's own: the main window's popup stays shut.
    app->report_confirm_above = kUnanalyzed - 1;
    app->show_path_report();
    CHECK(app->batch_job == nullptr);
    CHECK(app->batch_plan.todo_rows() == kUnanalyzed);
    CHECK(app->path_report_awaits_confirm());
    CHECK_FALSE(app->batch_confirm_in_main_window());
    CHECK(app->path_report_build() == ReportBuild::Building);
    {
        const hydra::ui::PathReportInput asking = hydra::ui::path_report_input(*app);
        CHECK(asking.confirm_question == "Analyze 5 charts that have no result yet?");
        CHECK(asking.confirm_question == app->batch_question(false));
        CHECK(asking.can_analyze);
    }

    // Its Analyze runs the batch, which the report then waits on.
    release = false;
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_failing_analyzer(release), 1);
        hydra::ui::path_report_input(*app).callbacks.analyze();
    }
    REQUIRE(app->batch_job != nullptr);
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK(app->path_report_batch() == app->batch_job.get());
    CHECK(hydra::ui::path_report_input(*app).confirm_question.empty());
    release = true;
    wait_batch_finished(*app);
    app->update_background_jobs();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    app->close_path_report();
    app->batch_job.reset();

    // Its Cancel is "don't analyze": nothing runs, and the report builds from
    // the results already stored rather than ending cancelled.
    app->show_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    hydra::ui::path_report_input(*app).callbacks.cancel();
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK(app->report_job != nullptr);
    app->update_background_jobs();
    CHECK(app->batch_job == nullptr);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    app->close_path_report();

    // Closing the window while it asks drops the question: nothing runs and
    // no popup takes it up in the main window.
    app->show_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    app->close_path_report();
    app->update_background_jobs();
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->batch_confirm_in_main_window());
    CHECK(app->batch_job == nullptr);
    CHECK(app->path_report_build() == ReportBuild::None);
}

TEST_CASE("a batch already running is waited on, and its finish builds the report") {
    ScratchPaths paths("appstate_pathopenwait");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopenwait");
    start_redo_batch(*app, held_failing_analyzer(release));
    app->show_path_report();
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report_batch() == app->batch_job.get());
    CHECK(app->path_report_build() == ReportBuild::Building);
    release = true;
    wait_batch_finished(*app);
    app->update_background_jobs();
    finish_path_report(*app);
    CHECK(app->path_report.built_on_open);
}

TEST_CASE("the toolbar offers the path report whenever the library has rows") {
    ScratchPaths paths("appstate_pathoffered");
    CHECK(app_on(paths)->path_report_offered());
    ScratchPaths empty("appstate_pathofferednone");
    CHECK_FALSE(app_with(empty, {})->path_report_offered());
}

// ---- the report that follows a batch ----------------------------------------

// The report that follows a batch lists what the batch analyzed, even when
// the settings bar moved on after it (D51 call 26).
TEST_CASE("the post-batch report lists the batch's cap and lens, not the live settings") {
    ScratchPaths paths("appstate_reportrun");
    std::unique_ptr<AppState> app = app_on(paths);
    REQUIRE(app->settings.sp_cap == 4);
    run_redo_batch_over(*app, library_entry(5).md5);
    const hydra::store::Lens batch_lens = app->batch_job->batch_run().profile.lens;

    app->settings.sp_cap = 5;  // edited, not committed
    app->settings.mslimit_enabled = !app->settings.mslimit_enabled;
    app->update_background_jobs();

    REQUIRE(app->report_job != nullptr);
    CHECK(app->report_job->profile().cap == hydra::store::CapQuery::at(4));
    CHECK(app->report_job->profile().lens == batch_lens);
    CHECK_FALSE(app->report_job->profile().lens == app->settings.lens());
    wait_until([&] { return app->report_job->finished(); }, "the batch's report job to finish");
}

// D103 item 28's 2026-10-09 answer: the finished strip shows "Open
// automatically" from report_started and the build state alone. These pin
// what the strip reads on the paths that matter: a batch's report cancelled
// in the window then refreshed, a close, and a new batch.
TEST_CASE("the strip's report_started holds through a cancel, a refresh and a close") {
    ScratchPaths paths("appstate_batchlanded");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "batchlanded");
    const std::string title = app->library_row_at(0).title;
    CHECK_FALSE(app->report_started);

    // The batch's own report is cancelled in the window, then refreshed: the
    // refresh's rows are the strip's to show, though its cause is Request.
    run_redo_batch_over(*app, title);
    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test,
                              held_report(release));
        app->update_background_jobs();  // starts the batch's own report, held
    }
    REQUIRE(app->report_job != nullptr);
    app->cancel_path_report();
    release = true;
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Cancelled);
    CHECK(app->report_started);
    app->request_path_report();  // the window's Refresh
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->report_started);

    app->close_path_report();
    CHECK(app->path_report_build() == ReportBuild::None);
    CHECK(app->report_started);

    app->batch_job.reset();  // the finished strip's X
    run_redo_batch_over(*app, title);
    CHECK_FALSE(app->report_started);  // the new run's report has not started
}

