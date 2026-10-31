#include "pathbuilder/pb_text.h"

#include <cmath>
#include <stdexcept>
#include <vector>

#include "app/display_format.h"
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
           " saved since your last activation. The number under each glowing fill is what "
           "activating there costs you. The highway below shows the notes wherever you hover or "
           "click.";
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
const char* const kBeforeLastActivationText =
    "This is before your last activation. You build left to right; Undo goes back a step.";
const char* const kGameHidesFillText = "The game doesn't show this fill here.";
const char* const kBestChoiceNowText = "That is the best choice right now.";
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

std::string fill_reason(const PreviewScene& scene, size_t fill_index) {
    const PreviewFill& fill = scene.fills.at(fill_index);
    if (fill.state == app::PreviewFillState::Taken) return {};
    const SongTiming& timing = scene_timing(scene);
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        // Whether the fill's chord falls under this window is the engine's
        // side-of-the-end rule (core::after_sp_end), the one the fill's state
        // came from, so a chord on the end tick is still under Star Power.
        const app::PreviewActivation& a = scene.activations[i];
        if (a.sp_window() && fill.chord_tick > a.tick && !core::after_sp_end(fill.chord_tick, a.sp_end_tick))
            return "Star Power is already on here. Activation " + std::to_string(i + 1) +
                   " runs until " + measure_at(timing, a.sp_end_tick) + ".";
    }
    if (!scene.activations.empty() && fill.chord_tick < scene.activations.back().tick)
        return kBeforeLastActivationText;
    if (fill.state == app::PreviewFillState::Offered) return {};
    const int bars = bars_saved_at(scene, fill);
    if (bars < kSpActivationBars)
        return "You have " + counted(bars, "phrase", "phrases") + " saved here. You need " +
               std::to_string(kSpActivationBars) + " to activate.";
    return kGameHidesFillText;
}

std::string squeeze_end_text(const Activation& act) {
    std::string out;
    for (const ActivationTiming& t : act.badge_timings()) {
        if (t.part == TimingPart::EarlyFill) continue;
        out += out.empty() ? ", ending with " : " and ";
        out += t.part == TimingPart::SqueezeOut ? "a squeeze-out" : "a squeeze-in";
        out += t.required ? " that needs " + format_ms(t.ms) : " that needs no timing";
    }
    return out;
}

std::string cost_text(int64_t cost) {
    if (cost <= 0) return "costs nothing";
    return "costs " + group_thousands(cost) + " points";
}

std::string fill_hover_text(const PreviewScene& scene, size_t fill_index,
                            const std::optional<FillHoverNumbers>& numbers) {
    const PreviewFill& fill = scene.fills.at(fill_index);
    std::string t = "Fill at " + fill_name(scene, fill) + " · " +
                    app::clock_str(scene_timing(scene).timecode(fill.chord_tick).ms()) + "\n";
    if (fill.state == app::PreviewFillState::Taken) return t + kYourActivationHoverText;
    const std::string reason = fill_reason(scene, fill_index);
    if (!reason.empty()) return t + reason;
    t += "You have " + counted(bars_saved_at(scene, fill), "phrase", "phrases") + " saved.\n";
    if (!numbers) return t + kPendingText;
    t += "If you activate here, the best you can still finish with is " +
         group_thousands(numbers->best_finish) + ".";
    if (numbers->cost <= 0) {
        t += std::string("\n") + kBestChoiceNowText;
    } else {
        t += "\nYour best choice right now is " + numbers->best_fill + ", which keeps " +
             group_thousands(numbers->best_now) + ".\nSo activating here " +
             cost_text(numbers->cost) + ".";
    }
    if (numbers->hydra_end) {
        if (const std::optional<int64_t> d = numbers->hydra_end->deact_tick())
            t += "\nHydra would end it at " + measure_at(scene_timing(scene), *d) +
                 squeeze_end_text(*numbers->hydra_end) + ".";
    }
    return t;
}

GuideMessage cant_activate_message(const std::string& fill, const std::string& reason) {
    return {"Can't activate at " + fill + ": " + reason, true};
}

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
    s.why = cost > 0 ? "Before it, the best you could still finish with was " +
                           group_thousands(before) + ". After it, " + group_thousands(after) +
                           "."
                     : "The best you can still finish with stays at " + group_thousands(after) +
                           ".";
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
