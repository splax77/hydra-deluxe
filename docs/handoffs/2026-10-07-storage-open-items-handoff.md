# Summary-only storage: open items (handoff, 2026-10-07)

The storage change itself is finished and pushed. T5 and T6 merged together as 6bddaa8, and the measured results are in `docs/handoffs/2026-10-07-summary-storage-results.md`. This note lists what is still open, so a fresh session can pick it up without the old conversation.

## 1. In flight: the D94 cleanup branch

One agent was still working when this note was written. The journal showed these lines for the last 30 minutes:

```
agent-a03df3466e24a893b: unfinished, last tool call SubagentHandback, touched 12:30
agent-a62e9d9d78bb4c1ae: unfinished, last tool call Read, touched 12:39
```

The first of these, a03df3466e24a893b, ran the library, upgrade and timing checks. Its hand-back report did arrive, and its results are in the results doc above.

The second, a62e9d9d78bb4c1ae, is the open one. It works in `.claude\worktrees\agent-a62e9d9d78bb4c1ae` on branch `worktree-agent-a62e9d9d78bb4c1ae`, cut from main at 39ddb5e. Its job has two parts:

- **D94, a user decision.** The Preview's changed-chart line becomes "This chart changed since it was analyzed. Click the song again to see its path." The UserGuide's line 145 changes to match, and a failing-first test pins the new text.
- **Dead code, a code-only change.** It removes the stored-result error path (`kStoredResult`, `ErrorKind::StoredResult`, `SerializeError`), along with whatever is dead in `src/store/serialize.*`. Its last status said that `BinaryWriter` is still used by the store, so it keeps that and trims the rest. It also fixes comments that still mention the Analyze button.

What's left after it reports:
1. Run `tools\derive_once_precheck.ps1 -Range "main...<tip>"`.
2. A Sonnet reviewer follows `docs/agents/derive-once-review.md` and signs the tip.
3. Trial-merge with `git merge --no-ff --no-commit`.
4. Delete `build-cpp\**\*.iobj` and `*.ipdb`, then do a clean `.\build_cpp.ps1`.
5. Run the full suite: `hydra_tests.exe` and `hydra_uitest.exe --all --jobs 4`.
6. Commit with trailers and push.
7. Remove the worktree and its branch.

If the agent never reports, its branch holds whatever it committed. Read `git log main..worktree-agent-a62e9d9d78bb4c1ae`, then finish the work from the description above with a fresh agent.

## 2. Proposed sessions waiting for the user

Each of these is a card in the session where this plan ran. Nothing starts until the user starts it.

- **Make hydra_replay use the engine, not a copy (Track R).** This is D87 item 8, the parallel track.
- **Fix the agent call counter that resets mid-run.** `agent_progress_gate.ps1` saves its state with `Set-Content`. When two tool calls run at once, a hook run can read the file half-written and start counting again from zero. One agent reached about 237 calls with the 150 cap never firing. The fix is an atomic write, plus failing loudly when the file can't be read.
- **Stop stale link state from breaking builds.** Leftover incremental link-time files (`.iobj` and `.ipdb`) caused C1001/LNK1000 link errors or a crashing test exe about seven times today. The fix is to find the cause in the CMake and LTCG flags, and ask the user before changing the shipped build's flags.
- **Find why the panel-split UI test flakes.** It failed once under `--all --jobs 4`, then passed 3 of 3 alone and 70 of 70 on a rerun. The rule is to find the root cause, never to add a retry.

## 3. Deferred minor findings from the reviews

None of these changes what the user sees today. Each is worth fixing when someone is next in that file.

- **"No records stored yet" on a database with no library.** The GUI report job shows that message there. A GUI database always has a library after a scan, so the user can't hit it.
- **The naming-copy join relies on SQLite's automatic column name.** `RecordStore::naming_copy_paths` joins on `c."MIN(rowid)"`. An alias in `kNamingCopiesSql` would be sturdier.
- **One UI test half skips on a busy machine.** The "no box" half of the `view-progress-delay` test skips there, and logs that it did.
- **A touched file is re-hashed on every click.** If a chart file's modified time changes but its content doesn't, every click re-hashes it until the next rescan, because the stored signature isn't refreshed. The Preview already behaved this way.
- **report.cpp asks one question twice.** It checks "does the store hold any results" twice in one flow.
- **One rule, two spellings.** The rule "this chart is in the library" is written with two different maps, in `collect_rows` and in `records_by_hash`. The two should share one owner.
- **One loose reason text.** The reason text at `tests/test_single_owner.cpp` around line 4540 is worded loosely.

## 4. Rulings the coordinator made without the user

These are the judgment calls made during the run, each with its cost if it was wrong. The user decided D87 and D89–D94 directly, and those are recorded in `docs/audit/2026-10-03-fix-decisions.md`.

1. **T4 removed only the CLI side of reindex.** It took out only `--reindex` and the dump store path, and `RecordStore::reindex()` moved to T5. If wrong, dead code lived until T5, and T5 has since deleted it.
2. **T2 waited for T3 as well as T1.** If wrong, T2 started later.
3. **`copies_of` kept its "counts once" fallback.** If wrong, a fallback branch in the reports is dead.
4. **T5's "grep finds nothing" check covered C++ files only.** `compare_db.py` has to keep naming the dropped tables so it can skip them. No cost.
5. **T1 reordered `test_fill_report`.** It builds the library before storing results. If wrong, one test would have gone red at the join. None did.
6. **`hydra_replay dump` got a one-sentence developer help line.** If wrong, one sentence needs rewording.
7. **The join compared report rows as a sorted set.** If wrong, the order of tied rows went unchecked.
8. **T3 added one store accessor, `naming_copy_paths`.** If wrong, a small merge conflict.
9. **The report follows `list_records` order.** Tied rows may come out in a different order. This was not treated as a display change. If wrong, tied rows show in a different order on the page.
10. **The click reuses its Song for the Dynamics count only when the parse options match.** A test proves the rules can't change that count: 388 charts, with fills moving on 110 of them. If wrong, a Dynamics count could differ from the Dynamics tab's own parse.
11. **No in-memory view log.** If wrong, a determinism bug would overwrite a row silently.
12. **One click analysis at a time, and the latest request wins.** If wrong, holding a +/- key shows the result after a short lag.
13. **Closing the panel cancels the analysis and saves nothing.** If wrong, a row stays Stale until the song is clicked again.
14. **A batch refreshes library rows but not the open panel.** No visible cost.
15. **If `reidentify_chart` fails, the click shows an error and saves nothing.** If wrong, one more error path to test.
16. **The digests still cover every path detail.** `digest::record_hash` walks the engine's in-memory result, and the literals were re-pinned once, with a comments-only engine diff as proof. If wrong, a later performance change could alter paths unseen.
17. **The upgrade deletes only rows with no stars and a score.** On the real database that was 290 old Stale rows. If wrong, a no-paths Ready row would be lost.
18. **The Stale tooltip on old-layout rows may add "other rules".** On the real database nothing changes. If wrong, the tooltip names one extra reason on a row that's already Stale.
20. **T5's finisher was allowed to finish past the call cap**, because the gate had lost count. If wrong, a few dozen extra calls.
22. **T5 and T6 merged as one**, because the docs test needs both. If wrong, one larger merge to review.

Numbers 19, 21 and 23 are the user's decisions D92, D93 and D94.

## 5. Elsewhere

- **video-tools (D89 item 3) is done.** `C:\Users\Patrick\Downloads\Hydra\video-tools` commit 170e9cb stopped passing `--db` to `hydra_replay dump`. That repository is separate, so check its own push state there.
- **Throwaway files.** The scratch databases and scripts from the join checks sit in the session scratchpad, under `join\`. They are temporary and not needed again.
