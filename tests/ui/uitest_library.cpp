#include <sqlite3.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "uitest_harness.h"

#include "../old_layout_fixture.h"  // tests/ is not on the runner's include path
#include "app/config.h"
#include "imgui_internal.h"
#include "ui/app_shell.h"  // remember_library_share
#include "ui/app_state.h"
#include "ui/details_view.h"  // kMinLibraryW
#include "ui/fonts.h"  // px()
#include "ui/library_model.h"
#include "ui/preview_controller.h"
#include "store/record_store.h"
#include "ui/widgets.h"  // widest_digits, button_slot_width

namespace uitest {

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
    h.app->select(h.app->library_row_at(1).entry);
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

// The settings bar's difficulty dropdown: it drives the chartmode everything
// else is keyed by. 2x Bass stays live at every difficulty (D20) and is part
// of the key there too.
void test_difficulty(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    ctx->SetRef(ctx->WindowInfo("//Hydra/##settingsbar").Window);

    IM_CHECK(h.app->settings.view_bass2x);  // the default the test relies on
    IM_CHECK((ctx->ItemInfo("2x Bass").ItemFlags & ImGuiItemFlags_Disabled) == 0);

    ctx->ComboClick("##difficulty/Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));
    IM_CHECK_STR_EQ(hydra::app::Settings::load_file(h.ini_path).view_difficulty.c_str(),
                    "Hard");
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 2x Bass");

    // Live at Hard: unticking it changes Hard's key, and ticking it restores it.
    IM_CHECK((ctx->ItemInfo("2x Bass").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(h.app->settings.effective_bass2x());
    ctx->ItemClick("2x Bass");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.view_bass2x; }, 5));
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 1x Bass");
    ctx->ItemClick("2x Bass");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_bass2x; }, 5));

    // Back on Expert the box still carries the user's own setting. (Checked
    // here rather than at the end of the test: the details modal opened below
    // has no close button the harness can address.)
    ctx->ComboClick("##difficulty/Expert");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Expert"; }, 5));
    IM_CHECK((ctx->ItemInfo("2x Bass").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(h.app->settings.effective_bass2x());
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Expert Pro Drums, 2x Bass");

    ctx->ComboClick("##difficulty/Hard");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_difficulty == "Hard"; }, 5));

    // Narrow to a chart that actually has a [HardDrums] section, so the
    // analysis below has notes to work with.
    ctx->SetRef("//Hydra");
    ctx->ItemInputValue("**/##search", "Pokemon Theme");
    IM_CHECK(wait_until(ctx, [&] { return h.app->search == "Pokemon Theme"; }, 5));
    IM_CHECK(wait_until(ctx, [&] { return h.app->library_shown_count() > 0; }, 5));
    open_details(ctx, 0);
    if (ctx->IsError()) return;
    // The settings bar, not the panel, names the difficulty now.
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
    ctx->SetRef("//Hydra");

    // Pro Drums off is a different chart mode: persisted at once.
    IM_CHECK(h.app->settings.view_prodrums);
    ctx->ItemClick("**/Pro Drums");
    IM_CHECK(!h.app->settings.view_prodrums);
    IM_CHECK(!hydra::app::Settings::load_file(h.ini_path).view_prodrums);
    IM_CHECK(h.app->settings.chartmode_key().find("Pro Drums") == std::string::npos);
    ctx->ItemClick("**/Pro Drums");
    IM_CHECK(h.app->settings.view_prodrums);

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

    // With a song open, Folder makes way, so each row says where it matched.
    h.app->select(h.app->library_row_at(0).entry);
    IM_CHECK(wait_until(ctx, [&] { return !ctx->ItemExists("**/Folder"); }, 5));
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("Matched on folder.") != std::string::npos;
    }, 5));
    h.app->close_details();
    IM_CHECK(wait_until(ctx, [&] { return ctx->ItemExists("**/Folder"); }, 5));

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

    // Nothing matches: the empty state, and Clear search brings it all back.
    ctx->ItemInputValue("**/##search", "zzqx");
    IM_CHECK(wait_until(ctx, [&] { return shown() == 0; }, 5));
    IM_CHECK(visible_text(h).find("No charts match your search.") != std::string::npos);
    ctx->ItemClick("**/Clear search");
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

// The settings bar, the chips and the Title column fit their room, at the
// narrowest library and on long data. Geometry, not text: the text log
// records a cut-off string in full.
void test_library_layout(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    // The window width this test changes goes back for the tests after it,
    // however it ends.
    struct WidthGuard {
        Harness& h;
        int width;
        ~WidthGuard() { h.width = width; }
    } guard{h, h.width};
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

    // The settings bar: nothing past its right edge, the lock message aside
    // (nothing is running). The bar spans the window, so at 1,280 px its
    // last block has wrapped under the first.
    ImGuiWindow* bar = ctx->WindowInfo("//Hydra/##settingsbar").Window;
    IM_CHECK(bar != nullptr);
    if (bar == nullptr) return;
    IM_CHECK_LE(bar->ContentSize.x, bar->ContentRegionRect.GetWidth() + 0.5f);
    ctx->SetRef(bar);
    const char* bar_items[] = {"##difficulty", "Pro Drums", "2x Bass", "##spcap",
                               "1.0 fills", "##depthvalue", "##depthmode",
                               "Path limit##mslimit", "##mslimitvalue"};
    for (const char* item : bar_items)
        IM_CHECK_LE(ctx->ItemInfo(item).RectFull.Max.x, bar->InnerRect.Max.x + 0.5f);
    const float first_line_y = ctx->ItemInfo("##difficulty").RectFull.Min.y;
    IM_CHECK_GT(ctx->ItemInfo("##mslimitvalue").RectFull.Min.y, first_line_y);
    // A wrapped block keeps its own pieces on one line.
    IM_CHECK_EQ(ctx->ItemInfo("Path limit##mslimit").RectFull.Min.y,
                ctx->ItemInfo("##mslimitvalue").RectFull.Min.y);

    // Score range holds six digits beside its step buttons. The sample
    // widest_digits gives is one measured run in the shipped font at this
    // size (audit finding 112).
    h.app->settings.depth_value = 999999;
    ctx->Yield(2);
    // The click analyzed (and saved) this song (D87), and the setting change
    // re-analyzes it (D90): let that settle, so the chip counts below stand still.
    IM_CHECK(wait_until(ctx, [&] { return h.app->view_settled(); }, 300));
    const std::string widest = hydra::ui::widest_digits(6);
    IM_CHECK_STR_EQ(widest.c_str(), "000000");
    IM_CHECK_GE(ctx->ItemInfo("##depthvalue").RectFull.GetWidth(),
                hydra::ui::button_slot_width(widest.c_str()));
    IM_CHECK_LE(ctx->ItemInfo("##depthvalue/+").RectFull.Max.x,
                ctx->ItemInfo("##depthmode").RectFull.Min.x);

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

    // The library at its widest on a wide window: the bar is one line.
    h.width = 1920;
    hydra::ui::remember_library_share(0.99f);
    h.app->library_ui.panel_was_open = false;
    ctx->Yield(3);
    ctx->SetRef(bar);
    IM_CHECK_LE(bar->ContentSize.x, bar->ContentRegionRect.GetWidth() + 0.5f);
    const float line_y = ctx->ItemInfo("##difficulty").RectFull.Min.y;
    for (const char* item :
         {"Pro Drums", "2x Bass", "##spcap", "1.0 fills", "##depthvalue", "##mslimitvalue"})
        IM_CHECK_EQ(ctx->ItemInfo(item).RectFull.Min.y, line_y);
    // The whole row sits centred on the two caption lines ("Analysis
    // settings" over "for every song"), not level with the first of them.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float caption_top = bar->Pos.y + style.WindowPadding.y;
    const float caption_h = ImGui::GetTextLineHeight() * 2.0f + style.ItemSpacing.y;
    IM_CHECK_FLOAT_NEAR_EQ(line_y, caption_top + (caption_h - ImGui::GetFrameHeight()) * 0.5f,
                           1.0f);
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

// SQL on a database file through a connection of the test's own. A copy of
// exec_on_file (tests/db_file_util.h), which needs doctest; the GUI harness
// has none.
bool exec_on_file(const std::string& path, const std::string& sql) {
    sqlite3* db = nullptr;
    bool ok = sqlite3_open(path.c_str(), &db) == SQLITE_OK &&
              sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
    sqlite3_close(db);
    return ok;
}

// The first column of the first row `sql` returns, or -1 when it fails. A
// copy of scalar_on_file (tests/db_file_util.h), for the same reason.
int64_t scalar_on_file(const std::string& path, const std::string& sql) {
    sqlite3* db = nullptr;
    int64_t v = -1;
    sqlite3_stmt* s = nullptr;
    if (sqlite3_open(path.c_str(), &db) == SQLITE_OK &&
        sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr) == SQLITE_OK &&
        sqlite3_step(s) == SQLITE_ROW)
        v = sqlite3_column_int64(s, 0);
    sqlite3_finalize(s);
    sqlite3_close(db);
    return v;
}

// The titles the startup test's library file holds.
constexpr const char* kStartupTitles[] = {"Startup Song One", "Startup Song Two"};

// Writes a library file in the layout before summary-only storage (D87): two
// charts, three results with the structure blob, and the detail tables. One
// result is from before the stars column (a score and no stars), which the
// upgrade leaves behind. Returns the file's path, or "" if a step failed.
std::string write_old_layout_library(Harness& h) {
    const std::string path = h.temp_dir + "\\old_layout_seed.db";
    std::error_code ec;
    for (const char* suffix : {"", "-wal", "-shm"}) fs::remove(fs::u8path(path + suffix), ec);
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
    // The results table as that layout made it, with literal rows: the old
    // build's columns, not this build's.
    const std::string sql =
        std::string("DROP TABLE results;") + hydra::test::kDetailLayoutResultsTableSql + ";" +
        "INSERT INTO results (result_id, hyhash, chartmode, hyversion, sp_cap, ms_enabled,"
        " ms_value, depth_mode, depth_value, bestpath, structure, score, stars, rules_fp) VALUES"
        " (1, 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa', 'mode', 'v', 8, 0, 0, 0, 2, '1', x'07', 1000, 5, x''),"
        " (2, 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb', 'mode', 'v', 8, 0, 0, 0, 2, '1', x'07', 2000, 6, x''),"
        " (3, 'bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb', 'other', 'v', 8, 0, 0, 0, 2, '1', x'07', 3000,"
        " NULL, x'');" +
        hydra::test::kDetailTablesSql;
    if (!exec_on_file(path, sql)) return "";
    return path;
}

// The window keeps drawing while the store opens (the DBUP plan, part 1). A
// library file in the old layout makes the open run the upgrade; an OpenGate
// holds that open on its worker thread, and the frames still come with the
// upgrade's screen on them, in the words the user chose for it. Once the gate
// opens, the library lists the file's charts and the upgrade left no temp file.
void test_startup_screen(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    const std::string seed = write_old_layout_library(h);
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
               text.find("Copying your library...") != std::string::npos;
    }, 10);
    IM_CHECK(upgrade_screen_drawn);
    // Frames kept coming while the open was held, and it is still held.
    IM_CHECK(ImGui::GetFrameCount() > frame_before);
    IM_CHECK(!h.app->store_ready());
    IM_CHECK(gate.started() > 0);
    // The bar under the words is fed by the store's own count of rows to copy.
    const hydra::ui::StoreOpenProgress held = h.app->store_open_progress();
    IM_CHECK(held.step == hydra::ui::StoreOpenProgress::Step::Copying);
    IM_CHECK(held.upgrading);
    IM_CHECK(held.rows_total > 0);

    gate.open();
    IM_CHECK(wait_until(ctx, [&] { return h.app->store_ready(); }, 30));
    IM_CHECK(!h.app->store_open_failed());
    IM_CHECK_EQ(h.app->library.rows().size(), size_t{2});
    for (const char* title : kStartupTitles)
        IM_CHECK(wait_until(ctx, [&] { return visible_text(h).find(title) != std::string::npos; }, 5));
    // The upgrade swapped its files and tidied up.
    for (const char* leftover : {".upgrading", ".old"})
        IM_CHECK(!fs::exists(fs::u8path(h.db_path + leftover)));
    // The two results with stars came across; the one without did not. The
    // detail tables are gone.
    IM_CHECK_EQ(scalar_on_file(h.db_path, "SELECT COUNT(*) FROM results"), int64_t{2});
    IM_CHECK_EQ(scalar_on_file(h.db_path, hydra::test::kDetailTablesCountSql), int64_t{0});
}

}  // namespace

const std::vector<TestEntry>& library_tests() {
    static const std::vector<TestEntry> entries = {
        {"scan", test_scan},
        {"difficulty", test_difficulty},
        {"rules-error", test_rules_error},
        {"library-state-per-app", test_library_state_per_app},
        {"view-settings", test_view_settings},
        {"library-search", test_library_search},
        {"library-sort-scroll", test_library_sort_scroll},
        {"library-column-order", test_library_column_order},
        {"library-layout", test_library_layout},
        {"library-twin-rows", test_library_twin_rows},
        {"startup-screen", test_startup_screen},
    };
    return entries;
}

}  // namespace uitest
