// The song panel's state rules on AppState: next/previous walk the rows the
// library shows and never wrap, the panel's teardown runs once on its closing
// edge (from tick(), not from draw code), nothing is locked while idle, and a
// running batch locks the settings as a batch. The library is the corpus
// (testdata/input) as the app's own scan reads it, and the batch analyzes its
// charts with the real analyzer.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"  // discover_charts
#include "app/config.h"
#include "corpus_util.h"  // root
#include "scratch_paths.h"
#include "store/record_store.h"
#include "ui/app_state.h"
#include "ui/library_jobs.h"
#include "wait_util.h"

using hydra::app::Settings;
using hydra::store::RecordStore;
using hydra::ui::AppState;

namespace {

// An AppState whose library is the corpus, scanned by the app's own scan
// (discover_charts) and written through the real store.
std::unique_ptr<AppState> app_with_corpus(const ScratchPaths& paths) {
    auto [items, errors] = hydra::app::discover_charts({corpus::root()});
    REQUIRE(errors.empty());
    auto store = std::make_unique<RecordStore>(paths.db);
    store->rebuild_chart_library(items);
    auto app = std::make_unique<AppState>(Settings{}, std::move(store));
    REQUIRE(app->view_row_count() >= 3);
    return app;
}

// Whether the selected song is one of the rows the library shows now.
bool selected_in_view(const AppState& app) {
    for (size_t i = 0; i < app.view_row_count(); ++i) {
        if (app.view_row(i).notespath == app.selected->notespath) return true;
    }
    return false;
}

}  // namespace

TEST_CASE("song panel: next and previous walk the view and never wrap") {
    ScratchPaths paths("panel_nav");
    auto app = app_with_corpus(paths);
    app->select(app->view_row(0), hydra::ui::SelectionSource::Click);
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
    app->select(app->view_row(last), hydra::ui::SelectionSource::Click);
    CHECK_FALSE(app->can_select_relative(1));
    app->select_relative(1);
    CHECK(app->selected->notespath == app->view_row(last).notespath);
}

// The real way a selected song leaves the view: a search that hides it. The
// search is another corpus song's title, tried from the last row up until one
// shows some rows and not the selected one.
TEST_CASE("song panel: a song outside the view has no neighbours") {
    ScratchPaths paths("panel_outside");
    auto app = app_with_corpus(paths);
    app->select(app->view_row(0), hydra::ui::SelectionSource::Click);
    std::vector<std::string> titles;
    for (size_t i = 1; i < app->view_row_count(); ++i)
        titles.push_back(app->library_row_at(i).title);

    bool hidden = false;
    for (auto it = titles.rbegin(); it != titles.rend() && !hidden; ++it) {
        app->set_search(*it);
        hidden = app->view_row_count() > 0 && !selected_in_view(*app);
    }
    REQUIRE(hidden);
    CHECK_FALSE(app->can_select_relative(1));
    CHECK_FALSE(app->can_select_relative(-1));
}

// file_checked_at is the teardown's mark: close_details sets it to "look
// now", so the next look at the chart file happens at once. The test reads
// that meaning through selected_file_ok, the panel's one look: a fresh look
// at a corpus file, which exists, says true, and a kept answer says what the
// test stored.
TEST_CASE("song panel: tick runs the teardown once, on the closing edge") {
    ScratchPaths paths("panel_edge");
    auto app = app_with_corpus(paths);
    app->select(app->view_row(0), hydra::ui::SelectionSource::Click);
    app->tick(0.0);
    app->details_ui.file_checked_at = 1.0;  // as if the file was looked at
    app->details_ui.file_ok = false;        // and its answer kept
    app->tick(0.1);                         // still open: no teardown
    CHECK(app->details_ui.file_checked_at == 1.0);
    CHECK_FALSE(app->selected_file_ok(1.1));  // the kept answer: no fresh look

    app->show_details = false;  // what the X and Escape do
    app->tick(0.2);
    CHECK_FALSE(app->details_open());
    CHECK(app->selected_file_ok(1.2));  // close_details ran: the file is looked at now

    // Once: the next tick while closed tears nothing down again.
    const double looked_at = app->details_ui.file_checked_at;
    app->details_ui.file_ok = false;
    app->tick(0.3);
    CHECK(app->details_ui.file_checked_at == looked_at);
    CHECK_FALSE(app->selected_file_ok(1.3));
}

TEST_CASE("song panel: nothing is locked while idle") {
    ScratchPaths paths("panel_lock");
    auto app = app_with_corpus(paths);
    CHECK(app->settings_lock() == AppState::SettingsLock::None);
    CHECK_FALSE(app->view_running());
    CHECK_FALSE(app->batch_running());
}

// The settings button reads the lock once per frame, so a batch that ends on
// its worker mid-frame can't change the answer halfway through. Only a batch
// locks the settings (D90 item 2). The batch analyzes the corpus with the
// real analyzer; the Pause button holds it mid-run, since a paused batch
// hands out no more charts and the corpus has more charts than one round of
// workers takes, and the Stop button ends it.
TEST_CASE("song panel: a running batch locks the settings as a batch, then unlocks") {
    ScratchPaths paths("panel_batchlock");
    auto app = app_with_corpus(paths);
    app->start_batch(false);
    REQUIRE(app->batch_job != nullptr);
    app->batch_job->pause();
    CHECK(app->settings_lock() == AppState::SettingsLock::Batch);

    app->batch_job->stop();
    testwait::wait_until([&] { return app->batch_job->snapshot().finished; },
                         "the stopped batch to finish");
    CHECK(app->settings_lock() == AppState::SettingsLock::None);
}
