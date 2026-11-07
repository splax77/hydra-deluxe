// The batch strip's and confirm's text helpers: counts with real plurals and
// thousands grouping, durations, and the settings lines the confirm lists.
// Also hydra_batch's choice of which charts a run reads.

#include "doctest.h"

#include "app/analysis.h"  // batch_cant_analyze_text
#include "app/config.h"
#include "app/display_format.h"  // format_duration, time_left_text
#include "cli/batch_scope.h"
#include "core/model.h"  // counted
#include "ui/library_parts.h"

using hydra::counted;
using hydra::app::Settings;
using namespace hydra::ui::detail;

TEST_CASE("batch text: counts group thousands and pick the right noun") {
    CHECK(counted(0, "chart", "charts") == "0 charts");
    CHECK(counted(1, "chart", "charts") == "1 chart");
    CHECK(counted(2, "chart", "charts") == "2 charts");
    CHECK(counted(12345, "chart", "charts") == "12,345 charts");
}

// The user's answer 6 (2026-10-15): the confirm's dim line for the rows in
// scope that can't be analyzed, counted like the confirm's other counts.
TEST_CASE("batch text: the confirm's can't-analyze line") {
    CHECK(hydra::app::batch_cant_analyze_text(59) ==
          "59 of these can't be analyzed. Check the box to try them again.");
    CHECK(hydra::app::batch_cant_analyze_text(1) ==
          "1 of these can't be analyzed. Check the box to try them again.");
    CHECK(hydra::app::batch_cant_analyze_text(12345) ==
          "12,345 of these can't be analyzed. Check the box to try them again.");
}

TEST_CASE("batch text: durations read m:ss under an hour and h:mm:ss over it") {
    CHECK(hydra::app::format_duration(0.0) == "0:00");
    CHECK(hydra::app::format_duration(42.4) == "0:42");
    CHECK(hydra::app::format_duration(723.0) == "12:03");
    CHECK(hydra::app::format_duration(3725.0) == "1:02:05");
    CHECK(hydra::app::format_duration(-3.0) == "0:00");
}

TEST_CASE("batch text: time left reads about m:ss left") {
    CHECK(hydra::app::time_left_text(90.0) == "about 1:30 left");
    CHECK(hydra::app::time_left_text(42.4) == "about 0:42 left");
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

    // A depth_mode set in memory to anything but 1 searches by scores, so it
    // reads scores (Settings::search_depth_mode). The file never holds one:
    // load_file lands a stray number on 0 or 1.
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
          "Spawn drum fills by Clone Hero 1.0's rule instead of Clone Hero 1.1's. A fill only "
          "appears if your Star Power was ready in time. Clone Hero 1.1 sets the deadline 4 "
          "beats before the fill starts. Clone Hero 1.0 set the deadline about one fill-length "
          "before the fill starts. Tick this only for runs played on Clone Hero 1.0.");
}

TEST_CASE("batch text: the empty library says what to do next") {
    Settings s;
    s.chartfolders.clear();
    CHECK(std::string(empty_library_message(s)).find("Manage folders...") != std::string::npos);
    s.chartfolders.push_back("C:\\songs");
    CHECK(std::string(empty_library_message(s)).find("Scan library") != std::string::npos);
    CHECK(std::string(empty_library_message(s)).find("Manage folders") == std::string::npos);
}

// The hooks-from-scratch plan, section 2 (the user's approval, 2026-10-25): a
// whole-library run is asked for by name.
using hydra::cli::batch_scope;
using hydra::cli::batch_scope_error;
using hydra::cli::BatchScope;

TEST_CASE("hydra_batch: the whole library needs --whole-library, and folders run alone") {
    CHECK(batch_scope(true, false) == BatchScope::Library);
    CHECK(batch_scope(false, true) == BatchScope::Folders);
    CHECK(batch_scope(false, false) == BatchScope::Refused);
    CHECK(batch_scope(true, true) == BatchScope::Ambiguous);

    CHECK(batch_scope_error(BatchScope::Refused) ==
          "A run over the whole library needs --whole-library. Name chart folders to analyze "
          "just those.");
    CHECK(batch_scope_error(BatchScope::Ambiguous) ==
          "--whole-library analyzes every folder in the app's settings, so it takes no folder "
          "names.");
    CHECK(batch_scope_error(BatchScope::Library).empty());
    CHECK(batch_scope_error(BatchScope::Folders).empty());
}
