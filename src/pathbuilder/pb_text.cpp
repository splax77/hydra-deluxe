#include "pathbuilder/pb_text.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "app/display_format.h"
#include "app/path_costs.h"  // costs_nothing
#include "app/path_view.h"   // activation_badges
#include "core/backend_value.h"  // after_sp_end

namespace hydra::pathbuilder {

const char* const kNoStepsText =
    "None yet. Each activation you make shows up here: where it starts and ends, and what it "
    "cost you.";
const char* const kActivationRemovedText = "Activation removed.";
const char* const kStartedOverText =
    "Started over. Click a glowing fill to make your first activation.";
const char* const kBuiltWholePathText =
    "Hydra built the whole path the best way. Undo takes it back.";
const char* const kYourActivationHoverText = "Your activation. Click to see it below.";
const char* const kBuildLeftToRightText =
    "You build left to right; Undo or Remove goes back a step.";

namespace {

// The locked sentence about `subject` ("This", or a fill's name).
std::string before_last_sentence(const std::string& subject) {
    return subject + " is before your last activation. " + kBuildLeftToRightText;
}

}  // namespace

const std::string kBeforeLastActivationText = before_last_sentence("This");
const char* const kGameHidesFillText = "The game doesn't show this fill here.";
const char* const kBestChoiceNowText = "This is the best choice now.";
const char* const kPendingText = "...";

namespace {

const SongTiming& scene_timing(const PreviewScene& scene) {
    if (!scene.timing) throw std::logic_error("pb_text: a scene with no song timing");
    return *scene.timing;
}

// The measure a tick falls in, as the screen names it (measure_label).
std::string measure_at(const SongTiming& timing, int64_t tick) {
    return measure_label(timing.timecode(tick));
}

}  // namespace

std::string fill_name(const PreviewScene& scene, const PreviewFill& fill) {
    return measure_at(scene_timing(scene), fill.chord_tick);
}

int bars_saved_at(const PreviewScene& scene, const PreviewFill& fill) {
    return static_cast<int>(std::floor(app::sp_meter_bars_at(scene.sp_meter, fill.span.end_ms)));
}

std::string fill_reason(const PreviewScene& scene, size_t fill_index, FillRefusal& kind) {
    const PreviewFill& fill = scene.fills.at(fill_index);
    kind = FillRefusal::Taken;
    if (fill.state == app::PreviewFillState::Taken) return {};
    const SongTiming& timing = scene_timing(scene);
    kind = FillRefusal::SpRunning;
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        // Whether the fill's chord falls under this window is the engine's
        // side-of-the-end rule (core::after_sp_end), the one the fill's state
        // came from, so a chord on the end tick is still under Star Power.
        const app::PreviewActivation& a = scene.activations[i];
        if (a.sp_window() && fill.chord_tick > a.tick && !core::after_sp_end(fill.chord_tick, a.sp_end_tick))
            return "Star Power is already on here. Activation " + std::to_string(i + 1) +
                   " runs until " + measure_at(timing, a.sp_end_tick) + ".";
    }
    kind = FillRefusal::BeforeLast;
    if (!scene.activations.empty() && fill.chord_tick < scene.activations.back().tick)
        return kBeforeLastActivationText;
    kind = FillRefusal::None;
    if (fill.state == app::PreviewFillState::Offered) return {};
    kind = FillRefusal::UnderBars;
    const int bars = bars_saved_at(scene, fill);
    if (bars < kSpActivationBars)
        return "You have " + counted(bars, "phrase", "phrases") + " saved here. You need " +
               std::to_string(kSpActivationBars) + " to activate.";
    kind = FillRefusal::GameHides;
    return kGameHidesFillText;
}

std::string fill_reason(const PreviewScene& scene, size_t fill_index) {
    FillRefusal kind = FillRefusal::None;
    return fill_reason(scene, fill_index, kind);
}

FillRefusal fill_refusal(const PreviewScene& scene, size_t fill_index) {
    FillRefusal kind = FillRefusal::None;
    fill_reason(scene, fill_index, kind);
    return kind;
}

namespace {

// An activation's timings as the Paths tab's badges word them
// (app::activation_badges), joined by app::kFactSeparator; empty when it has
// none. The builder words no squeeze or early fill itself: every place it
// names one reads this.
std::string badges_text(const Activation& act) {
    std::string out;
    for (const app::ActivationBadge& b : app::activation_badges(act)) {
        if (!out.empty()) out += app::kFactSeparator;
        out += b.text;
    }
    return out;
}

// " · squeeze out 12.3 ms" after an end measure: badges_text behind the
// separator, or nothing when the activation has no timing.
std::string badges_after(const Activation& act) {
    const std::string b = badges_text(act);
    return b.empty() ? b : app::kFactSeparator + b;
}

}  // namespace

const char* const kChoicesHeading = "Where to activate next";
const char* const kChoicesHint =
    "Cheapest first. Hover a row to see it on the highway; click it to activate there.";
const char* const kNoChoicesFinishedText =
    "No fill after your last activation can be activated. Your path is finished.";
const char* const kNoChoicesText = "No fill can be activated.";

std::string candidate_row_text(const PreviewScene& scene, size_t fill_index,
                               const Activation* end) {
    const PreviewFill& fill = scene.fills.at(fill_index);
    const SongTiming& timing = scene_timing(scene);
    std::string t = app::clock_str(timing.timecode(fill.chord_tick).ms()) + app::kFactSeparator +
                    counted(bars_saved_at(scene, fill), "phrase", "phrases");
    if (!end) return t + app::kFactSeparator + kPendingText;
    if (end->deact_tick()) t += app::kFactSeparator + end_choice_text(timing, *end);
    return t;
}

std::vector<int64_t> choice_order(const std::vector<int64_t>& offered,
                                  const std::vector<app::CandidateCost>& priced) {
    struct Row {
        int64_t tick;
        std::optional<int64_t> cost;  // empty while the fill is being priced
    };
    std::vector<Row> rows;
    rows.reserve(offered.size());
    for (const int64_t tick : offered) {
        Row r{tick, std::nullopt};
        for (const app::CandidateCost& c : priced)
            if (c.act_tick == tick) r.cost = c.cost;
        rows.push_back(r);
    }
    // Stable, so ties and the pending rows keep chart order.
    std::stable_sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        if (a.cost.has_value() != b.cost.has_value()) return a.cost.has_value();
        return a.cost && *a.cost < *b.cost;
    });
    std::vector<int64_t> out;
    out.reserve(rows.size());
    for (const Row& r : rows) out.push_back(r.tick);
    return out;
}

StepShort step_short(const SongTiming& timing, const Activation& act) {
    StepShort s;
    s.span = measure_label(act.timecode);
    if (const std::optional<int64_t> d = act.deact_tick())
        s.span += " \xE2\x86\x92 " + measure_at(timing, *d);  // a right arrow
    s.saved = counted(act.sp_meter(), "phrase", "phrases");
    s.squeezes = badges_text(act);
    return s;
}

std::string end_choice_text(const SongTiming& timing, const Activation& act) {
    const std::optional<int64_t> d = act.deact_tick();
    return (d ? "ends " + measure_at(timing, *d) : act.notationstr()) + badges_after(act);
}

std::string cost_or_best(int64_t cost) {
    return app::costs_nothing(cost) ? std::string(kBestFillLabel) : app::format_cost(cost);
}

namespace {

// "Hydra would end it at m97 · squeeze out 0.0 ms." for the candidate's
// activation, or nothing when it has no deact node. fill_hover_card says it.
std::optional<std::string> hydra_end_sentence(const PreviewScene& scene, const Activation& end) {
    const std::optional<int64_t> d = end.deact_tick();
    if (!d) return std::nullopt;
    return "Hydra would end it at " + measure_at(scene_timing(scene), *d) + badges_after(end) + ".";
}

// The heading's name and clock, "m87 · 2:25.655".
std::string fill_heading(const PreviewScene& scene, const PreviewFill& fill) {
    return fill_name(scene, fill) + app::kFactSeparator +
           app::clock_str(scene_timing(scene).timecode(fill.chord_tick).ms());
}

}  // namespace

FillHoverCard fill_hover_card(const PreviewScene& scene, size_t fill_index,
                              const std::optional<FillHoverNumbers>& numbers) {
    const PreviewFill& fill = scene.fills.at(fill_index);
    FillHoverCard card;
    card.heading = fill_heading(scene, fill);
    if (fill.state == app::PreviewFillState::Taken) {
        card.grey.push_back(kYourActivationHoverText);
        return card;
    }
    const std::string reason = fill_reason(scene, fill_index);
    if (!reason.empty()) {
        card.grey.push_back(reason);
        return card;
    }
    card.rows.push_back({kPhrasesSavedLabel, std::to_string(bars_saved_at(scene, fill))});
    if (!numbers) {
        card.rows.push_back({kBestFinishHereLabel, kPendingText});
        card.rows.push_back({kCostLabel, kPendingText});
        return card;
    }
    card.rows.push_back({kBestFinishHereLabel, group_thousands(numbers->best_finish)});
    card.rows.push_back({kCostLabel, cost_or_best(numbers->cost)});
    card.cost_tier = numbers->tier;
    card.grey.push_back(app::costs_nothing(numbers->cost)
                            ? std::string(kBestChoiceNowText)
                            : "Best choice now is " + numbers->best_fill + " (" +
                                  group_thousands(numbers->best_now) + ").");
    if (numbers->hydra_end)
        if (std::optional<std::string> end = hydra_end_sentence(scene, *numbers->hydra_end))
            card.grey.push_back(std::move(*end));
    return card;
}

GuideMessage cant_activate_message(const std::string& fill, const std::string& reason) {
    return {"Can't activate at " + fill + ": " + reason, true};
}

GuideMessage locked_message(const std::string& fill) { return {before_last_sentence(fill), true}; }

GuideMessage removed_message(size_t index, bool last) {
    if (last) return {kActivationRemovedText, false};
    return {"Removed activation " + std::to_string(index + 1) + " and everything after it.",
            false};
}

GuideMessage started_over_message() { return {kStartedOverText, false}; }

GuideMessage finished_for_player_message(size_t kept) {
    if (kept == 0) return {kBuiltWholePathText, false};
    return {"Hydra kept your " + counted(static_cast<int64_t>(kept), "activation", "activations") +
                " and added the best rest of the path. Undo takes it back.",
            false};
}

GuideMessage guide_text(const GuideFacts& facts) {
    if (facts.message) return *facts.message;
    const std::string need = std::to_string(kSpActivationBars);
    if (!facts.last_end) {
        std::string t = "Step 1. Click a glowing fill to make your first activation.";
        if (facts.earliest_fill)
            t += " The earliest you can activate is " + *facts.earliest_fill + " (" +
                 app::clock_str(facts.earliest_ms) + "), once you've saved " + need +
                 " phrases.";
        return {t + " Hover a fill to see it on the highway.", false};
    }
    std::string t = "Your Star Power runs until " + *facts.last_end + ".";
    if (facts.fills_after_last == 0)
        return {t + " No fill after that comes with " + need +
                    " saved phrases, so your path is finished. Your score is " +
                    group_thousands(facts.player_total) + ".",
                false};
    t += " Save " + need + " more phrases";
    const size_t star_at = t.size();
    t += ", then activate again.";
    if (facts.earliest_fill) t += " The next glowing fill is " + *facts.earliest_fill + ".";
    if (facts.best_fill) t += " Your best next choice is " + *facts.best_fill + ".";
    return {t, false, star_at};
}

}  // namespace hydra::pathbuilder
