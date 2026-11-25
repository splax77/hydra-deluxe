#include "core/squeeze_rating.h"

#include "core/backend_value.h"
#include "core/note_window.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <tuple>

namespace hydra {

std::optional<TransferScale> transfer_scale_between(int64_t anchor_tick,
                                                    int64_t end_tick,
                                                    const SongTiming& timing) {
    const double front_late = timing.ms_per_measure_at(anchor_tick);
    const double front_early = timing.ms_per_measure_at(anchor_tick - 1);
    const double end_late = timing.ms_per_measure_at(end_tick);
    const double end_early = timing.ms_per_measure_at(end_tick - 1);
    for (double m : {front_late, front_early, end_late, end_early})
        if (!std::isfinite(m) || m <= 0.0) return std::nullopt;
    return TransferScale{end_early / front_early, end_late / front_late};
}

std::optional<TransferScale> step_transfer_scale(const Activation& act,
                                                 size_t step_index,
                                                 const SongTiming& timing) {
    if (step_index >= act.sp_end_steps.size()) return std::nullopt;
    return transfer_scale_between(act.end_anchor_tick(step_index),
                                  act.sp_end_steps[step_index].end_tick, timing);
}

std::optional<ActTransferScales> frontend_transfer_scales(const Activation& act,
                                                          const SongTiming& timing) {
    // D is the last step's end.
    if (act.sp_end_steps.empty()) return std::nullopt;
    const std::optional<TransferScale> post =
        step_transfer_scale(act, act.sp_end_steps.size() - 1, timing);
    if (!post) return std::nullopt;
    ActTransferScales out;
    out.post = *post;
    // Each SqIn at the step its own offset was measured from.
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        if (act.sqinouts[k].kind != SqueezeKind::SqIn) continue;
        const std::optional<size_t> step = act.squeeze_end_step(k);
        if (!step) return std::nullopt;
        const std::optional<TransferScale> s = step_transfer_scale(act, *step, timing);
        if (!s) return std::nullopt;
        out.sqins.push_back(*s);
    }
    return out;
}

std::optional<ActTransferScales> stored_transfer_scales(const Activation& act) {
    if (!act.transfer_post) return std::nullopt;
    ActTransferScales s;
    s.post = *act.transfer_post;
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        if (act.sqinouts[k].kind != SqueezeKind::SqIn) continue;
        const std::optional<TransferScale>& own = stored_scale(act, k);
        if (!own) return std::nullopt;
        s.sqins.push_back(*own);
    }
    return s;
}

// ---- the multiplier a squeeze or a row reads -------------------------------

bool PricedScale::shown() const { return is_scaled(ratio); }

const std::optional<TransferScale>& stored_scale(const Activation& act, size_t k) {
    static const std::optional<TransferScale> none;
    if (k >= act.sqinouts.size()) return none;
    const SPSqueeze& q = act.sqinouts[k];
    return q.kind == SqueezeKind::SqOut ? act.transfer_post : q.transfer;
}

namespace {

// `scale` priced on the side `press` names (transfer_side owns that number).
PricedScale priced_on(const TransferScale& scale, Press press) {
    return PricedScale{scale, press, transfer_side(scale, press)};
}

}  // namespace

std::optional<PricedScale> squeeze_multiplier(const Activation& act, size_t k) {
    const std::optional<TransferScale>& scale = stored_scale(act, k);
    if (!scale) return std::nullopt;
    return priced_on(*scale, squeeze_presses(act.sqinouts[k]).anchor);
}

std::optional<PricedScale> row_multiplier(const Activation& act, const BackendSqueeze& row,
                                          double leeway_ms) {
    if (!row.offset_ms) return std::nullopt;
    if (act.is_sqout_backend(row)) {
        // The squeezed-out row is its squeeze out's note: one route, so the
        // row and its badge read one side.
        for (size_t k = 0; k < act.sqinouts.size(); ++k)
            if (act.sqinouts[k].kind == SqueezeKind::SqOut) return squeeze_multiplier(act, k);
        return std::nullopt;
    }
    if (!act.transfer_post) return std::nullopt;
    const Press press =
        core::counted_without_squeeze(row.offset(), leeway_ms) ? Press::Early : Press::Late;
    return priced_on(*act.transfer_post, press);
}

namespace {

// Every tick past 0 where the tempo or the count's ticks per measure takes a
// new value, sorted. A marker that repeats the value in force changes
// nothing, so it is left out; the opening entries at tick 0 start the song
// and change nothing either.
std::vector<int64_t> measure_length_changes(const SongTiming& timing) {
    std::vector<int64_t> out;
    const MsIndex& ms = timing.ms_index();
    for (const int64_t k : ms.change_ticks())
        if (k > 0 && ms.bpm_at(k) != ms.bpm_at(k - 1)) out.push_back(k);
    const MeasureCount& count = timing.measure_count();
    for (int i = 1; i < count.count(); ++i)
        if (count.tpm_at(i) != count.tpm_at(i - 1)) out.push_back(count.start_at(i));
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

// One tick's two sides: back to the last change before it, and on to the
// first change after it (see SteadySpans for why a change on the tick bounds
// neither).
std::pair<std::optional<double>, std::optional<double>> spans_at(
    int64_t tick, const std::vector<int64_t>& changes, const MsIndex& ms) {
    const auto self = [](int64_t t) { return t; };
    const auto none = changes.end();
    std::optional<double> before;
    std::optional<double> after;
    // The early side is in force at tick - 1, the late side at the tick.
    const auto early = last_at_or_before(changes.begin(), none, tick - 1, self);
    if (early != none) before = ms.at(tick) - ms.at(*early);
    const auto late = last_at_or_before(changes.begin(), none, tick, self);
    const auto next = late == none ? changes.begin() : std::next(late);
    if (next != none) after = ms.at(*next) - ms.at(tick);
    return {before, after};
}

}  // namespace

SteadySpans steady_spans(int64_t anchor_tick, int64_t end_tick, const SongTiming& timing) {
    const std::vector<int64_t> changes = measure_length_changes(timing);
    SteadySpans out;
    std::tie(out.before_anchor_ms, out.after_anchor_ms) =
        spans_at(anchor_tick, changes, timing.ms_index());
    std::tie(out.before_end_ms, out.after_end_ms) = spans_at(end_tick, changes, timing.ms_index());
    return out;
}

namespace {

// The combined squeeze budget for a given window: the one formula behind
// squeeze_budget_ms, and the unit-window ratio in effective_backend_ms.
double budget_for_window(double transfer_r, double window_ms) {
    return window_ms * (1.0 + transfer_r);
}

}  // namespace

double effective_backend_ms(double offset_ms, double transfer_r) {
    // The gap rescaled by the identity budget over the budget at this scale.
    // The window cancels in that ratio, so a unit window is used: it keeps
    // the arithmetic bit-identical to the older written-out form.
    constexpr double kUnitWindow = 1.0;
    return std::abs(offset_ms) * budget_for_window(kIdentityScale, kUnitWindow) /
           budget_for_window(transfer_r, kUnitWindow);
}

double squeeze_budget_ms(double transfer_r) {
    return budget_for_window(transfer_r, kTimingStepMs);
}

double nominal_budget_ms() {
    return squeeze_budget_ms(kIdentityScale);
}

NoteRating rate_note(double offset_ms, const PricedScale& priced) {
    NoteRating n;
    n.early = priced.press == Press::Early;
    n.scale = priced.ratio;
    n.budget_ms = squeeze_budget_ms(n.scale);
    if (priced.shown()) n.effective_ms = effective_backend_ms(offset_ms, n.scale);
    return n;
}

namespace {

// Marks the side the owner priced.
void mark_read(SidesRead& read, const PricedScale& priced) {
    (priced.press == Press::Early ? read.early : read.late) = true;
}

}  // namespace

bool is_frontend_decided(const BackendRating& row, double backend_leeway_ms) {
    return row.squeezed_out ||
           (row.row.offset_ms &&
            !core::counted_without_squeeze(row.row.offset(), backend_leeway_ms));
}

ActivationRating rate_activation(const Activation& act, double backend_leeway_ms) {
    ActivationRating out;

    // The scales the search stamped on the record. A stored fact is read,
    // never re-derived (ADRs 0011, 0013, 0014). Unset when any one is
    // unknown: then nothing below gets a figure (D4).
    out.scales = stored_transfer_scales(act);

    // Backend rows: every offset is measured from the deact node D, so they
    // read `post`. Which side each reads is row_multiplier's answer.
    std::vector<BackendSqueeze> backends = act.display_backends();
    out.backends.reserve(backends.size());
    for (const BackendSqueeze& bsq : backends) {
        BackendRating row;
        row.row = bsq;
        row.squeezed_out = act.is_sqout_backend(bsq);
        // Unknown scales ask the owner nothing: the row keeps NoteRating's
        // defaults (x1.00, budget 0, no figure), which are not real values.
        if (out.scales) {
            if (const std::optional<PricedScale> priced =
                    row_multiplier(act, bsq, backend_leeway_ms)) {
                row.note = rate_note(bsq.offset(), *priced);
                mark_read(out.post_read, *priced);
                out.scale_governs |= row.note.effective_ms.has_value();
            }
        }
        out.backends.push_back(std::move(row));
    }

    // Each squeeze, priced by squeeze_multiplier. A SqIn's offset is
    // measured from the end before its phrase extended SP, so it reads its
    // own stored scale and gets its figure here. A SqOut gets no figure here:
    // its offset is copied from its squeezed-out row (Activation::set_sqout),
    // and that row was rated above. Its side still goes on post_read, so a
    // side the owner priced is never missing from the scale line. A SqIn's
    // side goes on sqins_read in this same walk, so that list pairs with
    // scales->sqins the way stored_transfer_scales built it: one entry per
    // SqIn, in list order.
    out.note_effective_ms.reserve(act.sqinouts.size());
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        // Unknown scales: no rating, no figure (D4).
        const std::optional<PricedScale> priced =
            out.scales ? squeeze_multiplier(act, k) : std::optional<PricedScale>();
        if (act.sqinouts[k].kind == SqueezeKind::SqOut) {
            if (priced) mark_read(out.post_read, *priced);
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        if (!priced) {
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        const NoteRating n = rate_note(act.sqinouts[k].offset_ms, *priced);
        mark_read(out.sqins_read.emplace_back(), *priced);
        out.scale_governs |= n.effective_ms.has_value();
        out.note_effective_ms.push_back(n.effective_ms);
    }

    // The cap-clamped flag fires when the activation has a clamp_tick() AND
    // at least one squeeze the frontend decides: any SqIn/SqOut, or any
    // backend row is_frontend_decided accepts.
    if (act.clamp_tick().has_value()) {
        bool decided = !act.sqinouts.empty();
        for (const BackendRating& br : out.backends)
            decided = decided || is_frontend_decided(br, backend_leeway_ms);
        out.cap_clamped = decided;
    }

    return out;
}

std::vector<TimingTier> timing_tiers() {
    const double w = kTimingStepMs;
    // Normal's edge is the difficult floor past_difficult_floor tests, and
    // it is inclusive: a timing exactly on it (2.0 ms) is Normal (D48 Q3).
    return {
        {"Normal", "t0", kDifficultMs}, {"Hard", "t1", w / 2},
        {"Extreme", "t2", w},           {"Insane", "t3", 3 * w / 2},
        {"Insane+", "t4", nominal_budget_ms()},
        {"Beyond", "t5", std::nullopt},
        {"None", "tn", std::nullopt},
    };
}

const TimingTier& beyond_tier(const std::vector<TimingTier>& tiers) {
    return tiers[tiers.size() - 2];
}

const TimingTier& none_tier(const std::vector<TimingTier>& tiers) { return tiers.back(); }

std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             const std::vector<TimingTier>& tiers) {
    const TimingTier& none = none_tier(tiers);
    const TimingTier& beyond = beyond_tier(tiers);
    if (!ms) return {none.name, none.tok};
    // Each edge belongs to the band below it (D48 Q3). The table's first row
    // is Normal up to and including the difficult floor, so a timing on the
    // floor reads Normal (past_difficult_floor's rule), and a timing exactly
    // on the two-hit budget is Insane+, not Beyond.
    for (const TimingTier& t : tiers)
        if (t.cutoff && *ms <= *t.cutoff) return {t.name, t.tok};
    return {beyond.name, beyond.tok};
}

double beyond_edge_ms() {
    double edge = 0.0;
    for (const TimingTier& t : timing_tiers())
        if (t.cutoff) edge = std::max(edge, *t.cutoff);
    return edge;
}

}  // namespace hydra
