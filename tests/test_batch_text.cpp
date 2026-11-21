// The batch strip's and confirm's text helpers: counts with real plurals and
// thousands grouping, durations, and the settings lines the confirm lists.
// Also hydra_batch's choice of which charts a run reads.

#include "doctest.h"

#include <string>

#include "app/analysis.h"  // batch_cant_analyze_text
#include "app/config.h"
#include "app/display_format.h"  // format_duration, time_left_text
#include "app/report.h"  // hit_window_text
#include "cli/batch_scope.h"
#include "core/model.h"  // counted, format_ms_whole, fill_refuses, is_e0
#include "core/song.h"  // FillDeadlineRule
#include "search/graph.h"  // fill_rule_description, kCh11FillLeadBeats
#include "ui/library_parts.h"

using hydra::counted;
using hydra::app::Settings;
using namespace hydra::ui::detail;

// One, two and a thousand are tests/test_display_format.cpp's; zero is the
// case only this file states.
TEST_CASE("batch text: a count of zero takes the plural noun") {
    CHECK(counted(0, "chart", "charts") == "0 charts");
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
    CHECK(hydra::app::format_duration(12 * 60 + 3) == "12:03");
    CHECK(hydra::app::format_duration(3600 + 2 * 60 + 5) == "1:02:05");
    CHECK(hydra::app::format_duration(-3.0) == "0:00");
    // Rounds to the nearest second: a half second rounds up, so 59.5 s reads
    // a whole minute and 59.49 s does not.
    CHECK(hydra::app::format_duration(59.5) == "1:00");
    CHECK(hydra::app::format_duration(59.49) == "0:59");
}

TEST_CASE("batch text: time left reads about m:ss left") {
    CHECK(hydra::app::time_left_text(60 + 30) == "about 1:30 left");
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

    // Each setting on its own gives that setting's one phrase.
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

// The user's "Add a line" answer: the confirm lists the early-fill window,
// worded like Path limit.
TEST_CASE("batch text: the confirm's Early fill window line reads the window in ms") {
    Settings s;
    CHECK(batch_settings_summary(s).early_fill ==
          hydra::format_ms_whole(hydra::kDefaultEarlyFillWindowMs));
    s.early_fill_window_ms = hydra::kMaxEarlyFillWindowMs;
    CHECK(batch_settings_summary(s).early_fill ==
          hydra::format_ms_whole(hydra::kMaxEarlyFillWindowMs));
}

// The panel row's tooltip: one check per sentence, each beside the rule in
// core/model.h that makes it true.
TEST_CASE("batch text: the Early fill window help text") {
    const std::string help = early_fill_window_help_text();
    const auto says = [&help](const std::string& sentence) {
        INFO(sentence);
        CHECK(help.find(sentence) != std::string::npos);
    };
    const double w = hydra::kDefaultEarlyFillWindowMs;

    // fill_refuses: ready up to the window after the deadline still spawns.
    says("Ready up to this many ms after the deadline, you can still summon the fill by "
         "hitting early; later than that, the fill never spawns.");
    CHECK_FALSE(hydra::fill_refuses(-w, w));
    CHECK(hydra::fill_refuses(-w - 1.0, w));

    // is_e0: ready less than the window before the deadline is an E.
    says("Ready less than this many ms before the deadline, the activation is marked E.");
    CHECK(hydra::is_e0(w - 1.0, 0, w));
    CHECK_FALSE(hydra::is_e0(w, 0, w));

    // At a window of 0, an activation whose fill still spawns is no E.
    says("At 0 no activation is an early fill.");
    CHECK_FALSE(hydra::fill_refuses(0.0, 0.0));
    CHECK_FALSE(hydra::is_e0(0.0, 0, 0.0));
    CHECK(hydra::fill_refuses(-1.0, 0.0));

    // The most it allows is the hit window, and the setting clamps there.
    says(hydra::app::report::hit_window_text() + " ms, the hit window, is the most it allows.");
    CHECK(Settings::clamp(&Settings::early_fill_window_ms, hydra::kMaxEarlyFillWindowMs + 1) ==
          hydra::kMaxEarlyFillWindowMs);
}

// D74 item 4: the tooltip's deadline sentences are the ones the reports use,
// fill_rule_description's for each rule.
TEST_CASE("batch text: the 1.0 fills help text uses the shared fill-rule sentences") {
    const std::string help = legacy_fills_help_text();
    const std::string ch11 = hydra::fill_rule_description(hydra::FillDeadlineRule::Ch11);
    const std::string ch10 = hydra::fill_rule_description(hydra::FillDeadlineRule::Ch10);
    CHECK(help.find(ch11) != std::string::npos);
    CHECK(help.find(ch10) != std::string::npos);
    // The 1.1 sentence names the lead the search uses, in beats.
    CHECK(ch11.find(std::to_string(hydra::kCh11FillLeadBeats) + " beats") != std::string::npos);
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
