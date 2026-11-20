// Tests for ui/app_state's library: the search, the rows a batch refreshes,
// scans, the batch confirm, the startup open (DBUP) and reads that fail.
// Split from test_app_click.cpp; the shared fixtures are in
// app_state_fixtures.h.

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/user_messages.h"  // kDatabaseReadSentence, old_library_set_aside_notice, library_scan_blocked_notice
#include "app_state_fixtures.h"
#include "core/error_kind.h"
#include "core/stars.h"  // kMaxStars
#include "corpus_util.h"  // first_chart_with_paths, charts_with_paths
#include "db_file_util.h"  // exec_on_file, write_junk_db
#include "display_fixtures.h"  // save_batch_record
#include "library_fixtures.h"  // library_entry
#include "parse/song.h"  // NoNotesError
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "scratch_settings.h"
#include "store/record_store.h"
#include "store/upgrade_files.h"  // set_aside_path, with_side_files
#include "store_lock_holder.h"
#include "ui/app_state.h"
#include "ui/generation.h"
#include "wait_util.h"

using hydra::app::kDatabaseReadSentence;
using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStatus;
using hydra::store::RecordStore;
using hydra::test::returns_while_held;
using hydra::ui::AppState;
using hydra::ui::GenerationWatcher;
using hydra::ui::ViewedSong;
using testwait::wait_until;

// ---- a library the app's own scan made ---------------------------------------

namespace {

// An AppState on the scratch store whose library is its own scan of the
// corpus (scratch_settings' chart folders), read in by a frame as the
// window's frame would read it.
std::unique_ptr<AppState> app_on_scanned_corpus(const ScratchPaths& paths) {
    auto app =
        std::make_unique<AppState>(scratch_settings(), std::make_unique<RecordStore>(paths.db));
    app->start_scan();
    REQUIRE(app->scan_job != nullptr);
    wait_until([&] { return app->scan_job->snapshot().finished; }, "the scan to finish");
    const hydra::ui::ScanProgress done = app->scan_job->snapshot();
    REQUIRE_FALSE(done.cancelled);
    REQUIRE(done.charts_found > 0);
    app->tick_library(0.0);
    REQUIRE(app->library_shown_count() == static_cast<size_t>(done.charts_found));
    return app;
}

// Files the engine's analysis of corpus chart `chart` under the app's
// settings, the way a song's own analysis stores it: under the hash the scan
// gives the chart's file. The analysis must have been made under the app's
// settings.
void save_engine_result(AppState& app, const corpus::ChartWithPaths& chart) {
    app.store->save(hydra::store::prepare_record(hydra::app::hash_chart_file(chart.chart),
                                                 app.settings.profile(), chart.result.record));
}

// The first corpus chart with paths under scratch_settings, analyzed once per
// run.
const corpus::ChartWithPaths& scratch_chart_with_paths() {
    static const corpus::ChartWithPaths chart =
        corpus::first_chart_with_paths(scratch_settings().to_analysis_settings());
    return chart;
}

// Writes the app's own scan of the corpus to `paths.db`, with the engine's
// result for one corpus chart stored. Returns how many rows the library holds.
size_t seed_scanned_corpus(const ScratchPaths& paths) {
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    REQUIRE(app->settings.profile() == scratch_settings().profile());
    save_engine_result(*app, scratch_chart_with_paths());
    return app->library_shown_count();
}

// The rows on screen, top to bottom, as chart files.
std::vector<std::string> shown_notespaths(const AppState& app) {
    std::vector<std::string> out;
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        out.push_back(app.library_row_at(i).entry.notespath);
    return out;
}

// Where chart file `notespath` sits in `order`; order.size() when it is not
// there.
size_t place_of(const std::vector<std::string>& order, const std::string& notespath) {
    for (size_t i = 0; i < order.size(); ++i)
        if (order[i] == notespath) return i;
    return order.size();
}

// Whether `title` holds only letters, digits and spaces, so a search for it
// in quotes is one phrase with no filter in it.
bool plain_title(const std::string& title) {
    if (title.empty()) return false;
    for (const char c : title)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != ' ') return false;
    return true;
}

}  // namespace

// ---- the search --------------------------------------------------------------

// The search runs in memory over every chart; a song's own title finds it.
TEST_CASE("set_search narrows the library and the match count") {
    ScratchPaths paths("appstate_search");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const size_t all = app->library_shown_count();
    size_t pick = all;
    for (size_t i = all; i-- > 0;)
        if (plain_title(app->library_row_at(i).title)) {
            pick = i;
            break;
        }
    REQUIRE_MESSAGE(pick < all, "no corpus chart has a title of plain words");
    const hydra::ui::LibraryRow wanted = app->library_row_at(pick);

    app->set_search("\"" + wanted.title + "\"");
    CHECK(app->library.searching());
    const size_t found = app->library_shown_count();
    CHECK(found < all);
    CHECK(app->library.counts().all == found);
    CHECK(app->library_matches().size() == found);
    CHECK(place_of(shown_notespaths(*app), wanted.entry.notespath) < found);
    app->set_search("");
    CHECK_FALSE(app->library.searching());
    CHECK(app->library_shown_count() == all);
}

// A stars: value one past the most stars a path can earn (kMaxStars) is a
// filter that does not parse, so the search narrows nothing. The toolbar's
// batch button names the whole library unless LibraryModel::narrowed says
// otherwise (D48, Q15).
TEST_CASE("set_search: a filter that does not parse leaves the query empty") {
    ScratchPaths paths("appstate_badfilter");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const size_t all = app->library_shown_count();
    app->set_search("stars:" + std::to_string(hydra::kMaxStars + 1));
    CHECK_FALSE(app->library.query().errors.empty());
    CHECK(app->library.query().empty());
    CHECK_FALSE(app->library.searching());
    CHECK_FALSE(app->library.narrowed());
    CHECK(app->library_shown_count() == all);
}

// ---- the rows a batch refreshes ---------------------------------------------

// After a batch the library rows refresh, and the open panel is not analyzed
// again: it keeps what its own click showed. The chart's file is gone before
// the click, so the click showed it missing.
TEST_CASE("a batch result for the open chart refreshes its row, not the panel") {
    ScratchPaths paths("appstate_batchpanel");
    const ChartLibraryEntry open = corpus_chart("batchpanel");
    // The engine's result for the chart, made before its file goes.
    const hydra::app::AnalysisResult result =
        hydra::app::analyze_chart_file(open.notespath, Settings{}.to_analysis_settings());
    REQUIRE_FALSE(result.record.paths.empty());
    std::filesystem::remove(std::filesystem::u8path(open.notespath));
    std::unique_ptr<AppState> app = app_with(paths, {open});
    app->select(open, hydra::ui::SelectionSource::Click);
    REQUIRE(app->viewed.state == ViewedSong::State::FileMissing);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The batch files that result; with the file gone, its analyzer hands
    // back the result the engine made from the file.
    start_redo_batch(*app, [result](const std::string&, const hydra::app::AnalysisSettings&,
                                    const std::function<void(float)>&) { return result; });
    wait_batch_finished(*app);
    REQUIRE(app->batch_job->snapshot().failed == 0);

    app->tick_library(0.0);  // the batch's last refresh
    app->tick(0.0);
    CHECK(row_of(*app, open.md5).status == RecordStatus::Ready);
    CHECK(row_of(*app, open.md5).summary.has_scored_best_path());
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
    const std::string label_before = row_of(*app, song.md5).best_label();

    // The engine's result for the same chart with one more note, as a batch
    // that read the chart after an edit would file it.
    const hydra::app::AnalysisResult edited = hydra::app::analyze_chart_file(
        write_corpus_chart("click_batchredo_edited", true), app->settings.to_analysis_settings());
    REQUIRE_FALSE(edited.record.paths.empty());

    GenerationWatcher records;
    records.changed(app->record_generation);

    start_redo_batch(*app, [edited](const std::string&, const hydra::app::AnalysisSettings&,
                                    const std::function<void(float)>&) { return edited; });
    wait_batch_finished(*app);
    REQUIRE(app->batch_job->snapshot().failed == 0);

    app->tick_library(0.0);  // the batch's last refresh
    app->tick(0.0);
    CHECK(row_of(*app, song.md5).status == RecordStatus::Ready);
    CHECK(row_of(*app, song.md5).best_label() != label_before);
    // The row reads what the store now holds, as a second app's read of it.
    AppState second(Settings{}, std::make_unique<RecordStore>(paths.db));
    second.refresh_library_row(song.md5);
    CHECK(row_of(*app, song.md5).best_label() == row_of(second, song.md5).best_label());
    CHECK(app->view_job == nullptr);
    REQUIRE(app->viewed.ready());
    CHECK(app->viewed.record->paths.size() == shown_paths);
    CHECK_FALSE(records.changed(app->record_generation));
}

// A result stored behind the view's back shows once its row is re-read, and
// only that row is asked about: a second chart's result, stored as well,
// stays off its row.
TEST_CASE("refresh_library_row picks up one chart's new result") {
    ScratchPaths paths("appstate_row");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(app->settings.to_analysis_settings(), 3);
    REQUIRE_FALSE(charts.empty());
    const corpus::ChartWithPaths& asked = charts.front();
    const std::string refreshed = hydra::app::hash_chart_file(asked.chart);
    const corpus::ChartWithPaths* other = nullptr;
    for (const corpus::ChartWithPaths& c : charts)
        if (hydra::app::hash_chart_file(c.chart) != refreshed) {
            other = &c;
            break;
        }
    REQUIRE_MESSAGE(other != nullptr, "the first corpus charts with paths are one chart");

    std::vector<RecordStatus> before;
    for (size_t i = 0; i < app->library_shown_count(); ++i)
        before.push_back(app->library_row_at(i).status);
    const size_t analyzed_before = app->library.counts().analyzed;

    save_engine_result(*app, asked);
    save_engine_result(*app, *other);
    REQUIRE(app->refresh_library_row(refreshed));

    REQUIRE(app->library_shown_count() == before.size());
    size_t copies = 0;
    for (size_t i = 0; i < before.size(); ++i) {
        const hydra::ui::LibraryRow& row = app->library_row_at(i);
        if (row.entry.md5 == refreshed) {
            ++copies;
            CHECK(before[i] == RecordStatus::NotAnalyzed);
            CHECK(row.status == RecordStatus::Ready);
        } else {
            CHECK(row.status == before[i]);
        }
    }
    REQUIRE(copies > 0);
    CHECK(app->library.counts().analyzed == analyzed_before + copies);
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
// them and has put the store's answer for each in its tray. A chart is counted
// before its answer is made (BatchJob::answered_count), so the count alone
// would let a tick run ahead of the tray.
// It fails at once when the batch has finished short of that count.
void let_through(AppState& app, ChartGate& gate, int charts) {
    gate.allowed.store(charts);
    testwait::wait_until_or_gone(
        [&] {
            return app.batch_job->snapshot().completed == charts &&
                   app.batch_job->answered_count() == charts;
        },
        [&] { return app.batch_job->snapshot().finished; }, "the batch to store its charts",
        "the batch finished before it stored and answered that many charts");
}

// The engine's own analysis of each chart, let through `gate` one at a time
// as ChartGate says. The batch stores what the engine makes of the chart.
hydra::app::ChartAnalyzer gated_engine(std::shared_ptr<ChartGate> gate) {
    return [gate](const std::string& path, const hydra::app::AnalysisSettings& settings,
                  const std::function<void(float)>& on_progress) {
        const int turn = ++gate->started;
        wait_until(
            [&] {
                on_progress(0.0f);
                return gate->allowed.load() >= turn;
            },
            "the test to let a chart through");
        return hydra::app::analyze_chart_file(path, settings, on_progress);
    };
}

// kHeldCharts real corpus charts, one per song and each with its own best-path
// score under the default settings. Chart i is titled "Held i", and the charts
// go lowest score first, so with every row unscored the rows read in chart
// order, and each chart a batch runs in that order outscores the ones before.
struct HeldCharts {
    std::vector<ChartLibraryEntry> entries;   // chart i's library row
    std::vector<hydra::HydraRecord> records;  // the engine's analysis of chart i
    std::vector<int64_t> scores;              // the score chart i's row shows
};

const HeldCharts& held_charts() {
    static const HeldCharts held = [] {
        const std::vector<corpus::ChartWithPaths> found = corpus::charts_with_paths(
            Settings{}.to_analysis_settings(), static_cast<size_t>(2 * kHeldCharts));
        std::vector<const corpus::ChartWithPaths*> picked;
        std::vector<std::string> ids;
        std::vector<int64_t> scores;
        for (const corpus::ChartWithPaths& c : found) {
            if (picked.size() == static_cast<size_t>(kHeldCharts)) break;
            const std::string id = hydra::app::hash_chart_file(c.chart);
            const std::optional<int64_t> score =
                hydra::store::record_of(id, c.result.record).summary().score;
            if (!score) continue;
            if (std::find(ids.begin(), ids.end(), id) != ids.end()) continue;
            if (std::find(scores.begin(), scores.end(), *score) != scores.end()) continue;
            picked.push_back(&c);
            ids.push_back(id);
            scores.push_back(*score);
        }
        if (picked.size() != static_cast<size_t>(kHeldCharts))
            throw std::runtime_error("the corpus has too few charts with paths of their own score");
        std::vector<size_t> by_score;
        for (size_t i = 0; i < picked.size(); ++i) by_score.push_back(i);
        std::sort(by_score.begin(), by_score.end(),
                  [&](size_t a, size_t b) { return scores[a] < scores[b]; });
        HeldCharts out;
        for (size_t i = 0; i < by_score.size(); ++i) {
            const corpus::ChartWithPaths& c = *picked[by_score[i]];
            out.entries.push_back(corpus::scanned_entry(c.chart, "Held " + std::to_string(i)));
            out.records.push_back(c.result.record);
            out.scores.push_back(scores[by_score[i]]);
        }
        return out;
    }();
    return held;
}

// Held chart `chart`'s library row.
const ChartLibraryEntry& held_entry(int chart) {
    return held_charts().entries[static_cast<size_t>(chart)];
}

// The held chart with the highest score.
constexpr int kTopHeld = kHeldCharts - 1;

// A library of the held charts with no results, under Best path, highest
// first.
std::unique_ptr<AppState> app_on_held_charts(const ScratchPaths& paths) {
    std::unique_ptr<AppState> app = app_with(paths, held_charts().entries);
    app->library.set_sort(hydra::ui::LibrarySort::BestPath, false);
    return app;
}

// The rows on screen, top to bottom, as held chart numbers.
std::vector<int> shown_held(const AppState& app) {
    std::vector<int> out;
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        for (int chart = 0; chart < kHeldCharts; ++chart)
            if (app.library_row_at(i).entry.md5 == held_entry(chart).md5) out.push_back(chart);
    return out;
}

// The held charts whose rows show a score, in chart order.
std::vector<int> scored_held(const AppState& app) {
    std::vector<int> out;
    for (int chart = 0; chart < kHeldCharts; ++chart)
        if (row_of(app, held_entry(chart).md5).summary.has_scored_best_path())
            out.push_back(chart);
    return out;
}

// The score held chart `chart`'s row shows, if any.
std::optional<int64_t> held_row_score(const AppState& app, int chart) {
    return row_of(app, held_entry(chart).md5).summary.score;
}

// The order Best path, highest first, gives the held charts when the charts in
// `scored` show their scores and the rest show none: the scored charts by
// score, highest first, then the rest in title order. With nothing scored it
// is the title order the rows start in.
std::vector<int> best_path_order(const std::vector<int>& scored) {
    const HeldCharts& held = held_charts();
    std::vector<int> out = scored;
    std::sort(out.begin(), out.end(), [&](int a, int b) {
        return held.scores[static_cast<size_t>(a)] > held.scores[static_cast<size_t>(b)];
    });
    for (int chart = 0; chart < kHeldCharts; ++chart)
        if (std::find(scored.begin(), scored.end(), chart) == scored.end()) out.push_back(chart);
    return out;
}

// Stores the engine's analysis of held chart `chart` behind the view's back,
// as one song's own analysis stores it.
void save_held_result(AppState& app, int chart) {
    app.store->save(hydra::store::prepare_record(
        held_entry(chart).md5, app.settings.profile(),
        held_charts().records[static_cast<size_t>(chart)]));
}

// The frame time `n` batch refresh gaps after the first frame, at 0: each such
// frame is due a refresh (AppState::kBatchRefreshSeconds).
double refresh_frame(int n) { return n * AppState::kBatchRefreshSeconds; }

// The held charts under Best path, whose batch has stored the first two
// charts and whose rows have read both. The second outscores the first, so a
// sort would lift it above the first; the rows still sit in title order.
struct HeldBatchAfterTwo {
    std::unique_ptr<AppState> app;
    std::shared_ptr<ChartGate> gate = std::make_shared<ChartGate>();

    explicit HeldBatchAfterTwo(const ScratchPaths& paths) : app(app_on_held_charts(paths)) {
        start_redo_batch(*app, gated_engine(gate));
        let_through(*app, *gate, 2);
        app->tick_library(refresh_frame(0));
        REQUIRE(scored_held(*app) == std::vector<int>{0, 1});
        REQUIRE(best_path_order(scored_held(*app)) != best_path_order({}));
        REQUIRE(shown_held(*app) == best_path_order({}));
    }
};

}  // namespace

TEST_CASE("a batch refresh fills scores in where the rows sit; the batch's end sorts them") {
    ScratchPaths paths("appstate_batchhold");
    HeldBatchAfterTwo batch(paths);
    AppState& app = *batch.app;
    CHECK(held_row_score(app, 1) == held_charts().scores[1]);
    CHECK(app.library.counts().analyzed == scored_held(app).size());

    let_through(app, *batch.gate, 3);
    app.tick_library(refresh_frame(1));
    CHECK(scored_held(app).size() == 3);
    CHECK(app.library.counts().analyzed == 3);
    CHECK(shown_held(app) == best_path_order({}));

    batch.gate->allowed.store(kHeldCharts);
    wait_batch_finished(app);
    app.tick_library(refresh_frame(2));
    REQUIRE(scored_held(app).size() == static_cast<size_t>(kHeldCharts));
    CHECK(shown_held(app) == best_path_order(scored_held(app)));
}

// Stop leaves the count where the last refresh read it, so the refresh's own
// check finds nothing new: the same ending as a batch whose last result was
// read just before it finished, held still for the test.
TEST_CASE("a batch whose last result was already read still sorts when it ends") {
    ScratchPaths paths("appstate_batchreadend");
    HeldBatchAfterTwo batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    REQUIRE(app.batch_job->snapshot().completed == 2);
    app.tick_library(refresh_frame(1));
    CHECK(shown_held(app) == best_path_order(scored_held(app)));
}

TEST_CASE("a batch's end sorts once, and the next batch sorts when it ends") {
    ScratchPaths paths("appstate_batchonce");
    HeldBatchAfterTwo batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    app.tick_library(refresh_frame(1));
    const std::vector<int> first_end = best_path_order(scored_held(app));
    REQUIRE(shown_held(app) == first_end);

    // A song's own result after the end holds its place, however many frames
    // the finished batch stays on its strip. The top chart outscores every
    // other, so a sort would move it.
    save_held_result(app, kTopHeld);
    REQUIRE(app.refresh_library_row(held_entry(kTopHeld).md5));
    REQUIRE(best_path_order(scored_held(app)) != first_end);
    app.tick_library(refresh_frame(2));
    app.tick_library(refresh_frame(3));
    CHECK(shown_held(app) == first_end);

    start_redo_batch(app, gated_engine(std::make_shared<ChartGate>()));
    stop_batch(app);
    app.tick_library(refresh_frame(4));
    CHECK(shown_held(app) == best_path_order(scored_held(app)));
}

// The finished strip's X can let the job go before the frame's tick reads it.
TEST_CASE("a batch dismissed before a tick saw it finish still sorts") {
    ScratchPaths paths("appstate_batchdismiss");
    HeldBatchAfterTwo batch(paths);
    AppState& app = *batch.app;
    stop_batch(app);
    app.batch_job.reset();
    app.tick_library(refresh_frame(1));
    CHECK(shown_held(app) == best_path_order(scored_held(app)));
}

// ---- the rows take the batch's answers, not a store read --------------------

namespace {

// Whether chart `chart`'s row reads "<score>  <path>" with this score, as
// best_path_label writes it.
bool row_scores(const AppState& app, int chart, const char* score) {
    return row_of(app, library_entry(chart).md5).best_label().find(score) == 0;
}

}  // namespace

// A tick takes the batch's tray and asks the store nothing, so a save group
// holding the store's lock does not hold the frame.
TEST_CASE("a tick during a batch does not wait for the store") {
    ScratchPaths paths("appstate_ticknowait");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    let_through(*app, *gate, 2);  // both charts' lookups are done

    CHECK(returns_while_held(*app->store, [&] { app->tick_library(0.0); }));
    CHECK(row_scores(*app, 0, "1,000"));
    CHECK(row_scores(*app, 1, "2,000"));
}

// Question 1 of the batch-rows plan: a result saved by someone other than the
// batch reaches its row at the next whole read, not at a batch tick.
TEST_CASE("a tick during a batch re-reads no row the batch did not save") {
    ScratchPaths paths("appstate_tickonlysaved");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    let_through(*app, *gate, 1);  // chart 0
    save_scored(*app, 3, 5000);   // behind the batch's back
    app->tick_library(0.0);
    CHECK(row_scores(*app, 0, "1,000"));
    CHECK(row_of(*app, library_entry(3).md5).status == RecordStatus::NotAnalyzed);
}

TEST_CASE("a batch's end re-reads no row the batch did not save") {
    ScratchPaths paths("appstate_endonlysaved");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    // A batch over chart 0 alone, so it finishes with nothing stopped.
    app->set_search(library_entry(0).md5);
    REQUIRE(app->library_shown_count() == 1);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    app->set_search("");
    gate->allowed.store(1);
    wait_batch_finished(*app);
    REQUIRE(app->batch_job->snapshot().run_error.empty());
    REQUIRE_FALSE(app->batch_job->is_cancelled());
    save_scored(*app, 3, 5000);  // behind the batch's back, before the end's tick
    app->tick_library(0.0);
    CHECK(row_scores(*app, 0, "1,000"));
    CHECK(row_of(*app, library_entry(3).md5).status == RecordStatus::NotAnalyzed);
}

// The charts a run that fails as a whole saved never reach the tray
// (run_batch reports none of its open group), so its end reads every row.
TEST_CASE("a batch that fails as a whole still re-reads every row when it ends") {
    ScratchPaths paths("appstate_errorreread");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test,
                              gated_scores(std::make_shared<ChartGate>()), /*workers=*/0);
        app->start_batch(true);
    }
    REQUIRE(app->batch_job != nullptr);
    wait_batch_finished(*app);
    REQUIRE_FALSE(app->batch_job->snapshot().run_error.empty());
    save_scored(*app, 3, 5000);  // behind the batch's back
    app->tick_library(0.0);
    CHECK(row_scores(*app, 3, "5,000"));
}

// The batch's lookup fails once (the records table moved aside between chart
// 0's save and its lookup): the status line says what any failed store read
// says, and no row changes.
TEST_CASE("a lookup the batch could not make shows the store's problem") {
    ScratchPaths paths("appstate_traylookupfails");
    std::atomic<int> lookups{0};  // before the app, so it outlives the batch's thread
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    // Set after start(), while the gate holds every chart: the batch thread
    // reads the seam only once a chart is through, and letting the chart
    // through (the gate's atomic, then run_batch's own hand-off) orders this
    // write before that read.
    app->batch_job->set_lookup_hold_for_test([&lookups, db = paths.db] {
        if (lookups++ == 0)
            hydra::test::exec_on_file(db, "ALTER TABLE records RENAME TO records_aside");
    });
    const std::vector<int> before = shown(*app);
    gate->allowed.store(1);
    double now = 0.0;
    wait_until(
        [&] {
            app->tick_library(now += 1.0);
            return app->status_is_problem;
        },
        "the batch's lookup to fail");
    const std::string from_tick = app->status_message;
    CHECK(row_of(*app, library_entry(0).md5).status == RecordStatus::NotAnalyzed);
    CHECK(shown(*app) == before);
    CHECK(app->library.counts().analyzed == 0);

    // The same failure through the window's own row read (read_store).
    app->dismiss_status();
    CHECK_FALSE(app->refresh_library_row(library_entry(0).md5));
    CHECK(app->status_is_problem);
    CHECK(app->status_message == from_tick);
    CHECK_FALSE(from_tick.empty());

    hydra::test::exec_on_file(paths.db, "ALTER TABLE records_aside RENAME TO records");
}

// O4: the one answer for a chart fills every row that lists it.
TEST_CASE("a chart in two folders gets both rows from one batch result") {
    ScratchPaths paths("appstate_traycopy");
    ChartLibraryEntry copy = library_entry(0);
    copy.title = "Song hash000 copy";
    copy.notespath = "C:\\charts\\copy\\notes.chart";
    std::unique_ptr<AppState> app = app_with(paths, {library_entry(0), copy, library_entry(1)});
    app->set_search(library_entry(0).md5);
    REQUIRE(app->library_shown_count() == 2);
    start_redo_batch(*app, [](const std::string&, const hydra::app::AnalysisSettings&,
                              const std::function<void(float)>&) {
        return hydra::app::AnalysisResult{scored_record(7000), hydra::Song(192)};
    });
    wait_batch_finished(*app);
    REQUIRE(app->batch_job->answered_count() == 1);  // one chart, one answer
    app->set_search("");
    app->tick_library(0.0);

    int rows = 0;
    for (size_t i = 0; i < app->library_shown_count(); ++i) {
        const hydra::ui::LibraryRow& row = app->library_row_at(i);
        if (row.entry.md5 != library_entry(0).md5) continue;
        ++rows;
        CHECK(row.status == RecordStatus::Ready);
        CHECK(row.best_label().find("7,000") == 0);
    }
    CHECK(rows == 2);
    CHECK(row_of(*app, library_entry(1).md5).status == RecordStatus::NotAnalyzed);
}

// O7 and O13: a failure the store keeps reaches its row as the store's
// answer, read here by a second app's own row read on the same file; a
// failure the store does not keep leaves its row as it was.
TEST_CASE("a failed chart whose failure the batch stored shows on its row, and one it did not "
          "store keeps its row") {
    ScratchPaths paths("appstate_trayfailures");
    std::unique_ptr<AppState> app = app_with(paths, {library_entry(0), library_entry(1)});
    const std::string kept = library_entry(0).md5;
    const std::string not_kept = library_entry(1).md5;
    const RecordStatus not_kept_before = row_of(*app, not_kept).status;
    const std::string not_kept_path = library_entry(1).notespath;
    start_redo_batch(*app, [not_kept_path](const std::string& path,
                                           const hydra::app::AnalysisSettings&,
                                           const std::function<void(float)>&)
                               -> hydra::app::AnalysisResult {
        // A song file that cannot be opened is a failure the store does not
        // keep; a chart with no notes is one it keeps.
        if (path == not_kept_path)
            throw hydra::KindedError(hydra::ErrorKind::SongFileMissing, "cannot open file: x");
        throw hydra::NoNotesError(hydra::Difficulty::Expert, true);
    });
    wait_batch_finished(*app);
    REQUIRE(app->batch_job->snapshot().failed == 2);
    app->tick_library(0.0);

    AppState second(Settings{}, std::make_unique<RecordStore>(paths.db));
    second.refresh_library_row(kept);
    second.refresh_library_row(not_kept);
    CHECK(row_of(*app, kept).status == row_of(second, kept).status);
    CHECK(row_of(*app, kept).best_label() == row_of(second, kept).best_label());
    CHECK(row_of(*app, not_kept).status == not_kept_before);
    CHECK(row_of(*app, not_kept).status == row_of(second, not_kept).status);
}

TEST_CASE("a paused batch's stored results reach their rows") {
    ScratchPaths paths("appstate_traypaused");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    let_through(*app, *gate, 1);
    app->batch_job->pause();
    REQUIRE(app->batch_job->snapshot().paused);
    app->tick_library(0.0);
    CHECK(row_scores(*app, 0, "1,000"));
    CHECK(app->library.counts().analyzed == 1);
}

// No tick ran while the batch ran: the stopped batch's end puts both answers
// on their rows, then sorts once.
TEST_CASE("a stopped batch's stored results reach their rows, then the list sorts") {
    ScratchPaths paths("appstate_traystopped");
    std::unique_ptr<AppState> app = app_under_best_path(paths);
    auto gate = std::make_shared<ChartGate>();
    start_redo_batch(*app, gated_scores(gate));
    let_through(*app, *gate, 2);
    stop_batch(*app);
    app->tick_library(10.0);
    CHECK(row_scores(*app, 0, "1,000"));
    CHECK(row_scores(*app, 1, "2,000"));
    CHECK(shown(*app) == std::vector<int>{1, 0, 2, 3});
}

// One song's result, stored while no batch runs, shows on its row, and the
// row stays where it sits until the next sort.
TEST_CASE("one song's stored result keeps its row in place, with no batch running") {
    ScratchPaths paths("appstate_rowhold");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    app->library.set_sort(hydra::ui::LibrarySort::BestPath, false);
    const std::vector<std::string> before = shown_notespaths(*app);
    // Two charts with paths: at most one of them is the top row, so a sort
    // would lift the lower one, the only row with a score, to the top.
    const std::vector<corpus::ChartWithPaths> charts =
        corpus::charts_with_paths(app->settings.to_analysis_settings(), 2);
    REQUIRE(charts.size() == 2);
    const corpus::ChartWithPaths& lower =
        place_of(before, charts[0].chart) > place_of(before, charts[1].chart) ? charts[0]
                                                                              : charts[1];
    const size_t at = place_of(before, lower.chart);
    REQUIRE(at > 0);
    REQUIRE(at < before.size());

    save_engine_result(*app, lower);
    REQUIRE(app->refresh_library_row(hydra::app::hash_chart_file(lower.chart)));
    CHECK(shown_notespaths(*app) == before);
    CHECK(app->library_row_at(at).status == RecordStatus::Ready);
    CHECK(app->library_row_at(at).summary.has_scored_best_path());
    app->tick_library(0.0);  // no batch, so no end to sort at
    CHECK(shown_notespaths(*app) == before);
}

TEST_CASE("a settings change's re-read of the summaries sorts a held order at once") {
    ScratchPaths paths("appstate_settingssort");
    std::unique_ptr<AppState> app = app_on_held_charts(paths);
    // The top chart's own result: its row reads it in place, below the
    // unscored rows a sort would put after it.
    save_held_result(*app, kTopHeld);
    REQUIRE(app->refresh_library_row(held_entry(kTopHeld).md5));
    REQUIRE(scored_held(*app) == std::vector<int>{kTopHeld});
    REQUIRE(shown_held(*app) == best_path_order({}));
    app->refresh_library_summaries();
    CHECK(shown_held(*app) == best_path_order(scored_held(*app)));
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
    // One corpus chart copied into two song folders.
    const ChartLibraryEntry first = corpus_chart("selrow_first");
    const ChartLibraryEntry twin = corpus_chart("selrow_twin");
    REQUIRE(first.md5 == twin.md5);
    REQUIRE(first.notespath != twin.notespath);
    std::unique_ptr<AppState> app = app_with(paths, {first, twin});
    app->selected = first;
    CHECK(app->is_selected_row(first));
    CHECK_FALSE(app->is_selected_row(twin));
    app->selected.reset();
    CHECK_FALSE(app->is_selected_row(first));
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

// A library read before a scan that has since replaced the chart table, with
// no frame run in between, so the rows are older than the store. `kept` is
// what the scan wrote for a chart still in the folder, edited before the
// scan; `before_edit` is the row the library holds for it, the same folder's
// chart before the edit. `removed` is a corpus chart outside the scanned
// folder, so a row the scan dropped.
struct RowsOlderThanScan {
    ChartLibraryEntry kept;
    ChartLibraryEntry before_edit;
    ChartLibraryEntry removed;
    std::unique_ptr<AppState> app;
};

RowsOlderThanScan rows_older_than_scan(const ScratchPaths& paths, const std::string& tag) {
    RowsOlderThanScan s;
    // The edit takes out the note the first copy has at the top of its drums.
    s.before_edit = corpus_chart(tag, /*extra_note=*/true);
    s.kept = corpus_chart(tag);
    REQUIRE(s.kept.notespath == s.before_edit.notespath);
    REQUIRE(s.kept.md5 != s.before_edit.md5);
    REQUIRE(s.kept.sig != s.before_edit.sig);  // so the scan reads the file again
    s.removed = corpus::scanned_entry(corpus::first_chart_with_suffix(".mid"), "Removed");
    REQUIRE(s.removed.md5 != s.kept.md5);
    REQUIRE(s.removed.md5 != s.before_edit.md5);
    s.app = app_with(paths, {s.before_edit, s.removed});
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
    s.app->select(row_of(*s.app, s.before_edit.md5).entry, hydra::ui::SelectionSource::Click);
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
    latch.wait_entered(*app, 1);
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
    // One corpus chart in two song folders, and the same chart with one more
    // note, so a chart of its own.
    const ChartLibraryEntry first = corpus_chart("confirm_first");
    const ChartLibraryEntry copy = corpus_chart("confirm_copy");
    const ChartLibraryEntry other = corpus_chart("confirm_other", /*extra_note=*/true);
    REQUIRE(copy.md5 == first.md5);
    REQUIRE(other.md5 != first.md5);
    store->rebuild_chart_library({first, copy, other});
    AppState app(Settings{}, std::move(store));
    REQUIRE(app.library_shown_count() == 3);

    // The engine's result for the chart, stored behind the library's back: no
    // reload.
    save_engine_result(
        app, corpus::ChartWithPaths{first.notespath,
                                    hydra::app::analyze_chart_file(
                                        first.notespath, app.settings.to_analysis_settings())});
    app.open_batch_confirm();

    CHECK(app.batch_scope_charts() == app.library_shown_count());
    int64_t copies_of_first = 0;
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        if (app.library_row_at(i).entry.md5 == first.md5) ++copies_of_first;
    REQUIRE(copies_of_first == 2);  // the two song folders holding the chart
    CHECK(app.batch_scope_with_result() == copies_of_first);
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
    std::unique_ptr<AppState> app = app_with(paths, {held_entry(0), held_entry(1)});
    app->open_batch_confirm();
    REQUIRE(app->batch_scope_with_result() == 0);
    REQUIRE(app->batch_scope_charts() == 2);

    // The engine's result for chart 1, stored after the confirm opened: a
    // second read would now skip this chart.
    save_held_result(*app, 1);
    app->start_batch(false);  // the app's own analyzer
    REQUIRE(app->batch_job != nullptr);
    wait_batch_finished(*app);

    const hydra::ui::BatchJob::Snapshot s = app->batch_job->snapshot();
    CHECK(s.run_error.empty());
    CHECK(s.total == 2);
    CHECK(s.skipped == 0);
    CHECK(s.analyzed == 2);
    CHECK(s.failed == 0);
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

    using Step = hydra::ui::StoreOpenProgress::Step;
    // One report's step and counts.
    struct Seen {
        Step step;
        uint64_t done;
        uint64_t total;
    };
    // Hold the open at its first report, as the GUI test's gate does.
    std::mutex mu;
    std::atomic<bool> open{false};
    std::vector<Seen> seen;
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>& cancelled) {
            {
                std::lock_guard<std::mutex> lock(mu);
                seen.push_back({p.step, p.rows_done, p.rows_total});
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
    // A file that needs no upgrade reports the open, then the chart list row
    // by row, then the results slice by slice, every count out of the
    // library's one chart count, which is the rows the model holds.
    const uint64_t charts = static_cast<uint64_t>(app.store->chart_library_count());
    CHECK(charts == static_cast<uint64_t>(kChartCount));
    CHECK(charts == app.library.rows().size());
    std::lock_guard<std::mutex> lock(mu);
    std::vector<Step> steps;
    std::vector<uint64_t> charts_done;
    std::vector<uint64_t> results_done;
    for (const Seen& s : seen) {
        if (steps.empty() || steps.back() != s.step) steps.push_back(s.step);
        if (s.step == Step::ReadingCharts) charts_done.push_back(s.done);
        if (s.step == Step::ReadingResults) results_done.push_back(s.done);
        if (s.step != Step::Opening) CHECK(s.total == charts);
    }
    CHECK(steps == std::vector<Step>{Step::Opening, Step::ReadingCharts, Step::ReadingResults});
    // 0 before the first row, then once per row read.
    std::vector<uint64_t> every_row;
    for (uint64_t i = 0; i <= charts; ++i) every_row.push_back(i);
    CHECK(charts_done == every_row);
    // The library fits in one results slice: 0 before it, all once it is read.
    REQUIRE(charts < hydra::ui::kResultsSliceCharts);
    CHECK(results_done == std::vector<uint64_t>{0, charts});
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
    REQUIRE(seed_scanned_corpus(paths) > 0);
    // Dropped once the store is open (it would make a missing table again),
    // at the chart list's first report, before any row is read.
    const ScopedHook gate(
        hydra::ui::set_store_open_gate_for_test,
        [&](const hydra::ui::StoreOpenProgress& p, const std::function<bool()>&) {
            if (p.step == hydra::ui::StoreOpenProgress::Step::ReadingCharts && p.rows_done == 0)
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

// The scanned corpus library on `paths` (seed_scanned_corpus), its scan stamp
// set to one this build does not accept, with the corpus as the ini's chart
// folders plus `extra_folders`. Returns how many rows the library holds.
size_t seed_out_of_date_library(const ScratchPaths& paths,
                                const std::vector<std::string>& extra_folders = {}) {
    const size_t charts = seed_scanned_corpus(paths);
    Settings s = scratch_settings();
    for (const std::string& f : extra_folders) s.chartfolders.push_back(f);
    REQUIRE(s.save());
    hydra::test::exec_on_file(paths.db,
                              "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version'");
    return charts;
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
    const size_t seeded = seed_out_of_date_library(paths);
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
    CHECK(app.library_shown_count() == seeded);
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
    const size_t seeded = seed_out_of_date_library(paths, {missing});
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
    CHECK(app.library_shown_count() == seeded);
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
    const size_t seeded = seed_scanned_corpus(paths);
    Settings s = scratch_settings();
    s.chartfolders.push_back(missing);
    REQUIRE(s.save());
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
    CHECK(app.library_shown_count() == seeded);
    CHECK(app.store->record_count() == records_before);
}

TEST_CASE("the startup screen's line names what the open is doing") {
    using hydra::ui::StoreOpenProgress;
    using Step = StoreOpenProgress::Step;
    // The line for `step`, on an upgrading file or not.
    const auto line = [](Step step, bool upgrading) {
        StoreOpenProgress p;
        p.step = step;
        p.upgrading = upgrading;
        return p.label();
    };
    const std::string opening = line(Step::Opening, false);
    const std::string charts = line(Step::ReadingCharts, false);
    const std::string results = line(Step::ReadingResults, false);
    // Each step of a plain open has a line of its own.
    CHECK_FALSE(opening.empty());
    CHECK_FALSE(charts.empty());
    CHECK_FALSE(results.empty());
    CHECK(opening != charts);
    CHECK(opening != results);
    CHECK(charts != results);
    // The conversion's steps happen only on an upgrading file; anywhere else
    // they read as the open.
    CHECK(line(Step::ConvertingRecords, false) == opening);
    CHECK(line(Step::ReadingSavedPaths, false) == opening);

    // An upgrade names the library read's steps in the same words, and each
    // conversion step in words of its own.
    CHECK(line(Step::Opening, true) == opening);
    CHECK(line(Step::ReadingCharts, true) == charts);
    CHECK(line(Step::ReadingResults, true) == results);
    const std::string converting = line(Step::ConvertingRecords, true);
    const std::string saved_paths = line(Step::ReadingSavedPaths, true);
    CHECK(saved_paths == hydra::ui::kReadingSavedPathsLabel);
    for (const std::string& own : {converting, saved_paths}) {
        CHECK_FALSE(own.empty());
        CHECK(own != opening);
        CHECK(own != charts);
        CHECK(own != results);
    }
    CHECK(converting != saved_paths);
}

// D72 item 4: the Analyze-library click asks the store which charts already
// have a result. When that read fails, the status line says so, no confirm
// opens, and no batch starts.
TEST_CASE("Analyze library on a database that fails shows the sentence and opens no confirm") {
    ScratchPaths paths("appstate_confirmfail");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE records");

    app->open_batch_confirm();
    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(app->status_is_problem);
    CHECK_FALSE(app->batch_confirm_pending);

    app->start_batch(false);
    CHECK(app->batch_job == nullptr);
}

// D73 item 3: a read that fails while Hydra runs shows the read sentence in
// the status line, and the screens keep what they showed. The error never
// leaves the frame, which would close Hydra with no message.
TEST_CASE("a library reload whose reads fail keeps the library and says so") {
    ScratchPaths paths("appstate_reloadfail");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const corpus::ChartWithPaths& analyzed = scratch_chart_with_paths();
    const std::string analyzed_md5 = hydra::app::hash_chart_file(analyzed.chart);
    save_engine_result(*app, analyzed);
    REQUIRE(app->refresh_library_row(analyzed_md5));
    REQUIRE(row_of(*app, analyzed_md5).status == RecordStatus::Ready);
    const std::vector<std::string> before = shown_notespaths(*app);
    hydra::test::exec_on_file(paths.db, "DROP TABLE charts; DROP TABLE records;");

    app->reload_library();

    CHECK(app->status_is_problem);
    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(shown_notespaths(*app) == before);
    CHECK(row_of(*app, analyzed_md5).status == RecordStatus::Ready);
}

namespace {

// The first row on screen, other than chart `skip`'s, whose chart the library
// lists once, so the store's copies of it are that row alone.
size_t first_single_copy_row(const AppState& app, const std::string& skip) {
    for (size_t i = 0; i < app.library_shown_count(); ++i) {
        const std::string& md5 = app.library_row_at(i).entry.md5;
        if (md5 == skip) continue;
        size_t copies = 0;
        for (size_t j = 0; j < app.library_shown_count(); ++j)
            if (app.library_row_at(j).entry.md5 == md5) ++copies;
        if (copies == 1) return i;
    }
    FAIL("no chart in the library has a single copy");
    throw std::logic_error("unreachable");
}

}  // namespace

// ---- the window's reads go through the store's reader ------------------------

// Each test below holds a save group on the app's store from a helper thread,
// the way a batch's group holds the store's lock, and makes one of the
// window's reads. The read goes through store->reader(), whose own connection
// never waits for that lock. Of these reads, only opening a song can happen
// during a batch in the running app; the batch confirm and a settings change
// are closed then, so their tests prove the one-way rule only.

TEST_CASE("opening a song does not wait for a save") {
    ScratchPaths paths("appstate_readeropen");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const auto chart = app->library_row_at(first_single_copy_row(*app, "")).entry;
    CHECK(returns_while_held(*app->store, [&] {
        app->select(chart, hydra::ui::SelectionSource::Click);
    }));
    REQUIRE(app->selected.has_value());
    const std::vector<ChartLibraryEntry> copies =
        app->store->list_chart_library_copies(chart.md5);
    REQUIRE(copies.size() == 1);
    CHECK(app->selected->md5 == copies.front().md5);
    CHECK(app->selected->notespath == copies.front().notespath);
    CHECK(app->selected->title == copies.front().title);
    CHECK(app->selected->sig == copies.front().sig);
}

// A report row's click on a chart the search hides: the store lists its
// copies, and the row's whole entry is read, both through the reader.
TEST_CASE("select_chart on a hidden chart does not wait for a save") {
    ScratchPaths paths("appstate_readerselectchart");
    std::unique_ptr<AppState> app = app_on_scanned_corpus(paths);
    const hydra::ui::LibraryRow& hidden_row = app->library_row_at(first_single_copy_row(*app, ""));
    const std::string hidden = hidden_row.entry.md5;
    const std::string hidden_title = hidden_row.title;
    // A search for another chart's title, in quotes, hides it.
    std::string other_title;
    for (size_t i = 0; i < app->library_shown_count() && other_title.empty(); ++i) {
        const hydra::ui::LibraryRow& row = app->library_row_at(i);
        if (row.entry.md5 != hidden && plain_title(row.title) &&
            hidden_title.find(row.title) == std::string::npos)
            other_title = row.title;
    }
    REQUIRE_MESSAGE(!other_title.empty(), "no other corpus chart has a title of plain words");
    app->set_search("\"" + other_title + "\"");
    REQUIRE(app->library_shown_count() > 0);
    for (size_t i = 0; i < app->library_shown_count(); ++i)
        REQUIRE(app->library_row_at(i).entry.md5 != hidden);
    CHECK(returns_while_held(*app->store, [&] { app->select_chart(hidden); }));
    REQUIRE(app->selected.has_value());
    const std::vector<ChartLibraryEntry> copies =
        app->store->list_chart_library_copies(hidden);
    REQUIRE(copies.size() == 1);
    CHECK(app->selected->md5 == copies.front().md5);
    CHECK(app->selected->notespath == copies.front().notespath);
}

// The running app cannot open the confirm during a batch (the Analyze button
// is off then), so the test calls open_batch_confirm directly.
TEST_CASE("opening the batch confirm does not wait for a save") {
    ScratchPaths paths("appstate_readerconfirm");
    std::unique_ptr<AppState> app = app_on(paths);
    CHECK(returns_while_held(*app->store, [&] { app->open_batch_confirm(); }));
    REQUIRE(app->batch_confirm_pending);

    // The same questions from a second app on the same file, nothing held.
    AppState second(Settings{}, std::make_unique<RecordStore>(paths.db));
    second.open_batch_confirm();
    REQUIRE(second.batch_confirm_pending);
    CHECK(app->batch_question(false) == second.batch_question(false));
    CHECK(app->batch_question(true) == second.batch_question(true));
}

// The settings controls are locked during a batch, so the test changes the
// cap directly and makes the re-read a settings change makes.
TEST_CASE("a settings change re-reads the rows without waiting for a save") {
    ScratchPaths paths("appstate_readersettings");
    std::unique_ptr<AppState> app = app_on(paths);
    const std::string seeded = library_entry(0).md5;
    const RecordStatus seeded_before = row_of(*app, seeded).status;
    app->settings.sp_cap = kSeededCap + 1;
    CHECK(returns_while_held(*app->store, [&] { app->refresh_library_summaries(); }));

    // A second app opened under the changed settings reads every row itself.
    AppState second(app->settings, std::make_unique<RecordStore>(paths.db));
    REQUIRE(row_of(second, seeded).status != seeded_before);  // the change reaches a row
    for (int i = 0; i < kChartCount; ++i) {
        const std::string md5 = library_entry(i).md5;
        CHECK(row_of(*app, md5).status == row_of(second, md5).status);
        CHECK(row_of(*app, md5).best_label() == row_of(second, md5).best_label());
    }
}

// O22: a read the reader cannot make reads as any failed store read does.
TEST_CASE("a failed reader read shows the store's problem") {
    ScratchPaths paths("appstate_readerfails");
    std::unique_ptr<AppState> app = app_on(paths);
    // The app's first read opened the reader. Closed here, so the next read
    // opens it again, and the store's seam makes that open fail.
    dynamic_cast<hydra::store::StoreReader&>(app->store->reader()).close();
    app->store->fail_next_reader_open_for_test();
    const auto chart = row_of(*app, library_entry(5).md5).entry;
    app->select(chart, hydra::ui::SelectionSource::Click);
    CHECK(app->status_is_problem);
    const std::string from_select = app->status_message;
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == library_entry(0).md5);  // the click selected nothing

    // The same failure, read straight from the reader.
    app->store->fail_next_reader_open_for_test();
    std::string from_reader;
    try {
        (void)app->store->reader().list_chart_library_copies(chart.md5);
    } catch (const std::exception& e) {
        from_reader = hydra::app::plain_error(e);
    }
    CHECK_FALSE(from_reader.empty());
    CHECK(from_select == from_reader);
}
