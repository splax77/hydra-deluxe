#include "core/squeeze_rating.h"

#include "core/backend_value.h"

#include <algorithm>
#include <cmath>

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

std::optional<int64_t> activation_deact_tick(const Activation& act) {
    return act.deact_tick();
}

std::optional<ActTransferScales> frontend_transfer_scales(const Activation& act,
                                                          const SongTiming& timing) {
    // D and the note whose timing moves it, from the stored steps.
    const std::optional<int64_t> d = act.deact_tick();
    const std::optional<int64_t> d_anchor = act.deact_anchor_tick();
    if (!d || !d_anchor) return std::nullopt;
    const std::optional<TransferScale> post = transfer_scale_between(*d_anchor, *d, timing);
    if (!post) return std::nullopt;
    ActTransferScales out;
    out.post = *post;
    // Each SqIn at the end its own offset was measured from.
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        if (act.sqinouts[k].kind != SqueezeKind::SqIn) continue;
        const std::optional<int64_t> end = act.squeeze_end_tick(k);
        const std::optional<int64_t> anchor = act.squeeze_anchor_tick(k);
        if (!end || !anchor) return std::nullopt;
        const std::optional<TransferScale> s = transfer_scale_between(*anchor, *end, timing);
        if (!s) return std::nullopt;
        out.sqins.push_back(*s);
    }
    return out;
}

double effective_backend_ms(double offset_ms, double transfer_r) {
    return std::abs(offset_ms) * 2.0 / (1.0 + transfer_r);
}

double squeeze_budget_ms(double transfer_r, double hit_window_ms) {
    return hit_window_ms * (1.0 + transfer_r);
}

NoteRating rate_note(double offset_ms, bool inside, const TransferScale& at_end,
                     double hit_window_ms) {
    NoteRating n;
    n.early = inside;
    n.scale = inside ? at_end.early : at_end.late;
    n.budget_ms = squeeze_budget_ms(n.scale, hit_window_ms);
    if (is_scaled(n.scale)) n.effective_ms = effective_backend_ms(offset_ms, n.scale);
    return n;
}

bool is_frontend_decided(const BackendRating& row, double backend_leeway_ms) {
    return row.squeezed_out ||
           (row.row.offset_ms &&
            !core::counted_without_squeeze(*row.row.offset_ms, backend_leeway_ms));
}

ActivationRating rate_activation(const Activation& act,
                                 double hit_window_ms,
                                 double backend_leeway_ms) {
    ActivationRating out;

    // The scales the search stamped on the record. A stored fact is read,
    // never re-derived (ADRs 0011, 0013, 0014); every Ready record has them.
    out.scales.post = act.transfer_post;
    for (const SPSqueeze& sq : act.sqinouts)
        if (sq.kind == SqueezeKind::SqIn) out.scales.sqins.push_back(sq.transfer);

    // Backend rows: every offset is measured from the deact node D, so they
    // read `post`. A squeezed-out row is about its phrase, which the SP end
    // itself decides; every other row is about its points, which the engine
    // counts up to the leeway.
    std::vector<BackendSqueeze> backends = act.display_backends();
    out.backends.reserve(backends.size());
    for (const BackendSqueeze& bsq : backends) {
        BackendRating row;
        row.row = bsq;
        row.squeezed_out = act.is_sqout_backend(bsq);
        if (bsq.offset_ms) {
            const double o = *bsq.offset_ms;
            const bool inside = row.squeezed_out
                                    ? core::paid_by_sp_walk(o)
                                    : core::counted_without_squeeze(o, backend_leeway_ms);
            row.note = rate_note(o, inside, out.scales.post, hit_window_ms);
            out.scale_governs |= row.note.effective_ms.has_value();
        }
        out.backends.push_back(std::move(row));
    }

    // SqIn phrase notes: each offset is measured from the end before its
    // phrase extended SP, so each reads its own stored scale. A SqOut is not
    // rated here: its offset is its squeezed-out row's offset (both are the
    // deact edge's sqinout_timing), and that row was rated above, at the end
    // it is measured from.
    out.note_effective_ms.reserve(act.sqinouts.size());
    size_t j = 0;
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind == SqueezeKind::SqOut) {
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        // A free SqIn's note is inside SP (SPSqueeze::is_free, D13).
        const NoteRating n =
            rate_note(sq.offset_ms, sq.is_free(), out.scales.sqins[j++], hit_window_ms);
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

std::vector<TimingTier> timing_tiers(double hit_window_ms) {
    const double w = hit_window_ms;
    return {
        {"Normal", "t0", kDifficultMs}, {"Hard", "t1", w / 2},
        {"Extreme", "t2", w},           {"Insane", "t3", 3 * w / 2},
        {"Insane+", "t4", 2 * w},       {"Beyond", "t5", std::nullopt},
        {"None", "tn", std::nullopt},
    };
}

double beyond_edge_ms(double hit_window_ms) {
    double edge = 0.0;
    for (const TimingTier& t : timing_tiers(hit_window_ms))
        if (t.cutoff) edge = std::max(edge, *t.cutoff);
    return edge;
}

}  // namespace hydra
