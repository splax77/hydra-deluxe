// Tied variants keep their own state (decision D3): a path the search folds
// into a tied leader keeps its own steps, squeezes, banked bars and early
// fills. The squeeze rules these folds exposed (D34, D36, D38) are pinned
// here too, on the hand-made charts in testdata/input/test_folded_sqin.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "bank_check.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

namespace {

// A hand-built 4/4 120 BPM song (record_fixtures.h owns the note type and the
// song builder); a tick's ms (also record_fixtures.h).
using test::TailNote;
using test::build_tail_song;
using test::tick_ms;

// The audit's constructed chart for finding 90 ("midsp"), 120 BPM 4/4, run at
// cap 2. '1' activates at 10752; '0 1' activates at 4608, then at 11520. The
// phrase at 11904 clamps both windows to end at 14976, on the same SP node
// with the same score, so the search folds '0 1' into '1' there. The phrase
// at 13056 then clamps the window again, to 16128: after the fold.
//
// with_squeeze adds four notes. The phrase at 11328 lands in '1''s SP but
// before '0 1' activates, so only the leader collects it; the note at 6144
// keeps the two tied. The note at 16032 and the phrase at 16224 sit 250 ms
// either side of the SP end at 16128, so the leader splits there into '1+'
// (SqIn) and '1-' (SqOut), and the variant hangs off both.
std::vector<TailNote> midsp_notes(bool with_squeeze) {
    std::vector<TailNote> n;
    for (int64_t t = 0; t < 3072; t += 96) n.push_back({t, t == 768 || t == 2304, false});
    n.push_back({4608, false, true});
    if (with_squeeze) n.push_back({6144});
    n.push_back({8448, true, false});
    n.push_back({9216, true, false});
    n.push_back({10752, false, true});
    if (with_squeeze) n.push_back({11328, true, false});
    n.push_back({11520, false, true});
    n.push_back({11904, true, false});
    n.push_back({12672, false, true});
    n.push_back({13056, true, false});
    n.push_back({15360});
    if (with_squeeze) {
        n.push_back({16032});
        n.push_back({16224, true, false});
    }
    n.push_back({16896});
    n.push_back({17664});
    n.push_back({18432});
    return n;
}

// Every path of a search's output, each before its variants, is
// flatten_paths (model.h); the root named s is test::path_named
// (record_fixtures.h).
// The leader's variant whose own last activation is at `tick`.
const Path* variant_at(const Path& leader, int64_t tick) {
    for (const Path& v : leader.variants)
        if (!v.activations.empty() && v.activations.back().timecode.ticks() == tick)
            return &v;
    return nullptr;
}
using Step = std::tuple<int64_t, int64_t, SpEndKind>;
std::vector<Step> steps_of(const Activation& a) {
    std::vector<Step> out;
    for (const SpEndStep& s : a.sp_end_steps) out.emplace_back(s.tick, s.end_tick, s.kind);
    return out;
}

// D3's promise for a tied variant (record_fixtures.h).
using test::lone_pricing_mismatch;
// A root's tied variants, depth-first (record_fixtures.h).
using test::collect_tied;

}  // namespace

using test::deact_edge_at;

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's steps after the fold") {
    const Song song = build_tail_song(midsp_notes(false));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* leader = test::path_named(roots, "1");
    REQUIRE(leader != nullptr);
    const Path* variant = variant_at(*leader, 11520);
    REQUIRE(variant != nullptr);
    CHECK(variant->totalscore() == leader->totalscore());

    const Activation& mine = variant->activations.back();
    const Activation& lead = leader->activations.back();
    CHECK(steps_of(lead) == std::vector<Step>{{10752, 13824, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    // Its own activation step and its own clamp at the fold, then the
    // leader's clamp at 13056. Today the list stops at the fold: 14976.
    CHECK(steps_of(mine) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    CHECK(mine.deact_tick() == std::optional<int64_t>(16128));
    CHECK(mine.clamp_tick() == std::optional<int64_t>(13056));
    CHECK(mine.collected_phrase_ticks() == std::vector<int64_t>{11904, 13056});
    // The real deactivation's rows, not the song's last notes.
    CHECK(mine.backends == lead.backends);
    CHECK(mine.sqinouts.empty());
    CHECK(variant->pathstring() == "0 1");

    for (const Path* p : flatten_paths(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's closing SqIn or SqOut") {
    const Song song = build_tail_song(midsp_notes(true));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* in_lead = test::path_named(roots, "1+");
    const Path* out_lead = test::path_named(roots, "1-");
    REQUIRE(in_lead != nullptr);
    REQUIRE(out_lead != nullptr);
    const Path* in_var = variant_at(*in_lead, 11520);
    const Path* out_var = variant_at(*out_lead, 11520);
    REQUIRE(in_var != nullptr);
    REQUIRE(out_var != nullptr);

    // The leader collected 11328 in SP. The variant did not: it banked that
    // phrase before activating at 11520, so the step is the leader's alone.
    CHECK(in_lead->activations.back().collected_phrase_ticks() ==
          std::vector<int64_t>{11328, 11904, 13056, 16224});

    // The SqIn side: a late SqIn on the phrase at 16224 moves the end to 17664.
    const Activation& a = in_var->activations.back();
    CHECK(steps_of(a) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped},
                                           {16224, 17664, SpEndKind::SqIn}});
    REQUIRE(a.sqinouts.size() == 1);
    CHECK(a.sqinouts[0].kind == SqueezeKind::SqIn);
    CHECK(in_var->pathstring() == "0 1+");

    // The SqOut side: SP ends at 16128 and the phrase at 16224 has no step.
    const Activation& b = out_var->activations.back();
    CHECK(steps_of(b) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped}});
    REQUIRE(b.sqinouts.size() == 1);
    CHECK(b.sqinouts[0].kind == SqueezeKind::SqOut);
    CHECK(b.sqout_tick == std::optional<int64_t>(16224));
    CHECK_FALSE(b.backends.empty());  // 16032 and the squeezed-out 16224
    CHECK(b.backends == out_lead->activations.back().backends);
    CHECK(out_var->pathstring() == "0 1-");

    for (const Path* p : flatten_paths(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

// Review finding (Task 17): a variant that banked a phrase before activating
// can fold into a leader that collected the same phrase in its SP, and the
// leader can later squeeze that phrase in. The variant holds no step there.
// The chart is hand-made: 120 BPM, then 4000 BPM from tick 9600, so 500 ms
// spans several SP bars. '1' activates at 10752 and collects the phrase at
// 12288; '0 E0' activates at 4608, banks 8448 and 12288, and activates again
// at 13824. Both end at 16896 with the same score at 13824, and the leader
// later splits on 12288: '1+' (SqIn) and '1-' (SqOut). Before the activation
// rule they folded there, and the variant took each of the leader's squeezes.
//
// '0 E0' banked 12288 before activating at 13824, so it cannot squeeze that
// phrase in or out (core::activation_can_squeeze), while '1' can. The search
// groups the two apart (banked_phrase_in_reach), so '0 E0' is never folded
// into '1+' or '1-' and prices as its own root path with a plain SP end.
TEST_CASE("tied variants: a path that banked its leader's SqIn phrase is not folded into it") {
    const Song song = load_songpath(corpus::root() +
                                        "/test_folded_sqin/folded_sqin.chart",
                                    ChartReading{});
    const app::AnalysisSettings cfg = test::scores_settings(4);
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));

    const Path* in_lead = test::path_named(rec.paths, "1+");
    REQUIRE(in_lead != nullptr);
    // The leader's SqIn step sits on 12288, which it collected.
    CHECK(steps_of(in_lead->activations.back()) ==
          std::vector<Step>{{10752, 15360, SpEndKind::Activation}, {12288, 16896, SpEndKind::SqIn}});
    // D32: '1+' ends its SP at 16896, where 12288 is still in the window.
    // Before D32 that SP end was never taken (the window's chord made it
    // look like a squeeze choice), so '1+' ran on in SP and scored 7350.
    CHECK(in_lead->totalscore() == 6750);

    // '0 E0' is never folded into '1+' while SP runs (the group key keeps
    // them apart). Both SP ends fall on 16896 with the same score, so it
    // folds there, after SP, as a plain tied path with its own window.
    const Path* banked = variant_at(*in_lead, 13824);
    REQUIRE(banked != nullptr);
    CHECK(banked->pathstring() == "0 E0");
    const Activation& a = banked->activations.back();
    CHECK(steps_of(a) == std::vector<Step>{{13824, 16896, SpEndKind::Activation}});
    CHECK(a.sqinouts.empty());
    CHECK(banked->totalscore() == 6750);  // what hydra_replay prices 4608:7680,13824:16896 at
}

// The same family with the leader's SqIn phrase on the variant's own
// activation note: phrases at 8448, 9000 and 13824, none at 12288. The
// leader collects 13824 while its SP runs; the other path banks it on its
// activation chord. Before the activation rule, the two folded at 13824 and
// the leader's later SqIn on 13824 met the variant's Activation step there
// (the "not a collected step" throw). Both hand-made charts must analyze, and
// every variant on them must price as it does alone.
TEST_CASE("tied variants: the banked-phrase charts analyze and every variant prices as alone") {
    const app::AnalysisSettings cfg = test::scores_settings(4);
    for (const char* name : {"folded_sqin.chart", "relabel_case.chart"}) {
        CAPTURE(name);
        const Song song = load_songpath(
            corpus::root() + "/test_folded_sqin/" + name, ChartReading{});
        HydraRecord rec;
        REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            for (const Activation& act : p->walk_activations()) {
                REQUIRE_FALSE(act.sp_end_steps.empty());
                if (act.sqout_tick) CHECK(*act.sqout_tick > act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k)
                    CHECK(act.sp_end_steps[k].tick > act.timecode.ticks());
            }
        }
        for (const Path* v : test::all_tied(rec.paths)) {
            const std::string diff = lone_pricing_mismatch(song, cfg, *v);
            CHECK_MESSAGE(diff.empty(), diff);
        }
    }
}

// A folded variant's step on the leader's SqIn phrase can be Clamped: the
// variant's longer meter let the cap pin its end on that phrase, where the
// leader collected it. A lone search relabels a Clamped step SqIn too, so the
// fold must as well. Two fuzzed charts at 2,000 and 4,000 BPM (several SP bars
// inside 500 ms) reach this at cap 3; before the shared SqIn-step rule they
// threw "a folded variant's SqIn phrase is not a collected step". They do not
// check every variant against its lone pricing: at these tempos many variants
// already differ from it (the review's fuzz found such charts by the hundred),
// for reasons apart from this fold.
//
// Finding 37 (D29) changed what these charts produce. The pinned variant step
// ({16128, 20736, SqIn} on chart a, {15744, 20352, SqIn} on chart b) came from
// a squeeze the old graph offered by mistake: a deact node whose window held
// the clamped phrase offered it to any path whose end was one plain bar past
// the node, even when a later phrase, not the clamped one, had put the path's
// end there (on chart b, 15744 offered at node 20928 to a path whose end
// 22464 came from collecting 18048). Its squeeze-out then ended SP at a node
// the record never names. The squeeze choices now read extend_deacts, cap
// included, so those offers are gone. What these charts still check: they
// analyze, and every squeeze-out ends SP at the end its record names. The
// fold this test was written for (a variant's Clamped step on its leader's
// SqIn phrase, relabelled SqIn) no longer happens on them;
// test_sp_end_clamp.cpp's "folded variant's Clamped step" case covers it now.
TEST_CASE("clamped_sqin charts: every squeeze-out ends SP at the end its record names") {
    const app::AnalysisSettings cfg = test::scores_settings(3);
    for (const std::string name : {"clamped_sqin_a.chart", "clamped_sqin_b.chart"}) {
        CAPTURE(name);
        const Song song = load_songpath(
            corpus::root() + "/test_folded_sqin/" + name, ChartReading{});
        HydraRecord rec;
        REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
        int sqouts = 0;
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            for (const Activation& a : p->walk_activations()) {
                if (!a.sqout_tick) continue;
                ++sqouts;
                // The squeezed-out row's offset is measured from the node SP
                // really ended on; the record's end must be that node.
                REQUIRE(a.deact_tick().has_value());
                REQUIRE(a.sqout_row() != nullptr);
                CHECK(*a.sqout_row()->offset_ms ==
                      offset_from_sp_end(song.timecode(*a.sqout_tick).ms(),
                                         song.timecode(*a.deact_tick()).ms()));
            }
        }
        CHECK(sqouts > 0);
    }
}

// The group key's banked phrase and the squeeze rule draw one line: a phrase
// is banked exactly when activation_can_squeeze says the activation cannot
// squeeze it. Here the expected answer comes from that rule by a plain scan,
// so a banked_phrase_in_reach with its own boundary would drift from it. The
// chart has a phrase on an activation chord (13824), the edge case.
TEST_CASE("banked_phrase_in_reach banks exactly what activation_can_squeeze refuses") {
    const Song song = load_songpath(
        corpus::root() + "/test_folded_sqin/folded_sqin.chart", ChartReading{});
    const std::vector<SongTimestamp>& seq = song.sequence;
    int on_activation_chord = 0;
    for (const SongTimestamp& act : seq) {
        const int64_t act_tick = act.timecode.ticks();
        CAPTURE(act_tick);
        const SongTimestamp* banked = nullptr;
        for (const SongTimestamp& c : seq)
            if (c.flag_sp && !core::activation_can_squeeze(act_tick, c.timecode.ticks()))
                banked = &c;
        // A reach at the activation itself: any banked phrase within 500 ms
        // before it counts.
        const SongTimestamp* want =
            banked && within_squeeze_window(offset_from_sp_end(banked->timecode.ms(),
                                                               act.timecode.ms()))
                ? banked
                : nullptr;
        CHECK(core::banked_phrase_in_reach(song, act_tick, act.timecode) == want);
        if (want && want->timecode.ticks() == act_tick) ++on_activation_chord;
    }
    CHECK(on_activation_chord > 0);
}

// The hand-made chart the two twin-end cases below read (resolution 480,
// 4/4, 120 BPM until tick 24000, then 1200 BPM). It sits outside the corpus
// (testdata/squeeze_clamp) so adding it moves no corpus digest.
static std::string twin_end_chart() {
    return corpus::testdata_root() + "/squeeze_clamp/twin_end_clamped.chart";
}

// D34 at a normal tempo. Two SP ends sit one tick apart, 30720 and 30721:
// '0' ends at 30720 (19200 plus two bars, plus one bar for the phrase on
// 23041), and '1' ends at 30721 (the cap pins it to 23041 plus four
// measures). The phrase on 26000 sits in both ends' windows, and the cap pins
// each path's end to 26000 plus four measures, 33680, so both ends offer the
// phrase to the same path. It was squeezed in at the first and in or out again
// at the second: '0++' and '0+-', '1++' and '1+-'. (The ends one tick apart
// came from rounding once, when the measure sums were truncated doubles; now
// the cap's pin is what makes two ends reach one end.) Real charts do this
// (Thrice - Deadbolt, SoundHaven - Triad). A phrase is squeezed in once, so
// the second end offers the window's next phrase, and here there is none.
TEST_CASE("squeeze rule: twin SP ends a tick apart squeeze one phrase in once") {
    const Song song = load_songpath(twin_end_chart(), ChartReading{});
    const app::AnalysisSettings cfg = test::scores_settings(2);
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
    // One SqIn per phrase, and never out again once in (tests/bank_check.h).
    for (const Path* p : rec.all_paths()) bank_check::check_one_step_per_sqin(*p);
    // The best path keeps its score: the second "+" never moved anything.
    const Path* best = test::path_named(rec.paths, "0+");
    REQUIRE(best != nullptr);
    CHECK(best->totalscore() == 2650);  // pinned from one run
    CHECK(rec.best_path().totalscore() == 2650);
    CHECK(test::path_named(rec.paths, "0++") == nullptr);
    CHECK(test::path_named(rec.paths, "0+-") == nullptr);
}

// D36 gap c, on the same chart. '0' activates at 19200 and its phrase 26000
// moves its end from 30720; '1' activates at 21120 and 26000 moves its end
// from 30721. Both land on 33680 (the cap's pin, 26000 plus four measures),
// so both SP ends hold a choice of 26000 for a path whose end is 33680. Each
// window may squeeze 26000 only at the end its own step moved: before D36 the
// first matching node, 30720, took '1's squeeze too, so its SqIn or SqOut sat
// 0.1 ms off and a SqOut ended SP at a node its record never names. Each
// window is checked in a targeted search (search_target) of its own
// activation, so no fold hides it.
TEST_CASE("squeeze rule: twin SP ends a tick apart each squeeze only their own path") {
    const Song song = load_songpath(twin_end_chart(), ChartReading{});
    const app::AnalysisSettings cfg = test::scores_settings(2);
    const ScoreGraph graph(song, 2);
    CHECK(deact_edge_at(graph, 30720) != nullptr);
    CHECK(deact_edge_at(graph, 30721) != nullptr);
    // 1200 BPM at 480 ticks a beat is 5/48 ms a tick: 26000 sits 4720 ticks
    // (491.667 ms) before 30720 and 4721 ticks (491.771 ms) before 30721, so
    // each window's squeeze sits that far before its own end.
    struct Window {
        int64_t act, own_end;
        double own_ms;
    };
    for (const Window& w : {Window{19200, 30720, -491.0 - 2.0 / 3.0}, Window{21120, 30721, -491.0 - 37.0 / 48.0}}) {
        const int64_t act = w.act, own_end = w.own_end;
        CAPTURE(act);
        const doctest::Approx own_ms = doctest::Approx(w.own_ms).epsilon(1e-9);
        int squeezes = 0;
        const std::vector<Path> kept = search_target(song, cfg, {act});
        for (const Path* p : flatten_paths(kept)) {
            CAPTURE(p->pathstring());
            for (const Activation& a : p->walk_activations()) {
                REQUIRE(a.timecode.ticks() == act);
                for (const SPSqueeze& q : a.sqinouts) {
                    ++squeezes;
                    CHECK(q.offset_ms == own_ms);
                }
                if (a.sqout_tick) CHECK(a.deact_tick() == std::optional<int64_t>(own_end));
            }
        }
        CHECK(squeezes > 0);
    }
}

// The corpus-wide lone-pricing check for tied variants (D3) lives in
// test_replay.cpp ("every tied variant stores what a search pricing it alone
// stores"), at cap 4, depth 40, ms limit 10 and two more settings.

TEST_CASE("tied variants: a variant that finished the song keeps its own bank") {
    // Cap 4. Two phrases fill 2 bars, then two fills. '0' activates at 2304;
    // its SP ends at 5376, so the phrase at 5760 is banked: 1 bar left. '1'
    // activates at 3072; its SP still runs at 5760, so that phrase extends it
    // instead: nothing left. Each doubles two notes, so both score 400, and
    // the search folds one into the other once both finish.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 3});

    auto check_banks = [](const std::vector<const Path*>& all) {
        int tied = 0;
        for (const Path* p : all) {
            if (p->totalscore() != 400) continue;
            ++tied;
            REQUIRE(p->walk_activations().size() == 1);
            const int64_t at = p->walk_activations().front().timecode.ticks();
            const std::vector<int64_t> want =
                at == 2304 ? std::vector<int64_t>{5760} : std::vector<int64_t>{};
            CHECK_MESSAGE(p->trailing_bank_ticks == want, "activation at " << at);
            CHECK(p->leftover_sp() == static_cast<int>(want.size()));
        }
        CHECK(tied == 2);
    };
    const std::vector<const Path*> all = flatten_paths(roots);
    int variants = 0;
    for (const Path* p : all) variants += p->var_point.has_value() ? 1 : 0;
    REQUIRE(variants == 1);  // the two ties are one root and its variant
    check_banks(all);
}

TEST_CASE("tied variants: a variant folded between windows keeps its own banked bars") {
    // Cap 2. '0' activates at 2304 (SP to 5376), then banks 5760 and 8448;
    // the phrase at 9216 finds the meter full. '1' activates at 3072 (SP to
    // 7680, extended by 5760), then banks 8448 and 9216. At the fill at 9984
    // both hold 2 bars with the same score, outside SP, so the search folds
    // '0' into '1' there. The leader then activates at 10752 ('1 0' and its
    // variant '0 0', 1100) or never again ('1' and its variant '0', 850).
    // Either way the variant's 2 bars are its own, banked at 5760 and 8448.
    //
    // The plan's draft had no fill at 9984. Without it the two windows meet
    // only on the SP node after both activate at 10752, a fold mid-SP, so the
    // draft's case passed before the fix and tested nothing new.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448, true, false},
                                       {9216, true, false}, {9984, false, true},
                                       {10752, false, true},
                                       {11520}, {12288}, {16128}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});

    const std::vector<int64_t> own_0{5760, 8448};
    const std::vector<int64_t> own_1{8448, 9216};
    int again = 0, never = 0, variants = 0;
    for (const Path* p : flatten_paths(roots)) {
        const ActivationWalk acts = p->walk_activations();
        if (acts.empty()) continue;
        const int64_t first = acts[0].timecode.ticks();
        if (first != 2304 && first != 3072) continue;
        const std::vector<int64_t>& own = first == 2304 ? own_0 : own_1;
        CAPTURE(p->pathstring());
        CAPTURE(first);
        if (p->totalscore() == 1100) {
            ++again;
            REQUIRE(acts.size() == 2);
            CHECK(acts[1].timecode.ticks() == 10752);
            // '0' passed the fill at 9984 before the fold ('1' was refused
            // there), so its second activation is "1", its own (D38).
            CHECK(p->pathstring() == (first == 2304 ? "0 1" : "1 0"));
            CHECK(acts[1].bank_rise_ticks == own);
            CHECK(acts[1].sp_meter() == 2);
            CHECK(p->trailing_bank_ticks.empty());
        } else if (p->totalscore() == 850) {
            ++never;
            REQUIRE(acts.size() == 1);
            CHECK(p->pathstring() == (first == 2304 ? "0" : "1"));
            CHECK(p->trailing_bank_ticks == own);
        } else {
            continue;
        }
        variants += p->var_point.has_value() ? 1 : 0;
        CHECK(replay_stored_path(song, *p).faithful());
    }
    CHECK(again == 2);
    CHECK(never == 2);
    CHECK(variants == 2);  // each pair is one root and its variant
}

// D38: a variant folded between windows keeps its own early-fill facts for
// the activation its leader takes next. The case above with the last phrase
// moved from 9216 to 9619 (25,049.5 ms). Fills end 6 beats (3,000 ms) after
// their deadline: 9984's is 23,000 ms, 10752's 25,000 ms. '0' is ready at
// 8448 (22,000 ms), so it passes the fill at 9984 1,000 ms early and its
// next activation at 10752 is '0 1', no early fill. '1' is ready at 9619,
// too late for 9984 (refused), and 49.5 ms late for 10752: an E0, "Early
// fill: 49.5 ms (required)". Both stand on 9984 with 2 bars, the same score
// and the same verdict at every fill ahead, so the search folds one into the
// other there, and the variant used to show its leader's offset and passed
// fills.
TEST_CASE("tied variants: a variant folded between windows keeps its own early fill") {
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448, true, false},
                                       {9619, true, false}, {9984, false, true},
                                       {10752, false, true},
                                       {11520}, {12288}, {16128}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});
    const double ready_1 = tick_ms(song, 9619);
    int seen = 0, variants = 0;
    for (const Path* p : flatten_paths(roots)) {
        const ActivationWalk acts = p->walk_activations();
        if (acts.size() != 2 || acts[1].timecode.ticks() != 10752) continue;
        const int64_t first = acts[0].timecode.ticks();
        if (first != 2304 && first != 3072) continue;
        CAPTURE(p->pathstring());
        ++seen;
        variants += p->var_point.has_value() ? 1 : 0;
        const Activation& a = acts[1];
        if (first == 2304) {
            CHECK(p->pathstring() == "0 1");
            CHECK(a.skipped_fill_ticks == std::vector<int64_t>{9984});
            CHECK(a.e_offset == fill_e_offset(23000.0, 22000.0));
            CHECK_FALSE(a.is_e_critical());
        } else {
            CHECK(p->pathstring() == "1 E0");
            CHECK(a.skipped_fill_ticks.empty());
            CHECK(a.e_offset == fill_e_offset(25000.0, ready_1));
            CHECK(a.is_E0());
            CHECK(*a.e_difficulty(true) == doctest::Approx(49.479).epsilon(1e-4));
        }
        SearchSettings s;
        s.sp_cap = 2;
        s.depth_mode = DepthMode::Scores;
        s.depth_value = 10;
        const std::string diff = lone_pricing_mismatch(song, s, *p);
        CHECK_MESSAGE(diff.empty(), diff);
    }
    CHECK(seen == 2);
    CHECK(variants == 1);  // one root and its folded variant
}

TEST_CASE("tied variants: Don Broco - Actors at depth 40 shows each tied path's own bank") {
    // Audit finding 89: paths 29 '7' and 30 '3' tie at 233,100. '7' ends the
    // song with 1 bar of SP, '3' with 3. Today both read 1.
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Don Broco - Actors") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings s;
    s.sp_cap = 4;
    s.depth_mode = DepthMode::Scores;
    s.depth_value = 40;
    s.ms_filter = 10.0;
    const HydraRecord& rec = corpus::analyzed(chart, s);

    int seen = 0;
    for (const Path* p : rec.all_paths()) {
        if (p->totalscore() != 233100) continue;
        const int64_t first = p->walk_activations().front().timecode.ticks();
        if (first == 150720) { CHECK(p->leftover_sp() == 1); ++seen; }
        if (first == 82560) { CHECK(p->leftover_sp() == 3); ++seen; }
    }
    CHECK(seen == 2);
}
