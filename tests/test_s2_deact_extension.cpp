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
#include "search/pather.h"

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

using test::deact_edge_at;

// The edge's squeeze choice on the phrase chord at `chord_tick`, or nullptr.
const SqueezeChoice* choice_at(const ScoreGraphEdge& e, int64_t chord_tick) {
    for (const SqueezeChoice& c : e.squeeze_choices)
        if (c.chord.ticks() == chord_tick) return &c;
    return nullptr;
}

using test::extension_of;

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
    for (const Path* p : flatten_paths(paths)) {
        for (const Activation& act : p->all_activations()) {
            if (!act.sqout_tick || *act.sqout_tick != phrase_tick) continue;
            const std::optional<int64_t> end = act.deact_tick();
            REQUIRE(end.has_value());
            const BackendSqueeze* row = act.sqout_row();
            REQUIRE(row != nullptr);
            REQUIRE(row->offset_ms.has_value());
            out.push_back({act.timecode.ticks(), *end, *row->offset_ms});
        }
    }
    return out;
}

// A squeeze offset in ms, pinned. At 2400 BPM a tick is 25/192 ms, so the
// offsets below are whole numbers.
doctest::Approx ms(double v) { return doctest::Approx(v).epsilon(1e-9); }

}  // namespace

// D36 rewrote this case: the edge's own sqout_time and clamped flag are
// gone, because where the end moved now has one answer, the path's own step.
// So the step's facts are pinned: end 6528, Clamped, moved from 5376, which
// can give 3456 back.
TEST_CASE("finding 37: the squeeze-out edge expects extend_deacts' clamped end") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const ScoreGraphEdge* e = deact_edge_at(graph, 5376);
    REQUIRE(e != nullptr);
    const SqueezeChoice* c = choice_at(*e, 3456);
    REQUIRE(c != nullptr);
    CHECK_FALSE(c->late);
    // min(5376 + 1536, 3456 + 4 * 768) = 6528, and the ceiling won.
    const std::optional<SpExtension> x = extension_of(graph, 3456, 5376);
    REQUIRE(x.has_value());
    CHECK(x->to_tick == 6528);  // was 6912
    CHECK(x->clamped);
    CHECK(x->sqout_node);
    CHECK(c->sqin_time.ticks() == 6528);
}

TEST_CASE("finding 37: a clamped window still offers its squeeze-out") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const std::vector<SqOutSeen> seen =
        sqouts_of(run_search(graph, test::wide_search()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) {
        CHECK(s.act_tick == 2304);
        CHECK(s.end_tick == 5376);
        CHECK(s.sqout_ms == ms(-250.0));  // 3456 against 5376
    }
}

TEST_CASE("finding 37: two ends clamped to one tick each keep their own squeeze-out") {
    const Song song = build_fast_song(kTwoActivations);
    for (int64_t act : {int64_t{2304}, int64_t{2496}}) {
        // A targeted search (search_target): only this activation exists, so
        // no other path can crowd it out of its group, and a wrong squeeze-out
        // would show.
        const std::vector<SqOutSeen> seen =
            sqouts_of(search_target(song, test::scores_settings(2), {act}), 3456);
        const int64_t own_end = act == 2304 ? 5376 : 5568;
        const double own_ms = act == 2304 ? -250.0 : -275.0;  // 3456 against own_end
        INFO("activation at " << act);
        REQUIRE_FALSE(seen.empty());
        for (const SqOutSeen& s : seen) {
            CHECK(s.act_tick == act);
            CHECK(s.end_tick == own_end);
            // Deactivated at its own end, never at the other activation's:
            // the 2496 window squeezing out at 5376 is the bug finding 37's
            // guard stops (its record would still say 5568).
            CHECK(s.sqout_ms == ms(own_ms));
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
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    // Path A is the lone activation at 14208; path B activates at 2304 and
    // 14592. Each must squeeze 14976 out, back to its own end only.
    struct Want {
        std::vector<int64_t> acts;
        int64_t own_end;
        double own_ms;  // 14976 against own_end
        bool seen;
    };
    Want want[] = {{{14208}, 17280, -300.0, false}, {{2304, 14592}, 17664, -350.0, false}};
    for (const Path* p : flatten_paths(paths)) {
        const std::vector<int64_t> ticks = test::act_ticks(*p);
        for (Want& w : want) {
            if (ticks != w.acts) continue;
            const Activation last = p->all_activations().back();
            if (!last.sqout_tick) continue;
            INFO("window at " << w.acts.back());
            CHECK(*last.sqout_tick == 14976);
            CHECK(last.deact_tick() == std::optional<int64_t>(w.own_end));
            REQUIRE(last.sqout_row() != nullptr);
            CHECK(*last.sqout_row()->offset_ms == ms(w.own_ms));
            w.seen = true;
        }
    }
    CHECK(want[0].seen);
    CHECK(want[1].seen);
}

// One end moves a plain bar onto the cap's ceiling; a later end clamps onto
// the same tick. Phrases at 0 and 768 bank 2 bars. Activating at 1920 ends
// SP at 4992; collecting 3456 moves it 4992 + 1536 = 6528, which ties the
// cap-2 ceiling 3456 + 4 * 768, so it is not clamped. Activating at 2112 ends
// at 5184; collecting 3456 clamps it to the same 6528. Both ends hold 3456 in
// their squeeze window (200 and 225 ms).
const std::vector<TailNote> kPlainTiesClamp = {
    {0, true},  {768, true}, {1920, false, true}, {2112, false, true}, {3456, true},
    {4608},     {6000},      {6528},              {7000},              {7680}};

TEST_CASE("finding 37: a plain bar tying the ceiling squeezes out only for its own end") {
    const Song song = build_fast_song(kPlainTiesClamp);
    const ScoreGraph graph(song, 2);
    const ScoreGraphEdge* plain = deact_edge_at(graph, 4992);
    const ScoreGraphEdge* clamp = deact_edge_at(graph, 5184);
    REQUIRE(plain != nullptr);
    REQUIRE(clamp != nullptr);
    const SqueezeChoice* pc = choice_at(*plain, 3456);
    const SqueezeChoice* cc = choice_at(*clamp, 3456);
    REQUIRE(pc != nullptr);
    REQUIRE(cc != nullptr);
    // Both steps move to 6528; only one is clamped. Each step names its own
    // end, which is where its squeeze is offered (D36).
    const std::optional<SpExtension> px = extension_of(graph, 3456, 4992);
    const std::optional<SpExtension> cx = extension_of(graph, 3456, 5184);
    REQUIRE(px.has_value());
    REQUIRE(cx.has_value());
    CHECK(px->to_tick == 6528);
    CHECK_FALSE(px->clamped);
    CHECK(px->sqout_node);
    CHECK(cx->to_tick == 6528);
    CHECK(cx->clamped);
    CHECK(cx->sqout_node);

    for (int64_t act : {int64_t{1920}, int64_t{2112}}) {
        const std::vector<Path> paths = search_target(song, test::scores_settings(2), {act});
        const int64_t own_end = act == 1920 ? 4992 : 5184;
        const double own_ms = act == 1920 ? -200.0 : -225.0;  // 3456 against own_end
        INFO("activation at " << act);
        const std::vector<SqOutSeen> seen = sqouts_of(paths, 3456);
        // The 2112 window was once offered the 4992 squeeze-out (-200 ms,
        // record still 5184), and its SqIn branch then spent 3456, so its
        // own squeeze-out at 5184 never came.
        REQUIRE_FALSE(seen.empty());
        for (const SqOutSeen& s : seen) {
            CHECK(s.act_tick == act);
            CHECK(s.end_tick == own_end);
            CHECK(s.sqout_ms == ms(own_ms));
        }
        // Every squeeze this window shows, in or out, is 3456 measured from
        // its own end: the fake SqIn sat at -200 ms on the 2112 window.
        for (const Path* p : flatten_paths(paths))
            for (const Activation& a : p->all_activations()) {
                REQUIRE(a.timecode.ticks() == act);
                for (const SPSqueeze& q : a.sqinouts) {
                    CAPTURE(q.type_name());
                    CHECK(q.offset_ms == ms(own_ms));
                }
            }
    }
}

// Two paths end at one tick by different routes, then both clamp the same
// phrase from that same end, so they fold mid-SP as tied paths; the leader
// then squeezes that phrase in. 1200 BPM: a measure is 768 ticks and 200 ms.
//  - B activates at 2304, doubling 2304 and 3072, and rebanks 6144 and 6912.
//  - A passes 2304 (the cap wastes 6144 and 6912) and activates at 9216 (end
//    12288). 9984 clamps it to 9984 + 3072 = 13056; 11520 then moves it a
//    plain bar to 14592, tying the ceiling 11520 + 3072.
//  - B passes 9216 (the cap wastes 9984) and activates at 10752 (end 13824);
//    11520 clamps it to 14592.
// A doubled 9216 and 9984 where B doubled 2304 and 3072, all under the
// 10-note multiplier, so they tie. After 11520 they stay apart: the 13056
// edge offers 11520 to A alone. 12864 then clamps both from 14592 to 15936;
// it sits 450 ms before 14592, and 11520 sits 800 ms before, outside that
// window. Same state, same score: they fold at 12864, and the 14592 edge
// offers the leader 12864 (squeeze_window_phrases).
const std::vector<TailNote> kFoldSharedClamp = {
    {0, true},     {768, true},          {2304, false, true}, {3072},
    {6144, true},  {6912, true},         {9216, false, true}, {9984, true},
    {10752, false, true},                {11520, true},       {12864, true},
    {13824},       {16896},              {17664},             {18432}};

TEST_CASE("finding 37: a folded variant's Clamped step on its leader's SqIn phrase becomes SqIn") {
    const Song song = test::build_tempo_song(kFoldSharedClamp, {{0, 1200.0}});
    const ScoreGraph graph(song, 2);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const SpEndStep sqin{12864, 15936, SpEndKind::SqIn};
    // The variant's own steps run to the fold, so its 12864 step is its own
    // Clamped step; close_folded_act relabels it as the leader's SqIn
    // (is_sqin_step), the way relabel_sqin does on a lone path.
    auto step_at = [](const Path& p) -> std::optional<SpEndStep> {
        const std::vector<Activation> acts = p.all_activations();
        if (acts.empty()) return std::nullopt;
        for (const SpEndStep& s : acts.back().sp_end_steps)
            if (s.tick == 12864) return s;
        return std::nullopt;
    };
    const std::vector<int64_t> a_acts = {9216};
    const std::vector<int64_t> b_acts = {2304, 10752};
    bool folded = false;
    // Every returned path that carries tied variants, nested ones included.
    for (const Path* p : flatten_paths(paths)) {
        const std::vector<int64_t> lead = test::act_ticks(*p);
        if (lead != a_acts && lead != b_acts) continue;
        if (step_at(*p) != std::optional<SpEndStep>(sqin)) continue;
        for (const Path& v : p->variants) {
            const std::vector<int64_t> own = test::act_ticks(v);
            if (own != (lead == a_acts ? b_acts : a_acts)) continue;
            folded = true;
            CAPTURE(p->pathstring());
            CAPTURE(v.pathstring());
            CHECK(step_at(v) == std::optional<SpEndStep>(sqin));
        }
    }
    // The fold happened: the leader squeezed 12864 in and carries the other
    // route as its tied variant.
    CHECK(folded);
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
    const std::optional<SpExtension> x = extension_of(graph, 3456, 5376);
    REQUIRE(x.has_value());
    CHECK(x->to_tick == 6912);
    CHECK_FALSE(x->clamped);
    CHECK(c->sqin_time.ticks() == 6912);
    const std::vector<SqOutSeen> seen =
        sqouts_of(run_search(graph, test::wide_search()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) {
        CHECK(s.end_tick == 5376);
        CHECK(s.sqout_ms == ms(-250.0));  // 3456 against 5376
    }
}
