// Unit tests for the library and leaderboard jobs: Pause, Stop, the batch
// clock and time-left estimate, the chart being analyzed, the batch's counts,
// plain failure lines, quiet cancels, the startup open's step that reads
// saved paths, and its counted read of the library's charts and results. One
// opt-in case times the startup library read on a library file the caller
// names.
// The batch cases run the real analyzer on corpus charts, or hold it at a gate
// and then run it. A chart that must fail is a scanned corpus chart whose song
// file was then deleted. A few cases still use a stand-in analyzer: the ones
// that need a result of a chosen size, or a chart held at the store's lookup.
// The leaderboard cases use the real transport, and where a job would reach
// dmleaderboards itself, the seam hands it a failure the real transport
// threw. The startup open's cases read libraries a scan of the corpus writes.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "app/allocator.h"
#include "app/analysis.h"
#include "app/config.h"  // Settings::batch_run
#include "app/report.h"  // kNoChartLibrary
#include "app/report_files.h"
#include "app/time_left.h"
#include "app/user_messages.h"  // plain_error
#include "app/work_pool.h"      // run_work_pool
#include "core/error_kind.h"
#include "core/rules.h"  // RulesStamp
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file
#include "display_fixtures.h"  // save_batch_record, save_old_build_record, other_rules_record
#include "env_util.h"  // read_env
#include "net/dmbot_client.h"
#include "parse/song.h"  // NoNotesError
#include "scoped_hook.h"
#include "store/record_store.h"
#include "store_fixtures.h"  // save_as_old_build
#include "store_lock_holder.h"
#include "temp_util.h"
#include "ui/dm_jobs.h"
#include "ui/library_jobs.h"
#include "ui/library_model.h"
#include "wait_util.h"

using hydra::app::AnalysisResult;
using hydra::app::AnalysisSettings;
using hydra::app::BatchRun;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStore;
using hydra::test::StoreLockHolder;
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
    run.profile.chartmode = "jobs-test";
    return run;
}

// The plan the confirm would hand over for `charts` when none has a result.
hydra::app::BatchPlan plan_of(const std::vector<ChartLibraryEntry>& charts) {
    return hydra::app::plan_batch(charts, {});
}

// The library rows a scan of the corpus writes (discover_charts, the scan's
// own reader), read once. A discovery error throws: the corpus is checked in.
const std::vector<ChartLibraryEntry>& corpus_scan() {
    static const std::vector<ChartLibraryEntry> rows = [] {
        auto [items, errors] = hydra::app::discover_charts({corpus::root()});
        if (!errors.empty()) throw std::runtime_error("corpus scan error: " + errors.front());
        return items;
    }();
    return rows;
}

// The scan's row for the corpus chart at `notespath`.
ChartLibraryEntry corpus_row(const std::string& notespath) {
    for (const ChartLibraryEntry& e : corpus_scan())
        if (e.notespath == notespath) return e;
    throw std::runtime_error("the corpus scan has no row for " + notespath);
}

// The scan's rows for the first `n` (at most 4) corpus charts that analyze to
// a path under the default settings (corpus::charts_with_paths), so the real
// analyzer saves a result for each.
std::vector<ChartLibraryEntry> corpus_rows(size_t n) {
    static const std::vector<ChartLibraryEntry> four = [] {
        std::vector<ChartLibraryEntry> rows;
        for (const corpus::ChartWithPaths& c : corpus::charts_with_paths(AnalysisSettings{}, 4))
            rows.push_back(corpus_row(c.chart));
        return rows;
    }();
    REQUIRE(n <= four.size());
    return std::vector<ChartLibraryEntry>(four.begin(),
                                          four.begin() + static_cast<std::ptrdiff_t>(n));
}

// An empty scratch folder for this run (testtemp::temp_dir, cleared).
std::string empty_dir(const std::string& tag) {
    testtemp::remove_dir(testtemp::temp_dir(tag));
    return testtemp::temp_dir(tag);
}

// The scan's row for a copy of the chart `row` names, put in the empty folder
// `dir`: its chart file and, when its folder has one, its song.ini, read back
// by discover_charts. Same chart, so same md5, in a second folder.
ChartLibraryEntry copied_row(const ChartLibraryEntry& row, const std::string& dir) {
    const std::filesystem::path chart = hydra::os_path(row.notespath);
    const std::filesystem::path to = hydra::os_path(dir);
    std::filesystem::copy_file(chart, to / chart.filename());
    const std::filesystem::path ini = chart.parent_path() / "song.ini";
    if (std::filesystem::exists(ini)) std::filesystem::copy_file(ini, to / "song.ini");
    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    for (const ChartLibraryEntry& e : items)
        if (e.md5 == row.md5) return e;
    throw std::runtime_error("the scan of " + dir + " has no copy of " + row.notespath);
}

// copied_row, then the copy's chart file deleted, as a song moved or deleted
// after the scan leaves its row: the row stays and its file is gone.
ChartLibraryEntry missing_copy(const ChartLibraryEntry& row, const std::string& dir) {
    const ChartLibraryEntry copy = copied_row(row, dir);
    std::filesystem::remove(hydra::os_path(copy.notespath));
    REQUIRE_FALSE(std::filesystem::exists(hydra::os_path(copy.notespath)));
    return copy;
}

// The error the real analyzer throws for the chart at `notespath`, which must
// fail to analyze.
hydra::KindedError analysis_failure(const std::string& notespath) {
    try {
        (void)hydra::app::analyze_chart_file(notespath, AnalysisSettings{});
    } catch (const hydra::KindedError& e) {
        return e;
    }
    throw std::runtime_error("the analysis of " + notespath + " did not fail");
}

// The scan's row for the first corpus MIDI chart, copied alone into the empty
// folder `dir` with a song.ini of `ini_lines`. A MIDI file states no names of
// its own, so the row's names are the ini's.
ChartLibraryEntry midi_row_with_ini(const std::string& dir, const std::string& ini_lines) {
    const std::filesystem::path chart = hydra::os_path(corpus::first_chart_with_suffix(".mid"));
    const std::filesystem::path to = hydra::os_path(dir);
    std::filesystem::copy_file(chart, to / chart.filename());
    const std::string ini = "[song]\n" + ini_lines;
    testtemp::write_bytes(dir + "\\song.ini", std::vector<uint8_t>(ini.begin(), ini.end()));
    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    return items.front();
}

// Seconds left by the batch strip's rule (D283): the charts left, `total`
// less `done`, over the rate of the window, `window` charts in the time from
// the earliest of the window's finishes (`window_start_s`) to `now_s`.
double left_at_window_rate(double now_s, double window_start_s, int window, double done,
                           double total) {
    return (total - done) * (now_s - window_start_s) / window;
}

// The real analyzer (app::analyze_chart_file), with nothing in front of it.
hydra::app::ChartAnalyzer real_analyzer() {
    return [](const std::string& path, const AnalysisSettings& settings,
              const std::function<void(float)>& on_progress) -> AnalysisResult {
        return hydra::app::analyze_chart_file(path, settings, on_progress);
    };
}

// The real analyzer, held at the start of each chart until the test lets it
// go: the chart that started n-th (from 0) counts itself in `started`, waits
// until `released` is above n, then is analyzed for real.
hydra::app::ChartAnalyzer real_analysis_in_turn(std::atomic<int>& started,
                                                std::atomic<int>& released) {
    return [&started, &released](const std::string& path, const AnalysisSettings& settings,
                                 const std::function<void(float)>& on_progress) -> AnalysisResult {
        const int me = started++;
        wait_until([&released, me] { return released.load() > me; }, "the test to release it");
        return hydra::app::analyze_chart_file(path, settings, on_progress);
    };
}

// Waits for a step the batch reaches on its way, and fails at once if the
// batch finishes without reaching it (wait_until_or_gone in wait_util.h).
template <class Done>
void wait_for_batch_step(const BatchJob& job, Done done, const std::string& what) {
    testwait::wait_until_or_gone(done, [&job] { return job.snapshot().finished; }, what,
                                 "the batch has finished, so it can no longer reach this step");
}

}  // namespace

// AOA-W3 V9-SN1: an exception out of the scan ends the job with the error as
// its one problem line. It writes no library: the library a real scan of the
// corpus saved before stays as it was.
TEST_CASE("jobs: a scan that throws finishes with its error as a problem") {
    RecordStore store(":memory:");
    REQUIRE_FALSE(hydra::app::save_scan_as_library(store, corpus_scan()).has_value());
    const std::vector<ChartLibraryEntry> before = store.list_chart_library(0, -1);
    REQUIRE_FALSE(before.empty());
    const std::runtime_error broke("scan broke");
    const ScopedHook hold(hydra::ui::set_scan_hold_for_test, [&broke] { throw broke; });
    hydra::ui::ScanJob job({corpus::root()}, store);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the scan to finish");
    const hydra::ui::ScanProgress p = job.snapshot();
    REQUIRE(p.errors.size() == 1);
    CHECK(p.errors[0] == hydra::app::plain_error_line(broke));
    CHECK(p.phase == hydra::ui::ScanProgress::Phase::Done);
    CHECK_FALSE(p.cancelled);
    const std::vector<ChartLibraryEntry> after = store.list_chart_library(0, -1);
    REQUIRE(after.size() == before.size());
    for (size_t i = 0; i < before.size(); ++i) {
        CHECK(after[i].md5 == before[i].md5);
        CHECK(after[i].notespath == before[i].notespath);
    }
}

// Elapsed is the end less the start less the time spent paused. The end is
// the reading's time while running, the pause's start while paused, and the
// finish once finished.
TEST_CASE("jobs: the batch clock leaves paused time out") {
    const double start = 10.0;
    const double pause = 15.0;
    const double resume = 25.0;
    const double finish = 40.0;
    BatchClock clock;
    clock.start(start);
    clock.pause(pause);
    REQUIRE(clock.paused());
    // Paused: two readings far apart give the same answer.
    CHECK(clock.elapsed_s(pause + 5.0) == doctest::Approx(pause - start));
    CHECK(clock.elapsed_s(resume) == doctest::Approx(pause - start));
    clock.resume(resume);
    const double paused_for = resume - pause;
    const double running_at = 30.0;
    CHECK(clock.elapsed_s(running_at) == doctest::Approx(running_at - start - paused_for));
    clock.finish(finish);
    CHECK(clock.elapsed_s(100.0) == doctest::Approx(finish - start - paused_for));

    // Finished while paused: the end is the pause's start, not the finish.
    const double stop_start = 0.0;
    const double stop_pause = 5.0;
    BatchClock stopped_while_paused;
    stopped_while_paused.start(stop_start);
    stopped_while_paused.pause(stop_pause);
    stopped_while_paused.finish(9.0);
    CHECK(stopped_while_paused.elapsed_s(50.0) == doctest::Approx(stop_pause - stop_start));
    CHECK_FALSE(stopped_while_paused.paused());
}

// The batch strip's window is the last 2W finished charts (D283). These cases
// use W = 4, so the window is 8 charts, and a batch of 40. Charts arrive in
// save groups, so several share one finish time, as the strip sees them.
TEST_CASE("jobs: time left is the rate over the last 2W finished charts") {
    const int window = hydra::ui::kTimeLeftWorkerRounds * 4;
    REQUIRE(window == 8);  // the save groups below fill a window of 8
    const double total = 40;
    hydra::app::TimeLeftClock clock = hydra::app::TimeLeftClock::over_last_units(window);
    clock.record(1.0, 8);    // 8 charts that had a quick answer, in one group
    clock.record(11.0, 12);  // then 4 slow ones
    clock.record(21.0, 16);  // and 4 more
    // The last 8 finishes are the two slow groups, so the window starts at
    // the earlier one, 11 s.
    const std::optional<double> first = clock.read(25.0, total);
    REQUIRE(first.has_value());
    CHECK(*first == doctest::Approx(left_at_window_rate(25.0, 11.0, window, 16, total)));
    // Not the mean since the start: 16 charts in 25 s.
    CHECK(*first != doctest::Approx((total - 16) * 25.0 / 16));

    // A second reading inside the same second returns the first, however
    // many charts landed in between.
    clock.record(25.5, 20);
    const std::optional<double> held = clock.read(25.9, total);
    REQUIRE(held.has_value());
    CHECK(*held == doctest::Approx(*first));
    // A second on, the window has moved to the last 8: the groups at 21 s
    // and 25.5 s, so it starts at 21 s.
    const std::optional<double> next = clock.read(26.0, total);
    REQUIRE(next.has_value());
    CHECK(*next == doctest::Approx(left_at_window_rate(26.0, 21.0, window, 20, total)));
}

TEST_CASE("jobs: time left shows nothing until 2W charts have finished, then shows") {
    const int window = hydra::ui::kTimeLeftWorkerRounds * 4;
    const double total = 40;
    hydra::app::TimeLeftClock clock = hydra::app::TimeLeftClock::over_last_units(window);
    clock.record(1.0, window - 1);
    CHECK_FALSE(clock.read(2.0, total).has_value());
    clock.record(3.0, window);
    const std::optional<double> at_window = clock.read(4.0, total);
    REQUIRE(at_window.has_value());
    // The window's earliest finish is the group at 1 s.
    CHECK(*at_window == doctest::Approx(left_at_window_rate(4.0, 1.0, window, window, total)));
}

TEST_CASE("jobs: the batch strip's time left waits for two rounds of its workers") {
    // One worker, so the window is 2 charts. Each corpus chart waits for its
    // turn, then is analyzed for real.
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    BatchJob job(plan_of(corpus_rows(4)), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
    job.start();

    released = 1;
    wait_for_batch_step(job, [&] { return job.snapshot().completed == 1; },
                        "the first chart to finish");
    CHECK_FALSE(job.snapshot().eta_s.has_value());
    released = 2;
    wait_for_batch_step(job, [&] { return job.snapshot().completed == 2; },
                        "the second chart to finish");
    CHECK(job.snapshot().eta_s.has_value());

    released = 4;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    CHECK_FALSE(job.snapshot().eta_s.has_value());
}

// Paused time never counts (D283): a chart that finishes during a pause is
// timed at the pause, and the window after the resume spans only run time.
TEST_CASE("jobs: time left leaves a pause out of its window") {
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    BatchJob job(plan_of(corpus_rows(4)), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released),
                              /*workers=*/1);  // a window of 2
    job.start();
    wait_for_batch_step(job, [&] { return started.load() == 1; }, "the first chart to start");

    job.pause();
    released = 1;
    wait_for_batch_step(job, [&] { return job.snapshot().completed == 1; },
                        "the chart in flight to finish");
    CHECK_FALSE(job.snapshot().eta_s.has_value());
    constexpr auto kPause = 2000ms;  // longer than the run time that follows
    std::this_thread::sleep_for(kPause);

    const auto resumed = std::chrono::steady_clock::now();
    job.resume();
    released = 2;
    wait_for_batch_step(job, [&] { return job.snapshot().completed == 2; },
                        "the second chart to finish");
    const double run_s =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - resumed).count();
    const double pause_s = std::chrono::duration<double>(kPause).count();
    // The second chart's real analysis must take less than the pause, or the
    // check below cannot tell run time from paused time.
    REQUIRE(run_s < pause_s);
    // Two charts left over a rate of 2 charts in the window's span: the span
    // in seconds, which is run time only, so at most run_s. Counting the
    // pause would make it pause_s or more.
    const std::optional<double> eta = job.snapshot().eta_s;
    REQUIRE(eta.has_value());
    CHECK(*eta <= run_s);

    released = 4;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
}

TEST_CASE("jobs: pause lets the chart in flight finish and starts no new one") {
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    BatchJob job(plan_of(corpus_rows(3)), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
    job.start();
    wait_for_batch_step(job, [&] { return started.load() == 1; }, "the first chart to start");

    job.pause();
    released = 3;
    wait_for_batch_step(job, [&] { return job.snapshot().completed == 1; },
                        "the chart in flight to finish");
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
    CHECK(done.analyzed == 3);
    CHECK(done.failed == 0);
    CHECK_FALSE(done.paused);
}

TEST_CASE("jobs: stop while paused ends the run and fails nothing") {
    std::atomic<int> started{0};
    std::atomic<int> released{4};  // no chart is held: each may run at once
    RecordStore store(":memory:");
    BatchJob job(plan_of(corpus_rows(4)), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/2);
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
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    const std::vector<ChartLibraryEntry> charts = corpus_rows(1);
    REQUIRE_FALSE(charts[0].title.empty());
    REQUIRE_FALSE(charts[0].artist.empty());
    BatchJob job(plan_of(charts), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
    job.start();
    wait_for_batch_step(job, [&] { return started.load() == 1; }, "the first chart to start");

    BatchJob::Snapshot running = job.snapshot();
    CHECK(running.current_title == hydra::display_title(charts[0].title));
    CHECK(running.current_artist == hydra::display_artist(charts[0].artist));
    CHECK_FALSE(running.eta_s.has_value());
    // The clock runs while the chart is held.
    const double first_reading = running.elapsed_s;
    wait_for_batch_step(job, [&] { return job.snapshot().elapsed_s > first_reading; },
                        "the batch clock to move on while the chart runs");

    released = 1;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    BatchJob::Snapshot done = job.snapshot();
    CHECK(done.current_title.empty());
    CHECK(done.current_artist.empty());
    std::this_thread::sleep_for(50ms);
    CHECK(job.snapshot().elapsed_s == done.elapsed_s);
}

TEST_CASE("jobs: the progress line's artist reads (unknown) when it is only tags") {
    // D50 item 5: the batch strip's artist follows the title's rule.
    // A scanned song folder whose song.ini artist is only tags.
    const ChartLibraryEntry row =
        midi_row_with_ini(empty_dir("jobs_tag_artist"), std::string("name = Song\nartist = ") +
                                                            hydra::test::kTagOnlyTitle + "\n");
    REQUIRE(row.artist == hydra::test::kTagOnlyTitle);
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    BatchJob job(plan_of({row}), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
    job.start();
    wait_for_batch_step(job, [&] { return started.load() == 1; }, "the first chart to start");
    CHECK(job.snapshot().current_artist == hydra::kUnknownTitle);
    released = 1;
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
}

TEST_CASE("jobs: a failed chart reads in plain words and keeps the raw text") {
    // A corpus chart's row whose song file was deleted after the scan.
    const ChartLibraryEntry gone = missing_copy(corpus_rows(1)[0], empty_dir("jobs_failed_chart"));
    const hydra::KindedError failure = analysis_failure(gone.notespath);
    REQUIRE(failure.kind() == hydra::ErrorKind::SongFileMissing);
    const std::string raw = hydra::app::plain_error_detail(failure);
    REQUIRE_FALSE(raw.empty());  // the plain sentence does not quote the raw text

    RecordStore store(":memory:");
    BatchJob job(plan_of({gone}), test_run(), store);
    job.set_analyzer_for_test(real_analyzer(), /*workers=*/1);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    BatchJob::Snapshot s = job.snapshot();
    REQUIRE(s.failures.size() == 1);
    REQUIRE(s.failure_details.size() == 1);
    // The chart's title, then the plain sentence for a missing song file.
    CHECK(s.failures[0] == gone.title + ": " + hydra::app::plain_error(failure));
    CHECK(s.failures[0].find(raw) == std::string::npos);
    // The raw text is kept under the same title.
    CHECK(s.failure_details[0] == gone.title + ": " + raw);
}

// D79: a run that fails as a whole is not a library row. It finishes with its
// error shown and counts no chart. A pool of no workers is one such failure.
// (The store read that D72 item 4 covered now happens in the confirm, before
// the job: test_app_library.cpp.)
TEST_CASE("jobs: a batch that fails as a whole shows its error and counts no chart") {
    RecordStore store(":memory:");
    std::atomic<int> started{0};
    std::atomic<int> released{2};  // no chart is held: each may run at once
    BatchJob job(plan_of(corpus_rows(2)), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/0);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

    // What the batch's pool (app/work_pool.h) throws for a pool of no workers.
    std::optional<std::invalid_argument> refused;
    try {
        hydra::app::run_work_pool<int>(
            2, 0, nullptr, [](size_t) { return 0; }, [](int&&) {});
    } catch (const std::invalid_argument& e) {
        refused.emplace(e);
    }
    REQUIRE(refused.has_value());

    const BatchJob::Snapshot s = job.snapshot();
    CHECK(started.load() == 0);
    CHECK(s.failed == 0);
    CHECK(s.completed == 0);
    CHECK(s.failures.empty());
    // The run's own error, in plain words with its details, not a chart's.
    CHECK(s.run_error == hydra::app::plain_error(*refused));
    CHECK(s.run_error_detail == hydra::app::plain_error_detail(*refused));
}

TEST_CASE("jobs: the snapshot's counts come from the batch in one piece") {
    // Finding 142: one progress callback writes all five numbers, so no frame
    // can read a failure counted before its chart is. Every snapshot read
    // while the batch runs must add up. Two corpus charts analyze for real,
    // and a third's song file was deleted after the scan, so it fails.
    std::vector<ChartLibraryEntry> charts = corpus_rows(3);
    charts[1] = missing_copy(charts[1], empty_dir("jobs_counts_in_one_piece"));
    const size_t failing = 1;
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store(":memory:");
    BatchJob job(plan_of(charts), test_run(), store);
    job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
    job.start();

    // Each chart is let go in turn; every frame read on the way is checked.
    int frames = 0;
    int torn = 0;
    const auto read_frame = [&] {
        const BatchJob::Snapshot f = job.snapshot();
        ++frames;
        if (f.analyzed + f.failed + f.skipped != f.completed) ++torn;
        return f;
    };
    for (int i = 1; i <= static_cast<int>(charts.size()); ++i) {
        released = i;
        wait_for_batch_step(job, [&] { return read_frame().completed == i; },
                            "chart " + std::to_string(i) + " to be counted");
    }
    wait_until([&] { return read_frame().finished; }, "the batch to finish");
    CHECK(frames > 0);
    CHECK(torn == 0);

    const BatchJob::Snapshot s = job.snapshot();
    CHECK(s.total == static_cast<int>(charts.size()));
    CHECK(s.completed == s.total);
    CHECK(s.failed == static_cast<int>(failing));
    CHECK(s.skipped == 0);  // none had a result before the run
    CHECK(s.analyzed == s.total - s.failed);
    CHECK(job.batch_run().profile == test_run().profile);
}

// ---- The batch job's tray: the store's answer for each chart it saved -----

namespace {

using hydra::store::RecordLookup;
using SavedRows = BatchJob::SavedRows;

// A chart that returns `real` at once.
hydra::app::ChartAnalyzer always(const AnalysisResult& real) {
    return [&real](const std::string&, const AnalysisSettings&,
                   const std::function<void(float)>&) -> AnalysisResult { return real; };
}

// Whether two answers say the same thing, field by field (RecordLookup has
// no operator==).
bool same_answer(const RecordLookup& a, const RecordLookup& b) {
    return a.status == b.status && a.stale_build == b.stale_build &&
           a.stale_rules == b.stale_rules && a.record == b.record &&
           a.failure_kind == b.failure_kind && a.failure_text == b.failure_text;
}

// How many of the tray's answers are for `md5`.
size_t answers_for(const SavedRows& rows, const std::string& md5) {
    return static_cast<size_t>(std::count_if(rows.answers.begin(), rows.answers.end(),
                                             [&md5](const auto& a) { return a.first == md5; }));
}

// Checks each tray answer equals the store's own answer, asked now.
void check_answers_match_store(const SavedRows& rows, RecordStore& store, const BatchRun& run) {
    for (const auto& answer : rows.answers) {
        INFO("chart " << answer.first);
        CHECK(same_answer(answer.second, store.lookup_one(answer.first, run.profile)));
    }
}

// corpus_rows(n) with chart 0 copied into the empty folder `dir` too, as the
// scan reads that copy: one chart, two library rows.
std::vector<ChartLibraryEntry> corpus_rows_with_a_copy(size_t n, const std::string& dir) {
    std::vector<ChartLibraryEntry> rows = corpus_rows(n);
    rows.push_back(copied_row(rows[0], dir));
    return rows;
}

// A started one-worker batch job over the first two corpus charts, with an
// in-memory store and test_run(). Each chart is held at its start until
// `released` lets it go (real_analysis_in_turn). The members are built in the
// order they are declared, so the job is built last and torn down first.
struct HeldTwoChartBatch {
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    RecordStore store{":memory:"};
    const BatchRun run = test_run();
    const std::vector<ChartLibraryEntry> charts = corpus_rows(2);
    BatchJob job{plan_of(charts), run, store};

    HeldTwoChartBatch() {
        job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
        job.start();
    }
};

}  // namespace

TEST_CASE("jobs: answered_count reaches the number of charts saved, and counts a chart in two "
          "folders once") {
    const std::string dir = empty_dir("jobs_answered_copy");
    RecordStore store(":memory:");
    // Two charts, three library rows: chart 0 sits in two folders.
    BatchJob job(plan_of(corpus_rows_with_a_copy(2, dir)), test_run(), store);
    job.set_analyzer_for_test(real_analyzer(), /*workers=*/1);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

    CHECK(job.snapshot().completed == 3);
    CHECK(job.answered_count() == 2);
    // Taking the tray does not lower the count.
    (void)job.take_saved_rows();
    CHECK(job.answered_count() == 2);
    testtemp::remove_dir(dir);
}

TEST_CASE("jobs: a batch job's tray holds the store's answer for each chart it saved") {
    RecordStore store(":memory:");
    const BatchRun run = test_run();
    const std::vector<ChartLibraryEntry> charts = corpus_rows(2);
    BatchJob job(plan_of(charts), run, store);
    job.set_analyzer_for_test(real_analyzer(), /*workers=*/1);
    job.start();
    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

    const SavedRows rows = job.take_saved_rows();
    CHECK(rows.problem.empty());
    REQUIRE(rows.answers.size() == 2);
    CHECK(answers_for(rows, charts[0].md5) == 1);
    CHECK(answers_for(rows, charts[1].md5) == 1);
    check_answers_match_store(rows, store, run);
}

TEST_CASE("jobs: a chart in two folders is asked about once") {
    const std::string dir = empty_dir("jobs_asked_once_copy");
    RecordStore store(":memory:");
    const BatchRun run = test_run();
    const std::vector<ChartLibraryEntry> charts = corpus_rows_with_a_copy(2, dir);
    const hydra::app::BatchPlan plan = plan_of(charts);
    REQUIRE(plan.todo.size() == 2);
    REQUIRE(plan.todo[0].md5 == charts[0].md5);
    REQUIRE(plan.rows[0] == 2);
    std::atomic<int> lookups{0};  // before the job, so it outlives the job's thread
    {
        BatchJob job(plan, run, store);
        job.set_analyzer_for_test(real_analyzer(), /*workers=*/1);
        job.set_lookup_hold_for_test([&lookups] { ++lookups; });
        job.start();
        wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

        const SavedRows rows = job.take_saved_rows();
        CHECK(answers_for(rows, charts[0].md5) == 1);
        CHECK(answers_for(rows, charts[1].md5) == 1);
        CHECK(rows.answers.size() == 2);
        CHECK(lookups.load() == 2);  // one per chart, none for the second folder's row
        check_answers_match_store(rows, store, run);
    }
    testtemp::remove_dir(dir);
}

// O7 and O13: a failure the store keeps reaches its row as the store's answer
// (the test names no status); a failure it does not keep, and a failure whose
// save failed, leave nothing for their rows.
TEST_CASE("jobs: a failed chart whose failure was stored reaches the tray as the store's answer") {
    // A Hard run: the corpus has charts with no Hard part.
    hydra::app::Settings settings;
    settings.view_difficulty = "Hard";
    const BatchRun run = settings.batch_run();
    // Chart 0 has no Hard notes: a failure the store keeps. Chart 1 is a copy
    // of another corpus chart whose file is then deleted, as a chart moved
    // away since the scan: a song file that cannot be opened is a failure the
    // store does not keep. Chart 2 has Hard notes, and its save is refused.
    const ChartLibraryEntry no_notes =
        corpus_row(corpus::first_chart_without_notes(hydra::Difficulty::Hard));
    const ChartLibraryEntry refused =
        corpus_row(corpus::first_chart_with_notes(hydra::Difficulty::Hard));
    const std::string dir = empty_dir("jobs_tray_failures_gone");
    ChartLibraryEntry gone;
    for (const ChartLibraryEntry& e : corpus_scan()) {
        if (e.md5 == no_notes.md5 || e.md5 == refused.md5) continue;
        gone = copied_row(e, dir);
        break;
    }
    REQUIRE_FALSE(gone.md5.empty());
    std::filesystem::remove(hydra::os_path(gone.notespath));

    testtemp::ScopedFile db("jobs_tray_failures", ".db");
    { RecordStore create(db.path); }
    // A trigger refuses every record of chart 2, so its saves throw, the way
    // "run_batch counts a failed save as a failed chart and goes on" refuses
    // a save.
    const std::string trigger = "CREATE TRIGGER refuse_chart BEFORE INSERT ON records"
                                " WHEN NEW.song_id = '" +
                                refused.md5 + "' BEGIN SELECT RAISE(ABORT, 'boom'); END;";
    hydra::test::exec_on_file(db.path, trigger.c_str());
    {
        RecordStore store(db.path);
        BatchJob job(plan_of({no_notes, gone, refused}), run, store);
        job.set_analyzer_for_test(real_analyzer(), /*workers=*/1);
        job.start();
        wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
        CHECK(job.snapshot().failed == 3);

        const SavedRows rows = job.take_saved_rows();
        CHECK(rows.problem.empty());
        REQUIRE(rows.answers.size() == 1);
        CHECK(rows.answers[0].first == no_notes.md5);
        check_answers_match_store(rows, store, run);
        CHECK(job.answered_count() == 1);
    }
    testtemp::remove_dir(dir);
}

TEST_CASE("jobs: the tray is empty after it is taken, and a stopped run's tray holds only what it "
          "saved") {
    HeldTwoChartBatch held;

    held.released = 1;
    wait_for_batch_step(held.job, [&] { return held.job.answered_count() == 1; },
                        "the first chart's answer");
    held.job.stop();
    held.released = 2;  // the second chart runs after the stop, so it is dropped
    wait_until([&] { return held.job.snapshot().finished; }, "the stopped batch to finish");

    const SavedRows rows = held.job.take_saved_rows();
    REQUIRE(rows.answers.size() == 1);
    CHECK(rows.answers[0].first == held.charts[0].md5);
    check_answers_match_store(rows, held.store, held.run);

    const SavedRows again = held.job.take_saved_rows();
    CHECK(again.answers.empty());
    CHECK(again.problem.empty());
    CHECK(held.job.answered_count() == 1);
}

TEST_CASE("jobs: a paused batch job's tray keeps its answers") {
    HeldTwoChartBatch held;
    wait_for_batch_step(held.job, [&] { return held.started.load() == 1; },
                        "the first chart to start");

    // Paused with the first chart in flight: it finishes and is stored. The
    // second is let go too, so only the pause can keep it from starting.
    held.job.pause();
    held.released = 2;
    wait_for_batch_step(held.job, [&] { return held.job.answered_count() == 1; },
                        "the first chart's answer");
    const SavedRows paused = held.job.take_saved_rows();
    // Read once the paused chart is answered and its tray taken: the
    // analyzer's own count says the second chart never reached it.
    CHECK(held.started.load() == 1);
    CHECK(held.job.answered_count() == 1);  // nothing added while paused
    CHECK(held.job.snapshot().paused);

    REQUIRE(paused.answers.size() == 1);
    CHECK(paused.answers[0].first == held.charts[0].md5);
    check_answers_match_store(paused, held.store, held.run);

    held.job.resume();
    wait_until([&] { return held.job.snapshot().finished; }, "the batch to finish");
    CHECK(held.started.load() == 2);
    const SavedRows rest = held.job.take_saved_rows();
    REQUIRE(rest.answers.size() == 1);
    CHECK(rest.answers[0].first == held.charts[1].md5);
}

// A lookup that fails shows the same sentence a failed store read shows, and
// its chart is asked about again with the next saved chart. The records
// table is moved aside between chart 0's save and its lookup (the seam), so
// the save lands and the lookup fails, then moved back before chart 1 runs.
TEST_CASE("jobs: a lookup that fails leaves its sentence in the tray and is tried again") {
    testtemp::ScopedFile db("jobs_tray_lookup_fails", ".db");
    std::atomic<int> started{0};
    std::atomic<int> released{0};
    std::atomic<int> lookups{0};
    const BatchRun run = test_run();
    const std::vector<ChartLibraryEntry> charts = corpus_rows(2);
    {
        RecordStore store(db.path);
        BatchJob job(plan_of(charts), run, store);
        job.set_analyzer_for_test(real_analysis_in_turn(started, released), /*workers=*/1);
        job.set_lookup_hold_for_test([&] {
            if (lookups++ == 0)
                hydra::test::exec_on_file(db.path, "ALTER TABLE records RENAME TO records_aside");
        });
        job.start();

        released = 1;
        SavedRows failed;
        wait_for_batch_step(
            job,
            [&] {
                SavedRows taken = job.take_saved_rows();
                if (taken.problem.empty() && taken.answers.empty()) return false;
                failed = std::move(taken);
                return true;
            },
            "the first chart's lookup to fail");
        CHECK(failed.answers.empty());
        CHECK(job.answered_count() == 0);

        // The same failure, read by the test through the store.
        std::string expected;
        try {
            (void)store.lookup_one(charts[0].md5, run.profile);
        } catch (const std::exception& e) {
            expected = hydra::app::plain_error(e);
        }
        REQUIRE_FALSE(expected.empty());
        CHECK(failed.problem == expected);

        hydra::test::exec_on_file(db.path, "ALTER TABLE records_aside RENAME TO records");
        released = 2;
        wait_until([&] { return job.snapshot().finished; }, "the batch to finish");

        const BatchJob::Snapshot s = job.snapshot();
        CHECK(s.run_error.empty());
        CHECK(s.failed == 0);
        CHECK(lookups.load() == 2);  // chart 1's lookup asked about chart 0 again
        const SavedRows rows = job.take_saved_rows();
        CHECK(rows.problem.empty());
        CHECK(rows.answers.size() == 2);
        CHECK(answers_for(rows, charts[0].md5) == 1);
        CHECK(answers_for(rows, charts[1].md5) == 1);
        check_answers_match_store(rows, store, run);
        CHECK(job.answered_count() == 2);
    }
}

// The batch thread is held at its lookup while a helper takes the store lock
// the way a save group does; then the lookup is let go, so it waits on that
// lock. A frame's snapshot and tray take still return at once.
TEST_CASE("jobs: a frame's snapshot does not wait while the batch job waits on the store") {
    const AnalysisResult real = corpus::first_analyzed_with_paths(AnalysisSettings{});
    RecordStore store(":memory:");
    const BatchRun run = test_run();
    std::atomic<bool> at_lookup{false};
    std::atomic<bool> go{false};
    std::atomic<bool> into_lookup{false};
    BatchJob job(plan_of(fake_charts(1)), run, store);
    job.set_analyzer_for_test(always(real), /*workers=*/1);
    job.set_lookup_hold_for_test([&] {
        at_lookup = true;
        wait_until([&go] { return go.load(); }, "the test to let the lookup go");
        into_lookup = true;  // the hold's last step: the store's lookup is next
    });
    job.start();
    wait_for_batch_step(job, [&] { return at_lookup.load(); }, "the batch to reach its lookup");

    {
        StoreLockHolder holder(store);
        go = true;
        // The batch thread has left the hold for the store's lookup, which
        // cannot finish while the holder has the store's lock.
        wait_for_batch_step(job, [&] { return into_lookup.load(); },
                            "the batch to go on into its lookup");
        const BatchJob::Snapshot s = job.snapshot();
        const SavedRows rows = job.take_saved_rows();
        const bool returned_while_held = !holder.let_go();
        holder.release();
        CHECK(returned_while_held);
        CHECK_FALSE(s.finished);
        CHECK(rows.answers.empty());  // the lookup had not got the lock
    }

    wait_until([&] { return job.snapshot().finished; }, "the batch to finish");
    const SavedRows rows = job.take_saved_rows();
    REQUIRE(rows.answers.size() == 1);
    CHECK(rows.answers[0].first == "fake0");
    check_answers_match_store(rows, store, run);
}

// D88: every exe runs on mimalloc, this one too. An exe whose link puts
// mimalloc.dll after another DLL keeps the Windows heap without a word. The
// Debug runtime is never redirected, so only a Release build can tell. A build
// without mimalloc (HYDRA_MIMALLOC in CMakeLists.txt) has nothing to check.
#if defined(NDEBUG) && defined(HYDRA_MIMALLOC)
TEST_CASE("allocator: malloc goes to mimalloc") {
    CHECK(hydra::app::malloc_redirected());
}
#endif

// The hand-back tests measure mimalloc's purge, so a build without mimalloc
// (HYDRA_MIMALLOC in CMakeLists.txt) leaves them out.
#ifdef HYDRA_MIMALLOC
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
#endif  // HYDRA_MIMALLOC

TEST_CASE("jobs: a report job keeps the profile of the run it was given") {
    RecordStore store(":memory:");
    BatchRun run = test_run();
    run.profile.lens = hydra::store::Lens::from(std::optional<int>(20), 1, 7);
    const hydra::ui::ReportJob job(store, run);
    CHECK(job.profile() == run.profile);
}

// D97: an empty report shows generate_report's own reason when it gives one,
// word for word. Results with no chart library name the missing
// library; a database with no results gets the app's own sentence.
TEST_CASE("jobs: an empty report shows generate_report's reason, or the app's own") {
    const BatchRun run = test_run();

    RecordStore no_library(":memory:");
    hydra::test::save_batch_record(no_library, "orphan", run.profile);
    hydra::ui::ReportJob orphaned(no_library, run);
    orphaned.start();
    wait_until([&] { return orphaned.finished(); }, "the report with no library to finish");
    CHECK_FALSE(orphaned.ok());
    CHECK(orphaned.message() == std::string(hydra::app::report::kNoChartLibrary));

    RecordStore empty(":memory:");
    hydra::ui::ReportJob nothing(empty, run);
    nothing.start();
    wait_until([&] { return nothing.finished(); }, "the report with no results to finish");
    CHECK_FALSE(nothing.ok());
    CHECK(nothing.message() ==
          hydra::app::plain_error(hydra::KindedError(hydra::ErrorKind::NoRecords, "x")));
}

// ---- the startup open's fill of old rows' hardest figure ----------------------

namespace {

// What the startup open's gate saw at one report.
struct OpenReport {
    hydra::ui::StoreOpenProgress::Step step;
    std::string label;
    bool upgrading;
    uint64_t done;
    uint64_t total;
};

// Every report the startup open's gate sees while it lives. Declare it before
// the job, so the gate is cleared after the job is joined.
struct OpenRecorder {
    std::vector<OpenReport> seen;
    ScopedHook gate{hydra::ui::set_store_open_gate_for_test,
                    [this](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>&) {
                        seen.push_back({p.step, p.label(), p.upgrading, p.rows_done, p.rows_total});
                    }};
};

// The steps `seen` went through, in order, each once however many reports it
// made in a row.
std::vector<hydra::ui::StoreOpenProgress::Step> steps_in_order(
    const std::vector<OpenReport>& seen) {
    std::vector<hydra::ui::StoreOpenProgress::Step> steps;
    for (const OpenReport& r : seen)
        if (steps.empty() || steps.back() != r.step) steps.push_back(r.step);
    return steps;
}

// Checks the counted step `step`: it reported, its first report had none of
// `total` done and its last had all of them, and every report counted out of
// `total`.
void check_counted_step(const std::vector<OpenReport>& seen,
                        hydra::ui::StoreOpenProgress::Step step, uint64_t total) {
    std::vector<OpenReport> reports;
    for (const OpenReport& r : seen)
        if (r.step == step) reports.push_back(r);
    INFO("step " << static_cast<int>(step));
    REQUIRE_FALSE(reports.empty());
    CHECK(reports.front().done == 0);
    CHECK(reports.back().done == total);
    for (const OpenReport& r : reports) CHECK(r.total == total);
}

// The open's own words, which every other step's label differs from.
std::string opening_label() {
    hydra::ui::StoreOpenProgress opening;
    opening.step = hydra::ui::StoreOpenProgress::Step::Opening;
    return opening.label();
}

// A library file at `db` listing the corpus chart Smash Mouth "I'm A
// Believer", with its analysis under `settings` saved as an older build left
// it: rank 1's hardest figure empty. Its best path carries a squeeze-in at
// exactly 0.0 ms, so the fill has that row to replay.
void write_old_build_library(const std::string& db, const hydra::app::Settings& settings) {
    const std::string chart = corpus::chart_named("Smash Mouth - I_m A Believer");
    REQUIRE_FALSE(chart.empty());
    RecordStore store(db);
    hydra::test::save_as_old_build(store, chart, corpus::scanned_entry(chart, "I'm A Believer"),
                                   settings, {1});
}

}  // namespace

TEST_CASE("jobs: the startup open reads saved paths with its own words, then the library") {
    using Step = hydra::ui::StoreOpenProgress::Step;
    const std::string dir = testtemp::temp_dir("open_fill_words");
    testtemp::remove_dir(dir);
    const std::string db = testtemp::temp_dir("open_fill_words") + "\\library.db";
    const hydra::app::Settings settings;
    write_old_build_library(db, settings);

    OpenRecorder recorder;
    std::vector<OpenReport>& seen = recorder.seen;
    {
        hydra::ui::StoreOpenJob job(db, hydra::core::RulesStamp::of(settings.rules), settings);
        job.start();
        job.wait();
        REQUIRE(job.ok());
        CHECK(job.library_problem().empty());
        const std::unique_ptr<RecordStore> store = job.take_store();
        const hydra::ui::LibraryModel library = job.take_library();
        // The library lists one chart, and the model holds that one row. The
        // one old-build row is that chart's, so the fill has one row too.
        REQUIRE(store->chart_library_count() == 1);
        REQUIRE(library.rows().size() == 1);
        const uint64_t charts = library.rows().size();
        const std::vector<Step> upgrade_steps = {Step::Opening, Step::ReadingSavedPaths,
                                                 Step::ReadingCharts, Step::ReadingResults};
        CHECK(steps_in_order(seen) == upgrade_steps);
        check_counted_step(seen, Step::ReadingSavedPaths, charts);
        check_counted_step(seen, Step::ReadingCharts, charts);
        check_counted_step(seen, Step::ReadingResults, charts);
        // An upgrade: every step after the open says so, and the fill's step
        // has its own words, not the open's.
        for (const OpenReport& r : seen) {
            if (r.step == Step::Opening) continue;
            CHECK(r.upgrading);
            CHECK(r.label != opening_label());
            if (r.step == Step::ReadingSavedPaths) {
                CHECK(r.label == hydra::ui::kReadingSavedPathsLabel);
            }
        }
        CHECK(store->hardest_figure_filled(settings.profile()));
    }

    // The fill has run for this profile, so the next open goes straight to
    // the library, and nothing reads as an upgrade.
    seen.clear();
    {
        hydra::ui::StoreOpenJob again(db, hydra::core::RulesStamp::of(settings.rules), settings);
        again.start();
        again.wait();
        REQUIRE(again.ok());
        const hydra::ui::LibraryModel library = again.take_library();
        const uint64_t charts = library.rows().size();
        const std::vector<Step> library_steps = {Step::Opening, Step::ReadingCharts,
                                                 Step::ReadingResults};
        CHECK(steps_in_order(seen) == library_steps);
        check_counted_step(seen, Step::ReadingCharts, charts);
        check_counted_step(seen, Step::ReadingResults, charts);
        for (const OpenReport& r : seen) {
            if (r.step == Step::Opening) {
                CHECK(r.label == opening_label());
                continue;
            }
            CHECK_FALSE(r.upgrading);
            CHECK(r.label != opening_label());
        }
    }
    testtemp::remove_dir(dir);
}

TEST_CASE("jobs: a fill that fails still opens the library and leaves its sentence for the "
          "status line") {
    using Step = hydra::ui::StoreOpenProgress::Step;
    const std::string dir = testtemp::temp_dir("open_fill_fails");
    testtemp::remove_dir(dir);
    const std::string db = testtemp::temp_dir("open_fill_fails") + "\\library.db";
    const hydra::app::Settings settings;
    write_old_build_library(db, settings);
    // The file refuses the fill's write, as a full disk would.
    hydra::test::exec_on_file(db,
                              "CREATE TRIGGER refuse_fill BEFORE UPDATE ON record_paths"
                              " BEGIN SELECT RAISE(ABORT, 'refused'); END;");

    OpenRecorder recorder;
    const std::vector<OpenReport>& seen = recorder.seen;
    {
        hydra::ui::StoreOpenJob job(db, hydra::core::RulesStamp::of(settings.rules), settings);
        job.start();
        job.wait();
        REQUIRE(job.ok());
        // The library was still read to its last step, and the marker stays
        // unset so the next open tries again.
        REQUIRE_FALSE(seen.empty());
        CHECK(seen.back().step == Step::ReadingResults);
        const std::unique_ptr<RecordStore> store = job.take_store();
        CHECK_FALSE(store->hardest_figure_filled(settings.profile()));
        // The sentence is the one the store's own refused write reads as: the
        // test makes the fill's write itself, on the row the fill replays.
        const std::vector<hydra::store::MissingHardestRow> missing =
            store->rows_missing_hardest(settings.profile());
        REQUIRE_FALSE(missing.empty());
        std::string refused;
        try {
            store->set_hardest(missing[0].song_id, missing[0].profile_id,
                               {hydra::store::HardestFigure{missing[0].rank, 0.0}});
        } catch (const std::exception& e) {
            refused = hydra::app::plain_error(e);
        }
        REQUIRE_FALSE(refused.empty());
        CHECK(job.library_problem() == refused);
    }
    testtemp::remove_dir(dir);
}

namespace {

// Runs a startup open of the library file at `db` whose gate cancels it at
// the first report `stop_here` picks, checks the open ended as a cancel with
// no message, and returns every report the gate saw.
std::vector<OpenReport> open_stopped_at(
    const std::string& db, const hydra::app::Settings& settings,
    const std::function<bool(const hydra::ui::StoreOpenProgress&)>& stop_here) {
    std::vector<OpenReport> seen;
    hydra::ui::StoreOpenJob* held = nullptr;
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>&) {
            seen.push_back({p.step, p.label(), p.upgrading, p.rows_done, p.rows_total});
            if (held && stop_here(p)) held->cancel();
        });
    {
        hydra::ui::StoreOpenJob job(db, hydra::core::RulesStamp::of(settings.rules), settings);
        held = &job;
        job.start();
        job.wait();
        held = nullptr;
        CHECK(job.outcome() == hydra::ui::JobOutcome::Cancelled);
        CHECK(job.message().empty());
        CHECK(job.failure() == nullptr);
    }
    return seen;
}

}  // namespace

TEST_CASE("jobs: a stop while the startup open reads saved paths ends it as a cancel") {
    using Step = hydra::ui::StoreOpenProgress::Step;
    using hydra::ui::StoreOpenProgress;
    const std::string dir = testtemp::temp_dir("open_fill_stop");
    testtemp::remove_dir(dir);
    const std::string db = testtemp::temp_dir("open_fill_stop") + "\\library.db";
    const hydra::app::Settings settings;
    write_old_build_library(db, settings);

    open_stopped_at(db, settings,
                    [](const StoreOpenProgress& p) { return p.step == Step::ReadingSavedPaths; });
    // The stop came before the chart was read: the marker is unset and the
    // row still empty, so the next open fills it.
    {
        RecordStore store(db);
        CHECK_FALSE(store.hardest_figure_filled(settings.profile()));
        CHECK(store.rows_missing_hardest(settings.profile()).size() == 1);
    }

    // The library read's two steps stop the same way, on the library a scan of
    // the corpus saves.
    const std::string read_db = testtemp::temp_dir("open_fill_stop") + "\\read.db";
    uint64_t library_size = 0;
    {
        RecordStore store(read_db);
        REQUIRE_FALSE(hydra::app::save_scan_as_library(store, corpus_scan()).has_value());
        library_size = static_cast<uint64_t>(store.chart_library_count());
    }
    // The stop at the second row needs a third row after it.
    REQUIRE(library_size > 2);
    {
        // At the chart list's first report, before any row is read.
        const std::vector<OpenReport> seen =
            open_stopped_at(read_db, settings, [](const StoreOpenProgress& p) {
                return p.step == Step::ReadingCharts && p.rows_done == 0;
            });
        REQUIRE_FALSE(seen.empty());
        CHECK(seen.back().step == Step::ReadingCharts);
        CHECK(seen.back().done == 0);
    }
    {
        // Inside the chart list, at its second row: the store stops its own
        // read, and the job still ends as a cancel, not as a library problem
        // for the status line.
        const std::vector<OpenReport> seen =
            open_stopped_at(read_db, settings, [](const StoreOpenProgress& p) {
                return p.step == Step::ReadingCharts && p.rows_done == 2;
            });
        REQUIRE_FALSE(seen.empty());
        CHECK(seen.back().step == Step::ReadingCharts);
        CHECK(seen.back().done == 2);
        CHECK(seen.back().total == library_size);
    }
    {
        // At the results' first report.
        const std::vector<OpenReport> seen =
            open_stopped_at(read_db, settings, [](const StoreOpenProgress& p) {
                return p.step == Step::ReadingResults && p.rows_done == 0;
            });
        REQUIRE_FALSE(seen.empty());
        CHECK(seen.back().step == Step::ReadingResults);
        CHECK(seen.back().done == 0);
    }
    testtemp::remove_dir(dir);
}

namespace {

// Checks that row `got` is row `want` and shows the same saved result.
void check_same_result(const hydra::ui::LibraryRow& got, const hydra::ui::LibraryRow& want) {
    CAPTURE(want.entry.md5);
    CAPTURE(want.entry.notespath);
    CHECK(got.entry.md5 == want.entry.md5);
    CHECK(got.entry.notespath == want.entry.notespath);
    CHECK(got.status == want.status);
    CHECK(got.stale_build == want.stale_build);
    CHECK(got.stale_rules == want.stale_rules);
    CHECK(got.bestpath == want.bestpath);
    CHECK(got.failure_kind == want.failure_kind);
    CHECK(got.failure_text == want.failure_text);
    CHECK(got.best_label() == want.best_label());
}

// Checks that `got` has the rows of `want`, in the same order, each showing
// the same saved result.
void check_same_results(const hydra::ui::LibraryModel& got, const hydra::ui::LibraryModel& want) {
    REQUIRE(got.rows().size() == want.rows().size());
    for (size_t i = 0; i < want.rows().size(); ++i) check_same_result(got.rows()[i], want.rows()[i]);
}

// The row of `library` for the chart file at `notespath`.
const hydra::ui::LibraryRow& row_at_path(const hydra::ui::LibraryModel& library,
                                         const std::string& notespath) {
    for (const hydra::ui::LibraryRow& r : library.rows())
        if (r.entry.notespath == notespath) return r;
    throw std::runtime_error("the library has no row for " + notespath);
}

// Reads the store's charts, then their results in slices of `slice_charts`,
// into `sliced`. Returns the rows answered after each slice. `after_slice`, if
// given, runs after each slice's report with the rows answered so far.
std::vector<uint64_t> read_in_slices(RecordStore& store, const hydra::app::Settings& settings,
                                     hydra::ui::LibraryModel& sliced, size_t slice_charts,
                                     const std::function<void(uint64_t)>& after_slice = {}) {
    sliced.set_charts(store.list_chart_library(0, -1));
    std::vector<uint64_t> answered;
    hydra::ui::read_library_summaries(
        store, settings, sliced,
        [&](uint64_t rows_answered) {
            answered.push_back(rows_answered);
            if (after_slice) after_slice(rows_answered);
            return true;
        },
        slice_charts);
    return answered;
}

// The rows answered after each slice when `rows` rows are read in slices of
// `slice_charts`: one more slice each time, the last one ending on `rows`.
std::vector<uint64_t> slice_ends(size_t rows, size_t slice_charts) {
    std::vector<uint64_t> ends;
    for (size_t to = slice_charts; to < rows; to += slice_charts) ends.push_back(to);
    ends.push_back(rows);
    return ends;
}

}  // namespace

TEST_CASE("jobs: results read in slices match one read") {
    const hydra::app::Settings settings;
    const hydra::store::Profile profile = settings.profile();
    RecordStore store(":memory:");
    // Five corpus charts the real analyzer reads to paths, each with its real
    // analysis (charts_with_paths) and the row a scan of the corpus writes.
    const std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(settings.to_analysis_settings(), 5);
    REQUIRE(charts.size() == 5);
    std::vector<ChartLibraryEntry> scanned;
    for (const corpus::ChartWithPaths& c : charts) scanned.push_back(corpus_row(c.chart));
    // The chart with the last hash is asked late. The other four are, in
    // their order, Ready, another build's, other rules' and both.
    size_t late_at = 0;
    for (size_t i = 1; i < scanned.size(); ++i)
        if (scanned[i].md5 > scanned[late_at].md5) late_at = i;
    std::vector<size_t> saved_at;
    for (size_t i = 0; i < scanned.size(); ++i) {
        if (i == late_at) continue;
        REQUIRE(scanned[i].md5 < scanned[late_at].md5);
        saved_at.push_back(i);
    }
    // The late chart's row has an empty title. The library reads back by
    // title, then in the order the rows were written (LibraryOrder::ByName),
    // and no title sorts before an empty one, so written first it reads first.
    ChartLibraryEntry late = scanned[late_at];
    late.title.clear();
    const ChartLibraryEntry& ready = scanned[saved_at[0]];

    // The library a scan of the corpus writes: the late chart, the Ready
    // chart, every other scanned row, then a copy of the Ready chart in a
    // second folder, so one chart has two rows. Both rows have one hash, so
    // a slice of one chart reads them in different slices.
    const ChartLibraryEntry copy = copied_row(ready, empty_dir("slices_copy"));
    std::vector<ChartLibraryEntry> rows{late, ready};
    for (const ChartLibraryEntry& e : corpus_scan())
        if (e.notespath != ready.notespath && e.notespath != late.notespath) rows.push_back(e);
    rows.push_back(copy);
    store.rebuild_chart_library(rows);

    // Each saved chart's own real analysis: one saved by this build, one by
    // another build, one under other rules and one with both.
    const hydra::HydraRecord& build_record = charts[saved_at[1]].result.record;
    const hydra::HydraRecord& rules_record = charts[saved_at[2]].result.record;
    const hydra::HydraRecord& both_record = charts[saved_at[3]].result.record;
    store.save(hydra::store::prepare_record(ready.md5, profile, charts[saved_at[0]].result.record));
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(scanned[saved_at[1]].md5, profile, build_record));
    store.save(hydra::store::prepare_record(scanned[saved_at[2]].md5, profile,
                                            hydra::test::other_rules_record(rules_record)));
    hydra::test::save_old_build_record(
        store, hydra::store::prepare_record(scanned[saved_at[3]].md5, profile,
                                            hydra::test::other_rules_record(both_record)));

    // The reference: one store lookup for every row, in row order, made here
    // without read_library_summaries.
    hydra::ui::LibraryModel whole;
    whole.set_charts(store.list_chart_library(0, -1));
    whole.set_summaries(store.lookup(whole.hashes(), profile));
    const size_t n = whole.rows().size();
    REQUIRE(n == rows.size());
    REQUIRE(whole.rows().front().entry.md5 == late.md5);
    REQUIRE(whole.rows().front().entry.notespath == late.notespath);
    REQUIRE(row_at_path(whole, ready.notespath).entry.md5 == ready.md5);
    REQUIRE(row_at_path(whole, copy.notespath).entry.md5 == ready.md5);
    // The one read shows each saved result as the store files it.
    const auto row_of = [&whole](const std::string& md5) -> const hydra::ui::LibraryRow& {
        for (const hydra::ui::LibraryRow& r : whole.rows())
            if (r.entry.md5 == md5) return r;
        throw std::runtime_error("the one read has no row for " + md5);
    };
    REQUIRE(row_at_path(whole, ready.notespath).status == hydra::store::RecordStatus::Ready);
    REQUIRE(row_at_path(whole, copy.notespath).status == hydra::store::RecordStatus::Ready);
    REQUIRE(row_of(scanned[saved_at[1]].md5).status == hydra::store::RecordStatus::Stale);
    REQUIRE(row_of(scanned[saved_at[1]].md5).stale_build);
    REQUIRE(row_of(scanned[saved_at[2]].md5).status == hydra::store::RecordStatus::Stale);
    REQUIRE(row_of(scanned[saved_at[2]].md5).stale_rules);
    REQUIRE(row_of(scanned[saved_at[3]].md5).status == hydra::store::RecordStatus::Stale);
    REQUIRE(row_of(scanned[saved_at[3]].md5).stale_build);
    REQUIRE(row_of(scanned[saved_at[3]].md5).stale_rules);

    SUBCASE("one chart per slice") {
        hydra::ui::LibraryModel sliced;
        const std::vector<uint64_t> answered = read_in_slices(store, settings, sliced, 1);
        CHECK(answered == slice_ends(n, 1));
        check_same_results(sliced, whole);
        // The chart in two folders has its result in both rows.
        CHECK(row_at_path(sliced, ready.notespath).status == hydra::store::RecordStatus::Ready);
        CHECK(row_at_path(sliced, copy.notespath).status == hydra::store::RecordStatus::Ready);
    }
    SUBCASE("two charts per slice") {
        hydra::ui::LibraryModel sliced;
        const std::vector<uint64_t> answered = read_in_slices(store, settings, sliced, 2);
        // One report per slice, two rows more each time, ending on every row.
        CHECK(answered == slice_ends(n, 2));
        check_same_results(sliced, whole);
        CHECK(row_at_path(sliced, ready.notespath).status == hydra::store::RecordStatus::Ready);
        CHECK(row_at_path(sliced, copy.notespath).status == hydra::store::RecordStatus::Ready);
    }
    SUBCASE("the store is asked in hash order") {
        // The late chart gets its result after the first slice of two. Its
        // row is first, so asked in row order it was in the first slice and
        // would read Not analyzed. Asked in hash order it has at least two
        // smaller hashes before it (checked here), so it is not in the first
        // slice and its row reads that result.
        size_t smaller = 0;
        for (const ChartLibraryEntry& e : rows)
            if (e.md5 < late.md5) ++smaller;
        REQUIRE(smaller >= 2);
        REQUIRE(whole.rows().front().status == hydra::store::RecordStatus::NotAnalyzed);
        const hydra::HydraRecord& late_record = charts[late_at].result.record;
        hydra::ui::LibraryModel sliced;
        const std::vector<uint64_t> answered =
            read_in_slices(store, settings, sliced, 2, [&](uint64_t rows_answered) {
                if (rows_answered == 2)
                    store.save(hydra::store::prepare_record(late.md5, profile, late_record));
            });
        CHECK(answered == slice_ends(n, 2));
        REQUIRE(sliced.rows().front().entry.md5 == late.md5);
        CHECK(sliced.rows().front().status == hydra::store::RecordStatus::Ready);
    }
    SUBCASE("the real slice size") {
        hydra::ui::LibraryModel sliced;
        const std::vector<uint64_t> answered =
            read_in_slices(store, settings, sliced, hydra::ui::kResultsSliceCharts);
        CHECK(answered == slice_ends(n, hydra::ui::kResultsSliceCharts));
        check_same_results(sliced, whole);
    }
    SUBCASE("no callback, as the reload after a scan reads it") {
        // Slices of one, so the two rows of the chart in two folders are read
        // in different slices here too.
        hydra::ui::LibraryModel reread;
        reread.set_charts(store.list_chart_library(0, -1));
        hydra::ui::read_library_summaries(store, settings, reread, {}, 1);
        check_same_results(reread, whole);
    }
}

// ---- timing the startup library read, part by part ----------------------------

namespace {

// One run's milliseconds for each part of the startup library read.
struct StartupReadTimes {
    double open = 0;
    double list = 0;
    double set_charts = 0;
    double lookup = 0;
    double set_summaries = 0;
    double sort = 0;
    double total = 0;
};

double ms_between(std::chrono::steady_clock::time_point from,
                  std::chrono::steady_clock::time_point to) {
    return std::chrono::duration<double, std::milli>(to - from).count();
}

// Opens the library file at `db` and reads it the way read_library does, one
// call at a time so each can be timed. The calls and their order are
// read_library's and read_library_summaries' (ui/library_jobs.cpp); this
// holds no step of its own.
StartupReadTimes time_startup_read(const std::string& db, const hydra::app::Settings& settings) {
    using Clock = std::chrono::steady_clock;
    const Clock::time_point start = Clock::now();
    RecordStore store(db, hydra::core::RulesStamp::of(settings.rules));
    const Clock::time_point opened = Clock::now();
    std::vector<ChartLibraryEntry> charts = store.list_chart_library(0, -1);  // -1 = no limit
    const Clock::time_point listed = Clock::now();
    const size_t rows_read = charts.size();
    hydra::ui::LibraryModel library;
    library.set_charts(std::move(charts));
    const Clock::time_point charts_set = Clock::now();
    const std::vector<hydra::store::RecordLookup> lookups =
        store.lookup(library.hashes(), settings.profile());
    const Clock::time_point looked_up = Clock::now();
    library.set_summaries(lookups);
    const Clock::time_point summaries_set = Clock::now();
    library.sort_if_out_of_date();
    const Clock::time_point sorted = Clock::now();

    // Every row of the file was read; counted after the clock stops.
    CHECK(static_cast<int64_t>(rows_read) == store.chart_library_count());

    StartupReadTimes t;
    t.open = ms_between(start, opened);
    t.list = ms_between(opened, listed);
    t.set_charts = ms_between(listed, charts_set);
    t.lookup = ms_between(charts_set, looked_up);
    t.set_summaries = ms_between(looked_up, summaries_set);
    t.sort = ms_between(summaries_set, sorted);
    t.total = ms_between(start, sorted);
    return t;
}

void print_startup_read(const std::string& run, const StartupReadTimes& t) {
    MESSAGE(run << " open " << t.open << " ms");
    MESSAGE(run << " list " << t.list << " ms");
    MESSAGE(run << " set_charts " << t.set_charts << " ms");
    MESSAGE(run << " lookup " << t.lookup << " ms");
    MESSAGE(run << " set_summaries " << t.set_summaries << " ms");
    MESSAGE(run << " sort " << t.sort << " ms");
    MESSAGE(run << " total " << t.total << " ms");
}

}  // namespace

// A measurement, not a check of behaviour: D336 records what it measured on
// the user's library. It runs only when HYDRA_TIMING_DB names a library file, and it
// opens only that file, so point it at a copy of a library, never the live
// one. The first run is the nearest to a cold read; the fastest is the warm
// floor.
TEST_CASE("timing: the startup library read by part") {
    const std::optional<std::string> db = read_env("HYDRA_TIMING_DB");
    if (!db || db->empty()) return;
    REQUIRE(std::filesystem::is_regular_file(std::filesystem::u8path(*db)));

    const hydra::app::Settings settings;
    constexpr int kRuns = 5;  // the plan's run count
    std::vector<StartupReadTimes> runs;
    for (int i = 0; i < kRuns; ++i) runs.push_back(time_startup_read(*db, settings));

    size_t fastest = 0;
    for (size_t i = 1; i < runs.size(); ++i)
        if (runs[i].total < runs[fastest].total) fastest = i;
    print_startup_read("run 1:", runs[0]);
    print_startup_read("fastest (run " + std::to_string(fastest + 1) + "):", runs[fastest]);
}

// A cancel pressed before the fetch goes out. The real transport (the
// fetcher seam off) checks the cancel before it connects, so nothing leaves
// the machine, and the job ends not ok with no message.
TEST_CASE("jobs: a cancelled leaderboard fetch is not an error") {
    hydra::net::set_fetcher({});  // the real WinHTTP transport
    RecordStore store(":memory:");
    hydra::ui::DmReportJob job(store, "1", "someone", hydra::store::Profile{"jobs-test"});
    job.cancel();
    job.start();
    wait_until([&] { return job.finished(); }, "the cancelled fetch to finish");
    CHECK_FALSE(job.ok());
    CHECK(job.message().empty());
    // The transport's own cancel, kept as the details.
    CHECK(job.error() == hydra::app::plain_error_detail(hydra::Cancelled()));
}

TEST_CASE("jobs: a failed leaderboard fetch says what to do") {
    // A real failure to reach a server: the real transport asked for a
    // loopback port nothing listens on (port 1, tcpmux, which Windows does
    // not serve), so the connection is refused.
    hydra::net::set_fetcher({});  // the real WinHTTP transport
    std::optional<hydra::KindedError> unreachable;
    try {
        (void)hydra::net::fetch_users("http://127.0.0.1:1");
    } catch (const hydra::KindedError& e) {
        unreachable.emplace(e);
    }
    REQUIRE(unreachable.has_value());
    REQUIRE(unreachable->kind() == hydra::ErrorKind::NetUnreachable);
    REQUIRE(unreachable->service() == hydra::NetService::Leaderboard);

    // The job's fetch goes to dmleaderboards itself, which a test may not
    // call, so the seam hands the job that same real failure.
    const hydra::KindedError failure = *unreachable;
    const ScopedHook fetcher(
        hydra::net::set_fetcher,
        [failure](const std::string&, const std::atomic<bool>*) -> std::string { throw failure; });
    hydra::ui::DmFetchUsersJob job;
    job.start();
    wait_until([&] { return job.finished(); }, "the failed fetch to finish");
    CHECK_FALSE(job.ok());
    // The plain sentence for a server that cannot be reached, and the raw
    // text as the details.
    CHECK(job.message() == hydra::app::plain_error(failure));
    CHECK(job.message() != job.error());
    CHECK(job.error() == hydra::app::plain_error_detail(failure));
}

// Finding 212: run_guarded and ViewJob::start's thread-start catch record a
// failure through one fail(). This job exposes it so the test can call it.
TEST_CASE("jobs: a failed job records the raw text, the plain message, not ok and finished") {
    struct FailingJob : hydra::ui::ResultJobBase {
        using ResultJobBase::fail;
    };
    // A real failure: the analyzer asked for a chart file that is not there.
    const std::string gone = empty_dir("jobs_failed_job") + "\\notes.chart";
    std::optional<hydra::KindedError> thrown;
    try {
        (void)hydra::app::analyze_chart_file(gone, AnalysisSettings{});
    } catch (const hydra::KindedError& e) {
        thrown.emplace(e);
    }
    REQUIRE(thrown.has_value());
    const hydra::KindedError& failure = *thrown;
    const std::string raw = failure.what();

    FailingJob job;
    job.fail(failure);
    // The raw text is kept for the details line, and the message is the
    // plain sentence for its kind, not the raw text.
    CHECK(job.error() == raw);
    CHECK(job.message() == hydra::app::plain_error(failure));
    CHECK(job.message() != job.error());
    CHECK_FALSE(job.ok());
    CHECK(job.finished());

    // A cancel is the user's own click: the raw text stays, the message is empty.
    FailingJob cancelled;
    cancelled.cancel();
    cancelled.fail(failure);
    CHECK(cancelled.error() == raw);
    CHECK(cancelled.message().empty());
    CHECK_FALSE(cancelled.ok());
    CHECK(cancelled.finished());
}
