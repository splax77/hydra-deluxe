Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T1b: the upgrade tests on the four real old databases

Task id: `tf-t1`. The worktree exists: `.claude\worktrees\tf-t1` on `claude/tf-t1`, tip `14923bb7`, with `hydra_tests` built. Don't make a new one. Your spec is the plan's "Task 1", from "What each new test asserts" to the end, adjusted by this brief.

## First: bring the branch up to date

`main` has moved since this branch forked; the trim branch changed `tests/test_store.cpp` and `src/store/record_store.{h,cpp}`. Run `git merge --no-ff main` in the worktree first, committing it with the preamble's trailers (`-m` subject plus the three `--trailer`s; a merge with no Agent trailer is refused). Then warm-build.

## What you have

`testdata/store/README.md` (read it in full; it is your fact sheet) and four fixtures: `v1.8.4-schema2.db`, `v1.8.4-schema2-legacy-fills.db`, `v2.0.0-schema3.db`, `v2.1.0-schema4.db`. Each holds three results rows for the same three charts.

## Where the real files differ from the plan (orchestrator's decisions)

1. **v2.1.0's `user_version` is 0, not 4.** That is what the real release wrote (its `record_store.cpp` stopped setting it). Pin 0, and pin the column set (`rules_fp` present) as what tells that layout apart. Check how today's `RecordStore` recognises each old layout before you pin anything.
2. **The `charts` table is empty in all four.** `hydra_batch` never fills it. Drop the plan's "charts rows keep their names" check. Instead, match rows across the upgrade by `hyhash` (result ids differ between files; the README lists the three hashes) and check the set of `hyhash` and `chartmode` pairs is unchanged.
3. **All four hold the detail tables** (`kDetailTablesCountSql` reads 5 in each), so all four go through the summary-only upgrade. Check the detail-table count is 0 and the `structure` column is gone after the open, in every file.
4. **`.gitignore` hides `*.db`.** The fixtures were added with `git add -f`. Add one line `!testdata/store/*.db` right after the `*.db` line so a regenerate needs no force.

## What each test asserts

Per the plan, and per fixture: copy to a temp path first; before opening, `PRAGMA user_version` and the absent columns (literals from the README); open with today's `RecordStore` and a progress log; then the results row count (3), the `hyhash`/`chartmode` set unchanged, `rules_fp` on every row equal to the README's fingerprint literal, `legacy_fills` 0 (1 for the legacy-fills file only if today's upgrade sets it from the file's `engine_mode` stamp; find out which, and pin what the code does, saying which function decides it), every result reads **Stale** (through the store's own status call, never by comparing version strings in the test), detail tables gone, the progress log's upgrade steps (pin what each file reports), and a second open reporting no upgrade step with the same counts. One `TEST_CASE` per fixture, or one table-driven case; your choice.

A missing fixture fails with a `REQUIRE` on the file's existence. Red run: point one case at a wrong file name, keep the red line, restore.

## What goes away

The three synthetic schema cases and `downgrade_to_schema2/3` in `tests/test_store.cpp`. Then `kSchema2ResultsTableSql` has no reader: remove it from `src/store/record_store.cpp` and `.h`. Keep `kSchema2ResultsColumns` (the upgrade and `old_layout_fixture.h` use it). If a `tests/test_single_owner.cpp` row names the removed constant or the removed cases, update that row.

## Owned files

`tests/test_store.cpp`, `src/store/record_store.{h,cpp}` (the one constant), `tests/test_single_owner.cpp` (only a row naming what you removed), `.gitignore` (the one line), `testdata/store/README.md` (only to add what the tests pin, if the README lacks it).

## Tests

`-sf=*test_store*` and `-tc="single-owner*"`.
