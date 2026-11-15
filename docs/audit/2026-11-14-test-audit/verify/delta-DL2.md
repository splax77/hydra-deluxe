Agent id: a6e4ae1a6fb09efad (task T5-V-DL2)

# Verify of records/delta-DL2.md

Checked at `5ec682c7` by reading. Nothing run, nothing built. Every mark below is "by reading" unless it says otherwise; no run in the records folders proves any of these claims.

What I opened: `docs/agents/test-rubric.md`; the four files' diffs from `c10c7f54` to `5ec682c7`; `tests/test_store_open.cpp` 183-425, 440-560, 565-680, 825-1096; `tests/test_store_library.cpp` 733-925; `tests/store_file_fixtures.h` and `tests/store_lock_holder.h` in full; `tests/old_layout_fixture.h` 30-55; `src/store/library_file.cpp` 60-135; `src/store/record_store.cpp` 790-940, 965-990, 1069-1100, 1535-1548, 1958-2075 and the `record_tables_sql` block; `src/store/record_store.h` 505-556, 696-712, 955-975; `src/core/model.h` 98-118; `src/store/stored_versions.h` 50-70; `tests/wait_util.h` 15-25; rulings `code-and-engine.md` lines 23-24, 50, 56-57; D250 and D344 heads; `records/A32.md` headings, record 5 and verdict lines; `testdata/store/README.md` v2.12.1 section. Not opened: `docs/agents/test-rubric-examples.md`, `tests/test_store_records.cpp`, the callers' bodies in `test_app_*.cpp`, the adding commits themselves (I took the records' commit subjects as theirs, not mine), `docs/clone-hero/` (no chart claim is in this slice).

## Scope

Test cases new, changed or removed between the two commits, found by my own diff: `test_store_open.cpp` has 10 (changed: 183, 269, 353, 383, 570 and 649 through the helper `check_converted_file`, 898; new: 992, 1033, 1059; removed: none). `test_store_library.cpp` has 7 new (766, 776, 806, 823, 860, 886, 905; removed: none; the other hunks there are includes and `using` lines). `store_file_fixtures.h` has 2 new types and `store_lock_holder.h` has 2 new items. That is 21 in scope and 21 records. No case is missing a record. No record is for an unchanged test. The record's own scope list matches mine, line for line.

## Items that run through several records

**Redundant added line, checked.** The records say the added `CHECK(profiles_have_early_fill(path))` at 295, 372, 398, 543 and 925 is true by construction. I read it: `kind_of` returns `LacksEarlyFillColumn` before `Current` (library_file.cpp:112-115), and in 269, 353, 383 the seed is a file this build wrote (so the profiles table already has the column); in 570/649 the seed drops the profiles table (old_layout_fixture.h:48) and `create_result_tables` makes it new (record_store.cpp:875). So 295, 372, 398 and 543 hold. For 925 the line is implied by 926 only if the classifier's own column check is right; the independent reader there is 1000, on the same real file in the next test. I agree 925 can go, for that reason, which the record does not give.

**The same logic reaches lines the records keep (not raised by the records).** `charts_have_album` at 371, 395, 542 and 924 sits beside a `Current` check in the same way: `kind_of` names `LacksAlbumColumn` before `Current`. In 395 and 542 the seed has the album already, so those two are true by construction exactly as 398 and 543 are. The records delete one and keep the other. One line should follow one rule: delete both, or keep both.

**Splits.** One line each.
- S1. `HeldSaveGroup` (store_file_fixtures.h:38) and `StoreLockHolder` (store_lock_holder.h:18) do one job with different error handling, and `let_go()` is an action in one and a getter in the other. The record found this. I confirmed the exception difference: `begin_save_group` can throw (record_store.cpp:1069-1076, `ctl("BEGIN")` inside a try that rethrows), and in `StoreLockHolder` the thread lambda (store_lock_holder.h:21-29) has no try, so a throw there ends the process.
- S2. `remove_db` (test_store_open.cpp:81, also removes the set-aside file) and `StoreFile::remove_all` (store_file_fixtures.h:30). The record noted it and did not call it a split. It is one.
- S3. A v2.12.1 profile row was searched by a build whose stamp reads "2.8.0+efill85" (test_store_open.cpp:876; D250: orange up to 85, white 60), and the rebuild files it under the new default window (record_store.cpp:926 and the comment "no older file recorded one" at 917-918; `kDefaultEarlyFillWindowMs` 60, model.h:112). No ruling or decision names that choice. The record raised this in records 8 and 9. I found no ruling either: `code-and-engine.md` 56-57 says "default 60ms, supports 0ms to 85ms" for the setting, and nothing for old rows.
- S4. D344 item 1 (decision-log line 1989) says the early-fill window "is the hit window on both sides" (85). The filed lines 56-57 (same day, same session a3ba79d1) say the setting's default is 60. The records do not mention this. I read only D344's first three lines and did not check whether a later D-number settles it. It is a different slice's business, but it sits under the same value that records 8 and 9 pin.
- S5. The stamp value `'5'` is typed as the "pre-album" stamp in three places (test_store_open.cpp:348; test_store_library.cpp:794 and 876). `kChartMetaStamp` accepts only 6 (stored_versions.h:68). This is a typed copy of a fact about the stamp, not two answers; I list it because the records mention it twice and never as one thing.

**Golden files.** No record proposes a saved expected-output file. Record 7's rewrite reads the real file's own tables before the open and compares after; that reads the input, it does not save expected output.

**Question 2 callers the records missed.**
- Record 15 (reader's chart index) lists callers `app_state.cpp:667/669`, `dm_jobs.cpp:51`, `library_jobs.cpp:824`. It missed `src/app/dm_report.cpp:92`, `src/app/fill_report.cpp:218` and `:400`, and `src/app/report.cpp:215`, all `store.chart_index()` on a `StoreReads&` (report.cpp:94, 196-210; dm_report.cpp:76). They are screens that read the index, so the list was short, not wrong.
- Record 16 left open whether `report.cpp:201` reads through the reader. It takes `store::StoreReads&` (report.cpp:200), which the reader implements, so it is the same read.
- Record 19 (HeldSaveGroup) lists `test_store_records.cpp:400` and misses `test_store_records.cpp:149`.
- Records 11, 12, 13, 14, 17, 18, 20, 21: callers match my grep (`app_state.cpp:197, 235, 613, 874`; `library_jobs.cpp:63, 74, 92`; `dm_jobs.cpp:51`; the test callers).

## Records

### 1. classify_library_file names each generation of library file (test_store_open.cpp:183). Agree: Investigate, claim not checked only.
Claim (by reading): the new subcase (225-237) types three one-line tables; 228 `LacksEarlyFillColumn`, 232 `Current`, 236 `LacksAlbumColumn`. Each matches `kind_of` (library_file.cpp:99-115). Q2: sole `src/` caller is `RecordStore`'s constructor at record_store.cpp:799, plus its kind switch at 877-887. Q3: real input would be a real release's file; the real v2.12.1 file classifies `LacksAlbumColumn` (album named first), so no real file of the in-between generation exists in `testdata/store`. Delta line: A32 record 3's verdict stands. Q8: 228 red if the new branch is removed or its column name changes; 232 red if the check is turned around; 236 red if the early-fill check is moved above the album check. The record's side note is right: the comment at 233 says "older generations", plural, and only the album one is asserted; swapping the failure-column check with the early-fill check stays green. Splits: none new.

### 2. a 2.8.0 file without the failure columns opens and gains them in place (:269). Agree.
Line 295 is true by construction (see the shared item). Q8 for the rest, by reading: 288 old record equal, 289 count, 292 failure round trip, 294 kind, 296-298 old rows' columns NULL and path rows kept, 302-303 second open. The verdict reads "Rewrite ... otherwise Investigate": two verdicts. The one-line deletion is the Q8 fault; the test as a whole is still Investigate (claim not checked only; needs the real 2.8.0 file the record names as (b)). Delta line: A32 record 4 stands.

### 3. a file whose charts table has no album column (:353). Agree.
The seed writes `'5'` at 348, which `kChartMetaStamp{6,{6}}` does not accept, so line 364 (empty cache) is a real consequence of the stamp. 372 true by construction; see also 371 in the shared item. A32 record 6 stands. Real input needed: a real pre-album file with charts rows (none in `testdata/store`).

### 4. a 2.8.0 file gains the failure columns and the album column in one open (:383). Agree.
Same shape. 392-397 are the real assertions, 398 goes. `set_up_schema` 876-887 is the only owner. Note 395 (`charts_have_album`) is as redundant as 398 here.

### 5. the first open of a 2.2.0 to 2.7.0 file turns its results into records (:570). Agree.
543 true by construction (seed drops profiles; `create_result_tables` makes it fresh). I add that 542 (`charts_have_album`) in the same helper is as redundant as 543, see the shared item. Real input needed: a real 2.2.0-2.7.0 file; `testdata/store` has only v2.1.0 and older and v2.12.1.

### 6. a stop asked mid-conversion (:649). Agree.
Only change is through the helper (671). The cancel claim is checked on a real SQLite transaction and a real callback; the file is hand-built, so Q3 stays "claim not checked on real input". A32 record 12 stands.

### 7. a library file v2.12.1 wrote opens, gains the album column, and keeps its records (:898). Agree: Rewrite as A32 asked, plus delete 925.
Q3 holds on a real file: `testdata/store/v2.12.1_library.db`, written by release v2.12.1 (README), read by raw SQL before (900-902) and after (928). Stale expectation: the file's profile stamp "2.8.0+efill85" (876) against `kResultsStamp` "3.1.2+efillwindow" (stored_versions.h:55), which I opened. The typed 20 per-chart values are a copy of the file's contents; the A32 rewrite (read the rows before, compare after) is right and does not propose a saved file. 925 is covered by 1000 on the same file (see above).

### 8. the v2.12.1 file's profiles table is rebuilt with the early-fill window (:992). Agree: Investigate.
Q3 on real input for "every row and record kept" holds: real file, raw SQL before and after (`older_profile_columns` lists id, chartmode, sp_cap, the five lens columns, hyversion, hex(rules_fp); the five lens columns match `kLensColumnsBeforeEarlyFill`). Q8 per line, by reading: 998/1005 red if the rebuild detaches records; 1003 red if any column or the id changes; 1004 red if the row is filed under another window; 1013 Stale (the stamp's answer); 1017 NotAnalyzed at 85; 1026-1029 two windows save as two rows. The open question the record raised stands: no ruling or decision names "filed under the default window". One more point the record does not make: 1004 and 1051 compare against `Lens{}.efill_window_ms`, the same expression the rebuild inserts (record_store.cpp:926), so a change of the default moves test and code together and nothing flags it. The record praised "read from Lens{}, not retyped"; the rubric's "Named inputs" section says an expected value that comes out of production code is a copy. That is part of the same question to the user, not a separate one. The test also bundles three promises (rebuild keeps rows; window is part of the key; old rows read Stale). The 85 is `kMaxEarlyFillWindowMs` = `static_cast<int>(kHitWindowMs)` (model.h:115), so the record's note about 1016 versus 1021 is not a split.

### 9. a file whose profiles table lacks the early-fill window (:1033). Agree: Investigate.
Input is a file this build wrote, rewritten by SQL (`drop_early_fill_column`), so no real input. 1049 is redundant with 1048, but it is the only raw-SQL reader of the column in this test (the same classifier predicate gives 1037 and 1048); the real-file test's 1000 covers the same property. I accept deleting it. 1054-1055 (old table dropped) red if `DROP TABLE` is removed. Same 1051 default-window note as record 8.

### 10. an open that cannot rebuild the profiles table (:1059). Agree: Rewrite.
Confirmed by reading: `add_early_fill_column` runs `ALTER TABLE profiles RENAME TO profiles_before_efill` first (record_store.cpp:916), and the stray table the test made has that name, so the rename fails before anything changed; 1079-1086 all hold with `in_transaction` removed. Only 1076 (error kind) can go red. I did not work out which later step can be made to fail by test input (the new table's SQL has no named index to collide with that I could see); the record's rewrite says "make a later step fail" without a concrete way. That is the gap in the rewrite, not in the verdict.

### 11. the reader lists a chart's copies as the writer does (test_store_library.cpp:766). Agree: Investigate, claim not checked only.
Owner confirmed: the header says each method's rule lives in `RecordStore`'s comment of the same name (record_store.h:518), so writer-versus-reader is owner-versus-reader, not two Hydra functions with no owner. 772 non-empty, 773 the equality. The comparison tests the connection, as the record says. Real input needed: a scanned library.

### 12. the reader lists the library by name as the writer does (:776). Agree: Investigate, claim not checked only.
787/784 timing present, then the `'5'` stale-stamp case (S5). 790 compares the unsorted list as a set, which is right since order is not promised. Real input needed as 11.

### 13. a reader's chart list does not wait while a save group is open (:806). Dispute: Keep is ruled out by the record's own Q9; verdict Rewrite (small).
Q3 is met on the real mechanism: a real helper thread, the real store mutex taken by `begin_save_group` (record_store.cpp:1070), a real second SQLite connection in `StoreReader`. Q8 by reading: with the bug "the reader takes the store's mutex", the reads wait until the helper hits `kWaitCap` (30 s, wait_util.h:20), `held()` is false and 818 goes red; 819-820 red if the reader returns empty. Why not Keep: the record's Q9 admits one boolean covers two reads, so a red does not say which read waited; the rubric says an admitted fault rules Keep out. Fix is small: check `held()` after each read. A second point the record does not raise: the group is empty (`begin_save_group` runs a plain `BEGIN` and nothing is saved), so the test models the mutex, not a batch with writes in flight; the claim in the name ("while a save group is open") is still true.

### 14. a reader sees a rescan once it is saved (:823). Agree: Investigate, claim not checked only.
832-834 compare reader to writer after a rebuild; red if the reader kept a read transaction open between calls (`ReadTransaction` per call, record_store.cpp:2030-2046). Real input needed: a real library rescanned with a different set.

### 15. the reader builds the chart index the writer builds (:860). Agree: Investigate, with the missed callers above (dm_report, fill_report, report).
`check_same_index` compares ids, copies and the naming row per id, and `song_ids()` equality fixes the order. 868-871 are the non-vacuity guards.

### 16. a reader's chart index does not wait while a save group is open (:886). Dispute, same as 13.
`report.cpp:201` reads count through `StoreReads&` (answered above). Same one-boolean-for-two-reads admission, same empty group.

### 17. an own reader lists and indexes the library as the writer does (:905). Agree: Investigate, claim not checked only.
Callers `dm_jobs.cpp:51`, `library_jobs.cpp:824` confirmed. The record is right that 922-924 under a hold cannot show a hold's promise, because nothing writes between the reads; a hold that did nothing passes. Whether `test_store_records.cpp` carries that promise: not read by me either.

### 18. StoreFile (store_file_fixtures.h:22). Agree: Keep as a helper.
Callers by my grep: `test_store_library.cpp` (7 cases), `test_store_records.cpp` (31 uses), `test_preview_controller.cpp` (1 use). No `src/` caller. Split S2 against `remove_db` stands.

### 19. HeldSaveGroup (:38). Dispute the form: "Keep (as a helper), under the split above" is Keep with a flag.
The code reads as the record says: both thread bodies are in try blocks, and a throw from `begin_save_group` makes the constructor wait the full 30 s then throw. Callers: the record's list plus `test_store_records.cpp:149`. The split with `StoreLockHolder` (S1) is open; under the rubric the verdict is one of four, so either Rewrite with 20 or Investigate (the split goes to the user), not Keep.

### 20. StoreLockHolder (store_lock_holder.h:18). Agree: Rewrite (merge with HeldSaveGroup, catch the thread's exceptions).
Confirmed: lines 21-29 have no try; `begin_save_group` can throw. The merge is a split-closing change; the main session picks the exit.

### 21. returns_while_held (store_lock_holder.h:61). Dispute the form, same as 19.
By reading, the helper is right: a call that waited returns only after the helper's `let_go_ = true` (set before commit), so `!holder.let_go()` is false; a call that returned at once sees `let_go_` still false. `~StoreLockHolder` releases and joins if `call()` throws. "Keep, to move onto the merged holder when the split is closed" is Keep with a flag.

## Question 3 for the whole slice

Claims about files and the store (1-12, 14, 15, 17): real input exists only for 7 and 8, the v2.12.1 file; the independent reader is raw SQL. The rest run on hand-built seeds or fixtures and stay "claim not checked", so the right verdict is Investigate, and I agree with each of those. Claims about threads and locks (13, 16, 19, 20, 21) run on the real mechanism, so their Q3 holds; the disputes are for other reasons. Nothing here is about what a screen shows, so no capture is needed.

## Tally

Agree with 17 verdicts (1-12, 14, 15, 17, 18, 20). Dispute 4 (13, 16, 19, 21). No in-scope case without a record. No record not reached.
