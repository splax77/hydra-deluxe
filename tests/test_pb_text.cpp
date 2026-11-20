// The Path Builder's sentences (pathbuilder/pb_text.h, whose header names the
// mockups and the D-records behind them).
// The corpus cases run at the GUI's default settings. Every expected figure
// is read from the engine's answer on corpus charts (the record, a search, or
// the chart's own timing), and every fixed sentence from its owner in
// pb_text.h; each test walks the corpus for the case it needs.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/display_format.h"  // clock_str, format_cost, kFactSeparator
#include "app/path_costs.h"      // candidate_costs, cost_tier
#include "app/path_view.h"       // activation_badges
#include "app/preview_view.h"
#include "core/backend_value.h"  // after_sp_end
#include "core/model.h"
#include "core/timing.h"  // measure_label, kSpActivationBars
#include "corpus_default_cfg.h"
#include "corpus_util.h"
#include "pathbuilder/pb_coach.h"  // start_window
#include "pathbuilder/pb_text.h"
#include "search/pather.h"
#include "text_split.h"  // split_on

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

// One corpus chart whose best path activates: its song and its record at the
// GUI's default settings.
struct ChartCase {
    std::string path;
    const Song* song = nullptr;
    const HydraRecord* rec = nullptr;
};

// Every such chart, in corpus order.
const std::vector<ChartCase>& charts_with_activations() {
    static const std::vector<ChartCase> out = [] {
        std::vector<ChartCase> v;
        for (const std::string& chart : corpus::chart_paths()) {
            const Song& song = corpus::song(chart, default_cfg().reading(), default_cfg().rules);
            if (song.is_empty()) continue;
            const HydraRecord& rec = corpus::analyzed(chart, default_cfg());
            if (rec.paths.empty() || rec.best_path().all_activations().empty()) continue;
            v.push_back(ChartCase{chart, &song, &rec});
        }
        return v;
    }();
    return out;
}

// Calls `fn` on every activation of every corpus best path, with its chart.
void for_each_best_activation(const std::function<void(const ChartCase&, const Activation&)>& fn) {
    REQUIRE_FALSE(charts_with_activations().empty());
    for (const ChartCase& c : charts_with_activations())
        for (const Activation& act : c.rec->best_path().all_activations()) fn(c, act);
}

// The pieces of a line the screen joins with app::kFactSeparator.
std::vector<std::string> split_facts(const std::string& text) {
    return split_on(text, app::kFactSeparator);
}

// The Paths tab's badge words for `act` (app::activation_badges), in order.
std::vector<std::string> badge_texts(const Activation& act) {
    std::vector<std::string> out;
    for (const app::ActivationBadge& b : app::activation_badges(act)) out.push_back(b.text);
    return out;
}

bool has_part(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

// The measure the chart's own timing puts `tick` in, as the screen names it.
std::string measure_of(const SongTiming& timing, int64_t tick) {
    return measure_label(timing.timecode(tick));
}

// A fill card's heading for a fill riding on `tick`, read from the chart's
// own timing: its measure and its clock.
std::vector<std::string> heading_parts(const SongTiming& timing, int64_t tick) {
    return {measure_of(timing, tick), app::clock_str(timing.timecode(tick).ms())};
}

// The case an end can get wrong: `act` squeezed out a phrase chord in an
// earlier measure than its stored end (Activation::deact_tick), and that
// chord is not after the end (core::after_sp_end), so Star Power is over on
// the chord. The chord's measure when so, else none.
std::optional<std::string> earlier_squeezed_measure(const SongTiming& timing,
                                                    const Activation& act) {
    if (!act.sqout_tick || !act.deact_tick()) return std::nullopt;
    if (core::after_sp_end(*act.sqout_tick, *act.deact_tick())) return std::nullopt;
    const std::string chord = measure_of(timing, *act.sqout_tick);
    if (chord == measure_of(timing, *act.deact_tick())) return std::nullopt;
    return chord;
}

// The fixed word before an end measure in end_choice_text; pb_text.cpp types
// it in place, with no constant of its own.
const std::string kEndsWord = "ends ";
// The arrow step_short puts between a step's start and end, typed in
// pb_text.cpp the same way.
const std::string kSpanArrow = " \xE2\x86\x92 ";

// The measure an end piece names after kEndsWord; none when the piece does
// not start with that word.
std::optional<std::string> end_measure(const std::string& piece) {
    if (piece.compare(0, kEndsWord.size(), kEndsWord) != 0) return std::nullopt;
    return piece.substr(kEndsWord.size());
}

}  // namespace

TEST_CASE("candidate row: the clock, phrases, end and squeeze of a priced fill") {
    // The player's path is a corpus best path's windows before one that ends
    // on an earlier squeezed-out chord (earlier_squeezed_measure), so that
    // window's fill is the next choice, and its row is priced with the
    // record's activation there. The first such case in the corpus is used.
    for (const ChartCase& c : charts_with_activations()) {
        const std::vector<Activation> acts = c.rec->best_path().all_activations();
        for (size_t k = 0; k < acts.size(); ++k) {
            const Activation& act = acts[k];
            const std::optional<std::string> chord_m =
                earlier_squeezed_measure(c.song->timing(), act);
            if (!chord_m) continue;
            std::vector<int64_t> before;
            for (size_t j = 0; j < k; ++j) before.push_back(acts[j].timecode.ticks());
            const app::PreviewScene scene = scene_for(*c.song, player_path(*c.song, before));
            const std::optional<size_t> idx = app::fill_index_on_chord(scene, act.timecode.ticks());
            if (!idx || fill_refusal(scene, *idx) != FillRefusal::None) continue;
            INFO(c.path << " | activation " << k + 1 << " at " << act.timecode.ticks());
            const SongTiming& timing = *scene.timing;
            const PreviewFill& fill = scene.fills[*idx];
            const std::string clock = app::clock_str(timing.timecode(fill.chord_tick).ms());
            const std::string phrases = counted(bars_saved_at(scene, fill), "phrase", "phrases");
            const std::vector<std::string> row = split_facts(candidate_row_text(scene, *idx, &act));
            REQUIRE(row.size() >= 3);
            CHECK(row[0] == clock);
            CHECK(row[1] == phrases);
            // The window ends on the squeezed-out chord, not the stored end.
            CHECK(end_measure(row[2]) == *chord_m);
            // Then the Paths tab's badge words, one for one.
            CHECK(std::vector<std::string>(row.begin() + 3, row.end()) == badge_texts(act));
            // Before its search lands, the end reads kPendingText.
            CHECK(split_facts(candidate_row_text(scene, *idx, nullptr)) ==
                  std::vector<std::string>{clock, phrases, kPendingText});
            return;
        }
    }
    FAIL("no corpus best path has a next choice that ends on an earlier squeezed-out chord");
}

namespace {

// The fill card's numbers on one corpus chart with no activation yet, as the
// screen gathers them: BF(0) as best now, and for the fill at `tick` its best
// finish with it pinned, candidate_costs's cost and tier, the name of the
// fill Hydra's best path starts on, and the candidate's activation.
struct CardChart {
    const Song& song;
    app::PreviewScene empty;
    int64_t bf0 = 0;
    int64_t best_tick = 0;
    // The last numbers_at's best finish and its path's activations.
    int64_t with_total = 0;
    std::vector<Activation> with_acts;

    explicit CardChart(const ChartCase& c)
        : song(*c.song),
          empty(scene_for(*c.song, player_path(*c.song, {}))),
          bf0(best_finish(*c.song, default_cfg(), {}).paths.at(0).totalscore()),
          best_tick(c.rec->best_path().all_activations().front().timecode.ticks()) {}

    FillHoverNumbers numbers_at(int64_t tick) {
        const TargetResult with = best_finish(
            song, default_cfg(), {PinnedWindow{tick, std::nullopt, false, std::nullopt}});
        with_total = with.paths.at(0).totalscore();
        with_acts = with.paths.at(0).all_activations();
        const std::vector<app::CandidateCost> costs =
            app::candidate_costs(bf0, bf0, {app::CandidateFinish{tick, with_total}});
        FillHoverNumbers n;
        n.best_finish = costs.at(0).best_finish;
        n.best_now = bf0;
        n.cost = costs.at(0).cost;
        n.tier = costs.at(0).tier;
        n.best_fill = fill_name(empty, empty.fills[fill_index(empty, best_tick)]);
        for (const Activation& a : with_acts)
            if (a.timecode.ticks() == tick) n.hydra_end = &a;
        return n;
    }
};

std::vector<std::string> row_texts(const FillHoverCard& card) {
    std::vector<std::string> out;
    for (const CardRow& r : card.rows) out.push_back(r.label + "=" + r.value);
    return out;
}

std::string card_row(const char* label, const std::string& value) {
    return std::string(label) + "=" + value;
}

}  // namespace

TEST_CASE("choice order: cheapest first, ties in chart order, pending last") {
    // The first corpus chart whose path with no activation passes two fills
    // or more. Every passed fill but the last is priced by its own best
    // finish, as the candidate queue prices it; the last is still pending.
    for (const ChartCase& c : charts_with_activations()) {
        CardChart card(c);
        const std::vector<int64_t>& offered = card.empty.trailing_skipped_fill_ticks;
        if (offered.size() < 2) continue;
        INFO(c.path);
        std::vector<app::CandidateCost> priced;
        for (size_t i = 0; i + 1 < offered.size(); ++i) {
            const FillHoverNumbers n = card.numbers_at(offered[i]);
            priced.push_back(app::CandidateCost{offered[i], n.best_finish, n.cost, n.tier});
        }
        const std::vector<int64_t> order = choice_order(offered, priced);
        const auto cost_of = [&](int64_t tick) -> std::optional<int64_t> {
            for (const app::CandidateCost& p : priced)
                if (p.act_tick == tick) return p.cost;
            return std::nullopt;
        };
        const auto chart_pos = [&](int64_t tick) {
            return std::find(offered.begin(), offered.end(), tick) - offered.begin();
        };
        // Every passed fill, once.
        std::vector<int64_t> sorted_order = order;
        std::vector<int64_t> sorted_offered = offered;
        std::sort(sorted_order.begin(), sorted_order.end());
        std::sort(sorted_offered.begin(), sorted_offered.end());
        REQUIRE(sorted_order == sorted_offered);
        // The priced rows come first, then the pending ones.
        for (size_t i = 0; i < order.size(); ++i)
            CHECK(cost_of(order[i]).has_value() == (i < priced.size()));
        // Cheapest first, equal costs in chart order; the pending rows in
        // chart order.
        for (size_t i = 1; i < order.size(); ++i) {
            const std::optional<int64_t> a = cost_of(order[i - 1]);
            const std::optional<int64_t> b = cost_of(order[i]);
            INFO("rows " << order[i - 1] << " then " << order[i]);
            if (a && b) CHECK(*a <= *b);
            if ((!a && !b) || (a && b && *a == *b)) CHECK(chart_pos(order[i - 1]) < chart_pos(order[i]));
        }
        // Nothing priced yet: chart order.
        CHECK(choice_order(offered, {}) == offered);
        return;
    }
    FAIL("no corpus chart passes two fills with no activation");
}

TEST_CASE("step short: start, end and phrases") {
    // Every activation on every corpus best path: the span runs from the
    // activation's measure to its window's end (Activation::window_end_tick),
    // both as the chart's timing names them, and the phrases are the bars it
    // spends (Activation::sp_meter).
    for_each_best_activation([](const ChartCase& c, const Activation& act) {
        INFO(c.path << " | activation at " << act.timecode.ticks());
        const SongTiming& timing = c.song->timing();
        const StepShort s = step_short(timing, act);
        REQUIRE(act.window_end_tick().has_value());
        CHECK(split_on(s.span, kSpanArrow) ==
              std::vector<std::string>{measure_of(timing, act.timecode.ticks()),
                                       measure_of(timing, *act.window_end_tick())});
        CHECK(s.saved == counted(act.sp_meter(), "phrase", "phrases"));
    });
}

TEST_CASE("step short: the row carries the Paths tab's badge words") {
    // Every activation on every corpus best path carries its badges one for
    // one, and none when it has none. A window that ends on an earlier
    // squeezed-out chord ends its span on that chord's measure.
    int squeezed = 0;
    for_each_best_activation([&](const ChartCase& c, const Activation& act) {
        INFO(c.path << " | activation at " << act.timecode.ticks());
        const SongTiming& timing = c.song->timing();
        const StepShort s = step_short(timing, act);
        const std::vector<std::string> badges = badge_texts(act);
        if (badges.empty()) {
            CHECK(s.squeezes.empty());
        } else {
            CHECK(split_facts(s.squeezes) == badges);
        }
        if (const std::optional<std::string> chord = earlier_squeezed_measure(timing, act)) {
            ++squeezed;
            CHECK(split_on(s.span, kSpanArrow).back() == *chord);
        }
    });
    CHECK(squeezed > 0);
}

TEST_CASE("end choice text: the end measure and the Paths tab's badge words") {
    // Every activation on every corpus best path: "ends" and its window's
    // end measure, then the Paths tab's badges one for one. A window that
    // ends on an earlier squeezed-out chord names that chord's measure.
    int squeezed = 0;
    for_each_best_activation([&](const ChartCase& c, const Activation& act) {
        INFO(c.path << " | activation at " << act.timecode.ticks());
        const SongTiming& timing = c.song->timing();
        const std::vector<std::string> parts = split_facts(end_choice_text(timing, act));
        REQUIRE(act.window_end_tick().has_value());
        CHECK(end_measure(parts[0]) == measure_of(timing, *act.window_end_tick()));
        CHECK(std::vector<std::string>(parts.begin() + 1, parts.end()) == badge_texts(act));
        if (const std::optional<std::string> chord = earlier_squeezed_measure(timing, act)) {
            ++squeezed;
            CHECK(end_measure(parts[0]) == *chord);
        }
    });
    CHECK(squeezed > 0);
}

namespace {

// One scene of the player's path the refusal and card tests read: each
// corpus chart's best path, and the same chart with no activation.
struct SceneCase {
    std::string where;
    app::PreviewScene scene;
};

const std::vector<SceneCase>& refusal_scenes() {
    static const std::vector<SceneCase> out = [] {
        std::vector<SceneCase> v;
        for (const ChartCase& c : charts_with_activations()) {
            v.push_back(SceneCase{c.path + " | best path", scene_for(*c.song, c.rec->best_path())});
            v.push_back(SceneCase{c.path + " | no activation",
                                  scene_for(*c.song, player_path(*c.song, {}))});
        }
        return v;
    }();
    return out;
}

// The first of the scene's windows whose Star Power is on at the fill's chord
// (core::after_sp_end, the engine's side-of-the-end rule), or none.
std::optional<size_t> running_window_at(const app::PreviewScene& scene, const PreviewFill& fill) {
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const app::PreviewActivation& a = scene.activations[i];
        if (a.sp_window() && fill.chord_tick > a.tick &&
            !core::after_sp_end(fill.chord_tick, a.sp_end_tick))
            return i;
    }
    return std::nullopt;
}

}  // namespace

TEST_CASE("fill refusal: the five kinds in fill_reason's order, and fill_reason's sentence for "
          "each") {
    // Every fill of every scene in refusal_scenes. Each kind holds only when
    // the facts before it in fill_reason's order (pb_text.h) do not, and its
    // sentence names what the kind says.
    // One count per FillRefusal kind; GameHides is the last.
    std::vector<int> seen(static_cast<size_t>(FillRefusal::GameHides) + 1, 0);
    for (const SceneCase& sc : refusal_scenes()) {
        const app::PreviewScene& scene = sc.scene;
        for (size_t i = 0; i < scene.fills.size(); ++i) {
            const PreviewFill& fill = scene.fills[i];
            INFO(sc.where << " | fill at " << fill.chord_tick);
            FillRefusal kind = FillRefusal::None;
            const std::string reason = fill_reason(scene, i, kind);
            // The sentence and the kind come from one walk.
            CHECK(reason == fill_reason(scene, i));
            CHECK(kind == fill_refusal(scene, i));
            ++seen[static_cast<size_t>(kind)];
            const bool taken = fill.state == app::PreviewFillState::Taken;
            const std::optional<size_t> over = running_window_at(scene, fill);
            const bool before_last =
                !scene.activations.empty() && fill.chord_tick < scene.activations.back().tick;
            const bool offered = fill.state == app::PreviewFillState::Offered;
            const int bars = bars_saved_at(scene, fill);
            CHECK((kind == FillRefusal::Taken) == taken);
            if (taken) {
                CHECK(reason.empty());
                continue;
            }
            CHECK((kind == FillRefusal::SpRunning) == over.has_value());
            if (over) {
                CHECK(has_part(reason, "Activation " + std::to_string(*over + 1) + " "));
                CHECK(has_part(reason, measure_of(*scene.timing,
                                                  scene.activations[*over].sp_end_tick)));
                continue;
            }
            CHECK((kind == FillRefusal::BeforeLast) == before_last);
            if (before_last) {
                CHECK(reason == kBeforeLastActivationText);
                continue;
            }
            CHECK((kind == FillRefusal::None) == offered);
            if (offered) {
                CHECK(reason.empty());
                continue;
            }
            CHECK((kind == FillRefusal::UnderBars) == (bars < kSpActivationBars));
            if (bars < kSpActivationBars) {
                CHECK(has_part(reason, counted(bars, "phrase", "phrases")));
                CHECK(has_part(reason, std::to_string(kSpActivationBars)));
                continue;
            }
            CHECK(kind == FillRefusal::GameHides);
            CHECK(reason == kGameHidesFillText);
        }
    }
    // Every kind turns up somewhere in the corpus.
    for (size_t k = 0; k < seen.size(); ++k) {
        INFO("FillRefusal " << k);
        CHECK(seen[k] > 0);
    }
}

TEST_CASE("fill refusal: a fill on the window's end tick is still under Star Power") {
    // A chord on the end tick is still under Star Power (core::after_sp_end).
    // The case: one window on a fill a corpus chart offers with no
    // activation, whose Star Power ends on another fill's chord. The first
    // such case in the corpus is used.
    for (const ChartCase& c : charts_with_activations()) {
        const app::PreviewScene empty = scene_for(*c.song, player_path(*c.song, {}));
        for (const int64_t start : empty.trailing_skipped_fill_ticks) {
            const app::PreviewScene one = scene_for(*c.song, player_path(*c.song, {start}));
            if (one.activations.size() != 1 || !one.activations[0].sp_window()) continue;
            const int64_t end = one.activations[0].sp_end_tick;
            const std::optional<size_t> at_end = app::fill_index_on_chord(one, end);
            if (!at_end) continue;
            INFO(c.path << " | one window at " << start << " ending at " << end);
            CHECK(fill_refusal(one, *at_end) == FillRefusal::SpRunning);
            CHECK(has_part(fill_reason(one, *at_end), measure_of(*one.timing, end)));
            return;
        }
    }
    FAIL("no corpus chart has a one-window path whose Star Power ends on a fill's chord");
}

// The Path Builder's choice rows are the fills the path passed after its last
// activation (draw_choices reads trailing_skipped_fill_ticks), and each row
// prints the phrases saved there (candidate_row_text, through bars_saved_at).
// Inside a running window that figure would be the meter while SP drains, not
// phrases saved, so no row may sit there. fill_reason's SpRunning test is the
// screen's word for "inside a running window".
//
// The rows are built for the player's path, not for Hydra's stored paths: the
// screen's path is PbState's empty path or a click's ClickResult::path
// (PbApp::shown_path). So each case here builds the path the screen would
// show, and reads the rows from that path's scene as draw_choices does.
namespace {

// Checks every choice row of `scene` and returns how many there were.
int check_choice_rows(const app::PreviewScene& scene, const std::string& where) {
    int rows = 0;
    for (const int64_t tick : scene.trailing_skipped_fill_ticks) {
        const std::optional<size_t> idx = app::fill_index_on_chord(scene, tick);
        INFO(where << " | passed fill " << tick);
        REQUIRE(idx.has_value());
        ++rows;
        if (fill_refusal(scene, *idx) == FillRefusal::SpRunning)
            MESSAGE("row inside a window: " << candidate_row_text(scene, *idx, nullptr) << " | "
                                            << fill_reason(scene, *idx));
        CHECK(fill_refusal(scene, *idx) != FillRefusal::SpRunning);
    }
    return rows;
}

}  // namespace

TEST_CASE("candidate rows: no fill the path passed sits inside a running window") {
    // Every corpus chart with Star Power, after the player's first click on
    // the fill Hydra's best path activates first (start_window, the screen's
    // click). Only the last window can hold a row, since every row is after
    // it, so one window per chart is enough to put each chart's rows beside a
    // running window.
    int rows_checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, default_cfg().reading(), default_cfg().rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, default_cfg());
        if (rec.paths.empty() || rec.best_path().walk_activations().empty()) continue;
        const int64_t first = rec.best_path().walk_activations()[0].timecode.ticks();
        const ScoreGraph graph = search_graph(song, default_cfg());
        const ClickResult click = start_window(graph, default_cfg(), {}, first);
        INFO(chart << " | click at " << first << " | " << click.message);
        REQUIRE(click.message.empty());
        REQUIRE(click.path.has_value());
        rows_checked += check_choice_rows(scene_for(song, *click.path),
                                          chart + " | path " + click.path->pathstring());
    }
    MESSAGE("choice rows checked: " << rows_checked);
    CHECK(rows_checked > 0);
}

TEST_CASE("candidate rows: a fill on the window's end tick is not a choice row") {
    // Trivium "Feast of Fire" with one window at 180000 that ends at 195360,
    // and a fill exactly on 195360. A chord on the end tick is still under
    // Star Power (core::after_sp_end), so the engine must not list that fill
    // as passed, and so it is no row.
    const Song& song = song_named("Trivium - Feast of Fire");
    const app::PreviewScene one = scene_for(song, player_path(song, {180000}));
    REQUIRE(one.activations.size() == 1);
    REQUIRE(one.activations[0].sp_end_tick == 195360);
    const size_t at_end = fill_index(one, 195360);
    REQUIRE(fill_refusal(one, at_end) == FillRefusal::SpRunning);
    const std::vector<int64_t>& rows = one.trailing_skipped_fill_ticks;
    CHECK(std::find(rows.begin(), rows.end(), int64_t{195360}) == rows.end());
    CHECK(one.fills[at_end].state != app::PreviewFillState::Offered);
    // The path does pass fills after the window, so the rows are there to
    // check, and none of them is inside it.
    CHECK(check_choice_rows(one, "Feast of Fire | one window at 180000") > 0);
}

TEST_CASE("fill card: an offered fill reads the heading, three rows and two grey lines") {
    // The first fill a corpus chart offers with no activation that costs
    // something: its best finish with it pinned is below BF(0).
    for (const ChartCase& c : charts_with_activations()) {
        CardChart card(c);
        for (const int64_t tick : card.empty.trailing_skipped_fill_ticks) {
            if (tick == card.best_tick) continue;
            const FillHoverNumbers n = card.numbers_at(tick);
            if (card.with_total == card.bf0) continue;
            INFO(c.path << " | fill at " << tick);
            const size_t idx = fill_index(card.empty, tick);
            const PreviewFill& fill = card.empty.fills[idx];
            const SongTiming& timing = *card.empty.timing;
            REQUIRE(fill_refusal(card.empty, idx) == FillRefusal::None);
            // The cost is BF(0) less this fill's best finish, the two
            // searches this test ran.
            const int64_t cost = card.bf0 - card.with_total;
            CHECK(n.cost == cost);
            const FillHoverCard hover = fill_hover_card(card.empty, idx, n);
            CHECK(split_facts(hover.heading) == heading_parts(timing, fill.chord_tick));
            CHECK(row_texts(hover) ==
                  std::vector<std::string>{
                      card_row(kPhrasesSavedLabel, std::to_string(bars_saved_at(card.empty, fill))),
                      card_row(kBestFinishHereLabel, group_thousands(card.with_total)),
                      card_row(kCostLabel, app::format_cost(cost))});
            CHECK(hover.cost_tier == app::cost_tier(cost, card.bf0));
            // The grey lines name the best choice with BF(0), and where Hydra
            // would end the candidate's window.
            REQUIRE(hover.grey.size() == 2);
            CHECK(has_part(hover.grey[0], n.best_fill));
            CHECK(has_part(hover.grey[0], group_thousands(card.bf0)));
            REQUIRE(n.hydra_end != nullptr);
            REQUIRE(n.hydra_end->window_end_tick().has_value());
            CHECK(has_part(hover.grey[1], measure_of(timing, *n.hydra_end->window_end_tick())));
            return;
        }
    }
    FAIL("no corpus chart offers a fill that costs something with no activation");
}

TEST_CASE("fill card: a costless fill reads best and \"This is the best choice now.\"") {
    // The fill Hydra's best path starts on, on the first corpus chart that
    // offers it with no activation: pinning it keeps BF(0), so it costs
    // nothing.
    for (const ChartCase& c : charts_with_activations()) {
        CardChart card(c);
        const size_t idx = fill_index(card.empty, card.best_tick);
        if (fill_refusal(card.empty, idx) != FillRefusal::None) continue;
        INFO(c.path << " | fill at " << card.best_tick);
        const FillHoverNumbers n = card.numbers_at(card.best_tick);
        const PreviewFill& fill = card.empty.fills[idx];
        const SongTiming& timing = *card.empty.timing;
        CHECK(card.with_total == card.bf0);
        CHECK(n.cost == 0);
        const FillHoverCard hover = fill_hover_card(card.empty, idx, n);
        CHECK(split_facts(hover.heading) == heading_parts(timing, fill.chord_tick));
        CHECK(row_texts(hover) ==
              std::vector<std::string>{
                  card_row(kPhrasesSavedLabel, std::to_string(bars_saved_at(card.empty, fill))),
                  card_row(kBestFinishHereLabel, group_thousands(card.bf0)),
                  card_row(kCostLabel, kBestFillLabel)});
        CHECK(hover.cost_tier == app::cost_tier(0, card.bf0));
        REQUIRE(hover.grey.size() == 2);
        CHECK(hover.grey[0] == kBestChoiceNowText);
        REQUIRE(n.hydra_end != nullptr);
        REQUIRE(n.hydra_end->window_end_tick().has_value());
        CHECK(has_part(hover.grey[1], measure_of(timing, *n.hydra_end->window_end_tick())));
        return;
    }
    FAIL("no corpus chart offers its best path's first fill with no activation");
}

TEST_CASE("fill card: taken, locked, refused and pending fills read the heading and their one "
          "line") {
    // Every fill of every scene in refusal_scenes, before its search lands.
    // The heading is the fill's measure and clock by the chart's own timing.
    // A taken fill says kYourActivationHoverText alone; a refused one its
    // fill_reason alone; one the player can take shows its phrases and
    // kPendingText for the two figures still being searched.
    int taken = 0;
    int refused = 0;
    int pending = 0;
    for (const SceneCase& sc : refusal_scenes()) {
        const app::PreviewScene& scene = sc.scene;
        for (size_t i = 0; i < scene.fills.size(); ++i) {
            const PreviewFill& fill = scene.fills[i];
            INFO(sc.where << " | fill at " << fill.chord_tick);
            const FillHoverCard card = fill_hover_card(scene, i, std::nullopt);
            CHECK(split_facts(card.heading) == heading_parts(*scene.timing, fill.chord_tick));
            CHECK_FALSE(card.cost_tier.has_value());
            const std::string reason = fill_reason(scene, i);
            if (fill.state == app::PreviewFillState::Taken) {
                ++taken;
                CHECK(card.rows.empty());
                CHECK(card.grey == std::vector<std::string>{kYourActivationHoverText});
            } else if (!reason.empty()) {
                ++refused;
                CHECK(card.rows.empty());
                CHECK(card.grey == std::vector<std::string>{reason});
            } else {
                ++pending;
                CHECK(row_texts(card) ==
                      std::vector<std::string>{
                          card_row(kPhrasesSavedLabel, std::to_string(bars_saved_at(scene, fill))),
                          card_row(kBestFinishHereLabel, kPendingText),
                          card_row(kCostLabel, kPendingText)});
                CHECK(card.grey.empty());
            }
        }
    }
    CHECK(taken > 0);
    CHECK(refused > 0);
    CHECK(pending > 0);
}
