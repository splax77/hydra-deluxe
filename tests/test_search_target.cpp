// search_target and keep_target_paths: a targeted search keeps exactly the
// paths that took every named activation (D45), and a full pin of a stored
// path (Track R2) comes back alone, or names the window that broke and why.
// Most cases pin answers on the fuzzed fast-tempo charts (fast_tempo_charts.h).
// The free-after cases (EngineOptions::free_after, keep_prefix_paths) follow
// the pins and then search freely, on the corpus.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "corpus_util.h"
#include "core/model.h"
#include "core/replay.h"
#include "fast_tempo_charts.h"
#include "parse/song.h"
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

// The fuzzed charts and the analysis wrapper (fast_tempo_charts.h).
using test::FuzzChart;
using test::fuzz_chart;
using test::analyzes;

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

// Every root and tied variant in `paths`, roots first.
std::vector<const Path*> roots_and_tied(const std::vector<Path>& paths) {
    std::vector<const Path*> out;
    for (const Path& p : paths) out.push_back(&p);
    for (const Path* v : test::all_tied(paths)) out.push_back(v);
    return out;
}

// A search at the settings' depth and Path limit that follows `windows` (their
// act ticks, and the ends of those pinned in full) and runs free after them.
EngineOptions free_after_options(const SearchSettings& settings,
                                 const std::vector<PinnedWindow>& windows) {
    EngineOptions options;
    options.depth_mode = settings.depth_mode;
    options.depth_value = settings.depth_value;
    options.ms_filter = settings.ms_filter;
    options.free_after = true;
    std::vector<int64_t> ticks;
    for (const PinnedWindow& w : windows) {
        ticks.push_back(w.act_tick);
        if (w.deact_tick) options.target_deact_ticks.push_back(*w.deact_tick);
    }
    options.target_act_ticks = ticks;
    return options;
}

}  // namespace

// D45: a targeted search drops only the paths that came back without one of
// the named activations, and keeps the ones that took them all. On fuzz seed
// 5, told to activate at 3168 and 12864, the engine also returns paths that
// take only 3168 (SP still runs at 12864's fill); search_target drops those.
// Before D45 the one short path made it report the whole set unrealizable.
// The kept paths are pinned as read from one run.
// Step 1 read four kept paths here. Step 2 reads five: D21 (a phrase that
// runs past the last note pays on that note) awards seed 5's last phrase, at
// 22848. That brings back '0+- 0++', which takes 3168 and 12864 as a tied
// variant of a root that took only 3168, so search_target promotes it to a
// result of its own (the next test's case). Two other path strings gain a
// sign for that phrase ('0++- E0+' reads '0++- E0++'); no total moves.
// T10 (finding 37, D44 addendum) reads six: the squeeze choices now offer a
// squeeze at the cap's ceiling, where a full meter pins the SP end when a
// phrase is hit mid-SP. Seed 5's second window starts full, so it gains
// '0- 0+-' (in place of '0- 0', same 6,750), '0- 0++' (a tied variant at
// 7,150) and '0- 0-' (6,550). The best, 7,550, is unchanged.
TEST_CASE("search_target: a path missing a named activation is dropped, the rest kept") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, ChartReading{});
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const std::vector<int64_t> want = {3168, 12864};

    std::vector<bool> promoted;
    const std::vector<Path> kept = search_target(song, cfg, want, &promoted);
    CHECK(kept.size() == 6);
    CHECK(target_text(kept) ==
          "0+- 0++ 7550 | 0++- E0++ 7350 [0++- E0+- 7350] | "
          "0++- E0- 7150 [0+- 0+- 7150; 0- 0++ 7150] | 0+- 0- 6950 | 0- 0+- 6750 | "
          "0- 0- 6550 | ");
    // '0+- 0++' is the promoted variant; the other five are the engine's roots.
    CHECK(promoted == std::vector<bool>{true, false, false, false, false, false});
    for (const Path& p : kept) {
        CHECK(act_ticks(p) == want);
        std::vector<const Path*> tied;
        collect_tied(p, tied);
        for (const Path* v : tied) CHECK(act_ticks(*v) == want);
    }

    // A tick that is no fill node is still unrealizable: nothing comes back.
    CHECK(search_target(song, cfg, {3168, 12865}).empty());
}

// D45, a dropped root's variants: the engine can fold a path that takes
// every named activation under a tied root that missed one. search_target
// drops that root but keeps the variant, as a result of its own (the first
// such variant leads, the rest stay its tied variants). The chart is fuzz
// seed 5 with one more note after its last phrase, so that phrase is
// awarded: root '0++++-' takes only 3168, and its tied variant '0+- 0++'
// takes 3168 and 12864. Before the fix the variant went with the root.
// Under step 2, D21 already awards that phrase on seed 5 alone (the test
// above promotes '0+- 0++' too). The extra note stays: with it, every value
// pinned below reads the same as under step 1, except the list of results:
// T10 (finding 37, D44 addendum) offers squeezes at the cap's ceiling, so
// seed 5 gains '0- 0+-' (in place of '0- 0', same 6,950), '0- 0++' (a tied
// variant at 7,350) and '0- 0-' (6,750), as in the test above. The best,
// 7,750, is unchanged.
TEST_CASE("search_target: a variant that took every named activation outlives its dropped root") {
    FuzzChart fc = fuzz_chart(5);
    const std::string last_phrase = "  22848 = N 1 0\n";
    const size_t at = fc.text.find(last_phrase);
    REQUIRE(at != std::string::npos);
    fc.text.insert(at + last_phrase.size(), "  23040 = N 1 0\n");
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, ChartReading{});
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const std::vector<int64_t> want = {3168, 12864};

    std::vector<bool> was_promoted;
    const std::vector<Path> kept = search_target(song, cfg, want, &was_promoted);
    CHECK(target_text(kept) ==
          "0+- 0++ 7750 | 0++- E0++ 7550 [0++- E0+- 7550] | "
          "0++- E0- 7350 [0+- 0+- 7350; 0- 0++ 7350] | 0+- 0- 7150 | 0- 0+- 6950 | "
          "0- 0- 6750 | ");
    // search_target says which result it promoted: only the first.
    CHECK(was_promoted == std::vector<bool>{true, false, false, false, false, false});
    const Path* promoted = test::path_named(kept, "0+- 0++");
    REQUIRE(promoted != nullptr);
    CHECK(promoted->totalscore() == 7750);
    CHECK_FALSE(promoted->var_point.has_value());
    CHECK(promoted->variant_tail.empty());

    // Its windows are its own, not the dropped root's: pinned as read from
    // one run, and the same as the full search stores for that path (there a
    // tied variant of '0++++-', read through its root).
    CHECK(test::windows_text(*promoted) ==
          "3168 [steps 3168>7776:0 4800>9312:3 | sq +-232.5 -22.5 | out 9600 | backends "
          "4128/200/0/-405 4800/200/0/-352.5 7008/200/0/-180 9600/200/0/22.5 | bank 768 1536 "
          "2304 | e 1250 | passed] 12864 [steps 12864>17472:0 13440>19008:3 22848>20544:3 | sq "
          "+-315 +300 | out -1 | backends 19776/200/0/-60 19968/200/0/-45 22848/200/0/180 "
          "23040/200/0/195 | bank 9600 10080 11616 | e 127.5 | passed] trailing");
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    const Path* full = test::path_named(rec.all_paths(), "0+- 0++");
    REQUIRE(full != nullptr);
    CHECK(full->var_point.has_value());
    CHECK(full->totalscore() == 7750);
    CHECK(test::windows_text(*promoted) == test::windows_text(*full));

    // So the promoted copy would pass for the full search's variant, but it
    // came out of the same fold, so it is no oracle. The lone-pricing helper
    // reads search_target's promoted flag and reports the variant as tied
    // under a root (skipped), not as a lone match against its own fold.
    const test::LonePricing lone = test::lone_pricing(song, cfg, *full);
    CHECK(lone.tied_under_root);
    CHECK_FALSE(lone.diff.empty());
}

// D45 addendum, a kept root's variants: every tied path search_target
// returns has exactly the named activations. On fuzz seed 199, told to
// activate at 4896, 12288 and 21792, the engine ties '0+- E0++++' (it takes
// only 4896 and 12288) under the root '0++- E0++- 0', which takes all three,
// and '0- 0+++-' under '0++- E0- 0'. Both are dropped and their roots kept;
// before the addendum they stayed in brackets. The answer is pinned as read
// from one run.
TEST_CASE("search_target: a kept root's tied variant that missed a named activation is dropped") {
    const FuzzChart fc = fuzz_chart(199);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, ChartReading{});
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const std::vector<int64_t> want = {4896, 12288, 21792};

    std::vector<bool> promoted;
    const std::vector<Path> kept = search_target(song, cfg, want, &promoted);
    CHECK(target_text(kept) ==
          "0++- E0++- 0 12350 | 0++- E0+- 0 11750 | 0++- E0- 0 11350 [0+- E0+- 0 11350] | "
          "0- 0+- 0 11150 | 0+- E0- 0 10750 | 0- 0- 0 10550 | ");
    CHECK(promoted.size() == kept.size());
    std::vector<int> counts;
    for (const Path& p : kept) {
        CHECK(act_ticks(p) == want);
        std::vector<const Path*> tied;
        collect_tied(p, tied);
        for (const Path* v : tied) CHECK(act_ticks(*v) == want);
        counts.push_back(p.tied_pathcount());
    }
    // Each root's stored tied-path count, pinned as read from one run.
    CHECK(counts == std::vector<int>{1, 1, 2, 1, 1, 1});
}

// keep_target_paths on a hand-built list: it reads only each path's
// activation ticks. Labels ride in notecount, which it never touches.
TEST_CASE("keep_target_paths: every kept path and tied variant took exactly the named ticks") {
    auto made = [](int label, std::vector<int64_t> ticks) {
        Path p;
        p.notecount = label;
        for (const int64_t t : ticks) {
            Activation a;
            a.timecode = Timecode::raw(t);
            p.activations.push_back(a);
        }
        return p;
    };
    // Root 1 takes both. Its variant 2 misses 200, but 2's own variant 3
    // takes both; its variant 4 takes both and 4's variant 5 misses 100.
    Path r1 = made(1, {100, 200});
    Path v2 = made(2, {100});
    v2.variants.push_back(made(3, {100, 200}));
    Path v4 = made(4, {100, 200});
    v4.variants.push_back(made(5, {200}));
    r1.variants = {v2, v4};
    // Root 6 misses 200; its variants 7 and 8 take both, 7's variant 9
    // misses one. Root 10 takes neither and has no qualifying variant.
    Path r6 = made(6, {100});
    Path v7 = made(7, {100, 200});
    v7.variants.push_back(made(9, {300}));
    r6.variants = {v7, made(8, {100, 200})};
    const std::vector<Path> in = {r1, r6, made(10, {300})};

    std::vector<bool> promoted;
    const std::vector<Path> out = keep_target_paths(in, activation_pins({100, 200}), &promoted);
    REQUIRE(out.size() == 2);
    CHECK(promoted == std::vector<bool>{false, true});

    // Root 1 stays. 2 is dropped and its variant 3 rescued in its place,
    // sharing nothing with root 1; 4 stays as folded, without 5.
    const Path& k1 = out[0];
    CHECK(k1.notecount == 1);
    REQUIRE(k1.variants.size() == 2);
    CHECK(k1.variants[0].notecount == 3);
    CHECK(k1.variants[0].var_point == std::optional<int>(2));
    CHECK(k1.variants[1].notecount == 4);
    CHECK_FALSE(k1.variants[1].var_point.has_value());
    CHECK(k1.variants[1].variants.empty());
    CHECK(k1.tied_pathcount() == 3);

    // Root 6 is dropped: 7 leads, without 9, and 8 is its tied variant.
    const Path& k7 = out[1];
    CHECK(k7.notecount == 7);
    CHECK_FALSE(k7.var_point.has_value());
    REQUIRE(k7.variants.size() == 1);
    CHECK(k7.variants[0].notecount == 8);
    CHECK(k7.variants[0].var_point == std::optional<int>(2));
    CHECK(k7.tied_pathcount() == 2);
}

// Track R2: every corpus path, handed back to search_target as a full pin
// (pinned_windows: each window's activation, deactivation node and
// squeeze-out), comes back alone and stores what the search stored for it.
// full_pin_mismatch is the check hydra_replay selfcheck runs; windows_text
// adds every other stored fact of each window.
TEST_CASE("search_target: every corpus path pinned in full comes back alone, the same") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int paths = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        for (const Path* p : rec.all_paths()) {
            ++paths;
            CAPTURE(chart);
            CAPTURE(p->pathstring());
            CHECK(full_pin_mismatch(song, cfg, *p) == "");
            const TargetResult t = search_target(song, cfg, pinned_windows(*p));
            CHECK(t.paths.size() == 1);
            if (t.paths.size() != 1) continue;
            CHECK(test::windows_text(t.paths[0]) == test::windows_text(*p));
        }
    }
    CHECK(paths > 0);
}

// Track R2 on D45's chart (fuzz seed 5, activations 3168 and 12864): a full
// pin of one of its paths comes back alone. Moved to an end no Star Power
// reaches, or told a squeeze-out the engine does not make there, it comes
// back empty, naming the window that broke and how.
TEST_CASE("search_target: a full pin no path realizes names its window and why") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, ChartReading{});
    const app::AnalysisSettings cfg = scores_settings(fc.cap);
    const std::vector<Path> kept = search_target(song, cfg, {3168, 12864});
    const Path* path = test::path_named(kept, "0+- 0-");
    REQUIRE(path != nullptr);
    const std::vector<PinnedWindow> pins = pinned_windows(*path);
    REQUIRE(pins.size() == 2);

    const TargetResult whole = search_target(song, cfg, pins);
    REQUIRE(whole.paths.size() == 1);
    CHECK(whole.paths[0].pathstring() == path->pathstring());
    CHECK(whole.paths[0].totalscore() == path->totalscore());
    CHECK(whole.realized_prefix == 2);
    CHECK_FALSE(whole.failed_tick.has_value());
    CHECK(whole.failed_reason.empty());

    // One tick after its own activation, no Star Power can end.
    std::vector<PinnedWindow> unreachable = pins;
    unreachable[1].deact_tick = unreachable[1].act_tick + 1;
    const TargetResult end = search_target(song, cfg, unreachable);
    CHECK(end.paths.empty());
    CHECK(end.realized_prefix == 1);
    CHECK(end.failed_tick == std::optional<int64_t>(12864));
    CHECK(end.failed_reason == "window_end");

    // The second window's end as stored, but the other answer on how it
    // ended: a plain end where it squeezed out, or 22848 (a phrase chord)
    // squeezed out where it ended plainly.
    std::vector<PinnedWindow> other_sqout = pins;
    REQUIRE(other_sqout[1].check_sqout);
    other_sqout[1].squeezed_out =
        pins[1].squeezed_out ? std::nullopt : std::optional<int64_t>(22848);
    const TargetResult sqout = search_target(song, cfg, other_sqout);
    CHECK(sqout.paths.empty());
    CHECK(sqout.realized_prefix == 1);
    CHECK(sqout.failed_tick == std::optional<int64_t>(12864));
    CHECK(sqout.failed_reason == "sqout");
}

// Activation pins alone ask what the tick list asks: search_target over them
// returns what search_target over the ticks returns (D45's answer above).
// D45's unrealizable set still fails on 12865, a tick that is no fill node,
// now found by the binary search over the prefix length.
TEST_CASE("search_target: activation pins return the tick search's paths and its failing tick") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, ChartReading{});
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
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Mutemath - Allies") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.all_paths().empty());
    const std::vector<PinnedWindow> pins = pinned_windows(*rec.all_paths().front());
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
        EngineOptions plain = free_after_options(cfg, {});
        plain.free_after = false;
        plain.target_act_ticks.reset();
        const std::vector<Path> want = run_search(graph, plain);
        const std::vector<Path> got = run_search(graph, free_after_options(cfg, {}));
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
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Mutemath - Allies") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, cfg.reading());
    const HydraRecord& rec = corpus::analyzed(chart, cfg);
    REQUIRE_FALSE(rec.all_paths().empty());
    const std::vector<int64_t> best = act_ticks(*rec.all_paths().front());
    REQUIRE(best.size() >= 3);

    const ScoreGraph graph(song, kCloneHeroSpCap);
    const std::vector<Path> got =
        run_search(graph, free_after_options(cfg, {PinnedWindow{best.front()}}));
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
            EngineOptions pinned = free_after_options(cfg, pins);
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
        }

        EngineOptions unpinned = free_after_options(cfg, {});
        unpinned.ms_filter = 0.0;
        unpinned.hard_ms_filter = true;
        CHECK(keep_prefix_paths(run_search(graph, unpinned), pins).empty());
        return;
    }
    FAIL("no corpus chart has a best path over a 0 ms limit");
}

// keep_prefix_paths on a hand-built list, as the keep_target_paths case
// above: labels ride in notecount. A path must start with the pinned ticks;
// one that activates before or between them, or stops short, is dropped, and
// a dropped root's qualifying variant is rescued. keep_target_paths, on the
// same list, keeps only the exact match.
TEST_CASE("keep_prefix_paths keeps a prefix match and drops a path that activates earlier") {
    auto made = [](int label, std::vector<int64_t> ticks) {
        Path p;
        p.notecount = label;
        for (const int64_t t : ticks) {
            Activation a;
            a.timecode = Timecode::raw(t);
            p.activations.push_back(a);
        }
        return p;
    };
    Path r3 = made(3, {50, 100, 200});
    r3.variants.push_back(made(4, {100, 200, 400}));
    const std::vector<Path> in = {made(1, {100, 200, 300}), made(2, {100, 200}), r3,
                                  made(5, {100, 150, 200}), made(6, {100})};
    const std::vector<PinnedWindow> pins = activation_pins({100, 200});

    std::vector<bool> promoted;
    const std::vector<Path> prefix = keep_prefix_paths(in, pins, &promoted);
    std::vector<int> labels;
    for (const Path& p : prefix) labels.push_back(p.notecount);
    CHECK(labels == std::vector<int>{1, 2, 4});
    CHECK(promoted == std::vector<bool>{false, false, true});

    const std::vector<Path> exact = keep_target_paths(in, pins);
    REQUIRE(exact.size() == 1);
    CHECK(exact[0].notecount == 2);
}
