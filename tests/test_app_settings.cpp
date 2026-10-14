// Tests for ui/app_state's commit_settings (D90), the one place that knows
// which settings change a record's identity. A widget only mutates `settings`
// and commits; whether the library's summaries are re-read and the open song
// analyzed again is decided here and nowhere else. Split from
// test_app_click.cpp; the shared fixtures are in app_state_fixtures.h.

#include "doctest.h"

#include <atomic>
#include <cstdio>
#include <fstream>
#include <functional>
#include <memory>
#include <set>
#include <string>

#include "app/analysis.h"
#include "app/config.h"
#include "app/user_messages.h"  // kDatabaseReadSentence
#include "app_state_fixtures.h"
#include "core/model.h"
#include "db_file_util.h"  // exec_on_file
#include "library_fixtures.h"  // library_entry
#include "scoped_hook.h"
#include "scratch_paths.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "ui/app_state.h"
#include "ui/generation.h"
#include "wait_util.h"

using hydra::app::kDatabaseReadSentence;
using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStatus;
using hydra::ui::AppState;
using hydra::ui::GenerationWatcher;
using hydra::ui::ViewedSong;
using testwait::wait_until;

namespace {

using testtemp::temp_path;

// Holds every click's job started from now on inside its analysis until
// release(), the way a slow parse would: the job ignores Cancel while held,
// then stops at its first progress tick if it was cancelled. `entered` counts
// the jobs that reached it. Declare it AFTER the AppState: its destructor lets
// every held job through, and the app's destructor joins them.
struct ViewLatch {
    std::shared_ptr<std::atomic<bool>> open = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<std::atomic<int>> entered = std::make_shared<std::atomic<int>>(0);
    // After open and entered, which it copies; cleared after the release below.
    const ScopedHook seam{
        hydra::ui::set_view_analyzer_for_test,
        [open = open, entered = entered](const std::string& path,
                                         const hydra::app::AnalysisSettings& settings,
                                         const std::function<void(float)>& on_progress) {
            ++*entered;
            wait_until([&open] { return open->load(); }, "the test to open the latch");
            on_progress(0.0f);  // throws when the job was cancelled
            return hydra::app::analyze_chart_file(path, settings, on_progress);
        }};
    ~ViewLatch() { release(); }
    void release() { open->store(true); }
    // Waits until `n` jobs have reached the latch.
    void wait_entered(int n) {
        wait_until([&] { return entered->load() >= n; },
                   std::to_string(n) + " jobs to reach the latch");
    }
};

}  // namespace

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
    wait_until(
        [&] {
            if (!app->view_job && !app->view_pending) return true;
            // While a request waits, the job still running is an older one.
            if (app->view_pending) CHECK(app->view_job != nullptr);
            app->tick(0.0);
            if (app->view_job) started.insert(app->view_job->generation());
            return false;
        },
        "the held click jobs and the waiting request to end");
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

// ---- a settings change whose reads fail (D73 item 3) -----------------------

TEST_CASE("a settings change whose reads fail shows no other settings' record") {
    ScratchPaths paths("appstate_settingsfail");
    std::unique_ptr<AppState> app = app_on(paths);
    app->select(library_entry(0), hydra::ui::SelectionSource::Click);
    hydra::test::exec_on_file(paths.db, "DROP TABLE records;");

    app->settings.sp_cap = 8;
    app->commit_settings();

    CHECK(app->status_message == kDatabaseReadSentence);
    // The library keeps the rows it showed; the panel shows nothing ready,
    // since its last answer was for the old cap.
    CHECK(app->library_row_at(0).status == RecordStatus::Ready);
    CHECK_FALSE(app->viewed.ready());
}
