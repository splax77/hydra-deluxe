// Analysis orchestration: one run at the chosen SP cap. Discovery and the
// batch thread pool live in app/analysis; this is only what produces a
// record for one chart.

#ifndef HYDRA_SEARCH_PATHER_H
#define HYDRA_SEARCH_PATHER_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "parse/song.h"
#include "search/engine.h"
#include "search/graph.h"

namespace hydra {

// The depth the search and the app start from: how many scores (or points)
// below the best a path may sit and still be kept. app::Settings reads it too.
constexpr int kDefaultDepthValue = 4;

// Everything the search needs to know about one run. Mirrors the user's
// settings, but holds only what reaches the search (the chart-mode flags
// are parse-time and live on app::AnalysisSettings).
struct SearchSettings {
    DepthMode depth_mode = DepthMode::Scores;
    int depth_value = kDefaultDepthValue;
    std::optional<double> ms_filter;
    // The SP meter ceiling in bars (4 = Clone Hero's rule).
    int sp_cap = kCloneHeroSpCap;
    // Score fills by Clone Hero 1.0's spawn deadline instead of 1.1's flat 4
    // beats (FillDeadlineRule in search/graph.h). The app's "1.0 fills"
    // setting or hydra_batch --legacy-fills; the result is filed under it
    // (store::Lens::legacy_fills, docs/adr/0010).
    bool legacy_fill_deadline = false;
    // The user's rule choices (hydra_rules.ini). Defaults are today's rules.
    core::Rules rules = core::default_rules();
};

// The settings as one line of text, naming every field that changes a stored
// analysis. The test cache (corpus::analyzed) keys its analyses by it. A new
// SearchSettings field that changes the result must be added here.
std::string settings_key(const SearchSettings& settings);

// A points band wide enough to keep every path: a real chart scores a few
// million at most. The band is compared as `score + depth_value < best` in
// int64 arithmetic, so it cannot overflow. search_target uses it, and so do
// the tests that ask the engine to keep everything (audit R7.29).
constexpr int kKeepEveryPathBand = 1'000'000'000;

// How tall to build the search graph: the SP cap, but never more levels than
// the song has phrases to bank, and never fewer than one.
int graph_build_cap(int sp_cap, int sp_phrase_count);

// The best all-0 path over an already-built graph: the highest-scoring path
// that is all-0 (Path::is_allzero) and needs no timing (Path::needs_timing,
// applied in the engine through EngineOptions::no_timing), with its tied
// variations as variants. Empty when the chart offers no such
// path. This is a second, constrained search because the main search keeps
// paths by score band and drops the all-0 path when it scores below the band.
// It has no activation branching, so it is far cheaper than the main search.
std::vector<Path> search_allzero(const ScoreGraph& graph,
                                 const std::function<void(float)>& on_progress = {});

// The engine options search_allzero runs with: no skips and no timing, every
// other knob at EngineOptions' default (score depth 0: the top score and its
// ties).
EngineOptions allzero_options();

// The engine's own pricing of one specific path: activate at exactly
// `act_ticks` (node ticks of the activation fills, ascending) and at no other
// fill. Every squeeze variant of that path comes back, best score first, with
// deact_tick / sp_meter / skips / sqinouts stamped by the engine exactly as a
// normal search would stamp them. Every returned path, tied variants
// included, has exactly the named activations; the rest are dropped
// (keep_target_paths, decision D45 and its addendum). Empty when no path
// realizes the set (an activation under kSpActivationBars, a fill it cannot
// spawn in time, a tick that is not a fill node). The graph is built at
// graph_build_cap, as the main search builds it. settings.depth_* and
// ms_filter are ignored: the search keeps everything and applies no timing
// filter, because the caller asked for this path, not the best one.
// `promoted`, when given, is filled as keep_target_paths fills it.
std::vector<Path> search_target(const Song& song, const SearchSettings& settings,
                                const std::vector<int64_t>& act_ticks,
                                std::vector<bool>* promoted = nullptr);

// search_target's filter over the engine's paths (D45 and its addendum):
// keeps only the paths, tied variants included, whose activations are
// exactly `ticks` (ascending, no repeats). A kept path's tied variant that
// missed one is dropped; its own qualifying variants are rebuilt standalone
// and stay tied under the kept path, sharing nothing with it. When a root is
// dropped, its qualifying variants are rebuilt standalone: the first leads
// and the rest are its tied variants, in the engine's order. `promoted`,
// when given, gets one entry per returned path: true when that path is such
// a rebuilt variant. The search folded it, so it is not an unfolded root.
std::vector<Path> keep_target_paths(std::vector<Path> paths, const std::vector<int64_t>& ticks,
                                    std::vector<bool>* promoted = nullptr);

// Full analysis for one chart: one pass at settings.sp_cap bars (4 is Clone
// Hero's rule; any other number is a what-if). Throws std::logic_error when
// the song has no notes, because that is a caller bug: callers check first
// with require_notes (song.h), which throws the sentence the user sees.
HydraRecord analyze_chart(const Song& song, const SearchSettings& settings,
                          const std::function<void(float)>& on_progress = {});

}  // namespace hydra

#endif  // HYDRA_SEARCH_PATHER_H
