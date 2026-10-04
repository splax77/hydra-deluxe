// What one backend row is worth on one path: the single answer the search,
// the replay and the details table all read. Before this file each of the
// three wrote its own copy, and the table's copy ignored the row's offset
// (docs/audit/2026-09-24-derivation-audit.md, finding 1).
//
// Header-only and inline so the engine's per-edge loop pays no call.

#ifndef HYDRA_CORE_BACKEND_VALUE_H
#define HYDRA_CORE_BACKEND_VALUE_H

#include <cstdint>
#include <optional>

namespace hydra::core {

// Where a row sits against the activation's squeezed-out chord, compared by
// tick. NoSqOut when the activation did not squeeze out.
enum class SqOutPosition { NoSqOut, Before, Exact, After };

inline SqOutPosition sqout_position(int64_t row_tick,
                                    std::optional<int64_t> sqout_tick) {
    if (!sqout_tick) return SqOutPosition::NoSqOut;
    if (row_tick < *sqout_tick) return SqOutPosition::Before;
    if (row_tick == *sqout_tick) return SqOutPosition::Exact;
    return SqOutPosition::After;
}

// The SP walk pays every chord at or before the SP end (the window is
// inclusive at the deactivation node), before any backend pricing happens.
inline bool paid_by_sp_walk(double offset_ms) { return offset_ms <= 0.0; }

// The same edge by tick: is this chord after the SP end? A chord exactly on
// the end is not, so it agrees with paid_by_sp_walk wherever time runs
// forwards (parse refuses timing that does not, D6). The graph's late
// squeeze choice and the engine's squeeze-out bank tick ask it.
inline bool after_sp_end(int64_t chord_tick, int64_t end_tick) { return chord_tick > end_tick; }

// Counted under Star Power with no squeeze: at or before the SP end, or
// less than the leeway (Rules::backend_leeway_ms) after it. Exactly the
// leeway after it is not counted; the edge is strict (decision D29).
inline bool counted_without_squeeze(double offset_ms, double leeway_ms) {
    return paid_by_sp_walk(offset_ms) || offset_ms < leeway_ms;
}

// The SP score this row adds on this path. `points` is the row's full SP
// value, `sqout_points` what is left when it is the squeezed-out chord.
inline int backend_row_value(double offset_ms, int points, int sqout_points,
                             SqOutPosition pos, double leeway_ms) {
    if (pos == SqOutPosition::After) return 0;
    if (!counted_without_squeeze(offset_ms, leeway_ms)) return 0;
    return pos == SqOutPosition::Exact ? sqout_points : points;
}

// Did Star Power pay this row anything on this path? The one yes/no for
// "inside SP", built from the price above. The replay's xN disc doubles
// exactly when it is true (decision D2): a squeezed-out chord whose
// sqout_points are 0 reads no, a partly paid one reads yes.
inline bool paid_by_sp(double offset_ms, int points, int sqout_points,
                       SqOutPosition pos, double leeway_ms) {
    return backend_row_value(offset_ms, points, sqout_points, pos, leeway_ms) > 0;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_BACKEND_VALUE_H
