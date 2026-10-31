// Every sentence the Path Builder's redesigned screen says, as plain
// functions over plain values. The wording is the approved mockups'
// (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html,
// then docs/superpowers/specs/2026-10-31-path-builder-mockup-2/
// mockup_template.html with answers.md beside it); every figure in it comes
// from its owner, and this file works out no game fact of its own. Plans:
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md, task CO-1, and
// docs/superpowers/plans/2026-10-31-path-builder-redesign-2.md, task SC-3a.

#ifndef HYDRA_PATHBUILDER_PB_TEXT_H
#define HYDRA_PATHBUILDER_PB_TEXT_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/path_costs.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "core/timing.h"

namespace hydra::pathbuilder {

using app::PreviewFill;
using app::PreviewScene;

// The intro under the title; its phrase count is kSpActivationBars.
std::string intro_text();

// Where the Star Power star goes in a sentence the screen draws one in: a
// byte offset into the sentence's text, just after the word "phrases" the
// star stands beside. The text holds no star, so a test reads the sentence
// whole; the screen draws kStarOpen, the star (ui::draw_star) and kStarClose
// at the offset, as the mockup writes "phrases (★)".
inline constexpr const char* kStarOpen = " (";
inline constexpr const char* kStarClose = ")";

// The star's place in intro_text.
size_t intro_star_at();

// The sentences that take no figure, each from the mockup.
extern const char* const kNoStepsText;
extern const char* const kActivationRemovedText;
extern const char* const kStartedOverText;
extern const char* const kBuiltWholePathText;
extern const char* const kYourActivationHoverText;
// What a fill before the player's last activation says on hover, and the
// tail it shares with locked_message: answers.md answer 2's words.
extern const char* const kBuildLeftToRightText;
extern const std::string kBeforeLastActivationText;
extern const char* const kGameHidesFillText;
extern const char* const kBestChoiceNowText;
extern const char* const kNothingLeftText;
extern const char* const kPendingText;  // a figure whose search has not landed

// A fill's name on screen: its measure, "m96" (measure_label).
std::string fill_name(const PreviewScene& scene, const PreviewFill& fill);

// The bars of Star Power saved when `fill` ends, as the Preview's meter reads
// them (sp_meter_bars_at), in whole bars.
int bars_saved_at(const PreviewScene& scene, const PreviewFill& fill);

// Which kind of refusal a fill gets, or none. BeforeLast is the builder's
// own left-to-right rule (the locked look); SpRunning, UnderBars and
// GameHides are the game's; Taken is the player's own activation.
enum class FillRefusal { None, Taken, SpRunning, BeforeLast, UnderBars, GameHides };

// Why the player cannot activate at scene.fills[fill_index]: the kind in
// `kind`, and the sentence for it as the return value. The sentence is empty
// for None (an Offered fill after their last window) and for Taken (the fill
// is their own activation). `scene` is the player's path's scene: its
// activations are the player's windows. The one owner of the order the facts
// are read in, the order FillRefusal lists them: the fill is Taken; a
// window's Star Power is running at the fill's chord; the fill's chord is
// before the last activation (the screen builds left to right, the user's
// answer Q1); the fill is Offered (None); the meter at the fill is under
// kSpActivationBars; otherwise the engine did not offer it.
std::string fill_reason(const PreviewScene& scene, size_t fill_index, FillRefusal& kind);
// The sentence alone.
std::string fill_reason(const PreviewScene& scene, size_t fill_index);
// The kind alone.
FillRefusal fill_refusal(const PreviewScene& scene, size_t fill_index);

// ", ending with a squeeze-out that needs 12.3 ms and a squeeze-in that needs
// no timing", one clause per squeeze in Activation::badge_timings, each
// figure through format_ms. Empty when the activation has no squeeze.
std::string squeeze_end_text(const Activation& act);

// The "Where to activate next" list's words, from the mockup's
// renderChoices: its heading, the hint under it, and what it says when the
// engine lists no fill (after the player's last activation, or with none).
extern const char* const kChoicesHeading;
extern const char* const kChoicesHint;
extern const char* const kNoChoicesFinishedText;
extern const char* const kNoChoicesText;

// The middle of a list row for scene.fills[fill_index], its parts joined by
// app::kFactSeparator: the fill's clock (clock_str), the phrases saved there
// (bars_saved_at; the engine already held the meter at its cap, so no cap is
// applied here), and "ends m36" from `end`,
// the candidate's activation (DecisionList::candidate_end) with its
// squeezes in short form ("squeeze-out 12.3 ms", "squeeze-in, no timing").
// A null `end` (the candidate's search has not landed) reads kPendingText in
// place of the end; an `end` with no deact node leaves the end out.
std::string candidate_row_text(const PreviewScene& scene, size_t fill_index,
                               const Activation* end);

// The list's rows in order, as chord ticks: `offered` is every fill the
// engine lists (PreviewScene::trailing_skipped_fill_ticks), in chart order,
// and `priced` the candidates whose cost has landed (DecisionList::
// candidates()). Priced fills come first, cheapest first, ties in chart
// order; the fills still being priced follow, in chart order. A priced
// candidate whose tick is not in `offered` is left out.
std::vector<int64_t> choice_order(const std::vector<int64_t>& offered,
                                  const std::vector<app::CandidateCost>& priced);

// A compact step's two pieces: `span` is where the activation starts and
// where its Star Power ends, "m32 → m36" (measure_label of the activation
// and of its deact node; the start alone when it has none), and `saved` the
// phrases it spends, "2 phrases" (Activation::sp_meter, as step_sentence).
struct StepShort {
    std::string span;
    std::string saved;
};
StepShort step_short(const SongTiming& timing, const Activation& act);

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
// a refusal (drawn red-edged). star_at is the star's place in the text (as
// intro_star_at), set only on guide_text's "Save 2 more phrases" sentence.
struct GuideMessage {
    std::string text;
    bool refuse = false;
    std::optional<size_t> star_at;
};

// "Can't activate at m12: <reason>", a refusal, for every kind but
// BeforeLast.
GuideMessage cant_activate_message(const std::string& fill, const std::string& reason);
// A click on a fill refused as BeforeLast: "m12 is before your last
// activation." and kBuildLeftToRightText, a refusal.
GuideMessage locked_message(const std::string& fill);
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
