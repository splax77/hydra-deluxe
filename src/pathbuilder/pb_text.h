// Every sentence the Path Builder's redesigned screen says, as plain
// functions over plain values. The wording is the approved mockup's
// (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html);
// every figure in it comes from its owner, and this file works out no game
// fact of its own. Plan: docs/superpowers/plans/
// 2026-10-30-path-builder-redesign.md, task CO-1.

#ifndef HYDRA_PATHBUILDER_PB_TEXT_H
#define HYDRA_PATHBUILDER_PB_TEXT_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "app/preview_view.h"
#include "core/model.h"
#include "core/timing.h"

namespace hydra::pathbuilder {

using app::PreviewFill;
using app::PreviewScene;

// The intro under the title; its phrase count is kSpActivationBars.
std::string intro_text();

// The sentences that take no figure, each from the mockup.
extern const char* const kNoStepsText;
extern const char* const kActivationRemovedText;
extern const char* const kStartedOverText;
extern const char* const kBuiltWholePathText;
extern const char* const kYourActivationHoverText;
extern const char* const kBeforeLastActivationText;
extern const char* const kGameHidesFillText;
extern const char* const kBestChoiceNowText;
extern const char* const kNothingLeftText;
extern const char* const kPendingText;  // a figure whose search has not landed

// A fill's name on screen: its measure, "m96" (measure_label).
std::string fill_name(const PreviewScene& scene, const PreviewFill& fill);

// The bars of Star Power saved when `fill` ends, as the Preview's meter reads
// them (sp_meter_bars_at), in whole bars.
int bars_saved_at(const PreviewScene& scene, const PreviewFill& fill);

// Why the player cannot activate at scene.fills[fill_index], or empty when
// they can (an Offered fill after their last window) or when the fill is
// their own activation. `scene` is the player's path's scene: its activations
// are the player's windows. The facts it reads, in order: the fill is Taken;
// a window's Star Power is running at the fill's end (sp_running_at); the
// fill's chord is before the last activation (the screen builds left to
// right, the user's answer Q1); the fill is Offered; the meter at the fill
// is under kSpActivationBars; otherwise the engine did not offer it.
std::string fill_reason(const PreviewScene& scene, size_t fill_index);

// ", ending with a squeeze-out that needs 12.3 ms and a squeeze-in that needs
// no timing", one clause per squeeze in Activation::badge_timings, each
// figure through format_ms. Empty when the activation has no squeeze.
std::string squeeze_end_text(const Activation& act);

// "costs nothing" or "costs 1,234 points" (group_thousands).
std::string cost_text(int64_t cost);

// The candidate's numbers the hover explains, once its search has landed:
// best_finish is BF(n plus this fill), best_now is BF(n), cost is
// candidate_costs's difference, best_fill is the name of the fill that keeps
// BF(n), and hydra_end is the candidate's activation in BF(n plus this fill)'s
// path (null when the caller has none, which leaves out the end line).
struct FillHoverNumbers {
    int64_t best_finish = 0;
    int64_t best_now = 0;
    int64_t cost = 0;
    std::string best_fill;
    const Activation* hydra_end = nullptr;
};

// The hover over scene.fills[fill_index]: its name and clock (clock_str),
// then its fill_reason, "Your activation" for a Taken fill, or for a fill
// the player can take the bars saved and the two numbers its cost compares.
// `numbers` empty means the candidate's search has not landed ("...").
// Lines are separated by '\n'.
std::string fill_hover_text(const PreviewScene& scene, size_t fill_index,
                            const std::optional<FillHoverNumbers>& numbers);

// A one-shot message the guide line shows after an action, and whether it is
// a refusal (drawn red-edged).
struct GuideMessage {
    std::string text;
    bool refuse = false;
};

// "Can't activate at m12: <reason>", a refusal.
GuideMessage cant_activate_message(const std::string& fill, const std::string& reason);
// After Remove: "Activation removed." for the last window (index is 0-based
// and `last` says it was the last), else "Removed activation N and
// everything after it."
GuideMessage removed_message(size_t index, bool last);
GuideMessage started_over_message();
// After "Finish the rest for me", with `kept` the windows the player had.
GuideMessage finished_for_player_message(size_t kept);

// What the guide line needs to say the next step.
struct GuideFacts {
    // The earliest fill the player can activate on: its name and its
    // chord's time in ms. Empty when there is none.
    std::optional<std::string> earliest_fill;
    double earliest_ms = 0.0;
    // The fill that keeps BF(n), once its search has landed.
    std::optional<std::string> best_fill;
    // The last window's end measure (measure_label of its deact node);
    // empty while the player has no window.
    std::optional<std::string> last_end;
    // How many fills the engine lists after the last window
    // (PreviewScene::trailing_skipped_fill_ticks); none means finished.
    size_t fills_after_last = 0;
    int64_t player_total = 0;
    // The one-shot message, when an action just left one.
    std::optional<GuideMessage> message;
};

GuideMessage guide_text(const GuideFacts& facts);

// One step in the steps panel: the sentence, the line under it with the two
// numbers its cost compares, and the violet note (tight_window_note; empty
// when the window is inside the Path limit).
struct StepText {
    std::string sentence;
    std::string why;
    std::string note;
};

// The step for activation `act`: where it starts (measure_label, clock_str),
// the bars it spends (Activation::sp_meter), where its Star Power ends (its
// deact node), its squeezes (squeeze_end_text), and its cost, with `before`
// and `after` the best finish before and after it (the ladder's bf[i] and
// bf[i + 1]) and `cost` cost_list's figure for it.
StepText step_sentence(const SongTiming& timing, const Activation& act, int64_t cost,
                       int64_t before, int64_t after,
                       const std::optional<double>& limit_ms);

// The line after the steps: cost_list's after_last, what the best rest of
// the path would add.
std::string leftover_text(int64_t after_last);

// The glossary under the steps.
std::string glossary_text();

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_TEXT_H
