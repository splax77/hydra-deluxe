#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "uitest_harness.h"

#include "../scoped_hook.h"
#include "../testdata_dirs.h"  // corpus::root
#include "app/analysis.h"
#include "app/config.h"
#include "app/display_format.h"  // kAverageMultiplierLabel
#include "app/dynamics_breakdown.h"
#include "app/launch.h"  // set_sibling_dir_for_test, kPathBuilderExe
#include "app/path_view.h"  // build_path_buttons
#include "app/user_messages.h"  // kAnalysisOffSentence
#include "core/model.h"
#include "core/stars.h"
#include "core/winstr.h"  // join_folder, os_path
#include "ui/details_parts.h"  // kOpenInPathBuilderLabel and its hints
#include "imgui_internal.h"
#include "ui/app_state.h"
#include "ui/column_widths.h"
#include "ui/details_view.h"
#include "ui/library_jobs.h"  // set_view_analyzer_for_test
#include "ui/library_parts.h"  // settings_at_defaults
#include "ui/fonts.h"  // px()
#include "ui/library_view.h"  // library_split_width
#include "ui/preview_controller.h"
#include "ui/preview_load_job.h"  // PreviewLoadJob::Progress, the load bar's step names
#include "ui/text_role.h"
#include "ui/theme.h"  // kBestPathColor, kWarningColor

namespace uitest {

namespace {

// Whether column `column` of the drawn table `t` holds its header and every
// one of `cells` uncut: each text, measured in the role the Dynamics and
// Stars tables draw it in (a Label header, Figure cells; ui/text_role.h), is
// no wider than the room ImGui gave the column. ImGui truncates that room to
// a whole pixel (ImTrunc on WidthGiven, imgui_tables.cpp), so each text's
// width is truncated the same way before the compare.
bool column_fits(const ImGuiTable* t, int column, const char* header,
                 const std::vector<std::string>& cells) {
    const float room = t->Columns[column].WidthGiven;
    if (ImTrunc(hydra::ui::text_size(hydra::ui::TextRole::Label, header).x) > room) return false;
    for (const std::string& cell : cells)
        if (ImTrunc(hydra::ui::text_size(hydra::ui::TextRole::Figure, cell.c_str()).x) > room)
            return false;
    return true;
}

// The Dynamics tab's Totals ghost line for `bd` under this 2x Bass setting:
// the count is the breakdown's own (DynamicsBreakdown::played_total), so a
// test reads it from the record it holds, never types it.
std::string ghosts_line(const hydra::app::DynamicsBreakdown& bd, bool bass2x) {
    return "Ghosts: " + hydra::group_thousands(bd.played_total(bass2x).ghost);
}

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
        ctx->ItemCheck("**/##legacyfills");
    else
        ctx->ItemUncheck("**/##legacyfills");
    close_settings_panel(ctx);
}

// The status of the library's first row under the settings as they are now
// with the SP cap set to `cap`: what the row would read at that cap.
hydra::store::RecordStatus row0_status_at_cap(Harness& h, int cap) {
    hydra::app::Settings at = h.app->settings;
    at.sp_cap = cap;
    return h.app->store->lookup_one(h.app->library_row_at(0).entry.md5, at.profile()).status;
}

// The tables drawn in the last two frames whose column `column` has header
// `header`, in the order ImGui made them.
std::vector<ImGuiTable*> drawn_tables(int column, const char* header) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    std::vector<ImGuiTable*> found;
    for (int n = 0; n < g.Tables.GetMapSize(); ++n) {
        ImGuiTable* t = g.Tables.TryGetMapData(n);
        if (!t || t->LastFrameActive < g.FrameCount - 2 || t->ColumnsCount <= column) continue;
        if (std::strcmp(ImGui::TableGetColumnName(t, column), header) == 0) found.push_back(t);
    }
    return found;
}

// The kicks table the Dynamics tab drew in the last two frames (##kicktable,
// its first column headed "Pad"), or null when it drew none.
ImGuiTable* drawn_kick_table() {
    for (ImGuiTable* t : drawn_tables(0, "Pad"))
        if (t->ID == t->OuterWindow->GetID("##kicktable")) return t;
    return nullptr;
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
// already chose which path to show. The panel must not go on reading the
// freed record: that crashes the UI thread (bad_alloc from a garbage vector
// copy, 0xc0000409). Each switch analyzes the song again (D90 item 1), and
// the record lands from the click's job on the UI thread.
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
    IM_CHECK(h.app->store->record_count() == 1);

    set_legacy_fills(ctx, true);
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.legacy_fills; }, 5));
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).legacy_fills);
    IM_CHECK(compare_disabled());

    IM_CHECK(analyzed_with_paths(ctx));
    IM_CHECK(h.app->viewed.record->legacy_fills);
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    IM_CHECK(h.app->store->record_count() == 2);

    set_legacy_fills(ctx, false);
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.legacy_fills; }, 5));
    IM_CHECK(analyzed_with_paths(ctx));
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::Ready);
    IM_CHECK(!h.app->viewed.record->legacy_fills);
    IM_CHECK(h.app->store->record_count() == 2);  // the 1.1 row was already there
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best11) != std::string::npos; },
                        5));
    IM_CHECK(!compare_disabled());
}

// How many vertices last frame drew in `color` in the window whose name holds
// `part` (window_named), or 0 when no such window was drawn. Text is drawn as
// vertices in its colour, so this tells whether any text there wore it.
size_t vertices_in(const char* part, const ImVec4& color) {
    const ImGuiWindow* w = window_named(part);
    if (!w) return 0;
    const ImU32 c = ImGui::GetColorU32(color);
    size_t n = 0;
    for (const ImDrawVert& v : w->DrawList->VtxBuffer)
        if (v.col == c) ++n;
    return n;
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
    if (ctx->IsError()) return;

    // Every count below is the click's own breakdown (viewed.dynamics), read
    // through the functions that word it (app/dynamics_breakdown.h), so the
    // test types no count. The chart has 2x kicks, which the colour and "not
    // counted" checks need.
    const hydra::app::DynamicsBreakdown on = *h.app->viewed.dynamics;
    IM_CHECK(on.row(hydra::app::DynamicsRow::Kick2x).all() > 0);
    if (ctx->IsError()) return;
    // The header's 2x kick fact, drawn after the facts line's dot (one
    // TwoPartLine apart, render_headline), shown while this tab is active.
    const std::string header_fact =
        hydra::app::TwoPartLine{hydra::app::fact_separator_dot(),
                                hydra::app::dynamics_kick2x_fact(on)}.whole();
    // The kicks table's All column (4): the kicks, the 2x kicks and all kicks.
    auto all_cells = [](const hydra::app::DynamicsBreakdown& bd, bool bass2x) {
        return std::vector<std::string>{
            hydra::group_thousands(bd.row(hydra::app::DynamicsRow::Kick).all()),
            hydra::group_thousands(bd.row(hydra::app::DynamicsRow::Kick2x).all()),
            hydra::group_thousands(bd.kicks_total(bass2x).all())};
    };

    std::string text = visible_text(h);
    IM_CHECK(text.find(hydra::ui::detail::dynamics_enabled_text(on)) != std::string::npos);
    // With 2x Bass on the Kicks line has no tail, so it and the Chart box
    // show the same sentence; the status sits under the second.
    IM_CHECK_EQ(count_on_screen(h, hydra::app::dynamics_kick2x_line(on)), (size_t)2);
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_status(true)) != std::string::npos);
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_status(false)) == std::string::npos);
    IM_CHECK(text.find(header_fact) != std::string::npos);
    IM_CHECK(text.find(ghosts_line(on, true)) != std::string::npos);
    // Colours, read off last frame's vertices: the Chart box's count line is
    // gold, and with 2x Bass on nothing in either box is orange.
    IM_CHECK(vertices_in("dynright", hydra::ui::kBestPathColor) > 0);
    IM_CHECK_EQ(vertices_in("dynright", hydra::ui::kWarningColor), (size_t)0);
    IM_CHECK_EQ(vertices_in("dynleft", hydra::ui::kWarningColor), (size_t)0);
    // The kicks table's All column holds its header and its three cells uncut.
    {
        ImGuiTable* kicks = drawn_kick_table();
        IM_CHECK(kicks != nullptr);
        if (ctx->IsError()) return;
        IM_CHECK(column_fits(kicks, 4, "All", all_cells(on, true)));
    }

    // Toggle 2x Bass off via app state: the song is analyzed again under it
    // (D90 item 1), and the Dynamics tab shows the 2x kicks as not counted.
    h.app->settings.view_bass2x = false;
    h.app->commit_settings();
    ctx->Yield(2);
    const std::string off_row =
        hydra::app::dynamics_kick2x_row_label(h.app->settings.view_prodrums, false).whole();
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find(off_row) != std::string::npos;
    }, 5));
    if (ctx->IsError()) return;
    IM_CHECK(h.app->viewed.dynamics.has_value());
    if (ctx->IsError()) return;
    const hydra::app::DynamicsBreakdown& off = *h.app->viewed.dynamics;
    text = visible_text(h);
    IM_CHECK(text.find(hydra::app::dynamics_kicks_heading(off, false).whole()) !=
             std::string::npos);
    // The header shows the same count with 2x Bass off.
    IM_CHECK(text.find(header_fact) != std::string::npos);
    IM_CHECK(text.find(ghosts_line(off, false)) != std::string::npos);
    // With 2x Bass off the not counted words are orange in both boxes: the
    // Chart box's status, and the Kicks line's ending and 2x kick row.
    IM_CHECK(vertices_in("dynright", hydra::ui::kWarningColor) > 0);
    IM_CHECK(vertices_in("dynleft", hydra::ui::kWarningColor) > 0);
    {
        ImGuiTable* kicks = drawn_kick_table();
        IM_CHECK(kicks != nullptr);
        if (ctx->IsError()) return;
        IM_CHECK(column_fits(kicks, 4, "All", all_cells(off, false)));
    }

    // Restore.
    h.app->settings.view_bass2x = true;
    h.app->commit_settings();
}

// The Dynamics count comes with the click (D87 item 1: kick dynamics are no
// longer stored): after another chart was open and the panel closed, a
// reopen counts the chart again and shows the same numbers. With 2x Bass on,
// the count is ready the moment the click's analysis is, from that same job.
void test_dynamics_reopen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);

    // ---- Scenario 1: count, then re-open and count again ----
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;

    // Open Acid Romance on its Dynamics tab, wait for the click's job to
    // count it, and check the tab shows that count's ghosts. Returns the
    // count. The search goes through app state to avoid the ImGui
    // input-buffer residue from the previous dynamics test.
    auto open_and_count = [&]() -> std::optional<hydra::app::DynamicsBreakdown> {
        h.app->set_search("Acid Romance");
        ctx->Yield(2);
        IM_CHECK_RETV(h.app->library_shown_count() > 0, std::nullopt);
        IM_CHECK_RETV(h.app->library_row_at(0).title == "Acid Romance", std::nullopt);
        open_details(ctx, 0);
        if (ctx->IsError()) return std::nullopt;
        ctx->ItemClick("##DetailsTabs/Dynamics");
        IM_CHECK_RETV(wait_until(ctx, [&] {
            return !h.app->view_job && h.app->viewed.dynamics.has_value();
        }, 60), std::nullopt);
        const bool bass2x = h.app->settings.effective_bass2x();
        IM_CHECK_RETV(visible_text(h).find(ghosts_line(*h.app->viewed.dynamics, bass2x)) !=
                          std::string::npos,
                      std::nullopt);
        return h.app->viewed.dynamics;
    };
    const std::optional<hydra::app::DynamicsBreakdown> first = open_and_count();
    if (ctx->IsError() || !first) return;

    // Open a different chart, so the panel's count is that chart's, then
    // close the panel so the tab stops rendering.
    h.app->set_search("");
    size_t other_idx = 0;
    for (size_t i = 0; i < h.app->library_shown_count(); ++i) {
        if (h.app->library_row_at(i).title != "Acid Romance") {
            other_idx = i;
            break;
        }
    }
    h.app->select(h.app->library_row_at(other_idx).entry, hydra::ui::SelectionSource::Click);
    h.app->show_details = false;
    ctx->Yield(3);

    // Reopen Acid Romance. The click counts it again from the file, and the
    // count is the same as the first.
    const std::optional<hydra::app::DynamicsBreakdown> again = open_and_count();
    if (ctx->IsError() || !again) return;
    IM_CHECK(*again == *first);

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
    // The tab shows that count.
    if (ctx->IsError()) return;
    IM_CHECK(visible_text(h).find(ghosts_line(*h.app->viewed.dynamics, true)) !=
             std::string::npos);
}

// At Hard the Dynamics tab counts Hard's own kicks (findings 10, 12, 255).
// Pathfinder - When The Sunrise Breaks The Darkness charts 2x kicks at
// Expert (pitch 95) and none at Hard. Those Expert notes must not reach
// Hard's counts (D20): Hard's 2x kick row stays empty, the header carries no
// 2x kick fact, and 2x Bass off changes no Hard total.
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
    if (ctx->IsError()) return;
    const hydra::app::DynamicsBreakdown hard = *h.app->viewed.dynamics;

    // The same chart counted at Expert by the count's own parse
    // (app::load_dynamics_song): its 2x kicks are there to leak.
    hydra::ChartReading expert = h.app->settings.reading();
    expert.difficulty = hydra::Difficulty::Expert;
    const hydra::app::DynamicsBreakdown at_expert = hydra::app::count_dynamics(
        hydra::app::load_dynamics_song(h.app->selected->notespath, expert));
    IM_CHECK(at_expert.row(hydra::app::DynamicsRow::Kick2x).all() > 0);
    IM_CHECK(!hydra::app::dynamics_kick2x_fact(at_expert).empty());
    // None reached Hard: no 2x kick row, so no header fact.
    IM_CHECK_EQ(hard.row(hydra::app::DynamicsRow::Kick2x).all(), 0);
    IM_CHECK(hydra::app::dynamics_kick2x_fact(hard).empty());
    if (ctx->IsError()) return;

    std::string text = visible_text(h);
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_line(hard)) != std::string::npos);
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_fact(at_expert)) == std::string::npos);

    // 2x Bass off: Totals is unchanged, because Hard has no 2x kicks to drop.
    h.app->settings.view_bass2x = false;
    h.app->commit_settings();
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find(hydra::app::dynamics_kick2x_status(false)) !=
               std::string::npos;
    }, 5));
    if (ctx->IsError()) return;
    IM_CHECK(h.app->viewed.dynamics.has_value());
    if (ctx->IsError()) return;
    const hydra::app::DynamicsBreakdown& off = *h.app->viewed.dynamics;
    IM_CHECK(off.played_total(false) == hard.played_total(true));
    text = visible_text(h);
    IM_CHECK(text.find(hydra::app::dynamics_kicks_heading(off, false).whole()) !=
             std::string::npos);
    IM_CHECK(text.find(ghosts_line(off, false)) != std::string::npos);
    // The row's tag does not depend on the count.
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_row_label(h.app->settings.view_prodrums,
                                                             false).whole()) !=
             std::string::npos);
    IM_CHECK(text.find(hydra::app::dynamics_kick2x_fact(at_expert)) == std::string::npos);

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
    IM_CHECK(text.find("Base score: " + hydra::group_thousands(sc.chart_base_score)) !=
             std::string::npos);
    IM_CHECK(text.find("Solo bonus: " + hydra::group_thousands(sc.solo_bonus) +
                       " (not counted toward stars)") != std::string::npos);
    IM_CHECK(text.find("With full solo bonus") != std::string::npos);
    // The factor column's title is the shared label.
    const size_t table_at = text.find(hydra::app::kAverageMultiplierLabel);
    IM_CHECK(table_at != std::string::npos);
    if (ctx->IsError()) return;
    // Each star's row, in star order after the table's headers: its factor
    // (kStarMultipliers, one decimal as the tab words it), its cutoff, then
    // its with-solo figure, the last two star_cutoffs()'s. The tab draws a
    // row's cells one after another, so a figure found after the previous
    // row's last cell is in its own row, not in a headline or another row.
    size_t at = table_at;
    for (int stars = 1; stars <= hydra::kMaxStars; ++stars) {
        char factor[16];
        std::snprintf(factor, sizeof(factor), "%.1f",
                      static_cast<double>(hydra::kStarMultipliers[stars - 1]));
        at = text.find(factor, at);
        IM_CHECK(at != std::string::npos);
        if (ctx->IsError()) return;
        at = text.find(hydra::group_thousands(sc.cutoffs[stars - 1]), at);
        IM_CHECK(at != std::string::npos);
        if (ctx->IsError()) return;
        at = text.find(hydra::group_thousands(sc.with_solo[stars - 1]), at);
        IM_CHECK(at != std::string::npos);
        if (ctx->IsError()) return;
    }
    // Cutoff holds its header and the seven cutoffs uncut.
    {
        std::vector<std::string> cutoffs;
        for (int64_t cutoff : sc.cutoffs) cutoffs.push_back(hydra::group_thousands(cutoff));
        const std::vector<ImGuiTable*> tables = drawn_tables(2, "Cutoff");
        IM_CHECK_EQ(tables.size(), size_t{1});
        if (ctx->IsError()) return;
        IM_CHECK(column_fits(tables[0], 2, "Cutoff", cutoffs));
    }

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
    IM_CHECK(text.find("Base score: " + hydra::group_thousands(sc.chart_base_score)) !=
             std::string::npos);
    IM_CHECK(text.find(hydra::group_thousands(sc.cutoffs[hydra::kMaxStars - 1])) !=
             std::string::npos);
    IM_CHECK(text.find("Solo bonus") == std::string::npos);
    IM_CHECK(text.find("With full solo bonus") == std::string::npos);
}

// Copies the corpus chart Allister - Overrated (its notes.mid and song.ini)
// into a fresh scratch folder `folder` under the harness's temp dir, makes
// that folder the only song folder and scans it. Returns the chart's copied
// folder; the corpus itself is never touched.
std::filesystem::path scan_scratch_overrated(ImGuiTestContext* ctx, const std::string& folder) {
    namespace fs = std::filesystem;
    Harness& h = harness(ctx);
    const fs::path from = fs::u8path(corpus::root()) / "common" / "IB24" / "T1" / "Allister - Overrated";
    const fs::path songs = fs::u8path(h.temp_dir) / folder;
    const fs::path chart = songs / "Allister - Overrated";
    fs::remove_all(songs);
    fs::create_directories(chart);
    fs::copy_file(from / "notes.mid", chart / "notes.mid");
    fs::copy_file(from / "song.ini", chart / "song.ini");
    h.app->settings.chartfolders = {songs.u8string()};
    scan_library(ctx);
    return chart;
}

// Hiding the details panel by any route tears it down. The "Rescan library"
// button sets show_details = false directly and leaves the teardown to
// tick(). Without it, the Preview would keep its audio device and keep playing.
// The button shows only when the chart file is missing, so the test deletes
// the notes file of a scratch copy and clicks the button the panel shows.
void test_details_close_teardown(ImGuiTestContext* ctx) {
    namespace fs = std::filesystem;
    Harness& h = harness(ctx);
    reset_app(h);
    const fs::path chart = scan_scratch_overrated(ctx, "close_teardown");
    if (ctx->IsError()) return;
    IM_CHECK_EQ(h.app->library_shown_count(), (size_t)1);
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->ItemClick("**/Preview");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->preview && h.app->preview->active() && !h.app->preview->loading();
    }, 120));
    if (ctx->IsError()) return;
    IM_CHECK_STR_EQ(h.app->preview->error().c_str(), "");
    ctx->ItemClick("**/Play");
    IM_CHECK(h.app->preview->playing());
    if (ctx->IsError()) return;

    // The chart file goes; the panel's next file check (kFileCheckSeconds
    // apart, D54) finds it missing and shows the button. The wait ends at
    // once if the panel closes, since the button can then never show.
    fs::remove(chart / "notes.mid");
    IM_CHECK(wait_until(ctx, [&] {
        return !h.app->details_open() || !h.app->details_ui.file_ok;
    }, hydra::ui::AppState::kFileCheckSeconds * 3));
    IM_CHECK(h.app->details_open());
    if (ctx->IsError()) return;
    ctx->Yield(2);
    IM_CHECK(ctx->ItemExists("**/Rescan library"));
    if (ctx->IsError()) return;
    ctx->ItemClick("**/Rescan library");
    ctx->Yield(3);
    IM_CHECK(!h.app->details_open());
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

// A rescan that reads a changed song.ini changes the open song panel too: the
// panel re-reads its chart's entry by row_key, so its header follows the
// library row (D114 item 7). One chart is copied to scratch so its song.ini
// can be rewritten; the corpus itself is never touched.
void test_panel_follows_rescan(ImGuiTestContext* ctx) {
    namespace fs = std::filesystem;
    Harness& h = harness(ctx);
    reset_app(h);
    const fs::path chart = scan_scratch_overrated(ctx, "follow_rescan");
    if (ctx->IsError()) return;
    IM_CHECK_EQ(h.app->library_shown_count(), (size_t)1);
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->selected->title == "Overrated");

    // The same chart under a new name; the notes file is unchanged.
    std::string ini;
    {
        std::ifstream in(chart / "song.ini", std::ios::binary);
        ini.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    const std::string old_line = "name = Overrated";
    const size_t at = ini.find(old_line);
    IM_CHECK(at != std::string::npos);
    if (ctx->IsError()) return;
    ini.replace(at, old_line.size(), "name = Renamed By Rescan");
    {
        std::ofstream out(chart / "song.ini", std::ios::binary | std::ios::trunc);
        out << ini;
    }
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->details_open());
    IM_CHECK_EQ(h.app->library_row_at(0).title, std::string("Renamed By Rescan"));
    IM_CHECK_EQ(h.app->selected->title, h.app->library_row_at(0).title);
    // What the user reads: the old name is gone from the panel header too.
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, "Overrated"));
    IM_CHECK(on_screen(h, "Renamed By Rescan"));
}

// The Preview's loading bar while a real load runs: every frame's label
// starts with one of the load's step names, and the bar never moves
// backwards. The step names are PreviewLoadJob::Progress::label()'s own, one
// per Step with no bytes counted (the name alone). The bar's arithmetic is
// pinned in tests/test_preview_load_progress.cpp. A run that sees no frame
// of the load fails: it checked nothing.
void test_preview_load_bar(ImGuiTestContext* ctx) {
    using Job = hydra::ui::PreviewLoadJob;
    Harness& h = harness(ctx);
    start_preview(ctx);
    if (ctx->IsError()) return;
    std::vector<std::string> steps;
    for (Job::Step s : {Job::Step::Reading, Job::Step::Opening, Job::Step::Building,
                        Job::Step::Highway}) {
        Job::Progress p;
        p.step = s;
        steps.push_back(p.label());
    }
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
        for (const std::string& s : steps) named = named || lp.label.rfind(s, 0) == 0;
        if (!named) {
            ++bad_labels;
            if (first_bad.empty()) first_bad = lp.label;
        }
        if (lp.fraction < prev) ++backwards;
        prev = lp.fraction;
        return false;
    }, 120));
    ctx->LogInfo("load bar polled %d times; first bad label '%s'", polls, first_bad.c_str());
    IM_CHECK(polls > 0);
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
    auto library_w = [&] { return ctx->WindowInfo("//Hydra/##library").Window->Size.x; };
    const float closed_w = library_w();

    // Open, the library is as wide as the split gives it (library_split_width
    // at the remembered share, as test_panel_split checks the default), and
    // the panel sits beside it, not over it.
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    ImGuiWindow* panel = ctx->WindowInfo("//Hydra/##songpanel").Window;
    IM_CHECK(panel != nullptr);
    if (ctx->IsError()) return;
    const float room = library_w() + panel->Size.x;
    IM_CHECK_FLOAT_NEAR_EQ(library_w(),
                           hydra::ui::library_split_width(room, hydra::ui::library_share()), 1.0f);
    IM_CHECK(library_w() < closed_w);
    IM_CHECK(!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId));  // the panel is not a modal

    // The X closes it and the library gets back the width it had closed.
    ctx->ItemClick("X##closepanel");
    ctx->Yield(3);
    IM_CHECK(!h.app->details_open());
    IM_CHECK_EQ(library_w(), closed_w);

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(3);
    IM_CHECK(!h.app->details_open());
    IM_CHECK_EQ(library_w(), closed_w);
}

// The split beside the panel. A drag of the library's edge is the only thing
// that changes it: it survives closing and reopening the panel, and a
// hydra_ui.ini whose library entry holds the largest split loses to the
// [Hydra][Layout] share.
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

    // A hydra_ui.ini read while no song is open (as at startup): the library
    // child's own entry at a huge width, and a share. The share wins.
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

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    const float room = library()->Size.x + panel()->Size.x;
    const float shared = library()->Size.x;
    IM_CHECK(on_screen(h, "Hide library"));
    IM_CHECK(!hydra::ui::library_hidden());

    ctx->SetRef("//Hydra");
    ctx->ItemClick("**/Hide library");
    ctx->Yield(3);
    IM_CHECK(hydra::ui::library_hidden());
    IM_CHECK(on_screen(h, "Show library"));
    IM_CHECK(!on_screen(h, "Hide library"));
    IM_CHECK_FLOAT_NEAR_EQ(panel()->Size.x, room, 1.0f);
    IM_CHECK(!on_screen(h, "Library"));  // the pane's heading is gone

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
    IM_CHECK(on_screen(h, "Hide library"));
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, shared, 1.0f);
    IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x + panel()->Size.x, room, 1.0f);
}

// The song panel's "Open in Path Builder" button, with the builder's folder
// pointed at a scratch folder (the real build folder holds the real builder
// once it is built). `present` puts an empty file named after the builder's
// exe there. The button sits on the header's row, left of "Hide library"; it
// is greyed exactly when the exe is missing, and its hint says which case
// holds. An empty file is no program, so a click on the enabled button fails
// to start it, and the status line says so; nothing is launched.
void check_path_builder_button(ImGuiTestContext* ctx, bool present) {
    namespace ui = hydra::ui;
    Harness& h = harness(ctx);
    reset_app(h);
    const std::string dir = hydra::join_folder(h.temp_dir, "builder");
    std::filesystem::create_directories(hydra::os_path(dir));
    if (present)
        std::ofstream(hydra::os_path(hydra::join_folder(dir, hydra::app::kPathBuilderExe)));
    ScopedHook seam(hydra::app::set_sibling_dir_for_test, std::optional<std::string>(dir));
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    ctx->Yield(2);

    const char* label = ui::detail::kOpenInPathBuilderLabel;
    const std::string dump = widget_dump(ctx, "##songpanel", label);
    IM_CHECK(dump.find(std::string("\"") + label + "\"") != std::string::npos);
    IM_CHECK_EQ(dump.find(" disabled") != std::string::npos, !present);

    const ImGuiTestItemInfo builder = ctx->ItemInfo(label);
    const ImGuiTestItemInfo hide = ctx->ItemInfo("Hide library");
    if (ctx->IsError()) return;
    IM_CHECK_LE(builder.RectFull.Max.x, hide.RectFull.Min.x);
    IM_CHECK_EQ(builder.RectFull.Min.y, hide.RectFull.Min.y);

    ctx->MouseMove(label, ImGuiTestOpFlags_NoCheckHoveredId);
    const std::string hint =
        present ? std::string(ui::detail::kPathBuilderHint) : ui::detail::path_builder_missing_hint();
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, hint); }, 5));
    if (!present) return;

    ctx->ItemClick(label);
    ctx->Yield(2);
    IM_CHECK(h.app->status_is_problem);
    std::fprintf(stderr, "status: %s\n", h.app->status_message.c_str());
    IM_CHECK(h.app->status_message.find("Path Builder") != std::string::npos);
}

void test_open_in_path_builder_greyed(ImGuiTestContext* ctx) {
    check_path_builder_button(ctx, false);
}

void test_open_in_path_builder_enabled(ImGuiTestContext* ctx) {
    check_path_builder_button(ctx, true);
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
    int swept_with_rows = 0;  // Paths sweeps whose best path drew activation rows
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
            // A best path with activations draws its rows under the Expand all
            // toggle, which reads Collapse all once the first share has opened
            // them; neither toggle there means the rows were never drawn. A
            // best path with none (Themata's) draws no toggle at all.
            if (h.app->viewed.record->best_path().activations.empty()) {
                IM_CHECK(!ctx->ItemExists("**/Expand all"));
                IM_CHECK(!ctx->ItemExists("**/Collapse all"));
            } else {
                ++swept_with_rows;
                if (ctx->ItemExists("**/Expand all")) ctx->ItemClick("**/Expand all");
                ctx->Yield(2);
                IM_CHECK(ctx->ItemExists("**/Collapse all"));
            }
            if (ctx->IsError()) return;
            open_paths_folds(ctx);  // the backend tables and the two folds under the rows
            overflowing_windows(at + "Paths", found);
            ctx->ItemClick("**/##DetailsTabs/Preview");
            IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active() &&
                                                  !h.app->preview->loading(); }, 120));
            if (ctx->IsError()) return;
            ctx->Yield(3);
            overflowing_windows(at + "Preview", found);
            ctx->ItemClick("**/##DetailsTabs/Dynamics");
            IM_CHECK(wait_until(ctx, [&] { return h.app->viewed.dynamics.has_value(); }, 60));
            if (ctx->IsError()) return;
            ctx->Yield(3);
            // Themata is a MIDI chart without [ENABLE_CHART_DYNAMICS]; its
            // enabled line is the longest the right box draws.
            const hydra::app::DynamicsBreakdown& bd = *h.app->viewed.dynamics;
            if (std::strcmp(c.title, "Themata") == 0) IM_CHECK(!bd.dynamics_enabled);
            IM_CHECK(visible_text(h).find(hydra::ui::detail::dynamics_enabled_text(bd)) !=
                     std::string::npos);
            overflowing_windows(at + "Dynamics", found);
            ctx->ItemClick("**/##DetailsTabs/Stars");
            ctx->Yield(3);
            overflowing_windows(at + "Stars", found);
            if (ctx->IsError()) return;
        }
    }
    // The sweep must have opened real activation rows on some chart, or the
    // Paths checks above measured nothing.
    IM_CHECK(swept_with_rows > 0);
    for (const std::string& f : found) std::fprintf(stderr, "OVERFLOW %s\n", f.c_str());
    IM_CHECK_EQ(found.size(), (size_t)0);
}

// The click's analysis fails with a long path in its error, through the
// test seam: the engine has no input the harness can give that fails this way.
struct FailingAnalyzer {
    explicit FailingAnalyzer(std::string what)
        : seam(hydra::ui::set_view_analyzer_for_test,
               [what](const std::string&, const hydra::app::AnalysisSettings&,
                      const std::function<void(float)>&) -> hydra::app::AnalysisResult {
                   throw std::runtime_error(what);
               }) {}
    ScopedHook seam;
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

    // The other end: at the last row ">" is greyed and "<" is not.
    const size_t rows = h.app->view_row_count();
    IM_CHECK(rows > 1);
    if (ctx->IsError()) return;
    h.app->select(h.app->view_row(rows - 1), hydra::ui::SelectionSource::Click);
    ctx->Yield(3);
    IM_CHECK(h.app->details_open());
    IM_CHECK(h.app->selected->notespath == h.app->view_row(rows - 1).notespath);
    IM_CHECK((ctx->ItemInfo(">##nextsong").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("<##prevsong").ItemFlags & ImGuiItemFlags_Disabled) == 0);
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
        // No headline yet: the click holds no record, so nothing draws one.
        IM_CHECK(!h.app->viewed.record.has_value());
        IM_CHECK(text.find("Optimal path") == std::string::npos);
        // An artist made only of Clone Hero tags reads "(unknown)", as a
        // title does (D50 item 5).
        h.app->selected->artist = "<color=#FF8000></color><b></b>";
        ctx->Yield(2);
        IM_CHECK(visible_text(h).find("(unknown) \xC2\xB7 charted by Hoph2o") !=
                 std::string::npos);
        h.app->selected->artist = "Green Day";
        // A missing charter reads "(unknown)" and the line keeps its
        // "charted by" (D266 item 2), for the scan's placeholder and for
        // empty text alike.
        for (const char* missing : {"<unknown charter>", ""}) {
            h.app->selected->charter = missing;
            ctx->Yield(2);
            IM_CHECK(visible_text(h).find("Green Day \xC2\xB7 charted by (unknown)") !=
                     std::string::npos);
        }
        h.app->selected->charter = "Hoph2o";
        ctx->Yield(2);
    }

    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    // Every figure below is the click's record's, read at run time: the best
    // score (optimal_score), the best path's string and its stars, which the
    // headline takes from the summary row and must equal path_stars.
    const hydra::HydraRecord& record = *h.app->viewed.record;
    IM_CHECK(!record.paths.empty());
    if (ctx->IsError()) return;
    const hydra::Path& best = record.best_path();
    const std::string score = hydra::group_thousands(*record.optimal_score());
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find(score) != std::string::npos;
    }, 5));
    text = visible_text(h);
    IM_CHECK(text.find(best.pathstring()) != std::string::npos);
    IM_CHECK(h.app->viewed.summary.has_scored_best_path());
    IM_CHECK(h.app->viewed.summary.stars == std::optional<int>(hydra::path_stars(best)));
    const std::string facts = std::string("Optimal path") + hydra::app::kFactSeparator +
                              hydra::counted(hydra::path_stars(best), "star", "stars");
    IM_CHECK(text.find(facts) != std::string::npos);
    // The 2x kick fact follows the stars only when the chart has 2x kicks;
    // Burnout's count has none, so nothing follows them. The count itself is
    // there, so the empty fact is the chart's, not a missing count's.
    IM_CHECK(h.app->viewed.dynamics.has_value());
    if (ctx->IsError()) return;
    IM_CHECK(hydra::app::dynamics_kick2x_fact(*h.app->viewed.dynamics).empty());
    IM_CHECK(text.find(hydra::app::TwoPartLine{facts, hydra::app::fact_separator_dot()}.whole()) ==
             std::string::npos);
    // The hardest timing sits beside each path in the list (the best path's
    // button, build_path_buttons), not in the headline.
    IM_CHECK(text.find("hardest squeeze") == std::string::npos);
    const hydra::app::PathButtonsView list = hydra::app::build_path_buttons(
        record, h.app->settings.depth_mode, h.app->settings.depth_value);
    const hydra::app::PathButtonView* best_button = nullptr;
    for (const hydra::app::PathButtonView& b : list.buttons)
        if (b.path == &best) best_button = &b;
    IM_CHECK(best_button != nullptr);
    if (ctx->IsError()) return;
    // Some path in the list carries a hardest timing, and it is on screen
    // (a chart whose list names none would check nothing).
    bool timing_shown = false;
    for (const hydra::app::PathButtonView& b : list.buttons)
        if (!b.timing.empty() && text.find(b.timing) != std::string::npos) timing_shown = true;
    IM_CHECK(timing_shown);
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
    IM_CHECK(h.app->store->record_count() == 0);
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
    IM_CHECK(h.app->store->record_count() == 0);

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
        return h.app->viewed.state == hydra::ui::ViewedSong::State::AnalysisOff;
    }, 60));
    ctx->ItemClick("##DetailsTabs/Paths");
    ctx->Yield(3);
    std::string text = visible_text(h);
    // The panel uses the toolbar's sentence (D114 item 13), whose one home is
    // kAnalysisOffSentence (app/user_messages.h).
    IM_CHECK(text.find(hydra::app::kAnalysisOffSentence) != std::string::npos);
    IM_CHECK(text.find("Analysis is off until") == std::string::npos);
    IM_CHECK(!h.app->viewed.record.has_value());
    IM_CHECK(!ctx->ItemExists("**/##path0"));
    IM_CHECK(!ctx->ItemExists("**/Copy path"));

    ctx->ItemClick("##DetailsTabs/Dynamics");
    ctx->Yield(3);
    IM_CHECK(h.app->viewed.dynamics.has_value());
    if (ctx->IsError()) return;
    const hydra::app::DynamicsBreakdown& bd = *h.app->viewed.dynamics;
    text = visible_text(h);
    IM_CHECK(text.find(hydra::ui::detail::dynamics_enabled_text(bd)) != std::string::npos);
    IM_CHECK(text.find(ghosts_line(bd, h.app->settings.effective_bass2x())) != std::string::npos);
    IM_CHECK(h.app->store->record_count() == 0);  // nothing saved
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
        h.app->select(h.app->view_row(row), hydra::ui::SelectionSource::Click);
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
// and the panel shows the final cap's paths (D90 item 2).
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
    IM_CHECK(h.app->store->record_count() == 2);  // the first click's and the last cap's
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
        IM_CHECK((ctx->ItemInfo("**/##earlyfillwindow").ItemFlags & ImGuiItemFlags_Disabled) ==
                 0);
        IM_CHECK((ctx->ItemInfo("**/##prodrums").ItemFlags & ImGuiItemFlags_Disabled) == 0);
        IM_CHECK(!on_screen(h, "Stop the batch to change these."));
        IM_CHECK(!on_screen(h, "(locked)"));
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
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "(locked)"); }, 5));
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Stop the batch to change these."); }, 5));
    IM_CHECK((ctx->ItemInfo(settings_control(ctx, "##prodrums", "##difficulty")).ItemFlags &
              ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##earlyfillwindow").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##prodrums").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##noteshuffle").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK((ctx->ItemInfo("**/##mslimit").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    // The reset is greyed by the lock, not by having nothing to undo.
    IM_CHECK(!hydra::ui::detail::settings_at_defaults(h.app->settings));
    IM_CHECK((ctx->ItemInfo("**/Reset to defaults").ItemFlags & ImGuiItemFlags_Disabled) != 0);
    close_settings_panel(ctx);

    h.app->batch_job->stop();
    IM_CHECK(wait_until(ctx, [&] { return !h.app->batch_running(); }, 300));
    IM_CHECK(wait_until(ctx, [&] { return !jobs_busy(h); }, 120));
    IM_CHECK(wait_until(ctx, [&] { return !on_screen(h, "(locked)"); }, 5));
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK((ctx->ItemInfo("**/##spcap").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK((ctx->ItemInfo("**/##earlyfillwindow").ItemFlags & ImGuiItemFlags_Disabled) == 0);
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
        {"open-in-path-builder-greyed", test_open_in_path_builder_greyed},
        {"open-in-path-builder-enabled", test_open_in_path_builder_enabled},
        {"layout-sweep", test_layout_sweep},
        {"long-error-wraps", test_long_error_wraps},
        {"panel-prev-next", test_panel_prev_next},
        {"panel-headline", test_panel_headline},
        {"panel-close-cancels", test_panel_close_cancels},
        {"panel-follows-rescan", test_panel_follows_rescan},
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
