#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "uitest_harness.h"

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/stars.h"
#include "imgui_internal.h"
#include "ui/app_state.h"
#include "ui/details_view.h"
#include "ui/library_jobs.h"  // set_view_analyzer_for_test
#include "ui/fonts.h"  // px()
#include "ui/library_view.h"  // library_split_width
#include "ui/preview_controller.h"

namespace uitest {

namespace {

// Sets the SP cap in the settings panel, then closes the panel (with Esc,
// which must leave the song panel open) so the song panel can be clicked.
void set_sp_cap(ImGuiTestContext* ctx, int cap) {
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->ItemInputValue("**/##spcap", cap);
    close_settings_panel(ctx);
}

// Ticks or unticks "1.0 fills" the same way.
void set_legacy_fills(ImGuiTestContext* ctx, bool on) {
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    if (on)
        ctx->ItemCheck("**/1.0 fills");
    else
        ctx->ItemUncheck("**/1.0 fills");
    close_settings_panel(ctx);
}

// The status of the library's first row under the settings as they are now
// with the SP cap set to `cap`: what the row would read at that cap.
hydra::store::RecordStatus row0_status_at_cap(Harness& h, int cap) {
    hydra::app::Settings at = h.app->settings;
    at.sp_cap = cap;
    return h.app->store->get_summary(at.record_key(h.app->library_row_at(0).entry.md5)).status;
}

// Waits for the open song's analysis (wait_song_analyzed); true when it
// landed with paths.
bool analyzed_with_paths(ImGuiTestContext* ctx) {
    wait_song_analyzed(ctx);
    return !ctx->IsError() && !harness(ctx).app->viewed.record->paths.empty();
}

// A click analyzes the song and shows its best path; its library row turns
// Ready (D87 items 1 and 2).
void test_analyze(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::NotAnalyzed);
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(!h.app->viewed.record->paths.empty());
    std::string best = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
    // The library row's Best Path cell now shows it too.
    IM_CHECK(h.app->library_row_at(0).status ==
             hydra::store::RecordStatus::Ready);

    // Records are kept per SP cap: switching the cap away from 4 analyzes the
    // open song under the new cap and saves that cap's row (D90 item 1);
    // switching back shows the 4-bar best path again. The INI follows every
    // change. The SP cap box is in the settings panel.
    IM_CHECK(row0_status_at_cap(h, 8) == hydra::store::RecordStatus::NotAnalyzed);
    set_sp_cap(ctx, 8);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == 8; }, 5));
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->viewed.record->sp_cap == 8);
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    IM_CHECK(wait_until(ctx, [&] {
        return hydra::app::Settings::load_file(h.ini_path).sp_cap == 8;
    }, 5));
    set_sp_cap(ctx, 4);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == 4; }, 5));
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->viewed.record->sp_cap == 4);
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    // The headline shows the 4-bar record's best path again.
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
}

// Switching the SP cap swaps the viewed record mid-frame, after the panel
// already chose which path to show. That used to leave the details panel
// reading the freed record (1.5.1 crash: bad_alloc from a garbage vector
// copy, 0xc0000409 on the UI thread). Each switch now analyzes the song again
// (D90 item 1), and the record lands from the click's job on the UI thread.
void test_cap_switch(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(analyzed_with_paths(ctx));
    std::string best4 = h.app->viewed.record->best_path().pathstring();
    set_sp_cap(ctx, 6);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == 6; }, 5));
    IM_CHECK(analyzed_with_paths(ctx));
    std::string best6 = h.app->viewed.record->best_path().pathstring();

    // Flip back and forth; every switch must land on the current record's
    // best path, never on whatever the previous record's memory now holds.
    for (int cap : {4, 6, 4, 6, 4}) {
        set_sp_cap(ctx, cap);
        IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == cap; }, 5));
        IM_CHECK(analyzed_with_paths(ctx));
        IM_CHECK(h.app->viewed.record->sp_cap == cap);
        const std::string& best = cap == 4 ? best4 : best6;
        IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
    }

    // While a cap's analysis is still running, the Preview's SP gauge pins at
    // the Settings cap, the one that analysis runs at, not at 4 (D48, Q24).
    ViewGate gate;
    set_sp_cap(ctx, 5);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == 5; }, 5));
    IM_CHECK(!h.app->viewed.record.has_value());
    set_panel_ref(ctx);
    ctx->ItemClick("**/##DetailsTabs/Preview");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->preview && h.app->preview->active() && !h.app->preview->loading();
    }, 120));
    IM_CHECK(wait_until(ctx, [&] {
        const std::string readout = h.app->preview->sp_meter_readout();
        return readout.size() >= 2 && readout.substr(readout.size() - 2) == "/5";
    }, 60));
}

// "1.0 fills" keys a result like the SP cap does: ticking it analyzes the open
// song again as a 1.0 result and saves that row beside the 1.1 one (D90 item
// 1), and unticking analyzes it as 1.1 again, with the 1.1 best path. The
// leaderboard comparison greys out while it is on.
void test_legacy_fills(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    auto compare_disabled = [&] {
        return (ctx->ItemInfo("//Hydra/Compare with dmleaderboards...").ItemFlags &
                ImGuiItemFlags_Disabled) != 0;
    };
    IM_CHECK(analyzed_with_paths(ctx));
    IM_CHECK(!h.app->viewed.record->legacy_fills);
    const std::string best11 = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(!compare_disabled());
    IM_CHECK(h.app->store->counts().second == 1);

    set_legacy_fills(ctx, true);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.legacy_fills; }, 5));
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).legacy_fills);
    IM_CHECK(compare_disabled());

    IM_CHECK(analyzed_with_paths(ctx));
    IM_CHECK(h.app->viewed.record->legacy_fills);
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    IM_CHECK(h.app->store->counts().second == 2);

    set_legacy_fills(ctx, false);
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.legacy_fills; }, 5));
    IM_CHECK(analyzed_with_paths(ctx));
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    IM_CHECK(!h.app->viewed.record->legacy_fills);
    IM_CHECK(h.app->store->counts().second == 2);  // the 1.1 row was already there
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best11) != std::string::npos; },
                        5));
    IM_CHECK(!compare_disabled());
}

void test_dynamics(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // Search for the chart that the doctest pins dynamics on.
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", "Acid Romance");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "Acid Romance"; }, 5));
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->library_shown_count() > 0 &&
               h.app->library_row_at(0).title == "Acid Romance";
    }, 5));
    open_details(ctx, 0);
    if (ctx->IsError()) return;

    // Click the Dynamics tab and wait for the click's job to count them.
    ctx->ItemClick("##DetailsTabs/Dynamics");
    IM_CHECK(wait_until(ctx, [&] {
        return !h.app->view_job && h.app->viewed.dynamics.has_value();
    }, 60));

    std::string text = visible_text(h);
    IM_CHECK(text.find("Dynamics enabled: yes") != std::string::npos);
    IM_CHECK(text.find("2x kicks:") != std::string::npos);
    // The doctest pins 5 ghosts for this chart (all from the red snare).
    IM_CHECK(text.find("Ghosts: 5") != std::string::npos);

    // Toggle 2x Bass off via app state: the song is analyzed again under it
    // (D90 item 1), and the Dynamics tab shows the 2x kicks as not counted.
    h.app->settings.view_bass2x = false;
    h.app->commit_settings();
    ctx->Yield(2);
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("not counted (2x Bass off)") != std::string::npos;
    }, 5));

    // Restore.
    h.app->settings.view_bass2x = true;
    h.app->commit_settings();
}

// The Dynamics count comes from the click, never the store (D87 item 1): the
// first open counts the chart, and a second open counts it again from the
// file and shows the same numbers. With 2x Bass on, the count is ready the
// moment the click's analysis is, from that same job.
void test_dynamics_reopen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);

    // ---- Scenario 1: count, then re-open and count again ----
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // Open Acid Romance on its Dynamics tab, wait for the click's job to
    // count it, and check its 5 ghosts. The search goes through app state to
    // avoid the ImGui input-buffer residue from the previous dynamics test.
    auto open_and_count = [&] {
        h.app->set_search("Acid Romance");
        ctx->Yield(2);
        IM_CHECK(h.app->library_shown_count() > 0);
        IM_CHECK(h.app->library_row_at(0).title == "Acid Romance");
        open_details(ctx, 0);
        if (ctx->IsError()) return;
        ctx->ItemClick("##DetailsTabs/Dynamics");
        IM_CHECK(wait_until(ctx, [&] {
            return !h.app->view_job && h.app->viewed.dynamics.has_value();
        }, 60));
        IM_CHECK(visible_text(h).find("Ghosts: 5") != std::string::npos);
    };
    open_and_count();
    if (ctx->IsError()) return;

    // Select a different chart so Acid Romance's count is dropped, then
    // close the panel so the tab stops rendering.
    h.app->set_search("");
    size_t other_idx = 0;
    for (size_t i = 0; i < h.app->library_shown_count(); ++i) {
        if (h.app->library_row_at(i).title != "Acid Romance") {
            other_idx = i;
            break;
        }
    }
    h.app->select(h.app->library_row_at(other_idx).entry);
    h.app->show_details = false;
    ctx->Yield(3);

    // Reopen Acid Romance. The click counts it again from the file.
    open_and_count();
    if (ctx->IsError()) return;

    // ---- Scenario 2: the click's analysis and its count land together ----
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // Open any chart (first row); the click analyzes it with 2x Bass on (the
    // default).
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->settings.effective_bass2x());
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;

    // Now click the Dynamics tab: the counts are there already, from the
    // same job, with nothing left running.
    ctx->ItemClick("##DetailsTabs/Dynamics");
    ctx->Yield(3);
    IM_CHECK(h.app->viewed.dynamics.has_value());
    IM_CHECK(!h.app->view_job);
    // The counts are on screen (the first chart has notes, so "All" > 0).
    std::string text = visible_text(h);
    IM_CHECK(text.find("Ghosts:") != std::string::npos ||
             text.find("Accents:") != std::string::npos);
}

// At Hard the Dynamics tab counts Hard's own kicks (findings 10, 12, 255).
// Pathfinder - When The Sunrise Breaks The Darkness has 1,613 Expert 2x kicks
// (pitch 95) and no Hard ones; 139 of its 972 Hard kicks share a tick with a
// 95. Before D20 the tab showed 833 kicks, a phantom row of 1,613 "2x kicks"
// and Totals of 3,287.
void test_dynamics_hard(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    h.app->settings.view_difficulty = "Hard";
    h.app->commit_settings();
    IM_CHECK(h.app->settings.effective_bass2x());  // the default, now at Hard too
    open_titled(ctx, "Sunrise Breaks", "When The Sunrise Breaks The Darkness");
    if (ctx->IsError()) return;

    ctx->ItemClick("##DetailsTabs/Dynamics");
    IM_CHECK(wait_until(ctx, [&] {
        return !h.app->view_job && h.app->viewed.dynamics.has_value();
    }, 60));
    std::string text = visible_text(h);
    IM_CHECK(text.find("2x kicks: 0 of 972 kick notes (0%)") != std::string::npos);
    IM_CHECK(text.find(" of 3,426 (") != std::string::npos);  // Dynamic notes: X of 3,426

    // 2x Bass off: Totals is unchanged, because Hard has no 2x kicks to drop.
    h.app->settings.view_bass2x = false;
    h.app->commit_settings();
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("not counted (2x Bass off)") != std::string::npos;
    }, 5));
    IM_CHECK(visible_text(h).find(" of 3,426 (") != std::string::npos);

    // Restore the defaults the next test relies on.
    h.app->settings.view_bass2x = true;
    h.app->settings.view_difficulty = "Expert";
    h.app->commit_settings();
}

// The Stars tab: no cutoffs while the click's analysis runs, then the base
// score, the solo bonus and the seven cutoffs from star_cutoffs(). "87" has a
// drum solo; "I'm A Believer" has a solo only on guitar, so its drums show
// none.
void test_stars(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // ---- A song with a drum solo ----
    {
        ViewGate gate;  // holds the click's analysis while the tab is read
        open_titled(ctx, "Polyphia", "87");
        if (ctx->IsError()) return;
        ctx->ItemClick("##DetailsTabs/Stars");
        IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
        ctx->Yield(2);
        IM_CHECK(h.app->viewed.state == hydra::ui::ViewedSong::State::Analyzing);
        IM_CHECK(visible_text(h).find("Base score") == std::string::npos);
    }

    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("##DetailsTabs/Stars");
    ctx->Yield(3);

    hydra::StarCutoffs sc = hydra::star_cutoffs(h.app->viewed.record->best_path());
    IM_CHECK(sc.solo_bonus > 0);
    std::string text = visible_text(h);
    IM_CHECK(text.find("Base score: " + hydra::group_thousands(sc.base)) != std::string::npos);
    IM_CHECK(text.find("Solo bonus: " + hydra::group_thousands(sc.solo_bonus) +
                       " (not counted toward stars)") != std::string::npos);
    IM_CHECK(text.find("With full solo bonus") != std::string::npos);
    for (int64_t cutoff : sc.cutoffs)
        IM_CHECK(text.find(hydra::group_thousands(cutoff)) != std::string::npos);
    for (int64_t with_solo : sc.with_solo)
        IM_CHECK(text.find(hydra::group_thousands(with_solo)) != std::string::npos);
    IM_CHECK(text.find("4.4") != std::string::npos);

    // ---- A song with no drum solo ----
    h.app->show_details = false;
    ctx->Yield(3);
    open_titled(ctx, "Believer", "I'm A Believer (The Monkees cover)");
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("##DetailsTabs/Stars");
    ctx->Yield(3);

    sc = hydra::star_cutoffs(h.app->viewed.record->best_path());
    IM_CHECK(sc.solo_bonus == 0);
    text = visible_text(h);
    IM_CHECK(text.find("Base score: " + hydra::group_thousands(sc.base)) != std::string::npos);
    IM_CHECK(text.find(hydra::group_thousands(sc.cutoffs[hydra::kMaxStars - 1])) !=
             std::string::npos);
    IM_CHECK(text.find("Solo bonus") == std::string::npos);
    IM_CHECK(text.find("With full solo bonus") == std::string::npos);
}

// Hiding the details panel by any route tears it down. The "Rescan library"
// button used to set show_details = false directly, which skipped the
// teardown: the Preview kept its audio device and kept playing.
void test_details_close_teardown(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (!open_preview(ctx)) return;
    ctx->ItemClick("**/Play");
    IM_CHECK(h.app->preview->playing());

    // Exactly what the Rescan library button does. The button itself only
    // shows when the chart file is missing, which never happens here.
    h.app->request_scan = true;
    h.app->show_details = false;
    ctx->Yield(3);
    IM_CHECK(!h.app->preview->active());
    IM_CHECK(!h.app->preview->playing());

    // The rescan the button asked for runs; finish it so the app is idle.
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->scan_job && h.app->scan_job->snapshot().finished;
    }, 60));
    ctx->SetRef("//Scanning charts");
    ctx->ItemClick("Continue");
    ctx->Yield(2);
}

// The Preview's loading bar while a real load runs: every frame's label
// starts with one of the four step names, and the bar never moves backwards.
// The test charts load in a few frames, so this sees few polls; the bar's
// arithmetic itself is pinned in tests/test_preview_load_progress.cpp.
void test_preview_load_bar(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->ItemClick("**/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    static const char* const kSteps[] = {"Reading chart", "Opening audio", "Building scene",
                                         "Building highway"};
    int polls = 0;
    int bad_labels = 0;
    int backwards = 0;
    float prev = 0.0f;
    std::string first_bad;
    IM_CHECK(wait_until(ctx, [&] {
        if (!h.app->preview->loading()) return true;
        const hydra::ui::PreviewController::LoadProgress lp = h.app->preview->load_progress();
        ++polls;
        bool named = false;
        for (const char* s : kSteps) named = named || lp.label.rfind(s, 0) == 0;
        if (!named) {
            ++bad_labels;
            if (first_bad.empty()) first_bad = lp.label;
        }
        if (lp.fraction < prev) ++backwards;
        prev = lp.fraction;
        return false;
    }, 120));
    ctx->LogInfo("load bar polled %d times; first bad label '%s'", polls, first_bad.c_str());
    IM_CHECK_EQ(bad_labels, 0);
    IM_CHECK_EQ(backwards, 0);
    IM_CHECK_STR_EQ(h.app->preview->error().c_str(), "");
}

// The panel docks beside the library: opening it narrows the library, and
// the X and Escape both close it and give the library its width back.
void test_panel_open_close(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    const float main_w = ctx->GetWindowByRef("//Hydra")->Size.x;
    auto library_w = [&] { return ctx->WindowInfo("//Hydra/##library").Window->Size.x; };
    IM_CHECK(library_w() > main_w * 0.9f);

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(library_w() < main_w * 0.7f);
    IM_CHECK(ctx->WindowInfo("//Hydra/##songpanel").Window != nullptr);
    IM_CHECK(!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId));  // no modal any more

    ctx->ItemClick("X##closepanel");
    ctx->Yield(3);
    IM_CHECK(!h.app->details_open());
    IM_CHECK(library_w() > main_w * 0.9f);

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(3);
    IM_CHECK(!h.app->details_open());
}

// The split beside the panel. A drag of the library's edge is the only thing
// that changes it: it survives closing and reopening the panel, and a
// hydra_ui.ini from before the fix, whose library entry holds the largest
// split, loses to the [Hydra][Layout] share.
void test_panel_split(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    auto library = [&] { return ctx->WindowInfo("//Hydra/##library").Window; };
    auto panel = [&] { return ctx->WindowInfo("//Hydra/##songpanel").Window; };
    auto close = [&] {
        ctx->SetRef("//Hydra");
        ctx->ItemClick("**/X##closepanel");
        ctx->Yield(3);
    };
    auto open = [&] {
        open_details(ctx, 0);
        ctx->Yield(2);
    };

    // Opening: the default share, clamped so the panel keeps its minimum.
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    const float room = library()->Size.x + panel()->Size.x;
    const float min_panel = hydra::ui::px(hydra::ui::kMinSongPanelW);
    const float opened = hydra::ui::library_split_width(room, hydra::ui::kDefaultLibraryShare);
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, opened, 1.0f);
    IM_CHECK_GE(panel()->Size.x, min_panel - 1.0f);

    // Drag the library's right edge 80 px left.
    ImGuiWindow* lib = library();
    const ImVec2 edge(lib->Pos.x + lib->Size.x, lib->Pos.y + lib->Size.y * 0.5f);
    ctx->MouseMoveToPos(edge);
    ctx->MouseDown(ImGuiMouseButton_Left);
    ctx->MouseMoveToPos(ImVec2(edge.x - 80.0f, edge.y));
    ctx->MouseUp(ImGuiMouseButton_Left);
    ctx->Yield(2);
    const float dragged = library()->Size.x;
    IM_CHECK_FLOAT_NEAR_EQ(dragged, opened - 80.0f, 2.0f);
    IM_CHECK_FLOAT_NEAR_EQ(hydra::ui::library_share(), dragged / room, 0.002f);

    // Closing gives the library the full width; reopening puts the drag back.
    close();
    if (ctx->IsError()) return;  // WindowInfo answers null once a test has failed
    IM_CHECK(library()->Size.x > room * 0.9f);
    open();
    if (ctx->IsError()) return;
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, dragged, 1.0f);

    // An old hydra_ui.ini, read while no song is open (as at startup): the
    // library child's own entry at a huge width (what the bug saved) and a
    // share. The share wins.
    close();
    if (ctx->IsError()) return;
    const std::string old_ini = std::string("[Window][") + library()->Name +
                                "]\nSize=4000,600\n[Hydra][Layout]\nLibraryShare=0.3000\n";
    ImGui::LoadIniSettingsFromMemory(old_ini.c_str(), old_ini.size());
    IM_CHECK_FLOAT_NEAR_EQ(hydra::ui::library_share(), 0.3f, 0.0001f);
    open();
    if (ctx->IsError()) return;
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, hydra::ui::library_split_width(room, 0.3f), 1.0f);
}

// "Hide library" gives the song panel the whole width and flips to "Show
// library"; < and > still step through the list; closing the panel shows the
// library; the next song opens with it hidden again (it is remembered); "Show
// library" puts it back at its share.
void test_panel_hide_library(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    auto library = [&] { return ctx->WindowInfo("//Hydra/##library").Window; };
    auto panel = [&] { return ctx->WindowInfo("//Hydra/##songpanel").Window; };
    auto on_screen = [&](const char* s) { return visible_text(h).find(s) != std::string::npos; };

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    const float room = library()->Size.x + panel()->Size.x;
    const float shared = library()->Size.x;
    IM_CHECK(on_screen("Hide library"));
    IM_CHECK(!hydra::ui::library_hidden());

    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Hide library");
    ctx->Yield(3);
    IM_CHECK(hydra::ui::library_hidden());
    IM_CHECK(on_screen("Show library"));
    IM_CHECK(!on_screen("Hide library"));
    IM_CHECK_FLOAT_NEAR_EQ(panel()->Size.x, room, 1.0f);
    IM_CHECK(!on_screen("Library"));  // the pane's heading is gone

    // Next song still works with the library hidden.
    const std::string first = h.app->selected->notespath;
    ctx->ItemClick("**/>##nextsong");
    ctx->Yield(2);
    IM_CHECK(h.app->selected->notespath != first);
    IM_CHECK_FLOAT_NEAR_EQ(panel()->Size.x, room, 1.0f);

    // Closing shows the library full width; the next song opens hidden again.
    ctx->ItemClick("**/X##closepanel");
    ctx->Yield(3);
    if (ctx->IsError()) return;
    IM_CHECK(library()->Size.x > room * 0.9f);
    IM_CHECK(hydra::ui::library_hidden());
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    IM_CHECK_FLOAT_NEAR_EQ(panel()->Size.x, room, 1.0f);

    // Show library: back at its share, button reads Hide again.
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Show library");
    ctx->Yield(3);
    IM_CHECK(!hydra::ui::library_hidden());
    IM_CHECK(on_screen("Hide library"));
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, shared, 1.0f);
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x + panel()->Size.x, room, 1.0f);
}

// Every window on screen whose content is wider than its room (and that has
// no horizontal scroll bar to reach the rest): text or buttons running past
// an edge. Tooltips and ImGui's own debug window aside.
void overflowing_windows(const std::string& screen, std::vector<std::string>& found) {
    for (ImGuiWindow* w : ImGui::GetCurrentContext()->Windows) {
        if (!w->WasActive || (w->Flags & ImGuiWindowFlags_Tooltip) ||
            (w->Flags & ImGuiWindowFlags_HorizontalScrollbar) ||
            std::strncmp(w->Name, "Debug##", 7) == 0)
            continue;
        const float room = w->ContentRegionRect.GetWidth();
        if (w->ContentSize.x > room + 0.5f) {
            char line[512];
            std::snprintf(line, sizeof(line), "%s: %s content %.0f px, room %.0f px",
                          screen.c_str(), w->Name, w->ContentSize.x, room);
            found.push_back(line);
        }
    }
}

// Walks the song panel's four tabs on charts with long titles and long paths,
// at the narrowest and the widest panel, and fails naming every window whose
// content runs past its edge.
void test_layout_sweep(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    std::vector<std::string> found;
    ctx->Yield(2);
    overflowing_windows("library", found);

    struct Chart {
        const char* search;
        const char* title;
    };
    const Chart charts[] = {
        {"burnout", "Burnout"},
        {"spiraling void", "The Spiraling Void"},
        {"tapestry", "Tapestry of the Starless Abstract (Shortened)"},
        // A MIDI chart without [ENABLE_CHART_DYNAMICS]: the Dynamics tab's
        // right box carries its longest line.
        {"themata", "Themata"},
    };
    for (const Chart& c : charts) {
        open_titled(ctx, c.search, c.title);
        if (ctx->IsError()) return;
        wait_song_analyzed(ctx);
        if (ctx->IsError()) return;
        for (float share : {0.99f, 0.01f}) {
            hydra::ui::remember_library_share(share);
            h.app->library_ui.panel_was_open = false;
            ctx->Yield(3);
            const std::string at = std::string(c.title) + (share > 0.5f ? " narrow " : " wide ");
            set_panel_ref(ctx);
            ctx->ItemClick("**/##DetailsTabs/Paths");
            ctx->Yield(2);
            if (ctx->ItemExists("**/Expand all")) ctx->ItemClick("**/Expand all");
            ctx->Yield(2);
            overflowing_windows(at + "Paths", found);
            ctx->ItemClick("**/##DetailsTabs/Preview");
            wait_until(ctx, [&] { return h.app->preview && h.app->preview->active() &&
                                         !h.app->preview->loading(); }, 120);
            ctx->Yield(3);
            overflowing_windows(at + "Preview", found);
            ctx->ItemClick("**/##DetailsTabs/Dynamics");
            ctx->Yield(10);
            if (std::strcmp(c.title, "Themata") == 0)
                IM_CHECK(wait_until(ctx, [&] {
                    return visible_text(h).find(
                               "Dynamics enabled: no (markings ignored by Clone Hero)") !=
                           std::string::npos;
                }, 60));
            overflowing_windows(at + "Dynamics", found);
            ctx->ItemClick("**/##DetailsTabs/Stars");
            ctx->Yield(3);
            overflowing_windows(at + "Stars", found);
            if (ctx->IsError()) return;
        }
    }
    for (const std::string& f : found) std::fprintf(stderr, "OVERFLOW %s\n", f.c_str());
    IM_CHECK_EQ(found.size(), (size_t)0);
}

// The click's analysis fails with a long path in its error, through the
// test seam: the engine has no input the harness can give that fails this way.
struct FailingAnalyzer {
    explicit FailingAnalyzer(std::string what) {
        hydra::ui::set_view_analyzer_for_test(
            [what](const std::string&, const hydra::app::AnalysisSettings&,
                   const std::function<void(float)>&) -> hydra::app::AnalysisResult {
                throw std::runtime_error(what);
            });
    }
    ~FailingAnalyzer() { hydra::ui::set_view_analyzer_for_test(nullptr); }
};

// A long error message wraps inside its window instead of running past the
// panel's edge. The click's error on the Paths tab is the one the harness
// reaches: its plain sentence leads, and the raw text, with a long file path
// in it, goes on the dimmed line under it.
void test_long_error_wraps(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    std::string long_path = "C:\\Songs";
    for (int i = 0; i < 8; ++i)
        long_path += "\\A Rather Long Folder Name Kept For Testing " + std::to_string(i);
    long_path += "\\notes.chart";
    FailingAnalyzer failing("could not read " + long_path);
    open_titled(ctx, "burnout", "Burnout");
    if (ctx->IsError()) return;
    hydra::ui::remember_library_share(0.99f);  // the narrowest panel
    h.app->library_ui.panel_was_open = false;
    ctx->Yield(3);
    set_panel_ref(ctx);
    ctx->ItemClick("**/##DetailsTabs/Paths");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->viewed.state == hydra::ui::ViewedSong::State::Failed;
    }, 30));
    ctx->Yield(3);
    // The sentence leads; the long path is on the dimmed details line under it.
    IM_CHECK(!h.app->viewed.message.empty());
    IM_CHECK(visible_text(h).find(h.app->viewed.message) != std::string::npos);
    const std::string line = h.app->viewed.error;
    IM_CHECK(line.find(long_path) != std::string::npos);
    IM_CHECK(visible_text(h).find(line) != std::string::npos);
    IM_CHECK(ctx->ItemExists("**/Continue"));
    // Unwrapped, the line would be wider than the whole screen.
    IM_CHECK_GT(ImGui::CalcTextSize(line.c_str()).x, ImGui::GetIO().DisplaySize.x);

    // Only the song panel and its children: the rest of the window is
    // another task's (layout-sweep covers it).
    std::vector<std::string> all, found;
    overflowing_windows("long error", all);
    for (const std::string& f : all)
        if (f.find("##songpanel") != std::string::npos) found.push_back(f);
    for (const std::string& f : found) std::fprintf(stderr, "OVERFLOW %s\n", f.c_str());
    IM_CHECK_EQ(found.size(), (size_t)0);
}

// Previous / next step through the library's rows and stop at the ends.
void test_panel_prev_next(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK((ctx->ItemInfo("<##prevsong").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    ctx->ItemClick(">##nextsong");
    ctx->Yield(2);
    IM_CHECK(h.app->selected->notespath == h.app->view_row(1).notespath);
    IM_CHECK(h.app->details_open());
    ctx->ItemClick("<##prevsong");
    ctx->Yield(2);
    IM_CHECK(h.app->selected->notespath == h.app->view_row(0).notespath);
}

// The panel's top on Burnout (Green Day, charted by Hoph2o), before and after
// analysis, at the scratch settings (Expert, Pro Drums, 2x Bass, cap 4,
// 2 scores, 10 ms).
void test_panel_headline(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    std::string text;
    {
        ViewGate gate;  // holds the click's analysis: the top before it lands
        open_titled(ctx, "Burnout", "Burnout");
        if (ctx->IsError()) return;
        IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
        text = visible_text(h);
        IM_CHECK(text.find("Green Day \xC2\xB7 charted by Hoph2o") != std::string::npos);
        IM_CHECK(text.find("378,315") == std::string::npos);  // no headline yet
        // An artist made only of Clone Hero tags reads "(unknown)", as a
        // title does (D50 item 5).
        h.app->selected->artist = "<color=#FF8000></color><b></b>";
        ctx->Yield(2);
        IM_CHECK(visible_text(h).find("(unknown) \xC2\xB7 charted by Hoph2o") !=
                 std::string::npos);
        h.app->selected->artist = "Green Day";
        ctx->Yield(2);
    }

    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("378,315") != std::string::npos;
    }, 5));
    text = visible_text(h);
    IM_CHECK(text.find("3- 1 2") != std::string::npos);
    IM_CHECK(text.find("Optimal path \xC2\xB7 7 stars") != std::string::npos);
    // The hardest timing sits beside each path in the list, not in the headline.
    IM_CHECK(text.find("hardest squeeze") == std::string::npos);
    IM_CHECK(text.find("163.0 ms") != std::string::npos);
    // The click's record is the engine's own, so the panel never calls it out
    // of date; only the library's tooltip can (D87 item 6).
    IM_CHECK(text.find("Out of date") == std::string::npos);
}

// Closing the panel mid-analysis cancels it, and nothing is saved for the
// song: its row stays Not analyzed (D87 item 11, ruling 13).
void test_panel_close_cancels(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ViewGate gate;  // the analysis is provably still running at the close
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
    ctx->ItemClick("X##closepanel");
    ctx->Yield(2);
    IM_CHECK(!h.app->details_open());
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, 30));
    IM_CHECK(h.app->viewed.state == hydra::ui::ViewedSong::State::None);
    IM_CHECK(h.app->view_row_status(0) == hydra::store::RecordStatus::NotAnalyzed);
    IM_CHECK(h.app->store->counts().second == 0);
}

// Cancel shows "Analysis cancelled." and "Try again", and saves nothing; Try
// again analyzes the song and shows its paths (D87 item 11).
void test_view_cancel_try_again(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ViewGate gate;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("Analyzing chart") != std::string::npos;
    }, 5));
    ctx->ItemClick("**/Cancel");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->viewed.state == hydra::ui::ViewedSong::State::Cancelled;
    }, 30));
    IM_CHECK(visible_text(h).find("Analysis cancelled.") != std::string::npos);
    IM_CHECK(visible_text(h).find("Analyzing chart") == std::string::npos);
    IM_CHECK(ctx->ItemExists("**/Try again"));
    IM_CHECK(h.app->view_row_status(0) == hydra::store::RecordStatus::NotAnalyzed);
    IM_CHECK(h.app->store->counts().second == 0);

    gate.open();
    ctx->ItemClick("**/Try again");
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    const std::string best = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
    IM_CHECK(visible_text(h).find("Analysis cancelled.") == std::string::npos);
    IM_CHECK(h.app->view_row_status(0) == hydra::store::RecordStatus::Ready);
}

// With a bad hydra_rules.ini, the click shows only today's "Analysis is
// off..." line on the Paths tab, and the Dynamics tab still shows its counts,
// which need only the parse (D87 item 9).
void test_view_rules_broken(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h, "max_tied_paths = 0\n");
    IM_CHECK(h.app->analysis_blocked());
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_titled(ctx, "Acid Romance", "Acid Romance");
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->viewed.state == hydra::ui::ViewedSong::State::RulesBroken;
    }, 60));
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(3);
    std::string text = visible_text(h);
    IM_CHECK(text.find("Analysis is off until hydra_rules.ini is fixed and Hydra is restarted.") !=
             std::string::npos);
    IM_CHECK(!h.app->viewed.record.has_value());
    IM_CHECK(!ctx->ItemExists("**/##path0"));
    IM_CHECK(!ctx->ItemExists("**/Copy path"));

    ctx->ItemClick("##DetailsTabs/Dynamics");
    ctx->Yield(3);
    IM_CHECK(h.app->viewed.dynamics.has_value());
    text = visible_text(h);
    IM_CHECK(text.find("Dynamics enabled: yes") != std::string::npos);
    IM_CHECK(text.find("Ghosts: 5") != std::string::npos);
    IM_CHECK(h.app->store->counts().second == 0);  // nothing saved
}

// The panel has no Analyze or Re-analyze button in any state: a click
// analyzes (D87 item 6).
void test_no_analyze_button(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    auto no_button = [&] {
        const std::string text = visible_text(h);
        return !ctx->ItemExists("**/Analyze this song") && !ctx->ItemExists("**/Re-analyze") &&
               text.find("Analyze this song") == std::string::npos &&
               text.find("Re-analyze") == std::string::npos &&
               text.find("Not analyzed yet.") == std::string::npos;
    };
    {
        ViewGate gate;  // while the click analyzes
        open_details(ctx, 0);
        if (ctx->IsError()) return;
        IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
        IM_CHECK(no_button());
    }
    wait_song_analyzed(ctx);  // once it is Ready
    if (ctx->IsError()) return;
    IM_CHECK(no_button());
    ctx->ItemClick("##DetailsTabs/Stars");
    ctx->Yield(2);
    IM_CHECK(no_button());
}

// The progress box shows only once the click has run
// kViewProgressDelaySeconds, so a chart that finishes sooner shows its paths
// with no box (D87 items 6 and 10). AppState::view_progress_shown owns that
// rule, and its two legs are pinned apart here with held clicks, so no check
// depends on how fast the machine is. The "no box before the delay" checks
// cover the clock leg; the "no box once it finished" checks cover the
// running leg. A fast chart's every frame falls under one or the other. The
// second click comes long after the first one's delay has passed, so it pins
// that each request restarts the clock.
void test_view_progress_delay(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    using clock = std::chrono::steady_clock;
    const double delay = hydra::ui::AppState::kViewProgressDelaySeconds;
    auto box_on_screen = [&] {
        return visible_text(h).find("Analyzing chart") != std::string::npos;
    };
    // Request library row `row` the way a row click does, then return the
    // seconds to the first frame that shows the box, or -1 if it never
    // shows. It watches from the first frame after the request, and t0 is
    // taken before the request, so this can only overstate the wait.
    auto click_and_time_box = [&](size_t row) {
        const clock::time_point t0 = clock::now();
        h.app->select(h.app->view_row(row));
        double first_box = -1.0;
        wait_until(ctx, [&] {
            if (box_on_screen() && first_box < 0.0)
                first_box = std::chrono::duration<double>(clock::now() - t0).count();
            return first_box >= 0.0;
        }, 10);
        return first_box;
    };

    // ---- A held click: no box before the delay, then the box ----
    ctx->SetRef("//Hydra");
    {
        ViewGate gate;
        IM_CHECK_GE(click_and_time_box(0), delay);
        IM_CHECK(ctx->ItemExists("**/Cancel"));
    }
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(!box_on_screen());

    // ---- A second held click: the clock starts again ----
    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/X##closepanel");
    ctx->Yield(3);
    {
        ViewGate gate;
        IM_CHECK_GE(click_and_time_box(1), delay);
    }
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(!box_on_screen());
}

// A burst of setting changes with the song open (a held +/- box) ends with
// one analysis, of the final settings: the caps in between are never saved,
// and the panel shows the final cap's paths (D90, ruling 12).
void test_view_setting_burst(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(analyzed_with_paths(ctx));
    if (ctx->IsError()) return;
    const int start_cap = h.app->settings.sp_cap;
    const int last_cap = start_cap + 4;
    {
        ViewGate gate;  // no step's analysis can finish before the burst ends
        for (int cap = start_cap + 1; cap <= last_cap; ++cap) {
            set_sp_cap(ctx, cap);
            IM_CHECK(wait_until(ctx, [&] { return h.app->settings.sp_cap == cap; }, 5));
            IM_CHECK(h.app->view_running());
        }
    }
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->viewed.record->sp_cap == last_cap);
    for (int cap = start_cap + 1; cap < last_cap; ++cap)
        IM_CHECK(row0_status_at_cap(h, cap) == hydra::store::RecordStatus::NotAnalyzed);
    IM_CHECK(row0_status_at_cap(h, last_cap) == hydra::store::RecordStatus::Ready);
    IM_CHECK(h.app->store->counts().second == 2);  // the first click's and the last cap's
    const std::string best = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
}

// The settings lock while a batch runs and unlock when it stops. Locked, the
// panel still opens with every control greyed and says why, and the button's
// label ends in " (locked)".
void test_settings_lock(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    auto shows = [&](const char* text) { return visible_text(h).find(text) != std::string::npos; };
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    close_settings_panel(ctx);

    // A click's analysis doesn't lock the settings (D90 item 2).
    {
        ViewGate view_gate;
        open_details(ctx, 0);
        if (ctx->IsError()) return;
        IM_CHECK(wait_until(ctx, [&] { return view_gate.started() >= 1; }, 30));
        IM_CHECK(h.app->view_running());
        IM_CHECK(h.app->settings_lock() == hydra::ui::AppState::SettingsLock::None);
        open_settings_panel(ctx);
        if (ctx->IsError()) return;
        IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) == 0);
        IM_CHECK((ctx->ItemInfo("**/Pro Drums").ItemFlags & ImGuiItemFlags_Disabled) == 0);
        IM_CHECK(!shows("Stop the batch to change these."));
        IM_CHECK(!shows("(locked)"));
        close_settings_panel(ctx);
        ctx->ItemClick("**/X##closepanel");
        ctx->Yield(3);
    }
    IM_CHECK(wait_until(ctx, [&] { return !jobs_busy(h); }, 60));

    // The whole library still analyzes in a blink, so hold its first chart at
    // the gate: the checks below then look at a run that is provably going.
    BatchGate gate;
    h.app->start_batch(false);
    IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
    IM_CHECK(h.app->batch_running());
    IM_CHECK(wait_until(ctx, [&] { return shows("(locked)"); }, 5));
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return shows("Stop the batch to change these."); }, 5));
    IM_CHECK((ctx->ItemInfo(settings_control(ctx, "Pro Drums", "##difficulty")).ItemFlags &
              ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/Pro Drums").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/Note Shuffle").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/Path limit##mslimit").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    close_settings_panel(ctx);

    h.app->batch_job->stop();
    IM_CHECK(wait_until(ctx, [&] { return !h.app->batch_running(); }, 300));
    IM_CHECK(wait_until(ctx, [&] { return !jobs_busy(h); }, 120));
    IM_CHECK(wait_until(ctx, [&] { return !shows("(locked)"); }, 5));
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    close_settings_panel(ctx);
}

}  // namespace

const std::vector<TestEntry>& details_tests() {
    static const std::vector<TestEntry> entries = {
        {"analyze", test_analyze},
        {"cap-switch", test_cap_switch},
        {"legacy-fills", test_legacy_fills},
        {"dynamics", test_dynamics},
        {"dynamics-reopen", test_dynamics_reopen},
        {"dynamics-hard", test_dynamics_hard},
        {"stars", test_stars},
        {"details-close-teardown", test_details_close_teardown},
        {"preview-load-bar", test_preview_load_bar},
        {"panel-open-close", test_panel_open_close},
        {"panel-split", test_panel_split},
        {"panel-hide-library", test_panel_hide_library},
        {"layout-sweep", test_layout_sweep},
        {"long-error-wraps", test_long_error_wraps},
        {"panel-prev-next", test_panel_prev_next},
        {"panel-headline", test_panel_headline},
        {"panel-close-cancels", test_panel_close_cancels},
        {"settings-lock", test_settings_lock},
        {"view-cancel-try-again", test_view_cancel_try_again},
        {"view-rules-broken", test_view_rules_broken},
        {"no-analyze-button", test_no_analyze_button},
        {"view-progress-delay", test_view_progress_delay},
        {"view-setting-burst", test_view_setting_burst},
    };
    return entries;
}

}  // namespace uitest
