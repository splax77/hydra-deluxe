// Shared fixtures for the AppState tests, test_app_click.cpp and the files
// split from it: a seeded library with no chart files, real corpus charts to
// click, the frame loop that waits for a click, and a Redo batch run with a
// test analyzer. They were one file's helpers; they live here once so the
// split files share them instead of copying them.
#ifndef HYDRA_TESTS_APP_STATE_FIXTURES_H
#define HYDRA_TESTS_APP_STATE_FIXTURES_H

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "doctest.h"

#include "app/analysis.h"
#include "app/config.h"
#include "audio_chart_fixtures.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // save_batch_record
#include "library_fixtures.h"  // library_entry
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/library_jobs.h"  // set_app_batch_analyzer_for_test
#include "wait_util.h"

// ---- a seeded library with no chart files -----------------------------------

// The cap the seeded record is stored at. It is filed under
// hydra::test::batch_profile (a default Settings at this cap), so a change
// to the chart mode or the cap makes it unfindable.
inline constexpr int kSeededCap = 4;

// A library big enough to scroll; entry 0 ("Song hash000") sorts first by
// title, so its row is library_row_at(0).
inline constexpr int kChartCount = 60;

// A library of kChartCount charts, exactly one of which (entry 0) has a
// stored record, under the default chart mode at the default cap. The record
// itself is empty: a current-version row with no paths reads back as Ready
// (see test_store_results.cpp), and these tests only care which row a lookup
// finds.
inline std::unique_ptr<hydra::store::RecordStore> seeded_store(const std::string& db) {
    auto store = std::make_unique<hydra::store::RecordStore>(db);

    std::vector<hydra::store::ChartLibraryEntry> charts;
    for (int i = 0; i < kChartCount; ++i) charts.push_back(library_entry(i));
    store->rebuild_chart_library(charts);

    hydra::test::save_batch_record(*store, library_entry(0).md5, kSeededCap);
    return store;
}

// An AppState on the scratch store with the seeded chart selected (not
// clicked: its file does not exist).
inline std::unique_ptr<hydra::ui::AppState> app_on(const ScratchPaths& paths) {
    auto app = std::make_unique<hydra::ui::AppState>(hydra::app::Settings{},
                                                     seeded_store(paths.db));
    REQUIRE(app->library_shown_count() == static_cast<size_t>(kChartCount));
    REQUIRE(app->library_row_at(0).entry.md5 == library_entry(0).md5);
    REQUIRE(app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    app->selected = library_entry(0);
    return app;
}

// ---- real charts for the click ---------------------------------------------

// The library row a scan would write for the chart file at `notespath`
// (corpus::scanned_entry); these tests need its fingerprint to exist, so the
// chart's folder has a song.ini.
inline hydra::store::ChartLibraryEntry entry_for(const std::string& notespath,
                                                 const std::string& title) {
    hydra::store::ChartLibraryEntry e = corpus::scanned_entry(notespath, title);
    REQUIRE_FALSE(e.sig.empty());
    return e;
}

// The song.ini every click test writes beside its chart: a folder chart needs
// one to be scanned.
inline void write_click_ini(const std::string& dir) {
    audiochart::write_text_file(dir + "\\song.ini", "[song]\nname = Click\n");
}

// Writes the corpus's first .chart into folder `tag` with a song.ini (a
// folder chart needs one to be scanned). With `extra_note`, one more note
// goes at the top of its Expert drums, so the file hashes differently.
inline std::string write_corpus_chart(const std::string& tag, bool extra_note) {
    const std::string dir = testtemp::temp_dir(tag);
    const std::vector<uint8_t> bytes =
        hydra::read_file_bytes(corpus::first_chart_with_suffix(".chart"));
    std::string text(bytes.begin(), bytes.end());
    if (extra_note) {
        const size_t section = text.find("[ExpertDrums]");
        REQUIRE(section != std::string::npos);
        const size_t open = text.find('\n', text.find('{', section));
        REQUIRE(open != std::string::npos);
        text.insert(open + 1, "  0 = N 4 0\n");
    }
    const std::string notes = dir + "\\notes.chart";
    audiochart::write_text_file(notes, text);
    write_click_ini(dir);
    return notes;
}

// A copy of a corpus chart, as a library row.
inline hydra::store::ChartLibraryEntry corpus_chart(const std::string& tag,
                                                    bool extra_note = false) {
    return entry_for(write_corpus_chart(tag, extra_note), "Click " + tag);
}

// An AppState whose library is exactly `charts`, on the scratch database.
inline std::unique_ptr<hydra::ui::AppState> app_with(
    const ScratchPaths& paths, const std::vector<hydra::store::ChartLibraryEntry>& charts) {
    auto store = std::make_unique<hydra::store::RecordStore>(paths.db);
    store->rebuild_chart_library(charts);
    return std::make_unique<hydra::ui::AppState>(hydra::app::Settings{}, std::move(store));
}

// Runs frames until the click's job, and any request waiting on it, has
// ended and tick() has collected it, as the app's frames would.
inline void settle(hydra::ui::AppState& app) {
    testwait::wait_until(
        [&] {
            app.tick(0.0);
            return app.view_settled();
        },
        "the click's job to end");
}

// Clicks `entry` and waits for its analysis.
inline void click(hydra::ui::AppState& app, const hydra::store::ChartLibraryEntry& entry) {
    app.select(entry, hydra::ui::SelectionSource::Click);
    settle(app);
}

// The library row of chart `md5`, by search for nothing: every row shows.
inline const hydra::ui::LibraryRow& row_of(const hydra::ui::AppState& app,
                                           const std::string& md5) {
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        if (app.library_row_at(i).entry.md5 == md5) return app.library_row_at(i);
    FAIL("no library row for " << md5);
    throw std::logic_error("unreachable");
}

// ---- a Redo batch with a test analyzer -------------------------------------

// Starts a Redo batch with a test analyzer, and removes it from the global
// seam at once so no later batch inherits it.
inline void start_redo_batch(hydra::ui::AppState& app, hydra::app::ChartAnalyzer analyzer) {
    {
        const ScopedHook seam(hydra::ui::set_app_batch_analyzer_for_test, std::move(analyzer), 1);
        app.start_batch(true);
    }
    REQUIRE(app.batch_job != nullptr);
}

// Waits until the batch has finished (however it ends).
inline void wait_batch_finished(hydra::ui::AppState& app) {
    testwait::wait_until([&] { return app.batch_job->snapshot().finished; },
                         "the batch to finish");
}

// Runs a Redo batch over the one chart `search` finds and waits for it to
// end. The batch's own analysis fails on purpose, so it writes nothing: the
// result the test stored behind the app's back is the one the batch
// "stored". Redo, so the batch does not skip a chart with a result.
inline void run_redo_batch_over(hydra::ui::AppState& app, const std::string& search) {
    app.set_search(search);
    REQUIRE(app.library_shown_count() == 1);
    start_redo_batch(app, [](const std::string&, const hydra::app::AnalysisSettings&,
                             const std::function<void(float)>&) -> hydra::app::AnalysisResult {
        throw std::runtime_error("no chart file in this test");
    });
    wait_batch_finished(app);
}

// Presses the batch's Stop and waits for the batch to end.
inline void stop_batch(hydra::ui::AppState& app) {
    app.batch_job->stop();
    wait_batch_finished(app);
}

#endif  // HYDRA_TESTS_APP_STATE_FIXTURES_H
