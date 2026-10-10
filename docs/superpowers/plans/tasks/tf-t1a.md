Read docs/superpowers/plans/tasks/_tf-preamble.md first; it holds the rules.

# Task TF-T1a: build the four real old databases

Task id: `tf-t1`. Worktree `.claude\worktrees\tf-t1` on `claude/tf-t1`; make it as the preamble says. Your spec is the plan's "Task 1", the "How to produce them honestly" paragraph. You make and commit the fixture files and their README. You write no tests; a second agent (TF-T1b) writes them in your worktree from your README.

## The user's decisions

Four files, from four real releases: `v1.8.4`, `v1.8.4` run with `--legacy-fills`, `v2.0.0` and `v2.1.0`. Each file is at most 256 KB and the set at most 1 MB. If a file comes out bigger, rebuild it from two charts instead of three and say so.

## Steps

1. **Pick three charts** from `testdata/input`. The corpus holds no `.sng`, so take one `.chart` and two `.mid` (the plan's "one `.sng`" can't be met; say so in the README). Prefer short charts. Copy them into one scratch folder, `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\88f58f4f-74e2-4da8-bf4a-49622afa49e2\scratchpad\tf-t1\charts`, each in its own song folder with its `song.ini`.
2. **Build each old tag.** For each of `v1.8.4`, `v2.0.0`, `v2.1.0`: `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add --detach <scratch>\tf-t1\wt-<tag> <tag>`, then `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\build_slot.ps1 -Repo <scratch>\tf-t1\wt-<tag> -Target hydra_batch` (today's slot helper, run against the old tree). If the old tree's `build_cpp.ps1` takes no `-Target`, say so and build its default. Configure trouble under VS 2026: memory note `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\vs2026-incomplete-instance.md` has the generator-instance workaround. **Stop rule:** 10 minutes of trying per tag (not counting slot waiting); then stop that tag, carry on with the others, and report the exact error.
3. **Run each old `hydra_batch` from its own build folder**, so mimalloc's DLLs sit beside it: `<wt>\build-cpp\Release\hydra_batch.exe --db <scratch>\tf-t1\<name>.db <charts folder>`. Check the tag's `src/cli/batch.cpp` for the flags first (`git show <tag>:src/cli/batch.cpp`); the plan says `--db` exists in all three. For the legacy file add `--legacy-fills` to the v1.8.4 run. Never point it at the user's library.
4. **Shrink each file** with Python's sqlite3 (`py`): open the db, `VACUUM INTO '<worktree>\testdata\store\<name>.db'`. Names: `v1.8.4-schema2.db`, `v1.8.4-schema2-legacy-fills.db`, `v2.0.0-schema3.db`, `v2.1.0-schema4.db`. Check the size caps.
5. **Read the facts the tests will pin** from each frozen file with Python (copy it first, so nothing writes `-wal`/`-shm` beside it): `PRAGMA user_version`; the column list of `results`; the results row count and the sorted `result_id`s; `hyversion` or whatever stamp each row or `meta` carries; `legacy_fills` per row where the column exists; the `charts` rows and their names; for each results row, bytes 5 to 12 of the `structure` blob as little-endian hex (the rules fingerprint `rules_fp_of("structure")` reads; check the offset in today's `src/store/record_store.cpp` before you trust the plan's "byte 5"); and for v2.1.0 the row count of each detail table (`kDetailTablesCountSql` names them). If a chart failed to analyse in an old version, record that rather than swap charts silently.
6. **Write `testdata/store/README.md`** in plain English: for each file the tag, its commit hash (`git rev-parse <tag>^{commit}`), the exact commands, the three chart paths, the byte size, and every fact from step 5 as a table. Say how to regenerate and compare. Say the files are read-only fixtures: a test copies one to a temp path before opening it.
7. **Clean up** your three old-tag worktrees with `git -C <main checkout> worktree remove --force <path>` (you made them; nothing else).
8. Commit the four `.db` files and the README (stage by name; check `.gitignore` doesn't hide `*.db` under `testdata/`, and if it does, report it rather than editing `.gitignore`).

## Owned files

`testdata/store/*.db` (four new), `testdata/store/README.md` (new). Nothing else in the repo.

## Tests

None to run. Building `hydra_tests` in your worktree (done by `new_worktree.ps1`) is for TF-T1b.

## Return

The preamble's report, plus: per tag, build time and any workaround used; the facts table from step 5 copied into the report.
