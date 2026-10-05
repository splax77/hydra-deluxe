Read docs/superpowers/plans/tasks/_phase3-preamble.md first; it holds the rules.

# Task C4c: one budget edge (the code half of finding 94)

Task id: C4c. Base: main at dc7e582. Branch: claude/p3-c4c (worktree `.claude\worktrees\p3-c4c`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phases-3-5.md`, wave C table, "C4c One budget edge". Recheck: `docs/audit/2026-10-04-session-audit/step3b.md`, finding 94 in the status table and the "Rating ladders" paragraph. Decision: D48 Q3 in `docs/audit/2026-10-04-phase-3-5-questions.md`, which this task does not act on (see "Not in this task").

## Goal

The two-hit squeeze budget is twice the hit window. One function already owns that fact, `squeeze_budget_ms` in `src/core/squeeze_rating.cpp`, but two other functions in the same file write the factor of two again by hand. This task makes those two read the owner. Every tier name, cutoff and effective-ms figure stays exactly what it is today; this is code-only.

## Finding 94: what the code does today

`squeeze_budget_ms(transfer_r, hit_window_ms)` in `src/core/squeeze_rating.cpp` returns the window times one plus the scale. At the identity scale (1.0) that is twice the window: 170.0 ms at the default 85 ms window. `rate_note` and the Paths-tab tooltip in `src/app/path_view.cpp` already read it. It is the owner and it does not change.

`timing_tiers(hit_window_ms)`, same file, builds the report's tier ladder. Its Insane+ cutoff is written as the literal "2 * w". That is the budget written a second time.

`effective_backend_ms(offset_ms, transfer_r)`, same file, maps a raw gap onto the nominal two-window scale. It hard-codes the literal 2.0. That is the budget written a third time.

The audit's fourth copy, `transfer_is_material`, is already gone from `src/`. The recheck's lines (161 and 62) still match the base; nothing in these files moved between 50e4b0b and dc7e582.

## What changes

Owner: `squeeze_budget_ms`, home file `src/core/squeeze_rating.cpp`. Unchanged.

`timing_tiers` reads its Insane+ cutoff from `squeeze_budget_ms` at the identity scale and the given window, instead of the literal. The other four cutoffs (the 2.0 ms Normal floor from `kDifficultMs`, then half, one and one-and-a-half windows) stay as they are. `beyond_edge_ms` already reads the table, so it follows for free.

`effective_backend_ms` gets its factor of two from `squeeze_budget_ms` instead of the literal 2.0. The factor is the identity budget over the budget at the row's own scale. The window cancels in that ratio, so the function's signature stays and the header stays untouched. The result must be the same number as today at every existing pin, at the existing epsilons. If the ratio form misses a pin at its existing epsilon, stop and report; do not loosen an epsilon (that would be a new tolerance, which the plan forbids).

No scan row: this task adds no new owner, so there is no new single-owner rule to guard. The pin tests below do the guarding.

## Owned files (only these may change)

- `src/core/squeeze_rating.cpp`
- `tests/test_squeeze_rating.cpp`

The plan row names the production file. The test file comes with it because the plan's test filter points at it and the cases below live there. `src/core/squeeze_rating.h`, `src/app/report.cpp`, `src/app/path_view.cpp` and `tests/test_single_owner.cpp` are not yours.

## Test cases to add (in `tests/test_squeeze_rating.cpp`)

Write each red first, then green. Use the existing cases next to them as the pattern; no new helpers.

1. `timing_tiers: the Insane+ cutoff is the identity squeeze budget`. Pins: at the default window the Insane+ cutoff equals `squeeze_budget_ms(1.0, kDefaultHitWindowMs)` and equals 170.0; at the 70 ms window the existing ladder case already uses, it equals `squeeze_budget_ms(1.0, 70.0)`. Beyond and None still carry no cutoff.

2. `effective_backend_ms: the factor of two is the identity budget over the scaled budget`. Pins: `effective_backend_ms(170.0, 1.0)` equals 170.0 (at the identity scale the figure is the raw gap, because the budget is 170.0 at the default window); and the figure equals the raw gap times `squeeze_budget_ms(1.0, W)` over `squeeze_budget_ms(r, W)` for the scale the existing field-fixture case ("field fixture: Dumpweed SqOut end anchored on the deact node") already computes, with no change to that case's own pins.

Existing cases that must pass unchanged, as proof that nothing moved: `timing_tiers: the ladder at W=85 and at W=70`, `squeeze_budget_ms: identity scale is twice the hit window`, `beyond_edge_ms: the last finite timing-tier cutoff`, every `rate_note:` and `rate_activation:` case (their effective-ms pins at 1e-12), and in `tests/test_report.cpp` the `tier_for` cases, including `tier_for: raw-ms bands derived from the two-hit budget` where 170.0 still reads Beyond today.

The only new numbers you may type are 170.0, 1.0 and the 70.0 window the ladder case already uses. Every other value comes from calling the owner.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*squeeze_rating*`
- `build-cpp\Release\hydra_tests.exe -tc="tier_for*"`

Nothing else. Never the full suite.

## Not in this task

- The report's Timing column help in `src/app/report.cpp` still says "Beyond means at least twice the hit window" as prose. That is the display half of 94 and belongs to task K1a.
- D48 Q3 (a timing exactly on an edge counts as inside, so 170.0 ms becomes Insane+ and 2.0 ms stays not difficult) is a display change. The Normal edge belongs to O1 and the report's `tier_for` edge to K1a. Leave both edges exactly as they read today; do not re-pin 170.0 or 2.0.
- The header comments in `src/core/squeeze_rating.h` that describe the "nominal 2*W scale" are prose about the same fact and stay.

## Done when (from the plan)

- Same tiers and same numbers, with one source for the factor of two: no "2 * w" and no bare 2.0 budget factor remains in `src/core/squeeze_rating.cpp`; both readers call `squeeze_budget_ms`.
- The two new cases pass, and every existing case in the two filters above passes with no edit to its pins.
- `git diff --stat dc7e582..HEAD` lists only the two owned files.
- No score, path or stored record changes; the results stamp stays "2.1.0" (the main session proves it with byte-identical corpus scores at merge M_C).

## Open questions

None. D48 and the plan's test-limits section cover every number this task pins.

## Commits

One commit, trailers `Task: C4c` plus the preamble's others. Report as the preamble says: owner, file, one sentence per change, each case's red line and green result, the diff file list.
