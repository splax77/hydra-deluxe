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
#include "fast_tempo_charts.h"
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
    return load_songpath(corpus::root() + "/test_fast_tempo/" + name,
                         ChartReading{Difficulty::Expert, true, true, false});
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

// The analysis, or a failed check naming the throw (fast_tempo_charts.h).
using test::analyzes;

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

// The reviewer's fuzz chart shape, on a fixed generator (fast_tempo_charts.h).
using test::FuzzChart;
using test::fuzz_chart;

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
TEST_CASE("fast tempo: an SP end offers the next phrase after a banked or spent one") {
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
TEST_CASE("fast tempo: a squeeze-out at an SP end gives back only the newest phrase") {
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
TEST_CASE("fast tempo: early_sqin_twice offers its window one squeeze, 13248 at 16224") {
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
// own tick. The seeds come from the fuzz generator (fast_tempo_charts.h).
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
