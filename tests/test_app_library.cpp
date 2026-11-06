// Tests for ui/app_state's library: the search, the rows a batch refreshes,
// scans, the batch confirm, the startup open (DBUP) and reads that fail.
// Split from test_app_click.cpp; the shared fixtures are in
// app_state_fixtures.h.

#include "doctest.h"

#include <atomic>
#include <exception>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/user_messages.h"  // kDatabaseReadSentence, old_library_set_aside_notice, library_scan_blocked_notice
#include "app_state_fixtures.h"
#include "core/error_kind.h"
#include "db_file_util.h"  // exec_on_file, write_junk_db
#include "display_fixtures.h"  // save_batch_record
#include "library_fixtures.h"  // library_entry
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "scratch_settings.h"
#include "store/record_store.h"
#include "store/upgrade_files.h"  // set_aside_path, with_side_files
#include "ui/app_state.h"
#include "ui/generation.h"
#include "wait_util.h"

using hydra::app::kDatabaseReadSentence;
using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStatus;
using hydra::store::RecordStore;
using hydra::ui::AppState;
using hydra::ui::GenerationWatcher;
using hydra::ui::ViewedSong;
using testwait::wait_until;

// ---- the search --------------------------------------------------------------

// The search runs in memory over every chart; a word matches inside a title.
TEST_CASE("set_search narrows the library and the match count") {
    ScratchPaths paths("appstate_search");
    std::unique_ptr<AppState> app = app_on(paths);
    app->set_search("hash017");
    CHECK(app->library.searching());
    CHECK(app->library_shown_count() == 1);
    CHECK(app->library.counts().all == 1);
    CHECK(app->library_row_at(0).entry.md5 == "hash017");
    CHECK(app->library_matches().size() == 1);
    app->set_search("");
    CHECK_FALSE(app->library.searching());
    CHECK(app->library_shown_count() == static_cast<size_t>(kChartCount));
}

// "stars:9" is not a filter Hydra understands, so the search narrows nothing:
// the toolbar's batch button keeps reading "Analyze library..." (D48, Q15).
TEST_CASE("set_search: a filter that does not parse leaves the query empty") {
    ScratchPaths paths("appstate_badfilter");
    std::unique_ptr<AppState> app = app_on(paths);
    app->set_search("stars:9");
    CHECK(app->library.query().empty());
    CHECK_FALSE(app->library.searching());
    CHECK(app->library_shown_count() == static_cast<size_t>(kChartCount));
}

// ---- the rows a batch refreshes ---------------------------------------------

// Ruling 14: after a batch the library rows refresh, and the open panel is
// not analyzed again: it keeps what its own click showed (D48 Q16 restated).
TEST_CASE("a batch result for the open chart refreshes its row, not the panel") {
    ScratchPaths paths("appstate_batchpanel");
    std::unique_ptr<AppState> app = app_on(paths);
    const ChartLibraryEntry open = library_entry(5);
    app->select(open, hydra::ui::SelectionSource::Click);
    REQUIRE(app->viewed.state == ViewedSong::State::FileMissing);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The result arrives the way a batch files one (H1's fixture).
    hydra::test::save_batch_record(*app->store, open.md5, Settings{}.sp_cap);
    run_redo_batch_over(*app, open.md5);

    app->tick_library(0.0);  // the batch's last refresh
    app->tick(0.0);
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
    CHECK(app->view_job == nullptr);
    CHECK(app->viewed.state == ViewedSong::State::FileMissing);
    CHECK_FALSE(records.changed(app->record_generation));
}

// A batch that stores a different result for the open Ready chart changes
// its library row; the panel keeps the engine's own result from the click.
TEST_CASE("a batch result for an open Ready chart changes its row and keeps the panel") {
    ScratchPaths paths("appstate_batchredo");
    const ChartLibraryEntry song = corpus_chart("click_batchredo");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    REQUIRE(app->viewed.ready());
    const size_t shown_paths = app->viewed.record->paths.size();
    REQUIRE(shown_paths > 0);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // An empty result (no paths, so no score) under the same profile.
    hydra::test::save_batch_record(*app->store, song.md5, app->settings.profile());
    run_redo_batch_over(*app, song.title);

    app->tick_library(0.0);  // the batch's last refresh
    app->tick(0.0);
    CHECK(row_of(*app, song.md5).status == RecordStatus::Ready);
    CHECK_FALSE(row_of(*app, song.md5).summary.has_scored_best_path());
    CHECK(app->view_job == nullptr);
    REQUIRE(app->viewed.ready());
    CHECK(app->viewed.record->paths.size() == shown_paths);
    CHECK_FALSE(records.changed(app->record_generation));
}

// A result stored behind the view's back shows once its row is re-read, and
// only that row is asked about.
TEST_CASE("refresh_library_row picks up one chart's new result") {
    ScratchPaths paths("appstate_row");
    std::unique_ptr<AppState> app = app_on(paths);
    const ChartLibraryEntry fifth = library_entry(5);
    size_t at = app->library_shown_count();
    for (size_t i = 0; i < app->library_shown_count(); ++i)
        if (app->library_row_at(i).entry.md5 == fifth.md5) at = i;
    REQUIRE(at < app->library_shown_count());
    CHECK(app->library_row_at(at).status == RecordStatus::NotAnalyzed);

    hydra::test::save_batch_record(*app->store, fifth.md5, kSeededCap);
    app->refresh_library_row(fifth.md5);
    CHECK(app->library_row_at(at).status == RecordStatus::Ready);
    CHECK(app->library.counts().analyzed == 2);
}

// ---- the row order while results arrive -------------------------------------

namespace {

// How many charts the order tests' library holds.
constexpr int kHeldCharts = 4;

// An analysis whose best path scores `score` under the default settings.
hydra::HydraRecord scored_record(int64_t score) {
    hydra::HydraRecord record = hydra::test::empty_analysis_under(Settings{}.profile());
    hydra::Path best;
    best.notes_points = score;
    record.paths.push_back(best);
    return record;
}

// A result stored behind the view's back, as one song's analysis stores it.
void save_scored(AppState& app, int chart, int64_t score) {
    app.store->save(hydra::store::prepare_record(library_entry(chart).md5, app.settings.profile(),
                                                 scored_record(score)));
}

// A library of kHeldCharts charts with no results, under Best path, highest
// first. With every row unscored they read in title order, library_entry's.
std::unique_ptr<AppState> app_under_best_path(const ScratchPaths& paths) {
    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < kHeldCharts; ++i) charts.push_back(library_entry(i));
    std::unique_ptr<AppState> app = app_with(paths, charts);
    app->library.set_sort(hydra::ui::LibrarySort::BestPath, false);
    return app;
}

// The rows on screen, top to bottom, as library_entry numbers.
std::vector<int> shown(const AppState& app) {
    std::vector<int> out;
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        for (int chart = 0; chart < kHeldCharts; ++chart)
            if (app.library_row_at(i).entry.md5 == library_entry(chart).md5) out.push_back(chart);
    return out;
}

const std::vector<int> kTitleOrder{0, 1, 2, 3};

// Lets a batch's charts through one at a time, so a test knows exactly what the
// batch has stored at each tick with no sleep and no race. A chart waits here
// for its turn; its progress callback throws once Stop is pressed, so a chart
// still waiting is neither stored nor counted. The n-th chart through scores
// n * 1000, so each chart outscores the ones before it.
struct ChartGate {
    std::atomic<int> started{0};
    std::atomic<int> allowed{0};
};

hydra::app::ChartAnalyzer gated_scores(std::shared_ptr<ChartGate> gate) {
    return [gate](const std::string&, const hydra::app::AnalysisSettings&,
                  const std::function<void(float)>& on_progress) {
        const int turn = ++gate->started;
        wait_until(
            [&] {
                on_progress(0.0f);
                return gate->allowed.load() >= turn;
            },
            "the test to let a chart through");
        return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)};
    };
}

// Lets `charts` charts in all through the gate and waits until the batch counts
// them.
void let_through(AppState& app, ChartGate& gate, int charts) {
    gate.allowed.store(charts);
    wait_until([&] { return app.batch_job->snapshot().completed == charts; },
               "the batch to store its charts");
}

// A library under Best path whose batch has stored two charts and whose rows
// have read both: the first chart through still sits above the second, which
// outscores it.
struct BatchAfterTwoCharts {
    std::unique_ptr<AppState> app;
    std::shared_ptr<ChartGate> gate = std::make_shared<ChartGate>();

    explicit BatchAfterTwoCharts(const ScratchPaths& paths) : app(app_under_best_path(paths)) {
        start_redo_batch(*app, gated_scores(gate));
        let_through(*app, *gate, 2);
        app->tick_library(0.0);
        REQUIRE(shown(*app) == kTitleOrder);
    }
};

}  // namespace

TEST_CASE("a batch refresh fills scores in where the rows sit; the batch's end sorts them") {
    ScratchPaths paths("appstate_batchhold");
    BatchAfterTwoCharts batch(paths);
    AppState& app = *batch.app;
    CHECK(row_of(app, library_entry(1).md5).best_label().find("2,000") == 0);
    CHECK(app.library.counts().analyzed == 2);

    let_through(app, *batch.gate, 3);
    app.tick_library(10.0);
    CHECK(shown(app) == kTitleOrder);
    CHECK(app.library.counts().analyzed == 3);

    batch.gate->allowed.store(kHeldCharts);
    wait_batch_finished(app);
    app.tick_library(20.0);
    CHECK(shown(app) == std::vector<int>{3, 2, 1, 0});
}

// Stop leaves the count where the last refresh read it, so the refresh's own
// check finds nothing new: the same ending as a batch whose last result was
// read just before it finished, held still for the test.
TEST_CASE("a batch whose last result was already read still sorts when it ends") {
    ScratchPaths paths("appstate_batchreadend");
    BatchAfterTwoCharts batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    REQUIRE(app.batch_job->snapshot().completed == 2);
    app.tick_library(10.0);
    CHECK(shown(app) == std::vector<int>{1, 0, 2, 3});
}

TEST_CASE("a batch's end sorts once, and the next batch sorts when it ends") {
    ScratchPaths paths("appstate_batchonce");
    BatchAfterTwoCharts batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    app.tick_library(10.0);
    REQUIRE(shown(app) == std::vector<int>{1, 0, 2, 3});

    // A song's own result after the end holds its place, however many frames
    // the finished batch stays on its strip.
    save_scored(app, 3, 5000);
    REQUIRE(app.refresh_library_row(library_entry(3).md5));
    app.tick_library(20.0);
    app.tick_library(30.0);
    CHECK(shown(app) == std::vector<int>{1, 0, 2, 3});

    start_redo_batch(app, gated_scores(std::make_shared<ChartGate>()));
    stop_batch(app);
    app.tick_library(40.0);
    CHECK(shown(app) == std::vector<int>{3, 1, 0, 2});
}

// The finished strip's X can let the job go before the frame's tick reads it.
TEST_CASE("a batch dismissed before a tick saw it finish still sorts") {
    ScratchPaths paths("appstate_batchdismiss");
    BatchAfterTwoCharts batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    app.batch_job.reset();
    app.tick_library(10.0);
    CHECK(shown(app) == std::vector<int>{1, 0, 2, 3});
}

// A pool of no workers is a run that fails as a whole (BatchJob::finish_failed).
TEST_CASE("a batch that fails as a whole sorts a held order when it ends") {
    ScratchPaths paths("appstate_batcherror");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    save_scored(*app, 3, 5000);
    REQUIRE(app->refresh_library_row(library_entry(3).md5));
    REQUIRE(shown(*app) == kTitleOrder);
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              gated_scores(std::make_shared<ChartGate>()), /*workers=*/0);
        app->start_batch(true);
    }
    REQUIRE(app->batch_job != nullptr);
    wait_batch_finished(*app);
    REQUIRE_FALSE(app->batch_job->snapshot().run_error.empty());
    app->tick_library(0.0);
    CHECK(shown(*app) == std::vector<int>{3, 0, 1, 2});
}

// The user's wording: for a single opened song the row stays put until the
// next re-sort.
TEST_CASE("one song's stored result keeps its row in place, with no batch running") {
    ScratchPaths paths("appstate_rowhold");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    save_scored(*app, 3, 5000);
    REQUIRE(app->refresh_library_row(library_entry(3).md5));
    CHECK(shown(*app) == kTitleOrder);
    CHECK(row_of(*app, library_entry(3).md5).best_label().find("5,000") == 0);
    app->tick_library(0.0);  // no batch, so no end to sort at
    CHECK(shown(*app) == kTitleOrder);
}

TEST_CASE("a settings change's re-read of the summaries sorts a held order at once") {
    ScratchPaths paths("appstate_settingssort");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    save_scored(*app, 3, 5000);
    REQUIRE(app->refresh_library_row(library_entry(3).md5));
    app->refresh_library_summaries();
    CHECK(shown(*app) == std::vector<int>{3, 0, 1, 2});
}

// A reload replaces every row and then reads their summaries, and the list
// comes up sorted on them.
TEST_CASE("a reload comes up sorted under Best path") {
    ScratchPaths paths("appstate_reloadsort");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    save_scored(*app, 3, 5000);
    app->reload_library();
    CHECK(shown(*app) == std::vector<int>{3, 0, 1, 2});
}

// ---- scans and the batch ----------------------------------------------------

namespace {

// Starts a Redo batch over the whole library that keeps running until the
// test presses Stop: each chart's analysis waits on the batch's own cancel
// check (its progress callback throws once Stop is pressed).
void start_batch_until_stopped(AppState& app) {
    start_redo_batch(app, [](const std::string&, const hydra::app::AnalysisSettings&,
                             const std::function<void(float)>& on_progress)
                              -> hydra::app::AnalysisResult {
        testwait::tick_until_thrown([&] { on_progress(0.0f); }, "the test to press Stop");
    });
    REQUIRE(app.batch_running());
}

}  // namespace

// The same chart can sit in two folders; only the copy that was clicked is
// the selected row.
TEST_CASE("is_selected_row: the selected row is the one with its notespath, not its md5") {
    ScratchPaths paths("appstate_selrow");
    std::unique_ptr<AppState> app = app_on(paths);  // library_entry(0) is selected
    ChartLibraryEntry twin = library_entry(0);
    twin.notespath = "C:\\other\\hash000\\notes.chart";
    twin.rootfolder = "C:\\other";
    CHECK(app->is_selected_row(library_entry(0)));
    CHECK_FALSE(app->is_selected_row(twin));
    app->selected.reset();
    CHECK_FALSE(app->is_selected_row(library_entry(0)));
}

// Every way to start a scan is refused while a batch runs, and the status
// line says why, as news that fades (D51 call 24, D62 item 4).
TEST_CASE("a scan cannot start during a batch, and the status line says so") {
    ScratchPaths paths("appstate_scanbatch");
    std::unique_ptr<AppState> app = app_on(paths);
    app->settings.chartfolders = {"C:\\charts"};
    start_batch_until_stopped(*app);

    CHECK_FALSE(app->can_scan());
    app->start_scan();
    CHECK(app->scan_job == nullptr);
    CHECK(app->status_message == "A batch is running.");
    CHECK_FALSE(app->status_is_problem);

    stop_batch(*app);
    CHECK(app->can_scan());
}

namespace {

// The hash the rows hold for a chart edited before the scan.
const std::string kHashBeforeEdit = "hash-before-the-edit";

// A library read before a scan that has since replaced the chart table, with
// no frame run in between, so the rows are older than the store. `kept` is
// what the scan wrote for a chart still in the folder, edited before the scan
// (its row holds kHashBeforeEdit); `removed` is a row the scan dropped.
struct RowsOlderThanScan {
    ChartLibraryEntry kept;
    ChartLibraryEntry removed = library_entry(1);
    std::unique_ptr<AppState> app;
};

RowsOlderThanScan rows_older_than_scan(const ScratchPaths& paths, const std::string& tag) {
    RowsOlderThanScan s;
    s.kept = corpus_chart(tag);
    ChartLibraryEntry before_edit = s.kept;
    before_edit.md5 = kHashBeforeEdit;
    before_edit.sig = "sig-before-the-edit";  // so the scan reads the file again
    s.app = app_with(paths, {before_edit, s.removed});
    s.app->settings.chartfolders = {s.kept.rootfolder};
    s.app->start_scan();
    REQUIRE(s.app->scan_job != nullptr);
    wait_until([&] { return s.app->scan_job->snapshot().finished; }, "the scan to finish");
    const auto progress = s.app->scan_job->snapshot();
    REQUIRE(progress.charts_found == 1);
    REQUIRE(s.app->library_shown_count() == 2);  // no frame has read the new table
    return s;
}

}  // namespace

// A click in the frame a scan finishes acts like a click one frame later: it
// opens the chart the scan kept, under the hash the scan wrote.
TEST_CASE("a click right after a scan, before any frame, opens the chart the scan kept") {
    ScratchPaths paths("appstate_scanclick");
    RowsOlderThanScan s = rows_older_than_scan(paths, "scan_click");
    s.app->select(row_of(*s.app, kHashBeforeEdit).entry, hydra::ui::SelectionSource::Click);
    REQUIRE(s.app->selected.has_value());
    CHECK(s.app->details_open());
    CHECK(s.app->selected->notespath == s.kept.notespath);
    CHECK(s.app->selected->md5 == s.kept.md5);
    settle(*s.app);
}

// The one silent no-op left: the scan really removed the clicked row.
TEST_CASE("a click right after a scan on a row the scan removed does nothing") {
    ScratchPaths paths("appstate_scanclickgone");
    RowsOlderThanScan s = rows_older_than_scan(paths, "scan_click_gone");
    s.app->select(row_of(*s.app, s.removed.md5).entry, hydra::ui::SelectionSource::Click);
    CHECK_FALSE(s.app->selected.has_value());
    CHECK_FALSE(s.app->details_open());
    CHECK(s.app->status_message.empty());
}

// The confirm's count and the button's N count one thing, so they agree even
// in the frame a scan finishes.
TEST_CASE("the batch confirm right after a scan, before any frame, counts the library's N") {
    ScratchPaths paths("appstate_scanconfirm");
    RowsOlderThanScan s = rows_older_than_scan(paths, "scan_confirm");
    s.app->open_batch_confirm();
    REQUIRE(s.app->batch_confirm_pending);
    CHECK(s.app->batch_scope_charts() == static_cast<int64_t>(s.app->library_shown_count()));
    CHECK(s.app->batch_scope_charts() == 1);
}

// wait-idle in the GUI tests waits on this one list (finding 109).
TEST_CASE("any_job_running lists every background job") {
    ScratchPaths paths("appstate_anyjob");
    std::unique_ptr<AppState> app = app_on(paths);
    CHECK_FALSE(app->any_job_running());

    start_batch_until_stopped(*app);
    CHECK(app->any_job_running());
    stop_batch(*app);
    CHECK_FALSE(app->any_job_running());  // a finished batch is not running

    // The click's job counts while it runs, cancelled or not, until tick()
    // collects it. The latch holds it inside its analysis, so it is still
    // running when the check reads it.
    const ChartLibraryEntry song = corpus_chart("click_anyjob");
    app->store->rebuild_chart_library({song});
    ViewLatch latch;  // after the app: see ViewLatch
    app->select(song, hydra::ui::SelectionSource::Click);
    REQUIRE(app->view_job != nullptr);
    latch.wait_entered(1);
    app->cancel_view();
    REQUIRE_FALSE(app->view_job->finished());
    CHECK(app->any_job_running());
    latch.release();
    settle(*app);
    CHECK_FALSE(app->any_job_running());
}

// The confirm counts library rows, every copy included, like the library
// does (D76), and reads which have a result from the store, not from the
// library's cached rows.
TEST_CASE("the confirm counts every library row, and rows with a result from the store") {
    ScratchPaths paths("appstate_confirm");
    auto store = std::make_unique<RecordStore>(paths.db);
    const ChartLibraryEntry first = library_entry(1);
    ChartLibraryEntry copy = library_entry(1);
    copy.notespath = "C:\\other\\hash001\\notes.chart";
    copy.rootfolder = "C:\\other";
    store->rebuild_chart_library({first, copy, library_entry(2)});
    AppState app(Settings{}, std::move(store));
    REQUIRE(app.library_shown_count() == 3);

    // Stored behind the library's back: no reload.
    hydra::test::save_batch_record(*app.store, first.md5, Settings{}.sp_cap);
    app.open_batch_confirm();

    CHECK(app.batch_scope_charts() == app.library_shown_count());
    CHECK(app.batch_scope_with_result() == 2);  // both copies of chart 1
}

// D115: the Analyze button runs the rows on screen, the search and the chip
// both applied. Ten charts match "hash00", four of them have a result, and the
// Not analyzed chip shows the other six: the confirm counts those six, the
// heading's number, not the ten the All chip counts.
TEST_CASE("the batch confirm counts the rows on screen when a search and a chip narrow them") {
    ScratchPaths paths("appstate_confirmchip");
    auto store = std::make_unique<RecordStore>(paths.db);
    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < 20; ++i) charts.push_back(library_entry(i));
    store->rebuild_chart_library(charts);
    for (int i : {0, 3, 5, 8})
        hydra::test::save_batch_record(*store, library_entry(i).md5, Settings{}.sp_cap);
    AppState app(Settings{}, std::move(store));

    app.set_search("hash00");
    app.library.set_chip(hydra::ui::StatusChip::NotAnalyzed);
    REQUIRE(app.library.counts().all == 10);
    REQUIRE(app.library_shown_count() == 6);
    app.open_batch_confirm();
    REQUIRE(app.batch_confirm_pending);
    CHECK(app.batch_scope_charts() == static_cast<int64_t>(app.library_shown_count()));
    CHECK(app.batch_scope_charts() < static_cast<int64_t>(app.library.counts().all));
    CHECK(app.batch_scope_with_result() == 0);
    app.close_batch_confirm();

    // The chip alone narrows the table too.
    app.set_search("");
    REQUIRE(app.library_shown_count() == 16);
    app.open_batch_confirm();
    REQUIRE(app.batch_confirm_pending);
    CHECK(app.batch_scope_charts() == 16);
}

// D79: the plan the confirm showed is the plan the batch runs. A result that
// lands after the confirm opened changes nothing the batch does, because the
// run reads no second list from the store.
TEST_CASE("the batch runs the plan the confirm showed, with no second store read") {
    ScratchPaths paths("appstate_confirmplan");
    auto store = std::make_unique<RecordStore>(paths.db);
    store->rebuild_chart_library({library_entry(1), library_entry(2)});
    AppState app(Settings{}, std::move(store));
    app.open_batch_confirm();
    REQUIRE(app.batch_scope_with_result() == 0);
    REQUIRE(app.batch_scope_charts() == 2);

    // Stored after the confirm opened: a second read would now skip this chart.
    hydra::test::save_batch_record(*app.store, library_entry(2).md5, Settings{}.sp_cap);
    {
        const ScopedHook seam(
            hydra::ui::set_app_batch_analyzer_for_test,
            [](const std::string&, const hydra::app::AnalysisSettings&,
               const std::function<void(float)>&) -> hydra::app::AnalysisResult {
                throw std::runtime_error("no chart file in this test");
            },
            1);
        app.start_batch(false);
    }
    REQUIRE(app.batch_job != nullptr);
    wait_batch_finished(app);

    const hydra::ui::BatchJob::Snapshot s = app.batch_job->snapshot();
    CHECK(s.total == 2);
    CHECK(s.skipped == 0);
    CHECK(s.failed == 2);
}

// D72 item 2: the startup open reads "couldn't open" even when SQLite only
// notices the junk at its first statement. main() shows it in a message box.
TEST_CASE("an AppState whose database can't open fails its startup open with DatabaseOpen") {
    ScratchPaths paths("appstate_junkdb");  // puts the overrides back when it ends
    hydra::test::write_junk_db(paths.db);
    AppState app;
    app.wait_store_open();
    CHECK_FALSE(app.store_ready());
    REQUIRE(app.store_open_failed());
    CHECK(app.store_open_job == nullptr);
    try {
        std::rethrow_exception(app.store_open_error());
    } catch (const hydra::KindedError& e) {
        CHECK(e.kind() == hydra::ErrorKind::DatabaseOpen);
    }
}

// ---- the startup open (DBUP) ------------------------------------------------

TEST_CASE("the startup open runs on its own thread and tick collects the store and library") {
    ScratchPaths paths("appstate_startopen");
    seeded_store(paths.db).reset();

    // Hold the open at its first report, as the GUI test's gate does.
    std::mutex mu;
    std::atomic<bool> open{false};
    std::vector<hydra::ui::StoreOpenProgress::Step> steps;
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>& cancelled) {
            {
                std::lock_guard<std::mutex> lock(mu);
                steps.push_back(p.step);
            }
            wait_until([&] { return open.load() || cancelled(); },
                       "the test to let the open go on");
        });
    AppState app;

    // The constructor returned with the open still held: nothing is ready,
    // the job counts as running, and a tick collects nothing.
    CHECK_FALSE(app.store_ready());
    CHECK_FALSE(app.store_open_failed());
    CHECK(app.any_job_running());
    app.tick(0.0);
    CHECK_FALSE(app.store_ready());

    open = true;
    wait_until([&] { return !app.store_open_job || app.store_open_job->finished(); },
               "the startup open to finish");
    app.tick(0.0);

    REQUIRE(app.store_ready());
    CHECK(app.store_open_job == nullptr);
    CHECK_FALSE(app.any_job_running());
    CHECK(app.library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(app.status_message.empty());
    // A file that needs no upgrade reports the open, then the library read.
    std::lock_guard<std::mutex> lock(mu);
    CHECK(steps == std::vector<hydra::ui::StoreOpenProgress::Step>{
                       hydra::ui::StoreOpenProgress::Step::Opening,
                       hydra::ui::StoreOpenProgress::Step::LoadingLibrary});
}

TEST_CASE("an AppState closed during its startup open stops the open") {
    ScratchPaths paths("appstate_startclose");
    seeded_store(paths.db).reset();
    std::atomic<bool> gave_up{false};
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress&, const std::function<bool()>& cancelled) {
            wait_until(cancelled, "the closing app to cancel the open");
            gave_up = true;
        });
    {
        AppState app;
        CHECK_FALSE(app.store_ready());
    }  // the destructor cancels the held open and joins it
    CHECK(gave_up);
}

TEST_CASE("a library read that fails after the startup open is the status line's problem") {
    ScratchPaths paths("appstate_startreadfail");
    seeded_store(paths.db).reset();
    // Dropped once the store is open (it would make a missing table again),
    // just before the library read.
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>&) {
            if (p.step == hydra::ui::StoreOpenProgress::Step::LoadingLibrary)
                hydra::test::exec_on_file(paths.db, "DROP TABLE charts; DROP TABLE records;");
        });
    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    CHECK(app.library.rows().empty());
    CHECK(app.status_is_problem);
    CHECK(app.status_message == kDatabaseReadSentence);
}

// D115 A1-6b (user decision 2026-10-14): a library file older than 2.2.0 is
// set aside and a fresh library starts. The app says so once, in an
// information box, and the scan that refills the library waits for the box.
TEST_CASE("an AppState on a pre-2.2.0 library sets it aside, leaves one notice, then asks for "
          "a scan") {
    ScratchPaths paths("appstate_setaside");
    const std::string aside = hydra::store::set_aside_path(paths.db);
    auto remove_aside = [&] {
        for (const std::string& f : hydra::store::with_side_files(aside))
            std::filesystem::remove(std::filesystem::u8path(f));
    };
    remove_aside();
    REQUIRE(scratch_settings().save());  // chart folders that a scan could read
    // A copy: the checked-in file is never opened.
    std::filesystem::copy_file(
        std::filesystem::u8path(corpus::testdata_root() + "/store/v2.1.0-schema4.db"),
        std::filesystem::u8path(paths.db), std::filesystem::copy_options::overwrite_existing);
    {
        AppState app;
        app.wait_store_open();
        REQUIRE(app.store_ready());
        CHECK(std::filesystem::exists(std::filesystem::u8path(aside)));
        CHECK(app.library.rows().empty());
        // The sentence waits for main.cpp's box, and the scan waits for the
        // box to close.
        CHECK(app.startup_notice() == hydra::app::old_library_set_aside_notice(paths.db));
        CHECK_FALSE(app.request_scan);
        CHECK_FALSE(app.scan_started_by_open);
        app.tick(0.0);
        CHECK_FALSE(app.request_scan);
        CHECK(app.startup_notice() == hydra::app::old_library_set_aside_notice(paths.db));

        app.startup_notice_closed();
        CHECK(app.startup_notice().empty());
        CHECK(app.request_scan);
        // Hydra starts this scan on its own, like the scan of an out-of-date
        // library, so it is the open's scan (D268).
        CHECK(app.scan_started_by_open);
        // Once: a second close asks for nothing more.
        app.request_scan = false;
        app.startup_notice_closed();
        CHECK_FALSE(app.request_scan);
    }
    remove_aside();
}

// D268: the scan after the set-aside box is one Hydra starts itself, so a chart
// folder it cannot read stops it with the open's sentence, not the manual one.
TEST_CASE("the scan after the set-aside box says the library needs a scan when a chart folder "
          "cannot be read") {
    ScratchPaths paths("appstate_setasidemissing");
    const std::string aside = hydra::store::set_aside_path(paths.db);
    auto remove_aside = [&] {
        for (const std::string& f : hydra::store::with_side_files(aside))
            std::filesystem::remove(std::filesystem::u8path(f));
    };
    remove_aside();
    const std::string missing = testtemp::temp_path("appstate_setasidemissing", "_nofolder");
    REQUIRE_FALSE(std::filesystem::exists(std::filesystem::u8path(missing)));
    Settings s = scratch_settings();
    s.chartfolders.push_back(missing);
    REQUIRE(s.save());
    std::filesystem::copy_file(
        std::filesystem::u8path(corpus::testdata_root() + "/store/v2.1.0-schema4.db"),
        std::filesystem::u8path(paths.db), std::filesystem::copy_options::overwrite_existing);
    {
        AppState app;
        app.wait_store_open();
        REQUIRE(app.store_ready());
        REQUIRE_FALSE(app.startup_notice().empty());
        app.startup_notice_closed();
        CHECK_FALSE(app.request_scan);
        CHECK_FALSE(app.scan_started_by_open);
        CHECK(app.scan_job == nullptr);
        CHECK(app.status_is_problem);
        CHECK(app.status_message ==
              hydra::app::library_scan_blocked_notice(missing, hydra::app::ScanAsker::Open));
    }
    remove_aside();
}

TEST_CASE("an AppState on a current library leaves no notice and asks for no scan") {
    ScratchPaths paths("appstate_nosetaside");
    REQUIRE(scratch_settings().save());  // chart folders that a scan could read
    seeded_store(paths.db).reset();
    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    CHECK(app.startup_notice().empty());
    CHECK_FALSE(app.request_scan);
    CHECK_FALSE(app.scan_started_by_open);
    app.startup_notice_closed();
    CHECK_FALSE(app.request_scan);
    CHECK_FALSE(std::filesystem::exists(
        std::filesystem::u8path(hydra::store::set_aside_path(paths.db))));
}

// ---- the scan on open of an out-of-date library -----------------------------

namespace {

// The seeded library on `paths`, its scan stamp set to one this build does
// not accept, with the corpus as the ini's chart folders plus `extra_folders`.
void seed_out_of_date_library(const ScratchPaths& paths,
                              const std::vector<std::string>& extra_folders = {}) {
    Settings s = scratch_settings();
    for (const std::string& f : extra_folders) s.chartfolders.push_back(f);
    REQUIRE(s.save());
    seeded_store(paths.db).reset();
    hydra::test::exec_on_file(paths.db,
                              "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
}

}  // namespace

TEST_CASE("an AppState on a library scanned by an older Hydra asks for a scan, marked as its own") {
    ScratchPaths paths("appstate_rescanopen");
    seed_out_of_date_library(paths);
    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    CHECK(app.startup_notice().empty());
    CHECK(app.request_scan);
    CHECK(app.scan_started_by_open);
    CHECK(app.status_message.empty());
}

TEST_CASE("an AppState on an empty library asks for no scan") {
    ScratchPaths paths("appstate_rescanempty");
    REQUIRE(scratch_settings().save());
    std::make_unique<RecordStore>(paths.db).reset();
    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    CHECK(app.library.rows().empty());
    CHECK_FALSE(app.request_scan);
    CHECK_FALSE(app.scan_started_by_open);
}

// A cancelled scan writes nothing, so the library stays out of date and the
// next open asks again; a finished one makes it current.
TEST_CASE("a cancelled scan on open leaves the library out of date; a finished one clears it") {
    ScratchPaths paths("appstate_rescancancel");
    seed_out_of_date_library(paths);
    AppState app;
    app.wait_store_open();
    REQUIRE(app.request_scan);
    app.request_scan = false;  // what the main window's next frame does

    // Cancelled before it starts, so the cancel cannot lose a race with a scan
    // that finishes first; the job checks the flag after reading the charts.
    app.scan_job = std::make_unique<hydra::ui::ScanJob>(app.settings.chartfolders, *app.store);
    app.scan_job->cancel();
    app.scan_job->start();
    wait_until([&] { return app.scan_job->snapshot().finished; }, "the cancelled scan to end");
    CHECK(app.scan_job->snapshot().cancelled);
    CHECK(app.store->chart_library_out_of_date());
    app.tick_library(0.0);
    CHECK(app.library_shown_count() == static_cast<size_t>(kChartCount));
    app.scan_job.reset();

    app.start_scan();
    REQUIRE(app.scan_job != nullptr);
    wait_until([&] { return app.scan_job->snapshot().finished; }, "the scan to finish");
    const hydra::ui::ScanProgress done = app.scan_job->snapshot();
    CHECK_FALSE(done.cancelled);
    CHECK_FALSE(app.store->chart_library_out_of_date());
    app.tick_library(0.0);
    CHECK(done.charts_found > 0);
    CHECK(app.library_shown_count() == static_cast<size_t>(done.charts_found));
}

TEST_CASE("a refused start_scan drops the open's mark so a later scan the user starts has no notice") {
    ScratchPaths paths("appstate_rescanrefused");
    seed_out_of_date_library(paths);
    AppState app;
    app.wait_store_open();
    REQUIRE(app.scan_started_by_open);
    app.settings.chartfolders.clear();  // can_scan says no
    app.start_scan();
    CHECK(app.scan_job == nullptr);
    CHECK_FALSE(app.scan_started_by_open);
}

// A scan over a folder it cannot list would drop that folder's charts and
// their results, so the open asks for no scan and says which folder.
TEST_CASE("an out-of-date library with a chart folder that cannot be read starts no scan") {
    ScratchPaths paths("appstate_rescanmissing");
    const std::string missing = testtemp::temp_path("appstate_rescanmissing", "_nofolder");
    REQUIRE_FALSE(std::filesystem::exists(std::filesystem::u8path(missing)));
    seed_out_of_date_library(paths, {missing});
    int64_t records_before = 0;
    {
        RecordStore before(paths.db);
        records_before = before.record_count();
    }
    REQUIRE(records_before > 0);

    AppState app;
    app.wait_store_open();
    app.tick(0.0);
    REQUIRE(app.store_ready());
    CHECK_FALSE(app.request_scan);
    CHECK_FALSE(app.scan_started_by_open);
    CHECK(app.scan_job == nullptr);
    CHECK(app.status_is_problem);
    CHECK(app.status_message ==
          hydra::app::library_scan_blocked_notice(missing, hydra::app::ScanAsker::Open));
    CHECK(app.library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(app.store->record_count() == records_before);
}

// D266 (user ruling "Refuse, like on open"): the scan the user starts stops at
// a missing chart folder too, and writes nothing. D268 ("Its own sentence"):
// it says nothing was scanned, not that the library needs a scan.
TEST_CASE("start_scan with a chart folder that cannot be read starts no scan and keeps every "
          "result") {
    ScratchPaths paths("appstate_scanmissing");
    const std::string missing = testtemp::temp_path("appstate_scanmissing", "_nofolder");
    REQUIRE_FALSE(std::filesystem::exists(std::filesystem::u8path(missing)));
    Settings s = scratch_settings();
    s.chartfolders.push_back(missing);
    REQUIRE(s.save());
    seeded_store(paths.db).reset();
    int64_t records_before = 0;
    {
        RecordStore before(paths.db);
        records_before = before.record_count();
    }
    REQUIRE(records_before > 0);

    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    REQUIRE_FALSE(app.request_scan);  // a current library asks for no scan
    REQUIRE(app.can_scan());          // so the toolbar's button stays on
    app.start_scan();
    CHECK(app.scan_job == nullptr);
    CHECK_FALSE(app.scan_started_by_open);
    CHECK(app.status_is_problem);
    CHECK(app.status_message ==
          hydra::app::library_scan_blocked_notice(missing, hydra::app::ScanAsker::User));
    app.tick_library(0.0);
    CHECK(app.library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(app.store->record_count() == records_before);
}

TEST_CASE("the startup screen's line names what the open is doing") {
    using hydra::ui::StoreOpenProgress;
    StoreOpenProgress p;
    CHECK(p.label() == "Opening your library...");
    p.step = StoreOpenProgress::Step::LoadingLibrary;
    CHECK(p.label() == "Opening your library...");

    p.upgrading = true;
    p.step = StoreOpenProgress::Step::ConvertingRecords;
    CHECK(p.label() == "Updating the results table...");
    p.step = StoreOpenProgress::Step::LoadingLibrary;
    CHECK(p.label() == "Finishing...");
}

// D72 item 4: the Analyze-library click asks the store which charts already
// have a result. When that read fails, the status line says so, no confirm
// opens, and no batch starts.
TEST_CASE("Analyze library on a database that fails shows the sentence and opens no confirm") {
    ScratchPaths paths("appstate_confirmfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE records");

    app->open_batch_confirm();
    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(app->status_is_problem);
    CHECK_FALSE(app->batch_confirm_pending);

    app->start_batch(false);
    CHECK(app->batch_job == nullptr);
}

// D73 item 3: a read that fails while Hydra runs shows the read sentence in
// the status line, and the screens keep what they showed. Before, the throw
// left the frame and Hydra closed with no message.
TEST_CASE("a library reload whose reads fail keeps the library and says so") {
    ScratchPaths paths("appstate_reloadfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE charts; DROP TABLE records;");

    app->reload_library();

    CHECK(app->status_is_problem);
    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(app->library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
}
