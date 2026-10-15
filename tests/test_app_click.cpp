// Tests for ui/app_state: the click's analysis (D87 items 1-3, 9-11, D90),
// the rules file that turns analysis off, and the panel's other state. The
// other AppState tests are split by topic into test_app_settings.cpp,
// test_app_reports.cpp and test_app_library.cpp; the fixtures they share are
// in app_state_fixtures.h.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dm_report.h"  // GeneratedDmReport
#include "app/dynamics_breakdown.h"
#include "app/report.h"  // GeneratedReport
#include "app_state_fixtures.h"
#include "audio_chart_fixtures.h"
#include "core/model.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file, scalar_on_file, record_rows_in
#include "display_fixtures.h"  // batch_profile, save_old_build_record
#include "library_fixtures.h"  // library_entry
#include "old_layout_fixture.h"  // v220_layout_sql
#include "parse/song.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/library_parts.h"  // analyze_shown_label

using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::Profile;
using hydra::store::RecordStatus;
using hydra::store::RecordStore;
using hydra::ui::AppState;
using hydra::ui::ReportOutOfDate;
using hydra::ui::ViewedSong;

namespace {

using testtemp::temp_path;

// Makes every later change to a library row fail, so a click's
// reidentify_chart throws.
void refuse_library_row_updates(const ScratchPaths& paths) {
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_reidentify BEFORE UPDATE ON charts"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
}

// From here on, every record the store writes adds a row to record_writes,
// so a test can tell "stored nothing new" from "wrote the same row again".
// A record has no row id to compare, so the writes are counted instead.
void count_record_writes(const ScratchPaths& paths) {
    hydra::test::exec_on_file(paths.db,
                              "CREATE TABLE record_writes (n INTEGER);"
                              "CREATE TRIGGER count_record_writes AFTER INSERT ON records"
                              " BEGIN INSERT INTO record_writes VALUES (1); END;");
}

int64_t record_writes(const ScratchPaths& paths) {
    return hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM record_writes");
}

// How many records, and how many kept paths across them, the database holds.
std::pair<int64_t, int64_t> records_and_paths_in(const ScratchPaths& paths) {
    return {hydra::test::record_rows_in(paths.db),
            hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM record_paths")};
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
    const std::pair<int64_t, int64_t> rows = records_and_paths_in(paths);
    REQUIRE(rows.first == 1);
    REQUIRE(rows.second > 0);
    count_record_writes(paths);

    app->close_details();
    click(*app, song);

    REQUIRE(app->viewed.ready());
    CHECK_FALSE(app->viewed.record->paths.empty());
    CHECK(records_and_paths_in(paths) == rows);
    CHECK(record_writes(paths) == 0);
}

// D115 question 4: a click that writes a record changes what both reports
// read, so both open reports say the library changed.
TEST_CASE("the click's save marks both open reports out of date") {
    ScratchPaths paths("appstate_click_marks");
    const ChartLibraryEntry song = corpus_chart("click_marks");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    REQUIRE(row_of(*app, song.md5).status == RecordStatus::NotAnalyzed);
    app->path_report.result = std::make_shared<const hydra::app::report::GeneratedReport>();
    app->dm_report.result = std::make_shared<const hydra::app::dm_report::GeneratedDmReport>();
    REQUIRE(app->path_report.out_of_date == ReportOutOfDate::None);
    REQUIRE(app->dm_report.out_of_date == ReportOutOfDate::None);

    click(*app, song);

    REQUIRE(app->viewed.ready());
    CHECK(app->path_report.out_of_date == ReportOutOfDate::Library);
    CHECK(app->dm_report.out_of_date == ReportOutOfDate::Library);
    // No batch finished, so neither strip names a time.
    CHECK_FALSE(app->path_report.batch_finished.has_value());
    CHECK_FALSE(app->dm_report.batch_finished.has_value());
}

// D115 question 1: a file from 2.2.0 to 2.7.0 keeps its old rows as Stale
// records, and the next save of the chart replaces its record.
TEST_CASE("a Stale converted record is replaced on click") {
    ScratchPaths paths("appstate_click_converted");
    const ChartLibraryEntry song = corpus_chart("click_converted");
    const Settings settings;
    const hydra::app::AnalysisResult fresh =
        hydra::app::analyze_chart_file(song.notespath, settings.to_analysis_settings());
    REQUIRE(fresh.record.paths.size() > 1);  // so the converted record lacks some paths
    {
        RecordStore old(paths.db);
        old.rebuild_chart_library({song});
        hydra::test::save_old_build_record(
            old, hydra::store::prepare_record(song.md5, settings.profile(), fresh.record));
    }
    hydra::test::exec_on_file(paths.db, hydra::test::v220_layout_sql().c_str());

    std::unique_ptr<AppState> app = app_with(paths, {song});  // opening converts the file
    REQUIRE(app->store->lookup_one(song.md5, settings.profile()).status == RecordStatus::Stale);
    REQUIRE(records_and_paths_in(paths) == std::make_pair(int64_t{1}, int64_t{1}));

    click(*app, song);

    REQUIRE(app->viewed.ready());
    const hydra::store::RecordLookup found = app->store->lookup_one(song.md5, settings.profile());
    REQUIRE(found.status == RecordStatus::Ready);
    CHECK(found.record ==
          hydra::store::prepare_record(song.md5, settings.profile(), fresh.record).record);
    CHECK(row_of(*app, song.md5).status == RecordStatus::Ready);
    // The converted record is gone; the chart has the one new record with
    // every path it keeps.
    CHECK(records_and_paths_in(paths) ==
          std::make_pair(int64_t{1}, static_cast<int64_t>(found.record.paths.size())));
}

TEST_CASE("clicking a Stale song turns its row Ready") {
    ScratchPaths paths("appstate_click_stale");
    const ChartLibraryEntry song = corpus_chart("click_stale");
    auto store = std::make_unique<RecordStore>(paths.db);
    store->rebuild_chart_library({song});
    const Settings settings;
    const Profile profile = settings.profile();
    hydra::test::save_old_build_record(
        *store,
        hydra::store::prepare_record(
            song.md5, profile,
            hydra::app::analyze_chart_file(song.notespath, settings.to_analysis_settings())
                .record));
    AppState app(settings, std::move(store));
    REQUIRE(row_of(app, song.md5).status == RecordStatus::Stale);

    click(app, song);

    CHECK(app.viewed.ready());
    CHECK(row_of(app, song.md5).status == RecordStatus::Ready);
    CHECK(app.store->lookup_one(song.md5, profile).status == RecordStatus::Ready);
}

// D87 item 3: the click runs the rescan's unchanged test, re-hashes an edited
// chart, moves its library row to the new hash and analyzes the new file.
TEST_CASE("clicking a chart edited since the scan re-identifies it and shows the new file") {
    ScratchPaths paths("appstate_click_edit");
    const ChartLibraryEntry scanned = corpus_chart("click_edit");
    std::unique_ptr<AppState> app = app_with(paths, {scanned});
    click(*app, scanned);  // its row is saved under the scanned hash
    REQUIRE(app->viewed.ready());
    const Profile profile = app->settings.profile();
    REQUIRE(app->store->lookup_one(scanned.md5, profile).status == RecordStatus::Ready);
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
    CHECK(app->store->lookup_one(scanned.md5, profile).status == RecordStatus::NotAnalyzed);
    CHECK(hydra::test::scalar_on_file(paths.db, "SELECT COUNT(*) FROM records WHERE song_id = '" +
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
    CHECK(hydra::test::record_rows_in(paths.db) == 0);
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
    CHECK(hydra::test::record_rows_in(paths.db) == 0);
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

    app->select(a, hydra::ui::SelectionSource::Click);
    app->select(b, hydra::ui::SelectionSource::Click);
    settle(*app);

    REQUIRE(app->selected.has_value());
    CHECK(app->selected->md5 == b.md5);
    REQUIRE(app->viewed.ready());
    const Profile profile = app->settings.profile();
    const hydra::store::RecordLookup b_found = app->store->lookup_one(b.md5, profile);
    CHECK(b_found.status == RecordStatus::Ready);
    CHECK(app->store->lookup_one(a.md5, profile).status == RecordStatus::NotAnalyzed);
    CHECK(app->viewed.summary == b_found.record.summary());
}

TEST_CASE("Cancel shows Cancelled and saves nothing; Try again shows the paths") {
    ScratchPaths paths("appstate_click_cancel");
    const ChartLibraryEntry song = corpus_chart("click_cancel");
    std::unique_ptr<AppState> app = app_with(paths, {song});
    const Profile profile = app->settings.profile();

    app->select(song, hydra::ui::SelectionSource::Click);
    app->cancel_view();
    settle(*app);

    CHECK(app->viewed.state == ViewedSong::State::Cancelled);
    CHECK(app->store->lookup_one(song.md5, profile).status == RecordStatus::NotAnalyzed);

    app->start_view();  // the panel's "Try again"
    settle(*app);
    CHECK(app->viewed.ready());
    CHECK(app->store->lookup_one(song.md5, profile).status == RecordStatus::Ready);
}

// Ruling 13: closing the panel cancels the click's analysis; nothing is saved
// for it.
TEST_CASE("closing the panel cancels the click's analysis and saves nothing") {
    ScratchPaths paths("appstate_click_close");
    const ChartLibraryEntry song = corpus_chart("click_close");
    std::unique_ptr<AppState> app = app_with(paths, {song});

    app->select(song, hydra::ui::SelectionSource::Click);
    app->close_details();  // what the X, Escape and a new selection all run
    settle(*app);

    CHECK(app->viewed.state == ViewedSong::State::None);
    CHECK_FALSE(app->view_running());
    CHECK(app->store->lookup_one(song.md5, app->settings.profile()).status ==
          RecordStatus::NotAnalyzed);
}

TEST_CASE("a song whose file is gone shows FileMissing and starts no job") {
    ScratchPaths paths("appstate_click_missing");
    std::unique_ptr<AppState> app = app_on(paths);
    app->select(library_entry(3), hydra::ui::SelectionSource::Click);
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
    app->select(song, hydra::ui::SelectionSource::Click);
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
                              "CREATE TRIGGER refuse_records BEFORE INSERT ON records"
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
    audiochart::copy_file_utf8(corpus::root() +
                                   "/common/IB24/T3/Alpha Wolf - Acid Romance/notes.mid",
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
              hydra::app::count_dynamics(hydra::app::load_dynamics_song(song.notespath, as.reading())));
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
    CHECK(hydra::test::record_rows_in(paths.db) == 0);
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
    const std::string seeded = library_entry(0).md5;
    const Profile profile = hydra::test::batch_profile(kSeededCap);

    // Good (absent) rules file: the seeded record, written under the
    // default rules, is Ready.
    {
        AppState good;
        good.wait_store_open();
        CHECK(good.store->lookup_one(seeded, profile).status == RecordStatus::Ready);
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
    CHECK(bad.store->lookup_one(seeded, profile).status == RecordStatus::Stale);
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

// Finding 290: the batch button's narrowed label is built once; the button,
// its width sample and the GUI tests call it (D115 renamed it).
TEST_CASE("the toolbar's narrowed label comes from one function") {
    CHECK(hydra::ui::detail::analyze_shown_label(1) == "Analyze shown (1)...");
    CHECK(hydra::ui::detail::analyze_shown_label(1234) == "Analyze shown (1,234)...");
}
