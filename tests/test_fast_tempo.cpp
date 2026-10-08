// Charts so fast that one SP bar lasts less than the 500 ms squeeze window
// (D32). No library chart comes close: its shortest SP bar, measured from a
// phrase chord or an SP-end node, is about 600 ms. These charts only exist to
// keep the search's own rules consistent, so it never throws, a tied variant
// still stores what a lone search stores, and no bar is banked on a phrase a
// squeeze-in spent.
//
// The fixtures in testdata/input/test_fast_tempo are fuzzed charts at 2,000
// or 4,000 BPM, untrimmed. They have no song.ini, so the corpus scan skips
// them.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"
#include "bank_check.h"
#include "chart_text.h"
#include "corpus_util.h"
#include "core/model.h"
#include "core/replay.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

// Default settings at a cap, top 40 scores, no ms limit (record_fixtures.h).
using test::scores_settings;

Song fixture(const std::string& name) {
    return load_songpath(std::string(HYDRA_INPUT_DIR) + "/test_fast_tempo/" + name, true, true);
}

// A root's tied variants, theirs, and so on, and every root's at once; a
// path's activation ticks (record_fixtures.h).
using test::all_tied;
using test::collect_tied;
using test::act_ticks;

// D3's promise, checked without the corpus: the variant stores what the
// search stores when it is told to activate exactly where the variant does
// (test::lone_pricing in record_fixtures.h). Returns "" on a match, else
// what differed, or the throw.
std::string lone_mismatch(const Song& song, const app::AnalysisSettings& cfg, const Path& variant) {
    try {
        return test::lone_pricing_mismatch(song, cfg, variant);
    } catch (const std::exception& e) {
        return "'" + variant.pathstring() + "': the lone search threw: " + e.what();
    }
}

// The analysis, or a failed check naming the throw (so one chart's throw
// does not hide the others').
bool analyzes(const Song& song, const app::AnalysisSettings& cfg, HydraRecord& rec) {
    try {
        rec = analyze_chart(song, cfg);
        return true;
    } catch (const std::exception& e) {
        FAIL_CHECK("analyze_chart threw: " << std::string(e.what()));
        return false;
    }
}

// D34: each SqIn has its own step, never repeated or squeezed out
// (tests/bank_check.h).
using bank_check::check_one_step_per_sqin;

// The corpus test's bank checks (bank_check::check_record_banks) on every
// stored path, tied variants included: no bank list holds a phrase a
// squeeze-in spent, and every list banks in order between its windows. The
// corpus never reaches the D32 states, so these charts are where they are
// checked. The order check came back with D36: until then a squeezed-out bar
// could land on the real deact node past the record's deact_tick() (on
// node_before_phrase at cap 3, path "0-" stored 13440 and ended at 15648).
// That a closed window's record ends on the node SP ended at is rebuild's
// own check (engine.cpp), so analyze_chart throws when it breaks.
using bank_check::check_record_banks;

// The window activated on `act_tick`, if `p` has one.
const Activation* window_at(const Path& p, int64_t act_tick) {
    for (const Activation& a : p.walk_activations())
        if (a.timecode.ticks() == act_tick) return &a;
    return nullptr;
}

bool has_sqin_step(const Activation& a, int64_t tick) {
    const std::vector<int64_t> sqins = sqin_phrase_ticks(a);
    return std::find(sqins.begin(), sqins.end(), tick) != sqins.end();
}

// The reviewer's fuzz chart shape (review-sqout fuzz.cpp), on a fixed
// generator so the charts are the same on every build: four measures at
// 120 BPM with two or three phrases, then 2,000 or 4,000 BPM from a random
// tick, with 12 to 30 random notes that are phrases, fills or plain.
struct FuzzChart {
    std::string text;
    int cap = 2;
};
FuzzChart fuzz_chart(uint64_t seed) {
    uint64_t s = seed * 0x9E3779B97F4A7C15ull + 1;
    auto next = [&s]() {
        uint64_t z = (s += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    };
    auto U = [&next](int a, int b) { return a + (int)(next() % (uint64_t)(b - a + 1)); };
    const int fast_at = 3072 + 96 * U(0, 60);
    const std::string sync =
        testchart::kSync44At120 +
        testchart::line(fast_at, "B " + std::to_string(U(0, 1) ? 4000000 : 2000000));
    std::ostringstream c;  // the [ExpertDrums] lines
    std::vector<int> ticks;
    for (int t = 0; t < 3072; t += 96) ticks.push_back(t);
    const int n_rand = U(12, 30);
    for (int i = 0; i < n_rand; ++i) ticks.push_back(3072 + 96 * U(0, 220));
    std::sort(ticks.begin(), ticks.end());
    ticks.erase(std::unique(ticks.begin(), ticks.end()), ticks.end());
    for (int t : ticks) {
        bool phrase = t == 768 || t == 2304;
        bool fill = false;
        if (t == 1536) phrase = U(0, 1) == 1;
        if (t >= 3072) {
            const int r = U(0, 99);
            phrase = r < 28;
            fill = !phrase && r < 52;
        }
        if (fill) c << "  " << (t - 384) << " = S 64 384\n";
        if (phrase) c << "  " << t << " = S 2 10\n";
        c << "  " << t << " = N " << (fill ? 1 : U(0, 1) ? 1 : 2) << " 0\n";
    }
    return {testchart::chart_text(
                testchart::section("Events", "") + testchart::section("ExpertDrums", c.str()),
                192, "  Offset = 0\n", sync),
            U(2, 4)};
}

}  // namespace

// The graph adds a node where an SP end falls. An end the graph only learns
// of at a later phrase (a squeeze-in's end one bar past an SP end, when the
// bar is shorter than the squeeze window) used to land behind that phrase:
// the node at the earlier tick was built after it and took the phrase on its
// incoming edge. A path folded there then held a step past its fold (the
// close_folded_act throw). Both tracks must run forward in time, and each
// phrase must sit on the edge that reaches it.
TEST_CASE("fast tempo: graph tracks run forward and every phrase sits on its own edge") {
    for (const std::string name : {"node_before_phrase.chart", "end_on_window_node.chart",
                                   "sqin_end_before_phrase.chart", "late_sqin_twice.chart",
                                   "early_sqin_twice.chart"}) {
        const Song song = fixture(name);
        for (int cap : {2, 3, 4}) {
            CAPTURE(name);
            CAPTURE(cap);
            ScoreGraph graph(song, cap);
            for (const ScoreGraphNode* n = graph.start(); n; n = n->adv_edge ? n->adv_edge->dest : nullptr)
                if (n->adv_edge) CHECK(n->adv_edge->dest->timecode.ticks() > n->timecode.ticks());
            const ScoreGraphNode* sp_start = test::sp_track_start(graph);
            REQUIRE(sp_start != nullptr);
            for (const ScoreGraphNode* n = sp_start; n && n->adv_edge; n = n->adv_edge->dest) {
                const int64_t from = n->timecode.ticks(), to = n->adv_edge->dest->timecode.ticks();
                CHECK(to > from);
                for (const auto& [tc, ext] : n->adv_edge->sp_times) {
                    CHECK(tc.ticks() > from);
                    CHECK(tc.ticks() <= to);
                }
            }
        }
    }
}

// Each fixture threw before D32's fixes:
//   node_before_phrase      "a folded variant holds a step past its fold"
//   end_on_window_node      "search reached a broken state": a path whose SP
//                           ended on a node with a squeezable phrase in its
//                           window was never deactivated there
//   sqin_end_before_phrase  the same throw: a late squeeze-in's end, one bar
//                           past the old end, fell before its own phrase and
//                           had no node
//   late_sqin_twice         the same throw: a late squeeze-in's phrase was
//                           squeezed in a second time at the next SP end
// early_sqin_twice never threw. A path on it squeezed one early phrase in at
// two SP ends; D34 ended that (a phrase is squeezed in only once). A variant
// folded there could still differ from its lone pricing (D36 gap b): the
// search grouped running paths without the phrases their window already
// squeezed in, so a variant took its leader's squeeze of the next phrase
// where alone it would squeeze the first. D36 offers only the newest phrase,
// which every path at one node shares, so the chart is back in the list.
TEST_CASE("fast tempo: the crash charts analyze, one SqIn step per SqIn") {
    const std::vector<std::pair<std::string, int>> charts = {
        {"node_before_phrase.chart", 3},     {"end_on_window_node.chart", 2},
        {"sqin_end_before_phrase.chart", 3}, {"late_sqin_twice.chart", 2},
        {"early_sqin_twice.chart", 2}};
    for (const auto& [name, cap] : charts) {
        CAPTURE(name);
        const Song song = fixture(name);
        const app::AnalysisSettings cfg = scores_settings(cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        REQUIRE_FALSE(rec.paths.empty());
        for (const Path* p : rec.all_paths()) check_one_step_per_sqin(*p);
        check_record_banks(song, rec);
        const std::vector<const Path*> all = all_tied(rec.paths);
        for (const Path* v : all) {
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}

// The engine's "spent phrase stays buffered" branch, pinned. On this chart at
// cap 3, the path that activates at 5472 late-squeezes in the phrase at 13920,
// and its new SP end is 11616, before that phrase. So SP ends first, with an
// empty meter and the phrase still spent. The graph has a node between the
// two, so the path then walks an edge without the phrase: the meter math
// would go below zero there, and the engine keeps the phrase buffered instead.
// Hitting the phrase later must add nothing, so no bank list holds 13920 (a
// tick there would light a bar the SqIn already used).
TEST_CASE("fast tempo: a phrase a SqIn spent banks no bar when SP ends before it") {
    const Song song = fixture("sqin_end_before_phrase.chart");
    const app::AnalysisSettings cfg = scores_settings(3);
    constexpr int64_t kAct = 5472, kPhrase = 13920, kEnd = 11616;

    const ScoreGraph graph(song, std::optional<int>(graph_build_cap(cfg.sp_cap, song.sp_phrase_count())),
                           FillDeadlineRule::Ch11, cfg.rules);
    bool node_between = false;
    for (const ScoreGraphNode* n = graph.start(); n; n = n->adv_edge ? n->adv_edge->dest : nullptr) {
        const int64_t t = n->timecode.ticks();
        if (t > kEnd && t < kPhrase) node_between = true;
    }
    CHECK(node_between);

    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    int found = 0;
    for (const Path* p : rec.all_paths()) {
        bool ends_early = false;
        for (const Activation& a : p->walk_activations())
            for (const SpEndStep& s : a.sp_end_steps)
                if (a.timecode.ticks() == kAct && is_sqin_step_on(s.tick, s.kind, kPhrase) &&
                    s.end_tick == kEnd)
                    ends_early = true;
        if (!ends_early) continue;
        ++found;
        CAPTURE(p->pathstring());
        for (const Activation& a : p->walk_activations()) {
            CAPTURE(a.timecode.ticks());
            CHECK(std::count(a.bank_rise_ticks.begin(), a.bank_rise_ticks.end(), kPhrase) == 0);
        }
        CHECK(std::count(p->trailing_bank_ticks.begin(), p->trailing_bank_ticks.end(), kPhrase) == 0);
    }
    CHECK(found >= 1);
}

// D34: a phrase can be squeezed in only once. A phrase banked before SP
// started is never offered (D18), and neither is one this window already
// squeezed in: an SP end offers the window's newest phrase before it (D36),
// or, when the window's end is this end, the first phrase after it not yet
// squeezed in. Both hand-made charts run at 4,000 BPM from tick 9600, so
// 500 ms spans about four SP bars. The activation is on 12288 with 2 bars.
//   banked_then_next  the phrase on 12000 is banked before the activation and
//                     sits in the window of the SP end 15360. The next phrase,
//                     16000, comes after that end. Before D34 the banked
//                     phrase hid it, so nothing was squeezed there.
//   spent_then_next   the phrase on 17360 is squeezed in late at the SP end
//                     15360, which moves the end to 16896, still before it.
//                     The next phrase, 18360, sits in the window of 16896.
//                     Before D34 the spent phrase hid it, and SP just ended
//                     at 16896.
// Each chart must offer the next phrase both ways (in and out), analyze, and
// keep every variant priced as it is alone.
TEST_CASE("fast tempo: an SP end offers the next phrase after a banked or spent one (D34)") {
    struct Want {
        std::string name;
        int64_t skipped;  // the banked or spent phrase
        int64_t next;     // the window's next phrase, which must be offered
    };
    for (const Want& w : {Want{"banked_then_next.chart", 12000, 16000},
                          Want{"spent_then_next.chart", 17360, 18360}}) {
        CAPTURE(w.name);
        const Song song = fixture(w.name);
        const app::AnalysisSettings cfg = scores_settings(2);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        bool next_in = false, next_out = false;
        for (const Path* p : rec.all_paths()) {
            check_one_step_per_sqin(*p);
            const Activation* a = window_at(*p, 12288);
            if (!a) continue;
            if (has_sqin_step(*a, w.next)) next_in = true;
            if (a->sqout_tick && *a->sqout_tick == w.next) next_out = true;
            // The banked phrase is never squeezed by this window. The spent
            // one is squeezed in at most once, and never out after that
            // (check_one_step_per_sqin).
            if (w.name == "banked_then_next.chart") {
                CHECK_FALSE(has_sqin_step(*a, w.skipped));
                CHECK(a->sqout_tick.value_or(-1) != w.skipped);
            }
        }
        CHECK(next_in);
        CHECK(next_out);
        check_record_banks(song, rec);
        const std::vector<const Path*> all = all_tied(rec.paths);
        for (const Path* v : all) {
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}

// D36 gap a, pinned. On node_before_phrase at cap 3 the window activated on
// 8832 starts with its end on 13440 and collects 9696, 11040 and 14016; the
// last moves its end from 15648. All three sit in the squeeze window of the
// SP end 15648. A squeeze-out there can only be 14016's: 9696 and 11040 were
// hit before it, so a player who hits either after SP ran out also hits
// 14016 after SP ran out. Before D36 the search squeezed out 9696 (the first
// in the window), kept 11040's and 14016's extensions, ended at 15648 and
// stored the end 13440.
TEST_CASE("fast tempo: a squeeze-out at an SP end gives back only the newest phrase (D36)") {
    const Song song = fixture("node_before_phrase.chart");
    const app::AnalysisSettings cfg = scores_settings(3);
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    int at_15648 = 0;
    for (const Path* p : rec.all_paths()) {
        CAPTURE(p->pathstring());
        const Activation* a = window_at(*p, 8832);
        if (!a || !a->sqout_tick) continue;
        // Never 9696 or 11040, whatever the node.
        CHECK(*a->sqout_tick != 9696);
        CHECK(*a->sqout_tick != 11040);
        if (a->deact_tick() != std::optional<int64_t>(15648)) continue;
        ++at_15648;
        CHECK(*a->sqout_tick == 14016);
    }
    CHECK(at_15648 > 0);
}

// D36 gap b, pinned. On early_sqin_twice at cap 2 the window activated on
// 10176 collects 11712, 13152 and 13248. The SP end 16224 is the one 13248
// moved, and it is the only end where this window can squeeze anything:
// in or out, the phrase is 13248. Before D36 the SP end 14784 offered 11712
// or 13152 (the first not yet squeezed in), and a folded variant took its
// leader's choice of the two.
TEST_CASE("fast tempo: early_sqin_twice offers its window one squeeze, 13248 at 16224 (D36)") {
    const Song song = fixture("early_sqin_twice.chart");
    const app::AnalysisSettings cfg = scores_settings(2);
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    bool in = false, out = false;
    for (const Path* p : rec.all_paths()) {
        CAPTURE(p->pathstring());
        const Activation* a = window_at(*p, 10176);
        if (!a) continue;
        for (const int64_t t : sqin_phrase_ticks(*a)) CHECK(t == 13248);
        if (has_sqin_step(*a, 13248)) in = true;
        if (!a->sqout_tick) continue;
        CHECK(*a->sqout_tick == 13248);
        CHECK(a->deact_tick() == std::optional<int64_t>(16224));
        out = true;
    }
    CHECK(in);
    CHECK(out);
}

// Seeds whose only lone-pricing mismatch is the known SP-ready grouping gap.
// The search groups running paths without their SP-ready time (see the
// grouping key in Engine::reduce_group), so at these tempos a variant can
// inherit its leader's earlier SP-ready time and take a fill that a lone
// search refuses ("no lone path ties it"; the D32 review's fuzz chart 6006
// shows it).
// That gap is an open question with the user, not part of D32. A seed that
// fails only on it is listed here by number, so the check stays exact for
// every other seed. Seeds 1 to 48 hit none today, so the list is empty.
const std::vector<uint64_t> kSpReadyGapSeeds = {};

// A small deterministic fuzz: every chart analyzes, keeps one SqIn step per
// SqIn, and every tied variant prices as it does alone ("prices": gets the
// same score and stored facts as a search told to take its activations).
TEST_CASE("fast tempo: fuzzed charts analyze and their variants price as alone") {
    int analyzed = 0, variants = 0;
    for (uint64_t seed = 1; seed <= 48; ++seed) {
        CAPTURE(seed);
        const FuzzChart fc = fuzz_chart(seed);
        const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
        const Song song = load_songbytes_chart(bytes, true, true);
        const app::AnalysisSettings cfg = scores_settings(fc.cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        ++analyzed;
        for (const Path* p : rec.all_paths()) check_one_step_per_sqin(*p);
        check_record_banks(song, rec);
        const bool gap_seed = std::find(kSpReadyGapSeeds.begin(), kSpReadyGapSeeds.end(),
                                        seed) != kSpReadyGapSeeds.end();
        if (gap_seed) continue;
        const std::vector<const Path*> all = all_tied(rec.paths);
        for (const Path* v : all) {
            ++variants;
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), "seed " << seed << " " << diff);
        }
    }
    // The seeds, these two counts and the generator's ranges are test limits
    // the user approved (D43).
    CHECK(analyzed == 48);
    CHECK(variants > 50);
}

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
TEST_CASE("search_target: a path missing a named activation is dropped, the rest kept (D45)") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, true, true);
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
    const Song song = load_songbytes_chart(bytes, true, true);
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
    const Song song = load_songbytes_chart(bytes, true, true);
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
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
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
    const Song song = load_songbytes_chart(bytes, true, true);
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
TEST_CASE("search_target: activation pins return the tick search's paths and its failing tick (D45)") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, true, true);
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
    const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
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

// The phrases still ahead when SP ends, pinned (s1-fix-merge). D34 lets one
// window squeeze in two late phrases that both come after its final SP end:
// the path leaves SP with two spent phrases ahead and an empty meter. Its
// SqOut sibling squeezes a third phrase out at that end instead, so it leaves
// SP with two spent phrases and one banked bar ahead. Before the engine kept
// spent and banked phrases apart, the first edge after either end threw "the
// SP meter went below zero off SP". The record shows both states: SqIn steps
// on phrases past the window's deact tick, and for the second a squeezed-out
// phrase past it too. Each path that shows one must price as it does alone,
// bank no spent phrase, and bank the squeezed-out phrase's bar once, on its
// own tick. The seeds come from the fuzz generator above.
TEST_CASE("fast tempo: two spent phrases ahead, with or without a banked one, price as alone") {
    struct Case {
        uint64_t seed;
        int cap;
        bool banked;  // the SqOut sibling's state: two spent plus one banked
    };
    for (const Case& c : {Case{23, 2, false}, Case{23, 3, false}, Case{23, 4, false},
                          Case{39, 3, true}, Case{39, 4, true}}) {
        CAPTURE(c.seed);
        CAPTURE(c.cap);
        const FuzzChart fc = fuzz_chart(c.seed);
        const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
        const Song song = load_songbytes_chart(bytes, true, true);
        const app::AnalysisSettings cfg = scores_settings(c.cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        check_record_banks(song, rec);
        int shown = 0;
        for (const Path* p : rec.all_paths()) {
            const ActivationWalk walk = p->walk_activations();
            for (size_t i = 0; i < walk.size(); ++i) {
                const Activation& a = walk[i];
                const std::optional<int64_t> d = a.deact_tick();
                if (!d) continue;
                const std::vector<int64_t> sqins = sqin_phrase_ticks(a);
                const auto spent_ahead = std::count_if(sqins.begin(), sqins.end(),
                                                       [&d](int64_t t) { return t > *d; });
                if (spent_ahead < 2) continue;
                const bool banked_ahead = a.sqout_tick && *a.sqout_tick > *d;
                if (banked_ahead != c.banked || (!c.banked && a.sqout_tick)) continue;
                ++shown;
                CAPTURE(p->pathstring());
                CAPTURE(a.timecode.ticks());
                if (banked_ahead) {
                    const std::vector<int64_t>& next =
                        i + 1 < walk.size() ? walk[i + 1].bank_rise_ticks : p->trailing_bank_ticks;
                    CHECK(std::count(next.begin(), next.end(), *a.sqout_tick) == 1);
                }
                const std::string diff = lone_mismatch(song, cfg, *p);
                CHECK_MESSAGE(diff.empty(), diff);
            }
        }
        CHECK(shown > 0);
        const std::vector<const Path*> all = all_tied(rec.paths);
        for (const Path* v : all) {
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}
