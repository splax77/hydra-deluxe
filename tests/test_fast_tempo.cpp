// Charts so fast that one SP bar lasts less than the 500 ms squeeze window
// (D32). No library chart comes close: its shortest SP bar, measured from a
// phrase chord or an SP-end node, is about 600 ms. These charts only exist to
// keep the search's own rules consistent, so it never throws, a tied variant
// still stores what a lone search stores, and no bar is banked on a phrase a
// squeeze-in spent.
//
// The fixtures in testdata/input/test_fast_tempo run at 2,000 or 4,000 BPM.
// Most are fuzzed charts, untrimmed; the two named in the "next phrase" test
// are hand-made. They have no song.ini, so the corpus scan skips them.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
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
    return load_songpath(corpus::root() + "/test_fast_tempo/" + name, ChartReading{});
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
// checked. The order check also catches a squeezed-out bar landing on the
// real deact node past the record's deact_tick().
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
// bar is shorter than the squeeze window) must still land ahead of that
// phrase. A node behind it would take the phrase on its incoming edge, and a
// path folded there would hold a step past its fold (the close_folded_act
// throw). Both tracks must run forward in time, and each phrase must sit on
// the edge that reaches it.
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

// The throw each fixture guards against (D32):
//   node_before_phrase      "a folded variant holds a step past its fold"
//   end_on_window_node      "search reached a broken state": a path whose SP
//                           ends on a node with a squeezable phrase in its
//                           window must deactivate there
//   sqin_end_before_phrase  the same throw: a late squeeze-in's end, one bar
//                           past the end it moves, falls before its own phrase
//                           and needs a node
//   late_sqin_twice         the same throw: a late squeeze-in's phrase must
//                           not be squeezed in again at the next SP end
// early_sqin_twice guards two rules, not a throw. A phrase is squeezed in only
// once (D34). A variant folded there must match its lone pricing (D36 gap b):
// the search groups running paths without the phrases their window already
// squeezed in, so an SP end offers only the newest phrase, which every path at
// one node shares.
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

// Engine::advance's off-SP branch, where a spent phrase "stays spent". A late
// squeeze-in whose new SP end falls before its own phrase leaves SP first,
// with an empty meter and the phrase still spent. When the graph has a node
// between that end and the phrase, the path then walks an edge without the
// phrase, and the phrase stays spent. Hitting the phrase later must add
// nothing, so no bank list of that path holds the phrase (a tick there would
// light a bar the SqIn already used). Every such step is read from the
// record's own SqIn steps; sqin_end_before_phrase.chart at cap 3 is the
// fixture made to reach the case, so the test fails when no stored path
// walks off SP between such an end and its phrase.
TEST_CASE("fast tempo: a phrase a SqIn spent banks no bar when SP ends before it") {
    const Song song = fixture("sqin_end_before_phrase.chart");
    const app::AnalysisSettings cfg = scores_settings(3);

    const ScoreGraph graph(song, std::optional<int>(graph_build_cap(cfg.sp_cap, song.sp_phrase_count())),
                           FillDeadlineRule::Ch11, cfg.rules);
    // Does the graph's track hold a node strictly between `from` and `to`?
    const auto node_between = [&graph](int64_t from, int64_t to) {
        for (const ScoreGraphNode* n = graph.start(); n; n = n->adv_edge ? n->adv_edge->dest : nullptr) {
            const int64_t t = n->timecode.ticks();
            if (t > from && t < to) return true;
        }
        return false;
    };

    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    int walked_off_sp = 0;
    for (const Path* p : rec.all_paths()) {
        CAPTURE(p->pathstring());
        for (const Activation& a : p->walk_activations())
            for (const SpEndStep& s : a.sp_end_steps) {
                if (!is_sqin_kind(s.kind) || s.end_tick >= s.tick) continue;
                CAPTURE(a.timecode.ticks());
                CAPTURE(s.tick);
                CAPTURE(s.end_tick);
                if (node_between(s.end_tick, s.tick)) ++walked_off_sp;
                for (const Activation& b : p->walk_activations())
                    CHECK(std::count(b.bank_rise_ticks.begin(), b.bank_rise_ticks.end(), s.tick) == 0);
                CHECK(std::count(p->trailing_bank_ticks.begin(), p->trailing_bank_ticks.end(), s.tick) == 0);
            }
    }
    CHECK(walked_off_sp >= 1);
}

// D34: a phrase can be squeezed in only once. A phrase banked before SP
// started is never offered (D18), and neither is one this window already
// squeezed in: an SP end offers the window's newest phrase before it (D36),
// or, when the window's end is this end, the first phrase after it not yet
// squeezed in. Both hand-made charts run at 4,000 BPM from tick 9600, so
// 500 ms spans about four SP bars. The activation is on 12288 with 2 bars.
//   banked_then_next  the phrase on 12000 is banked before the activation and
//                     sits in the window of the SP end 15360. The next phrase,
//                     16000, comes after that end. The banked phrase must not
//                     hide it.
//   spent_then_next   the phrase on 17360 is squeezed in late at the SP end
//                     15360, which moves the end to 16896, still before it.
//                     The next phrase, 18360, sits in the window of 16896.
//                     The spent phrase must not hide it.
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

// D36 gap a. A squeeze-out gives back a phrase the player hits after SP ran
// out. Every phrase the same window collected was hit while SP still ran, so
// it comes before the squeezed-out phrase: a player who hits an earlier
// phrase after SP ran out also hits every later one after SP ran out. So a
// window's squeezed-out phrase is later than each phrase it collected, and a
// squeeze-out of an older phrase (dropping a newer phrase's SP-end step) is
// the bug. node_before_phrase.chart at cap 3 is the fixture made to reach a
// squeeze-out in a window that collected phrases, so the test fails when no
// stored path shows one.
TEST_CASE("fast tempo: a squeeze-out at an SP end gives back only the newest phrase") {
    const Song song = fixture("node_before_phrase.chart");
    const app::AnalysisSettings cfg = scores_settings(3);
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    int after_collected = 0;
    for (const Path* p : rec.all_paths()) {
        CAPTURE(p->pathstring());
        for (const Activation& a : p->walk_activations()) {
            if (!a.sqout_tick) continue;
            CAPTURE(a.timecode.ticks());
            CAPTURE(*a.sqout_tick);
            bool collected_any = false;
            for (const SpEndStep& s : a.sp_end_steps) {
                if (s.kind != SpEndKind::Collected && !is_clamp_kind(s.kind)) continue;
                collected_any = true;
                CAPTURE(s.tick);
                CHECK(s.tick < *a.sqout_tick);
            }
            if (collected_any) ++after_collected;
        }
    }
    CHECK(after_collected >= 1);
}

// D36 gap b. The search groups running paths without the phrases their
// window already squeezed in, so an SP end offers one phrase, which every
// path in the same state shares, in or out. The state is the window's
// activation and the SP-end steps it took before the squeeze: a squeeze-in's
// steps before its SqIn step, or a squeeze-out's whole list (its SP end is
// the last step's end). Each such state must offer one phrase only, read from
// the record. early_sqin_twice.chart at cap 2 is the fixture made to reach a
// state offered both ways, so the test fails when no state shows both.
TEST_CASE("fast tempo: early_sqin_twice offers each SP end one squeeze, in or out") {
    const Song song = fixture("early_sqin_twice.chart");
    const app::AnalysisSettings cfg = scores_settings(2);
    HydraRecord rec;
    REQUIRE(analyzes(song, cfg, rec));
    using StepKey = std::tuple<int64_t, int64_t, int>;
    using StateKey = std::pair<int64_t, std::vector<StepKey>>;
    struct Offer {
        std::set<int64_t> phrases;
        bool in = false, out = false;
    };
    std::map<StateKey, Offer> offers;
    for (const Path* p : rec.all_paths()) {
        for (const Activation& a : p->walk_activations()) {
            StateKey state{a.timecode.ticks(), {}};
            for (const SpEndStep& s : a.sp_end_steps) {
                if (is_sqin_kind(s.kind)) {
                    Offer& o = offers[state];
                    o.phrases.insert(s.tick);
                    o.in = true;
                }
                state.second.emplace_back(s.tick, s.end_tick, static_cast<int>(s.kind));
            }
            if (!a.sqout_tick) continue;
            Offer& o = offers[state];
            o.phrases.insert(*a.sqout_tick);
            o.out = true;
        }
    }
    int both_ways = 0;
    for (const auto& [state, o] : offers) {
        CAPTURE(state.first);
        CAPTURE(state.second.size());
        CAPTURE(*o.phrases.begin());
        CAPTURE(*o.phrases.rbegin());
        CHECK(o.phrases.size() == 1);
        if (o.in && o.out) ++both_ways;
    }
    CHECK(both_ways >= 1);
}

// The fuzz runs seeds 1 to kFuzzSeeds on its fixed generator, so the charts
// are the same on every build.
constexpr uint64_t kFuzzSeeds = 48;

// A small deterministic fuzz: every chart analyzes, keeps one SqIn step per
// SqIn, and every stored path prices as it does alone ("prices": gets the
// same score and stored facts as a search told to take its activations). A
// path the lone search also folds has no oracle and is not judged
// (lone_pricing_mismatch), so each chart must have at least one path that
// was judged, or its pricing went unchecked.
TEST_CASE("fast tempo: fuzzed charts analyze and their variants price as alone") {
    uint64_t analyzed = 0;
    for (uint64_t seed = 1; seed <= kFuzzSeeds; ++seed) {
        CAPTURE(seed);
        const FuzzChart fc = fuzz_chart(seed);
        const std::vector<uint8_t> bytes(fc.text.begin(), fc.text.end());
        const Song song = load_songbytes_chart(bytes, ChartReading{});
        const app::AnalysisSettings cfg = scores_settings(fc.cap);
        HydraRecord rec;
        if (!analyzes(song, cfg, rec)) continue;
        ++analyzed;
        for (const Path* p : rec.all_paths()) check_one_step_per_sqin(*p);
        check_record_banks(song, rec);
        int judged = 0, no_oracle = 0;
        for (const Path* p : rec.all_paths()) {
            test::LonePricing lone;
            try {
                lone = test::lone_pricing(song, cfg, *p);
            } catch (const std::exception& e) {
                FAIL_CHECK("seed " << seed << " '" << p->pathstring()
                                   << "': the lone search threw: " << std::string(e.what()));
                continue;
            }
            if (lone.tied_under_root) {
                ++no_oracle;
                continue;
            }
            ++judged;
            CHECK_MESSAGE(lone.diff.empty(), "seed " << seed << " " << lone.diff);
        }
        CAPTURE(no_oracle);
        CHECK(judged >= 1);
    }
    // No seed was skipped: each one analyzed.
    CHECK(analyzed == kFuzzSeeds);
}

// The phrases still ahead when SP ends, pinned. D34 lets one
// window squeeze in two late phrases that both come after its final SP end:
// the path leaves SP with two spent phrases ahead and an empty meter. Its
// SqOut sibling squeezes a third phrase out at that end instead, so it leaves
// SP with two spent phrases and one banked bar ahead. The engine keeps spent
// and banked phrases apart, so the first edge after either end must not throw
// "the SP meter went below zero off SP". The record shows both states: SqIn steps
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
        const Song song = load_songbytes_chart(bytes, ChartReading{});
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
