// Tests for ui/app_state's two reports in memory (D103): the path report and
// the leaderboards comparison, how they build, cancel, close and go out of
// date, and the report that follows a batch. The shared fixtures are in
// app_state_fixtures.h.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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
using testwait::wait_until_or_gone;

// ---- the two reports in memory (D103) ---------------------------------------

namespace {

using hydra::ui::ReportBuild;
using hydra::ui::ReportOutOfDate;

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

TEST_CASE("select_chart selects the first copy in the library's order") {
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
    // Three copies of one corpus chart, each in a folder of its own and
    // scanned in this order: "Song B" first, then two copies named "Song A".
    // The later "Song A" has the path that sorts first. The user's ruling
    // (docs/agents/rulings/ui-and-display.md, "Keep today's pick") keeps the
    // first copy by name, and of two copies with one name the one scanned
    // first: the middle copy. The first in the table's own order (what the
    // unsorted read gives) is "Song B", and the first by path is the last
    // "Song A".
    const ChartLibraryEntry b = entry_for(write_corpus_chart("selecthidden_m", false), "Song B");
    const ChartLibraryEntry a = entry_for(write_corpus_chart("selecthidden_p", false), "Song A");
    const ChartLibraryEntry a_later =
        entry_for(write_corpus_chart("selecthidden_a", false), "Song A");
    REQUIRE(a.md5 == b.md5);  // one chart, three copies
    REQUIRE(a_later.md5 == b.md5);
    REQUIRE(a_later.notespath < a.notespath);
    std::unique_ptr<AppState> app = app_with(paths, {b, a, a_later});
    REQUIRE(app->library_shown_count() == size_t{3});

    app->set_search("zzqxj");  // in no row's text
    REQUIRE(app->library.order().empty());
    app->select_chart(b.md5);
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == a.notespath);
}

// D310 item 3: a comparison row click opens the song in the bar's mode, even
// when the comparison was built in another.
TEST_CASE("a comparison row click selects the chart and keeps the bar's mode") {
    ScratchPaths paths("appstate_dmrowclick");
    // Two corpus charts, the second with one more note so it hashes apart.
    const ChartLibraryEntry shown = corpus_chart("dmrowclick_a");
    const ChartLibraryEntry clicked = corpus_chart("dmrowclick_b", true);
    REQUIRE_FALSE(shown.md5 == clicked.md5);
    std::unique_ptr<AppState> app = app_with(paths, {shown, clicked});
    app->selected = shown;
    const std::string bar = app->settings.chartmode_key();

    // The comparison keeps no mode of its own, so the click has none to switch
    // to; the bar's mode must stay as it was. A comparison row's identifier is
    // the library chart's md5 (collect_dm_rows joins on it). No saved real
    // leaderboard reply is checked in, so the one row is set here.
    auto result = std::make_shared<hydra::app::dm_report::GeneratedDmReport>();
    result->rows.emplace_back();
    result->rows.back().identifier = clicked.md5;
    app->dm_report.result = result;

    hydra::ui::dm_report_input(*app).callbacks.row_click(0);
    CHECK(app->settings.chartmode_key() == bar);
    CHECK(Settings::load_file(paths.ini).chartmode_key() == bar);  // nothing committed
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->notespath == clicked.notespath);
}

// ---- opening the report analyzes first (D249 item 8) --------

namespace {

// A batch analyzer that waits for `release`, then runs the real analyzer on
// the chart, so the test reads the batch's snapshot while it is held.
hydra::app::ChartAnalyzer held_analyzer(std::atomic<bool>& release) {
    return [&release](const std::string& path, const hydra::app::AnalysisSettings& settings,
                      const std::function<void(float)>& on_progress)
               -> hydra::app::AnalysisResult {
        wait_until([&release] { return release.load(); }, "the test to release the batch");
        return hydra::app::analyze_chart_file(path, settings, on_progress);
    };
}

// The charts beside the analyzed one that have no result: the rows an
// opening's batch runs.
constexpr int kUnanalyzed = 5;

// An app whose one corpus chart has a stored result, so its report has rows,
// and kUnanalyzed more corpus charts, as a scan lists them, with no result.
// Each has its own md5, so each is a row the batch runs.
std::unique_ptr<AppState> app_with_unanalyzed(const ScratchPaths& paths, const char* tag) {
    const ChartLibraryEntry song = corpus_chart(tag);
    std::vector<ChartLibraryEntry> charts{song};
    for (const std::string& notespath : corpus::chart_paths()) {
        if (charts.size() > static_cast<size_t>(kUnanalyzed)) break;
        const ChartLibraryEntry e =
            corpus::scanned_entry(notespath, "Corpus " + std::to_string(charts.size()));
        const bool listed = std::any_of(charts.begin(), charts.end(),
                                        [&](const ChartLibraryEntry& c) { return c.md5 == e.md5; });
        if (!listed) charts.push_back(e);
    }
    REQUIRE(charts.size() == static_cast<size_t>(kUnanalyzed) + 1);
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
                              held_analyzer(release), 1);
        app->show_path_report();
    }
    REQUIRE(app->batch_job != nullptr);
    CHECK(app->report_job == nullptr);
    CHECK(app->path_report.window_open);
    CHECK(app->path_report_build() == ReportBuild::Building);
    CHECK(app->path_report_batch() == app->batch_job.get());
    CHECK_FALSE(app->report_started);

    // The window's numbers are the batch snapshot's, the strip's numbers. A
    // batch that has finished counts no more charts, so the wait fails then.
    wait_until_or_gone([&] { return app->batch_job->snapshot().total == kUnanalyzed; },
                       [&] { return app->batch_job->snapshot().finished; }, "the batch's count",
                       "the batch finished before its total reached the expected count, and a "
                       "finished batch counts no more charts");
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
    // A chart the analyzer could not read counts as completed too, so the
    // batch must show every one analyzed and none failed.
    CHECK(done.failed == 0);
    CHECK(done.analyzed == kUnanalyzed);

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
                              held_analyzer(release), 1);
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
TEST_CASE("at the threshold the path report runs its batch without asking") {
    ScratchPaths paths("appstate_pathopenatthreshold");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopenatthreshold");
    app->report_confirm_above = kUnanalyzed;
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_analyzer(release), 1);
        app->show_path_report();
    }
    REQUIRE(app->batch_job != nullptr);
    CHECK_FALSE(app->batch_confirm_pending);
    CHECK_FALSE(app->path_report_awaits_confirm());
    release = true;
    wait_batch_finished(*app);
    CHECK(app->batch_job->snapshot().failed == 0);
    app->update_background_jobs();
    finish_path_report(*app);
    CHECK(app->path_report_build() == ReportBuild::Ready);
}

// The phases that run nothing come first: the batch at the end analyzes
// every row, and after it nothing is left to ask about.
TEST_CASE("more rows to run than the threshold open the batch confirm first") {
    ScratchPaths paths("appstate_pathopenconfirm");
    std::atomic<bool> release{false};
    std::unique_ptr<AppState> app = app_with_unanalyzed(paths, "pathopenconfirm");

    // Over the threshold, the report window asks on the batch's plan and
    // nothing runs yet. The question is the report's own: the main window's
    // popup stays shut.
    app->report_confirm_above = kUnanalyzed - 1;
    app->show_path_report();
    CHECK(app->batch_job == nullptr);
    CHECK(app->batch_plan.todo_rows() == kUnanalyzed);
    CHECK(app->path_report_awaits_confirm());
    CHECK_FALSE(app->batch_confirm_in_main_window());
    CHECK(app->path_report_build() == ReportBuild::Building);
    {
        const hydra::ui::PathReportInput asking = hydra::ui::path_report_input(*app);
        CHECK(asking.confirm_question == app->batch_question(false));
        // It names how many charts would run.
        CHECK(asking.confirm_question.find(std::to_string(kUnanalyzed)) != std::string::npos);
        CHECK(asking.can_analyze);
    }

    // Its Cancel is "don't analyze": nothing runs, and the report builds from
    // the results already stored rather than ending cancelled.
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

    // Its Analyze runs the batch, which the report then waits on.
    app->request_path_report();
    REQUIRE(app->path_report_awaits_confirm());
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              held_analyzer(release), 1);
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
    start_redo_batch(*app, held_analyzer(release));
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
    CHECK(app_with(paths, {corpus_chart("pathoffered")})->path_report_offered());
    ScratchPaths empty("appstate_pathofferednone");
    CHECK_FALSE(app_with(empty, {})->path_report_offered());
}

// ---- the report that follows a batch ----------------------------------------

// The report that follows a batch lists what the batch analyzed, even when
// the settings bar moved on after it (D51 call 26).
TEST_CASE("the post-batch report lists the batch's cap and lens, not the live settings") {
    ScratchPaths paths("appstate_reportrun");
    std::unique_ptr<AppState> app = app_with_result(paths, "reportrun");
    run_redo_batch_over(*app, app->library_row_at(0).title);
    const hydra::store::Profile batch_profile = app->batch_job->batch_run().profile;

    app->settings.sp_cap += 1;  // edited, not committed
    app->settings.mslimit_enabled = !app->settings.mslimit_enabled;
    // The bar now names another cap and lens than the batch ran with.
    REQUIRE_FALSE(app->settings.profile().cap == batch_profile.cap);
    REQUIRE_FALSE(app->settings.lens() == batch_profile.lens);
    app->update_background_jobs();

    REQUIRE(app->report_job != nullptr);
    CHECK(app->report_job->profile().cap == batch_profile.cap);
    CHECK(app->report_job->profile().lens == batch_profile.lens);
    wait_until([&] { return app->report_job->finished(); }, "the batch's report job to finish");
}

// The finished strip (render_batch_done) shows its report line and "Open
// automatically" from report_started and the build state. The strip draws
// inside ImGui and has no input a test can read, so this test checks the
// flag's lifetime on the paths that matter: a batch's report cancelled in
// the window then refreshed, a close (which keeps the report, D271 item 7),
// and a new batch.
TEST_CASE("report_started stays set through a cancel, a refresh and a close, and a new batch clears it") {
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

using hydra::app::report::GeneratedReport;

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

// Waits for the path report's job to finish without collecting it, so a test
// can look at what else is still going on first.
void wait_path_report_job(AppState& app) {
    REQUIRE(app.report_job != nullptr);
    wait_until([&] { return app.report_job->finished(); }, "the path report's job to finish");
}

// Arms the store so that, between the two reads inside the first lookup a
// thread other than this one makes (the report job's), the writer saves
// `record` as the result for the app's first library chart, and sets `saved`.
// The draw thread's own lookups pass by. The caller clears the seam afterwards
// and declares `saved` before the AppState, so the job never outlives it.
void save_during_report_lookup(AppState& app, std::atomic<bool>& saved,
                               const hydra::HydraRecord& record) {
    const hydra::store::PreparedRecord prepared = hydra::store::prepare_record(
        app.library_row_at(0).entry.md5, app.settings.profile(), record);
    const std::thread::id draw = std::this_thread::get_id();
    hydra::store::RecordStore* store = app.store.get();
    store->set_between_lookup_reads_for_test([prepared, draw, store, &saved] {
        if (std::this_thread::get_id() == draw || saved.exchange(true)) return;
        store->save(prepared);
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

TEST_CASE("a path report reads one moment") {
    ScratchPaths paths("appstate_pathmoment");
    std::unique_ptr<AppState> app = app_with_result(paths, "pathmoment");
    app->request_path_report();
    finish_path_report(*app);
    const std::shared_ptr<const GeneratedReport> before = app->path_report.result;
    REQUIRE(before != nullptr);
    REQUIRE_FALSE(before->paths.empty());

    // Between the report's two reads inside lookup, the writer replaces the
    // chart's record with the engine's analysis of the chart with one more
    // note, as a batch that read the chart after an edit would file it. Only
    // the report job's own reader saves: the draw thread's reads before the
    // job starts pass by.
    const std::string md5 = app->library_row_at(0).entry.md5;
    const hydra::store::Profile profile = app->settings.profile();
    const std::optional<int64_t> score_before =
        app->store->lookup_one(md5, profile).record.summary().score;
    const hydra::app::AnalysisResult edited = hydra::app::analyze_chart_file(
        write_corpus_chart("pathmoment_edited", true), app->settings.to_analysis_settings());
    REQUIRE_FALSE(edited.record.paths.empty());
    std::atomic<bool> saved{false};
    save_during_report_lookup(*app, saved, edited.record);
    app->request_path_report();
    wait_path_report_job(*app);
    app->store->set_between_lookup_reads_for_test({});
    finish_path_report(*app);

    REQUIRE(saved.load());
    // The save landed, and it changed the score a report reads.
    REQUIRE(app->store->lookup_one(md5, profile).record.summary().score != score_before);
    CHECK(app->path_report_build() == ReportBuild::Ready);
    REQUIRE(app->path_report.result != nullptr);
    check_same_path_report(*app->path_report.result, *before);

    // A report built after the save reads the edited chart's result.
    app->request_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report.result != nullptr);
    REQUIRE_FALSE(app->path_report.result->paths.empty());
    CHECK(app->path_report.result->paths.front().score != before->paths.front().score);
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
    // The mid-read save is the engine's analysis of that chart, as a batch
    // running beside the report would file it.
    const hydra::app::AnalysisResult analyzed =
        hydra::app::analyze_chart_file(song.notespath, app->settings.to_analysis_settings());
    save_during_report_lookup(*app, saved, analyzed.record);
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

// The comparison's half needs a leaderboard reply, and no saved real one is
// checked in, so only the path report is checked here.
TEST_CASE("a path report whose connection cannot open fails with the store's sentence") {
    ScratchPaths paths("appstate_reportopenfail");
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
}

// The comparison's half needs a leaderboard reply, and no saved real one is
// checked in, so only the path report is checked here.
TEST_CASE("a finished path report holds no connection") {
    ScratchPaths paths("appstate_reportclosed");
    std::unique_ptr<AppState> app = app_with_result(paths, "reportclosed");
    app->request_path_report();
    finish_path_report(*app);
    REQUIRE(app->path_report_build() == ReportBuild::Ready);
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

