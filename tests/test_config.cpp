// Tests for app/config: INI round-trip through an explicit temp file,
// tolerance for malformed input, chartmode_key, and the Settings ->
// AnalysisSettings mapping (the SP cap included).

#include "doctest.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"  // AnalysisSettings
#include "app/config.h"
#include "core/model.h"
#include "core/strutil.h"
#include "core/version.h"
#include "core/winstr.h"
#include "scratch_paths.h"
#include "search/pather.h"  // kDefaultDepthValue, SearchSettings
#include "store/record_store.h"  // CapQuery, Lens
#include "temp_util.h"
#include "ui/library_parts.h"  // settings_at_defaults, settings_changes_summary

using hydra::app::AnalysisSettings;
using hydra::app::Settings;
using hydra::ui::detail::settings_at_defaults;
using hydra::ui::detail::settings_changes_summary;

namespace {

// Loads settings from an INI holding exactly `text`, then deletes the file,
// whether the load returns or throws.
Settings load_ini_text(const char* tag, const std::string& text) {
    const std::string path = testtemp::temp_path(tag, ".ini");
    {
        std::ofstream f(path, std::ios::trunc);
        f << text;
    }
    struct Remove {
        const std::string& path;
        ~Remove() { std::remove(path.c_str()); }
    } remove_after{path};
    return Settings::load_file(path);
}

// Every field the INI holds, compared one by one.
void check_same_settings(const Settings& r, const Settings& s) {
    CHECK(r.chartfolders == s.chartfolders);
    CHECK(r.is_rescan == s.is_rescan);
    CHECK(r.view_difficulty == s.view_difficulty);
    CHECK(r.view_prodrums == s.view_prodrums);
    CHECK(r.view_bass2x == s.view_bass2x);
    CHECK(r.view_noteshuffle == s.view_noteshuffle);
    CHECK(r.depth_value == s.depth_value);
    CHECK(r.depth_mode == s.depth_mode);
    CHECK(r.mslimit_enabled == s.mslimit_enabled);
    CHECK(r.mslimit_value == s.mslimit_value);
    CHECK(r.backendlimit_enabled == s.backendlimit_enabled);
    CHECK(r.backendlimit_value == s.backendlimit_value);
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
    s.view_noteshuffle = true;
    s.depth_value = 25;
    s.depth_mode = 1;
    s.mslimit_enabled = false;
    s.mslimit_value = 42;
    s.backendlimit_enabled = true;
    s.backendlimit_value = 42;
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
        "view_noteshuffle",
        "depth_value",     "depth_mode",           "mslimit_enabled",    "mslimit_value",
        "backendlimit_enabled", "backendlimit_value", "preview_volume",
        "sp_cap",           "legacy_fills",         "auto_open_report",   "dm_last_user",
        "chartfolder"};
    CHECK(names == expected);
}

// The updater's two keys (D260): they round-trip and Reset to defaults leaves
// them alone. A 2.11.x file's check_updates and last_update_check lines load
// and are not written back.
TEST_CASE("the updater's settings round-trip and survive Reset to defaults") {
    Settings s;
    s.skipped_update_version = "9.9.9";
    s.last_run_version = "2.10.1";
    s.view_prodrums = false;  // an analysis setting, which the reset does put back

    const std::string path = testtemp::temp_path("update_keys", ".ini");
    REQUIRE(s.save_file(path));
    const Settings r = Settings::load_file(path);
    CHECK(r.skipped_update_version == "9.9.9");
    CHECK(r.last_run_version == "2.10.1");

    {
        std::ofstream f(path, std::ios::trunc);
        f << "check_updates=0\nlast_update_check=2026-10-22T13:45:00Z\nskipped_update_version=9.9.9\n";
    }
    const Settings old = Settings::load_file(path);
    CHECK(old.skipped_update_version == "9.9.9");
    REQUIRE(old.save_file(path));
    {
        std::ifstream in(path);
        const std::string saved{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        CHECK(saved.find("check_updates=") == std::string::npos);
        CHECK(saved.find("last_update_check=") == std::string::npos);
    }
    std::remove(path.c_str());

    const Settings reset = s.with_analysis_defaults();
    CHECK(reset.view_prodrums == Settings{}.view_prodrums);
    CHECK(reset.skipped_update_version == "9.9.9");
    CHECK(reset.last_run_version == "2.10.1");
}

TEST_CASE("the 1.0 fills setting reaches the search and the result's key together") {
    Settings s;
    CHECK_FALSE(s.legacy_fills);  // Clone Hero 1.1 by default
    CHECK_FALSE(s.to_analysis_settings().legacy_fills);
    CHECK(s.lens().legacy_fills == 0);

    s.legacy_fills = true;
    CHECK(s.to_analysis_settings().legacy_fills);
    CHECK(s.lens().legacy_fills == 1);
    CHECK(s.profile().lens.legacy_fills == 1);
    const hydra::app::BatchRun run = s.batch_run();
    CHECK(run.settings.legacy_fills);
    CHECK(run.profile.lens.legacy_fills == 1);
}

TEST_CASE("command-line settings take the fill rule from the flag, the rest from the INI") {
    // hydra_batch and hydra_bench both load through load_for_command_line, so
    // a bench digest matches a batch run whatever the app's "1.0 fills" box
    // was left on (perf follow-up B).
    const ScratchPaths paths("cmdline_fills");
    auto write_ini = [&](const std::string& text) {
        std::ofstream f(paths.ini, std::ios::trunc);
        f << text;
    };

    write_ini("legacy_fills=1\nsp_cap=2\n");
    const Settings off = Settings::load_for_command_line(false);
    CHECK_FALSE(off.legacy_fills);
    CHECK_FALSE(off.batch_run().settings.legacy_fills);
    CHECK(off.sp_cap == 2);  // every other setting still comes from the INI

    write_ini("legacy_fills=0\n");
    const Settings on = Settings::load_for_command_line(true);
    CHECK(on.legacy_fills);
    CHECK(on.batch_run().profile.lens.legacy_fills == 1);
}

// The tools' one loader (D270 call 4). The text it prints is checked through
// the tools themselves in test_cli.cpp.
TEST_CASE("load_tool_settings gives the tools' settings, or exit code 2 for a refused file") {
    const ScratchPaths paths("tool_settings");
    auto write_ini = [&](const std::string& text) {
        std::ofstream f(paths.ini, std::ios::trunc);
        f << text;
    };

    write_ini("legacy_fills=1\nsp_cap=2\n");
    Settings st;
    CHECK(hydra::app::load_tool_settings(st, std::nullopt) == 0);
    CHECK(st.legacy_fills);  // no flag of its own: the file's setting stands
    CHECK(st.sp_cap == 2);
    CHECK(hydra::app::load_tool_settings(st, false) == 0);
    CHECK_FALSE(st.legacy_fills);  // the tool's flag wins
    CHECK(st.sp_cap == 2);

    write_ini("sp_cap=abc\n");
    Settings untouched;
    untouched.sp_cap = 7;
    CHECK(hydra::app::load_tool_settings(untouched, std::nullopt) == 2);
    CHECK(hydra::app::load_tool_settings(untouched, true) == 2);
    CHECK(untouched.sp_cap == 7);
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
    CHECK(r.preview_volume == 40);
    CHECK_FALSE(r.legacy_fills);
}

// The app and the search start from one depth; this is the literal pin.
TEST_CASE("config: the default depth is the search's, 4") {
    CHECK(hydra::kDefaultDepthValue == 4);
    CHECK(Settings{}.depth_value == 4);
    CHECK(hydra::SearchSettings{}.depth_value == 4);
}

TEST_CASE("blank and comment lines load, and keys and values are trimmed") {
    const Settings r = load_ini_text("tidy", "# a comment line\n"
                                             "\n"
                                             "   \t \n"
                                             "  mslimit_value =  25  \n"
                                             "view_prodrums=0\n");
    // "  mslimit_value =  25  ": the key and the value are trimmed.
    CHECK(r.mslimit_value == 25);
    // "view_prodrums=0": 0 is off.
    CHECK(r.view_prodrums == false);
    // The comment and blank lines set nothing.
    CHECK(r.chartfolders.empty());
}

// D270 call 4 ("refuse anything bad"), calls 5 and 6, D276: each bad line on
// its own stops the load with an error naming the file, the key and the value.
TEST_CASE("a bad line refuses") {
    const std::string path = testtemp::temp_path("badline", ".ini");
    // Loads a file holding just `line`.
    auto load = [&path](const char* line) {
        {
            std::ofstream f(path, std::ios::trunc);
            f << line << "\n";
        }
        return Settings::load_file(path);
    };
    using hydra::app::SettingsFileError;
    CHECK_THROWS_WITH_AS(load("no_equals_sign_here"),
                         (path + ": \"no_equals_sign_here\" is not key = value").c_str(),
                         SettingsFileError);
    CHECK_THROWS_WITH_AS(load("unknown_key=whatever"),
                         (path + ": unknown key \"unknown_key\"").c_str(), SettingsFileError);
    CHECK_THROWS_WITH_AS(load("depth_value=not_a_number"),
                         (path + ": depth_value = \"not_a_number\" is not a whole number").c_str(),
                         SettingsFileError);
    CHECK_THROWS_WITH_AS(load("view_prodrums=yes"),
                         (path + ": view_prodrums = \"yes\" is not 0 or 1").c_str(),
                         SettingsFileError);
    CHECK_THROWS_WITH_AS(
        load("view_difficulty=Expret"),
        (path + ": view_difficulty = \"Expret\" is not Expert, Hard, Medium or Easy").c_str(),
        SettingsFileError);
    CHECK_THROWS_WITH_AS(load("sp_cap=abc"),
                         (path + ": sp_cap = \"abc\" is not a whole number").c_str(),
                         SettingsFileError);
    CHECK_THROWS_WITH_AS(load("sp_cap=auto"),
                         (path + ": sp_cap = \"auto\" is not a whole number").c_str(),
                         SettingsFileError);
    // A decimal in a whole-number key is not a clean number (D276).
    CHECK_THROWS_WITH_AS(load("preview_volume=40.5"),
                         (path + ": preview_volume = \"40.5\" is not a whole number").c_str(),
                         SettingsFileError);
    std::remove(path.c_str());
}

// D51 Q14, D270 calls 1 and 2.
TEST_CASE("settings: a value outside its range loads at the nearest edge") {
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
    CHECK(load_ini_text("range4b", "backendlimit_value=900\n").backendlimit_value ==
          static_cast<int>(hydra::kSqueezeWindowMs));
    // Volume is 0 to 100 percent.
    CHECK(load_ini_text("range5", "preview_volume=150\n").preview_volume == 100);
    CHECK(load_ini_text("range6", "preview_volume=-5\n").preview_volume == 0);
    // depth_mode's range is 0 to 1, so a stray number lands on an edge too.
    CHECK(load_ini_text("range7", "depth_mode=7\n").depth_mode == 1);
    CHECK(load_ini_text("range8", "depth_mode=-1\n").depth_mode == 0);
    CHECK(load_ini_text("range9", "depth_mode=1\n").depth_mode == 1);
    // The cap's floor is 1 bar, in the file as in the box.
    CHECK(load_ini_text("range10", "sp_cap=0\n").sp_cap == 1);
    CHECK(load_ini_text("range11", "sp_cap=-3\n").sp_cap == 1);
}

TEST_CASE("Settings::clamp is the one range every caller asks") {
    const int window = static_cast<int>(hydra::kSqueezeWindowMs);
    CHECK(Settings::clamp(&Settings::mslimit_value, 900) == window);
    CHECK(Settings::clamp(&Settings::mslimit_value, -900) == -window);
    CHECK(Settings::clamp(&Settings::mslimit_value, 25) == 25);
    CHECK(Settings::clamp(&Settings::backendlimit_value, -30) == 0);
    CHECK(Settings::clamp(&Settings::depth_value, -1) == 0);
    CHECK(Settings::clamp(&Settings::preview_volume, 150) == 100);
    // The cap's floor is 1 bar (D51 Q16).
    CHECK(Settings::clamp(&Settings::sp_cap, 0) == 1);
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

// D51 Q15.
TEST_CASE("settings: a # after a value is a comment") {
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
    // Anything but 0 or 1 refuses (D270 call 4).
    CHECK_THROWS_AS(load_ini_text("bool1", "view_prodrums=true\n"),
                    hydra::app::SettingsFileError);
    CHECK(load_ini_text("bool2", "view_prodrums=0\n").view_prodrums == false);
    CHECK_THROWS_AS(load_ini_text("bool3", "legacy_fills=yes\n"), hydra::app::SettingsFileError);
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

namespace {

// One Difficulty, Pro Drums and 2x Bass choice, with the key it spells with
// Note Shuffle off and on. The off keys are the ones already in users'
// stores, so they never move, byte for byte; the on keys are D104's.
struct ModeCase {
    const char* difficulty;
    hydra::Difficulty level;  // what difficulty() reads from that word
    bool prodrums;
    bool bass2x;
    const char* key_off;
    const char* key_on;
};

const ModeCase kModeCases[] = {
    {"Expert", hydra::Difficulty::Expert, true, true, "Expert Pro Drums, 2x Bass", "Expert Pro Drums, 2x Bass, Note Shuffle"},
    {"Expert", hydra::Difficulty::Expert, true, false, "Expert Pro Drums, 1x Bass", "Expert Pro Drums, 1x Bass, Note Shuffle"},
    {"Expert", hydra::Difficulty::Expert, false, true, "Expert Drums, 2x Bass", "Expert Drums, 2x Bass, Note Shuffle"},
    {"Expert", hydra::Difficulty::Expert, false, false, "Expert Drums, 1x Bass", "Expert Drums, 1x Bass, Note Shuffle"},
    {"Hard", hydra::Difficulty::Hard, true, true, "Hard Pro Drums, 2x Bass", "Hard Pro Drums, 2x Bass, Note Shuffle"},
    {"Hard", hydra::Difficulty::Hard, true, false, "Hard Pro Drums, 1x Bass", "Hard Pro Drums, 1x Bass, Note Shuffle"},
    {"Hard", hydra::Difficulty::Hard, false, true, "Hard Drums, 2x Bass", "Hard Drums, 2x Bass, Note Shuffle"},
    {"Hard", hydra::Difficulty::Hard, false, false, "Hard Drums, 1x Bass", "Hard Drums, 1x Bass, Note Shuffle"},
    {"Medium", hydra::Difficulty::Medium, true, true, "Medium Pro Drums, 2x Bass", "Medium Pro Drums, 2x Bass, Note Shuffle"},
    {"Medium", hydra::Difficulty::Medium, true, false, "Medium Pro Drums, 1x Bass", "Medium Pro Drums, 1x Bass, Note Shuffle"},
    {"Medium", hydra::Difficulty::Medium, false, true, "Medium Drums, 2x Bass", "Medium Drums, 2x Bass, Note Shuffle"},
    {"Medium", hydra::Difficulty::Medium, false, false, "Medium Drums, 1x Bass", "Medium Drums, 1x Bass, Note Shuffle"},
    {"Easy", hydra::Difficulty::Easy, true, true, "Easy Pro Drums, 2x Bass", "Easy Pro Drums, 2x Bass, Note Shuffle"},
    {"Easy", hydra::Difficulty::Easy, true, false, "Easy Pro Drums, 1x Bass", "Easy Pro Drums, 1x Bass, Note Shuffle"},
    {"Easy", hydra::Difficulty::Easy, false, true, "Easy Drums, 2x Bass", "Easy Drums, 2x Bass, Note Shuffle"},
    {"Easy", hydra::Difficulty::Easy, false, false, "Easy Drums, 1x Bass", "Easy Drums, 1x Bass, Note Shuffle"},
};

}  // namespace

// D104.
TEST_CASE("chartmode_key: Note Shuffle off leaves every key as it was, on adds an ending") {
    for (const ModeCase& c : kModeCases) {
        CAPTURE(c.key_off);
        Settings s;
        s.view_difficulty = c.difficulty;
        s.view_prodrums = c.prodrums;
        s.view_bass2x = c.bass2x;
        CHECK(s.difficulty() == c.level);
        CHECK(s.to_analysis_settings().difficulty == c.level);
        // The 2x Bass box counts at every difficulty, not only Expert (D20).
        CHECK(s.effective_bass2x() == c.bass2x);
        CHECK(s.to_analysis_settings().bass2x == c.bass2x);
        CHECK_FALSE(s.view_noteshuffle);  // off by default
        CHECK(s.chartmode_key() == c.key_off);
        s.view_noteshuffle = true;
        CHECK(s.chartmode_key() == c.key_on);
    }
}

TEST_CASE("with_chartmode rebuilds the settings behind every key, shuffled or not") {
    // A path-report row carries only its key; clicking it rebuilds the
    // settings from that key, so every key the app can file must come back.
    Settings base;
    base.sp_cap = 7;  // a setting the key doesn't name stays as it was
    for (const ModeCase& c : kModeCases) {
        for (bool shuffled : {false, true}) {
            const std::string key = shuffled ? c.key_on : c.key_off;
            CAPTURE(key);
            const std::optional<Settings> mode = base.with_chartmode(key);
            REQUIRE(mode.has_value());
            CHECK(mode->view_difficulty == c.difficulty);
            CHECK(mode->view_prodrums == c.prodrums);
            CHECK(mode->view_bass2x == c.bass2x);
            CHECK(mode->view_noteshuffle == shuffled);
            CHECK(mode->sp_cap == 7);
            CHECK(mode->chartmode_key() == key);
        }
    }
    CHECK_FALSE(base.with_chartmode("Expert Pro Drums, 2x Bass, Shuffled").has_value());
}

TEST_CASE("with_reading: reading() gives back the reading written, the rest kept") {
    Settings base;
    base.sp_cap = 7;
    const hydra::ChartReading hard{hydra::Difficulty::Hard, false, false, true};
    const Settings s = base.with_reading(hard);
    CHECK(s.reading() == hard);
    CHECK(s.view_difficulty == "Hard");
    CHECK(s.sp_cap == 7);
    CHECK(base.with_reading(base.reading()).chartmode_key() == base.chartmode_key());
}

// The tests that write a bare ChartReading{} lean on this pin: change a default
// and this case names it, instead of a dozen unrelated expectations moving.
TEST_CASE("the default ChartReading is Expert, Pro Drums, 2x Bass, no Note Shuffle") {
    CHECK(hydra::ChartReading{} ==
          hydra::ChartReading{hydra::Difficulty::Expert, true, true, false});
    CHECK(Settings{}.reading() == hydra::ChartReading{});
}

TEST_CASE("view_noteshuffle: off by default, and the INI key loads and saves it") {
    CHECK_FALSE(Settings{}.view_noteshuffle);
    CHECK_FALSE(load_ini_text("ns_missing", "view_prodrums=1\n").view_noteshuffle);
    CHECK(load_ini_text("ns_on", "view_noteshuffle=1\n").view_noteshuffle);
    CHECK_FALSE(load_ini_text("ns_off", "view_noteshuffle=0\n").view_noteshuffle);

    const std::string path = testtemp::temp_path("noteshuffle", ".ini");
    Settings s;
    s.view_noteshuffle = true;
    REQUIRE(s.save_file(path));
    std::vector<std::string> lines;
    {
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line)) lines.push_back(line);
    }
    CHECK(Settings::load_file(path).view_noteshuffle);
    std::remove(path.c_str());
    CHECK(std::find(lines.begin(), lines.end(), "view_noteshuffle=1") != lines.end());
}

TEST_CASE("view_difficulty round-trips, and a junk value refuses") {
    const std::string path = testtemp::temp_path("difficulty", ".ini");

    Settings s;
    s.view_difficulty = "Hard";
    REQUIRE(s.save_file(path));
    Settings r = Settings::load_file(path);
    CHECK(r.view_difficulty == "Hard");
    CHECK(r.difficulty() == hydra::Difficulty::Hard);
    std::remove(path.c_str());

    // A hand-edited INI can hold anything; a word that names no difficulty
    // stops the load (D270 call 4), so it can't reach chartmode_key.
    CHECK_THROWS_AS(load_ini_text("difficulty_junk", "view_difficulty=Legendary\n"),
                    hydra::app::SettingsFileError);
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

TEST_CASE("profile carries the chartmode, the SP cap and the lens") {
    namespace store = hydra::store;

    Settings s;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
    s.depth_mode = 0;
    s.depth_value = 4;
    const store::Profile default_profile = s.profile();
    CHECK(default_profile.chartmode == s.chartmode_key());
    CHECK(default_profile.cap == store::CapQuery::at(4));
    CHECK(default_profile.lens == store::Lens::from(10, 0, 4));

    // A fixed cap asks for exactly that cap, and the chartmode follows the
    // view flags.
    s.sp_cap = 32;
    s.view_prodrums = false;
    const store::Profile exact_profile = s.profile();
    CHECK(exact_profile.chartmode == "Expert Drums, 2x Bass");
    CHECK(exact_profile.cap == store::CapQuery::at(32));

    // Each searched-over setting moves the lens...
    s.depth_mode = 1;
    s.depth_value = 5000;
    CHECK(s.profile().lens == store::Lens::from(10, 1, 5000));

    // ...and switching the ms limit off drops its number, because the search
    // stops reading it.
    s.mslimit_enabled = false;
    CHECK(s.profile().lens == store::Lens::from(std::nullopt, 1, 5000));
    CHECK(s.lens().ms_value == 0);
    s.mslimit_value = 42;
    CHECK(s.lens() == store::Lens::from(std::nullopt, 1, 5000));
}

namespace {

// Settings with every key part moved off its default: the chartmode (Pro
// Drums off), the SP cap, and each lens field (ms limit, score range, fills).
Settings every_key_part_changed() {
    Settings s;
    s.view_prodrums = false;
    s.sp_cap = 32;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
    s.depth_mode = 1;
    s.depth_value = 5000;
    s.legacy_fills = true;
    return s;
}

}  // namespace

TEST_CASE("profile moves with every key part") {
    namespace store = hydra::store;

    const Settings defaults;
    const Settings s = every_key_part_changed();
    CHECK(s.profile().chartmode == "Expert Drums, 2x Bass");
    CHECK(s.profile().cap == store::CapQuery::at(32));
    CHECK(s.profile().lens == store::Lens::from(10, 1, 5000, true));
    CHECK(s.profile() != defaults.profile());
}

TEST_CASE("profile_at_clone_hero_cap differs from profile only in the cap") {
    namespace store = hydra::store;

    Settings s = every_key_part_changed();
    s.sp_cap = 8;
    const store::Profile at_ch = s.profile_at_clone_hero_cap();
    CHECK(at_ch.cap == store::CapQuery::at(hydra::kCloneHeroSpCap));
    CHECK(at_ch.cap != s.profile().cap);
    CHECK(at_ch.chartmode == s.profile().chartmode);
    CHECK(at_ch.lens == s.profile().lens);

    // Already at Clone Hero's cap, the variant is profile() itself.
    s.sp_cap = hydra::kCloneHeroSpCap;
    CHECK(s.profile_at_clone_hero_cap() == s.profile());
}

TEST_CASE("profile_with_fills differs from profile only in the fill rule") {
    namespace store = hydra::store;

    Settings s = every_key_part_changed();
    s.legacy_fills = false;
    const store::Profile ch10 = s.profile_with_fills(hydra::FillDeadlineRule::Ch10);
    CHECK(ch10.lens.legacy_fills == 1);
    CHECK(ch10.chartmode == s.profile().chartmode);
    CHECK(ch10.cap == s.profile().cap);
    store::Lens rest = ch10.lens;
    rest.legacy_fills = s.profile().lens.legacy_fills;
    CHECK(rest == s.profile().lens);

    // The other way round: 1.0 fills on, asked for the 1.1 rule.
    s.legacy_fills = true;
    const store::Profile ch11 = s.profile_with_fills(hydra::FillDeadlineRule::Ch11);
    CHECK(ch11.lens.legacy_fills == 0);
    CHECK(s.profile_with_fills(hydra::FillDeadlineRule::Ch10) == s.profile());
}

TEST_CASE("each retired settings key loads silently and is never written back") {
    for (const hydra::app::RetiredKey& r : hydra::app::kRetiredSettingsKeys) {
        CAPTURE(r.name);
        CHECK_FALSE(r.retired_in.empty());
        const std::string path = testtemp::temp_path("retired_key", ".ini");
        {
            std::ofstream f(path, std::ios::trunc);
            f << r.name << "=8\n";
        }
        const Settings s = Settings::load_file(path);
        CHECK(s.analysis_changes().empty());
        // save_file writes every live key, so a retired one must not appear.
        REQUIRE(s.save_file(path));
        std::ifstream in(path);
        const std::string saved{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        in.close();
        CHECK(saved.find(std::string(r.name) + "=") == std::string::npos);
        std::remove(path.c_str());
    }
}

// Every settings file Hydra wrote since the hit window was added holds this
// line (D270 call 7, D276), so it is named here and not left to the list loop.
TEST_CASE("hit_window_ms in an older file is skipped, whatever it says, and dropped on save") {
    for (const char* value : {"85", "85.5", "banana"}) {
        CAPTURE(value);
        const Settings s = load_ini_text("hitwin_retired", std::string("hit_window_ms=") + value +
                                                              "\nsp_cap=8\n");
        // The live line beside it still reads.
        CHECK(s.sp_cap == 8);
    }
    const std::string path = testtemp::temp_path("hitwin_dropped", ".ini");
    {
        std::ofstream f(path, std::ios::trunc);
        f << "hit_window_ms=85\n";
    }
    REQUIRE(Settings::load_file(path).save_file(path));
    std::ifstream in(path);
    const std::string saved{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    in.close();
    CHECK(saved.find("hit_window_ms") == std::string::npos);
    std::remove(path.c_str());
}

// The whole-number test is app::parse_whole_number's, the one the rules file
// reads through (D270 call 4): a whole value spelled as a decimal reads, one
// too big for an int refuses.
TEST_CASE("settings: a whole-number key reads a whole decimal and refuses a number too big") {
    CHECK(load_ini_text("whole1", "sp_cap=8.0\n").sp_cap == 8);
    CHECK(load_ini_text("whole2", "preview_volume=1e1\n").preview_volume == 10);
    CHECK_THROWS_AS(load_ini_text("whole3", "sp_cap=99999999999\n"),
                    hydra::app::SettingsFileError);
}

TEST_CASE("legacy_fills_for reads fill_rule_for backwards") {
    using hydra::FillDeadlineRule;
    CHECK(hydra::legacy_fills_for(FillDeadlineRule::Ch10));
    CHECK_FALSE(hydra::legacy_fills_for(FillDeadlineRule::Ch11));
    for (bool flag : {false, true})
        CHECK(hydra::legacy_fills_for(hydra::fill_rule_for(flag)) == flag);
}

TEST_CASE("batch_run bundles one Settings' profile and search settings") {
    Settings s;
    s.view_difficulty = "Hard";
    s.view_prodrums = false;
    s.mslimit_enabled = false;
    s.depth_mode = 1;
    s.depth_value = 5000;
    s.sp_cap = 16;

    const hydra::app::BatchRun run = s.batch_run();
    // The run files under Settings::profile, so the batch and every other
    // reader of these settings ask for the same records.
    CHECK(run.profile == s.profile());
    // The profile's cap and the search's cap are the one setting.
    CHECK(run.profile.cap == hydra::store::CapQuery::at(16));
    CHECK(run.settings.difficulty == hydra::Difficulty::Hard);
    CHECK(run.settings.prodrums == false);
    CHECK(!run.settings.ms_filter.has_value());
    CHECK(run.settings.depth_mode == hydra::DepthMode::Points);
    CHECK(run.settings.depth_value == 5000);
    CHECK(run.settings.sp_cap == 16);
}

TEST_CASE("sp_cap: 0 reads 1, auto and junk refuse, pre-1.6 keys are skipped") {
    const std::string path = testtemp::temp_path("spcap", ".ini");

    // A number reads back as that number, and cap_query asks for it exactly.
    Settings s;
    s.sp_cap = 64;
    REQUIRE(s.save_file(path));
    CHECK(Settings::load_file(path).sp_cap == 64);
    CHECK(Settings::load_file(path).cap_query().exact == 64);

    // Hydra 1.8.4 wrote "auto" for Auto, which is gone; old versions get no
    // support (D270 call 6), so it refuses like any junk.
    std::remove(path.c_str());
    CHECK_THROWS_AS(load_ini_text("spcap_auto", "sp_cap=auto\n"), hydra::app::SettingsFileError);
    CHECK_THROWS_AS(load_ini_text("spcap_junk", "sp_cap=banana\n"),
                    hydra::app::SettingsFileError);
    // 0 lands on the 1-bar floor (D270 call 1).
    CHECK(load_ini_text("spcap_zero", "sp_cap=0\n").sp_cap == 1);

    // The old Uncapped-edition keys, which the main app also used to write as
    // "off, 8", are retired: skipped, so they never turn a file into 8 bars.
    CHECK(load_ini_text("spcap_old", "sp_cap_enabled=0\nsp_cap_value=8\n").sp_cap == 4);
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

// save_file goes through write_file_replacing (D114 item 20): a second save
// replaces the first whole and leaves no .tmp beside it.
TEST_CASE("save_file replaces the settings file through a temp and a rename") {
    const std::string path = testtemp::temp_path("settings_replace", ".ini");
    Settings s;
    s.sp_cap = 6;
    REQUIRE(s.save_file(path));
    s.sp_cap = 3;
    REQUIRE(s.save_file(path));
    CHECK(Settings::load_file(path).sp_cap == 3);
    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));
    std::remove(path.c_str());
}

// The settings file keeps Windows CRLF line ends, the shape it had before it
// went through write_file_replacing (D114 item 28).
TEST_CASE("save_file writes CRLF line ends and no bare LF") {
    const std::string path = testtemp::temp_path("settings_crlf", ".ini");
    Settings s;
    REQUIRE(s.save_file(path));
    std::ifstream in(path, std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    CHECK(bytes.find("\r\n") != std::string::npos);
    size_t bare_lf = 0;
    for (size_t i = 0; i < bytes.size(); ++i)
        if (bytes[i] == '\n' && (i == 0 || bytes[i - 1] != '\r')) ++bare_lf;
    CHECK(bare_lf == 0);
    std::remove(path.c_str());
}

// A save that cannot land (here the target is read-only, so the rename is
// refused) reports false, leaves the old file whole and leaves no .tmp.
TEST_CASE("save_file that cannot land keeps the old file whole and leaves no .tmp") {
    namespace fs = std::filesystem;
    const std::string path = testtemp::temp_path("settings_readonly", ".ini");
    Settings s;
    s.sp_cap = 6;
    REQUIRE(s.save_file(path));
    const fs::perms writes = fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write;
    fs::permissions(path, writes, fs::perm_options::remove);
    s.sp_cap = 3;
    const bool saved = s.save_file(path);
    fs::permissions(path, writes, fs::perm_options::add);
    CHECK_FALSE(saved);
    CHECK(Settings::load_file(path).sp_cap == 6);
    CHECK_FALSE(fs::exists(path + ".tmp"));
    std::remove(path.c_str());
}

// The lens files a result by the unit the search ran with (search_depth_mode),
// so an int the search reads as scores files beside 0, never as a third row.
// load_file lands a stray number on 0 or 1, so the 7 here is set in memory.
TEST_CASE("lens: depth_mode is the search's unit, so a stray in-memory int files as scores") {
    Settings s;
    s.depth_mode = 7;
    CHECK(s.lens().depth_mode == 0);
    CHECK(s.lens() == Settings{}.lens());
    s.depth_mode = 1;
    CHECK(s.lens().depth_mode == 1);
}

// A value other than the field's default, for each kind of field in
// kAnalysisFields: the other switch position, the next number, another
// difficulty name.
static void change_analysis_field(Settings& t, const hydra::app::AnalysisField& f) {
    struct Change {
        Settings& t;
        void operator()(bool Settings::* m) const { t.*m = !(t.*m); }
        void operator()(int Settings::* m) const { t.*m += 1; }
        void operator()(std::string Settings::* m) const {
            t.*m = t.*m == "Hard" ? "Expert" : "Hard";
        }
    };
    std::visit(Change{t}, f.member);
}

// The reset is Settings::with_analysis_defaults. The panel's own words for
// what differs (settings_at_defaults, settings_changes_summary in
// ui/library_parts.h) are the probe, so the panel and the reset agree. The
// fields come from kAnalysisFields, the one list of them.
TEST_CASE("settings reset: puts back each panel setting and keeps the rest") {
    using hydra::app::AnalysisField;
    using hydra::app::kAnalysisFields;
    using hydra::app::KeyPart;
    const Settings d;
    // Each field on its own: the summary names its panel setting, the reset
    // clears it, and it moves the key part its row names and no other.
    for (const AnalysisField& f : kAnalysisFields) {
        Settings t;
        change_analysis_field(t, f);
        CHECK(!settings_at_defaults(t));
        CHECK(t.analysis_changes() == std::vector<hydra::app::AnalysisSetting>{f.setting});
        CHECK_NOTHROW(settings_changes_summary(t));
        CHECK(settings_at_defaults(t.with_analysis_defaults()));
        CHECK((t.chartmode_key() != d.chartmode_key()) == (f.part == KeyPart::ChartMode));
        CHECK(!(t.cap_query() == d.cap_query()) == (f.part == KeyPart::Cap));
        CHECK(!(t.lens() == d.lens()) == (f.part == KeyPart::Lens));
    }

    // All at once, with the settings the panel doesn't show changed too.
    Settings s;
    for (const AnalysisField& f : kAnalysisFields) change_analysis_field(s, f);
    s.chartfolders = {"C:/Songs"};
    s.preview_volume = 75;
    s.backendlimit_enabled = true;
    s.auto_open_report = true;
    s.dm_last_user = "1234";
    const Settings r = s.with_analysis_defaults();
    CHECK(settings_changes_summary(r) == "defaults");
    // Everything a stored result is keyed by, and everything the analysis
    // reads, matches the defaults', so a keyed setting the reset misses
    // fails here.
    CHECK(r.chartmode_key() == d.chartmode_key());
    CHECK(r.cap_query() == d.cap_query());
    CHECK(r.lens() == d.lens());
    const hydra::app::AnalysisSettings ra = r.to_analysis_settings();
    const hydra::app::AnalysisSettings da = d.to_analysis_settings();
    CHECK(ra.reading() == da.reading());
    CHECK(r.chartfolders == s.chartfolders);
    CHECK(r.preview_volume == 75);
    CHECK(r.backendlimit_enabled);
    CHECK(r.auto_open_report);
    CHECK(r.dm_last_user == "1234");
}

TEST_CASE("config: resource_dir is the resource folder beside the exe") {
    const std::string dir = hydra::app::resource_dir();
    CHECK(hydra::parent_folder(dir) == hydra::app::exe_dir());
    const std::string tail = "resource";
    REQUIRE(dir.size() > tail.size());
    CHECK(hydra::ends_with(dir, tail));
}

// The pieces are the app's words for each setting, which the batch confirm,
// the settings button and hydra_batch's header all print (D114 item 9).
TEST_CASE("config: describe_settings uses the app's words") {
    Settings s;
    hydra::app::SettingsText text = hydra::app::describe_settings(s.to_analysis_settings());
    CHECK(text.depth == "4 scores");
    CHECK(text.cap == "4 bars");
    CHECK(text.timing == "10 ms");

    s.mslimit_enabled = false;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).timing == "off");

    s.depth_mode = 1;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).depth == "4 points");
    s.depth_value = 1;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).depth == "1 point");
    s.depth_value = 2000;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).depth == "2,000 points");

    s.sp_cap = 1;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).cap == "1 bar");
}

// The one guard that CMakeLists.txt's two strings reach the exe unchanged.
TEST_CASE("version: the window title and taskbar id are the build's") {
    CHECK(std::wstring(hydra::kWindowTitleW) == L"Hydra Deluxe");
    CHECK(std::wstring(hydra::kAppUserModelIDW) == L"Hydra.Hydra");
}
