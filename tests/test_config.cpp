// Tests for app/config: INI round-trip through an explicit temp file,
// tolerance for malformed input, chartmode_key, and the Settings ->
// AnalysisSettings mapping (the SP cap included).

#include "doctest.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "core/model.h"
#include "core/version.h"
#include "core/winstr.h"
#include "temp_util.h"

using hydra::app::AnalysisSettings;
using hydra::app::Settings;

namespace {

// Loads settings from an INI holding exactly `text`, then deletes the file.
Settings load_ini_text(const char* tag, const std::string& text) {
    const std::string path = testtemp::temp_path(tag, ".ini");
    {
        std::ofstream f(path, std::ios::trunc);
        f << text;
    }
    Settings s = Settings::load_file(path);
    std::remove(path.c_str());
    return s;
}

// Every field the INI holds, compared one by one.
void check_same_settings(const Settings& r, const Settings& s) {
    CHECK(r.chartfolders == s.chartfolders);
    CHECK(r.is_rescan == s.is_rescan);
    CHECK(r.view_difficulty == s.view_difficulty);
    CHECK(r.view_prodrums == s.view_prodrums);
    CHECK(r.view_bass2x == s.view_bass2x);
    CHECK(r.depth_value == s.depth_value);
    CHECK(r.depth_mode == s.depth_mode);
    CHECK(r.mslimit_enabled == s.mslimit_enabled);
    CHECK(r.mslimit_value == s.mslimit_value);
    CHECK(r.backendlimit_enabled == s.backendlimit_enabled);
    CHECK(r.backendlimit_value == s.backendlimit_value);
    CHECK(r.hit_window_ms == s.hit_window_ms);
    CHECK(r.preview_volume == s.preview_volume);
    CHECK(r.sp_cap == s.sp_cap);
    CHECK(r.legacy_fills == s.legacy_fills);
    CHECK(r.auto_open_report == s.auto_open_report);
    CHECK(r.dm_last_user == s.dm_last_user);
}

}  // namespace

TEST_CASE("settings round-trip through an INI file") {
    Settings s;
    s.chartfolders = {"C:\\charts\\a", "C:\\charts\\b"};
    s.is_rescan = true;
    s.view_prodrums = false;
    s.view_bass2x = false;
    s.depth_value = 25;
    s.depth_mode = 1;
    s.mslimit_enabled = false;
    s.mslimit_value = 42;
    s.backendlimit_enabled = true;
    s.backendlimit_value = 42;
    s.hit_window_ms = 79;
    s.preview_volume = 23;
    s.sp_cap = 16;
    s.legacy_fills = true;
    s.auto_open_report = true;
    s.dm_last_user = "123456789";

    const std::string path = testtemp::temp_path("roundtrip", ".ini");
    REQUIRE(s.save_file(path));
    Settings r = Settings::load_file(path);
    std::remove(path.c_str());

    check_same_settings(r, s);
}

TEST_CASE("settings: load and save name the same keys") {
    // A default Settings reloads field for field equal.
    const std::string path = testtemp::temp_path("samekeys", ".ini");
    const Settings d;
    REQUIRE(d.save_file(path));
    check_same_settings(Settings::load_file(path), d);

    // The file's key names, in the order save_file writes them. Each one is a
    // name load_file reads (the round trip above proves it), so a name changed
    // on one side only fails here or there.
    Settings s;
    s.dm_last_user = "123";
    s.chartfolders = {"C:\\a"};
    REQUIRE(s.save_file(path));
    std::vector<std::string> names;
    {
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line)) names.push_back(line.substr(0, line.find('=')));
    }
    std::remove(path.c_str());
    const std::vector<std::string> expected = {
        "is_rescan",        "view_difficulty",      "view_prodrums",      "view_bass2x",
        "depth_value",      "depth_mode",           "mslimit_enabled",    "mslimit_value",
        "backendlimit_enabled", "backendlimit_value", "hit_window_ms",    "preview_volume",
        "sp_cap",           "legacy_fills",         "auto_open_report",   "dm_last_user",
        "chartfolder"};
    CHECK(names == expected);
}

TEST_CASE("the 1.0 fills setting reaches the search and the result's key together") {
    Settings s;
    CHECK_FALSE(s.legacy_fills);  // Clone Hero 1.1 by default
    CHECK_FALSE(s.to_analysis_settings().legacy_fill_deadline);
    CHECK(s.lens().legacy_fills == 0);

    s.legacy_fills = true;
    CHECK(s.to_analysis_settings().legacy_fill_deadline);
    CHECK(s.lens().legacy_fills == 1);
    CHECK(s.record_key("h").lens.legacy_fills == 1);
    const hydra::app::BatchRun run = s.batch_run();
    CHECK(run.settings.legacy_fill_deadline);
    CHECK(run.lens.legacy_fills == 1);
}

TEST_CASE("a missing INI yields defaults") {
    Settings r = Settings::load_file(testtemp::temp_path("missing_never_written", ".ini"));
    Settings d;
    CHECK(r.sp_cap == 4);
    CHECK(r.chartfolders.empty());
    CHECK(r.view_difficulty == d.view_difficulty);
    CHECK(r.depth_value == d.depth_value);
    CHECK(r.mslimit_enabled == d.mslimit_enabled);
    CHECK(r.mslimit_value == d.mslimit_value);
    CHECK(r.mslimit_value == 10);
    CHECK(r.backendlimit_enabled == false);
    CHECK(r.backendlimit_value == 50);
    CHECK(r.hit_window_ms == 85);
    CHECK(r.preview_volume == 40);
    CHECK_FALSE(r.legacy_fills);
}

// The app and the search start from one depth; this is the literal pin.
TEST_CASE("config: the default depth is the search's, 4") {
    CHECK(hydra::kDefaultDepthValue == 4);
    CHECK(Settings{}.depth_value == 4);
    CHECK(hydra::SearchSettings{}.depth_value == 4);
}

TEST_CASE("malformed INI lines are tolerated") {
    const std::string path = testtemp::temp_path("malformed", ".ini");
    {
        std::ofstream f(path, std::ios::trunc);
        f << "# a comment line\n"
          << "\n"
          << "   \t \n"
          << "no_equals_sign_here\n"
          << "unknown_key=whatever\n"
          << "depth_value=not_a_number\n"
          << "  mslimit_value =  25  \n"
          << "view_prodrums=0\n";
    }
    Settings r = Settings::load_file(path);
    std::remove(path.c_str());

    // "  mslimit_value =  25  ": the key and the value are trimmed.
    CHECK(r.mslimit_value == 25);
    // "view_prodrums=0": 0 is off.
    CHECK(r.view_prodrums == false);
    // "depth_value=not_a_number": junk reads as 0, and 0 is the depth floor.
    CHECK(r.depth_value == 0);
    // The comment, blank, "="-less and unknown lines set nothing.
    CHECK(r.chartfolders.empty());
}

TEST_CASE("settings: a value outside its range loads at the nearest edge (D51 Q14)") {
    // The Path limit's range is the squeeze window on either side.
    CHECK(load_ini_text("range1", "mslimit_value=900\n").mslimit_value ==
          static_cast<int>(hydra::kSqueezeWindowMs));
    CHECK(load_ini_text("range2", "mslimit_value=-900\n").mslimit_value ==
          -static_cast<int>(hydra::kSqueezeWindowMs));
    // A depth below 0 would crash the search (finding 311).
    CHECK(load_ini_text("range3", "depth_value=-1\n").depth_value == 0);
    // The Backend limit is 0 up to the squeeze window; the shown value and
    // the window the tables use are the same number (finding 31).
    const Settings back = load_ini_text("range4", "backendlimit_enabled=1\nbackendlimit_value=-30\n");
    CHECK(back.backendlimit_value == 0);
    REQUIRE(back.backend_limit().has_value());
    CHECK(*back.backend_limit() == 0.0);
    // Volume is 0 to 100 percent.
    CHECK(load_ini_text("range5", "preview_volume=150\n").preview_volume == 100);
    CHECK(load_ini_text("range6", "preview_volume=-5\n").preview_volume == 0);
    // depth_mode is a switch, not a range: anything but 1 is scores (D51
    // addendum), so the result is filed under the lens it searched by
    // (finding 56).
    CHECK(load_ini_text("range7", "depth_mode=2\n").depth_mode == 0);
    CHECK(load_ini_text("range8", "depth_mode=-1\n").depth_mode == 0);
    CHECK(load_ini_text("range9", "depth_mode=1\n").depth_mode == 1);
}

TEST_CASE("Settings::clamp is the one range every caller asks") {
    const int window = static_cast<int>(hydra::kSqueezeWindowMs);
    CHECK(Settings::clamp(&Settings::mslimit_value, 900) == window);
    CHECK(Settings::clamp(&Settings::mslimit_value, -900) == -window);
    CHECK(Settings::clamp(&Settings::mslimit_value, 25) == 25);
    CHECK(Settings::clamp(&Settings::backendlimit_value, -30) == 0);
    CHECK(Settings::clamp(&Settings::depth_value, -1) == 0);
    CHECK(Settings::clamp(&Settings::preview_volume, 150) == 100);
    // The cap's floor is 1 bar when typed (D51 Q16); only the file reads 0 as 4.
    CHECK(Settings::clamp(&Settings::sp_cap, 0) == 1);
    CHECK(load_ini_text("clamp1", "sp_cap=0\n").sp_cap == 4);
    // No edge to land on: the default. The hit window's case is
    // "settings: the hit window keeps a decimal", since it is not an int box.
    CHECK(Settings::clamp(&Settings::depth_mode, 2) == 0);
    // The volume percent as a gain.
    CHECK(Settings::volume_gain(40) == 0.4f);
    CHECK(Settings::volume_gain(150) == 1.0f);
    CHECK(Settings::volume_gain(-5) == 0.0f);
    // depth_mode as the search's enum.
    Settings s;
    s.depth_mode = 1;
    CHECK(s.search_depth_mode() == hydra::DepthMode::Points);
    s.depth_mode = 2;
    CHECK(s.search_depth_mode() == hydra::DepthMode::Scores);
}

TEST_CASE("settings: the hit window keeps a decimal, and 0 reads the default (D51 Q15)") {
    const Settings half = load_ini_text("hitwin1", "hit_window_ms=85.5\n");
    CHECK(half.hit_window_ms == 85.5);
    // 0 has no edge to land on (the file-load form of the old clamp line).
    CHECK(load_ini_text("hitwin2", "hit_window_ms=0\n").hit_window_ms == hydra::kDefaultHitWindowMs);

    // What save_file writes on the hit_window_ms line: 85.5 as typed, and the
    // default as the whole number today's files hold.
    const std::string path = testtemp::temp_path("hitwin3", ".ini");
    auto saved_line = [&path](const Settings& s) {
        REQUIRE(s.save_file(path));
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line))
            if (line.rfind("hit_window_ms=", 0) == 0) return line;
        return std::string();
    };
    CHECK(saved_line(half) == "hit_window_ms=85.5");
    CHECK(saved_line(Settings{}) == "hit_window_ms=85");
    std::remove(path.c_str());
}

TEST_CASE("settings: a # after a value is a comment (D51 Q15)") {
    const Settings hard = load_ini_text("hash1", "view_difficulty=Hard # practice\n");
    CHECK(hard.view_difficulty == "Hard");
    CHECK(hard.chartmode_key().rfind("Hard ", 0) == 0);
    CHECK(load_ini_text("hash2", "depth_value=7 # seven\n").depth_value == 7);
    // A folder path is free text: a # inside it is part of the folder's
    // name, as it always was, so a library folder never changes on load.
    CHECK(load_ini_text("hash3", "chartfolder=C:\\Songs\\#1 Hits\n").chartfolders ==
          std::vector<std::string>{"C:\\Songs\\#1 Hits"});
}

TEST_CASE("settings: on/off keys take 0 or 1 only") {
    // Anything but 0 or 1 leaves the setting as it was.
    CHECK(load_ini_text("bool1", "view_prodrums=true\n").view_prodrums == true);
    CHECK(load_ini_text("bool2", "view_prodrums=0\n").view_prodrums == false);
    CHECK(load_ini_text("bool3", "legacy_fills=yes\n").legacy_fills == false);
    CHECK(load_ini_text("bool4", "legacy_fills=1\n").legacy_fills == true);
}

TEST_CASE("chartmode_key builds its difficulty word from difficulty()") {
    // A word set in memory, never cleaned by load_file, still names a real
    // difficulty in the key (finding 134).
    Settings s;
    s.view_difficulty = "easy";
    CHECK(s.chartmode_key().rfind("Easy ", 0) == 0);
    s.view_difficulty = "Legendary";
    CHECK(s.chartmode_key().rfind("Expert ", 0) == 0);
}

TEST_CASE("chartmode_key names the view flags") {
    // All four Expert keys, byte for byte: every record already in a user's
    // store is filed under one of these, so none of them may ever move.
    Settings s;
    CHECK(s.chartmode_key() == "Expert Pro Drums, 2x Bass");
    s.view_bass2x = false;
    CHECK(s.chartmode_key() == "Expert Pro Drums, 1x Bass");
    s.view_prodrums = false;
    CHECK(s.chartmode_key() == "Expert Drums, 1x Bass");
    s.view_bass2x = true;
    CHECK(s.chartmode_key() == "Expert Drums, 2x Bass");
}

TEST_CASE("chartmode_key: 2x Bass applies at every difficulty (D20)") {
    Settings s;
    s.view_difficulty = "Hard";
    s.view_prodrums = true;
    s.view_bass2x = true;  // Clone Hero's Double Kick works at every difficulty
    CHECK(s.difficulty() == hydra::Difficulty::Hard);
    CHECK(s.effective_bass2x());
    CHECK(s.chartmode_key() == "Hard Pro Drums, 2x Bass");
    CHECK(s.to_analysis_settings().bass2x);
    CHECK(s.to_analysis_settings().difficulty == hydra::Difficulty::Hard);

    s.view_bass2x = false;
    CHECK_FALSE(s.effective_bass2x());
    CHECK(s.chartmode_key() == "Hard Pro Drums, 1x Bass");
    s.view_prodrums = false;
    CHECK(s.chartmode_key() == "Hard Drums, 1x Bass");
    s.view_difficulty = "Medium";
    CHECK(s.chartmode_key() == "Medium Drums, 1x Bass");
    s.view_bass2x = true;
    CHECK(s.chartmode_key() == "Medium Drums, 2x Bass");
    s.view_difficulty = "Easy";
    CHECK(s.chartmode_key() == "Easy Drums, 2x Bass");

    // Expert's keys don't move (the case above pins all four byte for byte).
    s.view_difficulty = "Expert";
    s.view_prodrums = true;
    CHECK(s.chartmode_key() == "Expert Pro Drums, 2x Bass");
}

TEST_CASE("view_difficulty round-trips, and a junk value normalizes to Expert") {
    const std::string path = testtemp::temp_path("difficulty", ".ini");

    Settings s;
    s.view_difficulty = "Hard";
    REQUIRE(s.save_file(path));
    Settings r = Settings::load_file(path);
    CHECK(r.view_difficulty == "Hard");
    CHECK(r.difficulty() == hydra::Difficulty::Hard);

    // A hand-edited INI can hold anything; the loaded settings never do, so a
    // junk word can't reach chartmode_key and invent a chartmode.
    {
        std::ofstream f(path, std::ios::trunc);
        f << "view_difficulty=Legendary\n";
    }
    Settings junk = Settings::load_file(path);
    CHECK(junk.view_difficulty == "Expert");
    CHECK(junk.difficulty() == hydra::Difficulty::Expert);
    CHECK(junk.chartmode_key() == "Expert Pro Drums, 2x Bass");
    std::remove(path.c_str());
}

TEST_CASE("view_difficulty matches any case and loads as the real name") {
    const std::string path = testtemp::temp_path("difficulty_case", ".ini");
    for (const char* word : {"hard", "HARD", "hArD"}) {
        CAPTURE(word);
        {
            std::ofstream f(path, std::ios::trunc);
            f << "view_difficulty=" << word << "\n";
        }
        Settings r = Settings::load_file(path);
        // What the app carries, and bakes into the store key, is the real name.
        CHECK(r.view_difficulty == "Hard");
        CHECK(r.difficulty() == hydra::Difficulty::Hard);
        // view_bass2x defaults on, and applies at Hard too (D20).
        CHECK(r.chartmode_key() == "Hard Pro Drums, 2x Bass");
    }
    std::remove(path.c_str());

    // The in-memory lookup matches any case too.
    Settings s;
    s.view_difficulty = "easy";
    CHECK(s.difficulty() == hydra::Difficulty::Easy);
}

TEST_CASE("record_key carries the chartmode, the SP cap and the lens") {
    namespace store = hydra::store;

    Settings s;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
    s.depth_mode = 0;
    s.depth_value = 4;
    store::RecordKey default_key = s.record_key("abc");
    CHECK(default_key.hyhash == "abc");
    CHECK(default_key.chartmode == s.chartmode_key());
    CHECK(default_key.cap == store::CapQuery::at(4));
    CHECK(default_key.lens == store::Lens::from(10, 0, 4));

    // A fixed cap asks for exactly that cap, and the chartmode follows the
    // view flags.
    s.sp_cap = 32;
    s.view_prodrums = false;
    store::RecordKey exact_key = s.record_key("abc");
    CHECK(exact_key.chartmode == "Expert Drums, 2x Bass");
    CHECK(exact_key.cap == store::CapQuery::at(32));

    // Each searched-over setting moves the lens...
    s.depth_mode = 1;
    s.depth_value = 5000;
    CHECK(s.record_key("abc").lens == store::Lens::from(10, 1, 5000));

    // ...and switching the ms limit off drops its number, because the search
    // stops reading it.
    s.mslimit_enabled = false;
    CHECK(s.record_key("abc").lens == store::Lens::from(std::nullopt, 1, 5000));
    CHECK(s.lens().ms_value == 0);
    s.mslimit_value = 42;
    CHECK(s.lens() == store::Lens::from(std::nullopt, 1, 5000));
}

TEST_CASE("batch_run bundles one Settings' chartmode, lens and search settings") {
    Settings s;
    s.view_difficulty = "Hard";
    s.view_prodrums = false;
    s.mslimit_enabled = false;
    s.depth_mode = 1;
    s.depth_value = 5000;
    s.sp_cap = 16;

    const hydra::app::BatchRun run = s.batch_run();
    CHECK(run.chartmode == s.chartmode_key());
    CHECK(run.lens == s.lens());
    CHECK(run.settings.difficulty == hydra::Difficulty::Hard);
    CHECK(run.settings.prodrums == false);
    CHECK(!run.settings.ms_filter.has_value());
    CHECK(run.settings.depth_mode == hydra::DepthMode::Points);
    CHECK(run.settings.depth_value == 5000);
    CHECK(run.settings.sp_cap == 16);
}

TEST_CASE("sp_cap round-trips as a number; auto, zero, junk and pre-1.6 keys read as 4") {
    const std::string path = testtemp::temp_path("spcap", ".ini");

    // A number reads back as that number, and cap_query asks for it exactly.
    Settings s;
    s.sp_cap = 64;
    REQUIRE(s.save_file(path));
    CHECK(Settings::load_file(path).sp_cap == 64);
    CHECK(Settings::load_file(path).cap_query().exact == 64);

    // Hydra 1.8.4 wrote "auto" for Auto, which is gone. It reads as Clone
    // Hero's 4, and saving writes the number back.
    {
        std::ofstream f(path, std::ios::trunc);
        f << "sp_cap=auto\n";
    }
    Settings from_auto = Settings::load_file(path);
    CHECK(from_auto.sp_cap == 4);
    CHECK(from_auto.cap_query().exact == 4);
    REQUIRE(from_auto.save_file(path));
    CHECK(Settings::load_file(path).sp_cap == 4);

    // The old Uncapped-edition keys, which the main app also used to write as
    // "off, 8", must not turn an existing INI into 8 bars. Garbage and zero
    // keep the default too.
    {
        std::ofstream f(path, std::ios::trunc);
        f << "sp_cap_enabled=0\n"
          << "sp_cap_value=8\n";
    }
    CHECK(Settings::load_file(path).sp_cap == 4);
    {
        std::ofstream f(path, std::ios::trunc);
        f << "sp_cap=0\n";
    }
    CHECK(Settings::load_file(path).sp_cap == 4);
    {
        std::ofstream f(path, std::ios::trunc);
        f << "sp_cap=banana\n";
    }
    CHECK(Settings::load_file(path).sp_cap == 4);
    std::remove(path.c_str());
}

TEST_CASE("to_analysis_settings maps the cap") {
    Settings s;
    s.depth_mode = 1;
    s.depth_value = 5000;
    s.mslimit_enabled = true;
    s.mslimit_value = 20;

    // The cap rides along as it is.
    s.sp_cap = 16;
    AnalysisSettings a = s.to_analysis_settings();
    CHECK(a.depth_mode == hydra::DepthMode::Points);
    CHECK(a.depth_value == 5000);
    CHECK(a.ms_filter == 20.0);
    CHECK(a.sp_cap == 16);

    // Disabled ms limit maps to no filter.
    s.mslimit_enabled = false;
    a = s.to_analysis_settings();
    CHECK_FALSE(a.ms_filter.has_value());
}

// The INI keeps depth_mode as a plain int, so the mapping to the search's
// enum is where a stray value has to land somewhere safe: anything that isn't
// 1 means the default, scores.
TEST_CASE("to_analysis_settings maps depth_mode onto the search's enum") {
    Settings s;

    s.depth_mode = 1;
    CHECK(s.to_analysis_settings().depth_mode == hydra::DepthMode::Points);

    s.depth_mode = 0;
    CHECK(s.to_analysis_settings().depth_mode == hydra::DepthMode::Scores);

    s.depth_mode = 7;  // out of range: falls back to scores, never a bad enum
    CHECK(s.to_analysis_settings().depth_mode == hydra::DepthMode::Scores);
}

TEST_CASE("config: resource_dir is the resource folder beside the exe") {
    const std::string dir = hydra::app::resource_dir();
    CHECK(hydra::parent_folder(dir) == hydra::app::exe_dir());
    const std::string tail = "resource";
    REQUIRE(dir.size() > tail.size());
    CHECK(dir.substr(dir.size() - tail.size()) == tail);
}

// The pieces are hydra_batch's header words; the expected text is the header
// as it printed before describe_settings existed.
TEST_CASE("config: describe_settings uses hydra_batch's header words") {
    Settings s;
    hydra::app::SettingsText text = hydra::app::describe_settings(s.to_analysis_settings());
    CHECK(text.depth == "scores 4");
    CHECK(text.cap == "4 bars");
    CHECK(text.timing == "10 ms");

    s.mslimit_enabled = false;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).timing == "none");

    s.depth_mode = 1;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).depth == "points 4");

    s.sp_cap = 1;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).cap == "1 bar");
}

// The one guard that CMakeLists.txt's two strings reach the exe unchanged.
TEST_CASE("version: the window title and taskbar id are the build's") {
    CHECK(std::wstring(hydra::kWindowTitleW) == L"Hydra Deluxe");
    CHECK(std::wstring(hydra::kAppUserModelIDW) == L"Hydra.Hydra");
}
