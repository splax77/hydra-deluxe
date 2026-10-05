// Tests for ui/app_state's commit_settings: the one place that knows which
// settings change a record's identity. A widget only mutates `settings` and
// commits; whether the library's summaries and the viewed record are re-read
// is decided here and nowhere else.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/report_files.h"
#include "audio_chart_fixtures.h"
#include "core/error_kind.h"
#include "core/model.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "db_file_util.h"  // exec_on_file, write_junk_db
#include "display_fixtures.h"  // store_batch_result
#include "parse/song.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/details_view.h"  // analyze_button_label
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

// An AppState on the scratch store with the seeded chart selected and its
// record loaded.
std::unique_ptr<AppState> app_on(const ScratchPaths& paths) {
    auto app = std::make_unique<AppState>(Settings{}, seeded_store(paths.db));
    REQUIRE(app->library_shown_count() == static_cast<size_t>(kChartCount));
    REQUIRE(app->library_row_at(0).entry.md5 == library_entry(0).md5);
    REQUIRE(app->library_row_at(0).status == RecordStatus::Ready);
    app->selected = library_entry(0);
    app->refresh_viewed_record();
    REQUIRE(app->viewed.status == RecordStatus::Ready);
    return app;
}

}  // namespace

TEST_CASE("commit_settings refreshes the viewed record when the chart mode changes") {
    ScratchPaths paths("appstate_mode");
    std::unique_ptr<AppState> app = app_on(paths);

    GenerationWatcher records;
    records.changed(app->record_generation);  // start from "already seen"

    // Pro Drums off is a different chart mode, so the stored record no longer
    // answers the question being asked.
    app->settings.view_prodrums = false;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
    CHECK(app->library_row_at(0).status == RecordStatus::NotAnalyzed);  // the row follows
    CHECK(app->library_shown_count() == static_cast<size_t>(kChartCount));
    CHECK(records.changed(app->record_generation));

    // The INI is written on the scratch path, not the user's.
    CHECK(Settings::load_file(paths.ini).view_prodrums == false);

    app->settings.view_prodrums = true;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::Ready);
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
}

TEST_CASE("commit_settings refreshes the record and the library row on an SP cap change") {
    ScratchPaths paths("appstate_cap");
    std::unique_ptr<AppState> app = app_on(paths);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // Changing the cap re-reads the record and the row's summary.
    app->settings.sp_cap = 8;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
    CHECK(app->library_row_at(0).status == RecordStatus::NotAnalyzed);
    CHECK(records.changed(app->record_generation));

    app->settings.sp_cap = kSeededCap;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::Ready);
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
}

TEST_CASE("commit_settings refreshes when the ms limit or the score range changes") {
    ScratchPaths paths("appstate_lens");
    std::unique_ptr<AppState> app = app_on(paths);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The stored result answered "best path under a 10 ms limit". Move the
    // limit and the question changes, so the answer no longer applies.
    const int seeded_ms = app->settings.mslimit_value;
    app->settings.mslimit_value = seeded_ms + 5;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
    CHECK(app->library_row_at(0).status == RecordStatus::NotAnalyzed);
    CHECK(records.changed(app->record_generation));
    CHECK(Settings::load_file(paths.ini).mslimit_value == seeded_ms + 5);

    // Moving it back is instant: the old result is still stored, and nothing
    // has to be analyzed again.
    app->settings.mslimit_value = seeded_ms;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::Ready);
    CHECK(app->analyze_job == nullptr);

    // The score range is part of the same identity.
    const int seeded_depth = app->settings.depth_value;
    app->settings.depth_value = seeded_depth + 1;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
    app->settings.depth_value = seeded_depth;
    app->commit_settings();
    CHECK(app->viewed.status == RecordStatus::Ready);
    CHECK(app->analyze_job == nullptr);
}

TEST_CASE("commit_settings with a non-identity change does not bump the record generation") {
    ScratchPaths paths("appstate_plain");
    std::unique_ptr<AppState> app = app_on(paths);

    GenerationWatcher records;
    records.changed(app->record_generation);

    // The hit window is a display setting -- it never reaches the search, so
    // nothing cached goes stale and nothing is re-read.
    app->settings.hit_window_ms += 1;
    app->commit_settings();
    CHECK_FALSE(records.changed(app->record_generation));
    CHECK(app->viewed.status == RecordStatus::Ready);
    CHECK(Settings::load_file(paths.ini).hit_window_ms == app->settings.hit_window_ms);
}

TEST_CASE("a bad hydra_rules.ini names the key and keeps analysis off") {
    ScratchPaths paths("appstate_badrules");
    {
        std::ofstream f(paths.rules);
        f << "max_tied_paths = 0\n";
    }
    seeded_store(paths.db).reset();  // the library and one record, on disk

    // The startup constructor: settings INI, rules file and database from
    // the (scratch) paths, exactly as Hydra.exe starts.
    AppState app;
    CHECK(app.rules_error.find("max_tied_paths") != std::string::npos);
    CHECK(app.analysis_blocked());

    // The buttons are disabled, and the state refuses too, so no other
    // caller can start an analysis on the wrong rules.
    app.selected = library_entry(0);
    app.start_analyze();
    CHECK(app.analyze_job == nullptr);
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

TEST_CASE("update_dynamics recounts a stored row with an older count stamp") {
    ScratchPaths paths("appstate_dyn_old");
    std::unique_ptr<AppState> app = app_on(paths);
    const hydra::store::DynamicsKey key = hydra::app::dynamics_store_key(
        library_entry(0).md5, app->settings.difficulty(), app->settings.view_prodrums);
    // A row counted before the last bump of the counter.
    app->store->put_dynamics(key, hydra::app::encode_dynamics(hydra::app::DynamicsBreakdown{}),
                             hydra::store::kDynamicsCountStamp.written - 1);

    app->update_dynamics();

    // The old row was not shown; a background recount started instead. (The
    // chart file does not exist, so the job itself fails. This test only
    // cares that the recount was started.)
    CHECK_FALSE(app->dynamics_result.has_value());
    CHECK(app->dynamics_job != nullptr);
}

TEST_CASE("update_dynamics uses a stored row with the current count stamp") {
    ScratchPaths paths("appstate_dyn_now");
    std::unique_ptr<AppState> app = app_on(paths);
    const hydra::store::DynamicsKey key = hydra::app::dynamics_store_key(
        library_entry(0).md5, app->settings.difficulty(), app->settings.view_prodrums);
    hydra::app::save_dynamics(*app->store, key, hydra::app::DynamicsBreakdown{});

    app->update_dynamics();

    CHECK(app->dynamics_result.has_value());
    CHECK(app->dynamics_job == nullptr);  // no recount
}

// A Dynamics count that finished while another tab showed is stored when the
// window closes, not thrown away. Before, only the Dynamics tab collected the
// job, so the next open parsed the chart again.
TEST_CASE("close_details keeps a Dynamics count that finished on another tab") {
    ScratchPaths paths("appstate_dyn_close");
    std::unique_ptr<AppState> app = app_on(paths);
    app->selected->notespath = corpus::first_chart_with_suffix(".chart");
    app->show_details = true;

    app->update_dynamics();  // the Dynamics tab was shown once: the parse starts
    REQUIRE(app->dynamics_job != nullptr);
    for (int i = 0; i < 1200 && !app->dynamics_job->finished(); ++i) Sleep(50);
    REQUIRE(app->dynamics_job->finished());
    REQUIRE(app->dynamics_job->ok());

    // The user went back to Paths, so update_dynamics never ran again.
    app->close_details();

    CHECK_FALSE(app->show_details);
    CHECK(app->dynamics_job == nullptr);
    CHECK_FALSE(app->dynamics_result.has_value());
    const hydra::store::DynamicsKey key = hydra::app::dynamics_store_key(
        library_entry(0).md5, app->settings.difficulty(), app->settings.view_prodrums);
    CHECK(hydra::app::load_stored_dynamics(*app->store, key).has_value());
}

// Finding 193: a Dynamics count whose save fails reads the database's
// sentence, in the same shape as "Analyzed, but saving failed. ".
TEST_CASE("a Dynamics save failure reads the database sentence") {
    ScratchPaths paths("appstate_dyn_savefail");
    seeded_store(paths.db).reset();
    // A trigger refuses every Dynamics row, so put_dynamics throws.
    hydra::test::exec_on_file(paths.db,
                              "CREATE TRIGGER refuse_dynamics BEFORE INSERT ON dynamics"
                              " BEGIN SELECT RAISE(ABORT, 'boom'); END;");
    std::unique_ptr<AppState> app = app_on(paths);
    app->selected->notespath = corpus::first_chart_with_suffix(".chart");

    app->update_dynamics();  // the parse starts
    REQUIRE(app->dynamics_job != nullptr);
    for (int i = 0; i < 1200 && !app->dynamics_job->finished(); ++i) Sleep(50);
    REQUIRE(app->dynamics_job->finished());
    REQUIRE(app->dynamics_job->ok());
    app->update_dynamics();  // collects the count and tries to save it

    CHECK(app->dynamics_store_error ==
          "Counted, but saving failed. Hydra couldn't save to its database (hydra.db). Check "
          "that the disk isn't full and that no other copy of Hydra is running, then try "
          "again.");
}

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

// The four short UI timings the user confirmed (D48, Q33): how long "Done!"
// and "Copied!" stay, how often typing re-filters the library, and how often
// a running batch refreshes its results.
TEST_CASE("UI timings: Done!, Copied!, search re-filter and batch refresh keep their seconds") {
    CHECK(AppState::kDoneFlashSeconds == 0.5);
    CHECK(AppState::kCopiedSeconds == 2.0);
    CHECK(AppState::kSearchThrottleSeconds == 0.15);
    CHECK(AppState::kBatchRefreshSeconds == 1.0);
    CHECK(AppState::kFileCheckSeconds == 2.0);    // D48 Q33, the one file-check interval
    CHECK(AppState::kStatusFadeSeconds == 6.0);   // the toolbar's since 2026-08-18
}

// Findings 290 and 289: the batch button's search label and the song panel's
// Analyze label are each built once; the button, its width sample and the
// GUI tests call these.
TEST_CASE("the toolbar and song panel labels come from one function each") {
    CHECK(hydra::ui::detail::analyze_search_label(1) == "Analyze search (1)...");
    CHECK(hydra::ui::detail::analyze_search_label(1234) == "Analyze search (1,234)...");
    using hydra::ui::analyze_button_label;
    CHECK(analyze_button_label(RecordStatus::NotAnalyzed) == std::string("Analyze this song"));
    CHECK(analyze_button_label(RecordStatus::Stale) == std::string("Re-analyze"));
    CHECK(analyze_button_label(RecordStatus::Ready) == std::string("Re-analyze"));
}

// The number boxes apply each step at once (the shown record follows live)
// but leave the INI until the edit ends: holding +/- used to rewrite the
// file every frame.
TEST_CASE("number boxes apply at once but write the INI only on flush") {
    ScratchPaths paths("appstate_flush");
    std::unique_ptr<AppState> app = app_on(paths);
    const int seeded_depth = app->settings.depth_value;

    app->settings.depth_value = seeded_depth + 3;
    app->edit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);  // applied live
    CHECK(Settings::load_file(paths.ini).depth_value == seeded_depth);  // not saved yet

    app->flush_settings();  // the edit ended
    CHECK(Settings::load_file(paths.ini).depth_value == seeded_depth + 3);
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

// Stepping away and back re-shows a lookup already made, without asking the
// store again (a big record's decode is the expensive part).
TEST_CASE("stepping a number box back reuses the lookup it already made") {
    ScratchPaths paths("appstate_parked");
    std::unique_ptr<AppState> app = app_on(paths);

    app->settings.sp_cap = 8;
    app->edit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
    app->settings.sp_cap = kSeededCap;
    app->edit_settings();
    CHECK(app->viewed.status == RecordStatus::Ready);

    // A cap-8 record appears behind the cache's back. Stepping to 8 shows the
    // parked "not analyzed" answer: proof the store was not asked again.
    hydra::test::store_batch_result(*app->store, library_entry(0).md5, 8);
    app->settings.sp_cap = 8;
    app->edit_settings();
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);

    // A new selection drops every parked lookup, so the store is asked again.
    app->select(library_entry(0));
    CHECK(app->viewed.status == RecordStatus::Ready);
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
// the Analyze button keeps reading "Analyze library..." (D48, Q15).
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

// Runs a Redo batch over chart `md5` alone and waits for it to end. The
// batch's own analysis fails, since the test has no chart file, so it writes
// nothing: the result the test stored behind the app's back is the one the
// batch "stored". Redo, so the batch does not skip a chart with a result.
void run_redo_batch_over(AppState& app, const std::string& md5) {
    app.set_search(md5);
    REQUIRE(app.library_shown_count() == 1);
    start_redo_batch(app, [](const std::string&, const hydra::app::AnalysisSettings&,
                             const std::function<void(float)>&) -> hydra::app::AnalysisResult {
        throw std::runtime_error("no chart file in this test");
    });
    wait_batch_finished(app);
}

}  // namespace

// A batch that stores a result for the chart the panel is open on turns the
// panel Ready on the batch's next refresh, without clicking away (D48, Q16).
TEST_CASE("a batch result for the open chart turns the panel Ready") {
    ScratchPaths paths("appstate_batchpanel");
    std::unique_ptr<AppState> app = app_on(paths);
    const ChartLibraryEntry open = library_entry(5);
    app->select(open);
    REQUIRE(app->viewed.status == RecordStatus::NotAnalyzed);

    // The result arrives the way a batch files one (H1's fixture).
    hydra::test::store_batch_result(*app->store, open.md5, Settings{}.sp_cap);
    run_redo_batch_over(*app, open.md5);

    app->tick_library(0.0);  // the batch's last refresh
    CHECK(app->viewed.status == RecordStatus::Ready);
}

// A Redo batch that stores a new result for a chart that was already Ready
// shows the new result too: the row stays Ready, but what it holds changed.
TEST_CASE("a batch result for an open Ready chart shows the new result") {
    ScratchPaths paths("appstate_batchredo");
    std::unique_ptr<AppState> app = app_on(paths);  // chart 0: Ready, no paths
    const ChartLibraryEntry open = library_entry(0);
    app->select(open);
    REQUIRE(app->viewed.status == RecordStatus::Ready);
    REQUIRE(app->viewed.record->paths.empty());

    // The new result has two paths (the shared tied-variant record).
    hydra::HydraRecord redone = hydra::test::tied_variant_record();
    redone.sp_cap = kSeededCap;
    redone.ms_limit = Settings{}.mslimit_value;
    app->store->add_record(hydra::test::batch_result_key(open.md5, kSeededCap), redone);
    run_redo_batch_over(*app, open.md5);

    app->tick_library(0.0);  // the batch's last refresh
    REQUIRE(app->viewed.status == RecordStatus::Ready);
    CHECK(app->viewed.record->paths.size() == 2);
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

// Opens library chart 0 on the chart file at `path` (a real corpus chart, no
// audio, when empty), its Ready record's song registered with a tempo map and
// no length read, the way a result saved before Hydra read audio lengths
// reads. Returns the chart file's path.
std::string open_chart_with_no_length(AppState& app, std::string path = {}) {
    const hydra::app::AnalysisSettings as = app.settings.to_analysis_settings();
    if (path.empty()) path = corpus::first_chart_with_notes(as.difficulty);
    const hydra::Song song =
        hydra::load_songpath(path, as.prodrums, as.bass2x, as.difficulty, as.rules);
    ChartLibraryEntry entry = library_entry(0);
    entry.notespath = path;
    app.store->add_song(entry.md5, entry.title, entry.artist, entry.charter, song);
    app.select(entry);
    REQUIRE(app.viewed.status == RecordStatus::Ready);
    REQUIRE(app.viewed.timing.has_value());
    REQUIRE_FALSE(app.viewed.song_length_read);
    REQUIRE_FALSE(app.viewed.song_length_ms.has_value());
    return path;
}

// Runs the open chart's length backfill to its end: starts it, waits for it
// and lets tick() store what it read.
void run_length_backfill(AppState& app) {
    app.tick(0.0);  // starts the backfill
    REQUIRE(app.length_job != nullptr);
    for (int i = 0; i < 1200 && !app.length_job->finished(); ++i) Sleep(50);
    REQUIRE(app.length_job->finished());
    app.tick(0.0);  // stores what it read
    CHECK(app.length_job == nullptr);
}

}  // namespace

// The length belongs to the song, so one read gives every difficulty its
// length (D69 item 2). It comes from song.ini, not the audio (D75): junk
// bytes under the audio's name change nothing. The library row is one an
// older scan wrote, with no stated length kept, so the backfill reads
// song.ini itself (SL1 open question 3).
TEST_CASE("the backfill reads a chart's stated length once, and every difficulty shows it") {
    ScratchPaths paths("appstate_length");
    std::unique_ptr<AppState> app = app_on(paths);
    // The same chart's Ready record under another chart mode.
    Settings other = app->settings;
    other.view_prodrums = !other.view_prodrums;
    const RecordKey other_key = other.record_key(library_entry(0).md5);
    hydra::test::store_batch_result(*app->store, other_key);
    const std::string notes = audiochart::short_chart_with_long_audio("backfill");
    const std::string folder = notes.substr(0, notes.rfind('\\'));
    audiochart::write_text_file(folder + "\\song.ini", "[song]\nsong_length = 4321\n");
    audiochart::write_text_file(folder + "\\song.ogg", "not audio at all");
    open_chart_with_no_length(*app, notes);

    run_length_backfill(*app);

    CHECK(app->viewed.song_length_ms == 4321.0);
    CHECK(app->store->get_record(app->settings.record_key(library_entry(0).md5)).song_length_ms ==
          4321.0);
    CHECK(app->store->get_record(other_key).song_length_ms == 4321.0);
}

// A chart that states no length reads its last Expert drum note (100 ms
// here), with no audio file at all (D75 items 2 and 5). The answer is stored,
// so it is not read again (D70).
TEST_CASE("a chart with no stated length and no audio reads its last note, once") {
    ScratchPaths paths("appstate_noaudio");
    std::unique_ptr<AppState> app = app_on(paths);
    const std::string notes = audiochart::short_chart_with_long_audio("backfill_noaudio");
    REQUIRE(std::remove((notes.substr(0, notes.rfind('\\')) + "\\song.ogg").c_str()) == 0);
    open_chart_with_no_length(*app, notes);

    run_length_backfill(*app);
    CHECK(app->viewed.song_length_ms == 100.0);
    const hydra::store::RecordLookup stored =
        app->store->get_record(app->settings.record_key(library_entry(0).md5));
    CHECK(stored.song_length_read);
    CHECK(stored.song_length_ms == 100.0);

    // Selecting it again starts no job.
    const ChartLibraryEntry open = *app->selected;
    app->select(open);
    app->tick(0.0);
    CHECK(app->length_job == nullptr);
}

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

    open_chart_with_no_length(*app);
    app->tick(0.0);  // starts the length read
    REQUIRE(app->length_job != nullptr);
    CHECK(app->any_job_running());
    for (int i = 0; i < 1200 && !app->length_job->finished(); ++i) Sleep(50);
    REQUIRE(app->length_job->finished());
    app->tick(0.0);
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

// The confirm counts charts, not copies, and reads which have a result from
// the store, not from the library's cached rows (D51 call 10, D62 item 3).
TEST_CASE("the confirm counts charts with a result from the store, once per chart (D51 Q10)") {
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

    CHECK(app.batch_scope_charts == 2);
    CHECK(app.batch_scope_with_result == 1);
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

TEST_CASE("a re-read of the open chart that fails keeps its record and says so") {
    ScratchPaths paths("appstate_rereadfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE results;");

    app->refresh_viewed_record();

    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK(app->viewed.status == RecordStatus::Ready);
}

TEST_CASE("a settings change whose reads fail shows no other settings' record") {
    ScratchPaths paths("appstate_settingsfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE results;");

    app->settings.sp_cap = 8;
    app->commit_settings();

    CHECK(app->status_message == kDatabaseReadSentence);
    // The library keeps the rows it showed; the panel shows nothing, since
    // its last answer was for the old cap.
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
    CHECK(app->viewed.status == RecordStatus::NotAnalyzed);
}

TEST_CASE("a Dynamics read that fails says so and counts the chart instead") {
    ScratchPaths paths("appstate_dynreadfail");
    std::unique_ptr<AppState> app = app_on(paths);
    hydra::test::exec_on_file(paths.db, "DROP TABLE dynamics;");

    app->update_dynamics();

    CHECK(app->status_message == kDatabaseReadSentence);
    CHECK_FALSE(app->dynamics_result.has_value());
    CHECK(app->dynamics_job != nullptr);  // the count starts, as on a store miss
}
