#include "app/config.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <variant>

#include "app/ini_errors.h"
#include "app/report_files.h"  // write_file_replacing

#include "core/model.h"
#include "core/strutil.h"
#include "core/winstr.h"

namespace hydra::app {

std::string exe_dir() {
    const std::string dir = parent_folder(exe_path_utf8());
    // A module path with no folder (Windows never hands one over) keeps the
    // files beside the exe relative to the working folder, as they always were.
    return dir.empty() ? std::string(".") : dir;
}

namespace {

std::optional<bool> g_installed_for_test;

// The folder the installer registered under Hydra's frozen AppId
// (installer/hydra.iss), empty when Hydra was never installed. The installer
// runs in 64-bit mode, so its uninstall key is in the 64-bit view.
std::wstring registered_install_location() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
                      L"{638FCDD7-88E0-438D-9C52-388C6C9CF4E3}_is1",
                      0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
        return {};
    const wchar_t* const name = L"InstallLocation";
    DWORD bytes = 0;
    std::wstring value;
    if (RegGetValueW(key, nullptr, name, RRF_RT_REG_SZ, nullptr, nullptr, &bytes) ==
            ERROR_SUCCESS &&
        bytes >= sizeof(wchar_t)) {
        value.resize(bytes / sizeof(wchar_t));
        if (RegGetValueW(key, nullptr, name, RRF_RT_REG_SZ, nullptr, value.data(), &bytes) ==
            ERROR_SUCCESS)
            value.resize(wcsnlen(value.c_str(), value.size()));
        else
            value.clear();
    }
    RegCloseKey(key);
    return value;
}

}  // namespace

bool installed_by_setup() {
    if (g_installed_for_test) return *g_installed_for_test;
    // The exe's folder and the registered install folder can't change while
    // Hydra runs, so one read of the registry is enough.
    static const bool answer = [] {
        const std::wstring registered = registered_install_location();
        if (registered.empty()) return false;
        const std::wstring here = win32_path(trim_trailing_separators(exe_dir()));
        const std::wstring there = win32_path(trim_trailing_separators(wide_to_utf8(registered)));
        if (here.empty() || there.empty()) return false;
        return CompareStringOrdinal(here.c_str(), static_cast<int>(here.size()), there.c_str(),
                                    static_cast<int>(there.size()), TRUE) == CSTR_EQUAL;
    }();
    return answer;
}

void set_installed_for_test(std::optional<bool> installed) { g_installed_for_test = installed; }

std::string resource_dir() { return join_folder(exe_dir(), "resource"); }

namespace {
PathOverrides g_overrides;
}

void set_path_overrides(PathOverrides overrides) { g_overrides = std::move(overrides); }
const PathOverrides& path_overrides() { return g_overrides; }

std::string db_path() {
    if (!g_overrides.db_path.empty()) return g_overrides.db_path;
    return join_folder(exe_dir(), "hydra.db");
}

std::string ini_path() {
    if (!g_overrides.ini_path.empty()) return g_overrides.ini_path;
    return join_folder(exe_dir(), std::string(kSettingsFileName));
}

std::unique_ptr<store::RecordStore> open_store(const std::string& db,
                                               core::RulesStamp rules) {
    // The command-line tools never rename a library file; the app does.
    return std::make_unique<store::RecordStore>(db, store::OldFilePolicy::Refuse, rules);
}

std::string asset_dir() {
    if (!g_overrides.asset_dir.empty()) return g_overrides.asset_dir;
    return join_folder(join_folder(exe_dir(), "assets"), "preview");
}

Settings Settings::load() { return load_file(ini_path()); }

Settings Settings::load_for_command_line(bool legacy_fills_flag) {
    Settings s = load();
    s.legacy_fills = legacy_fills_flag;
    return s;
}

namespace {

// ---- the key table: every hydra_settings.ini key, once ----------------------

struct Key {
    const char* name;
    std::variant<bool Settings::*, int Settings::*, std::string Settings::*,
                 std::vector<std::string> Settings::*>
        member;
    // Number keys: the allowed range, both edges allowed. A value outside it
    // lands on the nearest edge (D51 Q14, D270 calls 1 and 2).
    int lo = 0;
    int hi = 0;
    // Text keys: whether a # is part of the value (a folder path, a user id)
    // rather than the start of a comment (D51 Q15).
    bool free_text = false;
    // Text keys: whether the value is a difficulty name (difficulty_from_name).
    bool difficulty = false;
};

constexpr int kNoCeiling = std::numeric_limits<int>::max();
// The Path and Backend limits' edge: the engine's squeeze window (D41).
constexpr int kWindowMs = static_cast<int>(kSqueezeWindowMs);

Key on_off(const char* name, bool Settings::* m) { return Key{name, m}; }
Key number(const char* name, int Settings::* m, int lo, int hi) { return Key{name, m, lo, hi}; }
Key word(const char* name, std::string Settings::* m) { return Key{name, m}; }
Key difficulty_word(const char* name, std::string Settings::* m) {
    Key k{name, m};
    k.difficulty = true;
    return k;
}
Key free_text(const char* name, std::string Settings::* m) {
    Key k{name, m};
    k.free_text = true;
    return k;
}
Key free_text_list(const char* name, std::vector<std::string> Settings::* m) {
    Key k{name, m};
    k.free_text = true;
    return k;
}

// In the order save_file writes them. Each key's default is its field's
// initializer in config.h; the boxes and clamp take their ranges from here.
const std::vector<Key>& keys() {
    static const std::vector<Key> k = {
        on_off("is_rescan", &Settings::is_rescan),
        difficulty_word("view_difficulty", &Settings::view_difficulty),
        on_off("view_prodrums", &Settings::view_prodrums),
        on_off("view_bass2x", &Settings::view_bass2x),
        on_off("view_noteshuffle", &Settings::view_noteshuffle),
        // Below 0 the search crashes (finding 311).
        number("depth_value", &Settings::depth_value, 0, kNoCeiling),
        // A switch: 0 is scores, 1 is points.
        number("depth_mode", &Settings::depth_mode, 0, 1),
        on_off("mslimit_enabled", &Settings::mslimit_enabled),
        number("mslimit_value", &Settings::mslimit_value, -kWindowMs, kWindowMs),
        on_off("backendlimit_enabled", &Settings::backendlimit_enabled),
        number("backendlimit_value", &Settings::backendlimit_value, 0, kWindowMs),
        number("preview_volume", &Settings::preview_volume, 0, 100),
        // The smallest cap is 1 bar: it can never activate SP, but it stays
        // allowed as a what-if (D51 Q16).
        number("sp_cap", &Settings::sp_cap, 1, kNoCeiling),
        on_off("legacy_fills", &Settings::legacy_fills),
        on_off("auto_open_report", &Settings::auto_open_report),
        free_text("dm_last_user", &Settings::dm_last_user),
        word("skipped_update_version", &Settings::skipped_update_version),
        word("last_run_version", &Settings::last_run_version),
        free_text_list("chartfolder", &Settings::chartfolders),
    };
    return k;
}

// `value` on k's range, at its nearest edge when outside it.
int pull_into_range(const Key& k, int value) { return std::clamp(value, k.lo, k.hi); }

// The four difficulty names, as a refused view_difficulty line lists them:
// "Expert, Hard, Medium or Easy".
std::string difficulty_names() {
    std::string out;
    const size_t n = std::size(kAllDifficulties);
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) out += i + 1 == n ? " or " : ", ";
        out += difficulty_name(kAllDifficulties[i]);
    }
    return out;
}

// Sets k's field of `s` from one line of the file at `path`, or throws
// SettingsFileError naming the line.
void read_value(const std::string& path, const Key& k, const IniPair& line, Settings& s) {
    auto refuse = [&](std::string_view want) {
        throw SettingsFileError(ini_bad_value_text(path, line.key, line.value, want));
    };
    if (auto b = std::get_if<bool Settings::*>(&k.member)) {
        const std::optional<bool> on = parse_bool(line.value);
        if (!on) refuse("0 or 1");
        s.**b = *on;
    } else if (auto n = std::get_if<int Settings::*>(&k.member)) {
        const std::optional<int> whole = parse_whole_number(line.value);
        if (!whole) refuse(kWantWholeNumber);
        s.**n = pull_into_range(k, *whole);
    } else if (auto t = std::get_if<std::string Settings::*>(&k.member)) {
        if (k.difficulty) {
            // What the app carries is one of the real names, whatever the
            // file's case.
            const std::optional<Difficulty> d = difficulty_from_name(line.value);
            if (!d) refuse(difficulty_names());
            s.**t = difficulty_name(*d);
        } else {
            s.**t = k.free_text ? line.whole_value : line.value;
        }
    } else {
        auto list = std::get<std::vector<std::string> Settings::*>(k.member);
        (s.*list).push_back(k.free_text ? line.whole_value : line.value);
    }
}

}  // namespace

Settings Settings::load_file(const std::string& path) {
    Settings s;
    std::ifstream f(os_path(path));
    if (!f) return s;  // defaults

    std::string line;
    while (std::getline(f, line)) {
        const std::optional<IniPair> kv = split_ini_line(line);
        if (!kv) {
            // A blank or comment-only line is fine; any other line needs an =.
            const std::string_view text = ini_line_text(line);
            if (text.empty()) continue;
            throw SettingsFileError(ini_not_key_value_text(path, text));
        }
        const Key* key = nullptr;
        for (const Key& k : keys())
            if (kv->key == k.name) key = &k;
        // A live key wins, so a name brought back is never skipped.
        if (!key && is_retired_key(kRetiredSettingsKeys, kv->key)) continue;
        if (!key) throw SettingsFileError(ini_unknown_key_text(path, kv->key));
        read_value(path, *key, *kv, s);
    }
    return s;
}

bool Settings::save() const { return save_file(ini_path()); }

bool Settings::save_file(const std::string& path) const {
    // Built in memory, then saved through write_file_replacing (D114 item 20),
    // so a stopped write leaves the old file whole. The file keeps Windows
    // CRLF line ends (D114 item 28).
    std::ostringstream f;
    for (const Key& k : keys()) {
        if (auto b = std::get_if<bool Settings::*>(&k.member)) {
            f << k.name << "=" << (this->**b ? 1 : 0) << "\n";
        } else if (auto n = std::get_if<int Settings::*>(&k.member)) {
            f << k.name << "=" << this->**n << "\n";
        } else if (auto t = std::get_if<std::string Settings::*>(&k.member)) {
            // An empty text is left out, so it loads as its default.
            if (!(this->**t).empty()) f << k.name << "=" << this->**t << "\n";
        } else {
            for (const std::string& item : this->*std::get<std::vector<std::string> Settings::*>(k.member))
                f << k.name << "=" << item << "\n";
        }
    }
    return write_file_replacing(std::filesystem::u8path(path), f.str(), LineEnds::Crlf);
}

int Settings::clamp(int Settings::* field, int value) {
    for (const Key& k : keys()) {
        auto n = std::get_if<int Settings::*>(&k.member);
        if (n && *n == field) return pull_into_range(k, value);
    }
    return value;  // not a number setting: every int field is in the table
}

DepthMode Settings::search_depth_mode() const {
    return depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;
}

float Settings::volume_gain(int percent) {
    return static_cast<float>(clamp(&Settings::preview_volume, percent)) / 100.0f;
}

Difficulty Settings::difficulty() const {
    return difficulty_from_name(view_difficulty).value_or(Difficulty::Expert);
}

// 2x Bass applies at every difficulty, as Clone Hero's Double Kick modifier
// does (D20; its only gate, 0x20D32B7, has no difficulty check). Each
// difficulty has its own 2x kicks; the box decides whether they are read.
bool Settings::effective_bass2x() const { return view_bass2x; }

std::string chartmode_key(const ChartReading& reading) {
    std::string prodrums = reading.prodrums ? "Pro Drums" : "Drums";
    // The four Expert keys are byte-for-byte the ones already in the store.
    // Below Expert a key ends "2x Bass" with the box on since D20; results
    // stored before that are re-analyzed by step 1's results-stamp bump.
    std::string bass = reading.bass2x ? "2x Bass" : "1x Bass";
    std::string key = std::string(difficulty_name(reading.difficulty)) + " " + prodrums + ", " + bass;
    // Only an ending, so a key stored before Note Shuffle existed still names
    // the unshuffled result (D104).
    if (reading.noteshuffle) key += ", Note Shuffle";
    return key;
}

ChartReading Settings::reading() const {
    // The difficulty comes from difficulty(), so an uncleaned view_difficulty
    // ("easy", "Legendary") still names a real difficulty (finding 134).
    return ChartReading{difficulty(), view_prodrums, effective_bass2x(), view_noteshuffle};
}

Settings Settings::with_reading(const ChartReading& r) const {
    Settings out = *this;
    out.view_difficulty = difficulty_name(r.difficulty);
    out.view_prodrums = r.prodrums;
    out.view_bass2x = r.bass2x;
    out.view_noteshuffle = r.noteshuffle;
    return out;
}

std::string Settings::chartmode_key() const { return app::chartmode_key(reading()); }

Settings Settings::with_analysis_defaults() const {
    const Settings d;
    Settings out = *this;
    for (const AnalysisField& f : kAnalysisFields)
        std::visit([&](auto member) { out.*member = d.*member; }, f.member);
    return out;
}

std::vector<AnalysisSetting> Settings::analysis_changes() const {
    const Settings d;
    std::vector<AnalysisSetting> out;
    for (const AnalysisField& f : kAnalysisFields) {
        const bool differs =
            std::visit([&](auto member) { return this->*member != d.*member; }, f.member);
        if (differs && std::find(out.begin(), out.end(), f.setting) == out.end())
            out.push_back(f.setting);
    }
    return out;
}

std::optional<Settings> Settings::with_chartmode(const std::string& chartmode) const {
    for (Difficulty d : kAllDifficulties) {
        for (bool prodrums : {true, false}) {
            for (bool bass2x : {true, false}) {
                for (bool noteshuffle : {false, true}) {
                    // Spelled by app::chartmode_key, as every other reader
                    // spells it, and written back through with_reading.
                    const ChartReading r{d, prodrums, bass2x, noteshuffle};
                    if (app::chartmode_key(r) == chartmode) return with_reading(r);
                }
            }
        }
    }
    return std::nullopt;
}

Settings Settings::with_known_chartmode(const std::string& chartmode) const {
    std::optional<Settings> mode = with_chartmode(chartmode);
    if (!mode)
        throw std::runtime_error("No analysis settings give the chart mode \"" + chartmode + "\".");
    return std::move(*mode);
}

AnalysisSettings Settings::to_analysis_settings() const {
    AnalysisSettings s;
    s.reading() = reading();
    s.depth_mode = search_depth_mode();
    s.depth_value = depth_value;
    s.ms_filter = mslimit_enabled ? std::optional<double>(mslimit_value) : std::nullopt;
    s.sp_cap = sp_cap;
    s.legacy_fills = legacy_fills;
    s.rules = rules;
    return s;
}

std::optional<double> Settings::backend_limit() const {
    // No sign flip: load_file and the box keep the value at 0 or above
    // (finding 31), so the shown value is the window the tables use.
    return backendlimit_enabled ? std::optional<double>(backendlimit_value) : std::nullopt;
}

store::CapQuery Settings::cap_query() const {
    return store::CapQuery::at(sp_cap);
}

store::Lens Settings::lens() const {
    return store::Lens::from(
        mslimit_enabled ? std::optional<int>(mslimit_value) : std::nullopt,
        // The unit the search runs with (search_depth_mode), not the raw int,
        // so the lens stays canonical (concept audit M14).
        search_depth_mode() == DepthMode::Points ? 1 : 0, depth_value, legacy_fills);
}

store::Profile Settings::profile() const {
    return {chartmode_key(), cap_query(), lens()};
}

// The two variants change one setting on a copy and ask profile() again, so
// cap_query() and lens() (and through it Lens::from) stay the only encoders.
store::Profile Settings::profile_at_clone_hero_cap() const {
    Settings at_cap = *this;
    at_cap.sp_cap = kCloneHeroSpCap;
    return at_cap.profile();
}

store::Profile Settings::profile_with_fills(FillDeadlineRule rule) const {
    Settings with_rule = *this;
    with_rule.legacy_fills = legacy_fills_for(rule);
    return with_rule.profile();
}

BatchRun Settings::batch_run() const { return BatchRun{profile(), to_analysis_settings()}; }

std::string bars_text(int bars) { return counted(bars, "bar", "bars"); }

std::string score_range_text(DepthMode depth_mode, int depth_value) {
    return depth_mode == DepthMode::Points ? counted(depth_value, "point", "points")
                                           : counted(depth_value, "score", "scores");
}

SettingsText describe_settings(const AnalysisSettings& settings) {
    SettingsText text;
    text.depth = score_range_text(settings.depth_mode, settings.depth_value);
    text.cap = bars_text(settings.sp_cap);
    text.timing = settings.ms_filter ? format_ms_whole(*settings.ms_filter) : std::string("off");
    return text;
}

}  // namespace hydra::app
