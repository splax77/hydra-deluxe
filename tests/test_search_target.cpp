// search_target, keep_target_paths and keep_prefix_paths: a targeted search
// keeps exactly the engine's paths that took every named activation, and a
// full pin of a stored path comes back alone, or names the window that broke
// and why. Each case checks the filter against the engine's own answer before
// it filtered (unfiltered_target), on the fuzzed fast-tempo charts
// (fast_tempo_charts.h) or on the corpus. The free-after cases
// (EngineOptions::free_after, keep_prefix_paths) follow the pins and then
// search freely, on the corpus.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/user_messages.h"
#include "corpus_util.h"
#include "core/model.h"
#include "core/replay.h"
#include "fast_tempo_charts.h"
#include "parse/song.h"
#include "pathbuilder/pb_coach.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

// Default settings at a cap, top 40 scores, no ms limit; a root's tied
// variants; a path's activation ticks (record_fixtures.h).
using test::scores_settings;
using test::collect_tied;
using test::act_ticks;
using test::roots_and_tied;

// The fuzzed charts and the analysis wrapper (fast_tempo_charts.h).
using test::FuzzChart;
using test::fuzz_chart;
using test::analyzes;

// Fuzz seed 5's two fill ticks, the activations its targeted cases name.
const std::vector<int64_t> kSeed5Fills = {3168, 12864};

// search_target's answer as text: each root's path string and total, then
// its tied variants' in brackets, in the order it returns them.
std::string target_text(const std::vector<Path>& kept) {
    std::ostringstream o;
    for (const Path& p : kept) {
        o << p.pathstring() << ' ' << p.totalscore();
        std::vector<const Path*> tied;
        collect_tied(p, tied);
        if (!tied.empty()) {
            o << " [";
            for (size_t i = 0; i < tied.size(); ++i)
                o << (i ? "; " : "") << tied[i]->pathstring() << ' ' << tied[i]->totalscore();
            o << ']';
        }
        o << " | ";
    }
    return o.str();
}

// The corpus chart Mutemath "Allies" and its first stored path (four windows).
struct AlliesBest {
    const Song* song;
    const Path* best;
};
AlliesBest allies_best(const app::AnalysisSettings& cfg) {
    const std::string chart = corpus::chart_named("Mutemath - Allies");
    REQUIRE_FALSE(chart.empty());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.all_paths().empty());
    return {&corpus::song(chart, cfg.reading()), rec.all_paths().front()};
}

// A chart's text read as the app reads a .chart file's bytes.
Song chart_song(const std::string& text) { return load_songtext_chart(text); }

// Fuzz seed 5 with one more note after its last phrase chord (at 22848), a
// line a chart editor saves like any other note.
Song seed5_with_note_after_last_phrase(const FuzzChart& fc) {
    std::string text = fc.text;
    const std::string last_phrase = "  22848 = N 1 0\n";
    const size_t at = text.find(last_phrase);
    REQUIRE(at != std::string::npos);
    text.insert(at + last_phrase.size(), "  23040 = N 1 0\n");
    return chart_song(text);
}

// One corpus chart's file and its song.
struct CorpusSong {
    std::string chart;
    const Song* song;
};

// Every corpus chart's song under `cfg`'s reading. A chart whose song has no
// notes fails here, named, rather than being skipped, so a parser that empties
// a chart cannot shrink a corpus walk unseen.
std::vector<CorpusSong> corpus_songs(const app::AnalysisSettings& cfg) {
    std::vector<CorpusSong> out;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        CAPTURE(chart);
        CHECK_FALSE(song.is_empty());
        if (!song.is_empty()) out.push_back({chart, &song});
    }
    REQUIRE_FALSE(out.empty());
    return out;
}

// The engine's own paths for a search told to activate at exactly `ticks`,
// before search_target filters them: the same run on the same graph
// (search_graph), every survivor kept (wide_search) and no timing filter.
std::vector<Path> unfiltered_target(const Song& song, const SearchSettings& settings,
                                    const std::vector<int64_t>& ticks) {
    const ScoreGraph graph = search_graph(song, settings);
    EngineOptions options = test::wide_search();
    options.target_act_ticks = ticks;
    return run_search(graph, options);
}

// One path as text for comparing two searches: its path string, total and
// every stored window fact (windows_text, which names each window's ticks).
std::string path_key(const Path& p) {
    return p.pathstring() + ' ' + std::to_string(p.totalscore()) + ' ' + test::windows_text(p);
}

// Whether `roots`, tied variants included, hold the path `key` names.
bool holds_path(const std::vector<Path>& roots, const std::string& key) {
    for (const Path* p : roots_and_tied(roots))
        if (path_key(*p) == key) return true;
    return false;
}

// The activation rules the filters keep by: exactly `want`, or starting with it.
auto exactly_ticks(const std::vector<int64_t>& want) {
    return [want](const std::vector<int64_t>& t) { return t == want; };
}
auto beginning_with_ticks(const std::vector<int64_t>& pin) {
    return [pin](const std::vector<int64_t>& t) {
        return t.size() >= pin.size() && std::equal(pin.begin(), pin.end(), t.begin());
    };
}

// Whether some root of `roots` whose ticks pass `takes` (or, with `root_took`
// false, fail it) has a tied variant whose ticks pass it (`variant_took`) or
// fail it.
template <class Takes>
bool tied_case(const std::vector<Path>& roots, const Takes& takes, bool root_took,
               bool variant_took) {
    for (const Path& r : roots) {
        if (takes(act_ticks(r)) != root_took) continue;
        std::vector<const Path*> tied;
        collect_tied(r, tied);
        for (const Path* v : tied)
            if (takes(act_ticks(*v)) == variant_took) return true;
    }
    return false;
}

// `kept`, what keep_target_paths or keep_prefix_paths (or search_target, which
// runs keep_target_paths) returned from the engine's paths `raw`, checked
// against `takes`, the activation rule it keeps by:
// - every kept path, tied variants included, passes `takes`, and the kept
//   paths are exactly the paths of `raw` that pass it, none lost, none added;
// - the results not promoted are the roots of `raw` that pass, in order;
// - a promoted result is a lead of its own: no variant point, no tail;
// - each result's stored tied-path count is current: the filter recounted it
//   after it dropped variants (Path::recount_tied_paths, the count's owner).
template <class Takes>
void check_kept_from(const std::vector<Path>& raw, const std::vector<Path>& kept,
                     const std::vector<bool>& promoted, const Takes& takes) {
    std::multiset<std::string> want;
    std::multiset<std::string> got;
    for (const Path* p : roots_and_tied(raw))
        if (takes(act_ticks(*p))) want.insert(path_key(*p));
    for (const Path* p : roots_and_tied(kept)) {
        CAPTURE(p->pathstring());
        CHECK(takes(act_ticks(*p)));
        got.insert(path_key(*p));
    }
    CHECK(got == want);

    REQUIRE(promoted.size() == kept.size());
    std::vector<std::string> raw_roots;
    for (const Path& r : raw)
        if (takes(act_ticks(r))) raw_roots.push_back(path_key(r));
    std::vector<std::string> kept_roots;
    for (size_t i = 0; i < kept.size(); ++i) {
        const Path& k = kept[i];
        CAPTURE(k.pathstring());
        Path recounted = k;
        CHECK(recounted.recount_tied_paths() == k.tied_pathcount());
        if (!promoted[i]) {
            kept_roots.push_back(path_key(k));
            continue;
        }
        CHECK_FALSE(k.var_point.has_value());
        CHECK(k.variant_tail.empty());
    }
    CHECK(kept_roots == raw_roots);
}

// The first result search_target promoted, or nullptr.
const Path* first_promoted(const std::vector<Path>& kept, const std::vector<bool>& promoted) {
    for (size_t i = 0; i < kept.size() && i < promoted.size(); ++i)
        if (promoted[i]) return &kept[i];
    return nullptr;
}

}  // namespace

// A targeted search drops only the paths that came back without one of the
// named activations, and keeps the ones that took them all. On fuzz seed 5,
// told to activate at its two fills, the engine also returns paths that take
// only the first (Star Power still runs at the second's fill); search_target
// drops those and keeps every other path the engine returned.
TEST_CASE("search_target: a path missing a named activation is dropped, the rest kept") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = chart_song(fc.text);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const auto takes = exactly_ticks(kSeed5Fills);

    // The engine did return a path that missed one: the case this test is for.
    const std::vector<Path> raw = unfiltered_target(song, cfg, kSeed5Fills);
    bool missed_one = false;
    for (const Path* p : roots_and_tied(raw)) missed_one = missed_one || !takes(act_ticks(*p));
    REQUIRE(missed_one);

    std::vector<bool> promoted;
    const std::vector<Path> kept = search_target(song, cfg, kSeed5Fills, &promoted);
    REQUIRE_FALSE(kept.empty());
    check_kept_from(raw, kept, promoted, takes);

    // A tick that is no fill node is still unrealizable: nothing comes back.
    CHECK(search_target(song, cfg, {kSeed5Fills[0], kSeed5Fills[1] + 1}).empty());
}

// A dropped root's variants: the engine can fold a path that takes every
// named activation under a tied root that missed one. search_target drops
// that root but keeps the variant, as a result of its own (the first such
// variant leads, the rest stay its tied variants). The chart is fuzz seed 5
// with one more note after its last phrase, where the engine makes that fold.
TEST_CASE("search_target: a variant that took every named activation outlives its dropped root") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = seed5_with_note_after_last_phrase(fc);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const auto takes = exactly_ticks(kSeed5Fills);

    const std::vector<Path> raw = unfiltered_target(song, cfg, kSeed5Fills);
    REQUIRE(tied_case(raw, takes, false, true));

    std::vector<bool> was_promoted;
    const std::vector<Path> kept = search_target(song, cfg, kSeed5Fills, &was_promoted);
    check_kept_from(raw, kept, was_promoted, takes);
    const Path* promoted = first_promoted(kept, was_promoted);
    REQUIRE(promoted != nullptr);

    // Its windows are its own, not the dropped root's: the same as the full
    // search stores for that path, where it is a tied variant read through
    // its root.
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    const Path* full = test::path_named(rec.all_paths(), promoted->pathstring());
    REQUIRE(full != nullptr);
    CHECK(full->var_point.has_value());
    CHECK(full->totalscore() == promoted->totalscore());
    CHECK(test::windows_text(*promoted) == test::windows_text(*full));
}

// The lone-pricing helper (record_fixtures.h) on the fold above: the full
// search's copy of the variant search_target promoted would pass for a lone
// match, but the lone search folded it too, so it is no oracle. The helper
// reads search_target's promoted flag and reports it as tied under a root.
TEST_CASE("lone_pricing: a variant search_target promoted is tied under a root") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = seed5_with_note_after_last_phrase(fc);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);

    std::vector<bool> was_promoted;
    const std::vector<Path> kept = search_target(song, cfg, kSeed5Fills, &was_promoted);
    const Path* promoted = first_promoted(kept, was_promoted);
    REQUIRE(promoted != nullptr);
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    const Path* full = test::path_named(rec.all_paths(), promoted->pathstring());
    REQUIRE(full != nullptr);
    CHECK(test::lone_pricing(song, cfg, *full).tied_under_root);
}

// A kept root's variants: every tied path search_target returns has exactly
// the named activations. On fuzz seed 199, told to activate at three of its
// fills, the engine ties a path that misses one under a root that takes all
// three; search_target drops that variant and keeps its root.
TEST_CASE("search_target: a kept root's tied variant that missed a named activation is dropped") {
    const FuzzChart fc = fuzz_chart(199);
    const Song song = chart_song(fc.text);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const std::vector<int64_t> want = {4896, 12288, 21792};
    const auto takes = exactly_ticks(want);

    const std::vector<Path> raw = unfiltered_target(song, cfg, want);
    REQUIRE(tied_case(raw, takes, true, false));

    std::vector<bool> promoted;
    const std::vector<Path> kept = search_target(song, cfg, want, &promoted);
    REQUIRE_FALSE(kept.empty());
    check_kept_from(raw, kept, promoted, takes);
}

// keep_target_paths over the engine's own answer on every corpus chart: told
// to activate at the ticks of the chart's best stored path, search_target
// keeps exactly the engine's paths, tied variants included, that took those
// ticks. Each kept root is the engine's, and each promoted result leads on
// its own (check_kept_from).
TEST_CASE("keep_target_paths: every kept path and tied variant took exactly the named ticks") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const CorpusSong& c : corpus_songs(cfg)) {
        CAPTURE(c.chart);
        const HydraRecord& rec = corpus::analyzed(c.chart, cfg);
        REQUIRE_FALSE(rec.paths.empty());
        const Path& best = rec.paths.front();
        CAPTURE(best.pathstring());
        const std::vector<int64_t> want = act_ticks(best);

        std::vector<bool> promoted;
        const std::vector<Path> kept = search_target(*c.song, cfg, want, &promoted);
        REQUIRE_FALSE(kept.empty());
        check_kept_from(unfiltered_target(*c.song, cfg, want), kept, promoted,
                        exactly_ticks(want));
    }
}

// Track R2: every corpus path, handed back to search_target as a full pin
// (pinned_windows: each window's activation, deactivation node and
// squeeze-out), comes back alone and stores what the search stored for it.
// full_pin_mismatch is the check hydra_replay selfcheck runs; windows_text
// adds every other stored fact of each window. Every corpus chart runs
// (corpus_songs), and every one stores at least its best path.
TEST_CASE("search_target: every corpus path pinned in full comes back alone, the same") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const CorpusSong& c : corpus_songs(cfg)) {
        const HydraRecord& rec = corpus::analyzed(c.chart, cfg);
        CAPTURE(c.chart);
        CHECK_FALSE(rec.paths.empty());
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            CHECK(full_pin_mismatch(*c.song, cfg, *p) == "");
            const TargetResult t = search_target(*c.song, cfg, pinned_windows(*p));
            CHECK(t.paths.size() == 1);
            if (t.paths.size() != 1) continue;
            CHECK(test::windows_text(t.paths[0]) == test::windows_text(*p));
        }
    }
}

// Track R2 on fuzz seed 5's two fills: a full pin of a path search_target
// kept there, a root with no tied variant, comes back alone. Its second
// window moved to an end no Star Power reaches, or told the other answer on
// how it ended, it comes back empty and names that window. Its reason is one
// the Path Builder reads (impossible_window_text), a different one for each
// part that broke, and the part it names is the one that broke: with the
// end pin taken off, the moved end comes back; with the squeeze-out check
// taken off, the other answer comes back and the moved end still fails.
TEST_CASE("search_target: a full pin no path realizes names its window and why") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = chart_song(fc.text);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    std::vector<bool> promoted;
    const std::vector<Path> kept = search_target(song, cfg, kSeed5Fills, &promoted);
    const Path* path = nullptr;
    for (size_t i = 0; i < kept.size() && !path; ++i)
        if (!promoted[i] && kept[i].variants.empty()) path = &kept[i];
    REQUIRE(path != nullptr);
    CAPTURE(path->pathstring());
    const std::vector<PinnedWindow> pins = pinned_windows(*path);
    REQUIRE(pins.size() == kSeed5Fills.size());
    const size_t broken = pins.size() - 1;

    const TargetResult whole = search_target(song, cfg, pins);
    REQUIRE(whole.paths.size() == 1);
    CHECK(path_key(whole.paths[0]) == path_key(*path));
    CHECK(whole.realized_prefix == pins.size());
    CHECK_FALSE(whole.failed_tick.has_value());
    CHECK(whole.failed_reason.empty());

    // One tick after its own activation, no Star Power can end.
    std::vector<PinnedWindow> unreachable = pins;
    unreachable[broken].deact_tick = unreachable[broken].act_tick + 1;
    const TargetResult end = search_target(song, cfg, unreachable);
    CHECK(end.paths.empty());
    CHECK(end.realized_prefix == broken);
    CHECK(end.failed_tick == std::optional<int64_t>(pins[broken].act_tick));
    std::vector<PinnedWindow> end_unchecked = unreachable;
    end_unchecked[broken].check_sqout = false;
    CHECK(search_target(song, cfg, end_unchecked).paths.empty());
    std::vector<PinnedWindow> end_unpinned = unreachable;
    end_unpinned[broken].deact_tick.reset();
    end_unpinned[broken].check_sqout = false;
    CHECK_FALSE(search_target(song, cfg, end_unpinned).paths.empty());

    // The window's end as stored, but the other answer on how it ended: a
    // plain end where it squeezed out, or a squeeze-out on its own end node
    // where it ended plainly.
    std::vector<PinnedWindow> other_sqout = pins;
    REQUIRE(other_sqout[broken].check_sqout);
    REQUIRE(pins[broken].deact_tick.has_value());
    other_sqout[broken].squeezed_out =
        pins[broken].squeezed_out ? std::nullopt : pins[broken].deact_tick;
    const TargetResult sqout = search_target(song, cfg, other_sqout);
    CHECK(sqout.paths.empty());
    CHECK(sqout.realized_prefix == broken);
    CHECK(sqout.failed_tick == std::optional<int64_t>(pins[broken].act_tick));
    std::vector<PinnedWindow> sqout_unchecked = other_sqout;
    sqout_unchecked[broken].check_sqout = false;
    CHECK_FALSE(search_target(song, cfg, sqout_unchecked).paths.empty());

    // The activation itself broken, for the third reason: a tick one past
    // the window's fill node, which is no fill node.
    std::vector<PinnedWindow> no_fill = pins;
    no_fill[broken] = PinnedWindow{pins[broken].act_tick + 1};
    const TargetResult act = search_target(song, cfg, no_fill);
    CHECK(act.paths.empty());

    // Each reason is one the Path Builder puts in words, and no two parts
    // that broke give the same one.
    for (const std::string& reason : {end.failed_reason, sqout.failed_reason, act.failed_reason}) {
        CAPTURE(reason);
        CHECK(pathbuilder::impossible_window_text(reason, true) != app::kSomethingWentWrong);
    }
    CHECK(end.failed_reason != sqout.failed_reason);
    CHECK(end.failed_reason != act.failed_reason);
    CHECK(sqout.failed_reason != act.failed_reason);
}

// Activation pins alone ask what the tick list asks: search_target over them
// returns what search_target over the ticks returns (D45's answer above).
// D45's unrealizable set fails on 12865, a tick that is no fill node, found
// by the binary search over the prefix length.
TEST_CASE("search_target: activation pins return the tick search's paths and its failing tick") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = chart_song(fc.text);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);

    std::vector<bool> promoted;
    const std::vector<Path> by_ticks = search_target(song, cfg, {3168, 12864}, &promoted);
    const TargetResult by_pins = search_target(song, cfg, {PinnedWindow{3168}, PinnedWindow{12864}});
    CHECK(target_text(by_pins.paths) == target_text(by_ticks));
    CHECK(by_pins.promoted == promoted);
    CHECK(by_pins.realized_prefix == 2);

    const TargetResult bad =
        search_target(song, cfg, {PinnedWindow{3168}, PinnedWindow{12865}});
    CHECK(bad.paths.empty());
    CHECK(bad.realized_prefix == 1);
    CHECK(bad.failed_tick == std::optional<int64_t>(12865));
    CHECK(bad.failed_reason == "activation");
}

// The binary search over more than two windows, on corpus chart Mutemath
// "Allies" (its first stored path, four windows). One window's end is moved
// one tick past its activation, where no Star Power ends: the last window,
// then a middle one. The answers are pinned as read from one run.
TEST_CASE("search_target: the binary search names the last or a middle window that breaks") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const AlliesBest allies = allies_best(cfg);
    const Song& song = *allies.song;
    const std::vector<PinnedWindow> pins = pinned_windows(*allies.best);
    REQUIRE(pins.size() == 4);

    std::vector<PinnedWindow> last = pins;
    last[3].deact_tick = last[3].act_tick + 1;
    const TargetResult at_last = search_target(song, cfg, last);
    CHECK(at_last.paths.empty());
    CHECK(at_last.realized_prefix == 3);
    CHECK(at_last.failed_tick == std::optional<int64_t>(49920));
    CHECK(at_last.failed_reason == "window_end");

    std::vector<PinnedWindow> middle = pins;
    middle[1].deact_tick = middle[1].act_tick + 1;
    const TargetResult at_middle = search_target(song, cfg, middle);
    CHECK(at_middle.paths.empty());
    CHECK(at_middle.realized_prefix == 1);
    CHECK(at_middle.failed_tick == std::optional<int64_t>(22272));
    CHECK(at_middle.failed_reason == "window_end");
}

// EngineOptions::free_after with no tick pinned has no last pin, so every
// fill branches: on every corpus chart the search returns what the plain
// search at the same depth and Path limit returns.
TEST_CASE("free after: with nothing pinned the search is the plain search") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        ++charts;
        CAPTURE(chart);
        const ScoreGraph graph(song, kCloneHeroSpCap);
        EngineOptions plain = best_finish_options(cfg, {});
        plain.free_after = false;
        plain.target_act_ticks.reset();
        const std::vector<Path> want = run_search(graph, plain);
        const std::vector<Path> got = run_search(graph, best_finish_options(cfg, {}));
        CHECK(target_text(got) == target_text(want));
    }
    CHECK(charts > 0);
}

// Pinning the first window of a best path with four windows (Mutemath
// "Allies", as in the binary-search case above): fills before the pin are
// declined, the pin is taken, and the fills after it branch freely, so the
// paths that come back activate again after it.
TEST_CASE("free after: fills past the last pin branch, fills before it follow the pins") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const AlliesBest allies = allies_best(cfg);
    const Song& song = *allies.song;
    const std::vector<int64_t> best = act_ticks(*allies.best);
    REQUIRE(best.size() >= 3);

    const ScoreGraph graph(song, kCloneHeroSpCap);
    const std::vector<Path> got =
        run_search(graph, best_finish_options(cfg, {PinnedWindow{best.front()}}));
    REQUIRE_FALSE(got.empty());
    std::set<std::vector<int64_t>> afters;
    for (const Path* p : roots_and_tied(got)) {
        const std::vector<int64_t> ticks = act_ticks(*p);
        CAPTURE(p->pathstring());
        REQUIRE_FALSE(ticks.empty());
        CHECK(ticks.front() == best.front());
        afters.insert(std::vector<int64_t>(ticks.begin() + 1, ticks.end()));
    }
    // The fills after the pin branched: the paths activate after it, and not
    // all at the same fills.
    CHECK(afters.count({}) == 0);
    CHECK(afters.size() > 1);
}

// The Path limit never drops a window the caller pinned. The chart is the
// first corpus chart whose unlimited best path has a window that fails a
// 0 ms limit. With that path's windows up to and including that one pinned
// in full, a free-after search under a 0 ms limit, soft or hard, still
// returns a path that starts with them, and every window it chose itself
// after them fits the limit. Without the pins the hard limit drops it.
TEST_CASE("free after: a pinned window over the Path limit survives a soft and a hard limit") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const ScoreGraph graph(song, kCloneHeroSpCap);
        const std::vector<Path> unlimited = run_search(graph, EngineOptions{});
        if (unlimited.empty()) continue;
        const Path& best = unlimited.front();
        const std::vector<Activation> acts = best.all_activations();
        size_t over = 0;
        while (over < acts.size() && acts[over].within_ms_limit(0.0)) ++over;
        if (over == acts.size()) continue;
        CAPTURE(chart);
        CAPTURE(best.pathstring());
        std::vector<PinnedWindow> pins = pinned_windows(best);
        pins.resize(over + 1);

        for (const bool hard : {false, true}) {
            CAPTURE(hard);
            EngineOptions pinned = best_finish_options(cfg, pins);
            pinned.ms_filter = 0.0;
            pinned.hard_ms_filter = hard;
            const std::vector<Path> kept = keep_prefix_paths(run_search(graph, pinned), pins);
            REQUIRE_FALSE(kept.empty());
            for (const Path* p : roots_and_tied(kept)) {
                CAPTURE(p->pathstring());
                const std::vector<Activation> own = p->all_activations();
                for (size_t i = pins.size(); i < own.size(); ++i)
                    CHECK(own[i].within_ms_limit(0.0));
            }

            // The same limit with the pins taken off drops that window.
            if (hard) {
                pinned.target_act_ticks.reset();
                pinned.target_deact_ticks.clear();
                CHECK(keep_prefix_paths(run_search(graph, pinned), pins).empty());
            }
        }
        return;
    }
    FAIL("no corpus chart has a best path over a 0 ms limit");
}

// keep_prefix_paths and keep_target_paths over the engine's own answer: the
// unpinned search on fuzz seed 5, every survivor kept. The first of its
// roots that activates at least twice is the case: pinned at its first
// activation it is kept as a prefix match, and pinned at its second it is
// dropped, because it activates earlier. keep_target_paths, on the same
// paths and pins, keeps only the exact matches, so never that root.
TEST_CASE("keep_prefix_paths keeps a prefix match and drops a path that activates earlier") {
    const FuzzChart fc = fuzz_chart(5);
    const Song song = chart_song(fc.text);
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const ScoreGraph graph = search_graph(song, cfg);
    const std::vector<Path> raw = run_search(graph, test::wide_search());
    const Path* twice = nullptr;
    for (size_t i = 0; i < raw.size() && !twice; ++i)
        if (act_ticks(raw[i]).size() >= 2) twice = &raw[i];
    REQUIRE(twice != nullptr);
    CAPTURE(twice->pathstring());
    const std::vector<int64_t> ticks = act_ticks(*twice);
    const std::string key = path_key(*twice);

    for (const size_t at : {size_t{0}, size_t{1}}) {
        CAPTURE(at);
        const std::vector<int64_t> pin = {ticks[at]};
        const std::vector<PinnedWindow> pins = activation_pins(pin);

        std::vector<bool> promoted;
        const std::vector<Path> prefix = keep_prefix_paths(raw, pins, &promoted);
        check_kept_from(raw, prefix, promoted, beginning_with_ticks(pin));
        CHECK(holds_path(prefix, key) == (at == 0));

        std::vector<bool> exact_promoted;
        const std::vector<Path> exact = keep_target_paths(raw, pins, &exact_promoted);
        check_kept_from(raw, exact, exact_promoted, exactly_ticks(pin));
        CHECK_FALSE(holds_path(exact, key));
    }
}
