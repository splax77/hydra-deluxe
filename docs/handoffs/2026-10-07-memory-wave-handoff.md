# Handoff: the memory-fix wave (2026-10-07, evening)

Written 2026-10-07 at the user's request to stop all work. Nothing is running. The memory audit's fixes are built, reviewed and joined on a branch, but nothing has merged to main. One question is open before the merge: the join makes the app's UI about 0.37 s slower on a 326-chart test, and the cause isn't proven yet. Read this whole page before doing anything, and don't launch new runs until you can say what each one will decide.

## Where things stand

Two decision commits are on main. D95 (b929f55) is the user's four calls from the audit. D98 (7227894) is three follow-ups from the fixes. Both are in `docs/audit/2026-10-03-fix-decisions.md`. Main has moved on since then with other sessions' work (it was at 2460cc0 when this was written).

The join branch is `claude/mem-join` at f3195f7, in the worktree `.claude\worktrees\mem-join`. It is main at 7227894 with seven reviewed branches merged in, plus one join fix of mine. Each branch passed its derive-once review, and these are the keys the reviews signed off:

| Branch | What it does | Reviewed tip |
|---|---|---|
| `claude/mem-mem-f` | Fonts are memory-mapped instead of copied (fix 5, D95.3) | c53dacf |
| `claude/mem-mem-b` | The batch report's rows and hydra_batch's scan are freed as soon as they're done (fixes 1 and 4) | 9e739ee |
| `claude/mem-mem-t` | The Preview renderer frees what it built when setup fails, plus a Debug leak check (fixes 3 and 8) | 94f5b00 |
| `claude/mem-mem-p` | Per-chord arrays get their size up front; the chord list is trimmed after parsing (fix 2) | 3707b52 |
| `claude/mem-mem-g` | `mi_collect(true)` when a batch ends, and the Preview frees its GPU targets on close (D95.1, D95.4) | 181e3c9 |
| `claude/mem-mem-u` | The headless UI test runner flushes the GPU every frame (D98.1) | cbdbd6e |
| `claude/mem-mem-l` | Library rows keep only what the table needs; a click reads the rest from the store (fix 6, D98.2-3) | de8b5e1 |

Fix 1 needed no code: D87 had already made each worker free its song once the row is built. MEM-B confirmed it from the code.

The join fix, f3195f7, is mine and has had no review. A D96 test that came in on main read `row.entry.sig`, which MEM-L removed. Each branch compiled alone, but they broke together. I dropped that one check (the test still checks the store's row and the selected row), and reworded one comment in `src/ui/app_state.cpp`. MEM-R made the same test fix on its own branch, so the merge will need that hunk reconciled.

One more branch is reviewed CLEAN but not yet merged into the join. `claude/mem-mem-r` at 00f0431 adds `AppState::reload_after_scan()`. A click, previous/next and the batch confirm now first read a scan that has just finished. That closes the one-frame window where a click could silently do nothing, or the "Analyze search (N)" count could disagree with the confirm.

## What was measured, and how sure it is

**Correctness, solid.** I ran a fresh-database `hydra_batch` over the whole library with the baseline exe (main at 7227894, built in `.claude\worktrees\mem-baseline`) and with the join exe. `tools/compare_db.py` gave results 18,837 rows compared, 0 differ, charts 19,489 rows compared, 0 differ, and meta 3 rows compared, 0 differ. Both exes need `hydra_settings.ini` beside them for a fresh run (I copied the installed one into both build folders).

**Whole-library `hydra_batch --redo`, solid.** One warm-up, then baseline and join alternating, three each, through `tools/bench_run.ps1` with 0 compilers. Wall time didn't move: 3.21 to 4.65 s for the baseline, 3.46 to 4.24 s for the join. Peak committed memory fell from 771 to 901 MB down to 518 to 585 MB. The total memory allocated over the run halved, from about 68 GiB to 35 GiB. The live peak in mimalloc's stats rose slightly, from 450 to 458 MiB to 469 to 475 MiB. That rise isn't explained yet; the .mid reserve counts, which can run high before the trim, are one guess.

**Memory after an in-app batch, solid, not split by branch.** `hydra_uitest --db <copy of the installed db>` ran a whole-library batch. After it, the baseline sat at 170 to 172 MB private and the join at 61 to 64 MB. The peak during the batch was 732 to 870 MB against 304 to 337 MB. This is the join against the baseline, so it doesn't show how much of the drop is `mi_collect` itself and how much comes from MEM-B, MEM-L and MEM-P.

**The full-library in-app wall times are not reliable.** Those runs read 50 to 62 s for the baseline and 60 to 78 s for the join. But `wait-idle` in the test runner returned while the path report was still running in 7 of 10 small runs, so those times end at an arbitrary point. Don't quote them.

**The 326-chart UI slowdown is real, but its cause isn't proven.** On a copy of the installed db, the search "love" gives "Analyze search (326)...". The script is in the scratchpad as `join\mid.txt`, and `join\phases.ps1` timestamps every script line. Fastest of three:

| Phase | Baseline | Join | Difference |
|---|---|---|---|
| Startup | 0.855 s | 0.871 s | +0.016 s |
| First `wait 0.5` (library loads, first frames draw) | 1.492 s | 1.686 s | +0.194 s |
| Typing "love" | 0.820 s | 0.896 s | +0.077 s |
| Opening the confirm | 0.031 s | 0.083 s | +0.052 s |
| Batch plus report | 0.245 s | 0.266 s | +0.021 s |
| Shutdown | 0.035 s | 0.026 s | −0.009 s |

From reading the code, here is what explains it and what doesn't. The confirm's extra time has a known cause. `AppState::library_matches` (app_state.cpp:114) now reads all 19,436 chart rows from the store, once, to find the 326 it needs; before MEM-L it copied them from memory. The second wait, after the search narrows the table, is the same in both builds, so per-frame cost isn't the problem. That rules out MEM-U's flush and the draw-time best label (`best_label()` has one caller, the visible-row draw at library_table.cpp:400). The extra time is only in frames that draw something new: the first full table, the rows that change while typing, and the confirm, which draws at a new font size. Those are the frames where ImGui builds new glyphs, and MEM-F changed where the font data lives. So MEM-F is the main suspect. Nobody has proven it, and its mechanism (page faults on a cached mapped file) should cost milliseconds, not 0.2 s. Don't state it as the cause.

**The next step was this, and it was stopped before it ran.** Build the baseline with exactly one branch merged in, so the only difference is that branch. That exe exists for MEM-F: the scratchpad's `join\bf\hydra_uitest.exe`, with its two mimalloc DLLs, is main 7227894 plus `claude/mem-mem-f`. The baseline worktree was restored to 7227894 and rebuilt afterwards. Run the phase script with three exes (baseline, baseline plus F, join), fastest of three. If baseline plus F shows the +0.19 and +0.08 s, MEM-F is the cause. If not, do the same with MEM-L, the other change in those frames. Keep these runs at 326 charts. The user stopped a 19,000-chart run as a waste of time, and said to explain where a difference comes from and how a run will find it before proposing one.

**The per-branch run is void.** Each branch's own `hydra_uitest` was built on bc48742, not 7227894, so those numbers mix each change with main's own changes. MEM-T's Release exe in its worktree is also a leftover experimental build from the per-frame GPU-wait trials. Its later Release builds failed with compiler crashes, so it was never rebuilt.

## What the next session does, in order

1. Find the cause of the 326-chart slowdown with the one-branch builds above. If it's MEM-F, the fix stays in `src/ui/app_shell.cpp` (for example, reading the glyph-heavy file into memory while mapping only the large fallback). That's a code-only change, so check with the user that 0.2 s on the first draw is worth fixing before trading memory back. If it's MEM-L's full-table read, `library_matches` can read only the matched charts by key instead.
2. Merge `claude/mem-mem-r` into the join, keeping one copy of the D96 test fix.
3. Dispatch a derive-once reviewer for the join's own commits (f3195f7 and the MEM-R merge resolution). The merge to main needs a key the gate accepts.
4. Build the join with all targets. Delete `build-cpp\hydra_tests.dir\Release\hydra_tests.iobj` and `.ipdb` first: stale incremental links crashed the compiler (C1001, LNK1000) or produced crashing test exes in four worktrees today. Run the full suite once. It hasn't been run on the join yet.
5. Merge to main, then clean up the worktrees `mem-baseline`, `mem-join` and the `mem-mem-*` ones.

## Open items found along the way (none block the merge)

- **`wait-idle` in the test runner** can return between a batch finishing and its report starting (test-only, `AppState::any_job_running` and the harness).
- **Two library-row fields are kept,** about 2.4 MB together: the seven unused path-summary fields, and the md5 as text. Removing them means rewriting fixtures in `tests/test_app_state.cpp` and `tests/ui/uitest_harness.cpp`.
- **A scan row for `reload_after_scan`.** MEM-R's reviewer proposed one, with pattern and examples in its review file. Its header comment also lists its callers, which will drift.
- **An existing bug:** pressing Continue in the same frame a scan finishes leaves the scan unread until the next scan (`library_dialogs.cpp`).
- **Stale comments:** one in `tests/test_single_owner.cpp` around line 3018 says the scan guard has no owner, and another near line 3900 still describes a pre-D87 song-length save.
- **Small notes from MEM-G's reviewer:** `PreviewController` caches the renderer's size (`rt_w_`, `rt_h_`), and close() must keep it in step. Two WARP tests share a four-line idiom that could move to `tests/warp_util.h`.
- **D95 item 2** (no cap on giant charts) said to re-measure the batch peak after fix 2. The `--redo` peak above is that re-measure; it fell about 300 MB, so the question needn't come back.

## Files

Review files for every branch are in this session's scratchpad under `reviews\`; briefs and implementer reports are under `briefs\`. The measuring scripts are under `join\`: `redo_pairs.ps1`, `gui_mem.ps1`, `mid.txt`, `phases.ps1`, `bisect.ps1` (void). The purge-delay sweep is under `sweep\`. The scratchpad is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\04f776a5-d916-40ee-ab0a-85c2e0b9dad4\scratchpad\`. Copy anything you need out of it before it's cleaned.
