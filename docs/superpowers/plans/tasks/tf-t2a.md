Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T2a: the golden test's measurement, and everything except its thresholds

Task id: `tf-t2`. Worktree `.claude\worktrees\tf-t2`; make it. Your spec is the plan's "Task 2". The thresholds (the per-pixel delta D, the worst-tile budget P and the whole-frame budget F, and the tightened mean) are **not decided yet**. The user picks them from your measurement. So you build everything around them, measure, and stop. A second agent (TF-T2b) adds the user's numbers and the checks in your worktree.

## Steps

1. **The metric, printed but not checked.** In `tests/test_preview_golden.cpp`, add one function that computes, at the half resolution the test already compares at, over the unmasked pixels: each pixel's largest channel delta; for a given D, the share of pixels over D in each 16 by 16 tile (worst tile reported with its position) and in the whole frame. The test prints these with `MESSAGE` for a few D values. Don't add `CHECK`s on them yet; the mean's existing check stays as it is.
2. **The measurement aid.** Under the existing `HYDRA_PREVIEW_GOLDEN_DUMP` env var (line 180 today), also write the per-tile error map as CSV and the histogram of per-pixel max-channel deltas into the folder that var already writes to. Don't touch the separate `HYDRA_PREVIEW_DUMP` case; another branch deletes it.
3. **Measure.** Run the golden case with the dump on. Then measure the same numbers for the "bites" render the plan names: the chart rendered with `pro=false` (the test's `render_chart` takes `pro`), compared against the same golden. Use a temporary local edit for that run and revert it; don't commit it. Report, for both renders: the mean; the 50th, 90th, 99th and 99.9th percentile of the per-pixel delta; and for D = 16, 24, 32, 48 and 64 the worst tile's percent (and where it is) and the whole-frame percent. Then give the plan's suggestion: per D, what P and F would be with a 50 percent margin over today's render, and whether the `pro=false` render would fail at those P and F. That gap is the evidence the user picks from.
4. **Fail loudly without the fixture.** A missing `golden.json` or `golden_onyx.png` becomes a `REQUIRE`, not "skipped". Every key the test reads is `REQUIRE`d present, and the fallback values (`tolerance` 20 at line 151 and any other default) go. The file header comment's "the suite passes before the capture exists" sentence goes. Red run: rename the fixture path in a local edit, show the `REQUIRE` failing, restore.
5. **Recapture README.** Write `testdata/preview/README.md`: what `golden.json`'s `_comment` records today (Onyx build, chart, Expert Pro Drums, the pause at 0:36.913, the whole window, the `crop`), and numbered steps for recapturing so the crop comes out 1372 by 714 again. The Onyx window size and how the pause landed on exactly 0:36.913 aren't recorded anywhere; mark both "to confirm at the next recapture". Leave a placeholder sentence where the thresholds will be explained; TF-T2b fills it.
6. **ADR 0008.** Change its "re-capture the golden" sentence to point at `testdata/preview/README.md`.

Commit after steps 1-2, 4 and 5-6.

## Owned files

`tests/test_preview_golden.cpp`, `testdata/preview/golden.json` (only if a key's removal needs it; no new keys yet), `testdata/preview/README.md` (new), the ADR 0008 file under `docs/adr/` (one sentence).

## Tests

`build-cpp\Release\hydra_tests.exe -sf=*test_preview_golden*` only.

## Return

The preamble's report, with the measurement as two tables (today's render, `pro=false` render) and the margin suggestion per D.
