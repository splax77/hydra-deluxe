// Tests for ui/app_state: the click's analysis (D87 items 1-3, 9-11, D90).
// The settings cases are in test_app_settings.cpp and the report cases in
// test_app_reports.cpp; the fixtures the AppState tests share are in
// app_state_fixtures.h.

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
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/user_messages.h"  // kDatabaseReadSentence
#include "app_state_fixtures.h"
#include "audio_chart_fixtures.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file, write_junk_db
#include "display_fixtures.h"  // store_batch_result, old_build_row
#include "library_fixtures.h"  // library_entry
#include "net/dmbot_client.h"   // set_fetcher
#include "parse/song.h"
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/generation.h"
#include "ui/library_parts.h"  // analyze_search_label
#include "ui/library_jobs.h"  // set_app_batch_analyzer_for_test
#include "ui/report_window.h"  // path_report_input
#include "wait_util.h"

using hydra::app::kDatabaseReadSentence;
using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordKey;
using hydra::store::RecordStatus;
using hydra::store::RecordStore;
using hydra::ui::AppState;
using hydra::ui::GenerationWatcher;
using hydra::ui::ViewedSong;
using testwait::wait_until;

namespace {

using testtemp::temp_path;

int64_t results_rows(const ScratchPaths& paths) {
    return hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM results");
}

// Makes every later change to a library row fail, so a click's
// reidentify_chart throws.
void refuse_library_row_updates(const ScratchPaths& paths) {
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_reidentify BEFORE UPDATE ON charts"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
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
    refuse_library_row_updates(paths);

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
    // The store's row and the selected row hold the fingerprint the file
    // gives now, under the unchanged hash. The library row keeps no
    // fingerprint (LibraryChart); the next click reads the store's.
    const std::vector<ChartLibraryEntry> stored = app->store->list_chart_library(0, -1);
    REQUIRE(stored.size() == 1);
    CHECK(stored[0].md5 == scanned.md5);
    CHECK_FALSE(hydra::app::chart_changed_since(scanned.notespath, stored[0].sig).has_value());
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
    refuse_library_row_updates(paths);

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
// on a later click (store::sig_can_show_unchanged). Saving it would buy nothing
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

// A click's analysis doesn't lock the analysis settings; only a batch does (D90
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
    app.wait_store_open();
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
    app.wait_store_open();
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
        good.wait_store_open();
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
    bad.wait_store_open();
    REQUIRE(bad.analysis_blocked());
    CHECK(bad.store->get_summary(seeded).status == RecordStatus::Stale);
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
    s.app->select(row_of(*s.app, kHashBeforeEdit).entry);
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
    s.app->select(row_of(*s.app, s.removed.md5).entry);
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
    CHECK(s.app->batch_scope_charts() == static_cast<int64_t>(s.app->library_match_count()));
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
                hydra::test::exec_on_file(paths.db, "DROP TABLE charts; DROP TABLE results;");
        });
    AppState app;
    app.wait_store_open();
    REQUIRE(app.store_ready());
    CHECK(app.library.rows().empty());
    CHECK(app.status_is_problem);
    CHECK(app.status_message == kDatabaseReadSentence);
}

TEST_CASE("the startup screen's line names what the open is doing") {
    using hydra::ui::StoreOpenProgress;
    StoreOpenProgress p;
    CHECK(p.label() == "Opening your library...");
    p.step = StoreOpenProgress::Step::LoadingLibrary;
    CHECK(p.label() == "Opening your library...");

    p.upgrading = true;
    p.step = StoreOpenProgress::Step::UpdatingResultsKey;
    CHECK(p.label() == "Updating the results table...");
    p.step = StoreOpenProgress::Step::Copying;
    CHECK(p.label() == "Copying your library...");
    p.step = StoreOpenProgress::Step::Finishing;
    CHECK(p.label() == "Finishing...");
    p.step = StoreOpenProgress::Step::LoadingLibrary;
    CHECK(p.label() == "Finishing...");
}

TEST_CASE("the startup screen's time left follows the Preview loader's gate") {
    using hydra::ui::StoreOpenProgress;
    StoreOpenProgress p;
    p.upgrading = true;
    p.step = StoreOpenProgress::Step::Copying;
    p.time_left_s = 65.0;
    p.elapsed_s = 2.9;
    CHECK(p.time_left_text() == "");  // under the gate
    p.elapsed_s = 3.0;
    CHECK(p.time_left_text() == "about 1:05 left");
    p.time_left_s = -1.0;
    CHECK(p.time_left_text() == "");  // the rate is not known
    p.time_left_s = 65.0;
    p.step = StoreOpenProgress::Step::Finishing;
    CHECK(p.time_left_text() == "");  // only the copy has a rate
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
