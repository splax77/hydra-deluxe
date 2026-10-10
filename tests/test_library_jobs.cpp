// Unit tests for the library and leaderboard jobs: Pause, Stop, the batch
// clock and time-left estimate, the chart being analyzed, the batch's counts,
// plain failure lines, and quiet cancels.
// The analyzer and the network are fakes; only the counts case reads one
// corpus chart, for a result the store can save.

#include "doctest.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/allocator.h"
#include "app/analysis.h"
#include "app/config.h"  // Settings::batch_run
#include "app/report.h"  // kNoChartLibrary
#include "app/report_files.h"
#include "app/user_messages.h"  // plain_error
#include "core/error_kind.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file
#include "display_fixtures.h"  // kTagOnlyTitle
#include "net/dmbot_client.h"
#include "scoped_hook.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/dm_jobs.h"
#include "ui/library_jobs.h"
#include "wait_util.h"

using hydra::app::AnalysisResult;
using hydra::app::AnalysisSettings;
using hydra::app::BatchRun;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStore;
using hydra::ui::BatchClock;
using hydra::ui::BatchJob;

namespace {

using namespace std::chrono_literals;
using testwait::wait_until;

std::vector<ChartLibraryEntry> fake_charts(int n) {
    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < n; ++i) {
        ChartLibraryEntry e;
        e.md5 = "fake" + std::to_string(i);
        e.title = "fake " + std::to_string(i);
        e.artist = "artist " + std::to_string(i);
        e.notespath = "fake_" + std::to_string(i) + ".chart";
        charts.push_back(e);
    }
    return charts;
}

BatchRun test_run() {
    BatchRun run;
    run.chartmode = "jobs-test";
    return run;
}

// The plan the confirm would hand over for `charts` when none has a result.
hydra::app::BatchPlan plan_of(const std::vector<ChartLibraryEntry>& charts) {
    std::vector<hydra::app::ScanItem> items;
    for (const ChartLibraryEntry& e : charts) items.push_back(hydra::ui::scan_item_of(e));
    return hydra::app::plan_batch(items, {});
}

// A chart that counts itself, waits for `release`, then fails with `error`,
// thrown as `kind` the way a real thrower names it.
hydra::app::ChartAnalyzer gated_failure(std::atomic<int>& started, std::atomic<bool>& release,
                                        std::string error,
                                        hydra::ErrorKind kind = hydra::ErrorKind::ChartUnreadable) {
    return [&started, &release, error, kind](const std::string&, const AnalysisSettings&,
                                             const std::function<void(float)>&) -> AnalysisResult {
        ++started;
        wait_until([&release] { return release.load(); }, "the test to release the chart");
        throw hydra::KindedError(kind, error);
    };
}

}  // namespace

TEST_CASE("jobs: the batch clock leaves paused time out") {
    BatchClock clock;
    clock.start(10.0);
    clock.pause(15.0);
    CHECK(clock.elapsed_s(20.0) == doctest::Approx(5.0));
    CHECK(clock.paused());
    clock.resume(25.0);
    CHECK(clock.elapsed_s(30.0) == doctest::Approx(10.0));
    clock.finish(40.0);
    CHECK(clock.elapsed_s(100.0) == doctest::Approx(20.0));

    BatchClock stopped_while_paused;
    stopped_while_paused.start(0.0);
    stopped_while_paused.pause(5.0);
    stopped_while_paused.finish(9.0);
    CHECK(stopped_while_paused.elapsed_s(50.0) == doctest::Approx(5.0));
    CHECK_FALSE(stopped_while_paused.paused());
}

TEST_CASE("jobs: time left needs three finished charts") {
    CHECK_FALSE(hydra::ui::batch_eta_s(30.0, 2, 10).has_value());
    REQUIRE(hydra::ui::batch_eta_s(30.0, 3, 10).has_value());
    CHECK(*hydra::ui::batch_eta_s(30.0, 3, 10) == doctest::Approx(70.0));
    CHECK(*hydra::ui::batch_eta_s(30.0, 10, 10) == doctest::Approx(0.0));
    CHECK_FALSE(hydra::ui::batch_eta_s(30.0, 3, 0).has_value());
}

TEST_CASE("jobs: pause lets the chart in flight finish and starts no new one") {
    std::atomic<int> started{0};
    std::atomic<bool> release{false};
    RecordStore store(":memory:");
    BatchJob job(plan_of(fake_charts(3)), test_run(), store);
    job.set_analyzer_for_test(gated_failure(started, release, "MD5 hashing failed"),
                              /*workers=*/1);
    job.start();
    wait_until([&] { return started.load() == 1; }, "the first chart to start");

    job.pause();
    release = true;
    wait_until([&] { return job.snapshot().completed == 1; }, "the chart in flight to finish");
    std::this_thread::sleep_for(200ms);
    CHECK(started.load() == 1);  // nothing new started while paused
    BatchJob::Snapshot paused = job.snapshot();
    CHECK(paused.paused);
    CHECK_FALSE(paused.eta_s.has_value());

    job.resume();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    BatchJob::Snapshot done = job.snapshot();
    CHECK(started.load() == 3);
    CHECK(done.completed == 3);
    CHECK(done.failed == 3);
    CHECK_FALSE(done.paused);
}

TEST_CASE("jobs: stop while paused ends the run and fails nothing") {
    std::atomic<int> started{0};
    std::atomic<bool> release{true};
    RecordStore store(":memory:");
    BatchJob job(plan_of(fake_charts(4)), test_run(), store);
    job.set_analyzer_for_test(gated_failure(started, release, "MD5 hashing failed"),
                              /*workers=*/2);
    job.pause();
    job.start();
    std::this_thread::sleep_for(200ms);
    CHECK(started.load() == 0);
    CHECK(job.snapshot().paused);

    job.stop();
    wait_until([&] { return job.snapshot().finished; }, "the stopped batch to finish");
    BatchJob::Snapshot s = job.snapshot();
    CHECK(started.load() == 0);
    CHECK(s.completed == 0);
    CHECK(s.failed == 0);
    CHECK(s.failures.empty());
    CHECK(job.is_cancelled());
}

TEST_CASE("jobs: the snapshot names the chart being analyzed and freezes its clock at the end") {
    std::atomic<int> started{0};
    std::atomic<bool> release{false};
    RecordStore store(":memory:");
    BatchJob job(plan_of(fake_charts(1)), test_run(), store);
    job.set_analyzer_for_test(gated_failure(started, release, "MD5 hashing failed"),
                              /*workers=*/1);
    job.start();
    wait_until([&] { return started.load() == 1; }, "the first chart to start");

    BatchJob::Snapshot running = job.snapshot();
    CHECK(running.current_title == "fake 0");
    CHECK(running.current_artist == "artist 0");
    CHECK(running.elapsed_s >= 0.0);
    CHECK_FALSE(running.eta_s.has_value());

    release = true;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    BatchJob::Snapshot done = job.snapshot();
    CHECK(done.current_title.empty());
    CHECK(done.current_artist.empty());
    std::this_thread::sleep_for(50ms);
    CHECK(job.snapshot().elapsed_s == done.elapsed_s);
}

TEST_CASE("jobs: the progress line's artist reads (unknown) when it is only tags") {
    // D50 item 5: the batch strip's artist follows the title's rule.
    std::atomic<int> started{0};
    std::atomic<bool> release{false};
    RecordStore store(":memory:");
    std::vector<ChartLibraryEntry> charts = fake_charts(1);
    charts[0].artist = hydra::test::kTagOnlyTitle;
    BatchJob job(plan_of(charts), test_run(), store);
    job.set_analyzer_for_test(gated_failure(started, release, "MD5 hashing failed"),
                              /*workers=*/1);
    job.start();
    wait_until([&] { return started.load() == 1; }, "the first chart to start");
    CHECK(job.snapshot().current_artist == "(unknown)");
    release = true;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
}

TEST_CASE("jobs: a failed chart reads in plain words and keeps the raw text") {
    std::atomic<int> started{0};
    std::atomic<bool> release{true};
    RecordStore store(":memory:");
    BatchJob job(plan_of(fake_charts(1)), test_run(), store);
    job.set_analyzer_for_test(
        gated_failure(started, release, "cannot open file: C:\\Songs\\x\\notes.chart",
                      hydra::ErrorKind::SongFileMissing),
        1);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    BatchJob::Snapshot s = job.snapshot();
    REQUIRE(s.failures.size() == 1);
    REQUIRE(s.failure_details.size() == 1);
    CHECK(s.failures[0] ==
          "fake 0: Hydra couldn't open the song file. It may have been moved or deleted; "
          "run Scan library to update the library.");
    CHECK(s.failure_details[0] == "fake 0: cannot open file: C:\\Songs\\x\\notes.chart");
}

// D79: a run that fails as a whole is not a library row. It finishes with its
// error shown and counts no chart. A pool of no workers is one such failure.
// (The store read that D72 item 4 covered now happens in the confirm, before
// the job: test_app_state.)
TEST_CASE("jobs: a batch that fails as a whole shows its error and counts no chart") {
    RecordStore store(":memory:");
    std::atomic<int> started{0};
    std::atomic<bool> release{true};
    BatchJob job(plan_of(fake_charts(2)), test_run(), store);
    job.set_analyzer_for_test(gated_failure(started, release, "never analyzed"), /*workers=*/0);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

    const BatchJob::Snapshot s = job.snapshot();
    CHECK(started.load() == 0);
    CHECK(s.failed == 0);
    CHECK(s.completed == 0);
    CHECK(s.failures.empty());
    CHECK_FALSE(s.run_error.empty());
    CHECK(s.run_error_detail.find("worker count") != std::string::npos);
}

TEST_CASE("jobs: the snapshot's counts come from the batch in one piece") {
    // Finding 142: one progress callback writes all five numbers, so no frame
    // can read a failure counted before its chart is.
    // The two charts that succeed store one real chart's result.
    const AnalysisResult real = corpus::first_analyzed_with_paths(AnalysisSettings{});
    RecordStore store(":memory:");
    BatchJob job(plan_of(fake_charts(3)), test_run(), store);
    job.set_analyzer_for_test(
        [&real](const std::string& path, const AnalysisSettings&,
                const std::function<void(float)>&) -> AnalysisResult {
            if (path == "fake_1.chart") throw std::runtime_error("MD5 hashing failed");
            return real;
        },
        /*workers=*/1);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

    const BatchJob::Snapshot s = job.snapshot();
    CHECK(s.analyzed == 2);
    CHECK(s.failed == 1);
    CHECK(s.completed == 3);
    CHECK(s.skipped == 0);
    CHECK(s.total == 3);
    CHECK(job.batch_run().lens == test_run().lens);
    CHECK(job.batch_run().chartmode == test_run().chartmode);
}

// D88: every exe runs on mimalloc, this one too. An exe whose link puts
// mimalloc.dll after another DLL keeps the Windows heap without a word; the
// memory wave's join did that to every GUI exe. The Debug runtime is never
// redirected, so only a Release build can tell.
#ifdef NDEBUG
TEST_CASE("allocator: malloc goes to mimalloc") {
    CHECK(hydra::app::malloc_redirected());
}
#endif

namespace {

// The memory each chart's result carries in the hand-back tests below.
constexpr size_t kBlock = 1024;  // small blocks, as a chart's are
constexpr size_t kChartBytes = 128 * 1024 * 1024;

// An analyzer that returns a copy of `real` carrying kChartBytes, filled so
// it is committed.
hydra::app::ChartAnalyzer analyzer_carrying_chart_bytes(const AnalysisResult& real) {
    return [&real](const std::string&, const AnalysisSettings&,
                   const std::function<void(float)>&) -> AnalysisResult {
        AnalysisResult r = real;
        r.song.features.assign(kChartBytes / kBlock, std::string(kBlock, 'x'));
        return r;
    };
}

// Runs `job_body`, which builds a job, runs it to its end and destroys it
// (joining its thread), then checks the memory its charts freed went back
// to Windows. `what` names the job in the message.
void check_job_hands_memory_back(const std::string& what, const std::function<void()>& job_body) {
#ifdef NDEBUG
    // On the Windows heap the charts' memory never reaches mimalloc, and the
    // check below passes whatever the job did.
    REQUIRE(hydra::app::malloc_redirected());
#endif
    hydra::app::return_freed_memory();
    const size_t before = hydra::app::committed_bytes();
    job_body();
    const size_t after = hydra::app::committed_bytes();
    MESSAGE("committed before the " << what << " " << before / (1024 * 1024) << " MB, after it "
                                    << after / (1024 * 1024) << " MB");
    // A test margin, not an app number: less than half of one chart's
    // memory may still be committed.
    CHECK(after < before + kChartBytes / 2);
}

}  // namespace

// D95 call 1: once a batch ends, the memory its charts freed goes back to
// Windows at once, not after mimalloc's purge delay. As in a real batch, each
// result is built on a worker and freed on the batch's own thread once
// stored; here each one carries kChartBytes.
TEST_CASE("jobs: a finished batch hands the memory its charts freed back to Windows") {
    const AnalysisResult real = corpus::first_analyzed_with_paths(AnalysisSettings{});
    check_job_hands_memory_back("batch", [&real] {
        RecordStore store(":memory:");
        BatchJob job(plan_of(fake_charts(4)), test_run(), store);
        job.set_analyzer_for_test(analyzer_carrying_chart_bytes(real), /*workers=*/2);
        job.start();
        wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    });
}

// D95 call 1, for the path report's build: it analyzes every chart too, so
// once it ends the memory those charts freed goes back to Windows at once.
// Here the one listed chart's result carries kChartBytes.
TEST_CASE("jobs: a finished report build hands the memory its charts freed back to Windows") {
    const AnalysisResult real = corpus::first_analyzed_with_paths(AnalysisSettings{});
    // The run the default settings make, so the stored result is under its key.
    const BatchRun run = hydra::app::Settings{}.batch_run();
    RecordStore store(":memory:");
    hydra::test::name_chart(store, "fake0", "fake 0");
    hydra::test::store_batch_result(store, "fake0", run.cap_query().exact);
    check_job_hands_memory_back("report build", [&] {
        const ScopedHook seam(hydra::ui::set_report_analyzer_for_test,
                              analyzer_carrying_chart_bytes(real));
        hydra::ui::ReportJob job(store, run.cap_query(), run.lens, hydra::kDefaultHitWindowMs,
                                 run);
        job.start();
        wait_until([&] { return job.finished(); }, "the report build to finish");
        INFO(job.message());
        REQUIRE(job.ok());  // the chart was analyzed: the build has rows
    });
}

TEST_CASE("jobs: a report job carries the cap and lens it was built from") {
    RecordStore store(":memory:");
    const hydra::store::CapQuery cap = hydra::store::CapQuery::at(6);
    const hydra::store::Lens lens = hydra::store::Lens::from(std::optional<int>(20), 1, 7);
    const hydra::ui::ReportJob job(store, cap, lens, 85.5);
    CHECK(job.cap() == cap);
    CHECK(job.lens() == lens);
    CHECK(job.hit_window_ms() == 85.5);  // the decimal is kept (D51 call 15)
}

// D97: an empty report shows generate_report's own reason when it gives one,
// word for word. Results with no chart library name the missing
// library; a database with no results gets the app's own sentence.
TEST_CASE("jobs: an empty report shows generate_report's reason, or the app's own") {
    const BatchRun run = test_run();

    RecordStore no_library(":memory:");
    hydra::test::store_batch_result(no_library, "orphan", run.cap_query().exact);
    hydra::ui::ReportJob orphaned(no_library, run.cap_query(), run.lens,
                                  hydra::kDefaultHitWindowMs, run);
    orphaned.start();
    wait_until([&] { return orphaned.finished(); }, "the report with no library to finish");
    CHECK_FALSE(orphaned.ok());
    CHECK(orphaned.message() == std::string(hydra::app::report::kNoChartLibrary));

    RecordStore empty(":memory:");
    hydra::ui::ReportJob nothing(empty, run.cap_query(), run.lens,
                                 hydra::kDefaultHitWindowMs, run);
    nothing.start();
    wait_until([&] { return nothing.finished(); }, "the report with no results to finish");
    CHECK_FALSE(nothing.ok());
    CHECK(nothing.message() ==
          hydra::app::plain_error(hydra::KindedError(hydra::ErrorKind::NoRecords, "x")));
}

// Memory audit fix 4: the batch's rows are read by the report's one pass and
// nothing after it, so the job lets them go then, not when the finished
// strip is dismissed. An empty store ends the pass with no page, so no file
// is written.
TEST_CASE("jobs: a report job lets go of the batch's rows once its pass is done") {
    RecordStore store(":memory:");
    const BatchRun run = test_run();
    hydra::app::report::ReportSeed seed = hydra::app::report::ReportSeed::for_run(run);
    seed.rows["fake0"] = {};
    hydra::ui::ReportJob job(store, run.cap_query(), run.lens, 85.5, run, std::move(seed));
    REQUIRE(job.seed_charts_for_test() == 1);
    job.start();
    wait_until([&] { return job.finished(); }, "the report job to finish");
    CHECK(job.seed_charts_for_test() == 0);
}

TEST_CASE("jobs: a cancelled leaderboard fetch is not an error") {
    int opens = 0;
    // Before the jobs, which join the threads that call these.
    const ScopedHook browser(hydra::app::set_open_in_browser, [&](const std::wstring&) {
        ++opens;
        return true;
    });
    RecordStore store(":memory:");

    // What the WinHTTP fetch does when cancel lands between read chunks.
    {
        const ScopedHook fetcher(
            hydra::net::set_fetcher,
            [](const std::string&, const std::atomic<bool>* cancel) -> std::string {
                wait_until([cancel] { return cancel->load(); }, "the fetch to be cancelled");
                throw std::runtime_error("cancelled");
            });
        hydra::ui::DmReportJob thrown(store, "1", "someone", "jobs-test", hydra::store::Lens{});
        thrown.start();
        thrown.cancel();
        wait_until([&] { return thrown.finished(); }, "the cancelled fetch to finish");
        CHECK_FALSE(thrown.ok());
        CHECK(thrown.message().empty());
    }

    // The server answers after the cancel: nothing is written or opened.
    const ScopedHook fetcher(
        hydra::net::set_fetcher,
        [](const std::string&, const std::atomic<bool>* cancel) -> std::string {
            wait_until([cancel] { return cancel->load(); }, "the fetch to be cancelled");
            return "{}";
        });
    hydra::ui::DmReportJob late(store, "1", "someone", "jobs-test", hydra::store::Lens{});
    late.start();
    late.cancel();
    wait_until([&] { return late.finished(); }, "the late fetch to finish");
    CHECK_FALSE(late.ok());
    CHECK(late.message().empty());
    CHECK(opens == 0);
}

TEST_CASE("jobs: a failed leaderboard fetch says what to do") {
    const ScopedHook fetcher(
        hydra::net::set_fetcher, [](const std::string&, const std::atomic<bool>*) -> std::string {
            throw hydra::KindedError(hydra::ErrorKind::NetUnreachable,
                                     "could not send the request (error 12029)");
        });
    hydra::ui::DmFetchUsersJob job;
    job.start();
    wait_until([&] { return job.finished(); }, "the failed fetch to finish");
    CHECK_FALSE(job.ok());
    CHECK(job.message() ==
          "Hydra couldn't reach dmleaderboards. Check your internet connection and try again.");
    CHECK(job.error() == "could not send the request (error 12029)");
}

// Finding 212: run_guarded and AnalyzeJob::start's thread-start catch record a
// failure through one fail(). This job exposes it so the test can call it.
TEST_CASE("jobs: a failed job records the raw text, the plain message, not ok and finished") {
    struct FailingJob : hydra::ui::ResultJobBase {
        using ResultJobBase::fail;
    };
    const std::runtime_error boom("boom");

    FailingJob job;
    job.fail(boom);
    CHECK(job.error() == "boom");
    CHECK(job.message() == hydra::app::plain_error(boom));
    CHECK_FALSE(job.ok());
    CHECK(job.finished());

    // A cancel is the user's own click: the raw text stays, the message is empty.
    FailingJob cancelled;
    cancelled.cancel();
    cancelled.fail(boom);
    CHECK(cancelled.error() == "boom");
    CHECK(cancelled.message().empty());
    CHECK_FALSE(cancelled.ok());
    CHECK(cancelled.finished());
}

// The batch turns each library entry into a scan row through scan_item_of.
TEST_CASE("jobs: scan_item_of copies a library entry's fields by name") {
    ChartLibraryEntry e;
    e.md5 = "md5 a";
    e.title = "title b";
    e.artist = "artist c";
    e.charter = "charter d";
    e.notespath = "notes e.chart";
    e.rootfolder = "root f";
    e.sig = "sig g";
    const hydra::app::ScanItem item = hydra::ui::scan_item_of(e);
    CHECK(item.md5 == "md5 a");
    CHECK(item.title == "title b");
    CHECK(item.artist == "artist c");
    CHECK(item.charter == "charter d");
    CHECK(item.notespath == "notes e.chart");
    CHECK(item.rootfolder == "root f");
    CHECK(item.sig.empty());  // as the batch's rows have always had it
}
