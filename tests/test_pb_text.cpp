// The Path Builder's sentences (pathbuilder/pb_text.h; plan
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md, task CO-1, and
// docs/superpowers/plans/2026-10-31-path-builder-redesign-2.md, task SC-3a,
// and docs/superpowers/plans/2026-10-31-path-builder-redesign-3.md, task TX-3).
// The corpus cases run at the GUI's default settings, whose Path limit is
// 10 ms; every sentence is pinned from one run.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "corpus_default_cfg.h"
#include "corpus_util.h"
#include "pathbuilder/pb_text.h"
#include "search/pather.h"

using namespace hydra;
using namespace hydra::pathbuilder;

using corpus::default_cfg;
using corpus::song_named;

namespace {

// The one path search_target makes of activation-only pins at `ticks`.
Path player_path(const Song& song, const std::vector<int64_t>& ticks) {
    std::vector<PinnedWindow> wins;
    for (const int64_t t : ticks) wins.push_back(PinnedWindow{t, std::nullopt, false, std::nullopt});
    const TargetResult tr = search_target(song, default_cfg(), wins);
    REQUIRE(tr.paths.size() >= 1);
    return tr.paths[0];
}

app::PreviewScene scene_for(const Song& song, const Path& path) {
    return app::build_preview_scene(song, &path, default_cfg().sp_cap, default_cfg().rules);
}

size_t fill_index(const app::PreviewScene& scene, int64_t chord_tick) {
    for (size_t i = 0; i < scene.fills.size(); ++i)
        if (scene.fills[i].chord_tick == chord_tick) return i;
    FAIL("no fill at tick " << chord_tick);
    return 0;
}

}  // namespace

TEST_CASE("candidate row: the clock, phrases, end and squeeze of a priced fill") {
    // August Burns Red "White Washed" with the player's path on its best
    // path's first three activations: the fourth's fill is offered next, and
    // its candidate activation (best_finish with it pinned, as the candidate
    // queue prices it) ends with a squeeze-out. Pinned from one run.
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = corpus::analyzed(corpus::chart_named(name), default_cfg());
    const std::vector<Activation> acts = rec.best_path().all_activations();
    REQUIRE(acts.size() == 5);
    const app::PreviewScene three = scene_for(
        song, player_path(song, {acts[0].timecode.ticks(), acts[1].timecode.ticks(),
                                 acts[2].timecode.ticks()}));
    std::vector<PinnedWindow> pins;
    for (size_t k = 0; k < 4; ++k)
        pins.push_back(PinnedWindow{acts[k].timecode.ticks(), std::nullopt, false, std::nullopt});
    const TargetResult with = best_finish(song, default_cfg(), pins);
    const std::vector<Activation> ends = with.paths.at(0).all_activations();
    REQUIRE(ends.size() >= 4);
    const size_t idx = fill_index(three, 245760);
    CHECK(fill_refusal(three, idx) == FillRefusal::None);
    CHECK(candidate_row_text(three, idx, &ends[3]) ==
          "2:51.485 · 2 phrases · ends m136 · squeeze-out 130.4 ms");
    // The same squeeze, once, in the step's long form.
    CHECK(squeeze_end_text(ends[3]) == ", ending with a squeeze-out that needs 130.4 ms");
    // Before its search lands, the end reads kPendingText.
    CHECK(candidate_row_text(three, idx, nullptr) == "2:51.485 · 2 phrases · ...");
    // With no squeeze the row stops at the end: the best path's last
    // activation, read on the best path's own scene.
    const app::PreviewScene best = scene_for(song, rec.best_path());
    CHECK(candidate_row_text(best, fill_index(best, 272640), &acts[4]) ==
          "3:20.702 · 2 phrases · ends m150");
    // No corpus chart has a squeeze-in that needs no timing, so a hand-built
    // activation with the same end carries one of each kind that needs none
    // (SPSqueeze::needs_no_timing): the short form and the long form both
    // say so.
    Activation free_squeezes;
    free_squeezes.sp_end_steps.push_back({0, *ends[3].deact_tick(), SpEndKind::Activation});
    free_squeezes.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 0.0},
                              SPSqueeze{SqueezeKind::SqOut, 0.001}};
    CHECK(candidate_row_text(three, idx, &free_squeezes) ==
          "2:51.485 · 2 phrases · ends m136 · squeeze-in, no timing, squeeze-out, no timing");
    CHECK(squeeze_end_text(free_squeezes) ==
          ", ending with a squeeze-in that needs no timing and a squeeze-out that needs no "
          "timing");
}

TEST_CASE("choice order: cheapest first, ties in chart order, pending last") {
    // Allister "Overrated" with no activation: the engine lists these fills.
    // The costs are made up to put a tie and two pending fills in the list.
    const Song& song = song_named("Allister - Overrated");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    const std::vector<int64_t>& offered = empty.trailing_skipped_fill_ticks;
    REQUIRE(offered == std::vector<int64_t>{55680, 71040, 90240, 105600, 120960, 136320, 151680,
                                            167040});
    const std::vector<app::CandidateCost> priced = {
        {55680, 0, 2500, app::CostTier::Yellow},  {71040, 0, 0, app::CostTier::Green},
        {105600, 0, 2500, app::CostTier::Yellow}, {120960, 0, 900, app::CostTier::Yellow},
        {136320, 0, 0, app::CostTier::Green},     {999999, 0, 0, app::CostTier::Green},
    };
    CHECK(choice_order(offered, priced) ==
          std::vector<int64_t>{71040, 136320, 120960, 55680, 105600, 90240, 151680, 167040});
    // Nothing priced yet: chart order.
    CHECK(choice_order(offered, {}) == offered);
}

TEST_CASE("step short: start, end and phrases") {
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = corpus::analyzed(corpus::chart_named(name), default_cfg());
    const std::vector<Activation> acts = rec.best_path().all_activations();
    REQUIRE(acts.size() == 5);
    const app::PreviewScene scene = scene_for(song, rec.best_path());
    const StepShort first = step_short(*scene.timing, acts[0]);
    CHECK(first.span == "m23 → m29");
    CHECK(first.saved == "2 phrases");
    const StepShort third = step_short(*scene.timing, acts[2]);
    CHECK(third.span == "m108 → m116");
    CHECK(third.saved == "4 phrases");
}

TEST_CASE("locked message: the fill's name, no Can't-activate prefix") {
    const GuideMessage m = locked_message("m12");
    CHECK(m.text ==
          "m12 is before your last activation. You build left to right; Undo or Remove goes "
          "back a step.");
    CHECK(m.refuse);
    CHECK_FALSE(m.star_at);
    // The game's refusals keep the prefix.
    CHECK(cant_activate_message("m12", kGameHidesFillText).text ==
          "Can't activate at m12: The game doesn't show this fill here.");
    // The hover's form shares the tail.
    CHECK(kBeforeLastActivationText ==
          "This is before your last activation. You build left to right; Undo or Remove goes "
          "back a step.");
}

TEST_CASE("fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence for "
          "each") {
    // Trivium "Feast of Fire". With no activation, its third fill comes with
    // enough bars saved and the engine still does not offer it.
    const Song& song = song_named("Trivium - Feast of Fire");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    REQUIRE(empty.fills.size() >= 4);
    CHECK(fill_refusal(empty, 0) == FillRefusal::UnderBars);
    CHECK(fill_reason(empty, 0) == "You have 1 phrase saved here. You need 2 to activate.");
    CHECK(fill_refusal(empty, 2) == FillRefusal::GameHides);
    CHECK(fill_reason(empty, 2) == "The game doesn't show this fill here.");
    CHECK(fill_refusal(empty, 3) == FillRefusal::None);
    CHECK(fill_reason(empty, 3).empty());
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Trivium - Feast of Fire"), default_cfg());
    // On Hydra's best path, the fill after the first activation's fill is
    // inside that window, and a fill the path passed between two windows is
    // before the last activation although the engine offered it.
    const app::PreviewScene best = scene_for(song, rec.best_path());
    REQUIRE(best.fills.size() > 5);
    CHECK(best.fills[4].state == app::PreviewFillState::Taken);
    CHECK(fill_refusal(best, 4) == FillRefusal::Taken);
    CHECK(fill_reason(best, 4).empty());
    CHECK(best.fills[5].chord_tick == 72480);
    CHECK(fill_refusal(best, 5) == FillRefusal::SpRunning);
    CHECK(fill_reason(best, 5) == "Star Power is already on here. Activation 1 runs until m40.");
    CHECK(best.fills[3].state == app::PreviewFillState::Offered);
    CHECK(fill_refusal(best, 3) == FillRefusal::BeforeLast);
    CHECK(fill_reason(best, 3) == kBeforeLastActivationText);
    // A fill the game hides is locked too when it is before the last
    // activation: the left-to-right rule comes before the game's.
    CHECK(best.fills[0].state == app::PreviewFillState::Hidden);
    CHECK(fill_refusal(best, 0) == FillRefusal::BeforeLast);
    // The sentence and the kind come from one walk.
    FillRefusal kind = FillRefusal::None;
    CHECK(fill_reason(best, 5, kind) == fill_reason(best, 5));
    CHECK(kind == FillRefusal::SpRunning);
    // "Before your last activation" is a fill ahead of that activation's
    // start. With one window at 180000 that ends at 195360, the fill sitting
    // exactly on 195360 is after the last activation's start, so it is not
    // that sentence.
    const app::PreviewScene one = scene_for(song, player_path(song, {180000}));
    const size_t at_end = fill_index(one, 195360);
    REQUIRE(one.activations.size() == 1);
    REQUIRE(one.activations[0].sp_end_tick == 195360);
    CHECK(fill_reason(one, at_end) != kBeforeLastActivationText);
    // A chord on the end tick is still under Star Power (core::after_sp_end).
    CHECK(fill_reason(one, at_end) == "Star Power is already on here. Activation 1 runs until m102.");
    CHECK(fill_reason(one, fill_index(one, 156960)) == kBeforeLastActivationText);
}

TEST_CASE("step sentence: a squeeze-out over the limit carries the ms figure and the violet note") {
    // August Burns Red "White Washed": its best path's fourth window needs a
    // squeeze-out tighter than the 10 ms limit (test_path_costs.cpp).
    const std::string name = "August Burns Red - White Washed";
    const Song& song = song_named(name);
    const HydraRecord& rec = corpus::analyzed(corpus::chart_named(name), default_cfg());
    const std::vector<Activation> acts = rec.best_path().all_activations();
    REQUIRE(acts.size() == 5);
    const app::PreviewScene scene = scene_for(song, rec.best_path());
    const StepText s =
        step_sentence(*scene.timing, acts[3], 1234, 707414, 706180, default_cfg().ms_filter);
    // The cost and the two best finishes are the caller's figures; these are
    // made up to show both wordings.
    CHECK(s.sentence ==
          "Activate at m132 (2:51.485) with 2 phrases saved. Star Power runs until m136, ending "
          "with a squeeze-out that needs 130.4 ms. This choice costs 1,234 points.");
    CHECK(s.why == "Before it, the best you could still finish with was 707,414. After it, 706,180.");
    CHECK(s.note == "needs 130.4 ms, your limit is 10 ms");
    const StepText free =
        step_sentence(*scene.timing, acts[0], 0, 706180, 706180, default_cfg().ms_filter);
    CHECK(free.sentence ==
          "Activate at m23 (0:43.279) with 2 phrases saved. Star Power runs until m29. This "
          "choice costs nothing.");
    CHECK(free.why == "The best you can still finish with stays at 706,180.");
    CHECK(free.note.empty());
}

TEST_CASE("guide text: step 1 names the earliest offered fill; a finished path says so") {
    // Allister "Overrated": with no activation the first fill comes with one
    // bar saved, so the earliest the player can activate is the second.
    const Song& song = song_named("Allister - Overrated");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    GuideFacts start;
    for (size_t i = 0; i < empty.fills.size(); ++i) {
        if (empty.fills[i].state == app::PreviewFillState::Offered && fill_reason(empty, i).empty()) {
            start.earliest_fill = fill_name(empty, empty.fills[i]);
            start.earliest_ms = empty.timing->timecode(empty.fills[i].chord_tick).ms();
            break;
        }
    }
    start.fills_after_last = empty.trailing_skipped_fill_ticks.size();
    const GuideMessage step1 = guide_text(start);
    CHECK(step1.text ==
          "Step 1. Click a glowing fill to make your first activation. The earliest you can "
          "activate is m30 (0:45.789), once you've saved 2 phrases. Hover a fill to see it on "
          "the highway.");
    CHECK_FALSE(step1.refuse);

    // Hydra's best path ends with no fill offered after its last window.
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Allister - Overrated"), default_cfg());
    const Path& best = rec.best_path();
    const app::PreviewScene done = scene_for(song, best);
    REQUIRE(done.trailing_skipped_fill_ticks.empty());
    GuideFacts finished;
    finished.last_end = measure_label(
        done.timing->timecode(*best.all_activations().back().deact_tick()));
    finished.player_total = best.totalscore();
    const GuideMessage end = guide_text(finished);
    CHECK(end.text ==
          "Your Star Power runs until m90. No fill after that comes with 2 saved phrases, so "
          "your path is finished. Your score is 228,710.");

    // A one-shot message wins over both.
    finished.message = locked_message("m12");
    CHECK(guide_text(finished).text ==
          "m12 is before your last activation. You build left to right; Undo or Remove goes "
          "back a step.");
    CHECK(guide_text(finished).refuse);
    // Neither sentence carries the star; only "Save 2 more phrases" does.
    CHECK_FALSE(step1.star_at);
    CHECK_FALSE(end.star_at);
}

TEST_CASE("star place: just after the word phrases, in the intro and the guide") {
    const std::string intro = intro_text();
    REQUIRE(intro_star_at() <= intro.size());
    CHECK(intro.substr(0, intro_star_at()) ==
          "Build your own Star Power path, left to right. Click a glowing fill to activate Star "
          "Power there. You need at least 2 Star Power phrases");
    CHECK(intro.substr(intro_star_at(), 33) == " saved since your last activation");
    CHECK(intro.substr(intro_star_at()) ==
          " saved since your last activation. A fill's colour is what activating there costs "
          "you. The highway below shows the notes wherever you hover or click.");

    GuideFacts next;
    next.last_end = "m40";
    next.earliest_fill = "m52";
    next.fills_after_last = 1;
    const GuideMessage g = guide_text(next);
    CHECK(g.text ==
          "Your Star Power runs until m40. Save 2 more phrases, then activate again. The next "
          "glowing fill is m52.");
    REQUIRE(g.star_at);
    CHECK(g.text.substr(0, *g.star_at) == "Your Star Power runs until m40. Save 2 more phrases");
}

TEST_CASE("fill hover text: the two numbers and the difference agree") {
    // Allister "Overrated" with no activation: BF(0) is the best finish over
    // no windows, and a fill's own number is the best finish with it pinned.
    const Song& song = song_named("Allister - Overrated");
    const app::PreviewScene empty = scene_for(song, player_path(song, {}));
    const int64_t bf0 = best_finish(song, default_cfg(), {}).paths.at(0).totalscore();
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Allister - Overrated"), default_cfg());
    const int64_t best_tick = rec.best_path().all_activations().front().timecode.ticks();
    const size_t pick = fill_index(empty, 55680);
    const TargetResult with =
        best_finish(song, default_cfg(), {PinnedWindow{55680, std::nullopt, false, std::nullopt}});
    const Path& with_path = with.paths.at(0);
    const std::vector<Activation> with_acts = with_path.all_activations();
    FillHoverNumbers n;
    n.best_finish = with_path.totalscore();
    n.best_now = bf0;
    n.cost = bf0 - n.best_finish;  // candidate_costs's subtraction, for the fixture only
    n.best_fill = fill_name(empty, empty.fills[fill_index(empty, best_tick)]);
    n.hydra_end = &with_acts.front();
    // The three figures in the text, pinned from one run: 228,710 less
    // 226,210 is the 2,500 it says.
    CHECK(bf0 == 228710);
    CHECK(n.best_finish == 226210);
    CHECK(fill_hover_text(empty, pick, n) ==
          "Fill at m30 · 0:45.789\n"
          "You have 2 phrases saved.\n"
          "If you activate here, the best you can still finish with is 226,210.\n"
          "Your best choice right now is m38, which keeps 228,710.\n"
          "So activating here costs 2,500 points.\n"
          "Hydra would end it at m34.");
    // Before the candidate's search lands, and on a fill under two bars.
    CHECK(fill_hover_text(empty, pick, std::nullopt) ==
          "Fill at m30 · 0:45.789\nYou have 2 phrases saved.\n...");
    CHECK(fill_hover_text(empty, 0, std::nullopt) ==
          "Fill at m20 · 0:30.000\nYou have 1 phrase saved here. You need 2 to activate.");
    // The best fill itself costs nothing and says so.
    FillHoverNumbers b = n;
    b.best_finish = bf0;
    b.cost = 0;
    b.hydra_end = nullptr;
    CHECK(fill_hover_text(empty, fill_index(empty, best_tick), b) ==
          "Fill at m38 · 0:58.421\n"
          "You have 3 phrases saved.\n"
          "If you activate here, the best you can still finish with is 228,710.\n"
          "This is the best choice now.");
}

namespace {

// Allister "Overrated" with no activation, and the candidate numbers for the
// fill at `tick` as the screen gathers them: its best finish with the fill
// pinned, BF(0) as best now, candidate_costs's cost and tier, the name of the
// fill Hydra's best path starts on, and the candidate's activation.
struct OverratedCard {
    const Song& song = song_named("Allister - Overrated");
    app::PreviewScene empty = scene_for(song, player_path(song, {}));
    int64_t bf0 = best_finish(song, default_cfg(), {}).paths.at(0).totalscore();
    int64_t best_tick = corpus::analyzed(corpus::chart_named("Allister - Overrated"), default_cfg())
                            .best_path()
                            .all_activations()
                            .front()
                            .timecode.ticks();
    std::vector<Activation> with_acts;

    FillHoverNumbers numbers_at(int64_t tick) {
        const TargetResult with = best_finish(
            song, default_cfg(), {PinnedWindow{tick, std::nullopt, false, std::nullopt}});
        with_acts = with.paths.at(0).all_activations();
        const std::vector<app::CandidateCost> costs =
            app::candidate_costs(bf0, bf0, {{tick, with.paths.at(0).totalscore()}});
        FillHoverNumbers n;
        n.best_finish = costs.at(0).best_finish;
        n.best_now = bf0;
        n.cost = costs.at(0).cost;
        n.tier = costs.at(0).tier;
        n.best_fill = fill_name(empty, empty.fills[fill_index(empty, best_tick)]);
        n.hydra_end = &with_acts.front();
        return n;
    }
};

std::vector<std::string> row_texts(const FillHoverCard& card) {
    std::vector<std::string> out;
    for (const CardRow& r : card.rows) out.push_back(r.label + "=" + r.value);
    return out;
}

}  // namespace

TEST_CASE("fill card: an offered fill reads the heading, three rows and two grey lines") {
    OverratedCard c;
    const FillHoverCard card =
        fill_hover_card(c.empty, fill_index(c.empty, 55680), c.numbers_at(55680));
    // Pinned from one run: BF(0) 228,710 less this fill's 226,210 is the
    // 2,500 the Cost row reads, and m38 is where the best path starts.
    CHECK(card.heading == "m30 · 0:45.789");
    CHECK(row_texts(card) == std::vector<std::string>{"Phrases saved=2",
                                                      "Best finish here=226,210",
                                                      "Cost=\xE2\x88\x92" "2,500"});
    CHECK(card.cost_tier == app::CostTier::Red);
    CHECK(card.grey ==
          std::vector<std::string>{"Best choice now is m38 (228,710).", "Hydra would end it at m34."});
}

TEST_CASE("fill card: a costless fill reads best and \"This is the best choice now.\"") {
    OverratedCard c;
    const FillHoverCard card =
        fill_hover_card(c.empty, fill_index(c.empty, c.best_tick), c.numbers_at(c.best_tick));
    CHECK(card.heading == "m38 · 0:58.421");
    CHECK(row_texts(card) == std::vector<std::string>{"Phrases saved=3",
                                                      "Best finish here=228,710", "Cost=best"});
    CHECK(card.cost_tier == app::CostTier::Green);
    REQUIRE(card.grey.size() == 2);
    CHECK(card.grey[0] == "This is the best choice now.");
    CHECK(card.grey[1] == "Hydra would end it at m44.");
}

TEST_CASE("fill card: taken, locked, refused and pending fills read the heading and their one "
          "line") {
    // Trivium "Feast of Fire" on Hydra's best path: fill 4 is the first
    // activation, fill 3 is before the last one, and fill 5 is under the
    // first window's Star Power (the fill refusal test above).
    const Song& song = song_named("Trivium - Feast of Fire");
    const HydraRecord& rec =
        corpus::analyzed(corpus::chart_named("Trivium - Feast of Fire"), default_cfg());
    const app::PreviewScene best = scene_for(song, rec.best_path());
    REQUIRE(best.fills.size() > 5);
    const FillHoverCard taken = fill_hover_card(best, 4, std::nullopt);
    CHECK(taken.heading == "m34 · 0:45.435");
    CHECK(taken.rows.empty());
    CHECK_FALSE(taken.cost_tier);
    CHECK(taken.grey == std::vector<std::string>{kYourActivationHoverText});
    const FillHoverCard locked = fill_hover_card(best, 3, std::nullopt);
    CHECK(locked.heading == "m30 · 0:40.041");
    CHECK(locked.rows.empty());
    CHECK(locked.grey == std::vector<std::string>{kBeforeLastActivationText});
    const FillHoverCard running = fill_hover_card(best, 5, std::nullopt);
    CHECK(running.heading == "m38 · 0:50.828");
    CHECK(running.rows.empty());
    CHECK(running.grey ==
          std::vector<std::string>{"Star Power is already on here. Activation 1 runs until m40."});

    // Allister "Overrated" with no activation: the first fill comes with one
    // phrase, and the m30 fill's search has not landed.
    OverratedCard c;
    const FillHoverCard under = fill_hover_card(c.empty, 0, std::nullopt);
    CHECK(under.heading == "m20 · 0:30.000");
    CHECK(under.rows.empty());
    CHECK(under.grey ==
          std::vector<std::string>{"You have 1 phrase saved here. You need 2 to activate."});
    const FillHoverCard pending = fill_hover_card(c.empty, fill_index(c.empty, 55680), std::nullopt);
    CHECK(pending.heading == "m30 · 0:45.789");
    CHECK(row_texts(pending) == std::vector<std::string>{"Phrases saved=2",
                                                         "Best finish here=...", "Cost=..."});
    CHECK_FALSE(pending.cost_tier);
    CHECK(pending.grey.empty());
}
