# Delta DL3: tests/test_app_click.cpp, tests/test_app_library.cpp, tests/test_app_reports.cpp, tests/app_state_fixtures.h

Task T5-DL3, agent a600ff6b451c6ed6a, session e633f7f6-f313-4787-b622-8d901b577b64. Read-only; nothing was run or built. Diff base `c10c7f54`, tip `5ec682c7`. Old records: `records/A5.md`, `A6.md`, `A7.md`.

Scope found (from `git diff` of each file): test_app_click.cpp 11 new tests, 0 changed, 0 removed. test_app_library.cpp 14 new, 6 changed only through a helper (2 through `run_redo_batch_over`, 4 through `let_through`), 0 removed. test_app_reports.cpp 7 new, 3 changed only through `run_redo_batch_over`, 0 removed. app_state_fixtures.h 2 changed helpers (`stored_record_is_empty` new, `run_redo_batch_over` changed). The library file's `let_through` and `BatchAfterTwoCharts` are helpers inside the test file and they changed too. No test case was removed in any file (the diffs show only added lines and include/comment lines).

## What every record shares

**Read, common.** Each test's body, from the diff hunks at `5ec682c7` and the file lines named in the record. `ViewJob::run` and `ViewJob::write_results` (`src/ui/library_jobs.cpp:686-791`). `BatchJob::take_saved_rows`, `answered_count`, `note_saved`, `answer_waiting` (`library_jobs.cpp:492-543`). `ReportJob::run` (`library_jobs.cpp:808-848`). `AppState::tick_library` (`src/ui/app_state.cpp:256-298`), `close_details`, `start_view`, `launch_view_job`, `cancel_view`, `update_view_job`, `take_view_outcome` (`app_state.cpp:356-533`). `write_corpus_chart`, `corpus_chart`, `app_with` (`tests/app_state_fixtures.h:96-126`). `HeldSaveGroup` (`tests/store_file_fixtures.h:35-82`) and `returns_while_held` (`tests/store_lock_holder.h:57-67`). `record_rows_in` (`tests/db_file_util.h:38`, a raw SQL count over the file). D87 items 1 to 3 (`docs/audit/2026-10-03-fix-decisions.md:448-452`). Old records A5, A6, A7 (headers and the records named per test).

**Not read, common.** The base file lines of `tests/app_state_fixtures.h` outside the diff hunks (the changed helpers were read as diff lines, and lines 96-126 whole). `tests/store_file_fixtures.h` and `store_lock_holder.h` bodies beyond the lines above (those files belong to slice DL2). The store's `reader()`, `open_own_reader`, `OwnReader::hold`, `lookup` and `save_group` bodies (`src/store/record_store.*`). `LibraryRow::shows_same_as` body (`src/ui/library_model.cpp:61`, found by grep, not opened). D87 item 11 and D115 question 4 (the test comments cite them; D87's list was opened to item 3 only, D115 at `fix-decisions.md:704` not opened beyond its first sentence). The batch-rows plan the tests cite as "O4, O7, O13, O22" and "question 1 of the batch-rows plan": no file under `docs/superpowers/plans/` matching that name was found by `ls | grep -i "batch-row|reader|own-conn"`, so those labels are claims with no source opened. The adding commits: only the subjects in `git log c10c7f54..5ec682c7` were read (named per record).

**Question 3 for this file.** The engine's record (`records-engine/`) holds activations, squeezes and fills; none of these tests is about a chart's play. Two kinds of claim are here. A claim about threads, locks and what a worker saves (the click's job, the batch's tray, a report's own connection) is checked on the real mechanism when it runs the real thread pool, a real helper thread holding the real store lock, and a real SQLite file. Its real input is a real corpus chart: `write_corpus_chart` copies `corpus::first_chart_with_suffix(".chart")` into a temp folder (`app_state_fixtures.h:96-110`), and `extra_note` inserts one note line into it. A claim about what a row, a panel or a report window shows is a screen claim, and no capture exists of the Library table, the details panel or the report windows (`records-screen/index.md` says the path report is not captured; the Library and panel are not in it either). Those say "claim not checked", and the capture needed is "the Library table row and the details panel on a real corpus chart, before and after a click or a batch result".

**Question 6 for this file.** Where a test compares a row with a second app reading the same file, or with `store->lookup_one`, the expected side is the store (the owner of the stored answer), read item by item. Where a test reads `row.best_label()` against typed text such as "1,000", the typed text is a raw result (see the library records).

---

## Part 1: tests/test_app_click.cpp (11 new)

Shared answers for Part 1, repeated by number in each record only where they differ.
Question 2: `ViewJob::write_results` is called only by `ViewJob::run` (`library_jobs.cpp:737`). `ViewJob` is built only in `AppState::launch_view_job` (`app_state.cpp:412`). `take_view_outcome` is called at `app_state.cpp:389` and `:461`. The seams `set_view_before_save_check_for_test` and `set_view_after_save_check_for_test` are copied by each job at construction (`library_jobs.cpp:672-673`) and called at `:778` and `:782`. The engine owns the rule (the worker saves its own result; a cancel seen before the save stops it); the window follows the store (`take_view_outcome` refreshes the row from the store and marks both reports out of date). This is not a display rule invented by a display: the row is refreshed by `refresh_library_row`, which reads the store.
Question 7 for this file: the commits `d664ac4d` "The click's worker saves its own result" and `8f59f9cb` "A click cancelled, closed or replaced during its save gives the row the store's answer" added these. Subjects only were read. The failure named from the code: before `d664ac4d` the window thread saved the click's result, so a held batch save group froze the frame; and a click dropped by the user could leave the row out of date with the store.

### 1. a click's save does not stop the window while a save group is open (tests/test_app_click.cpp:549)
Delta round: new
Read: common; lines 549-576; `SaveGate` (479-525 region, from diff); `HeldSaveGroup`.
1 Claim: while a batch holds the store's save group, a click's result waits for it in the worker, the window's frame comes back at once and the panel stays on Analyzing; afterwards the row shows what a second app on the same file shows.
2 Owner: the job's thread writes (`library_jobs.cpp:751-791`), so the window never waits. See shared Q2.
3 Real chart: Engine: not an engine fact. Claim: the thread and lock part is checked on the real mechanism (a real helper thread holds the real store's save group on a real SQLite file, `store_file_fixtures.h:38-65`; the real click job runs on a real thread) with a real corpus chart as input. The part about what the panel and the row show: claim not checked (needs the Library row and details panel capture named above).
4 Direct: 576 `held.held()` after the tick: the helper still held the group when the frame returned (a property). 577 state is Analyzing (a state field). 585 REQUIRE ready. 587 the row is Ready. 588 the row's `best_label()` equals a second app's for the same chart (a property against the store's answer, read item by item).
5 Reason: no typed numbers. The `0.0` tick time is an input.
6 Display: row compared with a second app's read of the same file, not a typed expected value. Good. The panel's state is read as a field, not as the panel's text.
7 Failure: a window that saves on its own thread freezes while a batch holds the group (shared Q7). Still possible.
8 Deleted: 576 (held) removed: a tick that waited would still pass if the wait were shorter than the helper's hold (reasoned: the helper is held until `let_go`, and a tick that waited would block until `kWaitCap` and fail from the timeout, so the line mostly guards a tick that waits and then returns after the cap). 577: a frame that collected the job early. 587, 588: a row left behind by the save.
9 Stand-in: none. Leans on timing: `wait_until` with `kWaitCap`.
10 Same rule: `tests/test_app_library.cpp:346` "a tick during a batch does not wait for the store" is the same promise for the batch tick; test 1 here is the click's.
11 Real: yes; a player clicks during a batch.
Verdict: Investigate (claim not checked only). It needs a capture of the details panel and Library row on a real corpus chart during a batch save group. Every other answer holds.

### 2. a click's worker writes a changed chart's hash before it saves (tests/test_app_click.cpp:578)
Delta round: new
Read: common; lines 578-606; `ViewJob::write_results` 751-764; D87 item 3 (`fix-decisions.md:450-452`, first words only, item 3 text cut at "updates that chart's").
1 Claim: when the chart file was edited after the scan, the click's worker gives the library row the new hash before it saves, so nothing is ever saved under a hash no row has; the panel's entry ends up on the new row.
2 Owner: `ViewJob::write_results` (`library_jobs.cpp:758-762`, `store_.reidentify_chart`). See shared Q2.
3 Real chart: Engine: not an engine fact. Claim: checked on a real corpus chart edited by one added note, against the store's own `list_chart_library_copies` and `lookup_one` (independent of the job) and with the real thread at the seam. The row and entry text: claim not checked.
4 Direct: 592 `list_chart_library_copies(new_md5).size() == 1` and 593 `(scanned.md5).empty()` at the pre-save check: the row is under the new hash and not the old (property of the store). 594 `lookup_one(new_md5)` is NotAnalyzed: nothing saved yet. 598 ready. 599 `Ready` under the new hash. 601 `selected->md5 == new_md5`. 602 row Ready.
5 Reason: `Sleep(20)` (line 582) has no stated reason here; a later test's comment (line 739 region) says "the mtime has to move for the fingerprint to see the edit". The fingerprint also depends on size, and the added note line changes the size; whether the 20 ms is needed was not read (`chart_changed_since`, `src/app/analysis.cpp`, not opened). 20 is an input with no working, and it is a timing the test leans on.
6 Display: stores and rows compared with the store.
7 Failure: D87 item 3 (the edited chart is re-hashed on click): the job saved under the old hash and left an orphan. Still possible.
8 Deleted: 592 (hash not written first); 593 old hash still there; 594 a save made before the check; 601 the panel's entry staying on the old row.
9 Stand-in: none. Leans on a real clock tick (Sleep 20).
10 Same rule: test 10 (line 739) repeats the hash-before-save promise from the dropped-click side.
11 Real: yes; a user edits a chart and clicks it.
Verdict: Rewrite. Replace the bare `Sleep(20)` with a stated cause (set the file's time explicitly, or drop it if the size change alone shows the edit), so the test does not lean on the clock. Today it catches the hash-first order (kept).

### 3. a click cancelled before its save saves nothing (tests/test_app_click.cpp:607)
Delta round: new
Read: common; lines 607-620.
1 Claim: a click the user cancels before its worker's last cancel check saves nothing, and the panel reads Cancelled.
2 Owner: `ViewJob::write_results` `throw_if_cancelled` (`library_jobs.cpp:779`); `cancel_view` (`app_state.cpp:418-430`). See shared Q2.
3 Real chart: Engine: not an engine fact. Claim: checked on the real thread at the real seam, with a real corpus chart, and the empty store checked by a raw SQL count on the file (`record_rows_in`, independent of the store API).
4 Direct: 616 state Cancelled; 617 `lookup_one` status equals the status before; 618 no rows in the database file.
5 Reason: 0 is zero.
6 Display: n/a beyond the state field.
7 Failure: the job saves even though the user cancelled. Still possible.
8 Deleted: 616 a panel that does not follow the cancel; 618 a save. 617 is covered by 618 (a saved record is a row), so 617 catches nothing 618 does not (reasoned).
9 Stand-in: none.
10 Same rule: test 4 and test 5 (the close and replace cases, same seam).
11 Real: yes.
Verdict: Rewrite. Drop line 617; line 618 is the check. Today it catches: a save after a cancel (kept) and a panel that does not read Cancelled (kept).

### 4. a click closed before its save saves nothing (tests/test_app_click.cpp:621)
Delta round: new
Read: common; lines 621-635.
1 Claim: a click closed (details panel closed) before the last cancel check saves nothing, and the panel reads no song.
2 Owner: `close_details` cancels the job (`app_state.cpp:366`). See shared Q2.
3 Real chart: as test 3.
4 Direct: 630 `show_details` false; 631 state None; 632 status unchanged; 633 no rows.
5 Reason: none typed.
6 Display: fields.
7 Failure: a closed click's result still saved.
8 Deleted: 630, 631: close not recorded; 633 a save. 632 is covered by 633 (reasoned).
9 Stand-in: none.
10 Same rule: tests 3 and 5.
11 Real: yes.
Verdict: Rewrite. Drop line 632. Today it catches a save after a close (kept).

### 5. a click replaced before its save saves nothing (tests/test_app_click.cpp:636)
Delta round: new
Read: common; lines 636-657.
1 Claim: when a second song is clicked before the first job's last cancel check, the first saves nothing, and the panel shows the second song.
2 Owner: `start_view` cancels the running job (`app_state.cpp:392`) and queues the new one (`view_pending`). See shared Q2.
3 Real chart: real thread at the real seam; the two charts are real corpus copies with different hashes (the second has one added note). Checked against the store (`lookup_one`) and the row; panel text: claim not checked.
4 Direct: 650 REQUIRE the panel is ready; 652 `selected->md5 == next.md5`; 653 the first song's stored status is as before; 654 the first song's row status is as before.
5 Reason: none typed.
6 Display: the row compared with the status read from the store before.
7 Failure: an older click saving after a newer one. Still possible.
8 Deleted: 653 an older click's save under its hash; 654 the row picking up a save that was not made.
9 Stand-in: none.
10 Same rule: tests 3 and 4.
11 Real: yes; clicking another song quickly.
Verdict: Investigate (claim not checked only): the panel showing the second song needs a details-panel capture; the rest holds.

### 6. a click whose job was let go after it wrote still gives the row the store's answer (tests/test_app_click.cpp:658)
Delta round: new
Read: common; lines 658-696 with its two SUBCASEs; `update_view_job` and `start_view` (`app_state.cpp:381-409, 456-520`).
1 Claim: when a click's job has finished and saved, and the user then closes the panel or clicks another song before any frame collects it, the library row still says what the store now says.
2 Owner: `take_view_outcome` (`app_state.cpp:522-533`) is called both in `start_view` (:389) and `update_view_job` (:461), so the row follows what the job wrote even when the result is dropped.
3 Real chart: real thread, real corpus chart, row compared with `store->lookup_one` (the owner of the stored answer), item by item through `LibraryRow::shows_same_as`. Row text on screen: claim not checked.
4 Direct: 692 the stored status is Ready (the job saved); 693 `row.shows_same_as(found)` (the row equals the store's answer).
5 Reason: none typed.
6 Display: expected side is the store's record, read now. Good (shape of the rubric).
7 Failure: the row staying NotAnalyzed after a dropped click saved. Still possible.
8 Deleted: 692 the job saving at all; 693 the row following. The two SUBCASEs reach two different drop paths (`update_view_job`'s generation drop and `start_view`'s let-go).
9 Stand-in: none.
10 Same rule: tests 7 to 9 (same rule while the job is mid-save).
11 Real: yes.
Verdict: Investigate (claim not checked only): needs the Library row text capture. Every other answer holds. `shows_same_as` was not opened (see common).

### 7. a click cancelled during its save keeps the panel cancelled and gives the row the store's answer (tests/test_app_click.cpp:697)
Delta round: new
Read: common; lines 697-707; `check_row_shows_saved` (the helper at the file's hunk, lines 685-695).
1 Claim: a Cancel that arrives after the job's last check cannot stop the save; the panel reads Cancelled and the row says what the store says.
2 Owner: `update_view_job` line 478 turns a Done-but-cancelled job into Cancelled; `take_view_outcome` refreshes the row. See shared Q2.
3 Real chart: as test 6.
4 Direct: 704 panel state Cancelled; then `check_row_shows_saved`: stored status Ready, `shows_same_as`.
5 Reason: none typed.
6 Display: the row against the store. Good. The panel's Cancelled is a state field.
7 Failure: a late Cancel leaving the row stale or the panel Ready. Still possible.
8 Deleted: 704 the panel following the cancel; the helper's two lines the row.
9 Stand-in: none.
10 Same rule: tests 6, 8, 9.
11 Real: yes.
Verdict: Investigate (claim not checked only): details panel and row capture.

### 8. a click closed during its save gives the row the store's answer (tests/test_app_click.cpp:708)
Delta round: new
Read: common; lines 708-719.
1 Claim: closing the panel after the job's last check cannot stop the save; the panel is closed and the row says what the store says.
2 Owner: as test 7 (`close_details`, `take_view_outcome`).
3 Real chart: as test 6.
4 Direct: 715 `show_details` false; 716 state None; then the helper's two checks.
5-6 as test 7.
7 Failure: as test 7.
8 Deleted: 715, 716 the close; helper lines the row.
9 Stand-in: none.
10 Same rule: tests 6, 7, 9; test 4 is the pre-save close.
11 Real: yes.
Verdict: Investigate (claim not checked only): row capture.

### 9. a click replaced during its save gives the row the store's answer (tests/test_app_click.cpp:720)
Delta round: new
Read: common; lines 720-738.
1 Claim: a second click after the first job's last check lets the first save go through; the panel shows the second song and the first song's row says what the store says.
2 Owner: as test 5 and 7.
3 Real chart: two real corpus copies with different hashes; row against the store.
4 Direct: 733 REQUIRE ready; 735 `selected->md5 == next.md5`; the helper on the first song.
5-6 as above.
7 Failure: the first song's row stale after it was replaced.
8 Deleted: 735 panel showing the older song; the helper the row.
9 Stand-in: none.
10 Same rule: tests 5, 7, 8.
11 Real: yes.
Verdict: Investigate (claim not checked only): panel and row capture.

### 10. a dropped click that wrote a new hash still reloads the library (tests/test_app_click.cpp:739)
Delta round: new
Read: common; lines 739-757; `take_view_outcome` line 524 (`if (out.wrote_hash) reload_library()`); D87 item 3.
1 Claim: if a click was closed after its worker wrote a new hash for an edited chart, the library still reloads to the new hash, and the row says what the store says.
2 Owner: `take_view_outcome` (`app_state.cpp:524`). See shared Q2.
3 Real chart: a real corpus chart edited by one note, row against the store (`list_chart_library_copies`, `lookup_one`). Row text: claim not checked.
4 Direct: 752 state None; 753 `list_chart_library_copies(new_md5).size() == 1` (REQUIRE); the helper: stored Ready and the row equals the store (`row_of` fails the test when no row carries the new hash).
5 Reason: `Sleep(20)` at line 744, the comment says "the mtime has to move for the fingerprint to see the edit". The need for 20 ms is not shown (shared with test 2).
6 Display: store-compared. Good.
7 Failure: a dropped click leaving the library without the new hash.
8 Deleted: 753 a missing reload; helper lines the row.
9 Stand-in: none. Leans on the clock (Sleep 20).
10 Same rule: test 2 states hash-first; this is the dropped case.
11 Real: yes.
Verdict: Rewrite. Same fault as test 2: the 20 ms sleep stands for a file time that must move, with no stated reason, and makes the test lean on the clock. Today it catches: a dropped click's hash write not reaching the library (kept).

### 11. a dropped click's save marks both reports out of date (tests/test_app_click.cpp:758)
Delta round: new
Read: common; lines 758-790; `take_view_outcome` 525-531; A5 record 3 (the same claim for a click that is not dropped; Verdict: Keep).
1 Claim: when a click the user let go saved a result, both open reports are marked as out of date with the library.
2 Owner: `take_view_outcome` calls `mark_reports_out_of_date(ReportOutOfDate::Library, path=true, dm=true)` (`app_state.cpp:529`). Another caller: `app_state.cpp:721` (the batch's end). Window code reads `out_of_date` (the report strips; not opened).
3 Real chart: claim not checked: the test reads the `out_of_date` field; the report strip that tells a player is not captured (`records-screen/index.md` says reports are not captured).
4 Direct: 779, 780 REQUIRE both None before; 783 state None; 785 path report is `Library`; 786 dm report is `Library`.
5 Reason: D115 question 4 cited ("for a click the user let go"); not opened.
6 Display: reads the field, not the strip's drawn words.
7 Failure: the dropped click's save leaving reports looking current.
8 Deleted: 785, 786 each one report's marking.
9 Stand-in: the field for the strip's words.
10 Same rule: A5 record 3 (`test_app_click.cpp:136`, "the click's save marks both open reports out of date"). Split: A5 record 3 says Keep for the same claim read from the same field, and A7 record 23 says Rewrite for reading a raw flag instead of the strip. Two verdicts on one kind of check; both go to the main session.
11 Real: yes.
Verdict: Investigate (claim not checked only): needs a capture of the path report and comparison windows' out-of-date line on a real library after a dropped click.

---

## Part 2: tests/test_app_library.cpp

Shared answers for Part 2.
Question 2 for the batch-tray tests: `BatchJob::note_saved`/`answer_waiting` file the store's answer for each saved chart in a tray (`library_jobs.cpp:504-543`), and `AppState::tick_library` empties the tray onto the rows (`app_state.cpp:256-298`, caller of `take_saved_rows`: only `app_state.cpp:271`). `tick_library` is called from `src/ui/library_table.cpp:627` (per A6 record 3, line not re-checked). The engine (the store) owns the answer and the tray carries it; the window shows rows from the carried answers (derive-once: no display rule invented).
Question 3: as in the common section: a row-text claim is "claim not checked" (Library row capture on a real corpus chart during a batch); a lock claim is checked on the real mechanism.
Question 7: the commit `3cbaa4b5` "The window takes the batch's tray on a tick and reads no row from the store" and `f110b065` "The window's reads go through the store's reader" added these. Subjects only.
Question 6: row text read through `row_of(...).best_label()` against typed text "1,000" or "2,000" is a typed expected value. These come from `gated_scores` (the n-th chart through scores `1000 * n`, `test_app_library.cpp:204-215`), so the expected text is the test's own input printed by the number formatter, not the engine's record.

### 12. a tick during a batch does not wait for the store (tests/test_app_library.cpp:346)
Delta round: new
Read: common; lines 346-358; `returns_while_held` (`store_lock_holder.h:57-67`); `let_through` (222-230).
1 Claim: while a save group holds the store's lock, the window's tick takes the batch's tray and does not wait, and the rows read the batch's two answers.
2 Owner: `AppState::tick_library` (`app_state.cpp:256-298`); see shared Q2.
3 Real chart: the lock part is checked on the real mechanism (real helper thread holding the real lock). Rows' text: claim not checked.
4 Direct: 351 `returns_while_held` (property: the tick came back before the lock was let go); 352 row 0 text starts with "1,000"; 353 row 1 starts with "2,000".
5 Reason: "1,000" and "2,000" are `1000 * turn` from `gated_scores`; their working is in a helper comment (a raw result).
6 Display: expected text typed, not read from the store.
7 Failure: a tick that reads the store waits on the batch's save group. Still possible.
8 Deleted: 351 a tick that reads the store; 352 and 353 the rows taking tray answers (a tick that takes nothing).
9 Stand-in: 352, 353 stand in for "the rows took the batch's answers"; the owner (the store) is not read.
10 Same rule: click test 1 (`test_app_click.cpp:549`) and tests 21 to 25 (the reader tests).
11 Real: yes.
Verdict: Rewrite. Check each row against the store's own answer for that chart (`lookup_one`, read after the lock is let go), and keep the `returns_while_held` line. No typed "1,000". Today it catches a tick that waits on the store (kept) and rows that stay empty (kept).

### 13. a tick during a batch re-reads no row the batch did not save (tests/test_app_library.cpp:360)
Delta round: new
Read: common; lines 360-368.
1 Claim: a result saved by someone other than the batch does not reach its row at a batch tick; it waits for the next whole read.
2 Owner: `tick_library` takes only the tray's rows (`app_state.cpp:271-277`).
3 Real chart: claim not checked (row status on a Library capture). The store write is real.
4 Direct: 366 row 0 text starts with "1,000"; 367 chart 3's row is NotAnalyzed although the store holds its result.
5 Reason: "1,000" as above; 5000 is typed in `save_scored(*app, 3, 5000)` with no reason (any value works).
6 Display: typed text on 366.
7 Failure: a tick that re-reads every row (question 1 of the batch-rows plan, not opened).
8 Deleted: 367 a tick that reads rows it was not told about.
9 Stand-in: none. A floor-like pin: it pins current behaviour, "a result saved by someone else is NOT shown", as a requirement; whether a player wants that is the open question (a design choice from the plan).
10 Same rule: test 14 (the end) and test 15.
11 Real: a result saved behind the batch's back happens when a click saves while a batch runs (A click is disabled? not read); the test says only "behind the back".
Verdict: Investigate. The claim says the screen should not show a stored result until the next whole read. No decision I opened says so; the tests cite "question 1 of the batch-rows plan", which I could not find (see common). Goes to the user as a question.

### 14. a batch's end re-reads no row the batch did not save (tests/test_app_library.cpp:372)
Delta round: new
Read: common; lines 372-391.
1 Claim: when a batch ends, a result someone else saved still does not show on its row.
2 Owner: `tick_library` (`app_state.cpp:288-289`: reads all rows only when the run failed as a whole).
3 Real chart: claim not checked.
4 Direct: 385 row 0 starts "1,000"; 386 chart 3 NotAnalyzed.
5-6 as test 13.
7 Failure: the end of a batch re-reading all rows.
8 Deleted: 386.
9 Stand-in: as test 13.
10 Same rule: test 13; test 15 states the reverse for a failed run.
11 Real: as test 13.
Verdict: Investigate: same open question as test 13.

### 15. a batch that fails as a whole still re-reads every row when it ends (tests/test_app_library.cpp:393)
Delta round: new
Read: common; lines 393-410; `app_state.cpp:283-289`.
1 Claim: a run that fails as a whole, whose saved charts never reach the tray, reads every row once at its end.
2 Owner: `tick_library` (`app_state.cpp:288-289`).
3 Real chart: claim not checked.
4 Direct: 399, 400 the run has an error; 407 chart 3's row starts "5,000".
5 Reason: 5000 typed.
6 Display: typed text.
7 Failure: the answers a failed run saved never showing.
8 Deleted: 407.
9 Stand-in: none.
10 Same rule: tests 13 and 14 are its other side; they and this test together pin "rows read once for a failed run only", a rule with the two sides in separate tests.
11 Real: a run that fails as a whole needs a pool of no workers (`workers=0`); a real failure is not shown to happen. Test says only "A pool of no workers is a run that fails as a whole".
Verdict: Investigate: claim not checked and the scenario (zero workers) is the only way the test reaches it; the real case it stands for is not named.

### 16. a lookup the batch could not make shows the store's problem (tests/test_app_library.cpp:412)
Delta round: new
Read: common; lines 412-449.
1 Claim: when the batch's lookup for a saved chart fails, the status line says what any failed store read says, and no row changes.
2 Owner: `answer_waiting` leaves the sentence in the tray (`library_jobs.cpp:532-540`); `tick_library` sets it as a problem (`app_state.cpp:278-280`); `read_store` is the window's other source of the same sentence.
3 Real chart: real SQLite file with the `records` table renamed aside by a real `ALTER TABLE` between save and lookup; the sentence compared with the one from the window's own row read. Status line text on screen: claim not checked.
4 Direct: 440-443 the row is NotAnalyzed, the shown order unchanged, analyzed count 0; 447-449 the second sentence equals the first and is not empty.
5 Reason: none typed. `now += 1.0` is an input.
6 Display: sentence compared with another sentence from the same function family, item by item; not with a typed text.
7 Failure: a failed lookup showing nothing or a different sentence.
8 Deleted: 440-443 the rows changing on failure; 447-449 the two paths wording differently.
9 Stand-in: a test that compares two Hydra paths with each other (tray sentence and `read_store` sentence), neither checked against a rule (rubric Q9 fourth stand-in). The wording owner `app::plain_error` is what both call.
10 Same rule: test 25 (`readerfails`) compares the same two sentences for the reader.
11 Real: the table moved aside is a forced fault; a real lookup fails when the disk or file is gone. The test does not say which real case it stands for.
Verdict: Investigate (claim not checked only): the status line's text on a real fault needs a capture; the owner `plain_error` is shared.

### 17. a chart in two folders gets both rows from one batch result (tests/test_app_library.cpp:451)
Delta round: new
Read: common; lines 451-481.
1 Claim: a chart listed in two folders gets both rows filled from the one result the batch stores for it.
2 Owner: `note_saved` (`library_jobs.cpp:504-516`) answers once per chart (md5) and the tray's answer fills every row with that md5 (`library.set_summaries_for`).
3 Real chart: claim not checked (Library rows for a chart held in two folders on a real set). The fixture is a library entry copied with another path.
4 Direct: 470 `answered_count() == 1`; 475-476 each of the two rows is Ready and starts "7,000"; 479 `rows == 2`; 480 the other chart is NotAnalyzed.
5 Reason: 7000 is a typed score (`scored_record(7000)`), an input.
6 Display: typed text "7,000".
7 Failure: only one of two copy rows filling.
8 Deleted: 470 a second answer; 479 a missing second row; 480 a bleed onto another chart.
9 Stand-in: "7,000" for the row's own text.
10 Same rule: none found by grep for `answered_count`.
11 Real: yes; the same chart in two library folders.
Verdict: Investigate (claim not checked only), with the typed "7,000" a smaller fault: its expected side should be the store's answer.

### 18. a failed chart whose failure the batch stored shows on its row, and one it did not store keeps its row (tests/test_app_library.cpp:483)
Delta round: new
Read: common; lines 483-512; `NoNotesError` include.
1 Claim: a chart whose failure the store keeps shows as the store says; one whose failure the store does not keep (song file missing) keeps its row as it was.
2 Owner: the batch decides what to store (`app::` batch code, not read) and the tray reports the store's answer.
3 Real chart: claim not checked.
4 Direct: 507 `failed == 2`; 511-514 the row equals a second app's row for both charts (status and label); 515 the not-kept row status is what it was.
5 Reason: none typed.
6 Display: compared with a second app on the same file. Good.
7 Failure: a stored failure not reaching its row, or an unstored one changing it.
8 Deleted: each line.
9 Stand-in: none. The test name holds a comma (side note).
10 Same rule: test 13 and 17 neighbours.
11 Real: a file that cannot be opened and a chart with no notes both happen.
Verdict: Investigate (claim not checked only). Side note: the test name holds a comma (line 483), and it is split over two lines.

### 19. a paused batch's stored results reach their rows (tests/test_app_library.cpp:514)
Delta round: new
Read: common; lines 514-527.
1 Claim: after the batch is paused, its first stored result is on its row.
2 Owner: `tick_library`.
3 Real chart: claim not checked.
4 Direct: 520 REQUIRE paused; 522 row 0 starts "1,000"; 523 analyzed count is 1.
5 Reason: "1,000" and 1 follow from `let_through(...,1)`.
6 Display: typed text.
7 Failure: a paused batch's rows not refreshing.
8 Deleted: 522, 523.
9 Stand-in: the count `analyzed == 1` stands for "the right row".
10 Same rule: test 12.
11 Real: yes.
Verdict: Investigate (claim not checked only), typed "1,000" and the count are smaller faults.

### 20. a stopped batch's stored results reach their rows, then the list sorts (tests/test_app_library.cpp:529)
Delta round: new
Read: common; lines 529-542.
1 Claim: no tick ran while the batch ran; when it was stopped, one tick puts both answers on their rows and sorts once.
2 Owner: `tick_library`.
3 Real chart: claim not checked.
4 Direct: 536, 537 rows "1,000" and "2,000"; 538 the order is `{1,0,2,3}`.
5 Reason: raw vector with its working in a helper comment (as A6 record 6).
6 Display: typed.
7 Failure: the end sort or answers missing.
8 Deleted: 536-538.
9 Stand-in: the vector.
10 Same rule: A6 records 6 to 10 (the same sort promise). One verdict should cover them.
11 Real: yes.
Verdict: Rewrite with A6 record 6: express the order from the scores the test assigned and the rows' text from the store.

### 21. opening a song does not wait for a save (tests/test_app_library.cpp:1242)
Delta round: new
Read: common; lines of the hunk (diff `@@ -1010,0 +1232,105`); `app_state.cpp:197, 613` (reader calls).
1 Claim: a click that opens a song reads its copies through the store's reader and does not wait for a save group.
2 Owner: `AppState::select` reads `store->reader().list_chart_library_copies` (`app_state.cpp:613`, `read_store`). Callers: the reader is called at `app_state.cpp:197, 205, 217, 235, 613, 667, 669, 874`.
3 Real chart: lock part: real mechanism. The selected entry equals the store's copy: checked against the store's own list.
4 Direct: `returns_while_held`; `selected` has a value; copies size 1; `md5`, `notespath`, `title` and `sig` equal the store's copy.
5 Reason: none typed.
6 Display: compared with the store.
7 Failure: a click that waits on a batch's save.
8 Deleted: the held check; each equality a changed entry.
9 Stand-in: none.
10 Same rule: tests 22 to 25 and click test 1.
11 Real: yes (comment says only opening a song can happen during a batch).
Verdict: Keep.

### 22. select_chart on a hidden chart does not wait for a save (tests/test_app_library.cpp:1261)
Delta round: new
Read: common; hunk lines; `select_chart` (`app_state.cpp:613`).
1 Claim: a report row's click on a chart the search hides reads its copies and entry through the reader without waiting.
2 Owner: `select_chart` reader calls (`app_state.cpp:613`). Callers: not searched past the test and the report window (not read).
3 Real chart: lock part real.
4 Direct: the held check; a selection exists; `md5` and `notespath` equal the store's copy.
5-6 as 21.
7 Failure: as 21.
8 Deleted: each line.
9 Stand-in: none.
10 Same rule: 21.
11 Real: yes.
Verdict: Keep.

### 23. opening the batch confirm does not wait for a save (tests/test_app_library.cpp:1277)
Delta round: new
Read: common; hunk; `plan_batches` (`app_state.cpp:667-669`).
1 Claim: opening the batch confirm does not wait for a save and asks the same questions as a second app on the same file.
2 Owner: `open_batch_confirm`, `plan_batches` through `store->reader()`.
3 Real chart: lock part real; the questions compared with a second app (item by item).
4 Direct: held check; `batch_confirm_pending`; `batch_question(false)` and `(true)` equal the second app's.
5-6 compared with an independent app. Good.
7 Failure: a confirm that waits.
8 Deleted: each.
9 Stand-in: none. The comment says this cannot happen in the running app during a batch (the Analyze button is off), so the test proves a one-way rule only (the test's own words).
10 Same rule: 21, 22, 24.
11 No: the test says the confirm cannot be opened during a batch. The real case it stands for is not named beyond "the one-way rule".
Verdict: Investigate: the scenario cannot happen (the comment says so), and no real case is named. The reason it is kept is a plan label I could not open.

### 24. a settings change re-reads the rows without waiting for a save (tests/test_app_library.cpp:1293)
Delta round: new
Read: common; hunk; `refresh_library_summaries` (`app_state.cpp:205`).
1 Claim: after a settings change, the rows re-read through the reader without waiting, and match a second app opened under the changed settings.
2 Owner: `refresh_library_summaries` (`app_state.cpp:205`).
3 Real chart: lock part real; each row compared with a second app on the same file (an independent reader).
4 Direct: held check; REQUIRE the change reaches a row (`status != seeded_before`); every row's status and label equal the second app's.
5 Reason: `kSeededCap + 1` is a changed cap, an input; the seed's constant has its working in the fixture comment.
6 Display: compared with an independent app. Good.
7 Failure: a settings re-read that waits, or a stale row.
8 Deleted: each.
9 Stand-in: none. The settings are locked during a batch (the comment), so the case cannot happen as written.
10 Same rule: 21-23.
11 No, by the test's own comment (settings controls are locked during a batch); the real case is a settings change with a save group open, which no player's click reaches.
Verdict: Investigate: unreachable scenario, no real case named.

### 25. a failed reader read shows the store's problem (tests/test_app_library.cpp:1312)
Delta round: new
Read: common; hunk; `read_store` (`app_state.cpp:197`).
1 Claim: when the window's reader cannot open, the click shows the store's sentence, selects nothing, and the sentence equals the reader's own error.
2 Owner: `read_store` and `app::plain_error`. Both sides call `plain_error`.
3 Real chart: claim not checked for the status line's text; the reader failure is forced by the store's seam `fail_next_reader_open_for_test`, not a real broken file.
4 Direct: `status_is_problem`; `selected` still chart 0; `from_select == from_reader`, not empty.
5 Reason: none typed.
6 Display: two Hydra paths compared with each other, both through `plain_error`; neither is checked against a typed sentence or a rule (Q9 fourth stand-in).
7 Failure: a failed reader open crashing or being silent.
8 Deleted: each.
9 Stand-in: as 6.
10 Same rule: test 16 (the tray's sentence).
11 A reader that cannot open can happen (disk full, file locked); the seam forces it.
Verdict: Investigate (claim not checked only), with the Q9 note above: the sentence itself is never checked.

### Changed through a helper, test_app_library.cpp

The changed helpers: `let_through` now waits for `answered_count() == charts` as well as `completed == charts` (`test_app_library.cpp:222-230`), so a tick cannot run ahead of the tray. It does what its callers rely on: it holds until the tray has the answers. `run_redo_batch_over` (app_state_fixtures.h, below) now runs the engine for a Ready stored record.

### 26. a batch result for the open chart refreshes its row, not the panel (tests/test_app_library.cpp:83)
Delta round: changed, replaces A6.md record 3
Read: common; lines 83-103; fixture `run_redo_batch_over` and `stored_record_is_empty` (diff hunk).
Old verdict: Investigate (split with D48 Q16). Does it hold? Yes. The body is unchanged. The helper changed: the batch's analyzer returns the empty analysis when the stored record is missing or empty; this chart has no file and `save_batch_record` stores the empty record, so `stored_record_is_empty` returns true and the batch files the empty record, as before. The split with D48 Q16 and the unfound "Ruling 14" stand as A6 recorded. Questions 1 to 11: as A6 record 3, with one change: the helper's comment at `app_state_fixtures.h` no longer says the batch "fails on purpose"; the batch now files an analysis, so A6's Q11 "the test's batch analyzer fails on purpose" is out of date, but the chart (file gone) still cannot be batched for real.
Verdict: Investigate (same split as A6 record 3).

### 27. a batch result for an open Ready chart changes its row and keeps the panel (tests/test_app_library.cpp:107)
Delta round: changed, replaces A6.md record 4
Read: common; lines 107-131; the helper hunk.
Old verdict: Investigate (the split with D48 Q16). Does it hold? The body is unchanged, and the helper changed in a way that bears on A6's Q7/Q11. The test stores the empty record through `save_batch_record` before running the batch. `stored_record_is_empty` (`app_state_fixtures.h`) finds a record equal to `record_of(song_id, empty_analysis_under(profile))` and says empty, so the batch files the empty analysis, the same as the old "fails on purpose" helper. So the test still builds by force a stored empty result for a chart the click analysed with paths; A6's note that a real batch would store the engine's own record is still right. The old verdict holds.
Verdict: Investigate (same split).

### 28. a batch refresh fills scores in where the rows sit; the batch's end sorts them (tests/test_app_library.cpp:249)
Delta round: changed, replaces A6.md record 6
Read: common; lines 249-265 and `BatchAfterTwoCharts`, `let_through` (194-245).
Old verdict: Rewrite (typed "2,000" and `{3,2,1,0}`, helper-comment working). Does it hold? Yes. The body is unchanged. `let_through` now also waits for `answered_count`, which makes line 256's wait stricter, not different; the typed "2,000" at line 253 and the order vectors are still raw results with their working in the helper comment `gated_scores` (n-th chart through scores `1000 * n`, lines 194-198). Claim not checked on screen is unchanged.
Verdict: Rewrite (as A6 record 6).

### 29. a batch whose last result was already read still sorts when it ends (tests/test_app_library.cpp:270)
Delta round: changed, replaces A6.md record 7
Read: common; lines 270-278; `BatchAfterTwoCharts`.
Old verdict: Rewrite with record 6. Holds: body unchanged; the changed `let_through` inside the fixture makes the setup wait for the tray. The vector `{1,0,2,3}` is still a raw result.
Verdict: Rewrite (with A6 record 6).

### 30. a batch's end sorts once, and the next batch sorts when it ends (tests/test_app_library.cpp:280)
Delta round: changed, replaces A6.md record 8
Read: common; lines 280-300.
Old verdict: Rewrite with record 6. Holds. 5000 and the vectors are unchanged raw values. The fixture's changed `let_through` only strengthens the setup.
Verdict: Rewrite (with A6 record 6).

### 31. a batch dismissed before a tick saw it finish still sorts (tests/test_app_library.cpp:303)
Delta round: changed, replaces A6.md record 9
Read: common; lines 303-311.
Old verdict: Rewrite with record 6. Holds: `{1,0,2,3}` unchanged; the fixture change does not alter what is checked. One new point to carry: `tick_library` now reads the tray before the sort (`app_state.cpp:270-297`); with `batch_job.reset()` the tray is dropped with the job, so the rows were last filled by the setup's tick (`BatchAfterTwoCharts` line 242). The test still reaches "rows keep their answers when the job is let go".
Verdict: Rewrite (with A6 record 6).

---

## Part 3: tests/test_app_reports.cpp

Shared answers for Part 3.
Question 2: `ReportJob::run` opens a connection of its own with one hold: `store_.open_own_reader()`, `reader->hold()`, `generate_report(reader->reads(), options)` (`library_jobs.cpp:824-826`). Another caller of `open_own_reader`: `dm_jobs.cpp:51` (the comparison). The engine's store owns the hold; the reports are built from it. Callers of `set_between_lookup_reads_for_test`, `fail_next_reader_open_for_test`: tests only plus the store (not searched further).
Question 3: the lock, hold and connection claims are about the real store mechanism (a real SQLite file, a real helper thread for the group, a real writer thread for the between-reads save); the report's rows and text are screen claims: "claim not checked" (the path report and comparison windows are not captured: `records-screen/index.md` says so; A7 says the same).
Question 7: `e3b7b890` "Each report reads through a connection of its own, under one hold", `1dad7f1c` "A test shows a report's separate store calls read one moment under its hold", `a5f1e7b1` (the Cancel route and shared seam). Subjects only.

### 32. a path report builds while a save group is open (tests/test_app_reports.cpp:952)
Delta round: new
Read: common; lines 952-970; `check_same_path_report` (a hunk helper); `ReportJob::run`.
1 Claim: while a save group is open, the path report still builds, and its page equals the one built with nothing held.
2 Owner: the report's own connection (`library_jobs.cpp:824`).
3 Real chart: lock part real. The page compared with the unheld page from the same app (a property).
4 Direct: 966 `group.held()` when the job finished; 968 build state Ready; 970 the held report equals the unheld one field by field (subtitle, footer, songs, rows, library counts, each path's song, rank, path, score, copies).
5 Reason: none typed.
6 Display: expected side is the same report built unheld: a property, not a typed text. The report's text on screen: claim not checked.
7 Failure: a report that waits on a batch's save group.
8 Deleted: 966 a report that waited; 970 any field.
9 Stand-in: comparing two builds by the same function: each could be wrong the same way, but the claim is "the hold does not change the page", so a comparison with itself is the property.
10 Same rule: tests 33, 34.
11 Yes (a report requested during a batch).
Verdict: Investigate (claim not checked only): the report window's page on a real set.

### 33. a DM report builds while a save group is open (tests/test_app_reports.cpp:972)
Delta round: new
Read: common; lines 972-993.
1 Claim: same as test 32 for the comparison with dmleaderboards.
2 Owner: `dm_jobs.cpp:51`.
3 Real chart: lock part real; the leaderboard is a canned answer (`kDmUsersJson`, `kDmScoresJson`), not the real network, and the real WinHTTP stack is not used. Report text: claim not checked.
4 Direct: 987 held; 989 Ready; 991 the two comparisons equal (username, subtitle, footer, total, each row's identifier, optimal and status).
5 Reason: "111" and "alice" are inputs.
6 Display: as 32.
7-11 as test 32.
Verdict: Investigate (claim not checked only).

### 34. a path report reads one moment (tests/test_app_reports.cpp:994)
Delta round: new
Read: common; lines 994-1020; `save_during_report_lookup` (hunk).
1 Claim: if another writer saves a result between the report's two reads inside one lookup, the report still shows the store as it was at its start.
2 Owner: `OwnReader::hold` and the store's `lookup` (bodies not read); the seam `set_between_lookup_reads_for_test`.
3 Real chart: real mechanism (the real report job thread; a real writer saving between two real reads of the real file); checked against the unheld report. Report text: claim not checked.
4 Direct: 1011 REQUIRE saved; 1012 the stored record now has no paths (the save landed); 1013 Ready; 1015 the report equals the one before.
5 Reason: none typed.
6 Display: equals the earlier report (property).
7 Failure: reports mixing two moments (torn read).
8 Deleted: 1011 the writer not firing (vacuous pass); 1012 the save landing; 1015 any change in the page.
9 Stand-in: none; it asks the right thing. Leans on a seam inside the store between reads.
10 Same rule: test 35.
11 Yes: a click or batch saving while a report builds.
Verdict: Investigate (claim not checked only).

### 35. a path report's separate store calls read one moment (tests/test_app_reports.cpp:1022)
Delta round: new
Read: common; lines 1022-1054.
1 Claim: a report whose library has no stored result, with a result saved by the writer mid-build, says "nothing is analyzed yet", the answer at its first read, and not "nothing under these settings".
2 Owner: the hold and `record_count` (store bodies not read); the sentence is `plain_error(KindedError(NoRecords, "no records stored yet"))`.
3 Real chart: real mechanism; the sentence checked against `plain_error`, not captured on screen: claim not checked for the window's text.
4 Direct: 1048 REQUIRE saved; 1049 `record_count() > 0`; 1050 build state Failed; 1051 the message equals `plain_error(KindedError(NoRecords...))`.
5 Reason: `report_confirm_above = 0` lowers a threshold so the confirm opens; the 500 it normally holds is D249 item 8 (A7 quotes it; not opened by me).
6 Display: expected side is the app's own sentence function (`plain_error`), the owner. Good.
7 Failure: a report that reads `record_count` outside its hold.
8 Deleted: 1050, 1051.
9 Stand-in: none.
10 Same rule: test 34.
11 A report requested on a never-analyzed library with a writer saving mid-build; a click does this.
Verdict: Investigate (claim not checked only).

### 36. a report whose connection cannot open fails with the store's sentence (tests/test_app_reports.cpp:1056)
Delta round: new
Read: common; lines 1056-1082; `failed_open_sentence` (hunk).
1 Claim: when a report's connection cannot open, its window reads the store's own sentence, offers Try again, and builds on the retry; the same for the comparison.
2 Owner: `plain_error`; the store's seam.
3 Real chart: failure forced by a seam. Claim not checked for the window's text.
4 Direct: 1068 failed; 1069 message equals the store's sentence (the same function's output obtained from a direct open); 1071 Ready after Try again; same for the comparison.
5 Reason: none typed.
6 Display: against the store's own sentence from a direct open. Good.
7 Failure: a failed open leaving the report stuck or silent.
8 Deleted: each.
9 Stand-in: comparing two Hydra paths (the report's and the direct open's), both through `plain_error` (Q9 fourth stand-in); the sentence text is never checked against a rule.
10 Same rule: test 25.
11 Plausible (file locked).
Verdict: Investigate (claim not checked only).

### 37. a finished report holds no connection (tests/test_app_reports.cpp:1084)
Delta round: new
Read: common; lines 1084-1095; `check_log_empties` (hunk).
1 Claim: after a path report and a comparison finish, no connection of theirs stays open.
2 Owner: the job's scoped reader and hold end before the job finishes (`library_jobs.cpp:819-827`).
3 Real chart: real mechanism: the real WAL file; the truncating checkpoint empties the log only when no other connection holds a read open (the helper's comment; the store's own test "a reader holds nothing open between reads" was not opened).
4 Direct: 1090 REQUIRE reports Ready; `check_log_empties` REQUIRE log size > 0 inside the batch writes and CHECK log size 0 after.
5 Reason: none typed. 0 is zero.
6 Display: n/a.
7 Failure: a leaked connection that blocks the WAL checkpoint.
8 Deleted: the final size check (a leaked reader); the REQUIRE (a vacuous pass).
9 Stand-in: log size stands for "no open connection"; it relies on the store's checkpoint rule, which this test did not check (not read).
10 Same rule: test 38; the store's own test.
11 Yes.
Verdict: Keep. The claim is about files and connections, checked against the real WAL file size (an independent owner, the filesystem). Caveat in the open: the checkpoint rule is the store's, stated in the helper's comment and not read by me.

### 38. a cancelled report closes its connection (tests/test_app_reports.cpp:1097)
Delta round: new
Read: common; lines 1097-1112; `held_report`, `set_report_hold_for_test` (`library_jobs.cpp:796-802`).
1 Claim: a path report cancelled before its job reads closes its connection.
2 Owner: `ReportJob::run`: the hold seam runs at `:818` before the connection opens at `:824`, so the cancel is seen by `generate_report` and the scoped reader ends.
3 Real chart: real mechanism (WAL size).
4 Direct: 1108 build state Cancelled; `check_log_empties`.
5 Reason: none.
6 Display: n/a.
7 Failure: a cancelled report leaking its connection.
8 Deleted: 1108 the Cancelled state; the log check.
9 Stand-in: as 37. Note the seam sits before the connection opens (`:818` then `:824`), so this test cancels before any connection exists; a leak after a cancel mid-read is not what it exercises (reasoned from the seam placement).
10 Same rule: 37.
11 Yes.
Verdict: Investigate: the claim ("a cancelled report closes its connection") is not exercised, because the cancel arrives before the connection is opened (`library_jobs.cpp:818` before `:824`); a cancel while the report reads is the case the name promises.

### 39. the path report goes out of date on a batch or a setting it was built from (tests/test_app_reports.cpp:272)
Delta round: changed, replaces A7.md record 9
Read: common; lines 272-345 not re-read beyond the two `run_redo_batch_over` calls at 309 and 325 (found by grep and the diff; the body outside them is unchanged); the helper hunk.
Old verdict: Investigate (claim not checked; seven claims in one test; D115 question 2 not opened). Does it hold? Yes. The body is unchanged. The changed helper `run_redo_batch_over` now runs the engine for a Ready stored record, or files the empty record when the stored one is empty or missing; this test's batches (lines 309 and 325) run over `title` from `app_with_result`, whose stored record is an empty `save_batch_record` style one (not re-read), so the batch behaviour is the same as before. I did not confirm that `app_with_result` stores an empty record: the body of `app_with_result` was not read. If it stores an engine record, the batch now runs the engine on a chart that may have no file and the test could change. Stop: unconfirmed.
Verdict: Investigate (same as A7 record 9, with the helper's effect on the two batches unconfirmed).

### 40. the post-batch report lists the batch's cap and lens, not the live settings (tests/test_app_reports.cpp:793)
Delta round: changed, replaces A7.md record 22
Read: common; lines 793-815 by the diff context and the call at 797 (`run_redo_batch_over(*app, library_entry(5).md5)`); the helper hunk.
Old verdict: Investigate (claim not checked; the D51 citation does not state this claim). Does it hold? The body is unchanged and the citation fault stands. The helper change matters here: `library_entry(5)` is a seeded chart with no file (A6 record 3 relies on the same chart having a missing file), so `stored_record_is_empty` is true unless the seed stored an engine record for chart 5; A6 says chart 5 is NotAnalyzed in the seed, so it returns true and the batch files the empty record as before. This was read from A6 and the `app_with` base code, not run.
Verdict: Investigate (same as A7 record 22).

### 41. the strip's report_started holds through a cancel, a refresh and a close (tests/test_app_reports.cpp:816)
Delta round: changed, replaces A7.md record 23
Read: common; lines 816-850 (whole body, from the earlier command's output); the helper hunk.
Old verdict: Rewrite (reads the field, not the strip's text). Does it hold? Yes. The two `run_redo_batch_over(*app, title)` calls (lines 825 and 847) run over `app->library_row_at(0).title` of `app_with_result`; the helper change does not alter what the test reads (`report_started` and the build states). Verdict stands: the claim is about the strip, the test reads the raw flag. Side note: the test name holds a comma (line 816). The field's readers in `src/` are at `library_dialogs.cpp:492, 546` per A7.
Verdict: Rewrite (as A7 record 23).

---

## Part 4: helpers in tests/app_state_fixtures.h

### H1. stored_record_is_empty (tests/app_state_fixtures.h, diff lines +203-214)
Delta round: new
Read: the diff hunk; `RecordStore::lookup_one`, `record_of`, `empty_analysis_under` (callers' side only; bodies not read).
What callers rely on: it says true when the chart has no Ready record under the profile or has exactly the empty analysis `save_batch_record` files. A record equal to the empty analysis uses `HydraRecord::operator==` on `record_of(song_id, empty_analysis_under(profile))`; whether the stored record's song id and equality cover every field was not read.
Callers: `run_redo_batch_over` only (grep over `tests/`: no other caller).
Test cases that reach it: through `run_redo_batch_over`: tests 26, 27 (library), 39, 40, 41 (reports).
Does it do what its callers rely on? Likely, with one reading gap: for a chart whose stored record is the engine's (a click's ViewJob), it returns false, so the batch runs the real engine and fails with a thrown error if the chart has no file. Callers in the library and report tests store empty records, so they take the empty branch. Not confirmed: `app_with_result`'s stored record (see test 39).

### H2. run_redo_batch_over (tests/app_state_fixtures.h, changed)
Delta round: changed
Read: the diff hunk.
Before: the batch analyzer threw "no chart file in this test", so the batch stored nothing and the test's own stored record stood. After: the analyzer returns the empty analysis when the stored record is empty or missing, otherwise runs `analyze_chart_file` (the engine) under the batch's settings, as a real batch does; the batch files its own result and the window learns of it from the batch's tray. This follows the batch-rows change, in which the window no longer reads the store at a tick (`tick_library`, `app_state.cpp:256-298`). It does what its callers rely on (the batch ends and the row shows the stored record); the one hidden dependency is `app.library_row_at(0)`: it reads the first shown row's md5, so callers that searched first (`search` finds the chart) rely on the search leaving that chart at row 0.
Callers: tests 26, 27, 39, 40, 41 (tests/test_app_library.cpp:95, 121; tests/test_app_reports.cpp:309, 325, 797, 825, 847). Also `tests/ui/*.cpp` and other files: grep found no other caller in `tests/`.

---

## Splits and findings

Split 1. A5 record 3 (Keep) and A7 record 23 (Rewrite) judge the same kind of line (a test reading `out_of_date` or `report_started` as a field instead of the strip's text). New click test 11 follows A5 record 3's pattern; it is Investigate here. One verdict for all three.

Split 2. Tests 13, 14 and 15 pin "a result saved by someone else does not reach its row at a tick or at a batch end, but a run that fails as a whole reads every row at its end". Their comments cite "question 1 of the batch-rows plan". I could not find that plan or those labels (O4, O7, O13, O22) in `docs/superpowers/plans/` or the decision log; no decision I opened says the screen should hold a stored result back. If it is the user's choice it needs a quoted ruling; if not, the tests pin the display's own rule. Goes to the main session.

Production finding: none against a display function. `BatchJob::answer_waiting` and the tray carry the store's answer to the rows, which is the carry-out the rubric asks for.

Side notes. Test names with a comma: tests/test_app_library.cpp:483 (the failed chart test) and tests/test_app_reports.cpp:816. Test 2 and 10 both use `Sleep(20)` (`test_app_click.cpp:582, 744`). Test 38's seam sits before the connection opens (`library_jobs.cpp:818` before `:824`).

## Not reached or not confirmed

All 32 new and 9 changed test cases (and 2 helpers) have a record. Not confirmed: `app_with_result`'s stored record (tests 39 to 41), the store's hold and checkpoint bodies (tests 34 to 38).

Captures needed and not available: the details panel and the Library table row on a real corpus chart, during and after a click and a batch result; the path report window and the comparison window's page, out-of-date line and failure sentence on a real library; the status line's text for a failed store read.
