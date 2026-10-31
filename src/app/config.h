// Persisted user settings + user file locations, shared by the GUI
// shells and the command-line tools. Split out of ui/app_state so a console
// exe can read the same INI the app writes (hydra_batch.py's contract:
// "results match what the app would produce for the same songs").
//

#ifndef HYDRA_APP_CONFIG_H
#define HYDRA_APP_CONFIG_H

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "app/analysis.h"
#include "core/error_kind.h"
#include "core/rules.h"
#include "search/graph.h"  // FillDeadlineRule
#include "store/record_store.h"

namespace hydra::app {

// Directory containing the running executable (UTF-8): the exe path's
// parent_folder, or "." in the impossible case of a path with no folder.
// User files live here, mirroring hymisc.ROOTPATH's app-relative layout.
std::string exe_dir();

// Whether the running exe is the one Hydra's installer put in place: the one
// rule for "is this the installed exe" (decision 14 of
// docs/superpowers/plans/2026-10-22-auto-updater.md). Only the installed exe
// checks for updates, so a dev build, a worktree build, the test runner or a
// copy of the exe elsewhere never does.
bool installed_by_setup();

// Test seam: true or false makes installed_by_setup answer that; empty puts
// the registry check back. Tests set it through ScopedHook.
void set_installed_for_test(std::optional<bool> installed);

// The app's fonts and icons: the "resource" folder beside the exe (the build
// copies resource/ there; see CMakeLists.txt).
std::string resource_dir();

// The settings file's name, beside the exe.
inline constexpr std::string_view kSettingsFileName = "hydra_settings.ini";

// The user's database and settings file, next to the exe ("hydra.db" /
// kSettingsFileName).
std::string db_path();
std::string ini_path();

// A line of hydra_settings.ini that can't be read. Its text comes from
// app/ini_errors.h, the wording hydra_rules.ini's errors share; its kind is
// SettingsFile.
struct SettingsFileError : KindedError {
    explicit SettingsFileError(const std::string& what)
        : KindedError(ErrorKind::SettingsFile, what) {}
};

// The command-line switch that runs a tool under Clone Hero 1.0's fill rule.
// Every tool that takes it compares against this one spelling.
inline constexpr std::string_view kLegacyFillsFlag = "--legacy-fills";

// A key a released Hydra stopped reading. A file that still sets one keeps
// loading, its value is ignored, and the next save leaves it out (D270 call
// 7). Each file reader keeps its own list:
// kRetiredSettingsKeys below for hydra_settings.ini, and kRetiredRulesKeys in
// app/rules_file.cpp for hydra_rules.ini.
struct RetiredKey {
    std::string_view name;
    std::string_view retired_in;  // the first release that no longer read it
};

// Whether `key` is on a reader's retired list.
template <size_t N>
constexpr bool is_retired_key(const RetiredKey (&list)[N], std::string_view key) {
    for (const RetiredKey& r : list)
        if (r.name == key) return true;
    return false;
}

// hydra_settings.ini's retired keys: Settings::load_file skips them, and
// refuses any other key no setting has.
inline constexpr RetiredKey kRetiredSettingsKeys[] = {
    {"sp_cap_enabled", "1.6.0"},  // replaced by sp_cap
    {"sp_cap_value", "1.6.0"},    // replaced by sp_cap
    {"check_updates", "2.12.0"},      // every installed start checks (D260)
    {"last_update_check", "2.12.0"},  // every installed start checks (D260)
    {"hit_window_ms", "2.14.0"},      // one fixed hit window, kHitWindowMs (D270 call 8)
};

// Opens the store at `db`. `rules` gates Ready: a row analyzed
// under other rules reads Stale.
std::unique_ptr<store::RecordStore> open_store(
    const std::string& db, core::RulesStamp rules = core::default_stamp());

// The Preview's authored highway art (exe_dir()\assets\preview by default).
std::string asset_dir();

// Process-wide overrides for the three locations above. Empty = default.
// Set once at startup by harnesses that must run the real app code on scratch
// files (the GUI test runner, docs/agents/ui-testing.md); the app never sets
// them.
struct PathOverrides {
    std::string db_path;
    std::string ini_path;
    std::string asset_dir;
    std::string rules_path;  // hydra_rules.ini (app/rules_file.h)
};
void set_path_overrides(PathOverrides overrides);
const PathOverrides& path_overrides();

// ChartReading (the four choices that pick a chart's notes) lives in
// parse/chart_reading.h, beside the parser's loaders that take it.
// Settings::reading() and Settings::with_reading read it off and write it onto
// the settings.

// The stored chart mode a reading is filed under: "Expert Pro Drums, 2x
// Bass", mirroring HyAppUserSettings.chartmode_key. With Note Shuffle on it
// gains an ending (D104); with it off the key is the one Hydra has always
// stored.
std::string chartmode_key(const ChartReading& reading);

// The settings the Analysis settings panel shows, in the panel's order.
// kAnalysisFields says which Settings fields make up each one.
enum class AnalysisSetting {
    Difficulty,
    ProDrums,
    Bass2x,
    NoteShuffle,
    SpCap,
    Fills,
    ScoreRange,
    PathLimit
};

// The part of a stored result's key an analysis field is filed under:
// Settings::chartmode_key(), cap_query() or lens().
enum class KeyPart { ChartMode, Cap, Lens };

// Persisted user settings, mirroring HyAppUserSettings' fields. Loaded once at
// startup and written back to disk on every change (matching Python's
// HyAppUserSetting descriptor, which saves on every __set__).
struct Settings {
    std::vector<std::string> chartfolders;
    bool is_rescan = false;

    std::string view_difficulty = "Expert";
    bool view_prodrums = true;
    bool view_bass2x = true;
    // Score the chart as Clone Hero 1.1's Note Shuffle modifier rearranges
    // it (D104). Off by default. Part of a result's key through
    // chartmode_key(), like Pro Drums and 2x Bass. INI line:
    // view_noteshuffle=0.
    bool view_noteshuffle = false;

    int depth_value = kDefaultDepthValue;
    // 0 = scores, 1 = points. Stays an int: it is what the INI stores and what
    // the details view's Combo binds to; search_depth_mode() maps it to
    // search/engine.h's DepthMode.
    int depth_mode = 0;

    bool mslimit_enabled = true;
    int mslimit_value = 10;

    // The backend tables' display window: when enabled, only backend rows
    // within +/- this many ms are shown (squeezed-out rows always show).
    // Display-layer only: it never reaches the search, so changing it never
    // invalidates stored records.
    bool backendlimit_enabled = false;
    int backendlimit_value = 50;

    // Preview playback volume in percent. A summed multi-stem mix at 100 % is
    // loud and clips, so the default sits well below it.
    int preview_volume = 40;

    // The Star Power meter ceiling in bars, at least 1. 4 is Clone Hero's
    // rule (the default); other values are what-ifs. INI line: sp_cap=4.
    // The keys it replaced are in kRetiredSettingsKeys.
    int sp_cap = kCloneHeroSpCap;

    // Spawn fills by Clone Hero 1.0's deadline instead of 1.1's (search/graph.h
    // FillDeadlineRule). Off by default: 1.1 is the game people play now. Part
    // of a result's key, like the SP cap. INI line: legacy_fills=0.
    // A tool that takes its own kLegacyFillsFlag ignores it: see
    // load_for_command_line. (The path report follows it, docs/adr/0010.)
    bool legacy_fills = false;

    // Open the batch's path report window as soon as Hydra builds it. It
    // covers only the path report, not the leaderboard comparison (D103 item
    // 13): AppState::collect_path_report is its one reader. Off by default
    // (the finished strip offers an "Open report" button).
    bool auto_open_report = false;

    // The dmleaderboards user (Discord ID) last compared against, so the
    // "Compare dmleaderboards user" picker can pre-select it. Empty = none yet.
    std::string dm_last_user;

    // The updater's two keys (D259 and D260 in
    // docs/audit/2026-10-03-fix-decisions.md). Neither is part of a result's
    // profile, so "Reset to defaults" leaves them alone.
    // The release version "Skip this version" hid, as version_text writes it;
    // empty when none.
    std::string skipped_update_version;
    // The version the last start ran, as version_text writes it; empty before
    // the first start that wrote it.
    std::string last_run_version;

    // The rules this process runs under, loaded from hydra_rules.ini at
    // startup. Never written to hydra_settings.ini: the rules file is the
    // user's to edit, the app only reads it.
    core::Rules rules = core::default_rules();

    static Settings load();
    // The settings a command-line run works under: load()'s, except the fill
    // rule, which comes from the run's own kLegacyFillsFlag and never from
    // the app's "1.0 fills" setting. So a run means the same thing whatever
    // the app was left on. The tools with that flag reach it through
    // load_tool_settings.
    static Settings load_for_command_line(bool legacy_fills_flag);
    // False when the INI can't be written (the GUI surfaces this; the CLIs
    // never call save()).
    bool save() const;

    // Explicit-path forms, so tests can round-trip through a temp file
    // without touching the real INI. load/save delegate here. A missing file
    // loads the defaults. A line the file can't hold throws
    // SettingsFileError naming the file, the key and the value (D270 call 4);
    // a retired key (kRetiredSettingsKeys) is skipped.
    static Settings load_file(const std::string& path);
    bool save_file(const std::string& path) const;

    // A number setting pulled into its allowed range: the one the key table
    // in config.cpp keeps, which its box reads too. A value outside the range
    // lands on the nearest edge (D51 Q14, D270 calls 1 and 2). Name the
    // setting by its field: clamp(&Settings::mslimit_value, 900) is 500.
    // load_file runs every whole-number setting through here.
    static int clamp(int Settings::* field, int value);

    // depth_mode as the search's enum: 1 is points, anything else scores.
    // Every reader of the mode as a search setting asks here.
    DepthMode search_depth_mode() const;

    // A Preview volume percent as a playback gain, clamped first like the
    // preview_volume key in config.cpp. The one percent-to-gain rule.
    static float volume_gain(int percent);

    // view_difficulty as the parsers' enum. The name matches in any case
    // ("hard" reads as Hard). An unrecognized string, which only a caller
    // setting the field can leave (load_file refuses one), reads as Expert.
    // chartmode_key() names the difficulty through here, so a junk word never
    // reaches a key.
    Difficulty difficulty() const;

    // Whether an analysis, the Preview and a record's key read 2x kicks: the
    // 2x Bass box, at every difficulty (D20). Callers ask here rather than
    // reading view_bass2x, so the rule has one owner.
    bool effective_bass2x() const;

    // The chart reading these settings ask for: difficulty(), view_prodrums,
    // effective_bass2x() and view_noteshuffle (ChartReading).
    ChartReading reading() const;

    // These settings with the view fields set to ask for `reading`, the rest
    // kept as they are; reading() of the result gives `reading` back. The one
    // place a whole reading is written onto the settings.
    Settings with_reading(const ChartReading& reading) const;

    // app::chartmode_key of reading().
    std::string chartmode_key() const;

    // These settings with the Difficulty, Pro Drums, 2x Bass and Note Shuffle
    // choices that make chartmode_key() spell `chartmode`, the rest kept as they
    // are. When several choices spell it, the first one this function tries
    // wins. nullopt when no choice spells it.
    std::optional<Settings> with_chartmode(const std::string& chartmode) const;

    // with_chartmode for a caller that cannot go on without the mode: throws
    // std::runtime_error, naming the text, when no choice spells it.
    Settings with_known_chartmode(const std::string& chartmode) const;

    // These settings with every field in kAnalysisFields put back to a
    // default-constructed Settings'; the rest (folders, volume, rules) are
    // kept. The panel's "Reset to defaults" button applies it.
    Settings with_analysis_defaults() const;

    // The panel settings with a field in kAnalysisFields that differs from a
    // default-constructed Settings', each once, in the panel's order: what
    // with_analysis_defaults would undo. The settings button's summary words
    // these, so it only names what a reset undoes.
    std::vector<AnalysisSetting> analysis_changes() const;

    AnalysisSettings to_analysis_settings() const;

    // The backend display window as the view-model wants it: the value when
    // the limit is on, nullopt (show every stored row) when off.
    std::optional<double> backend_limit() const;

    // Which stored record the current SP cap asks for (store::CapQuery).
    store::CapQuery cap_query() const;

    // The rest of the settings a stored result is keyed by: the ms limit, the
    // score range and the fill rule, in the store's canonical form.
    store::Lens lens() const;

    // The profile a chart is analyzed and filed under with these settings:
    // the chartmode, the SP cap and the lens. Callers ask here, or through
    // the named variants below, which start from it.
    store::Profile profile() const;

    // profile() at Clone Hero's own SP cap (kCloneHeroSpCap), whatever cap
    // these settings hold: the profile the leaderboard comparison reads.
    store::Profile profile_at_clone_hero_cap() const;

    // profile() with its lens's fill rule set to `rule`, the rest kept: the
    // fill comparison reads one profile per rule.
    store::Profile profile_with_fills(FillDeadlineRule rule) const;

    // Everything a batch run under these settings needs: profile(), which its
    // records are filed under, and the search settings. Both come from this
    // one Settings, so they cannot disagree.
    BatchRun batch_run() const;
};

// How a command-line tool loads its settings: Settings::load_for_command_line
// when the tool has a fill-rule flag of its own (`legacy_fills_flag`), else
// Settings::load. On success it fills `out` and returns 0. A refused file
// prints ini_file_tool_text (app/user_messages.h) on stderr and returns the
// tool's exit code, before the tool opens anything (D270 call 4). It is the
// settings file's twin of load_tool_rules (app/rules_file.h). The app does not
// call this: it keeps running on the defaults instead (AppState::settings_error).
int load_tool_settings(Settings& out, std::optional<bool> legacy_fills_flag);

// One field of the analysis set: the Settings member, the panel setting it
// belongs to and the key part it is filed under. Its default is the one a
// default-constructed Settings holds.
struct AnalysisField {
    std::variant<std::string Settings::*, bool Settings::*, int Settings::*> member;
    AnalysisSetting setting;
    KeyPart part;
};

// The analysis set, stated once: every field the Analysis settings panel
// sets, in the panel's order. Settings::with_analysis_defaults and
// Settings::analysis_changes read it, and the Reset test in
// tests/test_config.cpp walks it, so a new analysis setting is a new row here
// (concept audit M13). Nothing can notice a field left out of it.
inline const std::array<AnalysisField, 10> kAnalysisFields{{
    {&Settings::view_difficulty, AnalysisSetting::Difficulty, KeyPart::ChartMode},
    {&Settings::view_prodrums, AnalysisSetting::ProDrums, KeyPart::ChartMode},
    {&Settings::view_bass2x, AnalysisSetting::Bass2x, KeyPart::ChartMode},
    {&Settings::view_noteshuffle, AnalysisSetting::NoteShuffle, KeyPart::ChartMode},
    {&Settings::sp_cap, AnalysisSetting::SpCap, KeyPart::Cap},
    {&Settings::legacy_fills, AnalysisSetting::Fills, KeyPart::Lens},
    {&Settings::depth_value, AnalysisSetting::ScoreRange, KeyPart::Lens},
    {&Settings::depth_mode, AnalysisSetting::ScoreRange, KeyPart::Lens},
    {&Settings::mslimit_enabled, AnalysisSetting::PathLimit, KeyPart::Lens},
    {&Settings::mslimit_value, AnalysisSetting::PathLimit, KeyPart::Lens},
}};

// A count of Star Power bars in words, "4 bars" or "1 bar": the SP cap and
// the meter both read it.
std::string bars_text(int bars);

// A score range in words, the number before its unit: "4 scores", "1 point",
// "2,000 points".
std::string score_range_text(DepthMode depth_mode, int depth_value);

// The search settings a run uses, in the app's words (D114 item 9). The batch
// confirm, the settings button and hydra_batch's header read the same values;
// each caller supplies its own labels.
struct SettingsText {
    std::string depth;   // the score range: score_range_text
    std::string cap;     // the SP cap: bars_text
    std::string timing;  // the path limit: "10 ms", or "off"
};
SettingsText describe_settings(const AnalysisSettings& settings);

}  // namespace hydra::app

#endif  // HYDRA_APP_CONFIG_H
