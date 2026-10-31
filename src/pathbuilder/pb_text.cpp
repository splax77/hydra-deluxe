#include "pathbuilder/pb_text.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "app/display_format.h"
#include "app/path_costs.h"  // costs_nothing
#include "core/backend_value.h"    // after_sp_end
#include "pathbuilder/pb_coach.h"  // tight_window_note

namespace hydra::pathbuilder {

namespace {

// intro_text up to the star.
std::string intro_before_star() {
    return "Build your own Star Power path, left to right. Click a glowing fill to activate Star "
           "Power there. You need at least " + std::to_string(kSpActivationBars) +
           " Star Power phrases";
}

}  // namespace

std::string intro_text() {
    return intro_before_star() +
           " saved since your last activation. A fill's colour is what activating there costs "
           "you. The highway below shows the notes wherever you hover or click.";
}

size_t intro_star_at() { return intro_before_star().size(); }

const char* const kNoStepsText =
    "None yet. Each activation you make shows up here: where it starts and ends, any squeeze it "
    "needs, and what it cost you.";
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
const char* const kNothingLeftText = "Nothing left on the table after your last activation.";
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

// The squeezes an activation's sentences name: its badge_timings without the
// early-fill part, in the order listed. squeeze_end_text and the list row's
// short form both read this, so which timings count is decided here once.
std::vector<ActivationTiming> squeezes_of(const Activation& act) {
    std::vector<ActivationTiming> out;
    for (const ActivationTiming& t : act.badge_timings())
        if (t.part != TimingPart::EarlyFill) out.push_back(t);
    return out;
}

const char* squeeze_word(const ActivationTiming& t) {
    return t.part == TimingPart::SqueezeOut ? "squeeze-out" : "squeeze-in";
}

// "squeeze-out 12.3 ms, squeeze-in, no timing": the mockup's sqShort.
std::string squeeze_short_text(const Activation& act) {
    std::string out;
    for (const ActivationTiming& t : squeezes_of(act)) {
        if (!out.empty()) out += ", ";
        out += squeeze_word(t);
        out += t.required ? " " + format_ms(t.ms) : ", no timing";
    }
    return out;
}

}  // namespace

std::string squeeze_end_text(const Activation& act) {
    std::string out;
    for (const ActivationTiming& t : squeezes_of(act)) {
        out += out.empty() ? ", ending with " : " and ";
        out += std::string("a ") + squeeze_word(t);
        out += t.required ? " that needs " + format_ms(t.ms) : " that needs no timing";
    }
    return out;
}

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
    if (const std::optional<int64_t> d = end->deact_tick()) {
        t += app::kFactSeparator + std::string("ends ") + measure_at(timing, *d);
        const std::string sq = squeeze_short_text(*end);
        if (!sq.empty()) t += app::kFactSeparator + sq;
    }
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
    return s;
}

std::string cost_text(int64_t cost) {
    if (app::costs_nothing(cost)) return "costs nothing";
    return "costs " + group_thousands(cost) + " points";
}

std::string cost_or_best(int64_t cost) {
    return app::costs_nothing(cost) ? std::string(kBestFillLabel) : app::format_cost(cost);
}

namespace {

// "Hydra would end it at m97, ending with ... ." for the candidate's
// activation, or nothing when it has no deact node. fill_hover_text and
// fill_hover_card both say it.
std::optional<std::string> hydra_end_sentence(const PreviewScene& scene, const Activation& end) {
    const std::optional<int64_t> d = end.deact_tick();
    if (!d) return std::nullopt;
    return "Hydra would end it at " + measure_at(scene_timing(scene), *d) + squeeze_end_text(end) +
           ".";
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

StepText step_sentence(const SongTiming& timing, const Activation& act, int64_t cost,
                       int64_t before, int64_t after,
                       const std::optional<double>& limit_ms) {
    StepText s;
    s.sentence = "Activate at " + measure_label(act.timecode) + " (" +
                 app::clock_str(act.timecode.ms()) + ") with " +
                 counted(act.sp_meter(), "phrase", "phrases") + " saved.";
    if (const std::optional<int64_t> d = act.deact_tick())
        s.sentence += " Star Power runs until " + measure_at(timing, *d) +
                      squeeze_end_text(act) + ".";
    s.sentence += " This choice " + cost_text(cost) + ".";
    s.why = !app::costs_nothing(cost)
                ? "Before it, the best you could still finish with was " + group_thousands(before) +
                      ". After it, " + group_thousands(after) + "."
                : "The best you can still finish with stays at " + group_thousands(after) + ".";
    s.note = tight_window_note(act, limit_ms);
    return s;
}

std::string leftover_text(int64_t after_last) {
    if (after_last <= 0) return kNothingLeftText;
    return "You still have Star Power to use. Activating the best way from here would add " +
           group_thousands(after_last) + " points.";
}

std::string glossary_text() {
    return "Cost: how many points a choice takes off the best score you can still reach. "
           "Squeeze-out: a Star Power phrase ends right where your Star Power runs out; hitting "
           "its last note late enough saves the phrase for your next activation. "
           "Squeeze-in: hitting a phrase's last note early enough that it still lands inside "
           "your Star Power and stretches it. "
           "The ms figure is how much timing it needs, as Hydra's Paths tab shows it.";
}

}  // namespace hydra::pathbuilder
