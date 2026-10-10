// The batch strip's and confirm's text helpers: counts with real plurals and
// thousands grouping, durations, and the settings lines the confirm lists.

#include "doctest.h"

#include "app/analysis.h"  // AnalysisSettings
#include "app/config.h"
#include "core/model.h"  // counted
#include "search/pather.h"  // settings_key
#include "store/record_store.h"  // CapQuery, Lens
#include "ui/library_parts.h"

#include <functional>

using hydra::counted;
using hydra::app::Settings;
using namespace hydra::ui::detail;

TEST_CASE("batch text: counts group thousands and pick the right noun") {
    CHECK(counted(0, "chart", "charts") == "0 charts");
    CHECK(counted(1, "chart", "charts") == "1 chart");
    CHECK(counted(2, "chart", "charts") == "2 charts");
    CHECK(counted(12345, "chart", "charts") == "12,345 charts");
}

TEST_CASE("batch text: durations read m:ss under an hour and h:mm:ss over it") {
    CHECK(format_duration(0.0) == "0:00");
    CHECK(format_duration(42.4) == "0:42");
    CHECK(format_duration(723.0) == "12:03");
    CHECK(format_duration(3725.0) == "1:02:05");
    CHECK(format_duration(-3.0) == "0:00");
}

TEST_CASE("batch text: time left reads about m:ss left") {
    CHECK(time_left_text(90.0) == "about 1:30 left");
    CHECK(time_left_text(42.4) == "about 0:42 left");
}

TEST_CASE("batch text: the confirm lists the settings a batch runs with") {
    Settings s;  // Expert, Pro Drums, 2x Bass, cap 4, 4 scores, 10 ms
    BatchSettingsSummary d = batch_settings_summary(s);
    CHECK(d.difficulty == "Expert \xC2\xB7 Pro Drums \xC2\xB7 2x Bass");
    CHECK(d.sp_cap == "4 bars (Clone Hero's rule)");
    CHECK(d.score_range == "4 scores");
    CHECK(d.path_limit == "10 ms");

    s.view_difficulty = "Hard";  // 2x Bass (still on) applies at Hard too (D20)
    s.view_prodrums = false;
    s.sp_cap = 1;
    s.depth_mode = 1;
    s.depth_value = 2000;
    s.mslimit_enabled = false;
    d = batch_settings_summary(s);
    CHECK(d.difficulty == "Hard \xC2\xB7 2x Bass");
    CHECK(d.sp_cap == "1 bar (a what-if)");
    CHECK(d.score_range == "2,000 points");
    CHECK(d.path_limit == "off");

    // A hand-edited depth_mode=2 searches by scores, so it reads scores
    // (Settings::search_depth_mode, D51 addendum).
    s.depth_mode = 2;
    s.depth_value = 4;
    d = batch_settings_summary(s);
    CHECK(d.score_range == "4 scores");
}

TEST_CASE("settings button: lists only the settings that differ from the defaults") {
    Settings s;
    CHECK(settings_changes_summary(s) == "defaults");
    CHECK(settings_button_label(s, false) == "Analysis settings: defaults");
    CHECK(settings_button_label(s, true) == "Analysis settings: defaults (locked)");

    // Each setting on its own, one phrase per row of the handoff's table.
    {
        Settings t;
        t.view_difficulty = "Hard";
        CHECK(settings_changes_summary(t) == "Hard");
    }
    {
        Settings t;
        t.view_prodrums = false;
        CHECK(settings_changes_summary(t) == "Pro Drums off");
    }
    {
        Settings t;
        t.view_bass2x = false;
        CHECK(settings_changes_summary(t) == "2x Bass off");
    }
    {
        Settings t;
        t.view_noteshuffle = true;
        CHECK(settings_changes_summary(t) == "Note Shuffle");
    }
    {
        Settings t;
        t.sp_cap = 5;
        CHECK(settings_changes_summary(t) == "SP cap 5 bars");
        t.sp_cap = 1;
        CHECK(settings_changes_summary(t) == "SP cap 1 bar");
    }
    {
        Settings t;
        t.legacy_fills = true;
        CHECK(settings_changes_summary(t) == "1.0 fills");
    }
    {
        Settings t;
        t.depth_value = 100;
        t.depth_mode = 1;
        CHECK(settings_changes_summary(t) == "Score range 100 points");
        t.depth_value = 4;  // the unit alone differs
        CHECK(settings_changes_summary(t) == "Score range 4 points");
        t.depth_mode = 0;
        t.depth_value = 2000;  // the number alone differs
        CHECK(settings_changes_summary(t) == "Score range 2,000 scores");
    }
    {
        Settings t;
        t.mslimit_value = 20;
        CHECK(settings_changes_summary(t) == "Path limit 20 ms");
        t.mslimit_enabled = false;
        CHECK(settings_changes_summary(t) == "Path limit off");
        t.mslimit_value = 10;  // the tick alone differs
        CHECK(settings_changes_summary(t) == "Path limit off");
    }

    // Several at once keep the table's order.
    s.mslimit_value = 20;
    s.view_noteshuffle = true;
    s.view_difficulty = "Hard";
    s.sp_cap = 5;
    CHECK(settings_changes_summary(s) ==
          "Hard \xC2\xB7 Note Shuffle \xC2\xB7 SP cap 5 bars \xC2\xB7 Path limit 20 ms");
    CHECK(settings_button_label(s, true) ==
          "Analysis settings: Hard \xC2\xB7 Note Shuffle \xC2\xB7 SP cap 5 bars \xC2\xB7 "
          "Path limit 20 ms (locked)");
}

TEST_CASE("settings reset: puts back each panel setting and keeps the rest") {
    // Each of the panel's settings on its own: the summary names it, and the
    // reset clears it.
    const std::function<void(Settings&)> changes[] = {
        [](Settings& t) { t.view_difficulty = "Hard"; },
        [](Settings& t) { t.view_prodrums = false; },
        [](Settings& t) { t.view_bass2x = false; },
        [](Settings& t) { t.view_noteshuffle = true; },
        [](Settings& t) { t.sp_cap = 5; },
        [](Settings& t) { t.legacy_fills = true; },
        [](Settings& t) { t.depth_value = 100; },
        [](Settings& t) { t.depth_mode = 1; },
        [](Settings& t) { t.mslimit_enabled = false; },
        [](Settings& t) { t.mslimit_value = 20; },
    };
    for (const auto& change : changes) {
        Settings t;
        change(t);
        CHECK(!settings_at_defaults(t));
        CHECK(settings_at_defaults(t.with_analysis_defaults()));
    }

    // All at once, with the settings the panel doesn't show changed too.
    Settings s;
    for (const auto& change : changes) change(s);
    s.chartfolders = {"C:/Songs"};
    s.preview_volume = 75;
    s.hit_window_ms = 60.5;
    s.backendlimit_enabled = true;
    s.auto_open_report = true;
    s.dm_last_user = "1234";
    const Settings r = s.with_analysis_defaults();
    CHECK(settings_changes_summary(r) == "defaults");
    // Everything a stored result is keyed by, and everything the analysis
    // reads, matches the defaults', so a keyed setting the reset misses
    // fails here.
    const Settings d;
    CHECK(r.chartmode_key() == d.chartmode_key());
    CHECK(r.cap_query() == d.cap_query());
    CHECK(r.lens() == d.lens());
    const hydra::app::AnalysisSettings ra = r.to_analysis_settings();
    const hydra::app::AnalysisSettings da = d.to_analysis_settings();
    CHECK(hydra::settings_key(ra) == hydra::settings_key(da));
    CHECK(ra.prodrums == da.prodrums);
    CHECK(ra.bass2x == da.bass2x);
    CHECK(ra.noteshuffle == da.noteshuffle);
    CHECK(ra.difficulty == da.difficulty);
    CHECK(r.chartfolders == s.chartfolders);
    CHECK(r.preview_volume == 75);
    CHECK(r.hit_window_ms == 60.5);
    CHECK(r.backendlimit_enabled);
    CHECK(r.auto_open_report);
    CHECK(r.dm_last_user == "1234");
}

TEST_CASE("settings reset: the tooltip says what a click undoes, or why it can't") {
    Settings s;
    CHECK(reset_settings_tooltip(s, false) == "The settings are already the defaults.");
    CHECK(reset_settings_tooltip(s, true) == "Stop the batch to change these.");
    s.view_prodrums = false;
    s.sp_cap = 5;
    CHECK(reset_settings_tooltip(s, false) == "Resets: Pro Drums off \xC2\xB7 SP cap 5 bars");
    CHECK(reset_settings_tooltip(s, true) == "Stop the batch to change these.");
}

TEST_CASE("batch text: cap 1 reads 1 bar and cap 1000 reads 1,000 bars") {
    Settings s;
    s.sp_cap = 1;
    CHECK(batch_settings_summary(s).sp_cap == "1 bar (a what-if)");
    s.sp_cap = 1000;
    CHECK(batch_settings_summary(s).sp_cap == "1,000 bars (a what-if)");
}

// D74 item 3: the scan modal's running counts group thousands like every
// other count.
TEST_CASE("batch text: the scan dialog's counts read 1,234") {
    CHECK(scan_folders_found_text(1234) == "Discovering folders... (1,234 found)");
    CHECK(scan_reused_text(1234) == "1,234 unchanged since last scan, reused.");
}

TEST_CASE("batch text: the confirm's Fills line names the fill rule") {
    Settings s;
    s.legacy_fills = false;
    CHECK(batch_settings_summary(s).fills == "Clone Hero 1.1");
    s.legacy_fills = true;
    CHECK(batch_settings_summary(s).fills == "Clone Hero 1.0");
}

// D74 item 4: the tooltip's deadline sentences are the ones the reports use.
TEST_CASE("batch text: the 1.0 fills help text uses the shared fill-rule sentences") {
    CHECK(legacy_fills_help_text() ==
          "Spawn drum fills by Clone Hero 1.0's rule instead of 1.1's. A fill only appears if "
          "your Star Power was ready in time. Clone Hero 1.1 made it a flat 4 beats. Clone "
          "Hero 1.0 gave you until about one fill-length before the fill. For runs played on "
          "1.0; current Clone Hero plays by 1.1.");
}

TEST_CASE("batch text: the empty library says what to do next") {
    Settings s;
    s.chartfolders.clear();
    CHECK(std::string(empty_library_message(s)).find("Manage folders...") != std::string::npos);
    s.chartfolders.push_back("C:\\songs");
    CHECK(std::string(empty_library_message(s)).find("Scan library") != std::string::npos);
    CHECK(std::string(empty_library_message(s)).find("Manage folders") == std::string::npos);
}
