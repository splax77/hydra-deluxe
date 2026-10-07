#include "search/pather.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "search/engine.h"
#include "search/graph.h"

namespace hydra {

namespace {

HydraRecord read(const ScoreGraph& graph, DepthMode depth_mode, int depth_value,
                 std::optional<double> ms_filter,
                 const std::function<void(float)>& on_progress) {
    HydraRecord record;
    record.ms_limit = ms_filter;
    record.multsqueezes = graph.multsqueezes();
    EngineOptions options;
    options.depth_mode = depth_mode;
    options.depth_value = depth_value;
    options.ms_filter = ms_filter;
    record.paths = run_search(graph, options, on_progress);
    return record;
}

// The share of the progress bar the main search owns. The all-0 pass has no
// activation branching, so it finishes in a small fraction of the time; it gets
// the tail so the bar still moves while it runs and Cancel still has a tick to
// unwind from.
constexpr float kMainProgressShare = 0.9f;

std::function<void(float)> scaled_progress(const std::function<void(float)>& cb,
                                           float lo, float hi) {
    if (!cb) return {};
    return [cb, lo, hi](float f) { cb(lo + f * (hi - lo)); };
}

// Run the all-0 pass over an already-built graph and hang the result on the
// record.
void attach_allzero(const ScoreGraph& graph, HydraRecord& record,
                    const std::function<void(float)>& on_progress) {
    // Nothing to add when the optimal path is itself an all-0 path inside the
    // all-0 pass's limit: it already answers the question, and it is already
    // at the top of the list. This is the same test search_allzero's paths
    // pass (Path::is_allzero, and the limit allzero_options sets), so this
    // shortcut and the display's identity dedupe agree by construction.
    if (!record.paths.empty()) {
        const Path& best = record.best_path();
        if (best.is_allzero() && best.within_ms_limit(*allzero_options().ms_filter)) return;
    }
    try {
        record.allzero_paths = search_allzero(graph, on_progress);
    } catch (const std::exception&) {
        // A broken search state is worth losing the section over, not the whole
        // analysis. Cancel unwinds through AnalysisCancelled, which does not
        // derive from std::exception, so it still propagates.
        record.allzero_paths.clear();
    }
}

}  // namespace

std::string settings_key(const SearchSettings& s) {
    std::ostringstream k;
    k.precision(17);
    k << s.sp_cap << '|';
    k << static_cast<int>(s.depth_mode) << '|' << s.depth_value << '|';
    if (s.ms_filter) k << *s.ms_filter;
    else k << "none";
    k << '|';
    k << s.legacy_fill_deadline << '|' << s.rules.fingerprint() << '|';
    return k.str();
}

EngineOptions allzero_options() {
    EngineOptions options;  // score depth 0: only the top score, plus its ties
    options.no_skips = true;
    // The 0 ms limit is the point of the section (D85): every squeeze and
    // required early fill on the path is 0 ms or easier. It is a requirement,
    // and it ignores the user's Path limit.
    options.ms_filter = 0.0;
    options.hard_ms_filter = true;
    return options;
}

std::vector<Path> search_allzero(const ScoreGraph& graph,
                                 const std::function<void(float)>& on_progress) {
    // depth_value 0 keeps only the top score; its tied peers still merge into
    // variants (up to Rules::max_tied_paths), which is where the E / + / - variations
    // of one all-0 path come from. The 0 ms limit (allzero_options) is applied
    // hard. A soft filter only prefers paths inside the limit and still
    // reports an over-limit one while nothing outscores it, which in a
    // no-skips search (a tiny candidate set, usually one path per group)
    // meant the section routinely showed a path needing hundreds of ms.
    std::vector<Path> paths;
    try {
        paths = run_search(graph, allzero_options(), on_progress);
    } catch (const std::runtime_error&) {
        // The hard limit can empty the frontier: this chart offers no all-0
        // path inside 0 ms. run() reports that the same way it reports a
        // broken state, so both end here as "no all-0 path". Cancel unwinds
        // through its own non-std::exception type and still propagates.
        return {};
    }

    // Only all-0 paths stay (Path::is_allzero). That also drops the single
    // path with no activations the search returns when a chart refuses every
    // activation opportunity (the early fill can never be summoned in time).
    paths.erase(std::remove_if(paths.begin(), paths.end(),
                               [](const Path& p) { return !p.is_allzero(); }),
                paths.end());
    return paths;
}

namespace {

template <class Keep>
void keep_qualifying_variants(Path& p, const Keep& keep);

// keep_target_paths' rescue for a dropped path (D45 addendum): its tied
// variants, and theirs under a dropped variant, that pass `keep`, appended
// to `out` in the search's order. Each comes out standalone: a variant
// stores only the activations before its fold and reads the rest from its
// parent (variant_tail), so its whole walk is copied into its own list and
// the link to the parent cleared. Its own tied variants stay with it, kept
// by the same rule; their tails index its walk, which is unchanged.
template <class Keep>
void rescue_variants(Path& dropped, const Keep& keep, std::vector<Path>& out) {
    for (Path& v : dropped.variants) {
        if (!keep(v)) {
            rescue_variants(v, keep, out);
            continue;
        }
        v.activations = v.all_activations();
        v.variant_tail.clear();
        v.var_point.reset();
        keep_qualifying_variants(v, keep);
        out.push_back(std::move(v));
    }
}

// Appends rescued (standalone) paths to `into`, the list that holds
// `parent`'s tied variants, in order, each sharing nothing with `parent`: a
// variant shares its parent's walk from var_point on, and pointing past
// `parent`'s walk shares nothing, so each keeps its whole walk as its own.
// The one place a rescued path is marked "shares nothing".
void adopt_unshared(const Path& parent, std::vector<Path> rescued, std::vector<Path>& into) {
    const int past_walk = static_cast<int>(parent.walk_activations().size());
    for (Path& r : rescued) {
        r.var_point = past_walk;
        into.push_back(std::move(r));
    }
}

// Under a kept path, only the tied variants that pass `keep` stay (D45
// addendum). A kept variant stays as folded, its own variants kept by the
// same rule. A dropped variant's passing variants are rescued and stay
// under `p`, in the search's order, sharing nothing with it.
template <class Keep>
void keep_qualifying_variants(Path& p, const Keep& keep) {
    std::vector<Path> stay;
    for (Path& v : p.variants) {
        if (keep(v)) {
            keep_qualifying_variants(v, keep);
            stay.push_back(std::move(v));
            continue;
        }
        std::vector<Path> rescued;
        rescue_variants(v, keep, rescued);
        adopt_unshared(p, std::move(rescued), stay);
    }
    p.variants = std::move(stay);
}

// keep_target_paths' filter for any test a path must pass: every returned
// path, tied variants included, passes `keep` (D45 and its addendum).
template <class Keep>
std::vector<Path> keep_paths_where(std::vector<Path> paths, const Keep& keep,
                                   std::vector<bool>* promoted) {
    std::vector<Path> kept;
    if (promoted) promoted->clear();
    for (Path& p : paths) {
        if (keep(p)) {
            keep_qualifying_variants(p, keep);
            p.recount_tied_paths();
            kept.push_back(std::move(p));
            if (promoted) promoted->push_back(false);
            continue;
        }
        // A dropped root's qualifying variants become a result of their own:
        // the first leads, the rest are its tied variants, in the search's
        // order, each sharing nothing with the lead (as above).
        std::vector<Path> rescued;
        rescue_variants(p, keep, rescued);
        if (rescued.empty()) continue;
        Path lead = std::move(rescued.front());
        rescued.erase(rescued.begin());
        adopt_unshared(lead, std::move(rescued), lead.variants);
        lead.recount_tied_paths();
        kept.push_back(std::move(lead));
        if (promoted) promoted->push_back(true);
    }
    return kept;
}

// Whether `p` activated at exactly `ticks`, in order, and nowhere else.
bool took_exactly(const Path& p, const std::vector<int64_t>& ticks) {
    const ActivationWalk acts = p.walk_activations();
    if (acts.size() != ticks.size()) return false;
    for (size_t i = 0; i < acts.size(); ++i)
        if (acts[i].timecode.ticks() != ticks[i]) return false;
    return true;
}

}  // namespace

std::vector<Path> keep_target_paths(std::vector<Path> paths, const std::vector<int64_t>& ticks,
                                    std::vector<bool>* promoted) {
    // A tick the search never met as an activation opportunity -- not a fill
    // node at all, or one the path was already under Star Power for -- does
    // not empty the frontier: that path just quietly comes back with fewer
    // activations than asked for.
    return keep_paths_where(
        std::move(paths), [&ticks](const Path& p) { return took_exactly(p, ticks); }, promoted);
}

namespace {

// The graph a target search runs on: as tall as the main search builds it
// (decision D45).
ScoreGraph target_graph(const Song& song, const SearchSettings& settings) {
    return ScoreGraph(song,
                      std::optional<int>(graph_build_cap(settings.sp_cap, song.sp_phrase_count())),
                      fill_rule_for(settings.legacy_fill_deadline), settings.rules);
}

// Whether the pinned windows of `p` ended as pinned: on their deactivation
// node, and, when the pin says how, on its squeeze-out or plainly.
bool ended_as_pinned(const Path& p, const std::vector<PinnedWindow>& windows) {
    const ActivationWalk acts = p.walk_activations();
    for (size_t i = 0; i < windows.size() && i < acts.size(); ++i) {
        const PinnedWindow& w = windows[i];
        if (!w.deact_tick) break;
        if (acts[i].deact_tick() != w.deact_tick) return false;
        if (w.check_sqout && acts[i].sqout_tick != w.squeezed_out) return false;
    }
    return true;
}

// The one target run both search_target entry points make, over a graph
// already built: activate at exactly the windows' act ticks (sorted, pinned
// ones first, as search_target over windows checks), each pinned window
// ending on its deactivation node. Only the paths that took exactly those
// activations and ended the pinned windows as pinned stay; none left means
// the windows are not realizable.
std::vector<Path> run_target(const ScoreGraph& graph, const std::vector<PinnedWindow>& windows,
                             std::vector<bool>* promoted) {
    std::vector<int64_t> ticks;
    EngineOptions options;
    for (const PinnedWindow& w : windows) {
        ticks.push_back(w.act_tick);
        if (w.deact_tick) options.target_deact_ticks.push_back(*w.deact_tick);
    }

    // The caller named the path, so nothing may prune it: the widest possible
    // points band keeps every survivor, and no timing filter is applied.
    std::vector<Path> paths;
    try {
        options.depth_mode = DepthMode::Points;
        options.depth_value = kKeepEveryPathBand;
        options.target_act_ticks = ticks;
        paths = run_search(graph, options);
    } catch (const std::runtime_error&) {
        // The frontier emptied: these windows are not realizable on this
        // chart. That is the normal failure for a targeted search, not a bug.
        if (promoted) promoted->clear();
        return {};
    }
    return keep_paths_where(
        std::move(paths),
        [&](const Path& p) { return took_exactly(p, ticks) && ended_as_pinned(p, windows); },
        promoted);
}

// The first `k` windows.
std::vector<PinnedWindow> prefix_of(const std::vector<PinnedWindow>& windows, size_t k) {
    return std::vector<PinnedWindow>(windows.begin(), windows.begin() + (std::ptrdiff_t)k);
}

}  // namespace

std::vector<Path> search_target(const Song& song, const SearchSettings& settings,
                                const std::vector<int64_t>& act_ticks,
                                std::vector<bool>* promoted) {
    std::vector<int64_t> ticks = act_ticks;
    std::sort(ticks.begin(), ticks.end());
    ticks.erase(std::unique(ticks.begin(), ticks.end()), ticks.end());
    std::vector<PinnedWindow> windows;
    for (const int64_t t : ticks) windows.push_back(PinnedWindow{t});
    return run_target(target_graph(song, settings), windows, promoted);
}

TargetResult search_target(const Song& song, const SearchSettings& settings,
                           std::vector<PinnedWindow> windows) {
    std::sort(windows.begin(), windows.end(),
              [](const PinnedWindow& a, const PinnedWindow& b) { return a.act_tick < b.act_tick; });
    for (size_t i = 0; i < windows.size(); ++i) {
        const PinnedWindow& w = windows[i];
        const std::string at = "target window at tick " + std::to_string(w.act_tick);
        if (i > 0 && windows[i - 1].act_tick == w.act_tick)
            throw std::invalid_argument(at + " is named twice");
        if (i > 0 && w.deact_tick && !windows[i - 1].deact_tick)
            throw std::invalid_argument(at + " pins its end after a window that does not");
        if (w.check_sqout && !w.deact_tick)
            throw std::invalid_argument(at + " checks its squeeze-out but pins no end");
        if (w.deact_tick && *w.deact_tick < w.act_tick)
            throw std::invalid_argument(at + " ends before it activates");
    }

    const ScoreGraph graph = target_graph(song, settings);
    TargetResult out;
    out.paths = run_target(graph, windows, &out.promoted);
    out.realized_prefix = windows.size();
    if (!out.paths.empty() || windows.empty()) return out;

    // The whole list failed. Some path realizes no window at all (decline
    // every fill), and realizing the first k + 1 windows realizes the first
    // k (decline the last one), so the realized prefix lengths run from 0 up
    // to some k and the binary search finds that k. Window k is the one no
    // path realizes.
    size_t realized = 0, failed = windows.size();
    while (failed - realized > 1) {
        const size_t mid = realized + (failed - realized) / 2;
        if (run_target(graph, prefix_of(windows, mid), nullptr).empty()) failed = mid;
        else realized = mid;
    }
    out.realized_prefix = realized;
    const PinnedWindow& broke = windows[realized];
    out.failed_tick = broke.act_tick;

    // Which part of it broke: the activation itself (pinned alone), else the
    // end, else the squeeze-out (the end pinned, how it ended not checked).
    std::vector<PinnedWindow> probe = prefix_of(windows, realized + 1);
    probe.back().deact_tick.reset();
    probe.back().check_sqout = false;
    if (!broke.deact_tick || run_target(graph, probe, nullptr).empty()) {
        out.failed_reason = "activation";
        return out;
    }
    if (!broke.check_sqout) {
        out.failed_reason = "window_end";
        return out;
    }
    probe.back().deact_tick = broke.deact_tick;
    out.failed_reason = run_target(graph, probe, nullptr).empty() ? "window_end" : "sqout";
    return out;
}

namespace {

// One replay window as a full pin, its squeeze-out on `squeezed_out`.
PinnedWindow full_pin(const ReplayWindow& w, std::optional<int64_t> squeezed_out) {
    PinnedWindow p;
    p.act_tick = w.act_tick;
    p.deact_tick = w.deact_tick;
    p.check_sqout = w.from_record || squeezed_out.has_value();
    p.squeezed_out = squeezed_out;
    return p;
}

}  // namespace

std::vector<PinnedWindow> pinned_windows(const Song& song,
                                         const std::vector<ReplayWindow>& windows) {
    std::vector<PinnedWindow> out;
    for (const ReplayWindow& w : windows) {
        std::optional<int64_t> squeezed_out = w.sqout_tick;
        if (!squeezed_out && w.sqout_offset_ms) squeezed_out = resolve_sqout_note(song, w).tick;
        out.push_back(full_pin(w, squeezed_out));
    }
    return out;
}

std::vector<PinnedWindow> pinned_windows(const Path& path) {
    const std::vector<ReplayWindow> windows = windows_for_path(path);
    const size_t acts = path.walk_activations().size();
    if (windows.size() != acts)
        throw std::invalid_argument("only " + std::to_string(windows.size()) + " of " +
                                    std::to_string(acts) +
                                    " activations have a deactivation node to pin");
    // A stored window names its squeeze-out by its chord (windows_for_path).
    std::vector<PinnedWindow> out;
    for (const ReplayWindow& w : windows) out.push_back(full_pin(w, w.sqout_tick));
    return out;
}

std::string full_pin_mismatch(const Song& song, const SearchSettings& settings,
                              const Path& path) {
    const std::vector<PinnedWindow> pins = pinned_windows(path);
    const TargetResult t = search_target(song, settings, pins);
    if (t.paths.empty())
        return "no path came back: the window at tick " +
               std::to_string(t.failed_tick.value_or(-1)) + " broke (" + t.failed_reason + ")";
    if (t.paths.size() != 1) return std::to_string(t.paths.size()) + " paths came back, not one";
    const Path& got = t.paths.front();
    if (!got.variants.empty())
        return "it came back with " + std::to_string(got.variants.size()) + " tied variant(s)";

    std::string diffs;
    const auto differ = [&diffs](const std::string& what, const std::string& got_s,
                                 const std::string& want_s) {
        diffs += (diffs.empty() ? "" : ", ") + what + " " + got_s + " vs " + want_s;
    };
    if (got.pathstring() != path.pathstring())
        differ("path", "'" + got.pathstring() + "'", "'" + path.pathstring() + "'");
    if (got.totalscore() != path.totalscore())
        differ("total", std::to_string(got.totalscore()), std::to_string(path.totalscore()));
    const ActivationWalk mine = path.walk_activations();
    const ActivationWalk theirs = got.walk_activations();
    const std::vector<PinnedWindow> got_pins = pinned_windows(got);
    for (size_t i = 0; i < pins.size() && i < got_pins.size(); ++i) {
        const std::string at = "window " + std::to_string(pins[i].act_tick) + " ";
        const auto tick = [](const std::optional<int64_t>& t) {
            return std::to_string(t.value_or(-1));
        };
        if (got_pins[i].act_tick != pins[i].act_tick)
            differ(at + "activation", std::to_string(got_pins[i].act_tick),
                   std::to_string(pins[i].act_tick));
        if (got_pins[i].deact_tick != pins[i].deact_tick)
            differ(at + "end", tick(got_pins[i].deact_tick), tick(pins[i].deact_tick));
        if (got_pins[i].squeezed_out != pins[i].squeezed_out)
            differ(at + "squeeze-out", tick(got_pins[i].squeezed_out), tick(pins[i].squeezed_out));
        if (theirs[i].sp_meter() != mine[i].sp_meter())
            differ(at + "SP meter", std::to_string(theirs[i].sp_meter()),
                   std::to_string(mine[i].sp_meter()));
        if (theirs[i].skips() != mine[i].skips())
            differ(at + "skips", std::to_string(theirs[i].skips()),
                   std::to_string(mine[i].skips()));
    }
    if (got_pins.size() != pins.size())
        differ("windows", std::to_string(got_pins.size()), std::to_string(pins.size()));
    return diffs;
}

int graph_build_cap(int sp_cap, int sp_phrase_count) {
    return std::max(max_sp_bars(sp_cap, sp_phrase_count), 1);
}

namespace {

// One pathing run with a given SP meter ceiling. build_cap, when set, is the
// ceiling the graph is actually built at (the record still reports sp_cap).
// want_allzero also runs search_allzero over the same graph and stores it in
// the record's allzero_paths.
HydraRecord analyze_at_cap(const Song& song, int sp_cap, DepthMode depth_mode,
                           int depth_value, std::optional<double> ms_filter,
                           std::optional<int> build_cap, bool legacy_fills,
                           const core::Rules& rules,
                           bool want_allzero = false,
                           const std::function<void(float)>& on_progress = {}) {
    std::optional<int> cap = build_cap.has_value() ? build_cap
                                                   : std::optional<int>(sp_cap);
    ScoreGraph graph(song, cap, fill_rule_for(legacy_fills), rules);
    const bool split = want_allzero && static_cast<bool>(on_progress);
    HydraRecord record = read(
        graph, depth_mode, depth_value, ms_filter,
        split ? scaled_progress(on_progress, 0.0f, kMainProgressShare) : on_progress);
    record.sp_cap = sp_cap;
    // The graph is still alive here, so the all-0 pass reuses it instead of
    // paying for a second build.
    if (want_allzero)
        attach_allzero(graph, record,
                       split ? scaled_progress(on_progress, kMainProgressShare, 1.0f)
                             : on_progress);
    return record;
}

}  // namespace

HydraRecord analyze_chart(const Song& song, const SearchSettings& settings,
                          const std::function<void(float)>& on_progress) {
    // Callers check for notes first (require_notes in song.h), because only
    // they know the difficulty and drum mode the user's sentence names. Reaching
    // here with an empty song is a caller bug, not a chart problem.
    if (song.is_empty())
        throw std::logic_error("analyze_chart was given a song with no notes; "
                               "check it with require_notes first");

    // One pass at the chosen ceiling, Clone Hero's 4 bars included. The graph
    // is only built as tall as the song has phrases to bank -- no run can
    // exceed that -- so a huge cap on a short song stays cheap, and a 4-bar
    // graph built lower stores the same bytes (test "a 4-bar graph built at
    // the song's phrase count stores the same paths").
    const int build_cap = graph_build_cap(settings.sp_cap, song.sp_phrase_count());
    HydraRecord record =
        analyze_at_cap(song, settings.sp_cap, settings.depth_mode, settings.depth_value,
                       settings.ms_filter, build_cap, settings.legacy_fill_deadline,
                       settings.rules, /*want_allzero=*/true, on_progress);
    record.rules_fingerprint = settings.rules.fingerprint();
    record.legacy_fills = settings.legacy_fill_deadline;
    return record;
}

}  // namespace hydra
