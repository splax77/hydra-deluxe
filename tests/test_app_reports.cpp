// Tests for ui/app_state's two reports in memory (D103): the path report and
// the leaderboards comparison, how they build, cancel, close and go out of
// date, and the report that follows a batch. The shared fixtures are in
// app_state_fixtures.h.

#include "doctest.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dm_report.h"  // GeneratedDmReport
#include "app/user_messages.h"  // plain_error
#include "app_state_fixtures.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // save_batch_record
#include "library_fixtures.h"  // library_entry
#include "net/dmbot_client.h"   // set_fetcher
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "store_file_fixtures.h"  // HeldSaveGroup
#include "ui/app_state.h"
#include "ui/report_window.h"  // path_report_input, dm_report_input
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

// Hydra.exe --open-report: the window opens at the first tick the toolbar's
// button shows, once, and a close afterwards stays closed.
TEST_CASE("--open-report opens the path report once the button shows, and only once") {
    ScratchPaths paths("appstate_pathopenflag");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathopenflag");
    app->open_path_report_once_shown();

    // Before the startup open brings the store, the main window and its
    // toolbar don't draw, so nothing opens.
    std::unique_ptr<hydra::store::RecordStore> held = std::move(app->store);
    REQUIRE_FALSE(app->path_report_button_shown());
    app->tick(0.0);
    CHECK_FALSE(app->path_report.window_open);
    CHECK(app->report_job == nullptr);

    app->store = std::move(held);
    REQUIRE(app->path_report_button_shown());
    app->tick(0.0);
    CHECK(app->path_report.window_open);
    finish_path_report(*app);
    CHECK(app->path_report.built_on_open);  // opened as the button opens it

    app->close_path_report();
    app->tick(0.0);
    app->tick(0.0);
    CHECK_FALSE(app->path_report.window_open);
}

// Without the option, the button showing opens nothing.
TEST_CASE("the path report stays shut at a tick without --open-report") {
    ScratchPaths paths("appstate_pathnoflag");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathnoflag");
    REQUIRE(app->path_report_button_shown());
    app->tick(0.0);
    CHECK_FALSE(app->path_report.window_open);
    CHECK(app->report_job == nullptr);
}

// D271 item 7: closing the window (its X, Esc or Ctrl+W) only hides it, as
// the comparison's close does, so reopening shows the same rows at once.
TEST_CASE("closing the path report keeps its rows, and reopening shows them without a build") {
    ScratchPaths paths("appstate_pathclose");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathclose");
    app->show_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    const hydra::app::report::GeneratedReport* rows = app->path_report.result.get();

    app->close_path_report();
    CHECK_FALSE(app->path_report.window_open);
    CHECK(app->path_report.result.get() == rows);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->parked_reports.empty());

    app->show_path_report();
    CHECK(app->path_report.window_open);
    CHECK(app->report_job == nullptr);  // nothing rebuilds
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->path_report.result.get() == rows);
}

// The user's answer to D103 item 28's follow-up (2026-10-09), which D271
// item 7 keeps: rows built because the window opened keep its filters;
// Refresh's start them over.
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

    // Closed and opened again: the Refresh's rows show, nothing rebuilds.
    app->close_path_report();
    app->show_path_report();
    CHECK(app->report_job == nullptr);
    CHECK_FALSE(app->path_report.built_on_open);
}

// D271 item 7: a build still running when the window closes is not
// cancelled. It lands with the window shut, and nothing is parked.
TEST_CASE("closing the path report mid-build lets the build finish and land") {
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
    CHECK_FALSE(app->path_report.window_open);
    CHECK(app->path_report_build() == ReportBuild::Building);
    REQUIRE(app->report_job != nullptr);
    CHECK_FALSE(app->report_job->is_cancelled());
    CHECK(app->parked_reports.empty());

    release = true;
    finish_path_report(*app);
    CHECK(app->parked_reports.empty());
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->path_report.result != nullptr);
    CHECK_FALSE(app->path_report.window_open);  // it lands with the window shut

    app->show_path_report();
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report_build() == ReportBuild::Ready);
}

// D278 item 3: a cancelled report stays cancelled after a close, as the
// comparison's does. Reopening shows "Report cancelled." with Try again.
TEST_CASE("reopening the path report after a cancelled build shows it cancelled") {
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
    CHECK(app->report_job == nullptr);  // no new build starts
    CHECK(app->path_report_build() == ReportBuild::Cancelled);
    CHECK(hydra::ui::path_report_input(*app).state == ReportBuild::Cancelled);

    // Try again builds it.
    hydra::ui::path_report_input(*app).callbacks.try_again();
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

    // The chart mode is one it reads: the page lists one profile (D115
    // item 5, the plan's question 2).
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

TEST_CASE("select_chart selects the first copy in the bar's mode") {
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

    app->select_chart("hash007");
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == library_entry(7).notespath);

    // The first copy in the library's order: sorted the other way, the copy.
    app->library.set_sort(hydra::ui::LibrarySort::Title, false);
    app->select_chart("hash007");
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == copy.notespath);

    // A hash the library doesn't list selects nothing new.
    app->select_chart("not-in-library");
    CHECK(app->selected->notespath == copy.notespath);
}

TEST_CASE("select_chart opens the first copy by name when the search hides every copy") {
    ScratchPaths paths("appstate_selecthidden");
    std::unique_ptr<AppState> app = app_on(paths);
    // Three copies of chart 7, scanned in this order: "Song B" first, then
    // two copies named "Song A". The later "Song A" has the path that sorts
    // first. The user's ruling (docs/agents/rulings/ui-and-display.md, "Keep
    // today's pick") keeps the first copy by name, and of two copies with one
    // name the one scanned first: the middle copy. The first in the table's
    // own order (what the unsorted read gives) is "Song B", and the first by
    // path is the last "Song A".
    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < kChartCount; ++i)
        if (i != 7) charts.push_back(library_entry(i));
    ChartLibraryEntry b = library_entry(7);
    b.title = "Song B";
    b.notespath = "C:\\m\\hash007\\notes.chart";
    ChartLibraryEntry a = library_entry(7);
    a.title = "Song A";
    a.notespath = "C:\\p\\hash007\\notes.chart";
    ChartLibraryEntry a_later = library_entry(7);
    a_later.title = "Song A";
    a_later.notespath = "C:\\a\\hash007\\notes.chart";
    charts.insert(charts.begin(), {b, a, a_later});
    app->store->rebuild_chart_library(charts);
    app->reload_library();

    app->set_search("zzqxj");  // in no row's text
    REQUIRE(app->library.order().empty());
    app->select_chart("hash007");
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == a.notespath);
}

// D310 item 3: a comparison row click opens the song in the bar's mode, even
// when the comparison was built in another.
TEST_CASE("a comparison row click selects the chart and keeps the bar's mode") {
    ScratchPaths paths("appstate_dmrowclick");
    std::unique_ptr<AppState> app = app_on(paths);
    const std::string bar = app->settings.chartmode_key();

    // The comparison keeps no mode of its own, so the click has none to switch
    // to; the bar's mode must stay as it was.
    auto result = std::make_shared<hydra::app::dm_report::GeneratedDmReport>();
    result->rows.emplace_back();
    result->rows.back().identifier = library_entry(7).md5;
    app->dm_report.result = result;

    hydra::ui::dm_report_input(*app).callbacks.row_click(0);
    CHECK(app->settings.chartmode_key() == bar);
    CHECK(Settings::load_file(paths.ini).chartmode_key() == bar);  // nothing committed
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == library_entry(7).notespath);
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

// ---- opening the report analyzes first (D249 item 8) --------

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
    app->batch_job.reset();  // the finished strip's X

    // Over it, the report window asks on the same plan and nothing runs yet.
    // The question is the report's own: the main window's popup stays shut.
    app->report_confirm_above = kUnanalyzed - 1;
    app->request_path_report();  // the window's Refresh
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
    app->batch_job.reset();

    // Its Cancel is "don't analyze": nothing runs, and the report builds from
    // the results already stored rather than ending cancelled.
    app->request_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    hydra::ui::path_report_input(*app).callbacks.cancel();
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK(app->report_job != nullptr);
    app->update_background_jobs();
    CHECK(app->batch_job == nullptr);
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);

    // Closing the window while it asks ends the question (D276 item 6):
    // nothing runs, no popup takes it up in the main window, and the rows in
    // memory stay to show on reopening.
    const hydra::app::report::GeneratedReport* rows = app->path_report.result.get();
    REQUIRE(rows != nullptr);
    app->request_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    app->close_path_report();
    app->update_background_jobs();
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->batch_confirm_in_main_window());
    CHECK(app->batch_job == nullptr);
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    app->show_path_report();
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK(app->path_report.result.get() == rows);

    // The toolbar's Analyze while the window asks: the user's own popup opens,
    // and the window's question counts as declined, so the report builds from
    // what is stored.
    app->request_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    app->open_batch_confirm();
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK(app->batch_confirm_in_main_window());
    CHECK(app->report_job != nullptr);
    CHECK(app->batch_job == nullptr);
    app->close_batch_confirm();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
}

// D276 item 6: closing the window while it asks "Analyze N charts?" means
// no. With nothing built, reopening asks again.
TEST_CASE("closing the path report while it asks ends the question, and reopening asks again") {
    ScratchPaths paths("appstate_pathcloseasks");
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathcloseasks");
    app->report_confirm_above = kUnanalyzed - 1;
    app->show_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    const std::string question = hydra::ui::path_report_input(*app).confirm_question;
    REQUIRE_FALSE(question.empty());

    app->close_path_report();
    app->update_background_jobs();
    CHECK_FALSE(app->path_report.window_open);
    CHECK_FALSE(app->path_report_awaits_confirm());
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->batch_confirm_in_main_window());
    CHECK(app->batch_job == nullptr);
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report_build() == ReportBuild::None);

    app->show_path_report();
    CHECK(app->path_report_awaits_confirm());
    CHECK(hydra::ui::path_report_input(*app).confirm_question == question);
    CHECK(app->batch_job == nullptr);
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
// in the window then refreshed, a close (which keeps the report, D271 item
// 7), and a new batch.
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
    CHECK(app->path_report_build() == ReportBuild::Ready);
    CHECK(app->report_started);

    app->batch_job.reset();  // the finished strip's X
    run_redo_batch_over(*app, title);
    CHECK_FALSE(app->report_started);  // the new run's report has not started
}

// ---- each report reads through a connection of its own --------------------

namespace {

using hydra::app::dm_report::GeneratedDmReport;
using hydra::app::report::GeneratedReport;

// The canned leaderboard of kDmUsersJson and kDmScoresJson, answered at once.
hydra::net::Fetcher canned_dm_fetcher() {
    return [](const std::string& url, const std::atomic<bool>*) -> std::string {
        return url.find("/all-users") != std::string::npos ? kDmUsersJson : kDmScoresJson;
    };
}

// Whether two path reports show the same page: every count, line and row the
// window reads.
void check_same_path_report(const GeneratedReport& got, const GeneratedReport& want) {
    CHECK(got.subtitle == want.subtitle);
    CHECK(got.footer == want.footer);
    CHECK(got.songs == want.songs);
    CHECK(got.rows == want.rows);
    CHECK(got.library_charts == want.library_charts);
    CHECK(got.cant_analyze_charts == want.cant_analyze_charts);
    CHECK(got.no_result_charts == want.no_result_charts);
    REQUIRE(got.paths.size() == want.paths.size());
    for (size_t i = 0; i < got.paths.size(); ++i) {
        CHECK(got.paths[i].song == want.paths[i].song);
        CHECK(got.paths[i].rank == want.paths[i].rank);
        CHECK(got.paths[i].path == want.paths[i].path);
        CHECK(got.paths[i].score == want.paths[i].score);
        CHECK(got.paths[i].copies == want.paths[i].copies);
    }
}

// The same for two comparisons.
void check_same_dm_report(const GeneratedDmReport& got, const GeneratedDmReport& want) {
    CHECK(got.username == want.username);
    CHECK(got.subtitle == want.subtitle);
    CHECK(got.footer == want.footer);
    CHECK(got.stats.total == want.stats.total);
    REQUIRE(got.rows.size() == want.rows.size());
    for (size_t i = 0; i < got.rows.size(); ++i) {
        CHECK(got.rows[i].identifier == want.rows[i].identifier);
        CHECK(got.rows[i].optimal == want.rows[i].optimal);
        CHECK(got.rows[i].status == want.rows[i].status);
    }
}

// Waits for the path report's job to finish without collecting it, so a test
// can look at what else is still going on first.
void wait_path_report_job(AppState& app) {
    REQUIRE(app.report_job != nullptr);
    wait_until([&] { return app.report_job->finished(); }, "the path report's job to finish");
}

// Arms the store so that, between the two reads inside the first lookup a
// thread other than this one makes (the report job's), the writer saves a
// result for the app's first library chart, and sets `saved`. The draw
// thread's own lookups pass by. The caller clears the seam afterwards and
// declares `saved` before the AppState, so the job never outlives it.
void save_during_report_lookup(AppState& app, std::atomic<bool>& saved) {
    const std::string md5 = app.library_row_at(0).entry.md5;
    const hydra::store::Profile profile = app.settings.profile();
    const std::thread::id draw = std::this_thread::get_id();
    hydra::store::RecordStore* store = app.store.get();
    store->set_between_lookup_reads_for_test([md5, profile, draw, store, &saved] {
        if (std::this_thread::get_id() == draw || saved.exchange(true)) return;
        hydra::test::save_batch_record(*store, md5, profile);
    });
}

// What a reader of `store` that cannot open reads as: the store's own
// failure, armed through its seam, worded by app::plain_error.
std::string failed_open_sentence(hydra::store::RecordStore& store) {
    store.fail_next_reader_open_for_test();
    try {
        const std::unique_ptr<hydra::store::OwnReader> reader = store.open_own_reader();
    } catch (const std::exception& e) {
        return hydra::app::plain_error(e);
    }
    FAIL("the armed open did not fail");
    return std::string();
}

// Saves one result under a batch's checkpoint guard and ends the guard. Its
// truncating checkpoint empties the log only when no other connection holds a
// read open (the store's "a reader holds nothing open between reads" test),
// so an empty log after it means no report kept its connection or its hold.
void check_log_empties(AppState& app, const ScratchPaths& paths) {
    const std::filesystem::path wal = std::filesystem::u8path(paths.db + "-wal");
    {
        const hydra::store::RecordStore::BatchWrites batch(*app.store);
        hydra::test::save_batch_record(*app.store, app.library_row_at(0).entry.md5,
                                       app.settings.profile());
        REQUIRE(std::filesystem::file_size(wal) > 0);
    }
    CHECK(std::filesystem::file_size(wal) == 0);
}

}  // namespace

TEST_CASE("a path report builds while a save group is open") {
    ScratchPaths paths("appstate_pathgroup");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathgroup");
    app->request_path_report();
    finish_path_report(*app);
    const std::shared_ptr<const GeneratedReport> unheld = app->path_report.result;
    REQUIRE(unheld != nullptr);

    // After the app, so the helper lets go before the app goes.
    hydra::test::HeldSaveGroup group(*app->store);
    app->request_path_report();
    wait_path_report_job(*app);
    CHECK(group.held());  // it finished with the group still open
    group.let_go();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    REQUIRE(app->path_report.result != nullptr);
    check_same_path_report(*app->path_report.result, *unheld);
}

TEST_CASE("a DM report builds while a save group is open") {
    ScratchPaths paths("appstate_dmgroup");
    // Before the app, which joins the fetch's thread.
    const ScopedHook fetcher(hydra::net::set_fetcher, canned_dm_fetcher());
    std::unique_ptr<AppState> app = app_on(paths);
    app->start_dm_report("111", "alice");
    finish_dm_report(*app);
    const std::shared_ptr<const GeneratedDmReport> unheld = app->dm_report.result;
    REQUIRE(unheld != nullptr);

    hydra::test::HeldSaveGroup group(*app->store);
    app->request_dm_report();
    REQUIRE(app->dm_report_job != nullptr);
    wait_until([&] { return app->dm_report_job->finished(); }, "the comparison's job to finish");
    CHECK(group.held());
    group.let_go();
    app->update_background_jobs();
    CHECK(app->dm_report_build() == ReportBuild::Ready);
    REQUIRE(app->dm_report.result != nullptr);
    check_same_dm_report(*app->dm_report.result, *unheld);
}

TEST_CASE("a path report reads one moment") {
    ScratchPaths paths("appstate_pathmoment");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathmoment");
    app->request_path_report();
    finish_path_report(*app);
    const std::shared_ptr<const GeneratedReport> before = app->path_report.result;
    REQUIRE(before != nullptr);
    REQUIRE_FALSE(before->paths.empty());

    // Between the report's two reads inside lookup, the writer replaces the
    // chart's record with one that kept no paths. Only the report job's own
    // reader saves: the draw thread's reads before the job starts pass by.
    const std::string md5 = app->library_row_at(0).entry.md5;
    const hydra::store::Profile profile = app->settings.profile();
    std::atomic<bool> saved{false};
    save_during_report_lookup(*app, saved);
    app->request_path_report();
    wait_path_report_job(*app);
    app->store->set_between_lookup_reads_for_test({});
    finish_path_report(*app);

    REQUIRE(saved.load());
    REQUIRE(app->store->lookup_one(md5, profile).record.paths.empty());  // the save landed
    CHECK(app->path_report_build() == ReportBuild::Ready);
    REQUIRE(app->path_report.result != nullptr);
    check_same_path_report(*app->path_report.result, *before);
}

TEST_CASE("a path report's separate store calls read one moment") {
    ScratchPaths paths("appstate_pathcalls");
    // Declared before the AppState, whose job's lookup sets it.
    std::atomic<bool> saved{false};
    // A library whose one chart has no stored result. A request would start
    // a batch for it first, so the test lowers the threshold: the window
    // asks, and its Cancel builds the report from what is stored, no batch.
    const ChartLibraryEntry song = corpus_chart("pathcalls");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    app->report_confirm_above = 0;
    save_during_report_lookup(*app, saved);
    app->request_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    REQUIRE(app->batch_job == nullptr);
    app->cancel_path_report();  // the window's Cancel: list what is stored
    // The report reads chart_index, then lookup, inside which the writer
    // saves a result. It finds no rows, so it then asks a separate question,
    // record_count, to say why.
    wait_path_report_job(*app);
    app->store->set_between_lookup_reads_for_test({});
    finish_path_report(*app);

    REQUIRE(saved.load());
    REQUIRE(app->store->record_count() > 0);  // the save landed
    // Under the one hold every call reads the store as it was at chart_index:
    // empty, so record_count says 0 and the sentence is the app's "nothing is
    // analyzed yet". Without the hold, record_count reads on its own after the
    // save, says 1, and the report says "nothing under these settings".
    CHECK(app->path_report_build() == ReportBuild::Failed);
    CHECK(app->path_report.message ==
          hydra::app::plain_error(
              hydra::KindedError(hydra::ErrorKind::NoRecords, "no records stored yet")));
}

TEST_CASE("a report whose connection cannot open fails with the store's sentence") {
    ScratchPaths paths("appstate_reportopenfail");
    const ScopedHook fetcher(hydra::net::set_fetcher, canned_dm_fetcher());
    std::unique_ptr<AppState> app = app_with_result(paths, "reportopenfail");
    // The window's own reader opens now, so the armed open below is a report's.
    app->store->reader().chart_library_count();
    const std::string sentence = failed_open_sentence(*app->store);
    REQUIRE_FALSE(sentence.empty());

    app->store->fail_next_reader_open_for_test();
    app->request_path_report();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Failed);
    CHECK(app->path_report.message == sentence);
    hydra::ui::path_report_input(*app).callbacks.try_again();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);

    app->store->fail_next_reader_open_for_test();
    app->start_dm_report("111", "alice");
    finish_dm_report(*app);
    CHECK(app->dm_report_build() == ReportBuild::Failed);
    CHECK(app->dm_report.message == sentence);
    hydra::ui::dm_report_input(*app).callbacks.try_again();
    finish_dm_report(*app);
    CHECK(app->dm_report_build() == ReportBuild::Ready);
}

TEST_CASE("a finished report holds no connection") {
    ScratchPaths paths("appstate_reportclosed");
    const ScopedHook fetcher(hydra::net::set_fetcher, canned_dm_fetcher());
    std::unique_ptr<AppState> app = app_with_result(paths, "reportclosed");
    app->request_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report_build() == ReportBuild::Ready);
    app->start_dm_report("111", "alice");
    finish_dm_report(*app);
    REQUIRE(app->dm_report_build() == ReportBuild::Ready);
    check_log_empties(*app, paths);
}

TEST_CASE("a cancelled report closes its connection") {
    ScratchPaths paths("appstate_reportcancelclosed");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_result(paths, "reportcancelclosed");
    {
        const ScopedHook hold(hydra::ui::set_report_hold_for_test, held_report(release));
        app->request_path_report();
    }
    app->cancel_path_report();  // inside the hold seam, before the job reads
    release = true;
    finish_path_report(*app);
    REQUIRE(app->path_report_build() == ReportBuild::Cancelled);
    check_log_empties(*app, paths);
}

