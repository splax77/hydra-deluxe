# Summary-only storage: open items (handoff, 2026-10-07)

The storage change itself is finished and pushed. T5 and T6 merged together as 6bddaa8, and the measured results are in `docs/handoffs/2026-10-07-summary-storage-results.md`. This note lists what is still open, so a fresh session can pick it up without the old conversation.

## 1. Done since this note was first written: the D94 cleanup

The D94 cleanup branch merged as 79d4b1a, after its derive-once review came back clean (ec50762). The full suite passed: hydra_tests 1144/1144 and hydra_uitest 70/70.

The Preview line now reads "This chart changed since it was analyzed. Click the song again to see its path.", and the UserGuide quotes it the same way. The dead stored-result error path is gone: `BinaryReader`, `SerializeError`, `restore_timecodes`, `ErrorKind::StoredResult` and `kStoredResult`. `src/store/serialize.*` keeps only `BinaryWriter`, which `rules_fp_bytes` still uses.

## 2. Proposed sessions waiting for the user

The user started all four on 2026-10-07, each in its own session. Their results live in those sessions, not here.

- **Make hydra_replay use the engine, not a copy (Track R).** This is D87 item 8, the parallel track.
- **Fix the agent call counter that resets mid-run.** `agent_progress_gate.ps1` saves its state with `Set-Content`. When two tool calls run at once, a hook run can read the file half-written and start counting again from zero. One agent reached about 237 calls with the 150 cap never firing. The fix is an atomic write, plus failing loudly when the file can't be read.
- **Stop stale link state from breaking builds.** Leftover incremental link-time files (`.iobj` and `.ipdb`) caused C1001/LNK1000 link errors or a crashing test exe about eight times today. The last time, the reviewer of the D94 cleanup saw it in third_party imgui_te_perftool.cpp on every try, and worked around it with a RelWithDebInfo build. The fix is to find the cause in the CMake and LTCG flags, and ask the user before changing the shipped build's flags.
- **Find why the panel-split UI test flakes.** It failed once under `--all --jobs 4`, then passed 3 of 3 alone and 70 of 70 on a rerun. The rule is to find the root cause, never to add a retry.

## 3. Deferred minor findings from the reviews: all closed (2026-10-07, second session)

A second session fixed every item below in four reviewed branches. Each passed its derive-once review clean. som-b merged as e729e37, som-c as 8a7c5c5, som-a as 660929b and som-d as 446b5e8.

The two items that touched what the user sees or stores went to the user first, as D96 and D97 in `docs/audit/2026-10-03-fix-decisions.md`. Under D96, a click on a chart whose file was saved again with the same content now stores the new size and time, so later clicks skip the re-hash. An empty fingerprint is never saved, because it can't prove anything; `store::sig_can_show_unchanged` owns that rule. Under D97, the GUI's empty Path Report now says "None of the songs in this run could be analyzed, so there is no report to show." The first D97 question gave a wrong premise (that the message showed before a first scan). The som-c agent caught it, and the user decided again with the real rule. Results with no chart library now show the same reason in the GUI as in the CLI.

The view-progress-delay UI test no longer half skips. Both held clicks watch for the box from the first frame, and the second checks that a new request restarts the delay. "Is this chart in the library" has one owner, `report::library_lists`, which the DM page uses too. "Does the store hold results" has one owner, `holds_results`. `serialize.*` and `Chord::from_code` are gone, and `Timecode::raw` stays because about 20 tests need it. A scan test now pins the UserGuide's quote of the D94 line.

The original list follows, for the record.

- **"No records stored yet" on a database with no library.** The GUI report job shows that message there. A GUI database always has a library after a scan, so the user can't hit it.
- **The naming-copy join relies on SQLite's automatic column name.** `RecordStore::naming_copy_paths` joins on `c."MIN(rowid)"`. An alias in `kNamingCopiesSql` would be sturdier.
- **One UI test half skips on a busy machine.** The "no box" half of the `view-progress-delay` test skips there, and logs that it did.
- **A touched file is re-hashed on every click.** If a chart file's modified time changes but its content doesn't, every click re-hashes it until the next rescan, because the stored signature isn't refreshed. The Preview already behaved this way.
- **report.cpp asks one question twice.** It checks "does the store hold any results" twice in one flow.
- **One rule, two spellings.** The rule "this chart is in the library" is written with two different maps, in `collect_rows` and in `records_by_hash`. The two should share one owner.
- **serialize.\* could go entirely.** `rules_fp_bytes` is now the only user of `BinaryWriter`, and only its `u64`. Calling `core::append_le_u64` directly would write the same bytes and let `serialize.{h,cpp}` go.
- **Code that only tests use.** `Timecode::raw` now has callers in tests only. `Chord::from_code` may be unused too, but nobody has checked.
- **The UserGuide quote isn't pinned.** The scan rows don't cover docs, so no test checks the UserGuide's quote of the D94 line against the code.
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
