#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "uitest_harness.h"

#include "../db_file_sql.h"  // tests/ is not on the runner's include path

#include "app/analysis.h"
#include "app/config.h"
#include "app/report_files.h"
#include "core/model.h"
#include "imgui_internal.h"
#include "ui/app_state.h"
#include "ui/library_jobs.h"
#include "ui/library_parts.h"  // analyze_search_label

namespace fs = std::filesystem;

namespace uitest {

namespace {

// A child window of the main window, or null. Child window names are
// mangled, so go through WindowInfo; NoError because "not there" is a result.
ImGuiWindow* child_window(ImGuiTestContext* ctx, const char* path) {
    return ctx->WindowInfo(path, ImGuiTestOpFlags_NoError).Window;
}

// Waits until the finished batch's path report has landed in its slot.
bool wait_report_landed(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    return wait_until(ctx, [&] {
        return h.app->report_started &&
               h.app->path_report_build() != hydra::ui::ReportBuild::Building;
    }, 60);
}

// Narrow the library to charts matching `search` and batch them through the
// confirm; waits for the batch and its report to finish.
bool batch_search(ImGuiTestContext* ctx, const std::string& search) {
    Harness& h = harness(ctx);
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", search.c_str());
    if (!wait_until(ctx, [&] { return h.app->search == search; }, 5)) return false;
    const std::string label = hydra::ui::detail::analyze_search_label(
        static_cast<int64_t>(h.app->library_match_count()));
    ctx->ItemClick(label.c_str());
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Start analyzing");
    return wait_until(ctx, [&] {
               return h.app->batch_job && h.app->batch_job->snapshot().finished;
           }, 300) &&
           wait_report_landed(ctx);
}

// Click the finished strip's X and wait for the strip to go.
void dismiss_done(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchdone") != nullptr; }, 5));
    ctx->SetRef(child_window(ctx, "//Hydra/##batchdone"));
    ctx->ItemClick("X##dismissdone");
    ctx->Yield(2);
    IM_CHECK(h.app->batch_job == nullptr);
    ctx->SetRef("//Hydra");
}

// Pick alice in the open player picker: the box closes, the comparison
// window opens, and the comparison builds (D103 item 8).
void pick_alice(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemClick("**/###111");
    ctx->Yield(2);
    IM_CHECK(!h.app->dm_picker_open);
    IM_CHECK(h.app->dm_report.window_open);
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->dm_report_build() == hydra::ui::ReportBuild::Ready;
    }, 60));
    IM_CHECK(h.app->dm_report.result != nullptr);
}

// The running strip shows the chart being worked on and live counts. Its Stop
// button and its width must not move as they change. The gate steps the run
// one chart at a time, so the strip is provably on screen for every change.
void test_batch_strip_drift(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    BatchGate gate;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Start analyzing");
    IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchstrip") != nullptr; }, 5));
    ctx->Yield(2);  // auto-sized children settle a frame after their content changes
    ImGuiWindow* strip = child_window(ctx, "//Hydra/##batchstrip");
    IM_CHECK(strip != nullptr);
    ctx->SetRef(strip);
    const ImRect stop0 = ctx->ItemInfo("Stop").RectFull;
    const float width0 = strip->Size.x;
    int title_changes = 0;
    std::string last_title = h.app->batch_job->snapshot().current_title;
    for (int step = 1; step <= 6; ++step) {
        // Chart `step` finishes and is counted; the next one starts and is
        // held. Every frame until then (and two after, so the screen shows
        // it), Stop and the strip stay put.
        gate.allow(step);
        // A test limit the user approved (D43), not an app setting.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
        int frames_after = 2;
        while (frames_after > 0) {
            IM_CHECK(std::chrono::steady_clock::now() < deadline);
            ctx->Yield();
            if (gate.started() > step && h.app->batch_job->snapshot().completed >= step)
                --frames_after;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            strip = child_window(ctx, "//Hydra/##batchstrip");
            IM_CHECK(strip != nullptr);  // the gate holds the run open
            ctx->SetRef(strip);
            const ImRect stop = ctx->ItemInfo("Stop").RectFull;
            IM_CHECK_FLOAT_NEAR_EQ(stop.Min.x, stop0.Min.x, 0.01f);
            IM_CHECK_FLOAT_NEAR_EQ(stop.Min.y, stop0.Min.y, 0.01f);
            IM_CHECK_FLOAT_NEAR_EQ(strip->Size.x, width0, 0.01f);
        }
        const std::string t = h.app->batch_job->snapshot().current_title;
        if (t != last_title) { last_title = t; ++title_changes; }
    }
    IM_CHECK(title_changes >= 2);  // the strip really showed different charts
    h.app->batch_job->stop();
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300));
    dismiss_done(ctx);
}

void test_settings_and_reports(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // A view-setting toggle lands in the INI at once.
    bool before = h.app->settings.view_bass2x;
    ctx->ItemClick("**/2x Bass");  // in the settings bar
    ctx->Yield();
    IM_CHECK(h.app->settings.view_bass2x != before);
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).view_bass2x != before);
    ctx->ItemClick("**/2x Bass");  // restore

    // Batch-analyze just the first chart (search narrows the batch), which
    // builds the path report; with auto-open off its window stays shut.
    // In quotes, so the word search matches the title as one phrase.
    const std::string title = "\"" + h.app->library_row_at(0).title + "\"";
    IM_CHECK(batch_search(ctx, title));
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(!h.app->path_report.window_open);
    ctx->SetRef(child_window(ctx, "//Hydra/##batchdone"));
    ctx->ItemClick("Open report");
    IM_CHECK(h.app->path_report.window_open);
    dismiss_done(ctx);

    // dmleaderboards comparison against the canned API: picking the player
    // closes the box and opens the comparison window (D103 item 8).
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Compare with dmleaderboards...");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->dm_users.empty(); }, 10));
    IM_CHECK(visible_text(h).find("alice") != std::string::npos);
    pick_alice(ctx);
    if (ctx->IsError()) return;
    // The canned score (100,000) is under the chart's optimal.
    IM_CHECK_EQ(h.app->dm_report.result->stats.under_optimal, 1);
}

// The dmleaderboards comparison, end to end: the button off away from Clone
// Hero's cap and Expert, the name filter, and every button of the finished
// report.
void test_dm_compare_flow(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");

    // The ladder plays by Clone Hero's rules at Expert. Any other cap or
    // difficulty disables the button.
    auto compare_disabled = [&] {
        return (ctx->ItemInfo("Compare with dmleaderboards...").ItemFlags &
                ImGuiItemFlags_Disabled) != 0;
    };
    h.app->settings.sp_cap = 8;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(compare_disabled());
    h.app->settings.sp_cap = hydra::kCloneHeroSpCap;
    h.app->commit_settings();

    // The difficulty picker lives in the settings bar; the button in the
    // toolbar above it.
    ctx->SetRef(ctx->WindowInfo("//Hydra/##settingsbar").Window);
    ctx->ComboClick("##difficulty/Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK(compare_disabled());
    ctx->SetRef(ctx->WindowInfo("//Hydra/##settingsbar").Window);
    ctx->ComboClick("##difficulty/Expert");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Expert"; }, 5));
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK(!compare_disabled());

    // The picker opens on the canned ladder; the filter narrows it as you
    // type, ignoring case.
    ctx->ItemClick("Compare with dmleaderboards...");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->dm_users.empty(); }, 10));
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemInputValue("##dmfilter", "");  // an earlier test may have left text
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find("alice") != std::string::npos; }, 5));
    ctx->ItemInputValue("##dmfilter", "bob");
    ctx->Yield(2);
    IM_CHECK(visible_text(h).find("alice") == std::string::npos);
    ctx->ItemInputValue("##dmfilter", "ALI");
    ctx->Yield(2);
    IM_CHECK(visible_text(h).find("alice") != std::string::npos);

    // Pick alice, and the choice is remembered (D103 item 8).
    pick_alice(ctx);
    if (ctx->IsError()) return;
    // Nothing is analyzed yet, so the one score is "not analyzed".
    IM_CHECK_EQ(h.app->dm_report.result->stats.not_analyzed, 1);
    IM_CHECK_EQ(h.app->dm_report.result->stats.under_optimal, 0);
    IM_CHECK_STR_EQ(h.app->dm_report.result->username.c_str(), "alice");
    IM_CHECK_STR_EQ(h.app->settings.dm_last_user.c_str(), "111");
    IM_CHECK_STR_EQ(hydra::app::Settings::load_file(h.ini_path).dm_last_user.c_str(), "111");

    // The window's "Compare another player...": back to the list, and the
    // comparison stays until another player replaces it (D103 item 9).
    h.app->reopen_dm_picker();
    ctx->Yield(3);
    IM_CHECK(h.app->dm_picker_open);
    IM_CHECK(h.app->dm_report.result != nullptr);
    IM_CHECK(visible_text(h).find("Pick a player") != std::string::npos);
    ctx->SetRef("//Compare dmleaderboards user");

    // Close: the picker goes away.
    ctx->ItemClick("Close");
    ctx->Yield(2);
    IM_CHECK(!h.app->dm_picker_open);
}

// The path report's buttons: the toolbar's "Open path report" appears once a
// report exists, "Open automatically" persists and then opens the next report
// by itself, and "Also re-analyze" re-analyzes a stored chart.
void test_report_buttons(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    IM_CHECK(!ctx->ItemExists("**/Open path report"));  // no report built yet

    // Batch just the first chart (the search narrows the batch).
    const std::string title = "\"" + h.app->library_row_at(0).title + "\"";
    IM_CHECK(batch_search(ctx, title));
    IM_CHECK(!h.app->path_report.window_open);  // auto-open is off

    // Tick "Open automatically" in the finished strip: it persists at once.
    ctx->SetRef(child_window(ctx, "//Hydra/##batchdone"));
    ctx->ItemClick("Open automatically");
    IM_CHECK(h.app->settings.auto_open_report);
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).auto_open_report);
    dismiss_done(ctx);

    // The toolbar now offers the report, and opens its window on a click.
    ctx->SetRef("//Hydra");
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Open path report"); }, 5));
    ctx->ItemClick("**/Open path report");
    IM_CHECK(h.app->path_report.window_open);
    h.app->path_report.window_open = false;  // the window's X

    // "Also re-analyze" re-analyzes the stored chart, and the confirm says so.
    const std::string label = hydra::ui::detail::analyze_search_label(
        static_cast<int64_t>(h.app->library_match_count()));
    ctx->ItemClick(label.c_str());
    ctx->Yield(2);
    ctx->SetRef("//Analyze library");
    ctx->ItemCheck("Also re-analyze charts that already have a result##redo");
    IM_CHECK(h.app->batch_redo);
    ctx->Yield(2);
    IM_CHECK(visible_text(h).find("Also re-analyze") != std::string::npos);
    IM_CHECK(visible_text(h).find("re-analyzing 1 that already has a result") != std::string::npos);
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(!h.app->batch_confirm_pending);
    IM_CHECK(batch_search(ctx, title));
    IM_CHECK_EQ(h.app->batch_job->snapshot().skipped, 0);  // nothing skipped: redone

    // With auto-open on, the new report opened its window by itself.
    IM_CHECK(h.app->path_report.window_open);
    dismiss_done(ctx);
    h.app->batch_redo = false;
}

// The confirm lists every setting and the real count; Escape backs out,
// Enter starts, and the batch runs in the strip, not a modal.
void test_batch_confirm(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->Yield(2);
    IM_CHECK(h.app->batch_confirm_pending);
    std::string text = visible_text(h);
    IM_CHECK(text.find("Analyze 97 charts that have no result yet?") != std::string::npos);
    IM_CHECK(text.find("Expert \xC2\xB7 Pro Drums \xC2\xB7 2x Bass") != std::string::npos);
    IM_CHECK(text.find("4 bars (Clone Hero's rule)") != std::string::npos);
    IM_CHECK(text.find("2 scores") != std::string::npos);
    IM_CHECK(text.find("10 ms") != std::string::npos);
    ctx->SetRef("//Analyze library");
    // Nothing has a result yet, so there is nothing to re-analyze.
    IM_CHECK((ctx->ItemInfo("Also re-analyze charts that already have a result##redo").ItemFlags &
              ImGuiItemFlags_Disabled) != 0);

    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
    IM_CHECK(!h.app->batch_confirm_pending);
    IM_CHECK(h.app->batch_job == nullptr);

    BatchGate gate;  // hold the run open, or it can end before the strip draws
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->Yield(2);
    ctx->KeyPress(ImGuiKey_Enter);
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job != nullptr; }, 5));
    IM_CHECK(!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId));  // no modal while it runs
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchstrip") != nullptr; }, 10));
    h.app->batch_job->stop();
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300));
    dismiss_done(ctx);
}

// Pause holds the run, Resume carries on, Stop ends it and keeps what's done.
void test_batch_pause_stop(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    // The test library analyzes in a blink, so a real run can finish between
    // two clicks. Hold the first chart at the gate; every click below then
    // lands while the run is provably still going.
    BatchGate gate;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Start analyzing");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->batch_job && !h.app->batch_job->snapshot().preparing;
    }, 30));
    IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchstrip") != nullptr; }, 5));
    ctx->SetRef(child_window(ctx, "//Hydra/##batchstrip"));
    ctx->ItemClick("Pause");
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().paused; }, 5));
    ctx->Yield(2);
    IM_CHECK(ctx->ItemExists("Resume"));
    ctx->ItemClick("Resume");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->batch_job->snapshot().paused; }, 5));
    ctx->Yield(2);
    ctx->ItemClick("Stop");
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300));
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchdone") != nullptr; }, 5));
    IM_CHECK(visible_text(h).find("Stopped:") != std::string::npos);
    ctx->Yield(5);
    IM_CHECK(h.app->report_job == nullptr);  // a stopped run builds no report
    dismiss_done(ctx);
}

// The strip with several workers at once, as the real app runs a batch. Three
// workers each hold a chart at the gate, so three charts run together. The
// strip counts each one as it finishes, Pause stops new charts while the
// running ones finish, Resume refills every worker, and Stop drops the charts
// still running without counting them.
void test_batch_strip_workers(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    constexpr int kWorkers = 3;
    BatchGate gate(kWorkers);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Start analyzing");

    // Frames run for a while with nothing let through. A fourth chart can
    // never reach the gate: every worker is busy. Three workers and a
    // 10-frame, 2 ms settle are test limits the user approved (D43).
    auto settle = [&] {
        for (int i = 0; i < 10; ++i) {
            ctx->Yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    };
    auto strip_text = [&] { return visible_text(h); };
    auto has = [&](const std::string& s) { return strip_text().find(s) != std::string::npos; };

    IM_CHECK(wait_until(ctx, [&] { return gate.started() == kWorkers; }, 30));
    settle();
    IM_CHECK_EQ(gate.started(), kWorkers);
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchstrip") != nullptr; }, 5));
    ctx->Yield(2);
    IM_CHECK(has("Analyzing...  0 of 97"));
    IM_CHECK(has("Now: "));
    ImGuiWindow* strip = child_window(ctx, "//Hydra/##batchstrip");
    IM_CHECK(strip != nullptr);
    ctx->SetRef(strip);
    const ImRect stop0 = ctx->ItemInfo("Stop").RectFull;

    // Two charts finish. Their workers each take the next chart, so three
    // are held again and two are counted.
    gate.allow(2);
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->batch_job->snapshot().completed == 2 && gate.started() == 2 + kWorkers;
    }, 60));
    IM_CHECK(has("Analyzing...  2 of 97"));
    const ImRect stop1 = ctx->ItemInfo("Stop").RectFull;
    IM_CHECK_FLOAT_NEAR_EQ(stop1.Min.x, stop0.Min.x, 0.01f);
    IM_CHECK_FLOAT_NEAR_EQ(stop1.Min.y, stop0.Min.y, 0.01f);

    // Pause with three charts running. They finish and are counted, but no
    // worker starts another.
    ctx->ItemClick("Pause");
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().paused; }, 5));
    gate.allow(2 + kWorkers);
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().completed == 2 + kWorkers; }, 60));
    settle();
    IM_CHECK_EQ(gate.started(), 2 + kWorkers);
    IM_CHECK(has("Paused  5 of 97"));
    IM_CHECK(!has("Now: "));
    IM_CHECK(ctx->ItemExists("Resume"));

    // Resume fills every worker again.
    ctx->ItemClick("Resume");
    IM_CHECK(wait_until(ctx, [&] { return gate.started() == 2 + 2 * kWorkers; }, 30));
    IM_CHECK(has("Analyzing...  5 of 97"));
    IM_CHECK(has("Now: "));

    // Stop: the three held charts are dropped, not counted and not failed.
    ctx->ItemClick("Stop");
    IM_CHECK(wait_until(ctx, [&] { return h.app->batch_job->snapshot().finished; }, 300));
    const hydra::ui::BatchJob::Snapshot done = h.app->batch_job->snapshot();
    IM_CHECK_EQ(done.completed, 2 + kWorkers);
    IM_CHECK_EQ(done.failed, 0);
    IM_CHECK(wait_until(ctx, [&] { return child_window(ctx, "//Hydra/##batchdone") != nullptr; }, 5));
    IM_CHECK(has("Stopped:"));
    dismiss_done(ctx);
}

// The finished strip: the report is ready, Open report opens its window, and
// there is no Show in folder any more (D103).
void test_batch_done_strip(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(batch_search(ctx, "Burnout"));
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(!h.app->path_report.window_open);  // auto-open is off
    ImGuiWindow* done = child_window(ctx, "//Hydra/##batchdone");
    IM_CHECK(done != nullptr);
    IM_CHECK(visible_text(h).find("The path report is ready.") != std::string::npos);
    ctx->SetRef(done);
    IM_CHECK(!ctx->ItemExists("Show in folder"));
    ctx->ItemClick("Open report");
    IM_CHECK(h.app->path_report.window_open);
    ctx->ItemClick("X##dismissdone");
    ctx->Yield(3);
    IM_CHECK(h.app->batch_job == nullptr);
    ctx->SetRef("//Hydra");
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Open path report"); }, 5));
}

// The report no longer goes through a browser, so a browser that refuses
// changes nothing: with Open automatically on, the report's window opens
// and the strip names no browser problem.
void test_batch_open_failure(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    h.app->settings.auto_open_report = true;
    h.app->commit_settings();
    hydra::app::set_open_in_browser([](const std::wstring&) { return false; });
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(batch_search(ctx, "Burnout"));
    IM_CHECK(h.app->path_report_build() == hydra::ui::ReportBuild::Ready);
    IM_CHECK(h.app->path_report.window_open);
    IM_CHECK(visible_text(h).find("The path report is ready.") != std::string::npos);
    IM_CHECK(visible_text(h).find("browser") == std::string::npos);
    // reset_app reinstalls the recording seam for the next test.
}

// News fades; a problem stays until dismissed.
void test_status_line(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    ctx->SetRef("//Hydra");
    h.app->set_status("Saved the thing.");
    ctx->Yield(2);
    IM_CHECK(h.frame_text.text.find("Saved the thing.") != std::string::npos);
    // The fade in the harness's fixed frame time, plus the first frame past
    // its edge.
    const int fade_frames = static_cast<int>(std::ceil(
                                hydra::ui::AppState::kStatusFadeSeconds / ImGui::GetIO().DeltaTime)) +
                            1;
    ctx->Yield(fade_frames);
    IM_CHECK(h.frame_text.text.find("Saved the thing.") == std::string::npos);

    h.app->set_problem("The thing broke.");
    ctx->Yield(fade_frames);
    IM_CHECK(h.frame_text.text.find("The thing broke.") != std::string::npos);
    ctx->ItemClick("**/X##dismissstatus");
    ctx->Yield(2);
    IM_CHECK(h.app->status_message.empty());
}

// D72 item 4: when the store can't say which charts already have a result,
// the Analyze-library click shows the database sentence in the status line,
// opens no confirm, and Hydra keeps running.
void test_analyze_db_fails(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    // A second connection drops the scratch library's results table.
    IM_CHECK_NO_RET(hydra::test::run_sql_on_file(h.db_path, "DROP TABLE results"));

    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->Yield(3);
    const std::string sentence =
        "Hydra couldn't read its database (hydra.db). Check that no other copy of Hydra is "
        "running, then try again.";
    IM_CHECK(h.frame_text.text.find(sentence) != std::string::npos);
    IM_CHECK(h.app->status_is_problem);
    IM_CHECK(!h.app->batch_confirm_pending);
    IM_CHECK(h.app->batch_job == nullptr);
    // Still running: the next frames draw.
    ctx->Yield(3);
    IM_CHECK(h.frame_text.text.find(sentence) != std::string::npos);
}

// The comparison only means something at Clone Hero's cap and Expert; off
// either, the button is disabled (with the reason on hover) instead of
// posting a refusal after the click.
void test_compare_disabled(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    auto disabled = [&] {
        return (ctx->ItemInfo("Compare with dmleaderboards...").ItemFlags &
                ImGuiItemFlags_Disabled) != 0;
    };
    IM_CHECK(!disabled());
    h.app->settings.sp_cap = 6;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(disabled());
    h.app->settings.sp_cap = hydra::kCloneHeroSpCap;
    h.app->settings.view_difficulty = "Hard";
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(disabled());
    h.app->settings.view_difficulty = "Expert";
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(!disabled());
}

// Escape closes Song folders; Enter continues a finished scan.
void test_dialog_keys(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    ctx->SetRef("//Hydra");
    // By ID, not by name: outside a frame there is no current window for
    // ImGui::IsPopupOpen(const char*) to hash the name against.
    const ImGuiID folders_popup = ctx->GetID("//Hydra/Song folders");
    ctx->ItemClick("Manage folders... (1)");
    ctx->Yield(2);
    IM_CHECK(ImGui::IsPopupOpen(folders_popup, ImGuiPopupFlags_AnyPopupLevel));
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
    IM_CHECK(!ImGui::IsPopupOpen(folders_popup, ImGuiPopupFlags_AnyPopupLevel));

    ctx->ItemClick("Scan library");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->scan_job && h.app->scan_job->snapshot().finished;
    }, 60));
    ctx->KeyPress(ImGuiKey_Enter);
    ctx->Yield(2);
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(!h.app->library.rows().empty());
}

// After a whole-library batch, the finished strip's analyzed count and the
// library's Analyzed chip are one number. The batch saves charts in groups
// and counts a chart only once its group is on disk (D86), so no chart the
// strip counted can be missing from the library.
void test_batch_chip_count(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Analyze library...");
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Start analyzing");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->batch_job && h.app->batch_job->snapshot().finished;
    }, 300));
    IM_CHECK(wait_report_landed(ctx));
    const hydra::ui::BatchJob::Snapshot done = h.app->batch_job->snapshot();
    IM_CHECK_GT(done.analyzed, 0);
    ctx->SetRef("//Hydra");
    const std::string chip = "**/Analyzed (" + std::to_string(done.analyzed) + ")##chipdone";
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists(chip.c_str()); }, 10));
    dismiss_done(ctx);
}

}  // namespace

const std::vector<TestEntry>& batch_report_tests() {
    static const std::vector<TestEntry> entries = {
        {"batch-strip-drift", test_batch_strip_drift},
        {"settings-and-reports", test_settings_and_reports},
        {"dm-compare-flow", test_dm_compare_flow},
        {"report-buttons", test_report_buttons},
        {"batch-confirm", test_batch_confirm},
        {"batch-pause-stop", test_batch_pause_stop},
        {"batch-strip-workers", test_batch_strip_workers},
        {"batch-done-strip", test_batch_done_strip},
        {"batch-open-failure", test_batch_open_failure},
        {"status-line", test_status_line},
        {"analyze-db-fails", test_analyze_db_fails},
        {"compare-disabled", test_compare_disabled},
        {"dialog-keys", test_dialog_keys},
        {"batch-chip-count", test_batch_chip_count},
    };
    return entries;
}

}  // namespace uitest
