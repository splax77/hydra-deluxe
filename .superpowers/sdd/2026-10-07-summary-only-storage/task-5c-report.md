# Task 5c report: compare_db --summary-only

Status: DONE_WITH_CONCERNS (one small concern, below).

Branch `worktree-agent-a2e3804de83a0eda7`, worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\agent-a2e3804de83a0eda7`. It starts from the store change (cbb1a52, a fast-forward merge). The code commit is cc8651a; this report is committed on top of it.

## What I built

`tools/compare_db.py` takes a new `--summary-only` flag. With it, the script compares only what a summary-only store keeps: `results`, `charts` and `meta`. It first prints which tables it skipped, so an old baseline's paths, path_refs, songmeta and dynamics show up by name instead of failing the run.

`results` is compared on the columns both files share, leaving out `result_id` and `structure`. The script prints those skipped columns too. Rows are matched by the table's unique key, read from the file itself (hyhash, chartmode and the settings columns), never by rowid. If the two files' unique keys differ, it says so and fails. `charts` and `meta` go through the same comparison as the full mode.

To avoid a second copy of the row-matching SQL, I split the old `compare_table` in two. The new `compare_rows` holds the matching and printing, and both modes call it. Without the flag, the script behaves exactly as before.

`tools/test_compare_db.py` gains two tests in the file's existing style, plus a `__main__` block. Before, `py tools\test_compare_db.py` ran nothing, because the file had no main. Now that command runs pytest on the file.

## Acceptance criteria

1. `--summary-only` compares `results` on shared columns without `result_id` and `structure`, by natural key, plus `charts` and `meta`, and names skipped tables. PASS.
2. Failing-first tests: an old-layout file against a new-layout file with equal summaries matches and names the skipped tables; one changed score is reported. PASS.
   The red run, before the code: `2 failed, 4 passed in 0.64s`, both new tests failing on `assert 2 == 0` and `assert 2 == 1`. That was the usage-error exit for the unknown flag.
   The green run: `py tools\test_compare_db.py` gives `6 passed in 0.60s`.
3. Acceptance on testdata. PASS.
   I built `hydra_batch` in this worktree (`.\build_cpp.ps1 -Target hydra_batch`). I built the baseline in a detached throwaway worktree at 523f719, `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\t5c-base`, through `tools\build_slot.ps1 -Target hydra_batch`. I left that worktree in place for the main session to remove.
   My first try put the baseline worktree in the scratchpad. Git refused it because some testdata paths ran past Windows' path-length limit. Nothing was left behind.
   Both exes ran with the same folder argument: this worktree's `testdata` as an absolute path. Each wrote to a fresh db in the scratchpad folder `t5c`. Neither exe has a `hydra_settings.ini` or `hydra_rules.ini` beside it, so both ran on the same built-in settings and rules. Those were Expert Pro Drums with 2x Bass, SP cap 4 bars and the Clone Hero 1.1 fill rule. Each run found 98 charts and analyzed all 98, none failed.

   ```
   py tools\compare_db.py --summary-only baseline.db new.db
   skipped tables: dynamics, path_refs, paths, songmeta
   skipped results columns: result_id, structure
   results: 98 rows compared, 0 differ
   charts: 0 rows compared, 0 differ
   meta: 2 rows compared, 0 differ
   exit 0
   ```

## Self-review

My change answers one question: do two stores hold the same summaries? `compare_db.py` already owned that question for the full layout. The new mode reuses its column reader, its unique-key reader and its row matcher. It holds no copy of the store schema; the keys and columns come from each file's own PRAGMAs. The only names it hard-codes are the three tables a summary-only store keeps and the two columns it leaves out, both from the brief.

## Concerns

The `charts` line compared 0 rows. hydra_batch run on folder arguments leaves the library alone, so both files have an empty `charts` table, and this run did not test that comparison. The unit test does cover it.
