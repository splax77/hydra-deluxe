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
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"
#include "bank_check.h"
#include "core/model.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

app::AnalysisSettings fast_settings(int cap) {
    app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    cfg.sp_cap = cap;
    cfg.depth_mode = DepthMode::Scores;
    cfg.depth_value = 40;
    cfg.ms_filter = std::nullopt;
    return cfg;
}

Song fixture(const std::string& name) {
    return load_songpath(std::string(HYDRA_INPUT_DIR) + "/test_fast_tempo/" + name, true, true);
}

// A root's tied variants, theirs, and so on (roots are their own lone pricing).
void collect_variants(const Path& p, std::vector<const Path*>& out) {
    for (const Path& v : p.variants) {
        out.push_back(&v);
        collect_variants(v, out);
    }
}

std::vector<int64_t> act_ticks(const Path& p) {
    std::vector<int64_t> at;
    for (const Activation& a : p.walk_activations()) at.push_back(a.timecode.ticks());
    return at;
}

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

// A late squeeze-in writes its own SqIn step on its phrase. The phrase is
// spent then, so no window holds two SqIn steps on one phrase (a late SqIn's
// phrase squeezed in again at the next SP end wrote a second one). D34: a
// phrase is squeezed in once, so each SqIn has its own step, and the phrase
// a window squeezes out is none it squeezed in.
void check_one_step_per_sqin(const Path& p) {
    for (const Activation& a : p.walk_activations()) {
        std::vector<int64_t> ticks;
        for (const SpEndStep& s : a.sp_end_steps)
            if (s.kind == SpEndKind::SqIn) ticks.push_back(s.tick);
        std::sort(ticks.begin(), ticks.end());
        CHECK_MESSAGE(std::adjacent_find(ticks.begin(), ticks.end()) == ticks.end(),
                      p.pathstring() << " at " << a.timecode.ticks());
        const size_t sqins = static_cast<size_t>(
            std::count_if(a.sqinouts.begin(), a.sqinouts.end(),
                          [](const SPSqueeze& q) { return q.kind == SqueezeKind::SqIn; }));
        CHECK_MESSAGE(sqins == ticks.size(), p.pathstring() << " at " << a.timecode.ticks());
        if (a.sqout_tick)
            CHECK_MESSAGE(!std::binary_search(ticks.begin(), ticks.end(), *a.sqout_tick),
                          p.pathstring() << " at " << a.timecode.ticks());
    }
}

// The corpus test's spent-phrase check (tests/bank_check.h) on every stored
// path: no bank list holds a phrase a squeeze-in spent. The corpus never
// reaches the D32 states, so these charts are where it is checked.
//
// The corpus test's order checks are not run here yet. They fail on a known
// D32 gap: when a squeeze-out's 500 ms window holds a second phrase after the
// squeezed one, the search deactivates one bar later than the stored steps
// say (the squeeze-out drops the second phrase's step), so the squeezed-out
// bar lands on the real deact node, not on deact_tick(). On
// node_before_phrase at cap 3, path "0-" stores deact_tick 13440 and banks
// the bar at 15648. Whether to fix that is an open question with the user.
void check_banks(const HydraRecord& rec) {
    std::vector<const Path*> all = rec.all_paths();
    for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
    for (const Path* p : all) {
        CAPTURE(p->pathstring());
        bank_check::check_spent_phrases(*p);
    }
}

// The window activated on `act_tick`, if `p` has one.
const Activation* window_at(const Path& p, int64_t act_tick) {
    for (const Activation& a : p.walk_activations())
        if (a.timecode.ticks() == act_tick) return &a;
    return nullptr;
}

bool has_sqin_step(const Activation& a, int64_t tick) {
    for (const SpEndStep& s : a.sp_end_steps)
        if (s.kind == SpEndKind::SqIn && s.tick == tick) return true;
    return false;
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
    std::ostringstream c;
    c << "[Song]\n{\n  Resolution = 192\n  Offset = 0\n}\n[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n";
    const int fast_at = 3072 + 96 * U(0, 60);
    c << "  " << fast_at << " = B " << (U(0, 1) ? 4000000 : 2000000)
      << "\n}\n[Events]\n{\n}\n[ExpertDrums]\n{\n";
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
    c << "}\n";
    return {c.str(), U(2, 4)};
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
            const ScoreGraphNode* sp_start = nullptr;
            for (const ScoreGraphNode* n = graph.start(); n; n = n->adv_edge ? n->adv_edge->dest : nullptr) {
                if (!sp_start && n->branch_edge) sp_start = n->branch_edge->dest;
                if (n->adv_edge) CHECK(n->adv_edge->dest->timecode.ticks() > n->timecode.ticks());
            }
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
// early_sqin_twice is not here: it never threw. A path on it squeezed one
// early phrase in at two SP ends; D34 ended that (a phrase is squeezed in
// only once). A variant folded there can still differ from its lone pricing:
// the search groups running paths without the phrases their window already
// squeezed in, so a variant can take its leader's squeeze of the next phrase
// where alone it would squeeze the first. Only at these tempos; open, like
// the SP-ready gap below. The chart stays for the graph test above.
TEST_CASE("fast tempo: the crash charts analyze, one SqIn step per SqIn") {
    const std::vector<std::pair<std::string, int>> charts = {
        {"node_before_phrase.chart", 3},     {"end_on_window_node.chart", 2},
        {"sqin_end_before_phrase.chart", 3}, {"late_sqin_twice.chart", 2}};
    for (const auto& [name, cap] : charts) {
        CAPTURE(name);
        const Song song = fixture(name);
        const app::AnalysisSettings cfg = fast_settings(cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        REQUIRE_FALSE(rec.paths.empty());
        for (const Path* p : rec.all_paths()) check_one_step_per_sqin(*p);
        check_banks(rec);
        std::vector<const Path*> all;
        for (const Path& r : rec.paths) collect_variants(r, all);
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
    const app::AnalysisSettings cfg = fast_settings(3);
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
                if (a.timecode.ticks() == kAct && s.kind == SpEndKind::SqIn && s.tick == kPhrase &&
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

// D34: a phrase can be squeezed in only once. An SP end offers the first
// phrase in its squeeze window that the running activation can still squeeze:
// not one banked before SP started (D18), not one this window already
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
        const app::AnalysisSettings cfg = fast_settings(2);
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
        std::vector<const Path*> all;
        for (const Path& r : rec.paths) collect_variants(r, all);
        for (const Path* v : all) {
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
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
        const app::AnalysisSettings cfg = fast_settings(fc.cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        ++analyzed;
        for (const Path* p : rec.all_paths()) check_one_step_per_sqin(*p);
        check_banks(rec);
        const bool gap_seed = std::find(kSpReadyGapSeeds.begin(), kSpReadyGapSeeds.end(),
                                        seed) != kSpReadyGapSeeds.end();
        if (gap_seed) continue;
        std::vector<const Path*> all;
        for (const Path& r : rec.paths) collect_variants(r, all);
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
        collect_variants(p, tied);
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
TEST_CASE("search_target: a path missing a named activation is dropped, the rest kept (D45)") {
    const FuzzChart fc = fuzz_chart(5);
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, true, true);
    const app::AnalysisSettings cfg = fast_settings(fc.cap);
    const std::vector<int64_t> want = {3168, 12864};

    const std::vector<Path> kept = search_target(song, cfg, want);
    CHECK(kept.size() == 5);
    CHECK(target_text(kept) ==
          "0+- 0++ 7550 | 0++- E0++ 7350 [0++- E0+- 7350] | 0++- E0- 7150 [0+- 0+- 7150] | "
          "0+- 0- 6950 | 0- 0 6750 | ");
    for (const Path& p : kept) {
        CHECK(act_ticks(p) == want);
        std::vector<const Path*> tied;
        collect_variants(p, tied);
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
// pinned below reads the same as under step 1.
TEST_CASE("search_target: a variant that took every named activation outlives its dropped root") {
    FuzzChart fc = fuzz_chart(5);
    const std::string last_phrase = "  22848 = N 1 0\n";
    const size_t at = fc.text.find(last_phrase);
    REQUIRE(at != std::string::npos);
    fc.text.insert(at + last_phrase.size(), "  23040 = N 1 0\n");
    const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
    const Song song = load_songbytes_chart(bytes, true, true);
    const app::AnalysisSettings cfg = fast_settings(fc.cap);
    const std::vector<int64_t> want = {3168, 12864};

    const std::vector<Path> kept = search_target(song, cfg, want);
    CHECK(target_text(kept) ==
          "0+- 0++ 7750 | 0++- E0++ 7550 [0++- E0+- 7550] | 0++- E0- 7350 [0+- 0+- 7350] | "
          "0+- 0- 7150 | 0- 0 6950 | ");
    const Path* promoted = nullptr;
    for (const Path& p : kept)
        if (p.pathstring() == "0+- 0++") promoted = &p;
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
    const Path* full = nullptr;
    for (const Path* p : rec.all_paths())
        if (p->pathstring() == "0+- 0++") full = p;
    REQUIRE(full != nullptr);
    CHECK(full->var_point.has_value());
    CHECK(full->totalscore() == 7750);
    CHECK(test::windows_text(*promoted) == test::windows_text(*full));
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
        const app::AnalysisSettings cfg = fast_settings(c.cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        check_banks(rec);
        int shown = 0;
        for (const Path* p : rec.all_paths()) {
            const ActivationWalk walk = p->walk_activations();
            for (size_t i = 0; i < walk.size(); ++i) {
                const Activation& a = walk[i];
                const std::optional<int64_t> d = a.deact_tick();
                if (!d) continue;
                int spent_ahead = 0;
                for (const SpEndStep& s : a.sp_end_steps)
                    if (s.kind == SpEndKind::SqIn && s.tick > *d) ++spent_ahead;
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
        std::vector<const Path*> all;
        for (const Path& r : rec.paths) collect_variants(r, all);
        for (const Path* v : all) {
            const std::string diff = lone_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}
