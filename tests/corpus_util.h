// Helpers for iterating the checked-in chart corpus (testdata/input) from the
// C++ tests. The corpus is enumerated with app::discover_charts — the same
// discovery the app's library scan uses — so the tests exercise exactly the
// set of charts a user's scan of this tree would find.
//
// (This header replaced tests/golden_util.h when the golden oracle data was
// removed; the tests assert structural invariants and self-consistency now,
// not byte parity against stored output.)

#ifndef HYDRA_TESTS_CORPUS_UTIL_H
#define HYDRA_TESTS_CORPUS_UTIL_H

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "json.hpp"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "search/pather.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

namespace corpus {

using json = nlohmann::json;

inline std::string root() { return HYDRA_INPUT_DIR; }

// Every chart file in the corpus (full notespath), sorted for a stable
// iteration order. Discovery errors throw: the corpus is checked in, so any
// error means a broken tree, not an environment problem.
inline const std::vector<std::string>& chart_paths() {
    static const std::vector<std::string> paths = [] {
        auto [items, errors] = hydra::app::discover_charts({root()});
        if (!errors.empty())
            throw std::runtime_error("corpus discovery error: " + errors.front());
        std::vector<std::string> out;
        out.reserve(items.size());
        for (const auto& item : items) out.push_back(item.notespath);
        std::sort(out.begin(), out.end());
        return out;
    }();
    return paths;
}

// First corpus chart of the format `suffix` names (e.g. ".mid"), read the
// way the app reads a chart file's format (chart_format_of, any case).
inline std::string first_chart_with_suffix(const std::string& suffix) {
    const hydra::ChartFormat want = hydra::chart_format_of(suffix);
    for (const std::string& p : chart_paths()) {
        if (hydra::chart_format_of(p) == want) return p;
    }
    throw std::runtime_error("no corpus chart ends in " + suffix);
}

// One corpus chart's file (its full notespath) and its analysis, for a caller
// that opens the chart again by its path.
struct ChartWithPaths {
    std::string chart;
    hydra::app::AnalysisResult result;
};

// The first `want` corpus charts, in chart_paths() order, that analyze under
// `settings` to a song with notes and at least one path, each with its file.
// A chart that fails to analyze is skipped, as is one with an empty song or
// no paths.
inline std::vector<ChartWithPaths> charts_with_paths(
    const hydra::app::AnalysisSettings& settings, size_t want) {
    std::vector<ChartWithPaths> out;
    for (const std::string& path : chart_paths()) {
        if (out.size() == want) break;
        try {
            hydra::app::AnalysisResult result = hydra::app::analyze_chart_file(path, settings);
            if (result.song.is_empty() || result.record.paths.empty()) continue;
            out.push_back({path, std::move(result)});
        } catch (const std::exception&) {
            continue;
        }
    }
    return out;
}

// The same charts' analyses alone.
inline std::vector<hydra::app::AnalysisResult> analyzed_with_paths(
    const hydra::app::AnalysisSettings& settings, size_t want) {
    std::vector<hydra::app::AnalysisResult> out;
    for (ChartWithPaths& c : charts_with_paths(settings, want)) out.push_back(std::move(c.result));
    return out;
}

// The first corpus chart with paths under `settings`, with its file. Throws,
// naming the corpus, when no chart has any.
inline ChartWithPaths first_chart_with_paths(const hydra::app::AnalysisSettings& settings) {
    std::vector<ChartWithPaths> one = charts_with_paths(settings, 1);
    if (one.empty())
        throw std::runtime_error("no chart under " + root() + " analyzes to any path");
    return std::move(one.front());
}

// The first corpus chart with paths under `settings`, its analysis alone.
inline hydra::app::AnalysisResult first_analyzed_with_paths(
    const hydra::app::AnalysisSettings& settings) {
    return std::move(first_chart_with_paths(settings).result);
}

inline std::string read_bytes(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline json load_json(const std::string& path) {
    return json::parse(read_bytes(path));
}

// ---- cached corpus work ---------------------------------------------------
// Many corpus loops parse the same chart, or analyze it at the same settings,
// as a loop in another test case. These two calls do each piece of work once
// per test run and hand every later caller the same answer. A failure is
// cached too: every call for that key throws the same exception again, so a
// loop's try/catch behaves exactly as it did around the direct call.
// Both return const references into caches that live for the whole run; a
// test that needs to change the Song or the record copies it first.

namespace detail {

template <class T>
struct Outcome {
    std::optional<T> value;
    std::exception_ptr error;
};

}  // namespace detail

// The Song for one corpus chart, parsed once per run for these load options
// (the arguments load_songpath takes).
inline const hydra::Song& song(const std::string& path, bool pro, bool bass2x,
                               hydra::Difficulty difficulty = hydra::Difficulty::Expert,
                               const hydra::core::Rules& rules = hydra::core::default_rules()) {
    static std::map<std::string, detail::Outcome<hydra::Song>> cache;
    std::ostringstream key;
    key.precision(17);
    key << path << '|' << pro << '|' << bass2x << '|' << static_cast<int>(difficulty) << '|'
        << rules.fingerprint();
    auto [it, fresh] = cache.try_emplace(key.str());
    detail::Outcome<hydra::Song>& o = it->second;
    if (fresh) {
        try {
            o.value.emplace(hydra::load_songpath(path, pro, bass2x, difficulty, rules));
        } catch (...) {
            o.error = std::current_exception();
        }
    }
    if (o.error) std::rethrow_exception(o.error);
    return *o.value;
}

namespace detail {

// First corpus chart whose song at `difficulty` (pro drums, 2x bass) has
// notes when `with_notes` is true, or has none when it is false.
inline std::string first_chart_where(hydra::Difficulty difficulty, bool with_notes) {
    for (const std::string& p : chart_paths()) {
        if (song(p, true, true, difficulty).is_empty() != with_notes) return p;
    }
    throw std::runtime_error(std::string("no corpus chart ") +
                             (with_notes ? "has" : "lacks") + " notes at that difficulty");
}

}  // namespace detail

// First corpus chart with notes at `difficulty` (pro drums, 2x bass).
inline std::string first_chart_with_notes(
    hydra::Difficulty difficulty = hydra::Difficulty::Expert) {
    return detail::first_chart_where(difficulty, true);
}

// First corpus chart with no notes at `difficulty` (pro drums, 2x bass), the
// chart a missing-difficulty test needs.
inline std::string first_chart_without_notes(hydra::Difficulty difficulty) {
    return detail::first_chart_where(difficulty, false);
}

// One corpus chart analyzed under `settings`, once per run: the record
// analyze_chart_file would return (parsed with settings.prodrums, bass2x,
// difficulty and rules, checked with require_notes, then analyze_chart).
// Throws what analyze_chart_file throws: NoNotesError when the chart has no
// notes at that difficulty, ChartFileError when the file cannot be read.
inline const hydra::HydraRecord& analyzed(const std::string& path,
                                          const hydra::app::AnalysisSettings& settings) {
    static std::map<std::string, detail::Outcome<hydra::HydraRecord>> cache;
    std::ostringstream key;
    key << path << '|' << settings.prodrums << '|' << settings.bass2x << '|'
        << static_cast<int>(settings.difficulty) << '|' << hydra::settings_key(settings);
    auto [it, fresh] = cache.try_emplace(key.str());
    detail::Outcome<hydra::HydraRecord>& o = it->second;
    if (fresh) {
        try {
            const hydra::Song& s = song(path, settings.prodrums, settings.bass2x,
                                        settings.difficulty, settings.rules);
            hydra::require_notes(s, settings.difficulty, settings.prodrums);
            o.value.emplace(hydra::analyze_chart(s, settings));
        } catch (...) {
            o.error = std::current_exception();
        }
    }
    if (o.error) std::rethrow_exception(o.error);
    return *o.value;
}

// The same for a loop that holds plain SearchSettings and parses with
// load_songpath(path, true, true): pro drums, 2x bass, Expert.
inline const hydra::HydraRecord& analyzed(const std::string& path,
                                          const hydra::SearchSettings& settings) {
    hydra::app::AnalysisSettings a;
    static_cast<hydra::SearchSettings&>(a) = settings;
    return analyzed(path, a);
}

// Empty when the activation carries every stored fact its transfer scales
// need; otherwise what is missing. D4: nothing here may be missing on a fresh
// record.
inline std::string unknown_scale_reason(const hydra::Activation& a) {
    auto good = [](const hydra::TransferScale& s) {
        return std::isfinite(s.early) && std::isfinite(s.late) && s.early > 0.0 &&
               s.late > 0.0;
    };
    if (!a.deact_tick()) return "no SP-end steps";
    if (!a.nominal_end()) return "no nominal end";
    if (!a.transfer_post) return "transfer_post unknown";
    if (!good(*a.transfer_post)) return "transfer_post not a positive finite number";
    for (size_t k = 0; k < a.sqinouts.size(); ++k) {
        if (a.sqinouts[k].kind != hydra::SqueezeKind::SqIn) continue;
        if (!a.squeeze_end_tick(k) || !a.squeeze_anchor_tick(k))
            return "SqIn without its SqIn step";
        if (!a.sqinouts[k].transfer) return "SqIn transfer unknown";
        if (!good(*a.sqinouts[k].transfer)) return "SqIn transfer not a positive finite number";
    }
    return {};
}

}  // namespace corpus

#endif  // HYDRA_TESTS_CORPUS_UTIL_H
