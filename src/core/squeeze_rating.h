// Squeeze rating — where an activation's Star Power ends, how frontend
// timing error carries to that end, and how the resulting squeezes are
// judged for display.
//
// One home for all of it: the transfer scales, which scale directions
// matter, which multiplier governs a note, and what its margin is worth on
// the nominal two-hit scale. The search never reads
// the judgement side; difficulty and the ms filter stay raw gap ms (see
// core/model.h).
//
// The deactivation node itself is not worked out here. The search stamps it
// onto every activation it produces, and everything below reads that field.
//
// Three functions are the module's entries:
//   rate_activation()          -- the details display's only interface.
//   activation_deact_tick()    -- the Preview's, for the active SP window.
//   frontend_transfer_scales() -- the engine's copy-out stamp, read from the
//                                 stored SP-end steps, so a record's stored
//                                 ratios and a live recompute agree.
// Everything else below is an internal piece, declared only so its own tests
// can reach it directly. No production caller should use them.

#ifndef HYDRA_CORE_SQUEEZE_RATING_H
#define HYDRA_CORE_SQUEEZE_RATING_H

#include <cmath>
#include <optional>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/timing.h"

namespace hydra {

// ---- transfer scales ------------------------------------------------------

// An activation's transfer scales, all from stored ticks: `post` at the
// deact node, and one per SqIn (in sqinouts order) at the end that SqIn's
// offset was measured from. Nothing is stepped back from D.
struct ActTransferScales {
    TransferScale post;
    std::vector<TransferScale> sqins;
};

// The transfer scale between two ticks: mspm(end)/mspm(anchor), probed at
// the tick (late direction) and tick-1 (early direction). nullopt when any
// of the four measure lengths is not a positive finite number.
std::optional<TransferScale> transfer_scale_between(int64_t anchor_tick,
                                                    int64_t end_tick,
                                                    const SongTiming& timing);

// The activation's deactivation node D, in ticks: where its Star Power runs
// out, phrases collected mid-SP included. This does not work anything out.
// The search knew D and wrote it onto the record, so this hands back
// act.deact_tick and nothing else. nullopt means the record was written
// before blob v4 and simply does not say.
std::optional<int64_t> activation_deact_tick(const Activation& act);

// The activation's transfer scales, from the stored SP-end steps only:
// `post` from deact_anchor_tick() to deact_tick(), each SqIn from
// squeeze_anchor_tick(k) to squeeze_end_tick(k). The engine stamps the
// stored scales through this function at copy-out. nullopt when a tick is
// missing or a measure length is not a positive finite number.
std::optional<ActTransferScales> frontend_transfer_scales(const Activation& act,
                                                          const SongTiming& timing);

// ---- rating pieces --------------------------------------------------------

// A backend squeeze's raw ms mapped onto the nominal 2*W scale the ratings
// assume. With frontend timing scaling by r at the SP end, the real combined
// squeeze budget is squeeze_budget_ms(r, W) = W*(1+r) rather than 2*W, so a
// raw |offset| counts for |offset| * 2 / (1+r) of the nominal budget (a
// W-free quantity).
double effective_backend_ms(double offset_ms, double transfer_r);
double squeeze_budget_ms(double transfer_r, double hit_window_ms = kDefaultHitWindowMs);

// A multiplier is x1.00 only when it equals 1 up to the round-off of the
// measure-length division (two equal measures reached through different
// tempos). This is not a display cutoff: x0.999 is scaled. The user chose
// 1e-9 (docs/audit/2026-10-03-fix-decisions.md, D14): no test-set multiplier
// is that close to 1.
constexpr double kScaleIdentityTolerance = 1e-9;
inline bool is_scaled(double r) { return std::abs(r - 1.0) > kScaleIdentityTolerance; }

// The most decimals the scale line needs so a scaled multiplier never prints
// as 1: rounding to d decimals moves a value by at most half a unit,
// 0.5 * 10^-d, so d must bring that within the tolerance. Derived from the
// tolerance, so lowering it raises this (1e-9 gives 9).
constexpr int kScaleIdentityDigits = [] {
    int digits = 0;
    double half_unit = 0.5;
    while (half_unit > kScaleIdentityTolerance) {
        half_unit /= 10.0;
        ++digits;
    }
    return digits;
}();

// The one rule for a single note near a Star Power end. The side of the end
// it sits on decides which activation-hit direction moves the end across it:
// a note inside SP is crossed by an early hit pulling the end back, a note
// outside by a late hit pushing the end past it. That holds whether the
// crossing loses the note (a counted row, a free squeeze) or wins it (a
// squeeze still to earn), so the squeeze kind never enters.
struct NoteRating {
    bool early = false;                  // which side of the end's TransferScale governs
    double scale = 1.0;                  // that side's stored multiplier, full precision
    double budget_ms = 0.0;              // squeeze_budget_ms(scale, W)
    std::optional<double> effective_ms;  // set exactly when is_scaled(scale)
};

// offset_ms: the note's ms minus the SP end's ms (negative = before it).
// inside: whether the note is inside SP on this path, by the counting rule
// that owns it (core/backend_value.h). at_end: the multipliers stored for
// the SP end that offset is measured from.
NoteRating rate_note(double offset_ms, bool inside, const TransferScale& at_end,
                     double hit_window_ms = kDefaultHitWindowMs);

// ---- rate_activation ------------------------------------------------------

// One backend table row, resolved: the display row, whether it is the
// squeezed-out note, and its rating at the deact-node end. A row with no
// offset keeps the default rating (x1.00, no figure).
struct BackendRating {
    BackendSqueeze row;
    bool squeezed_out = false;
    NoteRating note;
};

// Whether the frontend decides this row: it was squeezed out, or the engine
// does not count it without a squeeze (at or past the leeway). The overfill
// warning fires only for an activation that has such a row or a SqIn/SqOut.
bool is_frontend_decided(const BackendRating& row, double backend_leeway_ms);

// The full transfer-scale story for one activation, as the details display
// tells it.
struct ActivationRating {
    // The scales the search stored on the activation (transfer_post and
    // each SqIn's transfer).
    ActTransferScales scales;
    // True when a multiplier that isn't 1 governs at least one row or SqIn
    // on this activation: the scale line turns orange.
    bool scale_governs = false;
    // True when the SP cap clamped this activation's window AND the frontend
    // decides at least one of its squeezes: any SqIn/SqOut, or any row
    // is_frontend_decided accepts. Drives the overfill warning.
    bool cap_clamped = false;
    // One entry per act.display_backends() row, in that order.
    std::vector<BackendRating> backends;
    // One entry per act.sqinouts, in that order: a SqIn's figure, rated at
    // its own stored scale. Always empty for a SqOut, whose note is its
    // squeezed-out backend row and is rated there.
    std::vector<std::optional<double>> note_effective_ms;
};

// The display's whole view of an activation's squeezes: it builds the backend
// rows itself (act.display_backends()), so the caller renders and nothing
// more. It reads the scales the search stored on the activation. Backend
// rows are judged at the post (deact-node) end, each SqIn phrase note at
// its own end's scale.
// backend_leeway_ms: Rules::backend_leeway_ms. A plain row less than this
// past the SP end is counted by the engine, so it is not a late squeeze.
ActivationRating rate_activation(
    const Activation& act,
    double hit_window_ms = kDefaultHitWindowMs,
    double backend_leeway_ms = core::default_rules().backend_leeway_ms);

// ---- timing tiers ---------------------------------------------------------

// The report's timing tiers: raw squeeze ms banded against the two-hit
// budget 2*W, quarters of the budget after the kDifficultMs "Normal" floor
// (at the historical W = 70 this is the 2/35/70/105/140 ladder). In payload
// order; `cutoff` is the band's exclusive upper edge, unset for the open
// "Beyond" band and the "None" (no squeeze) entry.
struct TimingTier { const char* name; const char* tok; std::optional<double> cutoff; };
std::vector<TimingTier> timing_tiers(double hit_window_ms = kDefaultHitWindowMs);

// Where the report's "Beyond" tier starts: the last finite cutoff of
// timing_tiers. The footer and the page script both print this number.
double beyond_edge_ms(double hit_window_ms);

}  // namespace hydra

#endif  // HYDRA_CORE_SQUEEZE_RATING_H
