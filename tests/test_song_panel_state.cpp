// The song panel's state rules on AppState: next/previous walk the rows the
// library shows and never wrap, the panel's teardown runs on its closing
// edge (from tick(), not from draw code), nothing is locked while idle, and a
// running batch locks the settings as a batch.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "app/config.h"
#include "core/winstr.h"
#include "library_fixtures.h"  // library_entry
#include "scratch_paths.h"
#include "store/record_store.h"
#include "ui/app_state.h"
#include "ui/library_jobs.h"  // set_app_batch_analyzer_for_test
#include "wait_util.h"

using hydra::app::Settings;
using hydra::store::ChartLibraryEntry;
using hydra::store::RecordStore;
using hydra::ui::AppState;

namespace {

std::unique_ptr<AppState> app_with_library(const ScratchPaths& paths, int charts) {
    auto store = std::make_unique<RecordStore>(paths.db);
    std::vector<ChartLibraryEntry> all;
    for (int i = 0; i < charts; ++i) all.push_back(library_entry(i));
    store->rebuild_chart_library(all);
    auto app = std::make_unique<AppState>(Settings{}, std::move(store));
    REQUIRE(app->view_row_count() >= 3);
    return app;
}

}  // namespace

TEST_CASE("song panel: next and previous walk the view and never wrap") {
    ScratchPaths paths("panel_nav");
    auto app = app_with_library(paths, 20);
    app->select(app->view_row(0));
    CHECK(app->details_open());
    CHECK_FALSE(app->can_select_relative(-1));  // first row: nothing before it
    CHECK(app->can_select_relative(1));

    app->select_relative(1);
    CHECK(app->selected->notespath == app->view_row(1).notespath);
    app->select_relative(-1);
    CHECK(app->selected->notespath == app->view_row(0).notespath);
    app->select_relative(-1);  // no wrap to the last row
    CHECK(app->selected->notespath == app->view_row(0).notespath);

    const size_t last = app->view_row_count() - 1;
    app->select(app->view_row(last));
    CHECK_FALSE(app->can_select_relative(1));
    app->select_relative(1);
    CHECK(app->selected->notespath == app->view_row(last).notespath);
}

TEST_CASE("song panel: a song outside the view has no neighbours") {
    ScratchPaths paths("panel_outside");
    auto app = app_with_library(paths, 20);
    app->select(library_entry(999));  // not in the library at all
    CHECK_FALSE(app->can_select_relative(1));
    CHECK_FALSE(app->can_select_relative(-1));
}

TEST_CASE("song panel: tick runs the teardown once, on the closing edge") {
    ScratchPaths paths("panel_edge");
    auto app = app_with_library(paths, 5);
    app->select(app->view_row(0));
    app->tick(0.0);
    app->details_ui.file_checked_at = 1.0;  // as if the file was looked at
    app->tick(0.1);                          // still open: no teardown
    CHECK(app->details_ui.file_checked_at == 1.0);

    app->show_details = false;               // what the X and Escape do
    app->tick(0.2);
    CHECK_FALSE(app->details_open());
    CHECK(app->details_ui.file_checked_at == -1.0);  // close_details ran
}

TEST_CASE("song panel: nothing is locked while idle") {
    ScratchPaths paths("panel_lock");
    auto app = app_with_library(paths, 5);
    CHECK_FALSE(app->settings_locked());
    CHECK(app->settings_lock() == AppState::SettingsLock::None);
    CHECK_FALSE(app->view_running());
    CHECK_FALSE(app->batch_running());
}

// The settings button reads the lock once per frame. It used to ask
// batch_running() a second time; a batch that ended on its worker in between
// sent it to the one-song message, which read the analyze job's song through
// a null pointer (an access violation in hydra_uitest's batch-strip-workers
// under load). Only a batch locks the settings now (D90 item 2).
TEST_CASE("song panel: a running batch locks the settings as a batch, then unlocks") {
    ScratchPaths paths("panel_batchlock");
    std::atomic<bool> release{false};  // before the app, which joins the batch's thread
    auto app = app_with_library(paths, 5);
    hydra::ui::set_app_batch_analyzer_for_test(
        [&release](const std::string&, const hydra::app::AnalysisSettings&,
                   const std::function<void(float)>&) -> hydra::app::AnalysisResult {
            testwait::wait_until([&release] { return release.load(); },
                                 "the test to release the chart");
            throw std::runtime_error("no chart file in this test");
        },
        1);
    app->start_batch(false);
    hydra::ui::set_app_batch_analyzer_for_test(nullptr, 1);
    REQUIRE(app->batch_job != nullptr);
    CHECK(app->settings_lock() == AppState::SettingsLock::Batch);
    CHECK(app->settings_locked());

    release = true;
    testwait::wait_until([&] { return app->batch_job->snapshot().finished; },
                         "the batch to finish");
    CHECK(app->settings_lock() == AppState::SettingsLock::None);
    CHECK_FALSE(app->settings_locked());
}
