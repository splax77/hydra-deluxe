// Finding 37 (decision D29): when a phrase is collected during Star Power,
// the SP end moves by extend_deacts' rule, which stops at the cap's ceiling,
// as Clone Hero does (0x20F6800). The deactivation edge's squeeze choices
// must read that same answer, and must not hand a clamped squeeze-out to a
// path whose end before the phrase was a different tick.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <vector>

#include "core/model.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

using namespace hydra;

namespace {

using test::TailNote;

// 4/4 at 2400 BPM with 192 ticks per beat: a measure is 768 ticks and
// 100 ms, so an SP bar (two measures) is 200 ms and fits inside the 500 ms
// squeeze window. Only then can the cap clamp an end a squeeze-out still
// reaches.
Song build_fast_song(const std::vector<TailNote>& notes) {
    return test::build_tempo_song(notes, {{0, 2400.0}});
}

// Two phrases bank 2 bars; the activation at 2304 ends 4 measures later at
// 5376. The phrase at 3456 is collected mid-SP, 250 ms before 5376. A plain
// bar would move the end to 5376 + 1536 = 6912, but at cap 2 the ceiling is
// 3456 + 4 * 768 = 6528, which is earlier, so the end clamps to 6528.
const std::vector<TailNote> kOneActivation = {
    {0, true}, {768, true}, {2304, false, true}, {3456, true},
    {4608}, {6000}, {6528}, {7000}, {7680}};

// The same, plus a second fill at 2496. Activating there instead ends at
// 5568, also inside the window after 3456, and that end clamps to 6528 too.
const std::vector<TailNote> kTwoActivations = {
    {0, true}, {768, true}, {2304, false, true}, {2496, false, true},
    {3456, true}, {4608}, {6000}, {6528}, {7000}, {7680}};

// The deactivation edge whose node sits at `tick`, walking the SP track the
// way test_search.cpp's squeeze-window test does.
const ScoreGraphEdge* deact_edge_at(const ScoreGraph& graph, int64_t tick) {
    const ScoreGraphNode* sp = nullptr;
    for (const ScoreGraphNode* b = graph.start(); b && !sp;
         b = b->adv_edge ? b->adv_edge->dest : nullptr)
        if (b->branch_edge) sp = b->branch_edge->dest;
    for (; sp; sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
        const ScoreGraphEdge* e = sp->branch_edge;
        if (e && e->dest->timecode.ticks() == tick) return e;
    }
    return nullptr;
}

// The edge's squeeze choice on the phrase chord at `chord_tick`, or nullptr.
const SqueezeChoice* choice_at(const ScoreGraphEdge& e, int64_t chord_tick) {
    for (const SqueezeChoice& c : e.squeeze_choices)
        if (c.chord.ticks() == chord_tick) return &c;
    return nullptr;
}

struct SqOutSeen {
    int64_t act_tick;
    int64_t end_tick;   // the record's SP end (the trimmed step list)
    double sqout_ms;    // the squeezed-out row's offset from the deact node
};

// Every activation, in every returned path and its tied variants, that
// squeezed out the phrase at `phrase_tick`: where it activated, where its
// record says Star Power ended, and how far the squeezed-out chord sat from
// the deactivation node the engine actually used. A squeeze-out trims the
// steps from the phrase on, so the record's end is always the end before the
// phrase; only the row's offset shows which node the engine deactivated at.
std::vector<SqOutSeen> sqouts_of(const std::vector<Path>& paths, int64_t phrase_tick) {
    std::vector<SqOutSeen> out;
    auto scan = [&](const Path& p) {
        for (const Activation& act : p.all_activations()) {
            if (!act.sqout_tick || *act.sqout_tick != phrase_tick) continue;
            const std::optional<int64_t> end = act.deact_tick();
            REQUIRE(end.has_value());
            const BackendSqueeze* row = act.sqout_row();
            REQUIRE(row != nullptr);
            REQUIRE(row->offset_ms.has_value());
            out.push_back({act.timecode.ticks(), *end, *row->offset_ms});
        }
    };
    for (const Path& p : paths) {
        scan(p);
        for (const Path& v : p.variants) scan(v);
    }
    return out;
}

EngineOptions keep_losers() {
    EngineOptions o;
    o.depth_mode = DepthMode::Scores;
    o.depth_value = 50;  // keep the squeeze-out even though it scores lower
    return o;
}

}  // namespace

TEST_CASE("finding 37: the squeeze-out edge expects extend_deacts' clamped end") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const ScoreGraphEdge* e = deact_edge_at(graph, 5376);
    REQUIRE(e != nullptr);
    const SqueezeChoice* c = choice_at(*e, 3456);
    REQUIRE(c != nullptr);
    CHECK_FALSE(c->late);
    // min(5376 + 1536, 3456 + 4 * 768) = 6528, and the ceiling won.
    CHECK(c->sqout_time.ticks() == 6528);  // was 6912
    CHECK(c->sqin_time.ticks() == 6528);
    CHECK(c->clamped);
}

TEST_CASE("finding 37: a clamped window still offers its squeeze-out") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, keep_losers()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) {
        CHECK(s.act_tick == 2304);
        CHECK(s.end_tick == 5376);
        CHECK(s.sqout_ms == song.timecode(3456).ms() - song.timecode(5376).ms());
    }
}

TEST_CASE("finding 37: two ends clamped to one tick each keep their own squeeze-out") {
    const Song song = build_fast_song(kTwoActivations);
    const ScoreGraph graph(song, 2);
    for (int64_t act : {int64_t{2304}, int64_t{2496}}) {
        // A targeted search: only this activation exists, so no other path
        // can crowd it out of its group, and a wrong squeeze-out would show.
        EngineOptions o = keep_losers();
        o.target_act_ticks = std::vector<int64_t>{act};
        const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, o), 3456);
        const int64_t own_end = act == 2304 ? 5376 : 5568;
        INFO("activation at " << act);
        REQUIRE_FALSE(seen.empty());
        for (const SqOutSeen& s : seen) {
            CHECK(s.act_tick == act);
            CHECK(s.end_tick == own_end);
            // Deactivated at its own end, never at the other activation's:
            // the 2496 window squeezing out at 5376 is the bug finding 37's
            // guard stops (its record would still say 5568).
            CHECK(s.sqout_ms == song.timecode(3456).ms() - song.timecode(own_end).ms());
        }
    }
}

// Two paths tie on score and on their clamped end, but came from different
// ends. Phrases at 0 and 768 bank 2 bars. Path B activates at the fill at
// 2304 (its only note under SP), banks the phrases at 9600 and 10368, and
// activates again at 14592: SP ends at 17664. Path A passes 2304 (the cap
// wastes 9600 and 10368) and activates at 14208: SP ends at 17280. Both
// collect the phrase at 14976 with a full meter, and both ends clamp to
// 14976 + 4 * 768 = 18048. A doubled 14208, 14592 and 14976; B doubled 2304,
// 14592 and 14976; every note is one red gem under the 10-note multiplier,
// so their scores tie exactly. Folded as tied paths, the variant could only
// follow the leader's squeeze choices, and its own squeeze-out, back to its
// own end, would be lost (finding 37).
const std::vector<TailNote> kTiedClamps = {
    {0, true},     {768, true},          {2304, false, true}, {9600, true},
    {10368, true}, {14208, false, true}, {14592, false, true}, {14976, true},
    {15744},       {18048},              {18816},             {19584}};

TEST_CASE("finding 37: tied paths clamped from different ends keep their own squeeze-outs") {
    const Song song = build_fast_song(kTiedClamps);
    const ScoreGraph graph(song, 2);
    const std::vector<Path> paths = run_search(graph, keep_losers());
    // Path A is the lone activation at 14208; path B activates at 2304 and
    // 14592. Each must squeeze 14976 out, back to its own end only.
    struct Want {
        std::vector<int64_t> acts;
        int64_t own_end;
        bool seen;
    };
    Want want[] = {{{14208}, 17280, false}, {{2304, 14592}, 17664, false}};
    auto scan = [&](const Path& p) {
        const std::vector<Activation> acts = p.all_activations();
        std::vector<int64_t> ticks;
        for (const Activation& a : acts) ticks.push_back(a.timecode.ticks());
        for (Want& w : want) {
            if (ticks != w.acts) continue;
            const Activation& last = acts.back();
            if (!last.sqout_tick) continue;
            INFO("window at " << w.acts.back());
            CHECK(*last.sqout_tick == 14976);
            CHECK(last.deact_tick() == std::optional<int64_t>(w.own_end));
            REQUIRE(last.sqout_row() != nullptr);
            CHECK(*last.sqout_row()->offset_ms ==
                  song.timecode(14976).ms() - song.timecode(w.own_end).ms());
            w.seen = true;
        }
    };
    for (const Path& p : paths) {
        scan(p);
        for (const Path& v : p.variants) scan(v);
    }
    CHECK(want[0].seen);
    CHECK(want[1].seen);
}

TEST_CASE("finding 37: an unclamped window is unchanged") {
    // At cap 3 the ceiling is 3456 + 6 * 768 = 8064, later than 6912, so
    // nothing clamps and the plain bar is extend_deacts' answer too.
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 3);
    const ScoreGraphEdge* e = deact_edge_at(graph, 5376);
    REQUIRE(e != nullptr);
    const SqueezeChoice* c = choice_at(*e, 3456);
    REQUIRE(c != nullptr);
    CHECK(c->sqout_time.ticks() == 6912);
    CHECK(c->sqin_time.ticks() == 6912);
    CHECK_FALSE(c->clamped);
    const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, keep_losers()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) {
        CHECK(s.end_tick == 5376);
        CHECK(s.sqout_ms == song.timecode(3456).ms() - song.timecode(5376).ms());
    }
}
