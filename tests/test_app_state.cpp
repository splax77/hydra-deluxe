// Tests for ui/app_state: the click's analysis (D87 items 1-3, 9-11, D90)
// and commit_settings, the one place that knows which settings change a
// record's identity. A widget only mutates `settings` and commits; whether
// the library's summaries are re-read and the open song analyzed again is
// decided here and nowhere else.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/report_files.h"
#include "audio_chart_fixtures.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file, write_junk_db
#include "display_fixtures.h"  // store_batch_result, old_build_row
#include "parse/song.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/generation.h"
#include "ui/library_parts.h"  // analyze_search_label
#include "ui/library_jobs.h"  // set_app_batch_analyzer_for_test

using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordKey;
using hydra::store::RecordStatus;
using hydra::store::RecordStore;
using hydra::ui::AppState;
using hydra::ui::GenerationWatcher;
using hydra::ui::ViewedSong;

namespace {

// The cap the seeded record is stored at. It is filed under
// hydra::test::batch_result_key (a default Settings at this cap), so a change
// to the chart mode or the cap makes it unfindable.
const int kSeededCap = 4;

// A library big enough to scroll; entry 0 ("Song hash000") sorts first by
// title, so its row is library_row_at(0).
const int kChartCount = 60;

using testtemp::temp_path;

// What a failed database read shows (D73 item 1).
const std::string kDatabaseReadSentence =
    "Hydra couldn't read its database (hydra.db). Check that no other copy of Hydra is "
    "running, then try again.";

// Points app::ini_path()/db_path() at scratch files for one test, then puts
// the process back the way it was. commit_settings writes the INI through
// Settings::save(), so without this a test would overwrite the developer's
// real hydra_settings.ini.
struct ScratchPaths {
    hydra::app::PathOverrides previous;
    std::string ini;
    std::string db;
    std::string rules;

    explicit ScratchPaths(const char* tag)
        : previous(hydra::app::path_overrides()),
          ini(temp_path(tag, ".ini")),
          db(temp_path(tag, ".db")),
          rules(temp_path(tag, "_rules.ini")) {
        std::remove(ini.c_str());
        std::remove(db.c_str());
        std::remove(rules.c_str());
        hydra::app::PathOverrides overrides = previous;
        overrides.ini_path = ini;
        overrides.db_path = db;
        overrides.rules_path = rules;
        hydra::app::set_path_overrides(overrides);
    }

    ~ScratchPaths() {
        hydra::app::set_path_overrides(previous);
        std::remove(ini.c_str());
        std::remove(db.c_str());
        std::remove(rules.c_str());
    }
};

// A library row whose chart file does not exist: a click on it lands on
// FileMissing and starts no job.
ChartLibraryEntry library_entry(int i) {
    char hash[32];
    std::snprintf(hash, sizeof(hash), "hash%03d", i);
    ChartLibraryEntry e;
    e.md5 = hash;
    e.title = std::string("Song ") + hash;
    e.artist = "Artist";
    e.charter = "Charter";
    e.notespath = std::string("C:\\charts\\") + hash + "\\notes.chart";
    e.rootfolder = "C:\\charts";
    e.sig = "sig";
    return e;
}

// A library of kChartCount charts, exactly one of which (entry 0) has a
// stored record, under the default chart mode at the default cap. The record
// itself is empty: a current-version row with no paths reads back as Ready
// (see test_store), and these tests only care which row a lookup finds.
std::unique_ptr<RecordStore> seeded_store(const std::string& db) {
    auto store = std::make_unique<RecordStore>(db);

    std::vector<ChartLibraryEntry> charts;
    for (int i = 0; i < kChartCount; ++i) charts.push_back(library_entry(i));
    store->rebuild_chart_library(charts);

    hydra::test::store_batch_result(*store, library_entry(0).md5, kSeededCap);
    return store;
}

// An AppState on the scratch store with the seeded chart selected (not
// clicked: its file does not exist).
std::unique_ptr<AppState> app_on(const ScratchPaths& paths) {
    auto app = std::make_unique<AppState>(Settings{}, seeded_store(paths.db));
    REQUIRE(app->library_shown_count() == static_cast<size_t>(kChartCount));
    REQUIRE(app->library_row_at(0).entry.md5 == library_entry(0).md5);
    REQUIRE(app->library_row_at(0).status == RecordStatus::Ready);
    app->selected = library_entry(0);
    return app;
}

// ---- real charts for the click ---------------------------------------------

// The library row a scan would write for the chart file at `notespath`
// (corpus::scanned_entry); these tests need its fingerprint to exist, so the
// chart's folder has a song.ini.
ChartLibraryEntry entry_for(const std::string& notespath, const std::string& title) {
    ChartLibraryEntry e = corpus::scanned_entry(notespath, title);
    REQUIRE_FALSE(e.sig.empty());
    return e;
}

// The song.ini every click test writes beside its chart: a folder chart needs
// one to be scanned.
void write_click_ini(const std::string& dir) {
    audiochart::write_text_file(dir + "\\song.ini", "[song]\nname = Click\n");
}

// Writes the corpus's first .chart into folder `tag` with a song.ini (a
// folder chart needs one to be scanned). With `extra_note`, one more note
// goes at the top of its Expert drums, so the file hashes differently.
std::string write_corpus_chart(const std::string& tag, bool extra_note) {
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
ChartLibraryEntry corpus_chart(const std::string& tag, bool extra_note = false) {
    return entry_for(write_corpus_chart(tag, extra_note), "Click " + tag);
}

// An AppState whose library is exactly `charts`, on the scratch database.
std::unique_ptr<AppState> app_with(const ScratchPaths& paths,
                                   const std::vector<ChartLibraryEntry>& charts) {
    auto store = std::make_unique<RecordStore>(paths.db);
    store->rebuild_chart_library(charts);
    return std::make_unique<AppState>(Settings{}, std::move(store));
}

// Runs frames until the click's job, and any request waiting on it, has
// ended and tick() has collected it, as the app's frames would.
void settle(AppState& app) {
    for (int i = 0; i < 12000; ++i) {
        app.tick(0.0);
        if (app.view_settled()) return;
        Sleep(5);
    }
    FAIL("the click's job never ended");
}

// Clicks `entry` and waits for its analysis.
void click(AppState& app, const ChartLibraryEntry& entry) {
    app.select(entry);
    settle(app);
}

// The library row of chart `md5`, by search for nothing: every row shows.
const hydra::ui::LibraryRow& row_of(const AppState& app, const std::string& md5) {
    for (size_t i = 0; i < app.library_shown_count(); ++i)
        if (app.library_row_at(i).entry.md5 == md5) return app.library_row_at(i);
    FAIL("no library row for " << md5);
    throw std::logic_error("unreachable");
}

// Holds every click's job started from now on inside its analysis until
// release(), the way a slow parse would: the job ignores Cancel while held,
// then stops at its first progress tick if it was cancelled. `entered` counts
// the jobs that reached it. Declare it AFTER the AppState: its destructor lets
// every held job through, and the app's destructor joins them.
struct ViewLatch {
    std::shared_ptr<std::atomic<bool>> open = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<std::atomic<int>> entered = std::make_shared<std::atomic<int>>(0);
    ViewLatch() {
        hydra::ui::set_view_analyzer_for_test(
            [open = open, entered = entered](const std::string& path,
                                             const hydra::app::AnalysisSettings& settings,
                                             const std::function<void(float)>& on_progress) {
                ++*entered;
                while (!open->load()) Sleep(1);
                on_progress(0.0f);  // throws when the job was cancelled
                return hydra::app::analyze_chart_file(path, settings, on_progress);
            });
    }
    ~ViewLatch() {
        release();
        hydra::ui::set_view_analyzer_for_test(nullptr);
    }
    void release() { open->store(true); }
    // Waits until `n` jobs have reached the latch.
    void wait_entered(int n) {
        for (int i = 0; i < 12000 && entered->load() < n; ++i) Sleep(1);
        REQUIRE(entered->load() >= n);
    }
};

int64_t results_rows(const ScratchPaths& paths) {
    return hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results");
}

int64_t result_id(const ScratchPaths& paths, const std::string& md5) {
    return hydra::test::scalar_on_file(
        paths.db, "SELECT result_id FROM results WHERE hyhash = '" + md5 + "'");
}

}  // namespace

// ---- the click (D87 items 1-3, 9-11) ---------------------------------------

TEST_CASE("clicking a not-analyzed song shows its paths and turns its row Ready") {
    ScratchPaths paths("appstate_click_new");
    const ChartLibraryEntry song = corpus_chart("click_new");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    REQUIRE(row_of(*app, song.md5).status == RecordStatus::NotAnalyzed);

    click(*app, song);

    REQUIRE(app->viewed.ready());
    REQUIRE_FALSE(app->viewed.record->paths.empty());
    CHECK(app->viewed.timing.has_value());
    CHECK(app->viewed.dynamics.has_value());
    // The row reads what the headline reads.
    const hydra::ui::LibraryRow& row = row_of(*app, song.md5);
    CHECK(row.status == RecordStatus::Ready);
    REQUIRE(row.summary.has_scored_best_path());
    CHECK(*row.summary.score == app->viewed.record->best_path().totalscore());
    CHECK(app->viewed.summary == row.summary);
}

TEST_CASE("clicking a Ready song shows its paths and stores nothing new") {
    ScratchPaths paths("appstate_click_ready");
    const ChartLibraryEntry song = corpus_chart("click_ready");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    REQUIRE(app->viewed.ready());
    const int64_t rows = results_rows(paths);
    const int64_t id = result_id(paths, song.md5);

    app->close_details();
    click(*app, song);

    REQUIRE(app->viewed.ready());
    CHECK_FALSE(app->viewed.record->paths.empty());
    CHECK(results_rows(paths) == rows);
    CHECK(result_id(paths, song.md5) == id);
}

TEST_CASE("clicking a Stale song turns its row Ready") {
    ScratchPaths paths("appstate_click_stale");
    const ChartLibraryEntry song = corpus_chart("click_stale");
    auto store = std::make_unique<RecordStore>(paths.db);
    store->rebuild_chart_library({song});
    const Settings settings;
    const RecordKey key = settings.record_key(song.md5);
    store->add_row(hydra::test::old_build_row(
        key, hydra::app::analyze_chart_file(song.notespath, settings.to_analysis_settings())
                 .record));
    AppState app(settings, std::move(store));
    REQUIRE(row_of(app, song.md5).status == RecordStatus::Stale);

    click(app, song);

    CHECK(app.viewed.ready());
    CHECK(row_of(app, song.md5).status == RecordStatus::Ready);
    CHECK(app.store->get_summary(key).status == RecordStatus::Ready);
}

// D87 item 3: the click runs the rescan's unchanged test, re-hashes an edited
// chart, moves its library row to the new hash and analyzes the new file.
TEST_CASE("clicking a chart edited since the scan re-identifies it and shows the new file") {
    ScratchPaths paths("appstate_click_edit");
    const ChartLibraryEntry scanned = corpus_chart("click_edit");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    click(*app, scanned);  // its row is saved under the scanned hash
    REQUIRE(app->viewed.ready());
    const RecordKey old_key = app->settings.record_key(scanned.md5);
    REQUIRE(app->store->get_summary(old_key).status == RecordStatus::Ready);
    app->close_details();

    // One more note, written after the scan. The mtime has to move for the
    // fingerprint to see it, as it does for a user's edit.
    Sleep(20);
    write_corpus_chart("click_edit", /*extra_note=*/true);
    const std::string new_md5 = hydra::app::hash_chart_file(scanned.notespath);
    REQUIRE(new_md5 != scanned.md5);

    click(*app, scanned);

    REQUIRE(app->viewed.ready());
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == new_md5);
    CHECK(row_of(*app, new_md5).status == RecordStatus::Ready);
    CHECK(app->store->get_summary(old_key).status == RecordStatus::NotAnalyzed);
    CHECK(hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results WHERE hyhash = '" +
                                                    scanned.md5 + "'") == 0);
    // The paths are the new file's.
    const hydra::app::AnalysisResult fresh = hydra::app::analyze_chart_file(
        scanned.notespath, app->settings.to_analysis_settings());
    CHECK(app->viewed.record->best_path().totalscore() == fresh.record.best_path().totalscore());
}

// Ruling 15: when the library row cannot take the new hash, the click fails
// loudly and saves nothing, so no result sits under a hash no row has.
TEST_CASE("a click whose re-identify fails shows the error and saves nothing") {
    ScratchPaths paths("appstate_click_reidfail");
    const ChartLibraryEntry scanned = corpus_chart("click_reidfail");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    Sleep(20);
    write_corpus_chart("click_reidfail", /*extra_note=*/true);
    const std::string new_md5 = hydra::app::hash_chart_file(scanned.notespath);
    REQUIRE(new_md5 != scanned.md5);
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_reidentify BEFORE UPDATE ON charts"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");

    click(*app, scanned);

    CHECK(app->viewed.state == hydra::ui::ViewedSong::State::Failed);
    CHECK_FALSE(app->viewed.message.empty());
    CHECK_FALSE(app->viewed.record.has_value());
    // The row keeps its old hash, and nothing was saved under either hash.
    CHECK(app->selected->md5 == scanned.md5);
    CHECK(hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results") == 0);
}

// D96: a chart saved again with the same content has a new size-and-time
// fingerprint but the same hash. The click saves that fingerprint, so the
// next click finds the file unchanged and does not hash it again.
TEST_CASE("clicking a chart touched since the scan saves its new fingerprint") {
    ScratchPaths paths("appstate_click_touch");
    const ChartLibraryEntry scanned = corpus_chart("click_touch");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    // The same bytes written again: the mtime moves, the hash does not.
    Sleep(20);
    write_corpus_chart("click_touch", /*extra_note=*/false);
    REQUIRE(hydra::app::chart_changed_since(scanned.notespath, scanned.sig).has_value());
    REQUIRE(hydra::app::hash_chart_file(scanned.notespath) == scanned.md5);

    click(*app, scanned);

    REQUIRE(app->viewed.ready());
    // The store's row, the library row and the selected row all hold the
    // fingerprint the file gives now, under the unchanged hash.
    const std::vector<ChartLibraryEntry> stored = app->store->list_chart_library(0, -1);
    REQUIRE(stored.size() == 1);
    CHECK(stored[0].md5 == scanned.md5);
    CHECK_FALSE(hydra::app::chart_changed_since(scanned.notespath, stored[0].sig).has_value());
    CHECK_FALSE(hydra::app::chart_changed_since(scanned.notespath,
                                                row_of(*app, scanned.md5).entry.sig)
                    .has_value());
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == scanned.md5);
    CHECK_FALSE(hydra::app::chart_changed_since(scanned.notespath, app->selected->sig).has_value());
    // The click's result is saved under the unchanged hash.
    CHECK(row_of(*app, scanned.md5).status == RecordStatus::Ready);
}

// D96 keeps ruling 15 for a touched chart: when the fingerprint cannot be
// saved, the click fails loudly and saves nothing.
TEST_CASE("a touched chart whose fingerprint save fails shows the error and saves nothing") {
    ScratchPaths paths("appstate_click_touchfail");
    const ChartLibraryEntry scanned = corpus_chart("click_touchfail");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    Sleep(20);
    write_corpus_chart("click_touchfail", /*extra_note=*/false);
    REQUIRE(hydra::app::chart_changed_since(scanned.notespath, scanned.sig).has_value());
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_reidentify BEFORE UPDATE ON charts"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");

    click(*app, scanned);

    CHECK(app->viewed.state == hydra::ui::ViewedSong::State::Failed);
    CHECK_FALSE(app->viewed.message.empty());
    CHECK_FALSE(app->viewed.record.has_value());
    CHECK(app->selected->sig == scanned.sig);
    CHECK(hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results") == 0);
}

namespace {

// Deletes the song.ini beside a folder chart, so its files give no fingerprint.
void remove_click_ini(const ChartLibraryEntry& chart) {
    const std::string ini =
        std::filesystem::path(chart.notespath).parent_path().string() + "\\song.ini";
    REQUIRE(std::remove(ini.c_str()) == 0);
    REQUIRE(hydra::app::chart_files_sig(chart.notespath).empty());
}

}  // namespace

// D96 follow-up: a folder chart whose song.ini was deleted after the scan
// gives no fingerprint now, and that one could never show the files unchanged
// on a later click (app::sig_can_show_unchanged). Saving it would buy nothing
// and cost a database write and a library reload on every click.
TEST_CASE("clicking a touched chart that lost its song.ini leaves its library row alone") {
    ScratchPaths paths("appstate_click_noini");
    const ChartLibraryEntry scanned = corpus_chart("click_noini");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    remove_click_ini(scanned);
    REQUIRE(hydra::app::hash_chart_file(scanned.notespath) == scanned.md5);

    click(*app, scanned);

    REQUIRE(app->viewed.ready());
    const std::vector<ChartLibraryEntry> stored = app->store->list_chart_library(0, -1);
    REQUIRE(stored.size() == 1);
    CHECK(stored[0].md5 == scanned.md5);
    CHECK(stored[0].sig == scanned.sig);
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->sig == scanned.sig);
    CHECK(row_of(*app, scanned.md5).status == RecordStatus::Ready);
}

// The other side of the test above: an edited chart still takes its new hash
// when its files give no fingerprint, or its results would sit under a hash
// the file no longer has.
TEST_CASE("clicking an edited chart that lost its song.ini still re-identifies it") {
    ScratchPaths paths("appstate_click_editnoini");
    const ChartLibraryEntry scanned = corpus_chart("click_editnoini");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    write_corpus_chart("click_editnoini", /*extra_note=*/true);
    remove_click_ini(scanned);
    const std::string new_md5 = hydra::app::hash_chart_file(scanned.notespath);
    REQUIRE(new_md5 != scanned.md5);

    click(*app, scanned);

    REQUIRE(app->viewed.ready());
    const std::vector<ChartLibraryEntry> stored = app->store->list_chart_library(0, -1);
    REQUIRE(stored.size() == 1);
    CHECK(stored[0].md5 == new_md5);
    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == new_md5);
    CHECK(row_of(*app, new_md5).status == RecordStatus::Ready);
}

TEST_CASE("clicking A then B in one frame shows B, and A is never shown or saved") {
    ScratchPaths paths("appstate_click_ab");
    const ChartLibraryEntry a = corpus_chart("click_a");
    const ChartLibraryEntry b = corpus_chart("click_b", /*extra_note=*/true);
    REQUIRE(a.md5 != b.md5);
    std::unique_ptr<AppState> app = app_with(paths, {a, b});

    app->select(a);
    app->select(b);
    settle(*app);

    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == b.md5);
    REQUIRE(app->viewed.ready());
    const RecordKey a_key = app->settings.record_key(a.md5);
    const RecordKey b_key = app->settings.record_key(b.md5);
    CHECK(app->store->get_summary(b_key).status == RecordStatus::Ready);
    CHECK(app->store->get_summary(a_key).status == RecordStatus::NotAnalyzed);
    CHECK(app->viewed.summary == app->store->get_summary(b_key).summary);
}

TEST_CASE("Cancel shows Cancelled and saves nothing; Try again shows the paths") {
    ScratchPaths paths("appstate_click_cancel");
    const ChartLibraryEntry song = corpus_chart("click_cancel");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    const RecordKey key = app->settings.record_key(song.md5);

    app->select(song);
    app->cancel_view();
    settle(*app);

    CHECK(app->viewed.state == ViewedSong::State::Cancelled);
    CHECK(app->store->get_summary(key).status == RecordStatus::NotAnalyzed);

    app->start_view();  // the panel's "Try again"
    settle(*app);
    CHECK(app->viewed.ready());
    CHECK(app->store->get_summary(key).status == RecordStatus::Ready);
}

// Ruling 13: closing the panel cancels the click's analysis; nothing is saved
// for it.
TEST_CASE("closing the panel cancels the click's analysis and saves nothing") {
    ScratchPaths paths("appstate_click_close");
    const ChartLibraryEntry song = corpus_chart("click_close");
    std::unique_ptr<AppState> app = app_with(paths, {song});

    app->select(song);
    app->close_details();  // what the X, Escape and a new selection all run
    settle(*app);

    CHECK(app->viewed.state == ViewedSong::State::None);
    CHECK_FALSE(app->view_running());
    CHECK(app->store->get_summary(app->settings.record_key(song.md5)).status ==
          RecordStatus::NotAnalyzed);
}

TEST_CASE("a song whose file is gone shows FileMissing and starts no job") {
    ScratchPaths paths("appstate_click_missing");
    std::unique_ptr<AppState> app = app_on(paths);
    app->select(library_entry(3));
    CHECK(app->viewed.state == ViewedSong::State::FileMissing);
    CHECK(app->view_job == nullptr);
    CHECK_FALSE(app->view_pending);
}

// A click's analysis doesn't lock the settings bar; only a batch does (D90
// item 2).
TEST_CASE("the settings lock is the batch's alone") {
    ScratchPaths paths("appstate_click_lock");
    const ChartLibraryEntry song = corpus_chart("click_lock");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    app->select(song);
    CHECK(app->settings_lock() == AppState::SettingsLock::None);
    CHECK_FALSE(app->settings_locked());
    settle(*app);
}

// A store that refuses the save: the paths still show, and the status line
// says the save failed in the database's words.
TEST_CASE("a click whose save fails shows the paths and says so") {
    ScratchPaths paths("appstate_click_savefail");
    const ChartLibraryEntry song = corpus_chart("click_savefail");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_results BEFORE INSERT ON results"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");

    click(*app, song);

    CHECK(app->viewed.ready());
    CHECK(app->status_is_problem);
    CHECK(app->status_message ==
          "Analyzed, but saving failed. Hydra couldn't save to its database (hydra.db). Check "
          "that the disk isn't full and that no other copy of Hydra is running, then try "
          "again.");
}

// ---- the Dynamics count (ruling 10) ----------------------------------------

// The click's count is the Dynamics count's own parse's (load_dynamics_song).
// With 2x Bass off the analysis drops the 2x kicks, so the count parses
// again: the 2x kick row still counts them (37 on this chart, pinned in
// test_dynamics_breakdown).
TEST_CASE("the click's Dynamics count keeps the 2x kicks with 2x Bass off") {
    ScratchPaths paths("appstate_click_dyn");
    const std::string dir = testtemp::temp_dir("click_dyn");
    audiochart::copy_file_utf8(std::string(HYDRA_TESTDATA_DIR) +
                                   "/input/common/IB24/T3/Alpha Wolf - Acid Romance/notes.mid",
                               dir + "\\notes.mid");
    write_click_ini(dir);
    const ChartLibraryEntry song = entry_for(dir + "\\notes.mid", "Click dyn");

    for (const bool bass2x : {true, false}) {
        CAPTURE(bass2x);
        std::unique_ptr<AppState> app = app_with(paths, {song});
        app->settings.view_bass2x = bass2x;
        app->commit_settings();
        click(*app, song);
        REQUIRE(app->viewed.ready());
        REQUIRE(app->viewed.dynamics.has_value());
        CHECK(app->viewed.dynamics->row(hydra::app::DynamicsRow::Kick2x).normal == 37);
        const hydra::app::AnalysisSettings as = app->settings.to_analysis_settings();
        CHECK(*app->viewed.dynamics ==
              hydra::app::count_dynamics(
                  hydra::app::load_dynamics_song(song.notespath, as.prodrums, as.difficulty)));
    }
}

// ---- broken rules (D87 item 9) ---------------------------------------------

TEST_CASE("a bad hydra_rules.ini names the key and keeps analysis off") {
    ScratchPaths paths("appstate_badrules");
    {
        std::ofstream f(paths.rules);
        f << "max_tied_paths = 0\n";
    }
    const ChartLibraryEntry song = corpus_chart("click_badrules");
    {
        RecordStore store(paths.db);
        store.rebuild_chart_library({song});
    }

    // The startup constructor: settings INI, rules file and database from
    // the (scratch) paths, exactly as Hydra.exe starts.
    AppState app;
    CHECK(app.rules_error.find("max_tied_paths") != std::string::npos);
    CHECK(app.analysis_blocked());

    // A click analyzes nothing, but the Dynamics count needs only the parse,
    // so it still shows.
    click(app, song);
    CHECK(app.viewed.state == ViewedSong::State::RulesBroken);
    CHECK_FALSE(app.viewed.record.has_value());
    CHECK(app.viewed.dynamics.has_value());
    CHECK(hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results") == 0);
    app.start_batch(false);
    CHECK(app.batch_job == nullptr);
}

TEST_CASE("no hydra_rules.ini leaves analysis on") {
    ScratchPaths paths("appstate_norules");
    seeded_store(paths.db).reset();
    AppState app;
    CHECK(app.rules_error.empty());
    CHECK_FALSE(app.analysis_blocked());
}

TEST_CASE("under a bad hydra_rules.ini no stored record reads Ready") {
    ScratchPaths paths("appstate_badrules_stale");
    seeded_store(paths.db).reset();
    const RecordKey seeded = hydra::test::batch_result_key(library_entry(0).md5, kSeededCap);

    // Good (absent) rules file: the seeded record, written under the
    // default rules, is Ready.
    {
        AppState good;
        CHECK(good.store->get_summary(seeded).status == RecordStatus::Ready);
    }

    // Bad file: the store is gated on RulesStamp::none(), so the same
    // record reads Stale. Nothing is shown as Ready under rules the user
    // did not choose.
    {
        std::ofstream f(paths.rules);
        f << "max_tied_paths = 0\n";
    }
    AppState bad;
    REQUIRE(bad.analysis_blocked());
    CHECK(bad.store->get_summary(seeded).status == RecordStatus::Stale);
}

// ---- settings (D90) --------------------------------------------------------

TEST_CASE("commit_settings analyzes the open song again when the chart mode changes") {
    ScratchPaths paths("appstate_mode");
    const ChartLibraryEntry song = corpus_chart("click_mode");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    REQUIRE(app->viewed.ready());

    GenerationWatcher records;
    records.changed(app->record_generation);  // start from "already seen"

    // Pro Drums off is a different chart mode: the open song is analyzed
    // under it, and its row for that mode is saved.
    app->settings.view_prodrums = false;
    app->commit_settings();
    CHECK(app->viewed.state == ViewedSong::State::Analyzing);
    CHECK(records.changed(app->record_generation));
    settle(*app);
    CHECK(app->viewed.ready());
    CHECK(row_of(*app, song.md5).status == RecordStatus::Ready);
    CHECK(app->store->get_summary(app->settings.record_key(song.md5)).status ==
          RecordStatus::Ready);

    // The INI is written on the scratch path, not the user's.
    CHECK(Settings::load_file(paths.ini).view_prodrums == false);
}

TEST_CASE("commit_settings analyzes the open song again on an SP cap change") {
    ScratchPaths paths("appstate_cap");
    const ChartLibraryEntry song = corpus_chart("click_cap");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);

    app->settings.sp_cap = 8;
    app->commit_settings();
    REQUIRE(app->view_job != nullptr);
    CHECK(app->view_job->key().cap == app->settings.cap_query());
    settle(*app);
    CHECK(app->viewed.ready());
    CHECK(row_of(*app, song.md5).status == RecordStatus::Ready);
}

TEST_CASE("commit_settings analyzes again when the ms limit or the score range changes") {
    ScratchPaths paths("appstate_lens");
    const ChartLibraryEntry song = corpus_chart("click_lens");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);

    // The result answered "best path under this ms limit". Move the limit and
    // the question changes, so the song is analyzed under the new one.
    const int seeded_ms = app->settings.mslimit_value;
    app->settings.mslimit_value = seeded_ms + 5;
    app->commit_settings();
    REQUIRE(app->view_job != nullptr);
    CHECK(app->view_job->key().lens == app->settings.lens());
    CHECK(Settings::load_file(paths.ini).mslimit_value == seeded_ms + 5);
    settle(*app);
    CHECK(app->viewed.ready());

    // The score range is part of the same identity.
    app->settings.depth_value += 1;
    app->commit_settings();
    REQUIRE(app->view_job != nullptr);
    CHECK(app->view_job->key().lens == app->settings.lens());
    settle(*app);
    CHECK(app->viewed.ready());
}

TEST_CASE("commit_settings with a non-identity change analyzes nothing again") {
    ScratchPaths paths("appstate_plain");
    const ChartLibraryEntry song = corpus_chart("click_plain");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The hit window is a display setting -- it never reaches the search, so
    // nothing cached goes stale and nothing is analyzed again.
    app->settings.hit_window_ms += 1;
    app->commit_settings();
    CHECK_FALSE(records.changed(app->record_generation));
    CHECK(app->view_job == nullptr);
    CHECK(app->viewed.ready());
    CHECK(Settings::load_file(paths.ini).hit_window_ms == app->settings.hit_window_ms);
}

// Ruling 12: a burst of setting changes (a held +/- box) runs one job at a
// time and ends with one analysis of the final settings. The settings in
// between are never saved. The first step's job is held at a latch, as a
// slow parse would hold it, so the test doesn't depend on the machine's speed.
TEST_CASE("a burst of setting changes ends with one analysis of the final settings") {
    ScratchPaths paths("appstate_burst");
    const ChartLibraryEntry song = corpus_chart("click_burst");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    REQUIRE(app->viewed.ready());

    ViewLatch latch;  // after the app: see ViewLatch
    const int first_cap = app->settings.sp_cap + 1;
    const int last_cap = first_cap + 4;
    app->settings.sp_cap = first_cap;
    app->edit_settings();
    REQUIRE(app->view_job != nullptr);
    const int first_generation = app->view_job->generation();
    latch.wait_entered(1);  // the first job is inside its analysis now
    for (int cap = first_cap + 1; cap <= last_cap; ++cap) {
        app->settings.sp_cap = cap;
        app->edit_settings();
        // The one slot still holds the first job; the latest request waits.
        REQUIRE(app->view_job != nullptr);
        CHECK(app->view_job->generation() == first_generation);
        CHECK(app->view_pending);
        app->tick(0.0);  // a frame: the held job hasn't ended, so nothing starts
        CHECK(app->view_job->generation() == first_generation);
    }
    const int final_generation = app->view_generation.n;
    latch.release();

    std::set<int> started{first_generation};
    for (int i = 0; i < 12000 && (app->view_job || app->view_pending); ++i) {
        // While a request waits, the job still running is an older one.
        if (app->view_pending) CHECK(app->view_job != nullptr);
        app->tick(0.0);
        if (app->view_job) started.insert(app->view_job->generation());
        Sleep(5);
    }
    REQUIRE_FALSE(app->view_job);
    CHECK(started == std::set<int>{first_generation, final_generation});
    CHECK(latch.entered->load() == 2);

    CHECK(app->viewed.ready());
    Settings at = app->settings;
    for (int cap = first_cap; cap <= last_cap; ++cap) {
        CAPTURE(cap);
        at.sp_cap = cap;
        CHECK(app->store->get_summary(at.record_key(song.md5)).status ==
              (cap == last_cap ? RecordStatus::Ready : RecordStatus::NotAnalyzed));
    }
}

// The number boxes apply each step at once (the open song is analyzed under
// it) but leave the INI until the edit ends: holding +/- used to rewrite the
// file every frame.
TEST_CASE("number boxes apply at once but write the INI only on flush") {
    ScratchPaths paths("appstate_flush");
    const ChartLibraryEntry song = corpus_chart("click_flush");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    click(*app, song);
    const int seeded_depth = app->settings.depth_value;

    app->settings.depth_value = seeded_depth + 3;
    app->edit_settings();
    CHECK(app->viewed.state == ViewedSong::State::Analyzing);  // applied live
    CHECK(Settings::load_file(paths.ini).depth_value == seeded_depth);  // not saved yet

    app->flush_settings();  // the edit ended
    CHECK(Settings::load_file(paths.ini).depth_value == seeded_depth + 3);
    settle(*app);
}

// A number typed outside its range takes the path the settings boxes take
// (Settings::clamp, then edit_settings and the flush) and must land where a
// hand-edited INI line with the same number lands (D51 call 14).
TEST_CASE("a number setting edited outside its range lands on the edge the file loader uses") {
    ScratchPaths paths("appstate_range");
    std::unique_ptr<AppState> app = app_on(paths);

    app->settings.mslimit_value = Settings::clamp(&Settings::mslimit_value, 900);
    app->settings.depth_value = Settings::clamp(&Settings::depth_value, -1);
    app->settings.sp_cap = Settings::clamp(&Settings::sp_cap, 0);
    app->edit_settings();
    app->flush_settings();
    const Settings saved = Settings::load_file(paths.ini);

    // The same numbers written straight into an INI by hand.
    const std::string hand_ini = temp_path("appstate_range_hand", ".ini");
    {
        std::ofstream f(hand_ini, std::ios::trunc);
        f << "mslimit_value=900\ndepth_value=-1\n";
    }
    const Settings hand = Settings::load_file(hand_ini);
    std::remove(hand_ini.c_str());

    CHECK(saved.mslimit_value == static_cast<int>(hydra::kSqueezeWindowMs));
    CHECK(saved.mslimit_value == hand.mslimit_value);
    CHECK(saved.depth_value == 0);
    CHECK(saved.depth_value == hand.depth_value);
    // The cap floors at 1 bar (D51 call 16). A hand-written sp_cap=0 is the
    // one place the two differ on purpose: the file reads 0 as Clone Hero's 4.
    CHECK(saved.sp_cap == 1);
}

// ---- the song's length comes from the click --------------------------------

// The length comes from song.ini, not the audio (D75): junk bytes under the
// audio's name change nothing.
TEST_CASE("the click reads a chart's stated length") {
    ScratchPaths paths("appstate_length");
    const std::string notes = audiochart::short_chart_with_long_audio("click_length");
    const std::string folder = notes.substr(0, notes.rfind('\\'));
    audiochart::write_text_file(folder + "\\song.ini", "[song]\nsong_length = 4321\n");
    audiochart::write_text_file(folder + "\\song.ogg", "not audio at all");
    const ChartLibraryEntry song = entry_for(notes, "Click length");
    std::unique_ptr<AppState> app = app_with(paths, {song});

    click(*app, song);

    REQUIRE(app->viewed.ready());
    CHECK(app->viewed.song_length_ms == 4321.0);
}

// A chart that states no length reads its last Expert drum note (100 ms
// here), with no audio file at all (D75 items 2 and 5).
TEST_CASE("a chart with no stated length and no audio reads its last note") {
    ScratchPaths paths("appstate_noaudio");
    const std::string notes = audiochart::short_chart_with_long_audio("click_noaudio");
    const std::string folder = notes.substr(0, notes.rfind('\\'));
    REQUIRE(std::remove((folder + "\\song.ogg").c_str()) == 0);
    write_click_ini(folder);
    const ChartLibraryEntry song = entry_for(notes, "Click no audio");
    std::unique_ptr<AppState> app = app_with(paths, {song});

    click(*app, song);

    REQUIRE(app->viewed.ready());
    CHECK(app->viewed.song_length_ms == 100.0);
}

// ---- the panel's other state -----------------------------------------------

// The "Song file not found" check asks the disk when the window opens and
// then every two seconds, not on every frame.
TEST_CASE("the chart-file check runs on open and then every two seconds") {
    ScratchPaths paths("appstate_fileok");
    std::unique_ptr<AppState> app = app_on(paths);
    const std::string chart = temp_path("fileok_chart", ".chart");
    { std::ofstream f(chart); f << "[Song]\n"; }
    app->selected->notespath = chart;

    CHECK(app->selected_file_ok(10.0));        // first look
    std::remove(chart.c_str());
    CHECK(app->selected_file_ok(11.0));        // one second later: not asked again
    CHECK_FALSE(app->selected_file_ok(12.5));  // two seconds on: asked, and gone

    { std::ofstream f(chart); f << "[Song]\n"; }
    app->close_details();                      // the next open looks at once
    CHECK(app->selected_file_ok(12.6));
    std::remove(chart.c_str());
}

// The short UI timings the user confirmed (D48, Q33; D87 item 6): how long
// "Copied!" stays, how often typing re-filters the library, how often a
// running batch refreshes its results, and how long a click runs before its
// progress box shows.
TEST_CASE("UI timings: Copied!, search re-filter, batch refresh and the progress box delay") {
    CHECK(AppState::kCopiedSeconds == 2.0);
    CHECK(AppState::kSearchThrottleSeconds == 0.15);
    CHECK(AppState::kBatchRefreshSeconds == 1.0);
    CHECK(AppState::kFileCheckSeconds == 2.0);    // D48 Q33, the one file-check interval
    CHECK(AppState::kStatusFadeSeconds == 6.0);   // the toolbar's since 2026-08-18
    CHECK(AppState::kViewProgressDelaySeconds == 0.15);  // D87 item 6
}

// Finding 290: the batch button's search label is built once; the button,
// its width sample and the GUI tests call it.
TEST_CASE("the toolbar's search label comes from one function") {
    CHECK(hydra::ui::detail::analyze_search_label(1) == "Analyze search (1)...");
    CHECK(hydra::ui::detail::analyze_search_label(1234) == "Analyze search (1,234)...");
}

// The main window's "Open path report" button looks for the file every two
// seconds, not on every frame.
TEST_CASE("the report-file check is cached for two seconds") {
    ScratchPaths paths("appstate_report");
    std::unique_ptr<AppState> app = app_on(paths);
    const std::filesystem::path report(hydra::app::report_html_path());
    std::error_code ec;
    std::filesystem::remove(report, ec);

    CHECK_FALSE(app->report_file_shown(10.0));
    hydra::app::write_report_file(report, "<html></html>");
    CHECK_FALSE(app->report_file_shown(11.0));  // one second later: not asked
    CHECK(app->report_file_shown(12.5));        // two seconds on: asked, found
    std::filesystem::remove(report, ec);
}

// The search runs in memory over every chart; a word matches inside a title.
TEST_CASE("set_search narrows the library and the match count") {
    ScratchPaths paths("appstate_search");
    std::unique_ptr<AppState> app = app_on(paths);
    app->set_search("hash017");
    CHECK(app->library.searching());
    CHECK(app->library_shown_count() == 1);
    CHECK(app->library_match_count() == 1);
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

namespace {

// Starts a Redo batch with a test analyzer, and removes it from the global
// seam at once so no later batch inherits it.
void start_redo_batch(AppState& app, hydra::app::ChartAnalyzer analyzer) {
    hydra::ui::set_app_batch_analyzer_for_test(std::move(analyzer), 1);
    app.start_batch(true);
    hydra::ui::set_app_batch_analyzer_for_test(nullptr, 1);
    REQUIRE(app.batch_job != nullptr);
}

// Waits until the batch has finished (however it ends).
void wait_batch_finished(AppState& app) {
    while (!app.batch_job->snapshot().finished)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// Runs a Redo batch over the one chart `search` finds and waits for it to
// end. The batch's own analysis fails on purpose, so it writes nothing: the
// result the test stored behind the app's back is the one the batch
// "stored". Redo, so the batch does not skip a chart with a result.
void run_redo_batch_over(AppState& app, const std::string& search) {
    app.set_search(search);
    REQUIRE(app.library_shown_count() == 1);
    start_redo_batch(app, [](const std::string&, const hydra::app::AnalysisSettings&,
                             const std::function<void(float)>&) -> hydra::app::AnalysisResult {
        throw std::runtime_error("no chart file in this test");
    });
    wait_batch_finished(app);
}

}  // namespace

// Ruling 14: after a batch the library rows refresh, and the open panel is
// not analyzed again: it keeps what its own click showed (D48 Q16 restated).
TEST_CASE("a batch result for the open chart refreshes its row, not the panel") {
    ScratchPaths paths("appstate_batchpanel");
    std::unique_ptr<AppState> app = app_on(paths);
    const ChartLibraryEntry open = library_entry(5);
    app->select(open);
    REQUIRE(app->viewed.state == ViewedSong::State::FileMissing);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The result arrives the way a batch files one (H1's fixture).
    hydra::test::store_batch_result(*app->store, open.md5, Settings{}.sp_cap);
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

    // An empty result (no paths, so no score) under the same key.
    hydra::test::store_batch_result(*app->store, app->settings.record_key(song.md5));
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

    hydra::test::store_batch_result(*app->store, fifth.md5, kSeededCap);
    app->refresh_library_row(fifth.md5);
    CHECK(app->library_row_at(at).status == RecordStatus::Ready);
    CHECK(app->library.counts().analyzed == 2);
}

namespace {

// Starts a Redo batch over the whole library that keeps running until the
// test presses Stop: each chart's analysis waits on the batch's own cancel
// check (its progress callback throws once Stop is pressed).
void start_batch_until_stopped(AppState& app) {
    start_redo_batch(app, [](const std::string&, const hydra::app::AnalysisSettings&,
                             const std::function<void(float)>& on_progress)
                              -> hydra::app::AnalysisResult {
        for (;;) {
            on_progress(0.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    REQUIRE(app.batch_running());
}

void stop_batch(AppState& app) {
    app.batch_job->stop();
    wait_batch_finished(app);
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
TEST_CASE("a scan cannot start during a batch, and the status line says so (D51 Q24)") {
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
    // collects it.
    const ChartLibraryEntry song = corpus_chart("click_anyjob");
    app->store->rebuild_chart_library({song});
    app->select(song);
    REQUIRE(app->view_job != nullptr);
    app->cancel_view();
    if (!app->view_job->finished()) CHECK(app->any_job_running());
    settle(*app);
    CHECK_FALSE(app->any_job_running());
}

// The report that follows a batch lists what the batch analyzed, even when
// the settings bar moved on after it (D51 call 26).
TEST_CASE("the post-batch report lists the batch's cap and lens, not the live settings") {
    ScratchPaths paths("appstate_reportrun");
    std::unique_ptr<AppState> app = app_on(paths);
    REQUIRE(app->settings.sp_cap == 4);
    run_redo_batch_over(*app, library_entry(5).md5);
    const hydra::store::Lens batch_lens = app->batch_job->batch_run().lens;

    app->settings.sp_cap = 5;  // edited, not committed
    app->settings.mslimit_enabled = !app->settings.mslimit_enabled;
    app->update_background_jobs();

    REQUIRE(app->report_job != nullptr);
    CHECK(app->report_job->cap() == hydra::store::CapQuery::at(4));
    CHECK(app->report_job->lens() == batch_lens);
    CHECK_FALSE(app->report_job->lens() == app->settings.lens());
    while (!app->report_job->finished()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::error_code ec;
    std::filesystem::remove(std::filesystem::path(hydra::app::report_html_path()), ec);
}

// The confirm counts library rows, every copy included, like the library
// does (D76), and reads which have a result from the store, not from the
// library's cached rows.
TEST_CASE("the confirm counts every library row, and rows with a result from the store (D76)") {
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
    hydra::test::store_batch_result(*app.store, first.md5, Settings{}.sp_cap);
    app.open_batch_confirm();

    CHECK(app.batch_scope_charts() == app.library_shown_count());
    CHECK(app.batch_scope_with_result() == 2);  // both copies of chart 1
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
    hydra::test::store_batch_result(*app.store, library_entry(2).md5, Settings{}.sp_cap);
    hydra::ui::set_app_batch_analyzer_for_test(
        [](const std::string&, const hydra::app::AnalysisSettings&,
           const std::function<void(float)>&) -> hydra::app::AnalysisResult {
            throw std::runtime_error("no chart file in this test");
        },
        1);
    app.start_batch(false);
    hydra::ui::set_app_batch_analyzer_for_test(nullptr, 1);
    REQUIRE(app.batch_job != nullptr);
    wait_batch_finished(app);

    const hydra::ui::BatchJob::Snapshot s = app.batch_job->snapshot();
    CHECK(s.total == 2);
    CHECK(s.skipped == 0);
    CHECK(s.failed == 2);
}

// D72 item 2: the startup constructor's store open reads "couldn't open"
// even when SQLite only notices the junk at its first statement. main() shows
// it in a message box.
TEST_CASE("an AppState whose database can't open throws DatabaseOpen") {
    ScratchPaths paths("appstate_junkdb");  // puts the overrides back when it ends
    hydra::test::write_junk_db(paths.db);
    try {
        AppState app;
        FAIL("an AppState opened a file of junk bytes");
    } catch (const hydra::KindedError& e) {
        CHECK(e.kind() == hydra::ErrorKind::DatabaseOpen);
    }
}

// D72 item 4: the Analyze-library click asks the store which charts already
// have a result. When that read fails, the status line says so, no confirm
// opens, and no batch starts.
TEST_CASE("Analyze library on a database that fails shows the sentence and opens no confirm") {
    ScratchPaths paths("appstate_confirmfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE results");

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
    hydra::test::exec_on_file(paths.db, "DROP TABLE charts; DROP TABLE results;");

    app->reload_library();

    CHECK(app->status_is_problem);
    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(app->library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
}

TEST_CASE("a settings change whose reads fail shows no other settings' record") {
    ScratchPaths paths("appstate_settingsfail");
    std::unique_ptr<AppState> app = app_on(paths);
    app->select(library_entry(0));
    hydra::test::exec_on_file(paths.db, "DROP TABLE results;");

    app->settings.sp_cap = 8;
    app->commit_settings();

    CHECK(app->status_message == kDatabaseReadSentence);
    // The library keeps the rows it showed; the panel shows nothing ready,
    // since its last answer was for the old cap.
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
    CHECK_FALSE(app->viewed.ready());
}
