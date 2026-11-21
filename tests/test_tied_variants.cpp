// Tied variants keep their own state (decision D3): a path the search folds
// into a tied leader keeps its own steps, squeezes, banked bars and early
// fills. The rules these folds exposed (D34, D36, D38) are pinned here too,
// on hand-made charts; each case says which chart it reads.

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
// song builder).
using test::TailNote;
using test::build_tail_song;

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

// flatten_paths (model.h) lists every path of a search's output, each before
// its variants; test::path_named (record_fixtures.h) finds a path by its path
// string.

// The leader's variant whose own last activation is at `tick`.
const Path* variant_at(const Path& leader, int64_t tick) {
    for (const Path& v : leader.variants)
        if (!v.activations.empty() && v.activations.back().timecode.ticks() == tick)
            return &v;
    return nullptr;
}
// A root's tied variants, depth-first (record_fixtures.h).
using test::collect_tied;

// The settings run_search(graph, EngineOptions{DepthMode::Scores, depth}) on
// a ScoreGraph at cap `cap` stands for, as test::lone_pricing reads them.
SearchSettings scores_search(int cap, int depth) {
    SearchSettings s;
    s.sp_cap = cap;
    s.depth_mode = DepthMode::Scores;
    s.depth_value = depth;
    return s;
}

// D3's promise: each path in `paths` stores what a search pricing its own
// activations alone stores (test::lone_pricing). A path the lone search also
// folded has no oracle; it fails here by name, never skipped. Returns how
// many paths were compared.
int check_each_prices_alone(const Song& song, const SearchSettings& settings,
                            const std::vector<const Path*>& paths) {
    int compared = 0;
    for (const Path* p : paths) {
        const test::LonePricing lone = test::lone_pricing(song, settings, *p);
        CHECK_MESSAGE(lone.diff.empty(), lone.diff);
        ++compared;
    }
    return compared;
}

// D3 for a variant folded while SP ran. Its first SP-end step is its own
// activation, and its next step is a phrase that activation collects
// (core::activation_can_squeeze). From that step on it holds its leader's
// close: the leader's steps from that tick, its squeezes, its squeezed-out
// note and its backend rows.
void check_takes_leaders_close(const Activation& mine, const Activation& lead) {
    REQUIRE(mine.sp_end_steps.size() >= 2);  // it folded with SP still running
    CHECK(mine.sp_end_steps.front().tick == mine.timecode.ticks());
    CHECK(mine.sp_end_steps.front().kind == SpEndKind::Activation);
    const int64_t fold = mine.sp_end_steps[1].tick;
    CHECK(core::activation_can_squeeze(mine.timecode.ticks(), fold));
    std::vector<SpEndStep> leaders_after;
    for (const SpEndStep& s : lead.sp_end_steps)
        if (s.tick >= fold) leaders_after.push_back(s);
    const std::vector<SpEndStep> mine_after(mine.sp_end_steps.begin() + 1,
                                            mine.sp_end_steps.end());
    CHECK(mine_after == leaders_after);
    REQUIRE(mine.sqinouts.size() == lead.sqinouts.size());
    for (size_t i = 0; i < mine.sqinouts.size(); ++i) {
        CHECK(mine.sqinouts[i].kind == lead.sqinouts[i].kind);
        CHECK(mine.sqinouts[i].offset_ms == lead.sqinouts[i].offset_ms);
    }
    CHECK(mine.sqout_tick == lead.sqout_tick);
    CHECK(mine.backends == lead.backends);
}

// A root and one of its tied variants.
struct TiedPair {
    const Path* root;
    const Path* variant;
};

// Every root that holds tied variants, paired with each of them.
std::vector<TiedPair> tied_pairs(const std::vector<Path>& roots) {
    std::vector<TiedPair> out;
    for (const Path& r : roots) {
        std::vector<const Path*> tied;
        collect_tied(r, tied);
        for (const Path* v : tied) out.push_back(TiedPair{&r, v});
    }
    return out;
}

// Where the bars each window of `p` spent arrived, then the trailing bars
// (Path::bank_arrivals for every window and the stretch after the last).
std::vector<std::vector<int64_t>> bank_arrivals_of(const Path& p) {
    std::vector<std::vector<int64_t>> out;
    for (size_t i = 0; i <= p.walk_activations().size(); ++i) out.push_back(p.bank_arrivals(i));
    return out;
}

// The fills a search pricing `p`'s activations alone (search_target) lists
// as passed over after the last one, read off the unfolded root of that
// search that is `p` (path_identity). nullopt when no such root comes back.
std::optional<std::vector<int64_t>> lone_trailing_fills(const ScoreGraph& graph, const Path& p) {
    const TargetResult alone = search_target(graph, activation_pins(test::act_ticks(p)));
    for (size_t i = 0; i < alone.paths.size(); ++i) {
        const bool promoted = i < alone.promoted.size() && alone.promoted[i];
        if (!promoted && path_identity(alone.paths[i]) == path_identity(p))
            return alone.paths[i].trailing_skipped_fill_ticks;
    }
    return std::nullopt;
}

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

    // Its own activation step, then the leader's steps from the fold on, so
    // the leader's clamp at 13056, after the fold, is its too; and the real
    // deactivation's rows, not the song's last notes.
    check_takes_leaders_close(variant->activations.back(), leader->activations.back());
    check_each_prices_alone(song, scores_search(2, 6), {variant});

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

    // Each leader closes on the squeeze its path string names, so there is a
    // closing squeeze for the variant to take.
    const Activation& in_close = in_lead->activations.back();
    const Activation& out_close = out_lead->activations.back();
    REQUIRE(in_close.sqinouts.size() == 1);
    CHECK(in_close.sqinouts[0].kind == SqueezeKind::SqIn);
    REQUIRE(out_close.sqinouts.size() == 1);
    CHECK(out_close.sqinouts[0].kind == SqueezeKind::SqOut);
    CHECK(out_close.sqout_tick.has_value());
    CHECK_FALSE(out_close.backends.empty());  // 16032 and the squeezed-out 16224

    // The leader collected 11328 in SP; the variant banked it before its own
    // activation at 11520, so its steps after its activation start later
    // (check_takes_leaders_close asks activation_can_squeeze). From there it
    // takes the leader's SqIn or SqOut close.
    check_takes_leaders_close(in_var->activations.back(), in_close);
    check_takes_leaders_close(out_var->activations.back(), out_close);
    check_each_prices_alone(song, scores_search(2, 6), {in_var, out_var});

    for (const Path* p : flatten_paths(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

// A variant that banked a phrase before activating can tie a leader that
// collected the same phrase in its SP, and the leader can later squeeze that
// phrase in. The variant holds no step there.
// The chart is hand-made: 120 BPM, then 4000 BPM from tick 9600, so 500 ms
// spans several SP bars. '1' activates at 10752 and collects the phrase at
// 12288; '0 E0' activates at 4608, banks 8448 and 12288, and activates again
// at 13824. Both end at 16896 with the same score at 13824, and the leader
// later splits on 12288: '1+' (SqIn) and '1-' (SqOut). A fold at 13824 would
// give the variant each of the leader's squeezes.
//
// '0 E0' banked 12288 before activating at 13824, so it cannot squeeze that
// phrase in or out (core::activation_can_squeeze), while '1' can. The search
// groups the two apart while SP runs (banked_phrase_in_reach), so '0 E0' keeps
// a plain SP end and folds into '1+' only after SP.
TEST_CASE("tied variants: a path that banked its leader's SqIn phrase folds into it only after SP") {
    const Song song = load_songpath(corpus::root() +
                                        "/test_folded_sqin/folded_sqin.chart",
                                    ChartReading{});
    const app::AnalysisSettings cfg = test::scores_settings(4);
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));

    const Path* in_lead = test::path_named(rec.paths, "1+");
    REQUIRE(in_lead != nullptr);
    const Activation& lead = in_lead->activations.back();
    // The phrase the leader squeezed in: its SqIn step.
    const SpEndStep* sqin = nullptr;
    for (const SpEndStep& s : lead.sp_end_steps)
        if (is_sqin_kind(s.kind)) sqin = &s;
    REQUIRE(sqin != nullptr);

    // The path that activates at 13824 ties the leader and hangs off it.
    const Path* banked = variant_at(*in_lead, 13824);
    REQUIRE(banked != nullptr);
    CHECK(banked->totalscore() == in_lead->totalscore());
    const Activation& a = banked->activations.back();
    // It banked the leader's SqIn phrase before it activated, so it cannot
    // squeeze that phrase.
    CHECK_FALSE(core::activation_can_squeeze(a.timecode.ticks(), sqin->tick));
    // It was never folded into '1+' while SP ran (the group key keeps them
    // apart): its window is its own activation step alone, with no squeeze,
    // and it folded where both SP ends fall, after SP.
    REQUIRE(a.sp_end_steps.size() == 1);
    CHECK(a.sp_end_steps.front().tick == a.timecode.ticks());
    CHECK(a.sp_end_steps.front().kind == SpEndKind::Activation);
    CHECK(a.sqinouts.empty());
    CHECK(a.deact_tick() == lead.deact_tick());
    check_each_prices_alone(song, cfg, {banked});
}

// The same family with the leader's SqIn phrase on the variant's own
// activation note: phrases at 8448, 9000 and 13824, none at 12288. The
// leader collects 13824 while its SP runs; the other path banks it on its
// activation chord. A fold at 13824 would make the leader's later SqIn on
// 13824 meet the variant's Activation step there. Both hand-made charts must
// analyze, and every variant on them must price as it does alone.
TEST_CASE("tied variants: the banked-phrase charts analyze and every variant prices as alone") {
    const app::AnalysisSettings cfg = test::scores_settings(4);
    int compared = 0;
    for (const std::string name : {"folded_sqin.chart", "relabel_case.chart"}) {
        CAPTURE(name);
        const Song song = load_songpath(
            corpus::root() + "/test_folded_sqin/" + name, ChartReading{});
        HydraRecord rec;
        REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
        REQUIRE_FALSE(rec.paths.empty());
        for (const Path* p : rec.all_paths()) {
            CAPTURE(p->pathstring());
            for (const Activation& act : p->walk_activations()) {
                REQUIRE_FALSE(act.sp_end_steps.empty());
                if (act.sqout_tick) CHECK(*act.sqout_tick > act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k)
                    CHECK(act.sp_end_steps[k].tick > act.timecode.ticks());
            }
        }
        compared += check_each_prices_alone(song, cfg, test::all_tied(rec.paths));
    }
    // A floor, so the lone-pricing loop cannot pass on no variant at all:
    // folded_sqin holds a tied variant, the path that activates at 13824
    // under '1+' (the case above requires it).
    CHECK(compared > 0);
}

// Two charts that go from 120 to 4,000 BPM, read at cap 3, put several SP bars
// inside 500 ms. They check that the charts analyze and that every squeeze-out
// ends SP at the end its record names. core::offered_phrase picks the phrase
// an end offers, and a late squeeze-in's moved end is extend_deacts' answer,
// cap included (D29). They do not check every variant against its lone
// pricing: at these tempos many variants differ from it, for other reasons.
//
// A variant's Clamped step on its leader's SqIn phrase, relabelled SqIn, does
// not happen on these charts; test_sp_end_clamp.cpp's "folded variant's
// Clamped step" case covers it.
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

// The group key's banked phrase and the search graph's squeeze window draw one
// line. The expected answer is read off the window the graph lists on an SP
// end (core::squeeze_window_phrases), not off the helpers
// banked_phrase_in_reach itself walks with: of the phrase chords in the window
// of an SP end on the activation, the last one the activation cannot squeeze
// (core::activation_can_squeeze), or none. A banked_phrase_in_reach that
// walked to a different chord, or used its own boundary, would drift from it.
TEST_CASE("banked_phrase_in_reach banks exactly what activation_can_squeeze refuses") {
    const Song song = load_songpath(
        corpus::root() + "/test_folded_sqin/folded_sqin.chart", ChartReading{});
    const std::vector<SongTimestamp>& seq = song.sequence;
    int on_activation_chord = 0;
    for (const SongTimestamp& act : seq) {
        const int64_t act_tick = act.timecode.ticks();
        CAPTURE(act_tick);
        // A reach at the activation itself: an SP end on the activation chord.
        const SongTimestamp* want = nullptr;
        for (const SongTimestamp* c : core::squeeze_window_phrases(song, act.timecode))
            if (!core::activation_can_squeeze(act_tick, c->timecode.ticks())) want = c;
        CHECK(core::banked_phrase_in_reach(song, act_tick, act.timecode) == want);
        if (want && want->timecode.ticks() == act_tick) ++on_activation_chord;
    }
    // A floor, so the walk reaches the boundary's own case, a phrase on the
    // activation chord: every phrase chord, taken as the activation, is the
    // phrase it banked last, and folded_sqin has phrase chords.
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
// each path's end to 26000 plus four measures, 33680, so both ends could offer
// the phrase to the same path. Real charts do this (Thrice - Deadbolt,
// SoundHaven - Triad). A phrase is squeezed in once, so the second end offers
// the window's next phrase, and here there is none: '0+' has no '0++' or '0+-'
// after it.
TEST_CASE("squeeze rule: twin SP ends a tick apart squeeze one phrase in once") {
    const Song song = load_songpath(twin_end_chart(), ChartReading{});
    const app::AnalysisSettings cfg = test::scores_settings(2);
    HydraRecord rec;
    REQUIRE_NOTHROW(rec = analyze_chart(song, cfg));
    // One SqIn per phrase, and never out again once in (tests/bank_check.h).
    for (const Path* p : rec.all_paths()) bank_check::check_one_step_per_sqin(*p);
    // '0' squeezes the phrase on 26000 in once, and never a second time.
    REQUIRE(test::path_named(rec.paths, "0+") != nullptr);
    CHECK(test::path_named(rec.paths, "0++") == nullptr);
    CHECK(test::path_named(rec.paths, "0+-") == nullptr);
}

// D36 gap c, on the same chart. '0' activates at 19200 and its phrase 26000
// moves its end from 30720; '1' activates at 21120 and 26000 moves its end
// from 30721. Both land on 33680 (the cap's pin, 26000 plus four measures),
// so both SP ends hold a choice of 26000 for a path whose end is 33680. Each
// window may squeeze 26000 only at the end its own step moved, so its SqIn or
// SqOut sits at that end's offset and a SqOut ends SP at that end. Each
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
        const std::vector<Path> kept = search_target(song, cfg, activation_pins({act})).paths;
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
// stores"), at cap 4, depth 40, ms limit 10 and two more settings. It also
// covers the real chart audit finding 89 was found on (Don Broco - Actors at
// those settings): test::windows_text, which it compares, holds each path's
// trailing bars.

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

    const std::vector<TiedPair> pairs = tied_pairs(roots);
    REQUIRE(pairs.size() == 1);  // the two ties are one root and its variant
    const Path& root = *pairs[0].root;
    const Path& variant = *pairs[0].variant;
    CHECK(variant.totalscore() == root.totalscore());
    // The fold joined two different banks: one path banked the phrase at
    // 5760 after its SP, the other spent it inside its SP.
    CHECK(variant.trailing_bank_ticks != root.trailing_bank_ticks);
    // Each keeps the bank a search pricing it alone gives it.
    check_each_prices_alone(song, scores_search(4, 3), {&root, &variant});
}

// The chart the two tests below read: two tied first windows that the search
// folds together between windows, at the fill at 9984.
static Song fold_between_windows_song() {
    return build_tail_song({{0, true, false}, {768, true, false},
                            {2304, false, true}, {3072, false, true},
                            {5760, true, false}, {8448, true, false},
                            {9216, true, false}, {9984, false, true},
                            {10752, false, true},
                            {11520}, {12288}, {16128}});
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
    // The fill at 9984 is what makes this a fold between windows. Without it
    // the two windows meet only on the SP node after both activate at 10752,
    // a fold mid-SP.
    const Song song = fold_between_windows_song();
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});

    const SearchSettings settings = scores_search(2, 10);
    int differing = 0;
    for (const TiedPair& t : tied_pairs(roots)) {
        CAPTURE(t.root->pathstring());
        CAPTURE(t.variant->pathstring());
        CHECK(t.variant->totalscore() == t.root->totalscore());
        // Each spends and keeps the bars a search pricing it alone gives it
        // (test::windows_text holds every window's banked bars and the
        // trailing ones).
        check_each_prices_alone(song, settings, {t.root, t.variant});
        CHECK(replay_stored_path(song, *t.variant).faithful());
        if (bank_arrivals_of(*t.variant) != bank_arrivals_of(*t.root)) ++differing;
    }
    // A floor, so the loop cannot pass on folds that joined equal banks:
    // the two paths fold at 9984 holding bars banked at different ticks.
    CHECK(differing > 0);
}

// The chart above, read for the fills each path passed over after its last
// activation (Path::trailing_skipped_fill_ticks). '0' passed the fill at
// 9984 before the fold, where '1' was refused; both passed 10752 after it.
// So in the pair that never activates again, '0' lists 9984 and 10752 and
// '1' lists 10752, whichever of the two is the variant. In the pair that
// activates at 10752 there is nothing after it to pass. A search pricing
// each path alone lists the same.
TEST_CASE("tied variants: a variant's trailing fills are its own") {
    const Song song = fold_between_windows_song();
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});

    int differing = 0;
    for (const TiedPair& t : tied_pairs(roots)) {
        CAPTURE(t.root->pathstring());
        CAPTURE(t.variant->pathstring());
        // Each lists what a search pricing it alone lists.
        for (const Path* p : {t.root, t.variant}) {
            CAPTURE(p->pathstring());
            REQUIRE(p->has_activations());
            const std::optional<std::vector<int64_t>> alone = lone_trailing_fills(graph, *p);
            REQUIRE_MESSAGE(alone.has_value(), "no unfolded lone root is this path");
            CHECK(p->trailing_skipped_fill_ticks == *alone);
        }
        if (t.variant->trailing_skipped_fill_ticks != t.root->trailing_skipped_fill_ticks)
            ++differing;
    }
    // A floor, so the loop cannot pass on folds that joined equal lists: in
    // the pair that never activates again only one passed the fill at 9984.
    CHECK(differing > 0);
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
// other there. The variant keeps its own offset and passed fills, not its
// leader's.
TEST_CASE("tied variants: a variant folded between windows keeps its own early fill") {
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448, true, false},
                                       {9619, true, false}, {9984, false, true},
                                       {10752, false, true},
                                       {11520}, {12288}, {16128}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 10});
    const SearchSettings settings = scores_search(2, 10);
    int differing = 0;
    for (const TiedPair& t : tied_pairs(roots)) {
        CAPTURE(t.root->pathstring());
        CAPTURE(t.variant->pathstring());
        // Each keeps the early-fill facts a search pricing it alone gives it
        // (test::windows_text holds every window's offset and passed fills).
        check_each_prices_alone(song, settings, {t.root, t.variant});
        // The pairs that take the same activation after the fold: one needs
        // an early fill there and the other does not.
        const ActivationWalk mine = t.variant->walk_activations();
        const ActivationWalk lead = t.root->walk_activations();
        if (mine.empty() || mine.size() != lead.size()) continue;
        if (mine.back().timecode.ticks() != lead.back().timecode.ticks()) continue;
        if (mine.back().is_E0() != lead.back().is_E0()) ++differing;
    }
    // A floor, so the loop cannot pass on folds that joined equal early-fill
    // facts: the pair that activates at 10752 after the fold at 9984 is one
    // path with an E0 there and one without.
    CHECK(differing > 0);
}
