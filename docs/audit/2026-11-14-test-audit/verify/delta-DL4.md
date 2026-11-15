Agent: a66d9636037bb7e7f (task T5-V-DL4)

# Verify DL4: tests/test_library_jobs.cpp, tests/test_library_model.cpp, tests/test_model.cpp, tests/test_config.cpp

Scope, from `git diff c10c7f54 5ec682c7` on each file: 28 test cases in scope, 28 records, none missing, none removed. Jobs: 8 new. Library model: 5 new. Model: 3 changed (squeeze_difficulty and is_e0; allzero_activation; the refusal and E0 edges), 1 changed (Activation notationstr), 3 new. Config: 5 changed (round-trip, same keys, missing INI, out of range, clamp), 2 new early-fill tests, 1 new summary test. The helper `check_same_settings` changed and both its callers have records. `git diff 5ec682c7 HEAD` on the four test files and `src/` is empty, so I read the worktree as it stands. Nothing was run.

Result: agree with 23 verdicts, dispute 5 (jobs 1, 2, 3; model 4; config 8).

Faults that hit every record in the file. The line numbers the records give for assertions are often wrong against the file at `5ec682c7`. Jobs 1, 3, 4 are 3 lines high (record "477 completed == 3" is line 474), jobs 6 is 5 lines high, jobs 7 and 8 are 3 high, model 3 to 7 and config 4 to 7 are 1 to 4 off. The assertions themselves match, so no verdict moves. The ruling lines the records cite as 56-58 of `docs/agents/rulings/code-and-engine.md` are lines 55-57 (all three are under `## Confirmed`, which ends before line 61; the words match what the records quote). The records' Q8 "reasoned" marks are all by reading; I found no run in the records folders that proves any of them.

---

## tests/test_library_jobs.cpp

Helpers: `real_in_turn`, `always`, `answers_for`, `charts_with_a_copy` do what callers rely on (by reading). `same_answer` compares six fields by hand; I opened `RecordLookup` (record_store.h:332-345 region) and the record is right that a later field would be missed silently. `check_answers_match_store` is the owner's answer asked afterward, item by item: right shape.

Source reading that matters for several records: `run_batch`'s `report` (analysis.cpp:852-867) calls `on_result` once per row (`for r < wr.rows`), so a chart in two folders does reach `note_saved` twice. The dedupe at library_jobs.cpp:508-509 is live and tests 1 and 3 do exercise it. `answer_waiting` returns before the hook when `waiting_` is empty (library_jobs.cpp:520), as the record says.

### 1 answered_count ... (test_library_jobs.cpp:464): DISPUTE the Keep
Scope: found, record present. Delta line ok (new).
Claims checked by reading: `answered_count` (library_jobs.cpp:499) has no caller in `src/` (grep `answered_count` over `src/`: header and definition only), so the record's Q2 is right. 474 `completed == 3`, 475 and 478 `answered_count == 2` hold by reading `plan_batch` (analysis.cpp:791-809: one todo entry per md5, `rows` holds the copy count) and `report`.
Why I dispute: (a) the record's own Q3 says "claim not checked on a real library" and the rubric rules Keep out on that answer; the input is hand-built (`fake0`, `fake1`, a copied entry, a stubbed analyzer, an in-memory store). (b) The claim is about a counter no screen and no player path reads (record Q2/Q6 say so), and the promise that matters, a chart in two folders gets one answer, is made again by test 3 (record Q10 says one verdict covers 1 and 3). Verdict should be Investigate (claim not checked only), or fold into 3 and drop the test-only counter check. Q8 per assertion: 474 turns red if the copy row is not counted (`report` loop), 475 if the dedupe at 508 is removed (count 3), 478 if take_saved_rows touched `answered_`; by reading.
Split: none. Production note: `answered_count` and `set_lookup_hold_for_test` are production API only tests call.

### 2 a batch job's tray holds the store's answer (:481): DISPUTE the Keep
Scope found; record present. Callers: `take_saved_rows` is called at app_state.cpp:271 only (grep over `src/`), as the record says. Q8: 491 turns red if `answer_waiting` swallowed a store error into `tray_.problem` (library_jobs.cpp:536-538); 492-494 if the dedupe or `note_saved` dropped a chart; 495 if `answers` held anything but `store_.lookup` output. By reading, all hold.
Why I dispute: Q3 of the record says "Library screen side: claim not checked", and every input is made up: ids `fake0`/`fake1`, an in-memory store, one stubbed analysis result saved under both ids. Brief: a store claim whose only input is a hand-built fixture has not been checked on real input. Verdict should be Investigate, "claim not checked only": a real library file copy that went through a real batch, with the tray answers read against `lookup_one` on the copy. (The shape of the checks, the owner's answer item by item, is right and stays.)

### 3 a chart in two folders is asked about once (:498): DISPUTE the Keep
Same reasons as 2: hand-built copy entry, in-memory store, hook count read through a test seam in production code (record Q3 admits "not an independent owner"). Record Q9 also notes the count is exact only because the early return at library_jobs.cpp:520 precedes the hook; I confirm that by reading, and that is a stand-in the record leaves as "not a fault". Q8: 514-516 turn red if the dedupe at 508-509 is removed (three answers); 517 (`lookups == 2`) turns red the same way (3 lookups). Verdict should be Investigate (claim not checked only): a real chart folder scan with a real duplicate chart through `plan_batch`, not a stubbed `charts_with_a_copy`.

### 4 a failed chart whose failure was stored reaches the tray (:524): AGREE, Keep, with a correction
I read the part the record left "not read". `remembered_failure` (core/error_kind.h:81-85) lists NoNotes, ChartTimingRefused, NoteShuffleFreezes, ChartUnreadable, so `SongFileMissing` (fake1) is not kept and `NoNotesError` is (`NoNotesError` is a `ChartFileError`, model.h:47, which is a `KindedError`, error_kind.h:129, so the `dynamic_cast` at analysis.cpp:786 works). `report` calls `on_failure_saved` only when `wr.row` is still set (analysis.cpp:861), and `save_result` resets it when the save throws (:770). The claim holds by reading. Real mechanism: a real file store, a real SQLite trigger set from a second connection, a real worker thread; independent owner `lookup_one`. The record's stray "Not read" and "claim not checked" (screen) should be struck, since the claim is not a screen claim.
Q8: 549 `failed == 3` goes red if a failed chart is not counted; 553 and 554 if fake1 (unkept) or fake2 (save refused) got an answer; 556 if the count included them. By reading.

### 5 the tray is empty after it is taken, and a stopped run's tray (:560): AGREE, Keep
Read analysis.cpp:915-921 (a result that finishes after cancel is dropped); `job.stop()` sets `cancel_`. Real thread, real cancel flag. Q8 as the record, by reading: 577 goes red if a post-stop chart is answered, 581-584 if take does not empty or reset. The record's "Screen claim not checked" does not apply to this claim.
Split: none.

### 6 a paused batch job's tray keeps its answers (:587): AGREE, Rewrite, with a note
The 200 ms sleep at line 602 decides when to look for "nothing added while paused". A bug that lets the second chart through after 200 ms passes. Agree on Rewrite. Note: the record's fix (read the analyzer's start count) still has to pick a moment to look, so the rewrite cannot remove timing altogether; it can make the second chart's analyzer, which is the test's own, count its calls and assert the count after resume ordering (call 2 starts only after `resume()`). Lines 606-615 (tray keeps the paused answer, the rest arrives after resume) hold by reading.

### 7 a lookup that fails leaves its sentence in the tray (:622): AGREE, Keep
Read lines 618-677 against library_jobs.cpp:518-543 and 612. The rename hook runs once (`lookups++ == 0`) before the lookup, the table is put back before chart 1 is released, so `lookups == 2` and both answers arrive. The expected sentence comes from `store.lookup_one` run on the same broken store (`plain_error`), an independent call. Real mechanism: real file store, second real connection. By reading the failure the test names (a retained `waiting_` entry dropped on error) turns red at 648 and 668-675. The record's "claim not checked" on the screen side is not part of this claim.

### 8 a frame's snapshot does not wait while the batch job waits on the store (:682): AGREE, Rewrite, with a note
`StoreLockHolder` (store_lock_holder.h:18-43) is a real second thread holding the real lock. The 50 ms sleep at line 700 stands for "the worker is waiting on the lock"; if the worker is slower than that, a snapshot that wrongly holds `mu_` during the lookup passes. Same note as 6: the wait cannot be turned into a condition the test can observe without a production seam, so the rewrite must name one (for example the hold hook returning only after the store lock request, which the test cannot see today).

---

## tests/test_library_model.cpp

Callers: `set_summaries_for` has one caller in `src/`, app_state.cpp:277. `set_summary_for` is `set_summaries_for({{md5, lookup}})` (library_model.cpp:282-284). Also note, as a missed duplicate: `LibraryModel::set_summaries` (library_model.cpp:273-280, answers by row index) is a second bulk entry point next to `set_summaries_for` (answers by id). Both go through `apply_summary`, so I count it as shared code and not a split.

### 1 set_summaries_for gives every row the answer set_summary_for would (:749): AGREE, Rewrite
`CHECK(changed_at_once == changed_one_by_one)` and the five chip counts compare a function with its own wrapper: true by construction. Lines 771-773 compare each row with the answer given through `shows_same_as`, which is also the predicate `apply_summary` uses to decide a change (library_model.cpp:61-69, 81); it is a real row-by-row property but its oracle is production code. By reading.

### 2 applies a chart's later answer when it is listed twice (:781): AGREE, Remove
Only caller passes the tray; in the tray `answered_md5s_` and the `waiting_` check (library_jobs.cpp:508-509) stop a repeat inside one batch. `insert_or_assign` (library_model.cpp:292) is the later-wins rule and no caller can reach it. By reading.

### 3 updates both rows of a chart in two folders (:794): AGREE, Investigate (claim not checked only)
Chip counts count rows (805-806). 797 and 801-804 repeat test 1's per-row check. Needs the Library chip counts on a real chart in two folders after a batch.

### 4 holds the row order under the Best path sort (:811): AGREE, Rewrite
By reading `summaries_changed` (library_model.cpp:302-310) and the sort (:360-380): scored rows first in descending score, unscored by rank then the tie-break. The typed title list at 824-825 is a raw result (fits the data: YYZ 500000, Halo 400000, Burnout 378315 from `sample()` at :87, then the rest). Rewrite as the record says: state it as "scored rows in descending score, unscored after".

### 5 answers that change nothing rebuild nothing (:829): AGREE, Investigate (claim not checked only)
The three answers are `sample()`'s own rows (test_library_model.cpp:87). Line 845 turns red if the `changed > 0` guard at library_model.cpp:298 is removed (`summaries_changed` sets `order_out_of_date_` under Best path), by reading.

---

## tests/test_model.cpp

Callers of the window functions in `src/` (grep): `is_e0` at model.cpp:473, 475 and engine.cpp:859; `fill_refuses` at engine.cpp:862, 1007; `allzero_activation` at model.cpp:855 and engine.cpp:863, 1936; `fill_within_limit` at model.cpp:535 and engine.cpp:869, 1224. `pather.cpp` calls none of them: record 2's caller list ("pather.cpp (A18 record 8 lists 55, 104)") is carried over from the old record and is stale. The displays read the engine's answer through `Activation::is_e_critical`/`is_E0` (path_view.cpp:237, 273, 280); nothing displays its own rule.

### 1 squeeze_difficulty and is_e0 (:147): AGREE, Rewrite
Old record A18 7 was Rewrite; edited lines only change the window argument. By reading: 155-157 are the edge at the default window (a rule number, ruling `code-and-engine.md` lines 55-57), 164 is `Activation::is_E0` against the function it calls (model.cpp:475), true by construction. Real chart: `beg` shows "1- 0 0 E0" and "Early fill: 0.0 ms (required)" (records-screen/beg.txt:22, 67); the edge at 60 is on no chart.

### 2 allzero_activation (:171): AGREE, Investigate
Old A18 8 was Investigate; still holds. Lines 174-183 and 185-198 hold by reading `allzero_activation` (model.h:381-383) and `Path::is_allzero` (model.cpp:851-856). Record's caller list: strike pather.cpp (see above).

### 3 the refusal and the E0 edges (:206): AGREE, Rewrite
Old A18 9 was Investigate (edges on a real chart); the record carries it as a second finding. Lines 219-223 compare a member with the function it calls, stand-ins. Line 232-233 type -80 as the limit: nothing says why -80; at offset 70 the difficulty is -70 (`early_fill_difficulty`, model.h:340), so -80 is simply "a bit below". Agree it should be named from the offset.

### 4 every fill that refuses is an E0 at the same window (:239): DISPUTE the Remove
The record says test 3 already makes this promise. It does not: test 3 checks `is_e0` only at `w` and `w - 0.1`, never at a negative offset. This sweep (e from -200 to 200 at six windows) is the only line that checks `is_e0` at the offsets where `fill_refuses` is true, and the engine relies on it: `ready_class` (engine.cpp:856-860) breaks out of its loop when `is_e0` is false and says "none refuses (is_e0 holds for every fill that refuses)". A change that gave `is_e0` a lower bound (`e_offset >= 0 &&`, say) leaves test 3 green and turns line 247 red; by reading. The record's point that a constant read for the window would pass here is right, but that is test 3's job. Verdict should not be Remove. Investigate (claim not checked only): a real chart whose engine run has refused fills (engine.cpp:862 `refused`), none of the seven records carries one. The `refusing > 0` floor at 249 is a "loop ran" guard, fine.

### 5 at 0 ms no spawning activation is an E (:255): AGREE, Investigate
Claim not checked only: engine dump at window 0 (needs a build, so the main session).

### 6 every default is kDefaultEarlyFillWindowMs (:275): AGREE, Investigate
A property against the owner (model.h:112) plus a range check. Needs a record made at the window.

### 7 Activation notationstr (:654): AGREE, Investigate
Old A18 20 verdict still holds; 672-674 show the E prefix follows `e_window_ms` (model.cpp:544).

---

## tests/test_config.cpp

`check_same_settings` (:53) now compares the new field (:69) and does what its callers rely on. Config key at config.cpp:201; `Settings::to_analysis_settings`, `lens` carry the window (:411, :432); the Early fill window box calls `Settings::clamp` (settings_bar.cpp:161-162): the range has one owner. No rewrite here proposes a saved expected-output file. The records ask for a real `hydra_settings.ini` copy as an input, which is not one.

### 1 settings round-trip (:76): AGREE, Rewrite
Old A9 1 holds. `mslimit_value` and `backendlimit_value` are both 42 (lines 86, 88), so a swap passes. New line 92 sets 37: sound, by reading.

### 2 load and save name the same keys (:104): AGREE, Rewrite
The typed list (125-132) copies the key table (config.cpp:160-215); the new name is in it.

### 3 a missing INI yields defaults (:235): AGREE, Rewrite
Line 249 reads the owner's constant. The typed 4, 10, 50, 40 at 238-247 remain, as the old verdict said.

### 4 early_fill_window_ms round-trips (:254): AGREE, Investigate
Typed 60, 0, 85 are the ruling's numbers (default 60, 0 to 85); 37 and 72 are free. The refusals at 271-274 follow `number(...)` handling at config.cpp:201 (not opened beyond the key line). Claim not checked on a real settings file.

### 5 reaches the search and the result's key together (:279): AGREE, Investigate
By reading config.cpp:411, 432, 435-437, 453. Line 298 is a good property (differ in the window and nowhere else).

### 6 a value outside its range loads at the nearest edge (:370): AGREE, Rewrite
Lines 397-400 hold by reading; old A9 11 verdict stands.

### 7 Settings::clamp (:403): AGREE, Rewrite
Lines 414-416 repeat 397-398 through `clamp`. Old A9 12 stands.

### 8 settings summary: the early-fill window, off its default (:990): DISPUTE the verdict wording
The record's own Q6 says the display's expected text is typed ("60 ms", "Analysis settings: Early fill window 85 ms", "Resets: Early fill window 85 ms") and its Q9 says those strings are copies of the display's format. That is a Q4/Q6/Q9 fault, so the verdict cannot be "Investigate, claim not checked only". It should be Rewrite (read the value part from `describe_settings(...).early_fill` and the name from `kEarlyFillWindowName`, library_parts.h:50, and check what the label and tooltip are built from, not typed text), with Investigate for the capture (the settings button and Reset tooltip with the window off 60). The code at library_dialogs.cpp:80, 100, 363 matches the record's description.

---

## Splits

I found no new split. Lines that are two answers to one question, each one line:
- Two bulk ways to apply answers to library rows, `set_summaries` (by position) and `set_summaries_for` (by chart id), library_model.cpp:273 and 286; both end in `apply_summary`, so they agree by construction, and no test compares them. The record sees only the second.
- The default window is typed `60` in test_config.cpp:255-256, 258 and read as the constant at :249 in the same file, and `kMaxEarlyFillWindowMs` is `int(kHitWindowMs)` (model.h:115) while the tests type `85`; all follow the ruling lines 55-57, so not a conflict, only the same fact stated twice.

## Not reached

None. Every record was read against its test and the production code it names.
