// hydra_replay — price an arbitrary Star Power path, chord by chord.
//
// The GUI only ever shows the paths the search found. This tool answers the
// other question: "what is MY path worth, and where does each point come
// from?" It has four modes.
//
//   score     Walk a chart under a list of activation windows and dump every
//             chord with its own score breakdown, its running totals, and
//             whether it fell under Star Power. The windows come either from
//             --acts, typed by hand, or from --path, read straight out of a
//             file `dump` or `target` wrote so that nothing is retyped and
//             the squeeze-out offsets survive.
//   dump      Analyze the chart and print the paths the engine found, with
//             each activation's deactivation node resolved to a tick — the
//             input `score` wants.
//   target    Hand the engine an activation set -- "activate at exactly these
//             fill ticks and nowhere else" -- and get that path back priced the
//             engine's own way, with deactivation nodes, SP meters and squeeze
//             variants stamped as usual. Given whole windows (--path or --acts,
//             as score takes them) it pins each window's end too and returns
//             just that path. What `dump` gives you for the paths a search
//             happened to keep, this gives you for the path you name.
//   selfcheck Re-analyze the test corpus, replay every path the engine found,
//             and prove the replay's six score categories match the engine's
//             own. This is what keeps `score` honest. It also hands each path
//             back to target as a full pin, which must return it unchanged.
//
// The scoring itself lives in core/replay.h so this tool and tests/test_replay
// share one implementation.

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/rules_file.h"
#include "core/replay.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "replay_json.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "search/pather.h"

using namespace hydra;
using json = nlohmann::json;

namespace {

// ---- argument plumbing ---------------------------------------------------

struct Args {
    std::string command;
    std::string chart;
    std::string out;
    // The setting flags. Each is absent until typed: settings_from starts
    // from app::Settings, the app's own defaults, and changes only these.
    std::optional<std::string> cap;
    std::optional<std::string> ms;
    std::optional<std::string> depth_mode;
    std::optional<int> depth;
    std::optional<bool> prodrums;
    std::optional<bool> bass2x;
    std::optional<std::string> difficulty;
    std::string acts;
    std::string path;   // a dump/target JSON file to read a path out of
    int index = 0;      // which entry of that file's "paths" array
    std::string ticks;
    bool pretty = false;
    bool legacy_fills = false;
    std::string rules_path;  // --rules; empty = hydra_rules.ini next to the exe
    core::Rules rules;       // loaded once in main, before any command runs
};

void usage() {
    std::printf(
        "hydra_replay — score an arbitrary Star Power path, engine-exactly.\n\n"
        "  hydra_replay score --chart <file> [--acts \"actTick:deactTick[:sqoutMs],...\"]\n"
        "                     [--path <dump-or-target.json>] [--index N]\n"
        "                     [--out <json>] [--pretty] [--prodrums 0|1] [--bass2x 0|1]\n"
        "                     [--difficulty expert|hard|medium|easy]\n"
        "  hydra_replay dump  --chart <file> [--cap N]\n"
        "                     [--ms N|off] [--depth-mode scores|points] [--depth N]\n"
        "                     [--out <json>] [--pretty] [--legacy-fills]\n"
        "  hydra_replay target --chart <file> [--ticks \"t1,t2,...\"]\n"
        "                     [--acts \"actTick:deactTick[:sqoutMs],...\"]\n"
        "                     [--path <dump-or-target.json>] [--index N] [--cap N]\n"
        "                     [--out <json>] [--pretty] [--prodrums 0|1] [--bass2x 0|1]\n"
        "                     [--difficulty expert|hard|medium|easy] [--legacy-fills]\n"
        "  hydra_replay selfcheck [--chart <file>] [--verbose]\n\n"
        "Omitting both --acts and --path scores the chart with no Star Power\n"
        "anywhere. --path reads a path straight out of a file dump or target\n"
        "wrote, so nothing is retyped and no field is lost -- in particular the\n"
        "squeeze-out offset, which an --acts string typed by hand usually drops\n"
        "and which is worth real points. --index picks the entry of that file's\n"
        "\"paths\" array (default 0). --path and --acts cannot both be given.\n"
        "A typed SqOut offset is matched to the nearest phrase chord within %.0f ms of the SP end and the chord used is printed on stderr; an exact tie between chords, or a chord the engine would never squeeze out, is refused.\n"
        "Where a window ends on a Star Power phrase note but carries no\n"
        "squeeze-out offset, score prints a warning: the score is right if the\n"
        "player did not squeeze that note out, and high if they did. The\n"
        "warnings are also in the JSON, as \"warnings\".\n"
        "score's JSON also carries \"sections\": the chart's practice sections, "
        "each with tick, ms, and name.\n"
        "dump analyzes the chart every time and reads no database; its JSON\n"
        "field \"source\" is always \"analyzed\".\n"
        "target asks the engine to price one specific path. With --ticks it\n"
        "activates at exactly those fills and nowhere else, and every way of\n"
        "ending each activation comes back; on a long chart that can run for a\n"
        "very long time. With --path or --acts, read the way score reads them,\n"
        "each window's end is pinned too (and its squeeze-out, when the window\n"
        "names one), so exactly that path comes back, fast. Give one of the three.\n"
        "A typed SqOut offset is matched to its chord as score matches it, and\n"
        "\"pins\" in the JSON lists what each window was pinned to.\n"
        "Its JSON carries dump's \"paths\" shape plus \"realized\"; when that is\n"
        "false, \"failed_tick\" names the activation of the first window no path\n"
        "realizes, \"realized_prefix\" how many of the leading windows one does,\n"
        "and \"failed_reason\" what broke: \"activation\", \"window_end\" or\n"
        "\"sqout\" (see TargetResult in search/pather.h).\n"
        "--legacy-fills prices the chart under Clone Hero 1.0's fill deadline,\n"
        "which is what a 1.0 run was played under, the same as the app's\n"
        "\"1.0 fills\" setting.\n"
        "Every command takes --rules <file>: the rule choices to price under\n"
        "(default: hydra_rules.ini next to the exe). A bad file exits with 2.\n"
        "JSON is printed compact by default; --pretty indents it.\n",
        kSqueezeWindowMs);
}

// ---- shared settings -----------------------------------------------------

// The app's settings with the typed flags applied. A flag not given keeps the
// app's default, so this tool keys and prices a run exactly as the app does.
app::Settings settings_from(const Args& a) {
    using app::Settings;
    Settings s;  // the app's defaults
    s.rules = a.rules;
    if (a.prodrums) s.view_prodrums = *a.prodrums;
    if (a.bass2x) s.view_bass2x = *a.bass2x;
    // Settings::difficulty() reads the word, an unknown one as Expert.
    if (a.difficulty) s.view_difficulty = *a.difficulty;

    if (a.cap) {
        const int cap = std::atoi(a.cap->c_str());
        // The INI reads a cap below the floor as Clone Hero's cap; a cap typed
        // on the command line below it is a mistake, so it is refused. A cap
        // below the floor clamps to the floor, which the message names.
        const int in_range = Settings::clamp(&Settings::sp_cap, cap);
        if (in_range != cap)
            throw std::runtime_error("--cap takes a whole number of bars, " +
                                     std::to_string(in_range) + " or more (" +
                                     std::to_string(kCloneHeroSpCap) +
                                     " is Clone Hero's rule), not \"" + *a.cap + "\"");
        s.sp_cap = cap;
    }

    if (a.ms) {
        if (*a.ms == "off") s.mslimit_enabled = false;
        else { s.mslimit_enabled = true; s.mslimit_value = std::atoi(a.ms->c_str()); }
    }

    if (a.depth_mode) s.depth_mode = *a.depth_mode == "points" ? 1 : 0;
    if (a.depth) s.depth_value = *a.depth;

    // The 1.0 fill rule goes through Settings, so the search and the record
    // key carry it together, as the app's "1.0 fills" setting does.
    s.legacy_fills = a.legacy_fills;

    // Every number lands in its range the way the app's own settings do
    // (--depth -1 reads as 0, --ms 900 as 500). Settings::clamp owns the ranges.
    for (int Settings::* field : {&Settings::sp_cap, &Settings::mslimit_value,
                                  &Settings::depth_mode, &Settings::depth_value})
        s.*field = Settings::clamp(field, s.*field);
    return s;
}

void emit(const json& j, const std::string& out, bool pretty) {
    const std::string text = pretty ? j.dump(1, ' ') : j.dump();
    if (out.empty()) {
        std::fwrite(text.data(), 1, text.size(), stdout);
        std::fputc('\n', stdout);
        return;
    }
    std::ofstream f(hydra::os_path(out), std::ios::binary | std::ios::trunc);
    f << text << "\n";
    std::printf("wrote %s (%zu bytes)\n", out.c_str(), text.size());
}

// ---- score ---------------------------------------------------------------

// A strict, whole-string std::strtoll: no leading/trailing junk, no empty
// field. `what` and `item` name the offending piece in the exception, so a
// typo in --acts says exactly which field and which item it broke on rather
// than printing a silently wrong score.
int64_t strict_ll(const std::string& field, const std::string& what,
                  const std::string& item) {
    const std::string t = trim(field);
    if (t.empty())
        throw std::runtime_error("--acts item '" + item + "' has an empty " +
                                 what);
    char* end = nullptr;
    errno = 0;
    const long long v = std::strtoll(t.c_str(), &end, 10);
    if (end != t.c_str() + t.size() || errno == ERANGE)
        throw std::runtime_error("--acts item '" + item + "' has a bad " +
                                 what + " '" + t + "'");
    return v;
}

double strict_d(const std::string& field, const std::string& what,
                const std::string& item) {
    const std::string t = trim(field);
    if (t.empty())
        throw std::runtime_error("--acts item '" + item + "' has an empty " +
                                 what);
    char* end = nullptr;
    errno = 0;
    const double v = std::strtod(t.c_str(), &end);
    if (end != t.c_str() + t.size() || errno == ERANGE)
        throw std::runtime_error("--acts item '" + item + "' has a bad " +
                                 what + " '" + t + "'");
    return v;
}

// "12:3456,7890:9012" -> windows. Throws on anything malformed: a silently
// dropped or misparsed activation would print a wrong score with no hint
// why. Each field is parsed strictly (strtoll/strtod with an end-pointer
// check after trimming spaces); leftover characters, an empty field, a
// negative tick, or a deactivation before its activation all throw, naming
// the offending item.
std::vector<ReplayWindow> parse_acts(const std::string& spec) {
    std::vector<ReplayWindow> out;
    size_t i = 0;
    while (i < spec.size()) {
        size_t comma = spec.find(',', i);
        if (comma == std::string::npos) comma = spec.size();
        std::string item = spec.substr(i, comma - i);
        i = comma + 1;
        item = trim(item);
        if (item.empty()) continue;

        const size_t colon = item.find(':');
        if (colon == std::string::npos)
            throw std::runtime_error("--acts item '" + item +
                                     "' is not actTick:deactTick");
        const std::string rest = item.substr(colon + 1);
        const size_t colon2 = rest.find(':');
        const std::string deact_field = rest.substr(0, colon2);

        ReplayWindow w;
        w.act_tick = strict_ll(item.substr(0, colon), "act tick", item);
        w.deact_tick = strict_ll(deact_field, "deact tick", item);
        if (w.act_tick < 0)
            throw std::runtime_error("--acts item '" + item +
                                     "' has a negative act tick");
        if (w.deact_tick < 0)
            throw std::runtime_error("--acts item '" + item +
                                     "' has a negative deact tick");
        if (w.deact_tick < w.act_tick)
            throw std::runtime_error("--acts item '" + item +
                                     "' has deact tick before act tick");
        // Optional third field: the SqOut phrase note's ms offset from the
        // deactivation node, which `dump` prints for a squeeze-out activation.
        if (colon2 != std::string::npos)
            w.sqout_offset_ms =
                strict_d(rest.substr(colon2 + 1), "sqout offset", item);
        out.push_back(w);
    }
    return out;
}

// --index, whole-string. std::atoi would turn "1x" or "one" into 0 and price
// the first path as if that were what was asked for.
int parse_index(const std::string& v) {
    const std::string t = trim(v);
    char* end = nullptr;
    errno = 0;
    const long long n = std::strtoll(t.c_str(), &end, 10);
    if (t.empty() || end != t.c_str() + t.size() || errno == ERANGE || n < 0 ||
        n > 1000000)
        throw std::runtime_error("--index '" + v +
                                 "' is not a whole path number");
    return static_cast<int>(n);
}

// Read one path out of the JSON `dump` or `target` wrote. The conversion to
// windows itself lives in core/replay.h so tests can pin it; this only opens
// the file and picks the entry out of the "paths" array. Every failure names
// the file and the index, because "score came out wrong" is much harder to
// notice than "that file does not have a path 7".
std::vector<ReplayWindow> windows_from_file(const std::string& file, int index) {
    std::ifstream in(hydra::os_path(file), std::ios::binary);
    if (!in) throw std::runtime_error("cannot read --path file: " + file);

    json doc;
    try {
        in >> doc;
    } catch (const std::exception& e) {
        throw std::runtime_error("--path file " + file +
                                 " is not valid JSON: " + e.what());
    }
    if (!doc.is_object() || !doc.contains("paths") || !doc["paths"].is_array())
        throw std::runtime_error("--path file " + file +
                                 " has no \"paths\" array; it should be the "
                                 "JSON that dump or target writes");

    const json& paths = doc["paths"];
    if (index < 0 || static_cast<size_t>(index) >= paths.size())
        throw std::runtime_error("--path file " + file + " holds " +
                                 std::to_string(paths.size()) +
                                 " path(s), so --index " +
                                 std::to_string(index) + " is out of range");

    try {
        return windows_from_json(paths[index]);
    } catch (const std::exception& e) {
        throw std::runtime_error("--path file " + file + ", index " +
                                 std::to_string(index) + ": " + e.what());
    }
}

// `s` is settings_from(a), built once in main.
int cmd_score(const Args& a, const app::Settings& s) {
    if (a.chart.empty()) { usage(); return 2; }
    if (!a.path.empty() && !a.acts.empty())
        throw std::runtime_error(
            "--path and --acts each name a path to score; give one or the "
            "other, not both");

    Song song = load_songpath(a.chart, s.view_prodrums, s.effective_bass2x(),
                              s.difficulty(), s.rules);
    if (song.is_empty()) {
        std::fprintf(stderr, "chart has no notes: %s\n", a.chart.c_str());
        return 1;
    }

    std::vector<ReplayWindow> windows =
        a.path.empty() ? parse_acts(a.acts)
                       : windows_from_file(a.path, a.index);
    // A typed offset (or a dump from before sqout_tick existed) names a chord
    // only approximately. Resolve it and say which chord was used, so a typo
    // cannot quietly price a different squeeze-out. When resolve_sqout_note
    // refuses (its header lists when), main prints "error: ..." and exits 1,
    // so nothing is priced.
    for (ReplayWindow& w : windows) {
        if (!w.sqout_offset_ms || w.sqout_tick) continue;
        const SqOutNote n = resolve_sqout_note(song, w);
        std::fprintf(stderr,
                     "note: window %lld:%lld SqOut %.2f ms -> phrase chord at "
                     "tick %lld (%.2f ms from the SP end)\n",
                     (long long)w.act_tick, (long long)w.deact_tick,
                     *w.sqout_offset_ms, (long long)n.tick, n.offset_ms);
        w.sqout_tick = n.tick;
    }
    const ReplayResult r = replay_path(song, windows, s.rules);

    // Say so when a window could be hiding a squeeze-out. The score is left
    // exactly as it is: only the player knows whether they squeezed.
    const std::vector<std::string> warnings =
        ambiguous_window_warnings(song, r, windows, s.rules);
    for (const std::string& w : warnings)
        std::fprintf(stderr, "warning: %s\n", w.c_str());

    json acts = json::array();
    for (const ReplayWindow& w : windows) {
        json one{{"act_tick", w.act_tick}, {"deact_tick", w.deact_tick}};
        if (w.sqout_offset_ms) one["sqout_offset_ms"] = *w.sqout_offset_ms;
        if (w.sqout_tick) one["sqout_tick"] = *w.sqout_tick;
        acts.push_back(one);
    }

    json chords = json::array();
    for (const ReplayChord& c : r.chords) {
        json notes = json::array();
        for (const ReplayNote& n : c.notes)
            notes.push_back(json{{"color", color_str(n.color)},
                                 {"cymbal", n.cymbal},
                                 {"sp_points", n.sp_points},
                                 {"multiplier", n.multiplier},
                                 {"dynamics_bonus", n.dynamics_bonus},
                                 {"dynamic", dynamic_str(n.dynamic)}});
        json cum = score_json(c.cum);
        cum["total"] = c.cum.total();
        chords.push_back(json{
            {"index", c.index},
            {"tick", c.tick},
            {"ms", c.ms},
            {"measure", json{{"measure", c.measure},
                             {"beat", c.beat},
                             {"tick", c.measure_tick},
                             {"decimal", c.measures_decimal}}},
            {"chord_code", c.chord_code},
            {"notes", notes},
            {"is_fill", c.is_fill},
            {"is_solo", c.is_solo},
            {"is_sp_phrase_end", c.is_sp_phrase_end},
            {"combo_before", c.combo_before},
            {"multiplier", c.multiplier},
            {"multiplier_after", c.multiplier_after},
            {"in_sp", c.in_sp},
            {"points", score_json(c.points)},
            {"cum", cum},
            {"cum_onscreen_total", c.cum_onscreen_total},
        });
    }

    json final = score_json(r.final);
    final["total"] = r.final.total();

    json sections = json::array();
    for (const SongSection& sec : song.practice_sections)
        sections.push_back(json{{"tick", sec.tick},
                                {"ms", song.timing().timecode(sec.tick).ms()},
                                {"name", sec.name}});

    json out{{"chart", a.chart},
             {"chartmode", s.chartmode_key()},
             {"activations", acts},
             {"chords", chords},
             {"final", final},
             {"sections", sections},
             {"warnings", warnings}};
    // Only when the path came from a file, so a caller can tell a file-read
    // path from a hand-typed one without guessing.
    if (!a.path.empty())
        out["path_source"] = json{{"file", a.path}, {"index", a.index}};

    emit(out, a.out, a.pretty);
    return 0;
}

// ---- dump ----------------------------------------------------------------

// `s` is settings_from(a). With --legacy-fills it analyzes under Clone Hero
// 1.0's fill deadline (docs/adr/0010). dump reads no database (D87): the paths
// come from app::analyze_chart_file, the same analysis a click in the app runs.
// It only prints; nothing is stored.
int cmd_dump(const Args& a, const app::Settings& s) {
    if (a.chart.empty()) { usage(); return 2; }

    const std::string hyhash = app::hash_chart_file(a.chart);
    if (hyhash.empty()) {
        std::fprintf(stderr, "cannot hash chart: %s\n", a.chart.c_str());
        return 1;
    }

    const app::AnalysisResult analysis =
        app::analyze_chart_file(a.chart, s.to_analysis_settings());
    const HydraRecord& rec = analysis.record;

    // "source" was "record" or "analyzed" while dump could read a stored row.
    // It stays in the JSON, always "analyzed", so scripts that read it still work.
    emit(json{{"hyhash", hyhash},
              {"chartmode", s.chartmode_key()},
              {"source", "analyzed"},
              {"sp_cap", rec.sp_cap ? *rec.sp_cap : -1},
              {"result", result_json(rec)},
              {"paths", paths_json(rec.all_paths(), analysis.song.timing())}},
         a.out, a.pretty);
    return 0;
}

// ---- target --------------------------------------------------------------

// "129600,296160,..." -> ticks. Strict like parse_acts: a comma-separated list
// of non-negative integers, spaces trimmed, anything else throws naming the
// offending item. A misparsed tick would silently ask the engine for a
// different path than the one the user meant.
std::vector<int64_t> parse_ticks(const std::string& spec) {
    std::vector<int64_t> out;
    size_t i = 0;
    while (i < spec.size()) {
        size_t comma = spec.find(',', i);
        if (comma == std::string::npos) comma = spec.size();
        std::string item = trim(spec.substr(i, comma - i));
        i = comma + 1;
        if (item.empty()) continue;
        const int64_t t = strict_ll(item, "tick", item);
        if (t < 0)
            throw std::runtime_error("--ticks item '" + item +
                                     "' is a negative tick");
        out.push_back(t);
    }
    return out;
}

// `s` is settings_from(a), built once in main.
int cmd_target(const Args& a, const app::Settings& s) {
    if (a.chart.empty()) { usage(); return 2; }
    if ((!a.ticks.empty()) + (!a.path.empty()) + (!a.acts.empty()) > 1)
        throw std::runtime_error(
            "--ticks, --path and --acts each name the path to find; give one of them");

    const std::string hyhash = app::hash_chart_file(a.chart);
    const std::vector<int64_t> ticks = parse_ticks(a.ticks);

    Song song = load_songpath(a.chart, s.view_prodrums, s.effective_bass2x(),
                              s.difficulty(), s.rules);
    if (song.is_empty()) {
        std::fprintf(stderr, "chart has no notes: %s\n", a.chart.c_str());
        return 1;
    }

    // --ticks pins activations only; --path and --acts pin whole windows,
    // read exactly as score reads them.
    std::vector<PinnedWindow> windows;
    if (a.path.empty() && a.acts.empty()) {
        std::vector<int64_t> sorted = ticks;
        std::sort(sorted.begin(), sorted.end());
        sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
        for (const int64_t t : sorted) windows.push_back(PinnedWindow{t});
    } else {
        // A typed offset names its chord only approximately; pinned_windows
        // matches it, and "pins" in the JSON shows the chord used.
        windows = pinned_windows(
            song, a.path.empty() ? parse_acts(a.acts) : windows_from_file(a.path, a.index));
    }

    // With --legacy-fills, s prices under Clone Hero 1.0's fill deadline.
    // target only ever prints; nothing is stored.
    const SearchSettings cfg = s.to_analysis_settings();
    TargetResult found = search_target(song, cfg, windows);
    HydraRecord rec;
    rec.paths = std::move(found.paths);

    // When the windows are unrealizable, search_target has already named the
    // one that broke and why. Its cost is one graph build plus one search per
    // run, a binary search's worth of runs; a run with every window's end
    // pinned walks the graph once.
    const bool realized = !rec.paths.empty();
    std::vector<int64_t> act_ticks;
    // What each window was pinned to; -1 for a value not pinned, as dump
    // writes a missing one.
    json pins = json::array();
    for (const PinnedWindow& w : windows) {
        act_ticks.push_back(w.act_tick);
        pins.push_back(json{{"act_tick", w.act_tick},
                            {"deact_tick", w.deact_tick.value_or(-1)},
                            {"check_sqout", w.check_sqout},
                            {"sqout_tick", w.squeezed_out.value_or(-1)}});
    }

    emit(json{{"hyhash", hyhash},
              {"chartmode", s.chartmode_key()},
              {"source", "target"},
              {"sp_cap", cfg.sp_cap},
              {"ticks", a.ticks.empty() ? act_ticks : ticks},
              {"pins", pins},
              {"result", result_json(rec)},
              {"paths", paths_json(rec.all_paths(), song.timing())},
              {"realized", realized},
              {"failed_tick", found.failed_tick.value_or(-1)},
              {"failed_reason", found.failed_reason},
              {"realized_prefix", found.realized_prefix}},
         a.out, a.pretty);
    return 0;
}

// ---- selfcheck -----------------------------------------------------------

struct Tally {
    int charts = 0;
    int paths = 0;
    int pass = 0;
    int fail = 0;
    int skipped = 0;
    // The second check: each path handed back to target as a full pin
    // (full_pin_mismatch).
    int pinned_pass = 0;
    int pinned_fail = 0;
};

bool g_verbose = false;

void check_chart(const std::string& path, const core::Rules& rules, Tally* tally) {
    // The GUI's defaults, straight from app::Settings rather than five
    // hand-written literals, under the rules this run loaded.
    app::Settings defaults;
    defaults.rules = rules;
    const app::AnalysisSettings cfg = defaults.to_analysis_settings();

    std::optional<Song> song_opt;
    try {
        song_opt.emplace(
            load_songpath(path, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules));
    } catch (const std::exception& e) {
        std::printf("SKIP %s (%s)\n", path.c_str(), e.what());
        ++tally->skipped;
        return;
    }
    const Song& song = *song_opt;
    if (song.is_empty()) { ++tally->skipped; return; }

    HydraRecord rec;
    try {
        rec = analyze_chart(song, cfg);
    } catch (const std::exception& e) {
        std::printf("SKIP %s (%s)\n", path.c_str(), e.what());
        ++tally->skipped;
        return;
    }
    ++tally->charts;

    int index = 0, chart_fail = 0, chart_pinned_fail = 0;
    for (const Path* p : rec.all_paths()) {
        const int i = index++;
        ++tally->paths;

        // target must hand this path back, alone and the same, from a full
        // pin of its windows.
        std::string pin_diff;
        try {
            pin_diff = full_pin_mismatch(song, cfg, *p);
        } catch (const std::exception& e) {
            pin_diff = std::string("target threw: ") + e.what();
        }
        if (pin_diff.empty()) {
            ++tally->pinned_pass;
        } else {
            ++tally->pinned_fail;
            if (++chart_pinned_fail <= 5)
                std::printf("FAIL target %s\n     path %d [%s]: %s\n", path.c_str(), i,
                            p->pathstring().c_str(), pin_diff.c_str());
        }

        const PathReplay pr = replay_stored_path(song, *p, rules);
        const std::vector<ReplayWindow>& windows = pr.windows;
        const ReplayResult& r = pr.result;
        const ReplayScore& want = pr.stored;

        std::string diffs;
        if (!pr.totals_match()) {
            for (const ReplayScoreField& f : kReplayScoreFields) {
                const int64_t got = r.final.*(f.member);
                const int64_t wanted = want.*(f.member);
                if (got == wanted) continue;
                if (!diffs.empty()) diffs += ", ";
                diffs += std::string(f.name) + " " + std::to_string(got) + " vs " +
                         std::to_string(wanted) + " (" + std::to_string(got - wanted) + ")";
            }
        }
        if (!pr.all_windows())
            diffs += (diffs.empty() ? "" : ", ") + std::string("only ") +
                     std::to_string(windows.size()) + " of " +
                     std::to_string(pr.activations) +
                     " activations resolved";

        if (g_verbose) {
            std::printf("path %d [%s] windows:", i, p->pathstring().c_str());
            for (const ReplayWindow& w : windows)
                std::printf(" %lld:%lld", (long long)w.act_tick,
                            (long long)w.deact_tick);
            std::printf("\n");
        }
        if (diffs.empty()) {
            ++tally->pass;
        } else {
            ++tally->fail;
            ++chart_fail;
            if (chart_fail <= 5) {
                std::printf("FAIL %s\n     path %d [%s]: %s\n", path.c_str(), i,
                            p->pathstring().c_str(), diffs.c_str());
                // The windows the replay used, so a mismatch names the
                // activation whose deactivation node was not recovered.
                const ActivationWalk acts = p->walk_activations();
                for (size_t k = 0; k < acts.size(); ++k) {
                    const Activation& act = acts[k];
                    const int64_t act_tick = act.timecode.ticks();
                    const int64_t nominal = act.nominal_end().value_or(-1);
                    std::printf(
                        "       act %zu: tick %lld  deact %lld  nominal %lld  "
                        "sp_meter %d  skips %d  backends %zu  sqinouts %zu\n",
                        k, (long long)act_tick,
                        (long long)act.deact_tick().value_or(-1),
                        (long long)nominal, act.sp_meter(),
                        act.skips(), act.backends.size(),
                        act.sqinouts.size());
                }
            }
        }
    }
    if (chart_fail > 5)
        std::printf("     ...and %d more failing paths in this chart\n",
                    chart_fail - 5);
    if (chart_pinned_fail > 5)
        std::printf("     ...and %d more paths target failed on in this chart\n",
                    chart_pinned_fail - 5);
    std::fflush(stdout);
}

int cmd_selfcheck(const Args& a) {
    Tally tally;
    if (!a.chart.empty()) {
        check_chart(a.chart, a.rules, &tally);
    } else {
        for (const std::string& p : corpus::chart_paths()) check_chart(p, a.rules, &tally);
    }
    std::printf(
        "\nselfcheck: %d chart(s), %d path(s) — PASS %d, FAIL %d (%d chart(s) "
        "skipped)\n",
        tally.charts, tally.paths, tally.pass, tally.fail, tally.skipped);
    std::printf("target: %d path(s) pinned, PASS %d, FAIL %d\n",
                tally.pinned_pass + tally.pinned_fail, tally.pinned_pass, tally.pinned_fail);
    return tally.fail == 0 && tally.pinned_fail == 0 ? 0 : 1;
}

}  // namespace

int main() {
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());
    if (argc < 2) { usage(); return 2; }

    Args a;
    a.command = args[1];
    for (int i = 2; i < argc; ++i) {
        const std::string k = args[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("missing value for " + k);
            return args[++i];
        };
        // An on/off flag reads its value the way the app's settings file does.
        auto next_on_off = [&]() -> bool {
            const std::string v = next();
            const std::optional<bool> on = parse_bool(v);
            if (!on) throw std::runtime_error(k + " takes 0 or 1, not \"" + v + "\"");
            return *on;
        };
        try {
            if (k == "--chart") a.chart = next();
            else if (k == "--rules") a.rules_path = next();
            else if (k == "--out") a.out = next();
            else if (k == "--cap") a.cap = next();
            else if (k == "--ms") a.ms = next();
            else if (k == "--depth-mode") a.depth_mode = next();
            else if (k == "--depth") a.depth = std::atoi(next().c_str());
            else if (k == "--acts") a.acts = next();
            else if (k == "--path") a.path = next();
            // Parsed strictly: std::atoi turns a typo into 0, which would
            // quietly price the wrong path instead of saying anything.
            else if (k == "--index") a.index = parse_index(next());
            else if (k == "--ticks") a.ticks = next();
            else if (k == "--verbose") g_verbose = true;
            else if (k == "--prodrums") a.prodrums = next_on_off();
            else if (k == "--bass2x") a.bass2x = next_on_off();
            else if (k == "--difficulty") a.difficulty = next();
            else if (k == "--pretty") a.pretty = true;
            else if (k == app::kLegacyFillsFlag) a.legacy_fills = true;
            else { std::fprintf(stderr, "unknown option %s\n", k.c_str()); usage(); return 2; }
        } catch (const std::exception& e) {
            std::fprintf(stderr, "%s\n", e.what());
            return 2;
        }
    }

    try {
        a.rules = app::load_rules_file(a.rules_path.empty()
                                           ? app::default_rules_path()
                                           : std::filesystem::u8path(a.rules_path));
    } catch (const app::RulesFileError& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }

    // A bad setting flag (a cap below the floor) is a bad argument: exit 2,
    // like the others, before any command runs.
    app::Settings s;
    try {
        s = settings_from(a);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }

    try {
        if (a.command == "score") return cmd_score(a, s);
        if (a.command == "dump") return cmd_dump(a, s);
        if (a.command == "target") return cmd_target(a, s);
        if (a.command == "selfcheck") return cmd_selfcheck(a);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    usage();
    return 2;
}
