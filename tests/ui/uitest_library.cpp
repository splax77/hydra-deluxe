#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iterator>  // std::size
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "uitest_harness.h"

#include "../db_file_sql.h"          // tests/ is not on the runner's include path
#include "../old_layout_fixture.h"
#include "../testdata_dirs.h"  // root
#include "app/config.h"
#include "app/user_messages.h"  // old_library_set_aside_notice, library_rescan_notice
#include "core/error_kind.h"
#include "imgui_internal.h"
#include "parse/song.h"  // NoNotesError
#include "render/overlay_layout.h"  // ellipsize, the one cutting rule
#include "ui/app_shell.h"  // remember_library_share
#include "ui/app_state.h"
#include "ui/column_widths.h"
#include "ui/details_view.h"  // kMinLibraryW
#include "ui/fonts.h"  // px()
#include "ui/library_model.h"
#include "ui/library_parts.h"  // analyze_shown_label
#include "ui/preview_controller.h"
#include "ui/text_role.h"  // text_size, TextRole
#include "store/record_store.h"
#include "store/upgrade_files.h"  // set_aside_path, with_side_files
#include "ui/widgets.h"  // widest_digits, button_slot_width
#include "../scoped_hook.h"
#include "../update_reply.h"
#include "core/app_version.h"
#include "core/winstr.h"  // read_file_text, os_path
#include "net/http.h"     // set_fetcher, set_downloader
#include "ui/update_job.h"

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <thread>

namespace uitest {

// Defined in uitest_batch_reports.cpp.
bool batch_search(ImGuiTestContext* ctx, const std::string& search);
void dismiss_done(ImGuiTestContext* ctx);
void start_library_batch(ImGuiTestContext* ctx);

namespace {

namespace fs = std::filesystem;

void test_scan(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    IM_CHECK(h.app->library.rows().empty());
    scan_library(ctx);
    if (ctx->IsError()) return;
    // The first row's title is drawn in the table.
    const std::string& title = h.app->library_row_at(0).title;
    IM_CHECK(visible_text(h).find(title) != std::string::npos);
    // The scan flipped the button to its rescan label and persisted that.
    IM_CHECK(h.app->settings.is_rescan);
    IM_CHECK(ctx->ItemInfo("Scan library").ID != 0);
}

// One chart in two folders is two rows (row_key). A click on the second
// copy's row selects that copy, not the first one the scan listed, and the
// batch's entries come back in table order, leaving out a row the store no
// longer lists.
void test_library_twin_rows(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    auto entry = [](const char* title, const char* path) {
        hydra::store::ChartLibraryEntry e;
        e.md5 = "0123456789abcdef0123456789abcdef";
        e.title = title;
        e.notespath = path;
        e.rootfolder = "twins";
        return e;
    };
    const hydra::store::ChartLibraryEntry first = entry("Twin A", "C:\\twins\\a\\notes.chart");
    const hydra::store::ChartLibraryEntry second = entry("Twin B", "C:\\twins\\b\\notes.chart");
    hydra::store::ChartLibraryEntry other = entry("Zed", "C:\\twins\\z\\notes.chart");
    other.md5 = "fedcba9876543210fedcba9876543210";
    h.app->store->rebuild_chart_library({first, second, other});
    h.app->reload_library();
    IM_CHECK_EQ(h.app->library_shown_count(), size_t{3});

    // Rows sort by title: Twin A, Twin B, Zed.
    IM_CHECK(h.app->library_row_at(1).entry.notespath == second.notespath);
    h.app->select(h.app->library_row_at(1).entry, hydra::ui::SelectionSource::Click);
    IM_CHECK(h.app->selected && h.app->selected->notespath == second.notespath);
    IM_CHECK(h.app->selected && h.app->selected->title == "Twin B");
    h.app->close_details();

    // Table order, not the store's (which lists by name): titles descending.
    // No frame runs between these calls, so the table's own sort can't step in.
    h.app->library.set_sort(hydra::ui::LibrarySort::Title, false);
    std::vector<hydra::store::ChartLibraryEntry> matches = h.app->library_matches();
    IM_CHECK_EQ(matches.size(), size_t{3});
    if (matches.size() == 3) {
        IM_CHECK(matches[0].notespath == other.notespath);
        IM_CHECK(matches[1].notespath == second.notespath);
        IM_CHECK(matches[2].notespath == first.notespath);
    }

    // The store drops the first copy; the rows are not reloaded yet.
    h.app->store->rebuild_chart_library({second, other});
    matches = h.app->library_matches();
    IM_CHECK_EQ(matches.size(), size_t{2});
    if (matches.size() == 2) {
        IM_CHECK(matches[0].notespath == other.notespath);
        IM_CHECK(matches[1].notespath == second.notespath);
    }
    h.app->library.set_sort(hydra::ui::LibrarySort::Title, true);
}

// The settings panel's difficulty dropdown: it drives the chartmode
// everything else is keyed by. 2x Bass stays live at every difficulty (D20)
// and is part of the key there too.
void test_difficulty(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    open_settings_panel(ctx);
    if (ctx->IsError()) return;

    IM_CHECK(h.app->settings.view_bass2x);  // the default the test relies on
    IM_CHECK((ctx->ItemInfo("**/##bass2x").ItemFlags & ImGuiItemFlags_Disabled) == 0);

    settings_combo_pick(ctx, "##prodrums", "##difficulty", "Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));
    IM_CHECK_STR_EQ(hydra::app::Settings::load_file(h.ini_path).view_difficulty.c_str(),
                    "Hard");
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 2x Bass");

    // Live at Hard: unticking it changes Hard's key, and ticking it restores it.
    IM_CHECK((ctx->ItemInfo("**/##bass2x").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(h.app->settings.effective_bass2x());
    ctx->ItemClick("**/##bass2x");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.view_bass2x; }, 5));
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 1x Bass");
    ctx->ItemClick("**/##bass2x");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_bass2x; }, 5));

    // Back on Expert the box still carries the user's own setting. (Checked
    // here rather than at the end of the test: the details modal opened below
    // has no close button the harness can address.)
    settings_combo_pick(ctx, "##prodrums", "##difficulty", "Expert");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Expert"; }, 5));
    IM_CHECK((ctx->ItemInfo("**/##bass2x").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(h.app->settings.effective_bass2x());
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Expert Pro Drums, 2x Bass");

    settings_combo_pick(ctx, "##prodrums", "##difficulty", "Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));

    // Narrow to a chart that actually has a [HardDrums] section, so the
    // analysis below has notes to work with.
    close_settings_panel(ctx);
    ctx->ItemInputValue("**/##search", "Pokemon Theme");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "Pokemon Theme"; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return h.app->library_shown_count() > 0; }, 5));
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    // The settings, not the song panel, name the difficulty now.
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 2x Bass");

    // The click analyzed it under Hard (D87 item 1).
    wait_song_analyzed(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(!h.app->viewed.record->paths.empty());
    std::string best = h.app->viewed.record->best_path().pathstring();
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(best) != std::string::npos; }, 5));
    // The Hard record is filed under the Hard chartmode, so the library row
    // now reads Ready under it.
    IM_CHECK(h.app->library_row_at(0).status ==
             hydra::store::RecordStatus::Ready);

    // The Preview follows the selected difficulty: Hard's notes must load,
    // with no "Preview failed".
    ctx->ItemClick("**/Preview");
    IM_CHECK(wait_until(ctx, [&] { return h.app->preview && h.app->preview->active(); }, 10));
    IM_CHECK(wait_until(ctx, [&] { return !h.app->preview->loading(); }, 120));
    IM_CHECK_STR_EQ(h.app->preview->error().c_str(), "");
}

// The settings panel's "Note Shuffle" box (D104 items 1, 2 and 6): off by
// default, ticking it saves the INI and ends the mode string in ", Note
// Shuffle", and the dmleaderboards Compare button greys out with its sentence
// as its tooltip. Unticking puts both back.
void test_note_shuffle_switch(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    auto compare_disabled = [&] {
        return (ctx->ItemInfo("//Hydra/Compare with dmleaderboards...").ItemFlags &
                ImGuiItemFlags_Disabled) != 0;
    };
    const std::string refused =
        "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be "
        "compared with a shuffled path. Untick Note Shuffle to compare.";

    IM_CHECK(!h.app->settings.view_noteshuffle);
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Expert Pro Drums, 2x Bass");
    IM_CHECK(!compare_disabled());

    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK((ctx->ItemInfo("**/##noteshuffle").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    ctx->ItemCheck("**/##noteshuffle");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_noteshuffle; }, 5));
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).view_noteshuffle);
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(),
                    "Expert Pro Drums, 2x Bass, Note Shuffle");

    close_settings_panel(ctx);
    IM_CHECK(wait_until(ctx, compare_disabled, 5));
    // The tooltip shows on a disabled button too, after the hover delay.
    ctx->MouseMove("Compare with dmleaderboards...", ImGuiTestOpFlags_NoCheckHoveredId);
    IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(refused) != std::string::npos; },
                        5));

    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->ItemUncheck("**/##noteshuffle");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.view_noteshuffle; }, 5));
    IM_CHECK(!hydra::app::Settings::load_file(h.ini_path).view_noteshuffle);
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Expert Pro Drums, 2x Bass");
    // The open popup outlives the test, so the next test could hover nothing.
    close_settings_panel(ctx);
    IM_CHECK(wait_until(ctx, [&] { return !compare_disabled(); }, 5));
}

// A fresh app with every analysis setting at its default, the settings button
// reading so. The scratch INI keeps a smaller score range; this puts the
// default one back.
void reset_app_at_default_settings(ImGuiTestContext* ctx, Harness& h) {
    reset_app(h);
    h.app->settings.depth_value = hydra::app::Settings{}.depth_value;
    h.app->commit_settings();
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Analysis settings: defaults"); }, 5));
}

// The settings button (settings summary button handoff, 2026-10-09): it
// reads "defaults" with every setting at its default, its label follows a
// ticked box, and the panel stays open after the tick.
void test_settings_button(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app_at_default_settings(ctx, h);

    ImGuiWindow* panel = open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->ItemCheck("**/##noteshuffle");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Analysis settings: Note Shuffle"); }, 5));
    // Ticking a box leaves the panel open, still the window with focus.
    ctx->Yield(2);
    IM_CHECK(ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel));
    IM_CHECK(ctx->GetWindowByRef("//$FOCUSED") == panel);
    ctx->ItemUncheck("**/##noteshuffle");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Analysis settings: defaults"); }, 5));
    close_settings_panel(ctx);
}

// Two of the vendored test engine's Hydra patches (THIRD_PARTY_NOTICES.txt
// lists them): a "//$FOCUSED/**/" ref finds an item in the focused popup, and
// a wildcard check that finds nothing leaves later wildcard refs able to find
// items that cannot be ticked, such as a button.
void test_engine_wildcard_patches(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app_at_default_settings(ctx, h);
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    const ImGuiID bass2x = ctx->ItemInfo("**/##bass2x").ID;  // under the panel, open_settings_panel's SetRef
    IM_CHECK(bass2x != 0);
    IM_CHECK(ctx->ItemInfo("//$FOCUSED/**/##bass2x", ImGuiTestOpFlags_NoError).ID == bass2x);

    ctx->ItemCheck("**/###nosuchitem", ImGuiTestOpFlags_NoError);
    IM_CHECK(ctx->ItemInfo("//Hydra/**/###analysissettings", ImGuiTestOpFlags_NoError).ID != 0);
    close_settings_panel(ctx);
}

// "Reset to defaults" in the settings panel: greyed out at the defaults; after
// two changes one click puts both back, saves the INI, keeps the folders, and
// leaves the panel open with the button greyed again.
void test_settings_reset(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app_at_default_settings(ctx, h);
    auto reset_off = [&] {
        return (ctx->ItemInfo("**/Reset to defaults").ItemFlags & ImGuiItemFlags_Disabled) != 0;
    };
    const std::vector<std::string> folders = h.app->settings.chartfolders;

    ImGuiWindow* panel = open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(reset_off());
    ctx->ItemUncheck("**/##prodrums");
    ctx->ItemInputValue("**/##spcap", 5);
    IM_CHECK(wait_until(
        ctx,
        [&] { return on_screen(h, "Analysis settings: Pro Drums off \xC2\xB7 SP cap 5 bars"); },
        5));
    IM_CHECK(!reset_off());

    ctx->ItemClick("**/Reset to defaults");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Analysis settings: defaults"); }, 5));
    IM_CHECK(h.app->settings.view_prodrums);
    IM_CHECK(h.app->settings.sp_cap == hydra::kCloneHeroSpCap);
    IM_CHECK(h.app->settings.chartfolders == folders);
    const hydra::app::Settings saved = hydra::app::Settings::load_file(h.ini_path);
    IM_CHECK(saved.view_prodrums);
    IM_CHECK(saved.sp_cap == hydra::kCloneHeroSpCap);
    ctx->Yield(2);
    IM_CHECK(ctx->GetWindowByRef("//$FOCUSED") == panel);
    IM_CHECK(reset_off());
    close_settings_panel(ctx);
}

// A bad hydra_rules.ini: the app still opens and scans, the error naming the
// key stays on screen, Analyze library is disabled, and a click analyzes
// nothing: the song's row stays Not analyzed (D87 item 9).
void test_rules_error(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h, "max_tied_paths = 0\n");
    IM_CHECK(h.app->analysis_blocked());
    IM_CHECK(h.app->rules_error.find("max_tied_paths") != std::string::npos);
    scan_library(ctx);
    if (ctx->IsError()) return;

    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("analysis is off") != std::string::npos;
    }, 5));
    IM_CHECK(visible_text(h).find("max_tied_paths") != std::string::npos);
    IM_CHECK((ctx->ItemInfo("Analyze library...").ItemFlags & ImGuiItemFlags_Disabled) != 0);

    open_details(ctx, 0);
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->viewed.state == hydra::ui::ViewedSong::State::RulesBroken;
    }, 60));
    IM_CHECK(!h.app->viewed.record.has_value());
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::NotAnalyzed);
    // The state refuses as well: a batch starts nothing.
    h.app->start_batch(false);
    IM_CHECK(h.app->batch_job == nullptr);
}

// The library view's own state (the search text, the dmleaderboards filter,
// the status fade, the folder confirm) belongs to the AppState. When it lived
// in function statics, the next test's fresh app still showed the last test's
// search text and filter.
void test_library_state_per_app(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", "zzqx");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "zzqx"; }, 5));
    ctx->ItemClick("Compare with dmleaderboards...");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->dm_users.empty(); }, 10));
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemInputValue("##dmfilter", "zzqx");
    ctx->Yield(2);
    IM_CHECK(visible_text(h).find("alice") == std::string::npos);
    ctx->ItemClick("Close");
    ctx->Yield(2);

    // A fresh app: both boxes start empty. ImGui's text log shows an input
    // box's contents, so leftover text would be on screen.
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->search.empty());
    IM_CHECK(visible_text(h).find("zzqx") == std::string::npos);
    ctx->SetRef("//Hydra");
    ctx->ItemClick("Compare with dmleaderboards...");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->dm_users.empty(); }, 10));
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("alice") != std::string::npos;
    }, 5));
    IM_CHECK(visible_text(h).find("zzqx") == std::string::npos);
    ctx->SetRef("//Compare dmleaderboards user");
    ctx->ItemClick("Close");
    ctx->Yield(2);
}

// The View row and the library's own controls: Pro Drums, backing out of
// "Analyze library...", and removing a song folder.
void test_view_settings(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    // Pro Drums off is a different chart mode: persisted at once.
    IM_CHECK(h.app->settings.view_prodrums);
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("**/##prodrums");
    IM_CHECK(!h.app->settings.view_prodrums);
    IM_CHECK(!hydra::app::Settings::load_file(h.ini_path).view_prodrums);
    IM_CHECK(h.app->settings.chartmode_key().find("Pro Drums") == std::string::npos);
    ctx->ItemClick("**/##prodrums");
    IM_CHECK(h.app->settings.view_prodrums);
    close_settings_panel(ctx);

    // "Analyze library..." asks first; Cancel starts nothing.
    ctx->ItemClick("Analyze library...");
    ctx->SetRef("//Analyze library");
    IM_CHECK(visible_text(h).find("no result yet") != std::string::npos);
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK(h.app->batch_job == nullptr);
    IM_CHECK(!h.app->batch_confirm_pending);

    // Removing a song folder goes through a confirm. Cancel keeps it;
    // Remove drops it, persists that, and turns the scan button off.
    ctx->SetRef("//Hydra");
    IM_CHECK_EQ(h.app->settings.chartfolders.size(), (size_t)1);
    ctx->ItemClick("Manage folders... (1)");
    ctx->SetRef("//Song folders");
    ctx->ItemClick("**/X");
    ctx->SetRef("//Remove folder?");
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
    IM_CHECK_EQ(h.app->settings.chartfolders.size(), (size_t)1);
    ctx->SetRef("//Song folders");
    ctx->ItemClick("**/X");
    ctx->SetRef("//Remove folder?");
    ctx->ItemClick("Remove");
    ctx->Yield(2);
    IM_CHECK(h.app->settings.chartfolders.empty());
    IM_CHECK(hydra::app::Settings::load_file(h.ini_path).chartfolders.empty());
    IM_CHECK(!h.app->settings.is_rescan);
    ctx->SetRef("//Song folders");
    ctx->ItemClick("Close");
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK((ctx->ItemInfo("Scan library").ItemFlags & ImGuiItemFlags_Disabled) != 0);
}

ImGuiTable* library_table();  // below, beside the column tests

// The search box, the chips, the second line and the empty state, on the
// scratch library (97 charts in testdata\input).
void test_library_search(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    const auto shown = [&] { return h.app->library_shown_count(); };

    IM_CHECK_EQ(shown(), (size_t)97);
    IM_CHECK(visible_text(h).find("97 charts") != std::string::npos);
    IM_CHECK(ctx->ItemExists("**/All (97)##chipall"));
    IM_CHECK(ctx->ItemExists("**/Not analyzed (97)##chipnew"));
    IM_CHECK((ctx->ItemInfo("**/Stale (0)##chipstale").ItemFlags & ImGuiItemFlags_Disabled) != 0);

    // A quoted phrase: the five charts under "...\Tier 4".
    ctx->ItemInputValue("**/##search", "\"tier 4\"");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 5; }, 5));
    IM_CHECK(ctx->ItemExists("**/All (5)##chipall"));
    std::string text = visible_text(h);
    IM_CHECK(text.find("5 of 97 charts") != std::string::npos);
    for (const char* title : {"Burnout", "Chair", "Limb From Limb", "Unbound (The Wild Ride)", "YYZ"})
        IM_CHECK(text.find(title) != std::string::npos);

    // With a song open, Folder makes way, so each row shows its folder on
    // the title's second line. No note under the table says where it matched.
    h.app->select(h.app->library_row_at(0).entry, hydra::ui::SelectionSource::Click);
    IM_CHECK(wait_until(ctx, [&] { return !ctx->ItemExists("**/Folder"); }, 5));
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, "Matched on"));
    h.app->close_details();
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Folder"); }, 5));

    // album: limits a word to the album. Haken - The Good Doctor's song.ini
    // says album = Vector.
    ctx->ItemInputValue("**/##search", "album:vector");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "album:vector"; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return shown() == 1; }, 5));
    IM_CHECK_EQ(shown(), (size_t)1);
    IM_CHECK(h.app->library_row_at(0).title == "The Good Doctor");
    IM_CHECK(ctx->ItemExists("**/Album"));
    // With a song open, Album makes way with Charter and Folder, and the
    // album shows on the row's second line instead.
    h.app->select(h.app->library_row_at(0).entry, hydra::ui::SelectionSource::Click);
    IM_CHECK(wait_until(ctx, [&] { return !ctx->ItemExists("**/Album"); }, 5));
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Vector"); }, 5));
    // The user's case: the song stays open and the search box is emptied.
    // Every row still shows its album under the artist.
    ctx->ItemInputValue("**/##search", "");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search.empty() && shown() == 97; }, 5));
    ctx->Yield(2);
    IM_CHECK(!ctx->ItemExists("**/Album"));
    IM_CHECK(on_screen(h, "Vector"));
    const float open_no_search_h =
        ctx->ItemInfo(("**/" + escape_ref(h.app->library_row_at(0).title)).c_str())
            .RectFull.GetHeight();
    h.app->close_details();
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Album"); }, 5));

    // Only Album hidden, from the header menu: the rows grow a second line
    // that carries the album under Artist. Folder is still a column, so the
    // title's second line stays empty and the folder is drawn once.
    ImGuiTable* table = library_table();
    IM_CHECK_RETV(table != nullptr, );
    const std::string doctor_ref = "**/The Good Doctor";
    const float one_line_h = [&] {
        ctx->ItemInputValue("**/##search", "");
        wait_until(ctx, [&] { return h.app->search.empty(); }, 5);
        return ctx->ItemInfo(("**/" + escape_ref(h.app->library_row_at(0).title)).c_str())
            .RectFull.GetHeight();
    }();
    // Album shown, no search: one line. Album hidden by the open song: two.
    IM_CHECK_GT(open_no_search_h, one_line_h * 1.5f);
    ctx->TableSetColumnEnabled(table->ID, "Album", false);
    ctx->Yield(3);
    IM_CHECK(!table->Columns[hydra::ui::kColumnAlbum].IsEnabled);
    ctx->ItemInputValue("**/##search", "album:vector");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 1; }, 5));
    ctx->Yield(2);
    const float two_line_h = ctx->ItemInfo(doctor_ref.c_str()).RectFull.GetHeight();
    IM_CHECK_GT(two_line_h, one_line_h * 1.5f);
    IM_CHECK_EQ(count_on_screen(h, "Vector"), (size_t)1);
    IM_CHECK_EQ(count_on_screen(h, h.app->library_row_at(0).entry.rootfolder), (size_t)1);
    // The album sits in the Artist cell: the frame's text runs title, artist,
    // album line, then the Folder column.
    {
        const std::string all = visible_text(h);
        const hydra::ui::LibraryRow& doctor = h.app->library_row_at(0);
        const size_t title_at = all.find(doctor.title);
        const size_t artist_at = all.find(doctor.artist, title_at);
        const size_t album_at = all.find("Vector", artist_at);
        const size_t folder_at = all.find(doctor.entry.rootfolder, artist_at);
        IM_CHECK(title_at != std::string::npos && artist_at != std::string::npos);
        IM_CHECK(album_at != std::string::npos && folder_at != std::string::npos);
        IM_CHECK_LT(artist_at, album_at);
        IM_CHECK_LT(album_at, folder_at);
    }
    // No search, Album still hidden: the rows stay two lines tall and the
    // album is still under the artist.
    ctx->ItemInputValue("**/##search", "");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search.empty(); }, 5));
    ctx->Yield(2);
    const std::string first_ref = "**/" + escape_ref(h.app->library_row_at(0).title);
    IM_CHECK_EQ(ctx->ItemInfo(first_ref.c_str()).RectFull.GetHeight(), two_line_h);
    IM_CHECK(on_screen(h, "Vector"));
    // Album back as a column: one line, and the album is drawn once.
    ctx->TableSetColumnEnabled(table->ID, "Album", true);
    ctx->Yield(3);
    IM_CHECK_EQ(ctx->ItemInfo(first_ref.c_str()).RectFull.GetHeight(), one_line_h);
    IM_CHECK_EQ(count_on_screen(h, "Vector"), (size_t)1);

    // Words in any order, across title and artist.
    ctx->ItemInputValue("**/##search", "green burnout");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 1; }, 5));
    IM_CHECK(h.app->library_row_at(0).title == "Burnout");

    // A charter stored with colour tags is found and drawn without them.
    // Bloodline charted 15 of the scratch charts (13 of them tagged), so
    // every row shown is theirs, Acid Romance among them.
    ctx->ItemInputValue("**/##search", "bloodline");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "bloodline" && shown() > 1; }, 5));
    bool acid_shown = false;
    for (size_t i = 0; i < shown(); ++i) {
        IM_CHECK(h.app->library_row_at(i).charter == "Bloodline");
        acid_shown = acid_shown || h.app->library_row_at(i).title == "Acid Romance";
    }
    IM_CHECK(acid_shown);
    text = visible_text(h);
    IM_CHECK(text.find("Acid Romance") != std::string::npos);
    IM_CHECK(text.find("Bloodline") != std::string::npos);
    IM_CHECK(text.find("<color=") == std::string::npos);

    // A filter it can't read says so under the box.
    ctx->ItemInputValue("**/##search", "stars:9");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->library.query().errors.empty(); }, 5));
    IM_CHECK(visible_text(h).find(h.app->library.query().errors[0]) != std::string::npos);

    // Nothing matches: the empty state, with no button of its own. The X by
    // the box is the one way to clear it by mouse, and it brings it all back.
    ctx->ItemInputValue("**/##search", "zzqx");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 0; }, 5));
    IM_CHECK(visible_text(h).find("No charts match your search.") != std::string::npos);
    IM_CHECK(!ctx->ItemExists("**/Clear search"));
    ctx->ItemClick("**/X##clearsearch");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 97 && h.app->search.empty(); }, 5));

    // Escape in the box clears it; a second Escape leaves the box.
    ctx->ItemInputValue("**/##search", "chair");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "chair"; }, 5));
    ctx->ItemClick("**/##search");
    ctx->KeyPress(ImGuiKey_Escape);
    IM_CHECK(wait_until(ctx, [&] { return h.app->search.empty(); }, 5));
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);

    // Ctrl+F puts the cursor in the box.
    ctx->KeyPress(ImGuiMod_Ctrl | ImGuiKey_F);
    ctx->Yield(2);
    IM_CHECK_EQ(ctx->UiContext->ActiveId, ctx->ItemInfo("**/##search").ID);
    ctx->KeyPress(ImGuiKey_Escape);
    ctx->Yield(2);
}

// D115: the Analyze button follows the status chip. With no search, a chip
// alone narrows the table, and the button then reads "Analyze shown (N)..."
// with the heading's N, and the confirm counts those N.
void test_analyze_follows_chip(ImGuiTestContext* ctx) {
    using hydra::ui::StatusChip;
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    IM_CHECK(ctx->ItemExists("Analyze library..."));

    // One chart gets a result, so the Not analyzed chip leaves rows out.
    IM_CHECK(batch_search(ctx, "Burnout"));
    dismiss_done(ctx);
    ctx->ItemInputValue("**/##search", "");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search.empty(); }, 5));
    const size_t total = h.app->library.rows().size();
    const size_t not_analyzed = h.app->library.counts().not_analyzed;
    IM_CHECK_GT(not_analyzed, size_t{0});
    IM_CHECK_LT(not_analyzed, total);
    IM_CHECK(ctx->ItemExists("Analyze library..."));

    ctx->ItemClick(("**/" + hydra::ui::chip_label(StatusChip::NotAnalyzed, not_analyzed)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->library_shown_count() == not_analyzed; }, 5));
    const std::string heading =
        std::to_string(not_analyzed) + " of " + std::to_string(total) + " charts";
    IM_CHECK(visible_text(h).find(heading) != std::string::npos);
    const std::string label =
        hydra::ui::detail::analyze_shown_label(static_cast<int64_t>(not_analyzed));
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists(label.c_str()); }, 5));
    IM_CHECK(!ctx->ItemExists("Analyze library..."));

    ctx->ItemClick(label.c_str());
    ctx->Yield(2);
    IM_CHECK(h.app->batch_confirm_pending);
    IM_CHECK_EQ(h.app->batch_scope_charts(), static_cast<int64_t>(not_analyzed));
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Cancel");
    ctx->Yield(2);

    // Back to All: the whole library, and the button says so.
    ctx->SetRef("//Hydra");
    ctx->ItemClick(("**/" + hydra::ui::chip_label(StatusChip::All, total)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("Analyze library..."); }, 5));
}

// A chart whose failure the store kept (failure_record, saved on the scratch
// db the way a batch saves one) reads Can't analyze in the Best path cell,
// counts under its own chip, and that chip narrows the table and the Analyze
// button to it (D115). The corpus has no chart without Expert drums, so the
// failure is saved directly.
void test_library_cant_analyze(ImGuiTestContext* ctx) {
    using hydra::ui::StatusChip;
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    const size_t total = h.app->library.rows().size();
    IM_CHECK((ctx->ItemInfo(("**/" + hydra::ui::chip_label(StatusChip::CantAnalyze, 0)).c_str())
                  .ItemFlags &
              ImGuiItemFlags_Disabled) != 0);

    const std::string md5 = h.app->library_row_at(0).entry.md5;
    const hydra::NoNotesError no_notes(hydra::Difficulty::Expert, true);
    h.app->store->save(hydra::store::failure_record(md5, h.app->settings.profile(),
                                                    hydra::ErrorKind::NoNotes, no_notes.what()));
    h.app->refresh_library_summaries();
    IM_CHECK_EQ(h.app->library.counts().cant_analyze, size_t{1});
    IM_CHECK_EQ(h.app->library.counts().all, total);
    const std::string chip = "**/" + hydra::ui::chip_label(StatusChip::CantAnalyze, 1);
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists(chip.c_str()); }, 5));
    IM_CHECK((ctx->ItemInfo(chip.c_str()).ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(ctx->ItemExists(
        ("**/" + hydra::ui::chip_label(StatusChip::NotAnalyzed, total - 1)).c_str()));

    ctx->ItemClick(chip.c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->library_shown_count() == 1; }, 5));
    IM_CHECK(h.app->library_row_at(0).entry.md5 == md5);
    IM_CHECK(h.app->library_row_at(0).status == hydra::store::RecordStatus::CantAnalyze);
    // The cell draws the word: the chip's label and the cell both show it.
    const std::string word = hydra::ui::status_label(hydra::store::RecordStatus::CantAnalyze);
    const std::string text = visible_text(h);
    const size_t first = text.find(word);
    IM_CHECK(first != std::string::npos);
    IM_CHECK(text.find(word, first + word.size()) != std::string::npos);

    // The Analyze button counts the one chart on screen.
    const std::string label = hydra::ui::detail::analyze_shown_label(1);
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists(label.c_str()); }, 5));
    ctx->ItemClick(label.c_str());
    ctx->Yield(2);
    IM_CHECK(h.app->batch_confirm_pending);
    IM_CHECK_EQ(h.app->batch_scope_charts(), int64_t{1});
    ctx->SetRef("//Analyze library");
    ctx->ItemClick("Cancel");
    ctx->Yield(2);
}

// Sorting and scrolling: every chart is one scroll away, and only the rows on
// screen are drawn.
void test_library_sort_scroll(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");

    IM_CHECK(h.app->library.sort_column() == hydra::ui::LibrarySort::Title);
    IM_CHECK(h.app->library.ascending());
    const size_t count = h.app->library_shown_count();
    const std::string first = h.app->library_row_at(0).title;
    const std::string last = h.app->library_row_at(count - 1).title;
    IM_CHECK(visible_text(h).find(first) != std::string::npos);
    IM_CHECK(visible_text(h).find("Not analyzed") != std::string::npos);
    IM_CHECK(visible_text(h).find("-----") == std::string::npos);  // no filler rows

    // The table scrolls like any list: its end brings the last title into
    // view. (The harness's text log turns the row clipper off, so every row
    // is submitted while testing; "in view" is judged by the row's place
    // against the table's visible area instead of by what was drawn.)
    ImGuiWindow* table = nullptr;
    for (ImGuiWindow* w : ImGui::GetCurrentContext()->Windows)
        if ((w->Flags & ImGuiWindowFlags_ChildWindow) && std::strstr(w->Name, "##librarytable"))
            table = w;
    IM_CHECK(table != nullptr);
    if (table == nullptr) return;
    const std::string last_ref = "**/" + escape_ref(last);
    const auto last_in_view = [&] {
        ImGuiTestItemInfo info = ctx->ItemInfo(last_ref.c_str(), ImGuiTestOpFlags_NoError);
        return info.ID != 0 && table->InnerRect.Contains(info.RectFull.GetCenter());
    };
    IM_CHECK(!last_in_view());  // below the fold
    ctx->ScrollToBottom(ImGuiTestRef(table->ID));
    IM_CHECK(wait_until(ctx, last_in_view, 5));

    // The Title header reverses the order; a second click restores it (the
    // ImGui context, and so the table's sort, outlives this test's app).
    ctx->ItemClick("**/Title");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->library.ascending(); }, 5));
    IM_CHECK(h.app->library_row_at(0).title == last);
    ctx->ItemClick("**/Title");
    IM_CHECK(wait_until(ctx, [&] { return h.app->library.ascending(); }, 5));
    IM_CHECK(h.app->library_row_at(0).title == first);
}

// The Title column's table, found through its scrolling child window.
ImGuiTable* library_table() {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    for (int i = 0; i < g.Tables.GetMapSize(); ++i)
        if (ImGuiTable* t = g.Tables.TryGetMapData(i))
            if (t->InnerWindow && std::strstr(t->InnerWindow->Name, "##librarytable"))
                return t;
    return nullptr;
}

// The settings panel's Score range box, the chips and the Title column fit
// their room, at the narrowest library and on long data. Geometry, not text:
// the text log records a cut-off string in full.
void test_library_layout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    IM_CHECK_EQ(h.width, 1280);

    // A long title opens the song panel, clicked by its title as every test
    // finds a row; then the library goes as narrow as it gets.
    const std::string title = "Tapestry of the Starless Abstract (Shortened)";
    ctx->SetRef("//Hydra");
    ctx->ItemClick(("**/" + escape_ref(title)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->selected && h.app->selected->title == title; },
                        5));
    hydra::ui::remember_library_share(0.01f);
    h.app->library_ui.panel_was_open = false;
    ctx->Yield(3);

    // Score range holds six digits beside its step buttons, in the open
    // settings panel. The sample widest_digits gives is one measured run in
    // the shipped font at this size (audit finding 112).
    h.app->settings.depth_value = 999999;
    ctx->Yield(2);
    // The click analyzed (and saved) this song (D87), and the setting change
    // re-analyzes it (D90): let that settle, so the chip counts below stand still.
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, 300));
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    const std::string widest = hydra::ui::widest_digits(6);
    IM_CHECK_STR_EQ(widest.c_str(), "000000");
    IM_CHECK_GE(ctx->ItemInfo("**/##depthvalue").RectFull.GetWidth(),
                hydra::ui::button_slot_width(widest.c_str()));
    IM_CHECK_LE(ctx->ItemInfo("**/##depthvalue/+").RectFull.Max.x,
                ctx->ItemInfo(settings_control(ctx, "##mslimit", "##depthmode")).RectFull.Min.x);
    close_settings_panel(ctx);

    // The four chips stay inside the library, wrapping as they must: at
    // 320 px "Analyzed (0)" no longer fits after the other three.
    ImGuiWindow* lib = ctx->WindowInfo("//Hydra/##library").Window;
    IM_CHECK(lib != nullptr);
    if (lib == nullptr) return;
    IM_CHECK_LE(lib->Size.x, hydra::ui::px(hydra::ui::kMinLibraryW) + 1.0f);
    ctx->SetRef("//Hydra");
    // The labels carry the library's own counts (what the chips draw), which
    // moved when the click analyzed the open song.
    using hydra::ui::StatusChip;
    const StatusChip chip_order[] = {StatusChip::All, StatusChip::NotAnalyzed, StatusChip::Stale,
                                     StatusChip::Analyzed};
    std::string chips[4];
    for (size_t i = 0; i < 4; ++i)
        chips[i] = "**/" + hydra::ui::chip_label(chip_order[i],
                                                  h.app->library.counts().of(chip_order[i]));
    for (const std::string& chip : chips)
        IM_CHECK_LE(ctx->ItemInfo(chip.c_str()).RectFull.Max.x,
                    lib->ContentRegionRect.Max.x + 0.5f);
    IM_CHECK_GT(ctx->ItemInfo(chips[3].c_str()).RectFull.Min.y,
                ctx->ItemInfo(chips[0].c_str()).RectFull.Min.y);

    // The long title ends inside its cell: nothing in the Title column lays
    // out past the column's edge, while the row stays one click target
    // across every column.
    ImGuiTable* table = library_table();
    IM_CHECK(table != nullptr);
    if (table == nullptr) return;
    const ImGuiTableColumn& title_col = table->Columns[0];
    IM_CHECK_LT(title_col.WorkMaxX - title_col.WorkMinX, ImGui::CalcTextSize(title.c_str()).x);
    IM_CHECK_LE(title_col.ContentMaxXUnfrozen, title_col.WorkMaxX + 0.5f);
    const ImRect row = ctx->ItemInfo(("**/" + escape_ref(title)).c_str()).RectFull;
    IM_CHECK_GE(row.Max.x, table->Columns[table->RightMostEnabledColumn].WorkMaxX);
}

// Every column is as wide as the one width rule (ui/column_widths.h) says for
// the scanned rows. The expected widths come from the production
// place_columns on the table's own measure and room, never worked out here.
// At the narrowest library, with all six columns on, Title stops at its
// header's width and the table scrolls sideways instead.
void test_library_column_widths(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(3);  // the table settles, then takes the rule's widths
    ImGuiTable* table = library_table();
    IM_CHECK_RETV(table != nullptr && table->InnerWindow != nullptr, );
    const hydra::ui::LibraryViewState::ColumnWidths& cache = h.app->library_ui.column_widths;
    IM_CHECK_EQ(cache.specs.size(), static_cast<size_t>(hydra::ui::kLibraryColumnCount));
    IM_CHECK(hydra::ui::text_shares_sum_to_100(cache.specs));  // the real list
    // The headers left to right: Album sits between Artist and Charter.
    const char* const headers[] = {"Title", "Artist", "Album", "Charter", "Folder", "Best path"};
    IM_CHECK_EQ(table->ColumnsCount, static_cast<int>(std::size(headers)));
    for (int c = 0; c < table->ColumnsCount && c < static_cast<int>(std::size(headers)); ++c)
        IM_CHECK_STR_EQ(ImGui::TableGetColumnName(table, c), headers[c]);
    // A header is measured in the font it is drawn in. The Best path cells
    // draw as Notation, but TableHeadersRow draws the header as Label.
    IM_CHECK_EQ(cache.measured.header_widths[hydra::ui::kColumnBestPath],
                hydra::ui::text_size(hydra::ui::TextRole::Label, "Best path").x);
    hydra::ui::ColumnLayout layout;
    auto check_rule_widths = [&] {
        layout = hydra::ui::place_columns(cache.measured, cache.specs, cache.room);
        for (int c = 0; c < table->ColumnsCount; ++c) {
            if (!table->Columns[c].IsEnabled) continue;
            ctx->LogInfo("column %d: %.1f, rule %.1f", c, table->Columns[c].WidthGiven,
                         layout.widths[static_cast<size_t>(c)]);
            IM_CHECK_EQ(table->Columns[c].WidthGiven, layout.widths[static_cast<size_t>(c)]);
        }
    };
    for (int c = 0; c < table->ColumnsCount; ++c) IM_CHECK(table->Columns[c].IsEnabled);
    check_rule_widths();
    if (ctx->IsError()) return;
    IM_CHECK(!table->InnerWindow->ScrollbarX);  // the wide library fits every column

    // The song panel opens and the library goes as narrow as it gets. The
    // panel hides Album, Charter and Folder; they come back from the header
    // menu, so the six headers alone are wider than the library.
    ctx->SetRef("//Hydra");
    const std::string title = h.app->library_row_at(0).title;
    ctx->ItemClick(("**/" + escape_ref(title)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->selected && h.app->selected->title == title; },
                        5));
    hydra::ui::remember_library_share(0.01f);
    h.app->library_ui.panel_was_open = false;  // the split sets its width next frame
    ctx->Yield(3);
    // Each enable opens the header menu on the rightmost header. The columns
    // take their new widths over a few frames, so the table settles before
    // the next click looks for that header.
    ctx->TableSetColumnEnabled(table->ID, "Album", true);
    ctx->Yield(3);
    ctx->TableSetColumnEnabled(table->ID, "Charter", true);
    ctx->Yield(3);
    ctx->TableSetColumnEnabled(table->ID, "Folder", true);
    ctx->Yield(3);
    for (int c = 0; c < table->ColumnsCount; ++c) IM_CHECK(table->Columns[c].IsEnabled);
    check_rule_widths();
    if (ctx->IsError()) return;
    const float title_w = table->Columns[hydra::ui::kColumnTitle].WidthGiven;
    IM_CHECK_GE(title_w, cache.measured.header_widths[hydra::ui::kColumnTitle]);
    IM_CHECK_EQ(layout.inner_width, layout.min_inner_width);  // every text column at its floor
    IM_CHECK_GT(layout.inner_width, cache.room.available);
    IM_CHECK(table->InnerWindow->ScrollbarX);
}

// Every shown column's width as ImGui gave it this frame.
std::vector<float> given_widths(const ImGuiTable* table) {
    std::vector<float> out;
    for (int c = 0; c < table->ColumnsCount; ++c)
        out.push_back(table->Columns[c].IsEnabled ? table->Columns[c].WidthGiven : -1.0f);
    return out;
}

// What the cells say never moves a column (the user's decision 1, 2026-10-14).
// A batch turns every Best path cell from "Not analyzed" into "<score>  <path>",
// and turning 2x Bass off turns them back; the widths stay put both times.
void test_library_widths_hold(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(3);  // the table settles, then takes the rule's widths
    ImGuiTable* table = library_table();
    IM_CHECK_RETV(table != nullptr, );
    const std::vector<float> before = given_widths(table);
    const auto check_held = [&](const char* when) {
        const std::vector<float> now = given_widths(table);
        for (size_t c = 0; c < before.size(); ++c) {
            ctx->LogInfo("%s, column %zu: %.1f, before %.1f", when, c, now[c], before[c]);
            IM_CHECK_EQ(now[c], before[c]);
        }
    };

    // A batch over the corpus fills the Best path cells.
    start_library_batch(ctx);
    IM_CHECK_RETV(wait_until(ctx, [&] {
        return h.app->batch_job && h.app->batch_job->snapshot().finished;
    }, 300), );
    dismiss_done(ctx);
    const size_t analyzed = h.app->library.counts().analyzed;
    IM_CHECK_GT(analyzed, size_t{0});
    ctx->Yield(3);
    check_held("after the batch");

    // 2x Bass off: no record matches the new settings, so the cells change back.
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->settings.view_bass2x);  // the default the test relies on
    ctx->ItemClick("**/##bass2x");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.view_bass2x; }, 5));
    close_settings_panel(ctx);
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->view_settled() && h.app->library.counts().analyzed != analyzed;
    }, 30));
    ctx->Yield(3);
    check_held("after 2x Bass off");

    // And back on, which brings the batch's results back.
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->ItemClick("**/##bass2x");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_bass2x; }, 5));
    close_settings_panel(ctx);
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->view_settled() && h.app->library.counts().analyzed == analyzed;
    }, 30));
    ctx->Yield(3);
    check_held("after 2x Bass on");
}

// The Library's scrolling child window.
ImGuiWindow* library_table_window() {
    ImGuiTable* table = library_table();
    return table ? table->InnerWindow : nullptr;
}

// Whether no other shown row has row k's title (rows are found by title).
bool title_is_unique(Harness& h, size_t k) {
    const std::string& title = h.app->library_row_at(k).title;
    size_t same = 0;
    for (size_t i = 0; i < h.app->view_row_count(); ++i)
        if (h.app->library_row_at(i).title == title) ++same;
    return same == 1;
}

// Row k of the table (in view order), found by its title as every test finds
// a row. A title two shown rows share would find the wrong one, so that fails.
ImRect row_rect(ImGuiTestContext* ctx, size_t k) {
    Harness& h = harness(ctx);
    IM_CHECK_RETV(title_is_unique(h, k), ImRect());
    const std::string& title = h.app->library_row_at(k).title;
    return ctx->ItemInfo(("//Hydra/**/" + escape_ref(title)).c_str()).RectFull;
}

// The first shown row, from row `from` on, that sits wholly in the lower half
// of the list's visible area; none when there is none.
std::optional<size_t> row_in_lower_half(ImGuiTestContext* ctx, const ImGuiWindow* list,
                                        size_t from) {
    Harness& h = harness(ctx);
    const float middle = list->InnerRect.GetCenter().y;
    for (size_t k = from; k < h.app->view_row_count(); ++k) {
        if (!title_is_unique(h, k)) continue;
        const ImRect r = row_rect(ctx, k);
        if (r.Min.y > middle && fully_shown(list, r)) return k;
    }
    return std::nullopt;
}

// A click on a row selects it and leaves the list where it is (the user's
// rule, 2026-10-14: a click never scrolls). The clicked row sits in the lower
// half, where centring it would move the list.
void test_library_click_keeps_scroll(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(3);
    ImGuiWindow* list = library_table_window();
    IM_CHECK_RETV(list != nullptr, );
    IM_CHECK_GT(list->ScrollMax.y, 0.0f);  // the corpus is taller than the list
    ImGui::SetScrollY(list, IM_TRUNC(list->ScrollMax.y * 0.5f));
    ctx->Yield(3);
    const float scroll = list->Scroll.y;
    IM_CHECK_GT(scroll, 0.0f);

    const std::optional<size_t> k = row_in_lower_half(ctx, list, 0);
    IM_CHECK_RETV(k.has_value(), );
    const std::string title = h.app->library_row_at(*k).title;
    ctx->SetRef("//Hydra");
    ctx->ItemClick(("**/" + escape_ref(title)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->selected && h.app->selected->title == title; },
                        5));
    ctx->Yield(3);
    ctx->LogInfo("scroll before the click %.1f, after %.1f", scroll, list->Scroll.y);
    IM_CHECK_EQ(list->Scroll.y, scroll);
}

// Next in the song panel moves the list only for a row that is not wholly on
// screen, and only until that row's bottom meets the list's bottom edge (the
// user's answer 4). Previous to a row already on screen moves nothing.
void test_library_next_scrolls_just_enough(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(3);
    ImGuiWindow* list = library_table_window();
    IM_CHECK_RETV(list != nullptr, );

    // The panel opens on a row in the lower half of the view.
    const std::optional<size_t> start = row_in_lower_half(ctx, list, 0);
    IM_CHECK_RETV(start.has_value(), );
    const std::string title = h.app->library_row_at(*start).title;
    ctx->SetRef("//Hydra");
    ctx->ItemClick(("**/" + escape_ref(title)).c_str());
    IM_CHECK(wait_until(ctx, [&] { return h.app->selected && h.app->selected->title == title; },
                        5));
    ctx->Yield(3);

    // Next, one row at a time, until the list moves. Every row it passes on
    // the way was on screen, so the list holds still for those. The cap is
    // the corpus's row count: the list must move before Next runs out.
    const size_t rows = h.app->view_row_count();
    std::optional<size_t> moved_at;
    for (size_t k = *start + 1; k < rows && !ctx->IsError(); ++k) {
        const float scroll = list->Scroll.y;
        const bool was_shown = fully_shown(list, row_rect(ctx, k));
        ctx->ItemClick("**/>##nextsong");
        IM_CHECK_RETV(wait_until(ctx, [&] {
            return h.app->selected && h.app->selected->notespath == h.app->view_row(k).notespath;
        }, 5), );
        ctx->Yield(3);
        if (list->Scroll.y != scroll) {
            ctx->LogInfo("row %zu moved the list from %.1f to %.1f (it was %s)", k, scroll,
                         list->Scroll.y, was_shown ? "on screen" : "off screen");
            IM_CHECK(!was_shown);
            moved_at = k;
            break;
        }
    }
    IM_CHECK_RETV(moved_at.has_value(), );

    // The row came in at the bottom edge, not the middle.
    const ImRect row = row_rect(ctx, *moved_at);
    const float spacing = ImGui::GetStyle().ItemSpacing.y;
    ctx->LogInfo("row bottom %.1f, list bottom %.1f, spacing %.1f", row.Max.y,
                 list->InnerRect.Max.y, spacing);
    IM_CHECK(fully_shown(list, row));
    IM_CHECK_GE(row.Max.y, list->InnerRect.Max.y - spacing);

    // Previous goes to the row above, which is on screen: nothing moves.
    const float scroll = list->Scroll.y;
    ctx->ItemClick("**/<##prevsong");
    IM_CHECK(wait_until(ctx, [&] {
        return h.app->selected && h.app->selected->notespath == h.app->view_row(*moved_at - 1).notespath;
    }, 5));
    ctx->Yield(3);
    IM_CHECK_EQ(list->Scroll.y, scroll);
}

// A hydra_ui.ini that only records a sort on Best path (what sorting by it,
// quitting and restarting leaves) keeps the columns in their order. Dear
// ImGui (ocornut/imgui#9519) used to move the sorted column to the front.
void test_library_column_order(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ImGuiTable* table = library_table();
    IM_CHECK(table != nullptr);
    if (table == nullptr) return;
    const int best = hydra::ui::kColumnBestPath;
    auto load_sort = [&](int column, char dir) {
        char ini[128];
        std::snprintf(ini, sizeof(ini), "[Table][0x%08X,%d]\nColumn %d  Sort=0%c ID=0x%08X\n",
                      table->ID, hydra::ui::kLibraryColumnCount, column, dir,
                      table->Columns[column].ID);
        // As at startup: this line is the table's only saved entry (a load on
        // top of an existing entry would add a second one ImGui never reads).
        ImGui::ClearIniSettings();
        ImGui::LoadIniSettingsFromMemory(ini);
        ctx->Yield(3);
    };

    load_sort(best, '^');
    IM_CHECK_EQ(table->Columns[best].SortOrder, 0);  // the line loaded
    for (int n = 0; n < table->ColumnsCount; ++n)
        IM_CHECK_EQ(table->Columns[n].DisplayOrder, n);

    // Back to Title ascending: the table (and so its sort) outlives this test.
    load_sort(0, 'v');
    IM_CHECK_EQ(table->Columns[0].SortOrder, 0);
    IM_CHECK_EQ(table->Columns[best].SortOrder, -1);
}

// Charter and Folder show with the song panel closed, even when hydra_ui.ini
// saved them hidden (the column-widths handoff, Finding 1). ImGui reads a
// table's saved state during its first frame, after the table has said which
// columns the panel wants, so the saved "hidden" used to win at every start.
// This rebuilds that start: the table is dropped so its next frame is a first
// one, and the only saved entry hides both columns. Then a saved entry from
// before the Album column still lands on the columns it named.
void test_library_columns_at_start(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    // A first scan draws the table once, for its ID and its columns' IDs.
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    ImGuiTable* table = library_table();
    IM_CHECK_RETV(table != nullptr, );
    std::string ini;
    char line[96];
    std::snprintf(line, sizeof(line), "[Table][0x%08X,%d]\n", table->ID,
                  hydra::ui::kLibraryColumnCount);
    ini += line;
    for (int c = 0; c < table->ColumnsCount; ++c) {
        const bool hidden = c == hydra::ui::kColumnCharter || c == hydra::ui::kColumnFolder;
        std::snprintf(line, sizeof(line), "Column %d  Visible=%d ID=0x%08X\n", c, hidden ? 0 : 1,
                      table->Columns[c].ID);
        ini += line;
    }

    // A fresh app with no rows draws no table. The table goes, and the saved
    // entry is the only one, as at startup; the scan then brings the table
    // back for its first frame.
    const auto start_with = [&](const std::string& saved) {
        reset_app(h);
        if (h.app->details_open()) return false;
        ImGui::TableRemove(table);
        ImGui::ClearIniSettings();
        ImGui::LoadIniSettingsFromMemory(saved.c_str());
        scan_library(ctx);
        if (ctx->IsError()) return false;
        ctx->Yield(3);
        table = library_table();
        return table != nullptr;
    };
    IM_CHECK_RETV(start_with(ini), );
    IM_CHECK(!h.app->details_open());
    IM_CHECK(table->Columns[hydra::ui::kColumnCharter].IsEnabled);
    IM_CHECK(table->Columns[hydra::ui::kColumnFolder].IsEnabled);

    // A hydra_ui.ini saved before Album existed: five columns, Artist hidden
    // and the sort on Best path, highest first. ImGui keys each saved column
    // by its header label's hash, so the inserted Album takes no saved line:
    // it starts shown, and the old lines land on the columns they named.
    const char* const old_headers[] = {"Title", "Artist", "Charter", "Folder", "Best path"};
    ini.clear();
    std::snprintf(line, sizeof(line), "[Table][0x%08X,%d]\n", table->ID,
                  static_cast<int>(std::size(old_headers)));
    ini += line;
    for (int c = 0; c < static_cast<int>(std::size(old_headers)); ++c) {
        const std::string_view header = old_headers[c];
        std::snprintf(line, sizeof(line), "Column %d  Visible=%d%s ID=0x%08X\n", c,
                      header == "Artist" ? 0 : 1, header == "Best path" ? " Sort=0^" : "",
                      ImHashStr(old_headers[c]));
        ini += line;
    }
    IM_CHECK_RETV(start_with(ini), );
    IM_CHECK(!table->Columns[hydra::ui::kColumnArtist].IsEnabled);
    IM_CHECK(table->Columns[hydra::ui::kColumnAlbum].IsEnabled);
    IM_CHECK(table->Columns[hydra::ui::kColumnCharter].IsEnabled);
    IM_CHECK(table->Columns[hydra::ui::kColumnFolder].IsEnabled);
    IM_CHECK(table->Columns[hydra::ui::kColumnBestPath].IsEnabled);
    IM_CHECK(h.app->library.sort_column() == hydra::ui::LibrarySort::BestPath);
    IM_CHECK(!h.app->library.ascending());
    // The next test starts from the table's own defaults.
    ImGui::TableRemove(table);
    ImGui::ClearIniSettings();
}

// The first column of the first row `sql` returns, or -1 when it fails.
int64_t number_in_file(const std::string& path, const std::string& sql) {
    return hydra::test::first_int_on_file(path, sql).value_or(-1);
}

// The titles the startup test's library file holds.
constexpr const char* kStartupTitles[] = {"Startup Song One", "Startup Song Two"};

// Writes a library file in the layout Hydra 2.2.0 to 2.7.0 wrote: two charts
// and three results. One result is from before the stars column (a score and
// no stars), which the conversion leaves behind. Returns the file's path, or
// "" if a step failed.
std::string write_v220_library(Harness& h) {
    const std::string path = h.temp_dir + "\\v220_seed.db";
    if (!hydra::store::remove_with_side_files(path)) return "";
    {
        hydra::store::RecordStore seed(path);
        std::vector<hydra::store::ChartLibraryEntry> charts(2);
        for (size_t i = 0; i < charts.size(); ++i) {
            charts[i].md5 = std::string(32, static_cast<char>('a' + i));
            charts[i].title = kStartupTitles[i];
            charts[i].artist = "Startup Artist";
            charts[i].charter = "Startup Charter";
            charts[i].notespath = h.temp_dir + "\\startup" + std::to_string(i) + "\\notes.chart";
            charts[i].rootfolder = h.temp_dir;
        }
        seed.rebuild_chart_library(charts);
    }
    // The record tables go and the results table comes in as 2.2.0 made it,
    // with literal rows.
    const std::string sql =
        std::string("DROP TABLE record_paths; DROP TABLE records; DROP TABLE profiles;") +
        hydra::test::kV220ResultsTableSql +
        ";INSERT INTO results (result_id, hyhash, chartmode, hyversion, sp_cap, ms_enabled,"
        " ms_value, depth_mode, depth_value, legacy_fills, bestpath, score, stars, rules_fp)"
        " VALUES"
        " (1, 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', 'mode', 'v', 8, 0, 0, 0, 2, 0, '1', 1000, 5, x''),"
        " (2, 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb', 'mode', 'v', 8, 0, 0, 0, 2, 0, '1', 2000, 6, x''),"
        " (3, 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb', 'other', 'v', 8, 0, 0, 0, 2, 0, '1', 3000, NULL,"
        " x'');";
    if (!hydra::test::run_sql_on_file(path, sql)) return "";
    return path;
}

// The window keeps drawing while the store opens (the DBUP plan, part 1). A
// 2.2.0 library file makes the open convert its results to records; an
// OpenGate holds that open on its worker thread, and the frames still come
// with the upgrade's screen on them, in the words the user chose for it. Once
// the gate opens, the library lists the file's charts and nothing was set
// aside.
void test_startup_screen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const std::string seed = write_v220_library(h);
    IM_CHECK(!seed.empty());
    const std::string seed_before = h.seed_db;
    h.seed_db = seed;
    OpenGate gate;  // before reset_app: the open starts in AppState's constructor
    reset_app(h, "", /*wait_store=*/false);
    h.seed_db = seed_before;

    const int frame_before = ImGui::GetFrameCount();
    const bool upgrade_screen_drawn = wait_until(ctx, [&] {
        const std::string text = visible_text(h);
        return text.find("Updating your library file for this version of Hydra") !=
                   std::string::npos &&
               text.find("This happens once. Your charts and results are kept.") !=
                   std::string::npos &&
               text.find("Updating the results table...") != std::string::npos;
    }, 10);
    IM_CHECK(upgrade_screen_drawn);
    // Frames kept coming while the open was held, and it is still held.
    IM_CHECK(ImGui::GetFrameCount() > frame_before);
    IM_CHECK(!h.app->store_ready());
    IM_CHECK(gate.started() > 0);
    // The bar under the words is fed by the store's own count of rows to
    // convert.
    const hydra::ui::StoreOpenProgress held = h.app->store_open_progress();
    IM_CHECK(held.step == hydra::ui::StoreOpenProgress::Step::ConvertingRecords);
    IM_CHECK(held.upgrading);
    IM_CHECK(held.rows_total > 0);

    gate.open();
    IM_CHECK(wait_until(ctx, [&] { return h.app->store_ready(); }, 30));
    IM_CHECK(!h.app->store_open_failed());
    IM_CHECK_EQ(h.app->library.rows().size(), size_t{2});
    for (const char* title : kStartupTitles)
        IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(title) != std::string::npos; }, 5));
    // A file that converts is never set aside.
    for (const std::string& file :
         hydra::store::with_side_files(hydra::store::set_aside_path(h.db_path)))
        IM_CHECK(!fs::exists(fs::u8path(file)));
    // The two results with stars became records; the one without did not.
    // The results table is gone.
    IM_CHECK_EQ(number_in_file(h.db_path, hydra::test::kResultsRowCountSql), int64_t{2});
    IM_CHECK_EQ(number_in_file(h.db_path,
                               "SELECT COUNT(*) FROM sqlite_master WHERE name = 'results'"),
                int64_t{0});
}

// Removes the set-aside copy of the harness's library file when it goes, so
// a failed check cannot leave one for startup-screen to find.
struct SetAsideCleanup {
    std::string db;
    void remove() const {
        for (const std::string& file :
             hydra::store::with_side_files(hydra::store::set_aside_path(db))) {
            std::error_code ec;
            fs::remove(fs::u8path(file), ec);
        }
    }
    ~SetAsideCleanup() { remove(); }
};

// D115 A1-6b (user decision 2026-10-14): a library file older than 2.2.0 is
// set aside and Hydra starts a fresh library. The sentence for main.cpp's
// information box waits until the box closes, frames go on drawing without a
// scan meanwhile, and closing it starts the scan of the ini's chart folders.
// The box itself is Windows'; this test plays its part with
// startup_notice_closed, as main.cpp does.
void test_old_library_set_aside(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    // A real 2.1.0 file, copied: the checked-in one is never opened.
    const std::string fixture = corpus::testdata_root() + "/store/v2.1.0-schema4.db";
    const std::string seed = h.temp_dir + "\\v210_seed.db";
    std::error_code ec;
    IM_CHECK(fs::copy_file(fs::u8path(fixture), fs::u8path(seed),
                           fs::copy_options::overwrite_existing, ec));
    const SetAsideCleanup cleanup{h.db_path};
    cleanup.remove();
    const std::string seed_before = h.seed_db;
    h.seed_db = seed;
    reset_app(h);
    h.seed_db = seed_before;

    IM_CHECK(h.app->store_ready());
    IM_CHECK(fs::exists(fs::u8path(hydra::store::set_aside_path(h.db_path))));
    IM_CHECK(h.app->library.rows().empty());
    IM_CHECK_STR_EQ(h.app->startup_notice().c_str(),
                    hydra::app::old_library_set_aside_notice(h.db_path).c_str());
    // Frames draw while the box is up, and no scan starts behind it.
    ctx->Yield(3);
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(!h.app->startup_notice().empty());

    h.app->startup_notice_closed();
    IM_CHECK(h.app->startup_notice().empty());
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job != nullptr; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job->snapshot().finished; }, 60));
    IM_CHECK(h.app->scan_job->snapshot().charts_found > 0);
    ctx->SetRef("//Scanning charts");
    ctx->ItemClick("Continue");
    ctx->Yield(2);
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(h.app->library_shown_count() > 0);
}

// Scans the corpus, then restarts the app on a copy of that library as an
// older Hydra left it: the scan stamp is one this build does not accept and
// no row has an album. Returns how many rows the library showed, or 0 when a
// step failed. The open has finished when it returns.
size_t restart_on_out_of_date_library(ImGuiTestContext* ctx, Harness& h) {
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return 0;
    const size_t rows = h.app->library_shown_count();
    // VACUUM INTO copies what the open store holds, its write-ahead log included.
    const std::string seed = h.temp_dir + "\\out_of_date_seed.db";
    std::error_code ec;
    fs::remove(fs::u8path(seed), ec);
    IM_CHECK_RETV(hydra::test::run_sql_on_file(h.db_path, "VACUUM INTO '" + seed + "'"), 0);
    IM_CHECK_RETV(hydra::test::run_sql_on_file(
                      seed, "UPDATE meta SET value = '5' WHERE key = 'chart_meta_version';"
                            "UPDATE charts SET album = '';"),
                  0);
    const std::string seed_before = h.seed_db;
    h.seed_db = seed;
    reset_app(h);
    h.seed_db = seed_before;
    IM_CHECK_RETV(h.app->store_ready(), 0);
    IM_CHECK_RETV(h.app->library_shown_count() == rows, 0);
    for (const auto& r : h.app->library.rows()) IM_CHECK_RETV(r.album.empty(), 0);
    return rows;
}

// A library an older Hydra scanned scans itself on open, with no click, and
// the scan window's first line says why. The scan reads the albums the old
// rows lacked.
void test_rescan_on_open(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const size_t rows = restart_on_out_of_date_library(ctx, h);
    if (rows == 0) return;

    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job != nullptr; }, 5));
    IM_CHECK(on_screen(h, hydra::app::library_rescan_notice()));
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job->snapshot().finished; }, 60));
    ctx->Yield(2);
    IM_CHECK(on_screen(h, "Done: "));
    IM_CHECK(on_screen(h, hydra::app::library_rescan_notice()));
    ctx->SetRef("//Scanning charts");
    ctx->ItemClick("Continue");
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(!h.app->scan_started_by_open);
    IM_CHECK(!h.app->store->chart_library_out_of_date());
    IM_CHECK(h.app->library_shown_count() == rows);
    bool any_album = false;
    for (const auto& r : h.app->library.rows()) any_album = any_album || !r.album.empty();
    IM_CHECK(any_album);

    // A scan the user starts says nothing about an older version.
    scan_library(ctx);
    IM_CHECK(!h.app->scan_started_by_open);
}

// Cancel on the scan the app started leaves the library as it was, and the
// library is still out of date, so the next open asks again.
void test_rescan_on_open_cancel(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const size_t rows = restart_on_out_of_date_library(ctx, h);
    if (rows == 0) return;

    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job != nullptr; }, 5));
    ctx->SetRef("//Scanning charts");
    // The corpus scan can end before the click lands, and nothing holds it;
    // the unit test cancels deterministically. Here the Cancel button is
    // pressed when it is still there, and the test checks whichever end the
    // scan reached.
    ctx->ItemClick("Cancel", 0, ImGuiTestOpFlags_NoError);
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job->snapshot().finished; }, 60));
    const bool cancelled = h.app->scan_job->snapshot().cancelled;
    ctx->Yield(2);
    if (cancelled)
        IM_CHECK(on_screen(h, "Scan cancelled. The library was left unchanged."));
    else
        IM_CHECK(on_screen(h, "Done: "));
    ctx->ItemClick("Continue");
    ctx->Yield(2);
    ctx->SetRef("//Hydra");
    IM_CHECK(h.app->scan_job == nullptr);
    IM_CHECK(!h.app->scan_started_by_open);
    IM_CHECK(h.app->library_shown_count() == rows);
    IM_CHECK(h.app->store->chart_library_out_of_date() == cancelled);
}

// ---- the version bar ----------------------------------------------------------
// docs/superpowers/plans/2026-10-23-update-check-on-launch.md, task T1b.
// Nothing here touches the network: UpdateSeams serves the release reply, the
// download and the installer start.

std::string running_version_text() {
    return hydra::core::version_text(hydra::core::running_version());
}

// The committed test key (sample.bin.sig was signed with it).
std::array<uint8_t, 64> update_test_key(ImGuiTestContext* ctx) {
    std::array<uint8_t, 64> key{};
    IM_CHECK_NO_RET(update_fixtures::read_test_public_key(key));
    if (ctx->IsError()) return {};
    return key;
}

// Sits while `hold` is set and nothing cancels, for at most 30 s; the test's
// own waits give up before that.
void wait_released(const std::atomic<bool>& hold, const std::atomic<bool>* cancel) {
    const auto give_up = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (hold.load() && !(cancel && cancel->load()) && std::chrono::steady_clock::now() < give_up)
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

// The updater's seams for one test: Hydra counts as installed, GitHub's reply
// is `reply`, the download copies sample.bin and serves sample.bin.sig, and
// the installer "ends early", which is a declined UAC prompt (decision 10).
// hold_check and hold_download keep the check or the download running until
// the test clears them. Make it after reset_app, which installs the harness's
// own fetcher. Its destructor drops the app first, so no job of it calls a
// seam that is gone.
struct UpdateSeams {
    Harness& h;
    std::string reply = update_fixtures::release_reply_at("v9.9.9");
    std::string sig = hydra::read_file_text(update_fixtures::update_dir() + "/sample.bin.sig");
    std::atomic<bool> hold_check{false};
    std::atomic<bool> hold_download{false};
    std::string folder;
    ScopedHook installed;
    ScopedHook folder_seam;
    ScopedHook fetcher;
    ScopedHook downloader;
    ScopedHook launcher;

    explicit UpdateSeams(Harness& harness_)
        : h(harness_),
          folder(h.temp_dir + "/update-download"),
          installed(hydra::app::set_installed_for_test, std::optional<bool>(true)),
          folder_seam(hydra::ui::set_update_folder_for_test, folder),
          fetcher(hydra::net::set_fetcher,
                  [this](const std::string& url, const std::atomic<bool>* cancel) -> std::string {
                      if (url == hydra::ui::kLatestReleaseUrl) {
                          wait_released(hold_check, cancel);
                          return reply;
                      }
                      if (url.size() > 4 && url.substr(url.size() - 4) == ".sig") return sig;
                      throw std::runtime_error("no canned reply for " + url);
                  }),
          downloader(hydra::net::set_downloader,
                     [this](const std::string&, const std::string& path, uint64_t,
                            std::atomic<uint64_t>* received, const std::atomic<bool>* cancel) {
                         wait_released(hold_download, cancel);
                         fs::copy_file(hydra::os_path(update_fixtures::sample_path()), hydra::os_path(path),
                                       fs::copy_options::overwrite_existing);
                         if (received) received->store(fs::file_size(hydra::os_path(path)));
                     }),
          launcher(hydra::ui::set_launcher_for_test, [](const std::string&, const std::string&) {
              return hydra::ui::InstallerEnded([] { return true; });
          }) {}
    ~UpdateSeams() { h.app.reset(); }
    UpdateSeams(const UpdateSeams&) = delete;
    UpdateSeams& operator=(const UpdateSeams&) = delete;
};

// A new start of Hydra on the same INI and database, as closing and opening
// it again would be. `edit`, when given, changes the INI before the start
// reads it.
void restart_app(ImGuiTestContext* ctx, Harness& h,
                 const std::function<void(hydra::app::Settings&)>& edit) {
    h.app.reset();
    if (edit) {
        hydra::app::Settings s = hydra::app::Settings::load_file(h.ini_path);
        edit(s);
        IM_CHECK(s.save_file(h.ini_path));
    }
    h.app = std::make_unique<hydra::ui::AppState>();
    h.app->set_render_device(h.device.Get(), h.context.Get());
    h.app->update_public_key = update_test_key(ctx);
    h.app->wait_store_open();
    IM_CHECK(!h.app->store_open_failed());
}

bool item_disabled(ImGuiTestContext* ctx, const char* ref) {
    return (ctx->ItemInfo(ref).ItemFlags & ImGuiItemFlags_Disabled) != 0;
}

// The version bar across the top of the window, criteria B1 to B7 of the
// update-check-on-launch plan's T1b. Every check that needs a check to run
// starts the app again, as a user would.
void test_version_bar(ImGuiTestContext* ctx) {
    using Phase = hydra::ui::UpdateState::Phase;
    using Dot = hydra::ui::VersionBarStatus::Dot;
    using hydra::app::Settings;
    Harness& h = harness(ctx);
    reset_app(h);
    UpdateSeams seams(h);
    h.app->update_public_key = update_test_key(ctx);
    if (ctx->IsError()) return;
    auto phase_is = [&](Phase phase) { return h.app && h.app->update.phase == phase; };
    auto dot_is = [&](Dot dot) { return h.app && h.app->version_bar_status().dot == dot; };
    const std::string running = running_version_text();
    const std::string name = hydra::core::named_version(hydra::core::running_version());

    // B4: a copy the installer didn't put in place never checks, and says so.
    hydra::app::set_installed_for_test(false);
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    bool ever_running = false;
    for (int i = 0; i < 5; ++i) {
        ever_running = ever_running || h.app->update_check_running();
        ctx->Yield();
    }
    IM_CHECK(!ever_running);
    const std::string not_installed = "Only the installed copy of Hydra Deluxe checks for updates.";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, not_installed); }, 5));
    IM_CHECK(dot_is(Dot::Neutral));

    // B1: the bar is the window's first row, at the window padding and above
    // the toolbar, and its name is plain text: a click on it changes nothing.
    ImGuiWindow* const bar = ctx->WindowInfo("//Hydra/##versionbar").Window;
    ImGuiWindow* const main = ctx->GetWindowByRef("//Hydra");
    IM_CHECK_NO_RET(bar != nullptr);
    IM_CHECK_NO_RET(main != nullptr);
    if (ctx->IsError()) return;
    IM_CHECK(on_screen(h, name));
    const std::string manage =
        "//Hydra/Manage folders... (" + std::to_string(h.app->settings.chartfolders.size()) + ")";
    const ImRect manage_rect = ctx->ItemInfo(manage.c_str()).RectFull;
    IM_CHECK(bar->Pos.x == main->Pos.x + main->WindowPadding.x);
    IM_CHECK(bar->Pos.y < manage_rect.Min.y);
    {
        const std::string text = visible_text(h);
        IM_CHECK(text.find(name) < text.find("Manage folders..."));
    }
    const Phase phase_before = h.app->update.phase;
    const hydra::ui::VersionBarStatus before = h.app->version_bar_status();
    ctx->MouseMoveToPos(ImVec2(bar->Pos.x + bar->WindowPadding.x + hydra::ui::px(10),
                               bar->Pos.y + bar->Size.y * 0.5f));
    ctx->MouseClick(0);
    ctx->Yield(2);
    IM_CHECK(h.app->update.phase == phase_before);
    IM_CHECK(h.app->version_bar_status().text == before.text);
    IM_CHECK(h.app->version_bar_status().dimmed == before.dimmed);
    IM_CHECK(ctx->UiContext->OpenPopupStack.Size == 0);
    hydra::app::set_installed_for_test(true);

    // B2: with the latest release at the running version, the bar says
    // Checking for updates... while the check runs, then Up to date; nothing
    // about updates shows anywhere else.
    seams.reply = update_fixtures::release_reply_at("v" + running);
    seams.hold_check = true;
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    IM_CHECK(h.app->update_check_running());
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Checking for updates..."); }, 5));
    IM_CHECK(dot_is(Dot::Neutral));
    seams.hold_check = false;
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Up to date"); }, 10));
    IM_CHECK(dot_is(Dot::UpToDate));
    IM_CHECK(phase_is(Phase::None));
    IM_CHECK(!on_screen(h, "Hydra Deluxe " + running + " is the latest version."));
    IM_CHECK(!ctx->ItemExists("**/OK##update"));
    IM_CHECK(!ctx->ItemExists("**/Update##update"));
    IM_CHECK(screenshot(ctx, "versionbar-uptodate.png"));

    // B3: the offer, its three buttons, and the notes on hover of the
    // sentence, which ends just left of the Update button.
    const std::string notes_part = "shows accented letters in song names correctly";
    auto notes_show_on_hover = [&] {
        const ImRect update_rect = ctx->ItemInfo("**/Update##update").RectFull;
        IM_CHECK_NO_RET(!on_screen(h, notes_part));
        ctx->MouseMoveToPos(ImVec2(update_rect.Min.x - ImGui::GetStyle().ItemSpacing.x - hydra::ui::px(10),
                                   update_rect.GetCenter().y));
        IM_CHECK_NO_RET(wait_until(ctx, [&] { return on_screen(h, notes_part); }, 5));
        ctx->MouseMoveToPos(ImVec2(update_rect.Max.x + hydra::ui::px(40), update_rect.GetCenter().y));
        IM_CHECK_NO_RET(wait_until(ctx, [&] { return !on_screen(h, notes_part); }, 5));
    };
    seams.reply = update_fixtures::release_reply_at("v9.9.9");
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    const std::string offer = "Hydra Deluxe 9.9.9 is available.";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, offer); }, 10));
    IM_CHECK(phase_is(Phase::Offered));
    IM_CHECK(dot_is(Dot::Available));
    IM_CHECK(ctx->ItemExists("**/Update##update"));
    IM_CHECK(ctx->ItemExists("**/Skip this version"));
    IM_CHECK(ctx->ItemExists("**/Later"));
    IM_CHECK(screenshot(ctx, "versionbar-offer.png"));
    notes_show_on_hover();

    // B3: after Later the bar keeps the sentence, its notes and its Update
    // button; Skip this version and Later are gone.
    ctx->ItemClick("**/Later");
    IM_CHECK(wait_until(ctx, [&] { return phase_is(Phase::None) && !ctx->ItemExists("**/Later"); }, 5));
    IM_CHECK(!ctx->ItemExists("**/Skip this version"));
    IM_CHECK(ctx->ItemExists("**/Update##update"));
    IM_CHECK(on_screen(h, offer));
    IM_CHECK(dot_is(Dot::Available));
    notes_show_on_hover();

    // B7 (D3): the resting bar's Update downloads with a Cancel button; the
    // installer ending early brings decision 10's sentence and the three
    // buttons back.
    seams.hold_download = true;
    ctx->ItemClick("**/Update##update");
    const std::string downloading = "Downloading Hydra Deluxe 9.9.9...";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, downloading); }, 5));
    IM_CHECK(ctx->ItemExists("**/Cancel##update"));
    IM_CHECK(!ctx->ItemExists("**/Skip this version"));
    seams.hold_download = false;
    const std::string declined = "The installer didn't start, so Hydra wasn't updated.";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, declined); }, 10));
    IM_CHECK(ctx->ItemExists("**/Update##update"));
    IM_CHECK(ctx->ItemExists("**/Skip this version"));
    IM_CHECK(ctx->ItemExists("**/Later"));
    IM_CHECK(!ctx->ItemExists("**/Cancel##update"));

    // B3: Skip this version, and the next start names the release, marked
    // Skipped., with no buttons.
    ctx->ItemClick("**/Skip this version");
    IM_CHECK(wait_until(ctx, [&] { return phase_is(Phase::None) && !on_screen(h, declined); }, 5));
    IM_CHECK(Settings::load_file(h.ini_path).skipped_update_version == "9.9.9");
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return !h.app->update_check_running() && on_screen(h, offer); }, 10));
    ctx->Yield(2);
    IM_CHECK(phase_is(Phase::None));
    IM_CHECK(on_screen(h, "Skipped."));
    IM_CHECK(dot_is(Dot::Available));
    IM_CHECK(!ctx->ItemExists("**/Update##update"));
    IM_CHECK(!ctx->ItemExists("**/Skip this version"));
    IM_CHECK(!ctx->ItemExists("**/Later"));

    // B5: a start-up check that fails says so, with the job's plain_error
    // sentence dimmed after it, and no buttons.
    seams.reply = "this is not JSON";
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    const std::string could_not = "Hydra couldn't check for updates when it started.";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, could_not); }, 10));
    ctx->Yield(2);
    const std::string reason = h.app->update.check_error;
    IM_CHECK(!reason.empty());
    IM_CHECK(on_screen(h, reason));
    IM_CHECK(dot_is(Dot::Neutral));
    IM_CHECK(!ctx->ItemExists("**/Later"));
    IM_CHECK(!ctx->ItemExists("**/Update##update"));

    // B6: the Analysis settings panel is back to its three groups and Reset to
    // defaults; nothing in it is about updates.
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    ctx->Yield(2);
    IM_CHECK(!on_screen(h, "Check for updates"));
    IM_CHECK(!ctx->ItemExists("**/Check now"));
    IM_CHECK(on_screen(h, "Clone Hero rules"));
    IM_CHECK(on_screen(h, "Paths kept"));
    IM_CHECK(ctx->ItemExists("**/Reset to defaults"));
    close_settings_panel(ctx);

    // B6: with a batch holding the settings, the lock line still shows.
    scan_library(ctx);
    if (ctx->IsError()) return;
    {
        BatchGate gate;
        h.app->start_batch(false);
        IM_CHECK(wait_until(ctx, [&] { return gate.started() >= 1; }, 30));
        open_settings_panel(ctx);
        if (ctx->IsError()) return;
        IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Stop the batch to change these."); }, 5));
        IM_CHECK(item_disabled(ctx, "**/##prodrums"));
        close_settings_panel(ctx);
        h.app->batch_job->stop();
        IM_CHECK(wait_until(ctx, [&] { return !h.app->batch_running(); }, 300));
    }
    IM_CHECK(wait_until(ctx, [&] { return !jobs_busy(h); }, 120));

    // B7 (A8 on screen): the first start after a version change says so in
    // the bar, and OK gives way to the offer the start-up check found.
    seams.reply = update_fixtures::release_reply_at("v9.9.9");
    restart_app(ctx, h, [](Settings& s) {
        s.last_run_version = "0.0.1";
        s.skipped_update_version.clear();
    });
    if (ctx->IsError()) return;
    const std::string updated = "Updated to Hydra Deluxe " + running + ".";
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, updated); }, 5));
    IM_CHECK(wait_until(ctx, [&] { return !h.app->update_check_running(); }, 10));
    ctx->ItemClick("**/OK##update");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, offer) && !on_screen(h, updated); }, 5));
    IM_CHECK(phase_is(Phase::Offered));
    IM_CHECK(ctx->ItemExists("**/Update##update"));

    // B7 (D5): with nothing newer, OK gives the bar back to Up to date, and
    // the next start shows no note.
    seams.reply = update_fixtures::release_reply_at("v" + running);
    restart_app(ctx, h, [](Settings& s) { s.last_run_version = "0.0.1"; });
    if (ctx->IsError()) return;
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, updated); }, 5));
    IM_CHECK(Settings::load_file(h.ini_path).last_run_version == running);
    ctx->ItemClick("**/OK##update");
    IM_CHECK(wait_until(ctx, [&] { return phase_is(Phase::None) && !on_screen(h, updated); }, 5));
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, "Up to date"); }, 10));
    restart_app(ctx, h, {});
    if (ctx->IsError()) return;
    ctx->Yield(3);
    IM_CHECK(phase_is(Phase::None));
    IM_CHECK(!on_screen(h, updated));
}

// Decision 10 of the 2026-10-24 album-search plan: one short line and a
// "Search tips" link under the search box, and a popup listing every
// app::search_tips() entry, at the full width and with a song open.
void test_library_search_tips(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef("//Hydra");
    const std::string note(hydra::app::search_tips_note());
    const auto popup_shows_every_tip = [&] {
        for (const hydra::app::SearchTip& tip : hydra::app::search_tips()) {
            IM_CHECK(on_screen(h, std::string(tip.example)));
            IM_CHECK(on_screen(h, std::string(tip.meaning)));
        }
        IM_CHECK(on_screen(h, note));
    };

    // The empty box shows its placeholder; the old hint line is gone.
    IM_CHECK(on_screen(h, "Search songs"));
    IM_CHECK(!on_screen(h, "Narrow with"));
    IM_CHECK(on_screen(h, "Type any words, or narrow it down."));
    IM_CHECK(ctx->ItemExists("**/Search tips##searchtips"));
    IM_CHECK(!on_screen(h, note));

    // The link opens the popup over a typed search; Escape closes it and
    // leaves the search as it was.
    ctx->ItemInputValue("**/##search", "chair");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "chair"; }, 5));
    ctx->ItemClick("**/Search tips##searchtips");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, note); }, 5));
    popup_shows_every_tip();
    ctx->KeyPress(ImGuiKey_Escape);
    IM_CHECK(wait_until(ctx, [&] { return !on_screen(h, note); }, 5));
    ctx->Yield(2);
    IM_CHECK(h.app->search == "chair");
    IM_CHECK(std::strcmp(h.app->library_ui.search_buf, "chair") == 0);

    // A filter it can't read still says so under the box.
    ctx->ItemInputValue("**/##search", "stars:9");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->library.query().errors.empty(); }, 5));
    IM_CHECK(on_screen(h, h.app->library.query().errors[0]));
    IM_CHECK(on_screen(h, "Type any words, or narrow it down."));

    // With a song open the library is narrow; the line, the link and the
    // popup still show, and the Escape that closes the popup leaves the song
    // panel open.
    ctx->ItemInputValue("**/##search", "chair");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "chair" && h.app->library_shown_count() > 0; }, 5));
    h.app->select(h.app->library_row_at(0).entry, hydra::ui::SelectionSource::Click);
    IM_CHECK(wait_until(ctx, [&] { return !ctx->ItemExists("**/Folder"); }, 5));
    IM_CHECK(on_screen(h, "Type any words, or narrow it down."));
    IM_CHECK(ctx->ItemExists("**/Search tips##searchtips"));
    ctx->ItemClick("**/Search tips##searchtips");
    IM_CHECK(wait_until(ctx, [&] { return on_screen(h, note); }, 5));
    popup_shows_every_tip();
    ctx->KeyPress(ImGuiKey_Escape);
    IM_CHECK(wait_until(ctx, [&] { return !on_screen(h, note); }, 5));
    ctx->Yield(2);
    IM_CHECK(h.app->details_open());
    IM_CHECK(h.app->search == "chair");
    h.app->close_details();
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Folder"); }, 5));
}

}  // namespace

const std::vector<TestEntry>& library_tests() {
    static const std::vector<TestEntry> entries = {
        {"scan", test_scan},
        {"difficulty", test_difficulty},
        {"note-shuffle-switch", test_note_shuffle_switch},
        {"settings-button", test_settings_button},
        {"engine-wildcard-patches", test_engine_wildcard_patches},
        {"settings-reset", test_settings_reset},
        {"rules-error", test_rules_error},
        {"library-state-per-app", test_library_state_per_app},
        {"view-settings", test_view_settings},
        {"library-search", test_library_search},
        {"analyze-follows-chip", test_analyze_follows_chip},
        {"library-cant-analyze", test_library_cant_analyze},
        {"library-sort-scroll", test_library_sort_scroll},
        {"library-column-order", test_library_column_order},
        {"library-layout", test_library_layout},
        {"library-column-widths", test_library_column_widths},
        {"library-widths-hold", test_library_widths_hold},
        {"library-click-keeps-scroll", test_library_click_keeps_scroll},
        {"library-next-scrolls-just-enough", test_library_next_scrolls_just_enough},
        {"library-columns-at-start", test_library_columns_at_start},
        {"library-twin-rows", test_library_twin_rows},
        {"startup-screen", test_startup_screen},
        {"old-library-set-aside", test_old_library_set_aside},
        {"rescan-on-open", test_rescan_on_open},
        {"rescan-on-open-cancel", test_rescan_on_open_cancel},
        {"version-bar", test_version_bar},
        {"library-search-tips", test_library_search_tips},
    };
    return entries;
}

}  // namespace uitest
