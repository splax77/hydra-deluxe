Agent ad0fd529105bd0b9a (task T5-V-DL3, session e633f7f6-f313-4787-b622-8d901b577b64). Read-only; nothing built or run. Everything below is "by reading" unless it says otherwise. No record names a run, and I found none in the records folders for these tests.

# Verify DL3: records/delta-DL3.md

Code read at `5ec682c7` (the worktree tests and src are byte-identical to it: `git diff 5ec682c7 HEAD -- tests src` is empty).

## 1. Scope

I ran `git diff c10c7f54 5ec682c7` on the four files and listed the test cases myself.

- `tests/test_app_click.cpp`: 11 new, none changed, none removed. Matches the record.
- `tests/test_app_library.cpp`: 14 new. Two changed through `run_redo_batch_over` (`:83`, `:107`). Four changed through `let_through` (`:249`, `:270`, `:280`, `:303`, all through `BatchAfterTwoCharts` except `:249`, which calls `let_through` itself). `gated_scores` did not change (the hunk at `:214` is `let_through`'s comment). 20 in scope. None removed.
- `tests/test_app_reports.cpp`: 7 new. Three changed through `run_redo_batch_over` (`:272`, `:793`, `:816`). 10 in scope. None removed.
- `tests/app_state_fixtures.h`: `stored_record_is_empty` new, `run_redo_batch_over` changed.

41 test cases in scope, 41 records, plus the 2 helper records. No in-scope test case lacks a record. No record is for an unchanged test. Every record has its "Delta round" line, and each changed one names an old record that exists (A6 records 3, 4, 6, 7, 8, 9; A7 records 9, 22, 23). I opened A6's and A7's verdict lines for those and they match what the records say.

A fault in every record: the line numbers inside the bodies do not match the file. The `TEST_CASE` line is right, but body lines are off by 2 to 12 (click test 1 cites 576/577/585/587/588; the real lines are 564/565/568/572/573. Library test 12 cites 351-353; real 353-355. Reports test 32 cites 966/968/970; real 964/967/969). Tests 3 and 4 are exact. A reader following the cited lines lands on the wrong line. The claims still hold; only the pointers are wrong. I give the real lines below where it matters.

## 2. Part 1: test_app_click.cpp (11)

**1. a click's save does not stop the window while a save group is open (`:549`).** Verdict Investigate (claim not checked only): agree. The lock part is real: `HeldSaveGroup` (`tests/store_file_fixtures.h:35-82`) runs `begin_save_group` on a helper thread against the real store file, and the job is a real thread. Per line: `:564` `held.held()` is the load-bearing line. If the window waited, the helper's own wait times out at `kWaitCap`, sets `held_` false, and `:564` goes red. The record's wording for q8 on this line is muddled; this is the reason. `:565` (state Analyzing) is weak. The gate is released at the end of the `SaveGate` scope and `tick(0.0)` follows at once, so Analyzing also holds if the job simply has not run yet. No line proves the job's save really waited on the group; a job that saved without the lock would still pass. `:568`, `:572`, `:573` (Ready, row Ready, row label equals a second app's) catch a row left behind by the save. Q2 holds: `ViewJob::write_results` is the only writer, called only from `run` (`library_jobs.cpp:737`).

**2. a click's worker writes a changed chart's hash before it saves (`:578`).** Record verdict Rewrite (the bare `Sleep(20)`). I dispute it. The sleep is not load-bearing. The fingerprint is `size:mtime` of the notes file plus the ini (`src/app/analysis.cpp:228`), and `chart_changed_since` compares it (`:350-356`). The extra note adds 11 bytes (`app_state_fixtures.h`, `text.insert(open + 1, "  0 = N 4 0\n")`), so the size alone shows the edit, and the ini is rewritten as well. The record left this open ("`chart_changed_since` ... not opened"); I opened it. So the test does not lean on the clock, and no other fault is named. The claim is about the store (new hash first, no orphan) and was checked on a real corpus chart edited by one line, against the store's own `list_chart_library_copies` and `lookup_one`: that meets the brief's rule for store claims. Each assertion fails if the hash is written after the save: `:594` (new hash has 1 copy at the pre-save check) goes red, since at that moment nothing would be under the new hash. `:595` (old hash gone), `:596` (nothing saved yet), `:601` (Ready under the new hash), `:603` (panel's entry on the new row). Should be Keep, with the `Sleep(20)` deleted as housekeeping. If the main session reads the sleep as a timing fault anyway, Rewrite is the most it can be.

**3. a click cancelled before its save saves nothing (`:607`).** Rewrite (drop `:617`): agree. `:618` counts rows in the file with raw SQL (`record_rows_in`); the store starts empty, so `:617` (status unchanged) cannot fail unless `:618` does. `:616` fails if the panel does not follow the cancel. Real thread, real seam (`library_jobs.cpp:778-779`), real corpus chart.

**4. closed before its save (`:621`).** Rewrite (drop `:632`): agree, same reason. `close_details` sets the panel to None (`app_state.cpp:~370`), so `:631` is right.

**5. replaced before its save (`:636`).** Investigate (panel capture): agree. `start_view` cancels the running job (`app_state.cpp:392`) and queues the new one. At the real lines: 651 (second song selected), 652 (first song's store status unchanged) and 653 (its row status unchanged) fail if the old job saves. The record cites them as 652-654.

**6. let go after it wrote (`:658`).** Investigate: agree. Expected side is the store (`lookup_one`), compared through `LibraryRow::shows_same_as` (`src/ui/library_model.cpp:61-69`), which compares status, stale flags, best path, score, stars, hardest ms and failure kind and text item by item. I opened it; the record did not. `start_view`'s let-go path (`app_state.cpp:387-390`) calls `take_view_outcome`, which I read. I did not open `update_view_job`'s call for the first SUBCASE, so that half is by the record's cite only.

**7. cancelled during its save (`:697`).** Investigate: agree. `update_view_job` turns a finished-but-cancelled job into Cancelled (the line `if (outcome == JobOutcome::Done && job->is_cancelled()) outcome = Cancelled;`, which I read). Deleting it turns `:704` red.

**8. closed during its save (`:708`)** and **9. replaced during its save (`:720`).** Investigate: agree. Same helper, `check_row_shows_saved` (`:687-691`).

**10. a dropped click that wrote a new hash still reloads the library (`:739`).** Record verdict Rewrite (same `Sleep(20)` as test 2). I dispute it for the same reason as test 2. The comment on `:743` ("the mtime has to move for the fingerprint to see the edit") is itself wrong, because the size changes. `take_view_outcome` calls `reload_library()` when `wrote_hash` (`app_state.cpp:~524`). Delete that and `row_of(new_md5)` finds no row and fails the test (the helper `FAIL`s). So `:753` (real `:751`) and the helper catch a missing reload. Should be Keep, with the sleep and its wrong comment removed.

**11. a dropped click's save marks both reports out of date (`:758`).** Investigate (claim not checked only): agree. The record's split is real and I confirmed it: A5 record 3 (Keep), A7 record 23 (Rewrite), this record (Investigate) all read a flag (`out_of_date`, `report_started`) instead of the strip's words. New fact: A5 record 3's owner cite, `save_view_record` at `app_state.cpp:571`, no longer exists (`grep save_view_record src` finds nothing). After commit `d664ac4d` the mark happens in `take_view_outcome`. A5 record 3's Keep rests on an owner that moved; its test body did not change so it has no delta record. Goes to the main session with the split.

## 3. Part 2: test_app_library.cpp

**12. a tick during a batch does not wait for the store (`:346`).** Rewrite (no typed "1,000"/"2,000"): agree. `returns_while_held` (`tests/store_lock_holder.h:61-67`) is a real check: if the call waited it would run to `kWaitCap`, and `let_go()` would then be true, so it returns false. The typed text is the test's own analyzer printed by the formatter (`gated_scores`, `1000 * turn`). One worker (`start_redo_batch` passes `1`, `app_state_fixtures.h:190`), so the order is deterministic. Real lines 353-355. Deleting `set_summaries_for` from the tick turns `:354`/`:355` red.

**13. a tick during a batch re-reads no row the batch did not save (`:360`), and 14. a batch's end re-reads no row the batch did not save (`:372`).** Investigate: agree with the verdict, but the record's facts need replacing. The record says the plan could not be found and "no decision I opened says so". The plan exists: `docs/superpowers/plans/2026-11-15-batch-rows-without-reread.md` at commit `7cd9de67f8d0128758c9f1d242654833e5c2749d` (branch `claude/handoff-batch-rows`; not on this branch). Its Question 1 is exactly this case: "Results that another program writes during a window batch ... After the change that row keeps saying 'Not analyzed' until the next whole read." The real case is `hydra_batch` run from a terminal on the same database (`src/cli/batch.cpp:122`). The record guessed a click saving during a batch; that case is handled elsewhere (a click's save refreshes its own row through `take_view_outcome`). The handoff at that commit (`docs/handoffs/2026-11-15-batch-rows-execute-handoff.md`, line 21) quotes the user: "restate the question in plain english and clarify if this is something that can actually happen or it's an insane edge case you invented". It says the main session called it an edge case and the plan stands, the user did not object, and it is "not a filed ruling". So: a plan question closed by the main session, no filed ruling. The labels O4, O7, O13, O22 are rows of that plan's owner map. Real lines for 13: 368, 369; for 14: 387, 388.

**15. a batch that fails as a whole still re-reads every row (`:393`).** Investigate: agree. `finish_failed` sets `run_error` (`library_jobs.cpp:468-471`); the tick's whole read for a failed run is at `app_state.cpp:~288`. The scenario (pool of zero workers) is a forced way in. I did not look for a real cause of a whole-run failure, so "no real case" is as unverified as the record's.

**16. a lookup the batch could not make shows the store's problem (`:412`).** Investigate (claim not checked only): agree. The record also calls the equality of two sentences a Q9 stand-in. Both sentences come from `app::plain_error`, which is the owner, and the claim is exactly that they match, so it is not a stand-in; the record's verdict is consistent with that. I read the rename seam: `ALTER TABLE records RENAME` through `exec_on_file` is a real file change.

**17. a chart in two folders gets both rows from one batch result (`:451`).** Investigate: agree. The typed "7,000" is the test's own `scored_record(7000)`.

**18. a failed chart whose failure the batch stored (`:483`).** Investigate: agree. Side note holds (comma in the name). Lines 510 and 511 (not-kept row unchanged, equals a second app) say the same thing when the store holds nothing for it.

**19. a paused batch (`:514`)** and **20. a stopped batch (`:529`).** 19 Investigate, 20 Rewrite with A6 record 6: agree. `batch_refreshed_at_` starts at -1.0 (`app_state.h:955`), so the first tick at 0.0 does take the tray.

**21. opening a song does not wait for a save (`:1242`).** Keep: agree, with two notes. (a) The record's owner is wrong: `select` reads through `read_entry` (`app_state.cpp:195-201`, line 197); `:613` is `select_chart` (test 22). The caller list (197, 205, 217, 235, 613, 667, 669, 874) is right. (b) The lock claim is real (real helper thread, real lock, real file). The four equalities at `:1253-1256` compare the selected entry with the store's copy of a hand-built `library_entry(5)` row. They fail only if `select` stops taking the store's copy. If the main session reads the brief's file/store rule strictly (real input needed), the data part rests on a hand-built fixture and 21 and 22 would be Investigate (claim not checked only). I read the claim in the name as the lock, and agree with Keep.

**22. select_chart on a hidden chart (`:1261`).** Keep: agree, same notes as 21. Caller: `select_chart` at `app_state.cpp:~606-620`; I read `:613`.

**23. opening the batch confirm does not wait for a save (`:1277`).** Investigate: agree. The test's own comment says the confirm cannot open during a batch. The second-app comparison of the two `batch_question` answers (`:1287-1288`) is a real check of the confirm's questions apart from the lock.

**24. a settings change re-reads the rows without waiting (`:1293`).** Investigate: agree. Same unreachable case by the test's comment. `REQUIRE(... != seeded_before)` at `:1303` shows the change reaches a row, so the loop is not vacuous.

**25. a failed reader read shows the store's problem (`:1312`).** Investigate (claim not checked only): agree, with one correction. The record says the failure is "not a real broken file". The seam opens a path that does not exist (`path_ + "-not-there"`, `record_store.cpp:1972-1975`), so SQLite raises its own error as a real failed open would.

**26. (`:83`) batch result for the open chart refreshes its row, not the panel.** Changed through `run_redo_batch_over`; old A6 record 3 Investigate. Still holds: `app_on` stores no record for chart 5, then the test stores the empty one with `save_batch_record`; `stored_record_is_empty` is true, so the batch files the empty analysis, as the old helper's stored nothing/threw did. Agree: Investigate (split with D48 Q16). I did not open D48 Q16 or "Ruling 14".

**27. (`:107`).** Changed; old A6 record 4. Agree Investigate, same split.

**28-31. (`:249`, `:270`, `:280`, `:303`).** Changed through `let_through`. Agree Rewrite (with A6 record 6). The helper change only makes the setup wait for the tray; nothing the tests assert changed.

## 4. Part 3: test_app_reports.cpp

**32. a path report builds while a save group is open (`:952`).** Investigate (claim not checked only): agree. The load-bearing line is `wait_path_report_job` (a waiting job never finishes and the wait times out); `:964` `group.held()` holds by construction once the job finished. Real lines 964, 967, 969. `ReportJob::run` opens its own connection (`library_jobs.cpp:824`).

**33. a DM report (`:972`).** Investigate: agree. The leaderboard is a canned answer, not WinHTTP, so nothing here checks the transport. It is not claimed to.

**34. a path report reads one moment (`:994`).** Investigate: agree. Non-vacuous: `REQUIRE(saved)` and `REQUIRE(...paths.empty())` at `:1015-1016` prove the write landed in between, and the page must still equal the earlier one. Deleting the hold in `ReportJob::run` makes the page differ and turns `:1019` red. The seam runs only on the job's thread (`:917`).

**35. a path report's separate store calls read one moment (`:1022`).** Investigate: agree. Without the hold `record_count` reads 1 after the save, `generate_report` gives a `why_empty` sentence, and `:1051` goes red.

**36. a report whose connection cannot open (`:1056`).** Investigate: agree. The same seam as test 25, so the failure is a real failed open.

**37. a finished report holds no connection (`:1084`).** Keep: agree, with a narrowing. The record's open question is answered. `~BatchWrites` runs `PRAGMA wal_checkpoint(TRUNCATE)` (`record_store.cpp:1113`), and its own comment says a checkpoint cannot finish while another connection is reading. The input is not hand-built: `app_with_result` (`test_app_reports.cpp:76-82`) clicks a real corpus chart, so the stored record is the engine's own. That also answers the record's open question about tests 39-41. What the test catches is a leaked read transaction (a leaked hold). A leaked idle connection with no read open would not stop the truncate, so the name's "holds no connection" claims more than the check proves. Keep stands; the name overreaches by that much.

**38. a cancelled report closes its connection (`:1097`).** Record verdict Investigate, because the cancel arrives before the connection exists. I dispute the reason. `hold_()` is at `library_jobs.cpp:818`, and the connection opens at `:824`, after the seam is released. So the cancelled job does open its connection, runs `generate_report` with the cancel flag set, then closes it. `check_log_empties` shows the cancel path leaves no read open. What is not tested is a cancel in the middle of a read. Same footing as test 37, so the verdict should be Keep (with the mid-read cancel as an untested neighbour), not Investigate.

**39. (`:272`).** Changed; old A7 record 9 Investigate. The record stops at "unconfirmed" on what `app_with_result` stores. It stores the engine's own record (it clicks a real corpus chart). So `stored_record_is_empty` is false, and the helper now runs `analyze_chart_file` on the real chart file. The batches at 309 and 325 therefore run the real engine on a real file. Old verdict stands: Investigate (seven claims in one test; the screen strip not captured).

**40. (`:793`).** Changed; old A7 record 22 Investigate. `app_on` seeds a record only for chart 0, so chart 5 is NotAnalyzed, the helper takes the empty branch, and the batch files the empty analysis. Record's reasoning holds (I read `:793-815` and the `app_on` call). Investigate stands.

**41. (`:816`).** Changed; old A7 record 23 Rewrite. `app_with_result` again: the batches run the real engine. The test reads `report_started` and the build state, so the Rewrite reason is unchanged. Agree.

## 5. Part 4: helpers

**H1. `stored_record_is_empty` (`app_state_fixtures.h:203-214`).** Does what its callers rely on: true when the chart has no Ready record, or exactly the empty analysis; false for the engine's own record. Only `run_redo_batch_over` calls it. The record's "not confirmed" (what `app_with_result` stores) is settled above: the engine's record, so the false branch is taken by tests 39 and 41 and the true branch by 26, 27, 40.

**H2. `run_redo_batch_over` (`:222-235`).** Agree with the record. One extra point: the `empty` flag is read before the batch starts, from the chart at row 0 after the search. Callers 26, 27, 39, 40, 41 are the only ones (checked with `git grep` over `tests`, `src`, `tools` at `5ec682c7`).

**`let_through`** (library, `:222-230`). Waits for `completed == charts && answered_count() == charts`. Correct for its callers; `answered_count` is read under the job's lock (`library_jobs.cpp:499-502`).

## 6. Counts

41 records. Agree with the verdict on 38. Dispute 3:
- Test 2 (Rewrite should be Keep: the sleep is dead).
- Test 10 (same).
- Test 38 (Investigate rests on a wrong reason; Keep).

Agreeing verdicts whose facts I correct: 13 and 14 (plan found), 21 (wrong owner cite), 25 (seam is a real failed open), 39 (settled), 37 (name overreaches).

## 7. Splits (one per line)

1. A5 record 3 (Keep), A7 record 23 (Rewrite) and DL3 click test 11 (Investigate) judge the same kind of line, a flag read instead of the strip's words. A5 record 3's owner (`save_view_record`, `:571`) is gone.
2. Tests 13 and 14 pin that a result saved by another program is not shown until the next whole read. The plan's Question 1 asked the user; the main session closed it on the user's words "insane edge case you invented"; the handoff says not a filed ruling. The record said no source was found; there is one.
3. Test 15 against 13 and 14: a failed whole run reads every row at its end; any other run reads none. Two opposite outcomes of the same tick, in three tests.
4. Test 26/27 and D48 Q16 versus "Ruling 14" (carried from A6, not re-opened by me).
5. Test 25 (reader) and test 16 (tray) and test 36 (report) each compare a sentence with `plain_error`'s output. No split; one owner. Listed because the record flagged them.
6. Test 23, 24 versus 21, 22: the same reader rule is proven on reachable (21, 22) and unreachable (23, 24) paths, with the unreachable ones graded Investigate. Same-rule consistency check, not a disagreement.
7. Test 37 name ("holds no connection") versus what it checks (no open read), see above.
8. Missed by the auditor: none other found.

## 8. Rewrites that propose a saved expected-output file

None. The proposed rewrites read the store (`lookup_one`), compute the order from the scores the test assigned, or drop a line. None proposes a golden file (`docs/agents/rulings/code-and-engine.md:50`).

## 9. Not reached or not confirmed

All 41 records and both helpers were reached. Not opened by me: `update_view_job`'s drop path for the first SUBCASE of test 6, D48 Q16, D87 item 11, D115 question 4, "Ruling 14", the store's own "a reader holds nothing open between reads" test, and a real cause of a whole-run failure (test 15). Question 3 captures still needed for every row/panel/strip/status-line claim: the Library row and details panel on a real corpus chart around a click and a batch, the path and comparison report windows (page, out-of-date line, failure sentence), and the status line on a failed store read.
