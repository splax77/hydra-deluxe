// Charts so fast that one SP bar lasts less than the 500 ms squeeze window
// (D32). No library chart comes close: its shortest SP bar, measured from a
// phrase chord or an SP-end node, is about 600 ms. These charts only exist to
// keep the search's own rules consistent, so it never throws and a tied
// variant still stores what a lone search stores.
//
// The fixtures in testdata/input/test_fast_tempo are fuzzed charts at 2,000
// or 4,000 BPM, untrimmed. They have no song.ini, so the corpus scan skips
// them.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/config.h"
#include "core/model.h"
#include "parse/song.h"
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

void collect_all(const Path& p, std::vector<const Path*>& out) {
    out.push_back(&p);
    for (const Path& v : p.variants) collect_all(v, out);
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

// Every window of a path written out: activation, SP-end steps, squeezes,
// squeezed-out note and backend rows.
std::string windows_text(const Path& p) {
    std::ostringstream o;
    for (const Activation& a : p.walk_activations()) {
        o << a.timecode.ticks() << " [steps";
        for (const SpEndStep& s : a.sp_end_steps)
            o << ' ' << s.tick << '>' << s.end_tick << ':' << static_cast<int>(s.kind);
        o << " | sq";
        for (const SPSqueeze& q : a.sqinouts) o << ' ' << q.symbol() << q.offset_ms;
        o << " | out " << a.sqout_tick.value_or(-1) << " | backends";
        for (const BackendSqueeze& b : a.backends)
            o << ' ' << b.timecode.ticks() << '/' << b.points << '/' << b.sqout_points << '/'
              << (b.offset_ms ? *b.offset_ms : -1.0);
        o << "] ";
    }
    return o.str();
}

// D3's promise, checked without the corpus: the variant stores what the
// search stores when it is told to activate exactly where the variant does.
// A targeted search keeps ties as variants too, and can also return paths
// that dropped an activation it could not take, so every lone path is
// looked at and only those with the variant's activations count. Returns ""
// on a match, else what differed.
std::string lone_mismatch(const Song& song, const app::AnalysisSettings& cfg, const Path& variant) {
    const std::vector<int64_t> at = act_ticks(variant);
    const ScoreGraph graph(song, std::optional<int>(graph_build_cap(cfg.sp_cap, song.sp_phrase_count())),
                           FillDeadlineRule::Ch11, cfg.rules);
    EngineOptions o;
    o.depth_mode = DepthMode::Points;
    o.depth_value = 1'000'000'000;
    o.target_act_ticks = at;
    std::vector<Path> lone;
    try {
        lone = run_search(graph, o);
    } catch (const std::exception& e) {
        return "'" + variant.pathstring() + "': the lone search threw: " + e.what();
    }
    std::vector<const Path*> all;
    for (const Path& r : lone) collect_all(r, all);
    const std::string mine = windows_text(variant);
    std::string seen;
    for (const Path* t : all) {
        if (act_ticks(*t) != at || t->totalscore() != variant.totalscore()) continue;
        const std::string theirs = windows_text(*t);
        if (theirs == mine) return "";
        seen += "\n  lone:    " + theirs;
    }
    return "'" + variant.pathstring() + "' " + std::to_string(variant.totalscore()) +
           "\n  variant: " + mine + (seen.empty() ? "\n  no lone path ties it" : seen);
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
// phrase squeezed in again at the next SP end wrote a second one).
void check_one_step_per_sqin(const Path& p) {
    for (const Activation& a : p.walk_activations()) {
        std::vector<int64_t> ticks;
        for (const SpEndStep& s : a.sp_end_steps)
            if (s.kind == SpEndKind::SqIn) ticks.push_back(s.tick);
        std::sort(ticks.begin(), ticks.end());
        CHECK_MESSAGE(std::adjacent_find(ticks.begin(), ticks.end()) == ticks.end(),
                      p.pathstring() << " at " << a.timecode.ticks());
    }
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
// early_sqin_twice is not here: it never threw. A path on it squeezes one
// early phrase in at two SP ends, and a variant folded between the two
// stores no SqIn step for its SqIn. That double squeeze also happens on two
// library charts, so changing it is a user decision (open, reported with
// D32's fixes); the chart stays for the graph test above.
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
    CHECK(analyzed == 48);
    CHECK(variants > 50);
}
