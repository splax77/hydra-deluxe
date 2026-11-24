// Tests for app/config: INI round-trip through an explicit temp file,
// tolerance for malformed input, chartmode_key, and the Settings ->
// AnalysisSettings mapping (the SP cap included).

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"  // AnalysisSettings
#include "app/config.h"
#include "app/report_files.h"  // replacing_temp_path
#include "core/model.h"
#include "core/strutil.h"
#include "core/version.h"
#include "core/winstr.h"
#include "db_file_util.h"  // scalar_on_file
#include "parse/song.h"  // difficulty_name
#include "scratch_paths.h"
#include "search/pather.h"  // kDefaultDepthValue, SearchSettings
#include "source_tree.h"  // sourcetree::root
#include "store/record_store.h"  // CapQuery, Lens
#include "temp_util.h"
#include "testdata_dirs.h"  // testdata_root
#include "ui/icons.h"  // icon_files
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

// The lines save_file writes for `s`, one string per line.
std::vector<std::string> saved_lines(const Settings& s, const char* tag) {
    const std::string path = testtemp::temp_path(tag, ".ini");
    REQUIRE(s.save_file(path));
    std::vector<std::string> lines;
    {
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line)) lines.push_back(line);
    }
    std::remove(path.c_str());
    return lines;
}

// A value other than `value` that its key's line still loads: the other
// position of an on/off value, a number plus one, else another difficulty
// name (text keys take any word).
std::string other_value(const std::string& value) {
    if (const std::optional<bool> on = hydra::parse_bool(value)) return *on ? "0" : "1";
    if (const std::optional<double> n = hydra::parse_finite_number(value)) {
        return std::to_string(static_cast<long long>(*n) + 1);
    }
    for (const hydra::Difficulty d : hydra::kAllDifficulties) {
        const std::string name = hydra::difficulty_name(d);
        if (name != value) return name;
    }
    return value;
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
    CHECK(r.early_fill_window_ms == s.early_fill_window_ms);
    CHECK(r.auto_open_report == s.auto_open_report);
    CHECK(r.dm_last_user == s.dm_last_user);
}

}  // namespace

TEST_CASE("settings round-trip through an INI file") {
    Settings s;
    s.chartfolders = {"C:\\charts\\a", "C:\\charts\\b"};
    s.is_rescan = true;
    // Off its default, so a difficulty dropped on save or load shows.
    s.view_difficulty = hydra::difficulty_name(hydra::Difficulty::Hard);
    s.view_prodrums = false;
    s.view_bass2x = false;
    s.view_noteshuffle = true;
    s.depth_value = 25;
    s.depth_mode = 1;
    s.mslimit_enabled = false;
    s.mslimit_value = 42;
    s.backendlimit_enabled = true;
    // Unlike mslimit_value, so a swap of the two shows.
    s.backendlimit_value = s.mslimit_value + 1;
    s.preview_volume = 23;
    s.sp_cap = 16;
    s.legacy_fills = true;
    s.early_fill_window_ms = 37;
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
    std::remove(path.c_str());

    // Every line save_file writes is, on its own, a line load_file takes: a
    // key written under a name the loader does not read refuses as unknown.
    Settings s;
    s.dm_last_user = "123";
    s.chartfolders = {"C:\\a"};
    const std::vector<std::string> lines = saved_lines(s, "samekeys_lines");
    REQUIRE_FALSE(lines.empty());
    for (const std::string& line : lines) {
        CAPTURE(line);
        CHECK_NOTHROW(load_ini_text("samekeys_line", line + "\n"));
    }
}

// The updater's two keys (D260): they round-trip and Reset to defaults leaves
// them alone. A file holding retired keys still loads its live ones; that the
// retired keys are never written back is the retired-keys test's.
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
    CHECK_FALSE(s.lens().legacy_fills != 0);

    s.legacy_fills = true;
    CHECK(s.to_analysis_settings().legacy_fills);
    CHECK(s.lens().legacy_fills != 0);
    CHECK(s.profile().lens == s.lens());
    // The key differs from the off key in the fill rule alone.
    {
        Settings off = s;
        off.legacy_fills = false;
        CHECK(s.lens() != off.lens());
        hydra::store::Lens rest = s.lens();
        rest.legacy_fills = off.lens().legacy_fills;
        CHECK(rest == off.lens());
    }
    const hydra::app::BatchRun run = s.batch_run();
    CHECK(run.settings.legacy_fills);
    CHECK(run.profile == s.profile());
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
    CHECK(on.batch_run().profile.lens.legacy_fills != 0);
}

// The tools' one loader. The text it prints is checked through the tools
// themselves in test_cli.cpp; the flag winning over the file is the
// command-line test's above.
TEST_CASE("load_tool_settings gives the tools' settings, or a failing exit code for a refused file") {
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

    write_ini("sp_cap=abc\n");
    Settings untouched;
    untouched.sp_cap = 7;
    CHECK(hydra::app::load_tool_settings(untouched, std::nullopt) != 0);
    CHECK(hydra::app::load_tool_settings(untouched, true) != 0);
    CHECK(untouched.sp_cap == 7);
}

TEST_CASE("a missing INI yields defaults") {
    const std::string path = testtemp::temp_path("missing_never_written", ".ini");
    REQUIRE_FALSE(std::filesystem::exists(hydra::os_path(path)));
    const Settings r = Settings::load_file(path);
    const Settings d;
    check_same_settings(r, d);
    CHECK(r.skipped_update_version == d.skipped_update_version);
    CHECK(r.last_run_version == d.last_run_version);
}

TEST_CASE("early_fill_window_ms: round-trips, and a file without the line loads the default") {
    // A file that sets other keys but not this one: the default.
    CHECK(load_ini_text("efill_missing", "sp_cap=8\n").early_fill_window_ms ==
          Settings{}.early_fill_window_ms);
    for (const int ms : {0, 37, hydra::kMaxEarlyFillWindowMs}) {
        CAPTURE(ms);
        Settings s;
        s.early_fill_window_ms = ms;
        const std::string path = testtemp::temp_path("efill_roundtrip", ".ini");
        REQUIRE(s.save_file(path));
        const Settings r = Settings::load_file(path);
        std::remove(path.c_str());
        CHECK(r.early_fill_window_ms == ms);
    }
    CHECK(load_ini_text("efill_line", "early_fill_window_ms=72\n").early_fill_window_ms == 72);
    // Text that is not a whole number refuses, as every number key does.
    CHECK_THROWS_AS(load_ini_text("efill_text", "early_fill_window_ms=wide\n"),
                    hydra::app::SettingsFileError);
    CHECK_THROWS_AS(load_ini_text("efill_decimal", "early_fill_window_ms=60.5\n"),
                    hydra::app::SettingsFileError);
}

// The window travels with the settings into the search and into the result's
// key, and moves nothing else in the key.
TEST_CASE("early_fill_window_ms reaches the search and the result's key together") {
    namespace store = hydra::store;
    const Settings at_default;
    Settings at_max;
    at_max.early_fill_window_ms = hydra::kMaxEarlyFillWindowMs;
    REQUIRE(at_max.early_fill_window_ms != at_default.early_fill_window_ms);

    // The search and the result's key carry the same window, for each setting.
    for (const Settings* s : std::initializer_list<const Settings*>{&at_default, &at_max}) {
        CAPTURE(s->early_fill_window_ms);
        CHECK(s->to_analysis_settings().early_fill_window_ms ==
              static_cast<double>(s->lens().efill_window_ms));
    }
    // Two different windows search differently and file under different keys.
    CHECK(at_default.to_analysis_settings().early_fill_window_ms !=
          at_max.to_analysis_settings().early_fill_window_ms);

    // The two profiles differ in the lens's window and nowhere else.
    const store::Profile p_default = at_default.profile();
    const store::Profile p_max = at_max.profile();
    CHECK(p_default != p_max);
    CHECK(p_default.chartmode == p_max.chartmode);
    CHECK(p_default.cap == p_max.cap);
    CHECK(p_default.lens != p_max.lens);
    store::Lens rest = p_max.lens;
    rest.efill_window_ms = p_default.lens.efill_window_ms;
    CHECK(rest == p_default.lens);

    // A batch files under the window it searches with.
    const hydra::app::BatchRun run = at_max.batch_run();
    CHECK(run.settings.early_fill_window_ms ==
          static_cast<double>(run.profile.lens.efill_window_ms));
    CHECK(run.profile == p_max);
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

// Each bad line on its own stops the load with an error naming the file, the
// key and, for a key the file knows, the value. The wording is the ini_*_text builders'; this checks the
// message holds the file's path and the line's own key and value.
TEST_CASE("a bad line refuses") {
    const std::string path = testtemp::temp_path("badline", ".ini");
    // Loads a file holding `key=value`, or just `key` when `value` is empty,
    // and returns the refusal's text, or nothing when the file loads.
    auto refusal = [&path](const std::string& key,
                           const std::string& value) -> std::optional<std::string> {
        {
            std::ofstream f(path, std::ios::trunc);
            f << (value.empty() ? key : key + "=" + value) << "\n";
        }
        try {
            Settings::load_file(path);
        } catch (const hydra::app::SettingsFileError& e) {
            return std::string(e.what());
        }
        return std::nullopt;
    };
    struct Bad {
        const char* key;
        const char* value;
        bool names_value;  // an unknown key is refused by its name alone
    };
    const Bad bad_lines[] = {
        {"no_equals_sign_here", "", true},
        {"unknown_key", "whatever", false},
        {"depth_value", "not_a_number", true},
        {"view_prodrums", "yes", true},
        {"legacy_fills", "yes", true},  // a second on/off key, a word instead of 0 or 1
        {"view_difficulty", "Expret", true},
        {"sp_cap", "abc", true},
        {"sp_cap", "auto", true},
        {"preview_volume", "40.5", true},  // a decimal in a whole-number key
    };
    for (const Bad& b : bad_lines) {
        CAPTURE(b.key);
        CAPTURE(b.value);
        const std::optional<std::string> msg = refusal(b.key, b.value);
        REQUIRE(msg.has_value());
        CHECK(msg->find(path) != std::string::npos);
        CHECK(msg->find(b.key) != std::string::npos);
        if (b.names_value) {
            CHECK(msg->find(b.value) != std::string::npos);
        }
    }
    std::remove(path.c_str());
}

// D51 Q14, D270 calls 1 and 2. The range is stated once, by Settings::clamp
// (the key table every box asks); the file must land on the same number.
TEST_CASE("settings: a value outside its range loads at the nearest edge") {
    struct NumberKey {
        const char* name;
        int Settings::* field;
    };
    const NumberKey number_keys[] = {
        {"depth_value", &Settings::depth_value},
        {"depth_mode", &Settings::depth_mode},
        {"mslimit_value", &Settings::mslimit_value},
        {"backendlimit_value", &Settings::backendlimit_value},
        {"preview_volume", &Settings::preview_volume},
        {"sp_cap", &Settings::sp_cap},
        {"early_fill_window_ms", &Settings::early_fill_window_ms},
    };
    const int window = static_cast<int>(hydra::kSqueezeWindowMs);
    // Beyond every key's edges on both sides, and a value inside each range.
    const int values[] = {-window - 400, -5, -1, 0, 1, 7, 42, window + 400};
    for (const NumberKey& k : number_keys) {
        for (const int v : values) {
            CAPTURE(k.name);
            CAPTURE(v);
            const Settings r =
                load_ini_text("range", std::string(k.name) + "=" + std::to_string(v) + "\n");
            CHECK(r.*k.field == Settings::clamp(k.field, v));
        }
    }

    // The Path limit's edges are the squeeze window on either side (D41).
    CHECK(Settings::clamp(&Settings::mslimit_value, window + 400) == window);
    CHECK(Settings::clamp(&Settings::mslimit_value, -window - 400) == -window);
    // The Backend limit's top is the same window, so the shown value and the
    // window the tables use are the same number (finding 31).
    CHECK(Settings::clamp(&Settings::backendlimit_value, window + 400) == window);
    // The early-fill window's top is the timing step.
    CHECK(Settings::clamp(&Settings::early_fill_window_ms, window + 400) ==
          hydra::kMaxEarlyFillWindowMs);
    // Below the range a value lands where the range starts: a depth below 0
    // would crash the search (finding 311), and a backend limit below 0 is
    // the narrowest window the tables draw.
    CHECK(load_ini_text("range_depth", "depth_value=-1\n").depth_value ==
          load_ini_text("range_depth0", "depth_value=0\n").depth_value);
    const Settings back =
        load_ini_text("range_back", "backendlimit_enabled=1\nbackendlimit_value=-30\n");
    const Settings back0 =
        load_ini_text("range_back0", "backendlimit_enabled=1\nbackendlimit_value=0\n");
    REQUIRE(back.backend_limit().has_value());
    REQUIRE(back0.backend_limit().has_value());
    CHECK(*back.backend_limit() == *back0.backend_limit());
    CHECK(load_ini_text("range_efill", "early_fill_window_ms=-5\n").early_fill_window_ms ==
          load_ini_text("range_efill0", "early_fill_window_ms=0\n").early_fill_window_ms);
    // "An SP cap below 1 reads 1" (D270 call 1).
    CHECK(load_ini_text("range_cap0", "sp_cap=0\n").sp_cap == 1);
    CHECK(load_ini_text("range_cap3", "sp_cap=-3\n").sp_cap == 1);
    // "depth_mode=7 reads 1" (D270 call 2): points, as the search reads it.
    const Settings mode7 = load_ini_text("range_mode7", "depth_mode=7\n");
    CHECK(mode7.depth_mode == 1);
    CHECK(mode7.search_depth_mode() == hydra::DepthMode::Points);
}

TEST_CASE("Settings::volume_gain is the clamped percent as a playback gain") {
    for (const int percent : {-5, 0, 40, 100, 150}) {
        CAPTURE(percent);
        const float gain = Settings::volume_gain(percent);
        CHECK(gain == static_cast<float>(Settings::clamp(&Settings::preview_volume, percent)) /
                          100.0f);
        // Whatever the file says, the gain stays between silence and full
        // volume, so playback never amplifies past the mix.
        CHECK(gain >= 0.0f);
        CHECK(gain <= 1.0f);
    }
}

// D51 Q15.
TEST_CASE("settings: a # after a value is a comment") {
    const Settings hard = load_ini_text("hash1", "view_difficulty=Hard # practice\n");
    CHECK(hard.view_difficulty == "Hard");
    CHECK(hard.chartmode_key().rfind("Hard ", 0) == 0);
    CHECK(load_ini_text("hash2", "depth_value=7 # seven\n").depth_value == 7);
    // A folder path is free text: a # inside it is part of the folder's
    // name, so a library folder never changes on load.
    CHECK(load_ini_text("hash3", "chartfolder=C:\\Songs\\#1 Hits\n").chartfolders ==
          std::vector<std::string>{"C:\\Songs\\#1 Hits"});
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

// A scratch copy of the database `file` in testdata/store, so reading it never
// writes beside the checked-in file. The copy is deleted when it goes out of
// scope.
testtemp::ScopedFile copy_of_store_fixture(const char* file) {
    CAPTURE(file);
    const std::string source = corpus::testdata_root() + "/store/" + file;
    REQUIRE(std::filesystem::exists(hydra::os_path(source)));
    testtemp::ScopedFile copy("store_fixture", ".db");
    std::filesystem::copy_file(hydra::os_path(source), hydra::os_path(copy.path));
    return copy;
}

// The chart-mode keys real Hydra releases filed, read from the databases they
// wrote in testdata/store (its README.md names each release and run).
struct StoredKeys {
    std::vector<std::string> keys;  // each kModeCases key some file holds
    int64_t rows = 0;               // rows with a chart mode, across the files
    int64_t matched = 0;            // of those, rows whose key is a kModeCases key
};

StoredKeys stored_chartmodes() {
    struct Library {
        const char* file;
        const char* table;  // the table that release filed its keys in
    };
    const Library libraries[] = {
        {"v1.8.4-schema2.db", "results"},
        {"v1.8.4-schema2-legacy-fills.db", "results"},
        {"v2.0.0-schema3.db", "results"},
        {"v2.1.0-schema4.db", "results"},
        {"v2.12.1_library.db", "profiles"},
    };
    StoredKeys out;
    for (const Library& lib : libraries) {
        const testtemp::ScopedFile copy = copy_of_store_fixture(lib.file);
        const std::string table = lib.table;
        out.rows += hydra::test::scalar_on_file(copy.path, "SELECT COUNT(chartmode) FROM " + table);
        for (const ModeCase& c : kModeCases) {
            for (const char* key : {c.key_off, c.key_on}) {
                const int64_t n = hydra::test::scalar_on_file(
                    copy.path,
                    "SELECT COUNT(*) FROM " + table + " WHERE chartmode = '" + key + "'");
                out.matched += n;
                if (n > 0 && std::find(out.keys.begin(), out.keys.end(), key) == out.keys.end())
                    out.keys.push_back(key);
            }
        }
    }
    return out;
}

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

    // The keys above are the ones in users' stores: every key a real release
    // filed is one of them. Each library in testdata/store holds result rows
    // (its README.md), so there are keys to check.
    const StoredKeys stored = stored_chartmodes();
    REQUIRE(stored.rows > 0);
    CHECK(stored.matched == stored.rows);
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

    // Every key a real release filed comes back the same way.
    const StoredKeys stored = stored_chartmodes();
    REQUIRE_FALSE(stored.keys.empty());
    for (const std::string& key : stored.keys) {
        CAPTURE(key);
        const std::optional<Settings> mode = base.with_chartmode(key);
        REQUIRE(mode.has_value());
        CHECK(mode->sp_cap == 7);
        CHECK(mode->chartmode_key() == key);
    }
}

TEST_CASE("a default Settings reads the default ChartReading") {
    CHECK(Settings{}.reading() == hydra::ChartReading{});
}

TEST_CASE("view_difficulty matches any case and loads as the real name") {
    const std::string hard_name = hydra::difficulty_name(hydra::Difficulty::Hard);
    // The key the app files for Hard when the word is set in memory already
    // spelled right, with every other setting at its default.
    Settings in_memory;
    in_memory.view_difficulty = hard_name;
    const std::string hard_key = in_memory.chartmode_key();

    const std::string path = testtemp::temp_path("difficulty_case", ".ini");
    for (const char* word : {"hard", "HARD", "hArD"}) {
        CAPTURE(word);
        {
            std::ofstream f(path, std::ios::trunc);
            f << "view_difficulty=" << word << "\n";
        }
        Settings r = Settings::load_file(path);
        // What the app carries, and bakes into the store key, is the real name.
        CHECK(r.view_difficulty == hard_name);
        CHECK(r.difficulty() == hydra::Difficulty::Hard);
        CHECK(r.chartmode_key() == hard_key);
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
    s.view_prodrums = false;
    s.sp_cap = 32;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
    s.depth_mode = 1;
    s.depth_value = 5000;
    const store::Profile p = s.profile();
    CHECK(p.chartmode == s.chartmode_key());
    CHECK(p.cap == s.cap_query());
    CHECK(p.lens == s.lens());

    // The chartmode follows the view flags: the key reads back with Pro Drums
    // off, and differs from the default settings' key.
    const std::optional<Settings> from_key = Settings{}.with_chartmode(p.chartmode);
    REQUIRE(from_key.has_value());
    CHECK_FALSE(from_key->view_prodrums);
    CHECK(p.chartmode != Settings{}.profile().chartmode);
    // A fixed cap asks for the record analyzed at exactly that cap.
    CHECK(p.cap == store::CapQuery::at(s.sp_cap));

    // Switching the ms limit off drops its number, because the search stops
    // reading it: the lens no longer moves with the number.
    s.mslimit_enabled = false;
    const store::Lens off = s.lens();
    CHECK(off.ms_enabled == 0);
    CHECK(off.ms_value == 0);
    s.mslimit_value = 42;
    CHECK(s.lens() == off);
    CHECK(s.profile().lens == off);

    // A real v2.12.1 run filed one profile row (testdata/store/README.md
    // gives its settings: Expert Pro Drums 2x Bass, SP cap 4 bars, path limit
    // 10 ms, 4 scores, Clone Hero 1.1 fills). The same settings here build a
    // profile that matches that row column for column.
    Settings run;
    run.view_difficulty = hydra::difficulty_name(hydra::Difficulty::Expert);
    run.view_prodrums = true;
    run.view_bass2x = true;
    run.sp_cap = 4;
    run.mslimit_enabled = true;
    run.mslimit_value = 10;
    run.depth_mode = 0;
    run.depth_value = 4;
    run.legacy_fills = false;
    const store::Profile filed = run.profile();
    const testtemp::ScopedFile library = copy_of_store_fixture("v2.12.1_library.db");
    CHECK(hydra::test::scalar_on_file(
              library.path,
              "SELECT COUNT(*) FROM profiles WHERE chartmode = '" + filed.chartmode +
                  "' AND sp_cap = " + std::to_string(filed.cap.exact) +
                  " AND ms_enabled = " + std::to_string(filed.lens.ms_enabled) +
                  " AND ms_value = " + std::to_string(filed.lens.ms_value) +
                  " AND depth_mode = " + std::to_string(filed.lens.depth_mode) +
                  " AND depth_value = " + std::to_string(filed.lens.depth_value) +
                  " AND legacy_fills = " + std::to_string(filed.lens.legacy_fills)) == 1);
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
    CHECK((ch10.lens.legacy_fills != 0) == hydra::legacy_fills_for(hydra::FillDeadlineRule::Ch10));
    CHECK(ch10.lens.legacy_fills != s.profile().lens.legacy_fills);
    CHECK(ch10.chartmode == s.profile().chartmode);
    CHECK(ch10.cap == s.profile().cap);
    store::Lens rest = ch10.lens;
    rest.legacy_fills = s.profile().lens.legacy_fills;
    CHECK(rest == s.profile().lens);

    // The other way round: 1.0 fills on, asked for the 1.1 rule.
    s.legacy_fills = true;
    const store::Profile ch11 = s.profile_with_fills(hydra::FillDeadlineRule::Ch11);
    CHECK((ch11.lens.legacy_fills != 0) == hydra::legacy_fills_for(hydra::FillDeadlineRule::Ch11));
    CHECK(ch11.lens.legacy_fills != s.profile().lens.legacy_fills);
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

// An older settings file holds this line (D270 call 7, D276). The retired-key
// loop above covers its save; this test covers values the key's old parser
// would have refused, which the skip must never read.
TEST_CASE("hit_window_ms in an older file is skipped, whatever it says") {
    for (const char* value : {"85", "85.5", "banana"}) {
        CAPTURE(value);
        const Settings s = load_ini_text("hitwin_retired", std::string("hit_window_ms=") + value +
                                                              "\nsp_cap=8\n");
        // The live line beside it still reads.
        CHECK(s.sp_cap == 8);
    }
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
    // Each function undoes the other, from either side, so the two can
    // never name a different rule for one flag.
    for (bool flag : {false, true})
        CHECK(hydra::legacy_fills_for(hydra::fill_rule_for(flag)) == flag);
    for (FillDeadlineRule rule : {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11})
        CHECK(hydra::fill_rule_for(hydra::legacy_fills_for(rule)) == rule);
}

TEST_CASE("batch_run bundles one Settings' profile and search settings") {
    // The defaults, every key part moved, and a view of another difficulty
    // with the ms limit off.
    Settings hard;
    hard.view_difficulty = hydra::difficulty_name(hydra::Difficulty::Hard);
    hard.view_prodrums = false;
    hard.mslimit_enabled = false;
    hard.sp_cap = 16;
    for (const Settings& s : {Settings{}, every_key_part_changed(), hard}) {
        const std::string key = s.chartmode_key();
        CAPTURE(key);
        const hydra::app::BatchRun run = s.batch_run();
        // The record's key and the search it files read the same settings.
        CHECK(run.profile.chartmode == hydra::app::chartmode_key(run.settings.reading()));
        CHECK(run.profile.cap == hydra::store::CapQuery::at(run.settings.sp_cap));
    }
}

TEST_CASE("sp_cap: a number reads back, and cap_query asks for exactly it") {
    const std::string path = testtemp::temp_path("spcap", ".ini");
    Settings s;
    s.sp_cap = 64;
    REQUIRE(s.save_file(path));
    const Settings loaded = Settings::load_file(path);
    std::remove(path.c_str());
    CHECK(loaded.sp_cap == 64);
    CHECK(loaded.cap_query().exact == 64);
}

TEST_CASE("to_analysis_settings carries the SP cap and the ms limit, and no filter when it is off") {
    Settings s;
    s.mslimit_enabled = true;
    s.mslimit_value = 20;
    s.sp_cap = 16;
    AnalysisSettings a = s.to_analysis_settings();
    CHECK(a.ms_filter == 20.0);
    CHECK(a.sp_cap == 16);

    s.mslimit_enabled = false;
    a = s.to_analysis_settings();
    CHECK_FALSE(a.ms_filter.has_value());
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
    CHECK_FALSE(std::filesystem::exists(hydra::app::replacing_temp_path(path)));
    std::remove(path.c_str());
}

// The settings file keeps Windows CRLF line ends (D114 item 28).
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
    CHECK_FALSE(fs::exists(hydra::app::replacing_temp_path(path)));
    std::remove(path.c_str());
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
        // profile() carries every key part, so the one moved part moves it.
        CHECK(t.profile() != d.profile());
    }

    // A keyed setting left out of kAnalysisFields. The lines here come from
    // the key table through save_file, not from that list: each line, changed
    // on its own, either leaves the profile alone or is a change the panel
    // names and the reset undoes.
    int keyed_lines = 0;
    for (const std::string& line : saved_lines(d, "reset_keys")) {
        const std::optional<hydra::IniPair> pair = hydra::split_ini_line(line);
        REQUIRE(pair.has_value());
        const std::string changed = pair->key + "=" + other_value(pair->value) + "\n";
        CAPTURE(changed);
        const Settings t = load_ini_text("reset_key", changed);
        if (t.profile() == d.profile()) continue;
        ++keyed_lines;
        CHECK_FALSE(t.analysis_changes().empty());
        CHECK(t.with_analysis_defaults().profile() == d.profile());
    }
    // A default file writes the sp_cap line, and the SP cap is part of the
    // profile (Settings::cap_query), so at least that line moves it.
    CHECK(keyed_lines > 0);

    // All at once, with the settings the panel doesn't show changed too.
    Settings s;
    for (const AnalysisField& f : kAnalysisFields) change_analysis_field(s, f);
    s.chartfolders = {"C:/Songs"};
    s.preview_volume = 75;
    s.backendlimit_enabled = true;
    s.auto_open_report = true;
    s.dm_last_user = "1234";
    const Settings r = s.with_analysis_defaults();
    // The summary after the reset is the summary a default Settings gets.
    CHECK(settings_at_defaults(r));
    CHECK(settings_changes_summary(r) == settings_changes_summary(d));
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
    CHECK(r.preview_volume == s.preview_volume);
    CHECK(r.backendlimit_enabled == s.backendlimit_enabled);
    CHECK(r.auto_open_report == s.auto_open_report);
    CHECK(r.dm_last_user == s.dm_last_user);
}

// The settings button and the Reset tooltip name the early-fill window by
// kEarlyFillWindowName and describe_settings' value, like Path limit.
TEST_CASE("settings summary: the early-fill window, off its default") {
    using hydra::ui::detail::reset_settings_tooltip;
    using hydra::ui::detail::settings_button_label;
    using hydra::ui::detail::kEarlyFillWindowName;
    // The phrase the button and tooltip use for `s`'s window: the setting's
    // name and describe_settings' value for it.
    auto phrase = [](const Settings& s) {
        return std::string(kEarlyFillWindowName) + " " +
               hydra::app::describe_settings(s.to_analysis_settings()).early_fill;
    };
    Settings s;
    CHECK(hydra::app::describe_settings(s.to_analysis_settings()).early_fill ==
          hydra::format_ms_whole(s.early_fill_window_ms));
    // At its default the window is not named on the button.
    CHECK(settings_button_label(s, false).find(kEarlyFillWindowName) == std::string::npos);
    for (const int ms : {hydra::kMaxEarlyFillWindowMs, 0}) {
        CAPTURE(ms);
        s.early_fill_window_ms = ms;
        REQUIRE(s.early_fill_window_ms != Settings{}.early_fill_window_ms);
        CHECK(hydra::app::describe_settings(s.to_analysis_settings()).early_fill ==
              hydra::format_ms_whole(ms));
        CHECK(settings_button_label(s, false).find(phrase(s)) != std::string::npos);
        CHECK(reset_settings_tooltip(s, false).find(phrase(s)) != std::string::npos);
    }
    CHECK(settings_at_defaults(s.with_analysis_defaults()));
    CHECK(s.with_analysis_defaults().early_fill_window_ms == Settings{}.early_fill_window_ms);
}

// load_icons reads each file icon_files() names from resource_dir(). The
// build copies resource/ beside each exe that reads it (CMakeLists.txt, the
// post-build steps of Hydra, HydraPathBuilder and hydra_tests), so the folder
// resource_dir() gives this exe must hold every one of them.
TEST_CASE("config: resource_dir is the resource folder beside the exe") {
    namespace fs = std::filesystem;
    const std::string dir = hydra::app::resource_dir();
    CAPTURE(dir);
    REQUIRE(fs::is_directory(hydra::os_path(dir)));
    REQUIRE_FALSE(hydra::ui::icon_files().empty());
    for (const hydra::ui::IconFile& icon : hydra::ui::icon_files()) {
        CAPTURE(icon.file);
        CHECK(fs::is_regular_file(hydra::os_path(hydra::join_folder(dir, icon.file))));
    }
}

// The pieces are the app's words for each setting, which the batch confirm,
// the settings button and hydra_batch's header all print (D114 item 9).
TEST_CASE("config: describe_settings uses the app's words") {
    // Each value is set here, so the words below follow from the test's own
    // settings and not from the fields' defaults.
    Settings s;
    s.depth_mode = 0;
    s.depth_value = 4;
    s.sp_cap = 4;
    s.mslimit_enabled = true;
    s.mslimit_value = 10;
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

// The one guard that CMakeLists.txt's two strings reach the exe unchanged:
// each is read from its set(...) line in the repo's CMakeLists.txt.
TEST_CASE("version: the window title and taskbar id are the build's") {
    const std::string cmake =
        hydra::read_file_text((sourcetree::root() / "CMakeLists.txt").u8string());
    // The quoted value of `set(<name> "...")`, or empty when the line is missing.
    auto cmake_string = [&cmake](const std::string& name) -> std::string {
        const std::string head = "set(" + name + " \"";
        const size_t at = cmake.find(head);
        if (at == std::string::npos) return "";
        const size_t start = at + head.size();
        const size_t end = cmake.find('"', start);
        if (end == std::string::npos) return "";
        return cmake.substr(start, end - start);
    };
    const std::string title = cmake_string("HYDRA_APP_NAME");
    const std::string model_id = cmake_string("HYDRA_APP_USER_MODEL_ID");
    REQUIRE_FALSE(title.empty());
    REQUIRE_FALSE(model_id.empty());
    CHECK(std::wstring(hydra::kWindowTitleW) == hydra::utf8_to_wide(title));
    CHECK(std::wstring(hydra::kAppUserModelIDW) == hydra::utf8_to_wide(model_id));
}
