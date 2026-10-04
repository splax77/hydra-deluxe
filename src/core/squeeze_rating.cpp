#include "core/squeeze_rating.h"

#include "core/backend_value.h"

#include <algorithm>
#include <cmath>

namespace hydra {

std::optional<TransferScale> transfer_scale_between(int64_t act_tick,
                                                    int64_t end_tick,
                                                    const SongTiming& timing) {
    TransferScale scale;
    double front_late = timing.ms_per_measure_at(act_tick);
    double front_early = timing.ms_per_measure_at(act_tick - 1);
    if (front_late <= 0.0 || front_early <= 0.0) return std::nullopt;
    scale.late = timing.ms_per_measure_at(end_tick) / front_late;
    scale.early = timing.ms_per_measure_at(end_tick - 1) / front_early;
    return scale;
}

std::optional<int64_t> activation_deact_tick(const Activation& act) {
    return act.deact_tick;
}

std::optional<ActTransferScales> frontend_transfer_scales(const Activation& act,
                                                          const SongTiming& timing) {
    if (!act.deact_tick) return std::nullopt;

    int64_t act_tick = act.timecode.ticks();
    bool has_sqin = false;
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind == SqueezeKind::SqIn) {
            has_sqin = true;
            break;
        }
    }

    // The SP end the search recorded, straight off the record.
    int64_t post_tick = *act.deact_tick;
    // The SqIn phrase is judged against the end as it stood before that
    // phrase extended SP: one 2-measure step down from D. With several
    // SqIns, or a plain collection after the last one, this is exact only
    // for the last extension -- one `pre` per activation is all the data
    // model (and the blob) carries.
    int64_t pre_tick =
        has_sqin ? timing.plusmeasure(timing.timecode(post_tick), -sp_bars_to_measures(1)).ticks()
                 : post_tick;

    std::optional<TransferScale> post =
        transfer_scale_between(act_tick, post_tick, timing);
    if (!post) return std::nullopt;

    ActTransferScales scales{*post, *post};
    if (pre_tick != post_tick) {
        if (std::optional<TransferScale> pre =
                transfer_scale_between(act_tick, pre_tick, timing))
            scales.pre = *pre;
    }
    return scales;
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
    out.scales = ActTransferScales{act.transfer_pre, act.transfer_post};

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

    // SqIn phrase notes: the offset is measured from the end before the
    // phrase extended SP, so they read `pre`. A SqOut is not rated here: its
    // offset is its squeezed-out row's offset (both are the deact edge's
    // sqinout_timing), and that row was rated above, at the end it is
    // measured from.
    out.note_effective_ms.reserve(act.sqinouts.size());
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind == SqueezeKind::SqOut) {
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        const NoteRating n = rate_note(sq.offset_ms, core::paid_by_sp_walk(sq.offset_ms),
                                       out.scales.pre, hit_window_ms);
        out.scale_governs |= n.effective_ms.has_value();
        out.note_effective_ms.push_back(n.effective_ms);
    }

    // The cap-clamped flag fires when the activation has a clamp_tick AND at
    // least one squeeze the frontend decides: any SqIn/SqOut, or any backend
    // row is_frontend_decided accepts.
    if (act.clamp_tick.has_value()) {
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
