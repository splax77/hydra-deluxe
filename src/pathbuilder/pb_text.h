// Every sentence the Path Builder's redesigned screen says, as plain
// functions over plain values. The wording is the approved mockups'
// (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html (removed; git show 6804353f:docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html),
// then docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html (removed; git show 6804353f:docs/superpowers/specs/2026-10-31-path-builder-mockup-2/mockup_template.html)
// with answers.md beside it); every figure in it comes
// from its owner, and this file works out no game fact of its own. Plan:
// docs/superpowers/plans/2026-10-30-path-builder-redesign.md (removed; git show 307d0b5c:docs/superpowers/plans/2026-10-30-path-builder-redesign.md), task CO-1.
// The user's answers on the second mockup are D303.
// The fill card follows the third mockup
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-3/, removed; git show 6804353f:docs/superpowers/specs/2026-10-31-path-builder-mockup-3/), D303.
// The step rows and ending lines follow the fourth mockup
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-4/, removed; git show 6804353f:docs/superpowers/specs/2026-10-31-path-builder-mockup-4/), D302.
// The top of the screen follows the fifth mockup
// (docs/superpowers/specs/2026-10-31-path-builder-mockup-5/top-mockup.html (removed; git show 6804353f:docs/superpowers/specs/2026-10-31-path-builder-mockup-5/top-mockup.html)),
// D304: the head row, then the timeline.

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

// The sentences that take no figure, each from the mockup.
extern const char* const kNoStepsText;
extern const char* const kYourActivationHoverText;
// What a fill before the player's last activation says on hover: answers.md
// answer 2's words.
extern const char* const kBeforeLastActivationText;
extern const char* const kGameHidesFillText;
extern const char* const kBestChoiceNowText;
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

// Every squeeze and early fill the builder names is worded by the Paths tab's
// badges (app::activation_badges); pb_text.cpp's badges_text is the one place
// that joins them.

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
// timings as end_choice_text words them.
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
// and of its deact node; the start alone when it has none), `saved` the
// phrases it spends, "2 phrases" (Activation::sp_meter),
// and `squeezes` its timings as the Paths tab's badges word them (badges_text;
// empty when it has none).
// The screen joins `squeezes` to `saved` with app::kFactSeparator only when
// it is not empty (the fourth mockup's step row).
struct StepShort {
    std::string span;
    std::string saved;
    std::string squeezes;
};
StepShort step_short(const SongTiming& timing, const Activation& act);

// One ending line of an opened step, "ends m89 · squeeze out 0.0 ms": where
// `act`'s Star Power ends (measure_label of its deact node; its
// Activation::notationstr when it has none), then its timings (badges_text),
// joined by app::kFactSeparator, when it has any. The
// list row's end (candidate_row_text) reads it too.
std::string end_choice_text(const SongTiming& timing, const Activation& act);

// The word under the best next fills on the timeline, and the cost of a list
// row, a step, an end choice or a fill card that loses nothing.
inline constexpr const char* kBestFillLabel = "best";

// The cost of a list row, a step, an end choice or a fill card: kBestFillLabel
// when it loses nothing (costs_nothing), else the cost in full (format_cost).
std::string cost_or_best(int64_t cost);

// The candidate's numbers the hover explains, once its search has landed:
// best_finish is BF(n plus this fill), best_now is BF(n), cost is
// candidate_costs's difference and tier its colour (CandidateCost::tier),
// best_fill is the name of the fill that keeps BF(n), and hydra_end is the
// candidate's activation in BF(n plus this fill)'s path (null when the caller
// has none, which leaves out the end line).
struct FillHoverNumbers {
    int64_t best_finish = 0;
    int64_t best_now = 0;
    int64_t cost = 0;
    app::CostTier tier = app::CostTier::Green;
    std::string best_fill;
    const Activation* hydra_end = nullptr;
};

// The labels of a fill card's rows, the mockup's fillTip words.
inline constexpr const char* kPhrasesSavedLabel = "Phrases saved";
inline constexpr const char* kBestFinishHereLabel = "Best finish here";
inline constexpr const char* kCostLabel = "Cost";

// One row of a fill card: a label and its value.
struct CardRow {
    std::string label;
    std::string value;
};

// A fill's hover as a short card (the third mockup's fillTip, answers.md
// answer 3): a heading, labelled rows, and grey lines under them. cost_tier
// is the Cost row's colour, set only once the cost has landed.
struct FillHoverCard {
    std::string heading;
    std::vector<CardRow> rows;
    std::optional<app::CostTier> cost_tier;
    std::vector<std::string> grey;
};

// The card for scene.fills[fill_index]. The heading is the fill's name and
// its clock (clock_str), joined by app::kFactSeparator. A Taken fill adds one
// grey line, kYourActivationHoverText; a fill with a fill_reason adds that
// sentence alone. A fill the player can take gets three rows: the phrases
// saved (bars_saved_at), the best finish here and the cost (cost_or_best),
// then a grey line naming the best choice (kBestChoiceNowText when this fill
// costs nothing) and, when `numbers` has a hydra_end with a deact node, where
// Hydra would end it (its timings by badges_text). `numbers` empty means the
// candidate's search has not landed: the last two rows read kPendingText and
// no grey line follows.
FillHoverCard fill_hover_card(const PreviewScene& scene, size_t fill_index,
                              const std::optional<FillHoverNumbers>& numbers);

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_TEXT_H
