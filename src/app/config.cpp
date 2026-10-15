#include "app/config.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <variant>

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
    return join_folder(exe_dir(), "hydra_settings.ini");
}

std::unique_ptr<store::RecordStore> open_store(const std::string& db,
                                               core::RulesStamp rules) {
    return std::make_unique<store::RecordStore>(db, rules);
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

// Where a number outside its range lands.
enum class Outside {
    // The nearest edge of the range (D51 Q14).
    NearestEdge,
    // The setting's default: the range has no edge to land on.
    Default,
    // From the file, a value below the floor reads as the default; anywhere
    // else (clamp), it lands on the floor. sp_cap only: 0, junk and 1.8.4's
    // "auto" have always read as Clone Hero's 4 (ui-redesign decision 6).
    DefaultFromFile,
};

struct Key {
    const char* name;
    std::variant<bool Settings::*, int Settings::*, double Settings::*, std::string Settings::*,
                 std::vector<std::string> Settings::*>
        member;
    // Number keys: the allowed range, both edges allowed, and where a value
    // outside it lands. A double holds every int edge exactly.
    double lo = 0;
    double hi = 0;
    Outside outside = Outside::NearestEdge;
    // Text keys: whether a # is part of the value (a folder path, a user id)
    // rather than the start of a comment (D51 Q15).
    bool free_text = false;
};

constexpr int kNoCeiling = std::numeric_limits<int>::max();
// The Path and Backend limits' edge: the engine's squeeze window (D41).
constexpr int kWindowMs = static_cast<int>(kSqueezeWindowMs);

Key on_off(const char* name, bool Settings::* m) { return Key{name, m}; }
Key number(const char* name, int Settings::* m, int lo, int hi,
           Outside outside = Outside::NearestEdge) {
    return Key{name, m, static_cast<double>(lo), static_cast<double>(hi), outside};
}
Key number(const char* name, double Settings::* m, double lo, double hi,
           Outside outside = Outside::NearestEdge) {
    return Key{name, m, lo, hi, outside};
}
Key word(const char* name, std::string Settings::* m) { return Key{name, m}; }
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
        word("view_difficulty", &Settings::view_difficulty),
        on_off("view_prodrums", &Settings::view_prodrums),
        on_off("view_bass2x", &Settings::view_bass2x),
        on_off("view_noteshuffle", &Settings::view_noteshuffle),
        // Below 0 the search crashes (finding 311).
        number("depth_value", &Settings::depth_value, 0, kNoCeiling),
        // A switch: 0 is scores, 1 is points, and anything else is scores,
        // as the search has always read it (D51 addendum).
        number("depth_mode", &Settings::depth_mode, 0, 1, Outside::Default),
        on_off("mslimit_enabled", &Settings::mslimit_enabled),
        number("mslimit_value", &Settings::mslimit_value, -kWindowMs, kWindowMs),
        on_off("backendlimit_enabled", &Settings::backendlimit_enabled),
        number("backendlimit_value", &Settings::backendlimit_value, 0, kWindowMs),
        // Below 1 (including 0) has no edge to land on, so it reads the
        // default. The floor is the 1 ms it was while the setting was whole.
        number("hit_window_ms", &Settings::hit_window_ms, 1.0, kNoCeiling, Outside::Default),
        number("preview_volume", &Settings::preview_volume, 0, 100),
        // The smallest cap is 1 bar: it can never activate SP, but it stays
        // allowed as a what-if (D51 Q16).
        number("sp_cap", &Settings::sp_cap, 1, kNoCeiling, Outside::DefaultFromFile),
        on_off("legacy_fills", &Settings::legacy_fills),
        on_off("auto_open_report", &Settings::auto_open_report),
        free_text("dm_last_user", &Settings::dm_last_user),
        free_text_list("chartfolder", &Settings::chartfolders),
    };
    return k;
}

const Settings& defaults() {
    static const Settings d;
    return d;
}

// `value` pulled into k's range; T is the key's number type (int or double).
// `from_file` is load_file's call.
template <typename T>
T pull_into_range(const Key& k, T value, bool from_file) {
    const T lo = static_cast<T>(k.lo);
    const T hi = static_cast<T>(k.hi);
    if (value >= lo && value <= hi) return value;
    const bool to_default =
        k.outside == Outside::Default ||
        (k.outside == Outside::DefaultFromFile && from_file && value < lo);
    if (to_default) return defaults().*std::get<T Settings::*>(k.member);
    return std::clamp(value, lo, hi);
}

// Sets k's field of `s` from one line of the file.
void read_value(const Key& k, const IniPair& line, Settings& s) {
    if (auto b = std::get_if<bool Settings::*>(&k.member)) {
        // Anything but 0 or 1 keeps the setting as it was.
        if (const std::optional<bool> on = parse_bool(line.value)) s.**b = *on;
    } else if (auto n = std::get_if<int Settings::*>(&k.member)) {
        // Junk reads as 0, as atoi always made it, then takes the range.
        s.**n = pull_into_range(k, std::atoi(line.value.c_str()), true);
    } else if (auto d = std::get_if<double Settings::*>(&k.member)) {
        // A decimal reads through parse_finite_number. Junk reads as 0, as
        // for a whole number, then takes the range.
        s.**d = pull_into_range(k, parse_finite_number(line.value).value_or(0.0), true);
    } else if (auto t = std::get_if<std::string Settings::*>(&k.member)) {
        s.**t = k.free_text ? line.whole_value : line.value;
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
        // A blank, comment-only or "="-less line is skipped.
        const std::optional<IniPair> kv = split_ini_line(line);
        if (!kv) continue;
        // A key no setting has is skipped: a retired one
        // (kRetiredSettingsKeys) or any other.
        for (const Key& k : keys())
            if (kv->key == k.name) read_value(k, *kv, s);
    }
    // Normalize the difficulty word: whatever was in the file, what the app
    // carries is one of the four real names.
    s.view_difficulty = difficulty_name(s.difficulty());
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
        } else if (auto d = std::get_if<double Settings::*>(&k.member)) {
            // The stream's own form: a whole number writes as one (85), so a
            // file written before the decimal keeps its bytes.
            f << k.name << "=" << this->**d << "\n";
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
        if (n && *n == field) return pull_into_range(k, value, false);
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

store::RecordKey Settings::record_key(const std::string& hyhash) const {
    return store::RecordKey{hyhash, profile()};
}

BatchRun Settings::batch_run() const {
    return BatchRun{chartmode_key(), lens(), to_analysis_settings()};
}

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
