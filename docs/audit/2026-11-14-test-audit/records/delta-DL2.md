# Delta DL2 records: test_store_open, test_store_library, store_file_fixtures.h, store_lock_holder.h

Task T5-DL2, delta round. Read-only audit by `docs/agents/test-rubric.md`. Diff base `c10c7f54`, head `5ec682c7`. Nothing run, nothing built.

In scope: `tests/test_store_open.cpp` 10 test cases (7 changed, 3 new, 0 removed); `tests/test_store_library.cpp` 7 test cases (7 new, 0 removed; the file's other 24 cases are untouched, only includes and `using` lines changed above them); `tests/store_file_fixtures.h` 2 new types (StoreFile, HeldSaveGroup); `tests/store_lock_holder.h` 2 new items (StoreLockHolder, returns_while_held). 21 records in all.

## What every record below shares

Read once: `docs/agents/test-rubric.md`; `docs/agents/brief-preamble.md`; the full diffs of the four files; `tests/test_store_open.cpp` 50-135, 183-425, 440-690, 818-1096; `tests/test_store_library.cpp` 420-510 and 730-925 (the diff hunk and its helpers); `tests/store_file_fixtures.h` and `tests/store_lock_holder.h` in full; `tests/wait_util.h` 1-40; `tests/library_fixtures.h` 14-29; `tests/old_layout_fixture.h` by grep; old records `records/A32.md` 63-142, 160-222, 308-325. Production: `src/store/library_file.cpp` 63-132; `src/store/library_file.h` by grep; `src/store/record_store.cpp` 860-960 (set_up_schema tail, add_album_column, add_early_fill_column, add_failure_columns), 1535-1547 (reader, open_own_reader), 1958-2068 (StoreReader, OwnReader), 1903-1948 by grep (list_library_on, library_copies_on); `src/store/record_store.h` 360-390, 490-645, 960-975; `src/store/stored_versions.h` 55, 68 by grep; `src/core/model.h` 100-118. Commits by subject and body: 97eb4f73, 5ea45a09, 856dfbd2, a311dde7, 5a998908, 883ac36f, eef15e6a, 95a499ca, 9da33c72. Rulings and decisions: `docs/agents/rulings/code-and-engine.md` lines 23-24, 56-57 (grep); `docs/audit/2026-10-03-fix-decisions.md` D250 (line 1021, head) and D344 (line 1987, items 1-2); `testdata/store/README.md` file list (grep) and the diff of 97eb4f73 to it. Session cards: grep of `docs/sessions/index.md` for session a3ba79d1 found no card. Anything else is written "not read" in its record. Not read in any record: `tests/test_store_records.cpp` (DL1's file), the callers' bodies in `test_app_*.cpp` and `test_library_jobs.cpp`, `docs/clone-hero/` (no Clone Hero rule is typed in any of these tests; no chart is involved).

**Line numbers.** Test-case start lines are exact. Line numbers inside the bodies of records 11 to 17 (test_store_library.cpp) were counted from the diff hunk and can be off by up to 3; the test_store_open.cpp, header and production lines were read with numbers.

**Question 3 for all of them.** No test here is about a chart, so `records-engine/` and `records-screen/` do not apply. Every claim is about a file, the store or a lock. The real input that exists is `testdata/store/v2.12.1_library.db`, written by the real v2.12.1 release (README). Raw SQL on a file is the independent owner. No real library file from a release between the album column and the early-fill window column is checked in (the README lists v1.8.4, v2.0.0, v2.1.0 and v2.12.1 only), and no real library with a non-empty `charts` table is checked in. Those are the captures named below as "needed".

**Two findings that apply to several records.**

Redundant added line (rubric Q8). Commit 97eb4f73 added `CHECK(profiles_have_early_fill(path))` after each open-and-reach-Current check. `classify_library_file` returns `Current` only when the profiles table has the column (`kind_of`, library_file.cpp:112-115: `lacks_early_fill` returns `LacksEarlyFillColumn` first). So where the line sits next to `classify_library_file(path) == LibraryFileKind::Current`, deleting it hides no bug (reasoned). It is also true by construction in four cases, because the seed file already has the column: 269 (the seed is a file this build wrote, only the failure columns dropped, line 279), 353 and 383 (`seed_without_album`, line 340, same), and 570/649 (`seed_v220_layout` drops the profiles table, old_layout_fixture.h:48, and `create_result_tables` makes it new with the column, record_store.cpp:875). Only 898 starts from a file that truly lacks the column, and there the real file's `LacksAlbumColumn` kind is named first.

Split: two helpers for one job. `HeldSaveGroup` (`tests/store_file_fixtures.h:38`) and `StoreLockHolder` (`tests/store_lock_holder.h:18`) both open a save group on a helper thread, hold the store's lock, and let go on a flag or at `kWaitCap`. They differ: `HeldSaveGroup` catches exceptions in its thread and `let_go()` is an action; `StoreLockHolder` catches nothing and `let_go()` is a getter. Callers: `HeldSaveGroup` in test_store_library.cpp (two cases), test_store_records.cpp:400, test_app_click.cpp:553, test_app_reports.cpp:961 and 982; `StoreLockHolder` in test_library_jobs.cpp:698 and, through `returns_while_held`, in test_app_library.cpp:353, 1246, 1266, 1280, 1299. Commit 856dfbd2 merged two copies of one into `HeldSaveGroup`, and 5a998908 and a311dde7 made `StoreLockHolder`; nobody merged those two.

---

## 1. classify_library_file names each generation of library file (tests/test_store_open.cpp:183)

Delta round: changed, replaces records/A32.md record 3
Read: lines 183-265; `kind_of` and `lacks_early_fill` (library_file.cpp:71-116); commit 97eb4f73 subject and body.
1 Claim: Each layout a release wrote is named by its tables and columns: no file, empty file or `:memory:` is Fresh; this build's file is Current; 2.8.0's records table is LacksFailureColumns; a charts table without album is LacksAlbumColumn; a profiles table without the early-fill window column is LacksEarlyFillColumn; the 2.2.0 to 2.7.0 results table is Convertible; older evidence is TooOld; junk is Unreadable. New: the LacksEarlyFillColumn subcase (lines 225-237).
2 Owner: `classify_library_file` and `kind_of` (library_file.cpp:77-116). Callers in `src/`: the `RecordStore` constructor, `set_up_schema` (record_store.cpp:760-ish, not re-read this round) and the kind switch at record_store.cpp:876-887. Engine; no display.
3 Real chart: claim not checked. The new subcase types three one-line tables. No real file of a release that has the album column and not the early-fill column is checked in. Needed: a real library file from a release between the album column and the early-fill window (the v2.12.1 real file classifies LacksAlbumColumn, because album is named first).
4 Direct: 228 profiles without the column is LacksEarlyFillColumn; 232 profiles with it is Current; 236 charts without album plus profiles without the column is LacksAlbumColumn. The rest are the old subcases (A32 record 3).
5 Reason: table and column names are typed from `kind_of`, so the test partly copies the code's predicate (as A32 said). `kEarlyFillWindowColumn` is read from the owner (library_file.h:55), not retyped, which is better than the older subcases.
6 Display: no.
7 Failure: 97eb4f73 ("the older kinds end with the column"): a file read as the wrong kind and opened without its rebuild. Still possible.
8 Deleted: 228 (the new branch of `kind_of` removed: the profiles table never rebuilt); 232 (the column check turned around); 236 (the order of the checks changed so the early-fill kind wins over the album kind, which would skip the album add). The comment at 233 says "the older generations are named first" in the plural, but only the album generation is asserted; the failure-column generation against the early-fill kind is not (reasoned: swapping those two checks would stay green).
9 Stand-in: none.
10 Same rule: tests 2, 3, 4, 7 and 8 below run each kind through the real open; this is the one place that states each classification (as A32 said).
11 Real: yes for every kind.
Verdict: Investigate: claim not checked only. Needs a real library file from a release that has the album column and not the early-fill column. The old verdict (Investigate, claim not checked only, A32 record 3) still holds; the new subcase adds nothing that changes it.
Side note: the subcase title at line 225 and 206 are fine; the plural in the comment at 233 promises one more assertion than the code makes.

## 2. a 2.8.0 file without the failure columns opens and gains them in place (tests/test_store_open.cpp:269)

Delta round: changed, replaces records/A32.md record 4
Read: lines 269-306; `add_failure_columns` (record_store.cpp:938-951); `kind_of` 99-116.
1 Claim: A library from before a stored failure existed opens, gains both failure columns, reads its old record as before, can then store a failure, and a second open changes nothing. Changed line: 295 also expects the profiles table to have the early-fill window column afterwards.
2 Owner: `add_failure_columns` (record_store.cpp:938), called from `set_up_schema` (line 877) only. Engine.
3 Real chart: claim not checked. The 2.8.0 table is made by `ALTER TABLE DROP COLUMN` on a table this build wrote (lines 279-280). Needed: real file (b), a library a real 2.8.0 release wrote.
4 Direct: 288 old record equals saved record; 289 one record; 292 failure reads CantAnalyze; 294 kind Current; 295 profiles have the early-fill column; 296-297 old record has both failure columns NULL; 298 path rows unchanged; 302-303 second open reads both.
5 Reason: "mode", 4, "real", "failed" are free inputs.
6 Display: no.
7 Failure: as A32 record 4: a 2.8.0 user's open throwing or losing rows. Still possible. The new line 295 has no failure of its own to name: the seed already has the column (the file was written by this build, line 275-277, then only the failure columns dropped), and line 294 already implies it.
8 Deleted: 288, 296-298 as in A32. 295: none; true by construction (the seed's profiles table is this build's) and implied by 294 (`kind_of` reaches Current only with the column). The line goes (reasoned).
9 Stand-in: none.
10 Same rule: 3 below runs the album upgrade, 4 below combines the two; 2b (A32 record 5) fails the same upgrade.
11 Real: yes.
Verdict: Rewrite. Delete line 295. What is left is the old record's verdict: Investigate, claim not checked only, needs real file (b).
Today it catches: old rows lost or changed on the failure-column upgrade (kept); nothing is lost by dropping 295.

## 3. a file whose charts table has no album column opens and gains it in place (tests/test_store_open.cpp:353)

Delta round: changed, replaces records/A32.md record 6
Read: lines 329-381; `seed_without_album` (340-349); `add_album_column` (record_store.cpp:894-907).
1 Claim: A library from before the album column opens, gains the column, keeps its rows and records with an empty album, hands back no rescan cache, takes the album a rescan writes, and a second open reads it. Changed line: 372 expects the profiles early-fill column too.
2 Owner: `add_album_column` (record_store.cpp:894) and the chart-meta stamp (`kChartMetaStamp{6,{6}}`, stored_versions.h:68). Called from `set_up_schema` (880-882). Engine.
3 Real chart: claim not checked. Needed: real file (c), a library a real pre-album release wrote with a non-empty charts table.
4 Direct: 362 one library row; 363 album empty; 364 cache empty; 365 record Ready; 369 album written; 371 column present; 372 early-fill column present; 373 Current; 377-378 second open reads the album and the cache has the row.
5 Reason: the stamp '5' typed at 348 is a value `kChartMetaStamp` does not accept (accepted list {6}, stored_versions.h:68); the value is the pre-album stamp by history only. 'Album A' is a free input.
6 Display: no.
7 Failure: as A32 record 6. Still possible. Line 372 names no failure: `seed_without_album` writes the seed with this build, so the column is there from the start; 373 implies it.
8 Deleted: as A32 record 6 for the old lines. 372: none (true by construction, implied by 373). The line goes (reasoned).
9 Stand-in: none.
10 Same rule: test 4 below combines this with the failure columns; test 7 below does it on the real file.
11 Real: yes.
Verdict: Rewrite. Delete line 372. The rest keeps the old verdict: Investigate, claim not checked only, needs real file (c).
Today it catches: the album upgrade losing rows or the rescan cache logic (kept).

## 4. a 2.8.0 file gains the failure columns and the album column in one open (tests/test_store_open.cpp:383)

Delta round: changed, replaces records/A32.md record 7
Read: lines 383-401; `set_up_schema` 876-887.
1 Claim: A 2.8.0 file that also lacks the album gets both upgrades in one open, keeping its record and library row. Changed line: 398 expects the early-fill column too.
2 Owner: `set_up_schema` (record_store.cpp:876-887), constructor only. Engine.
3 Real chart: claim not checked. Needed: real file (b) with a charts row.
4 Direct: 392 record Ready; 393 one library row; 395 album present; 396-397 both failure columns present; 398 early-fill column present; 399 Current.
5 Reason: the 2 at 397 is the number of failure columns (copies `kFailureColumnsSql`); as A32 said.
6 Display: no.
7 Failure: the upgrades interfering (the `||` chain at 880-886 dropping a kind). Still possible. 398 names none: the seed has the column; 399 implies it.
8 Deleted: 392-397 as A32 record 7. 398: none, the line goes (reasoned).
9 Stand-in: none.
10 Same rule: tests 2 and 3 above state each upgrade alone.
11 Real: yes, a 2.8.0 user can lack both.
Verdict: Rewrite. Delete line 398. Old verdict otherwise holds: Investigate, claim not checked only, needs real file (b).
Today it catches: the two upgrades running together (kept).

## 5. the first open of a 2.2.0 to 2.7.0 file turns its results into records and keeps every status (tests/test_store_open.cpp:570)

Delta round: changed, replaces records/A32.md record 9
Read: lines 440-614 including `seed_v220_layout`, `check_converted_rows`, `check_converted_file` (539-548); `tests/old_layout_fixture.h` lines 45-48 by grep; `add_early_fill_column` (record_store.cpp:909-936).
1 Claim: As A32 record 9. Changed through the shared helper `check_converted_file`, which now also expects the profiles early-fill column (line 543).
2 Owner: `convert_results` (record_store.cpp:959) and `set_up_schema`. Called from the constructor only. Engine.
3 Real chart: claim not checked. The input is the hand-built seed. Needed: real file (a), a library a real 2.2.0 to 2.7.0 release wrote (README says none is checked in).
4 Direct: as A32 record 9, plus 543 (early-fill column present after the conversion).
5 Reason: as A32 record 9; the counts kSeedProfiles 5 and the others are the seed's own counts.
6 Display: no.
7 Failure: as A32 record 9. 543 names none.
8 Deleted: the old lines as A32. 543: none. The seed drops the profiles table (old_layout_fixture.h:48) and `create_result_tables` makes it fresh with the column (record_store.cpp:875), and 541 (`Current`) already implies it. The line goes (reasoned).
9 Stand-in: counts at 585-591 stand in for "the right rows"; per-row statuses are checked item by item in `check_converted_rows`.
10 Same rule: test 6 below calls the same helper.
11 Real: yes, a 2.2.0 to 2.7.0 user.
Verdict: Rewrite. Delete line 543 from `check_converted_file`. The old verdict otherwise holds: Investigate, claim not checked only, needs real file (a).
Today it catches: wrong status or lost rows in the conversion (kept).

## 6. a stop asked mid-conversion leaves the results table whole and throws Cancelled (tests/test_store_open.cpp:649)

Delta round: changed, replaces records/A32.md record 12
Read: lines 649-673; `check_converted_file` (539-548).
1 Claim: As A32 record 12. Changed only through `check_converted_file` at line 671 (now with the early-fill column check at 543).
2 Owner: `convert_results`' transaction and `report`. Constructor only. Engine.
3 Real chart: claim not checked on a real 2.2.0 to 2.7.0 file; the mechanism (real SQLite transaction, real callback) is real. Needed: real file (a).
4 Direct: as A32 record 12.
5 Reason: `rows_done == 2` is a free input.
6 Display: no.
7 Failure: a stopped open leaving half a conversion. Still possible.
8 Deleted: as A32 record 12. 543 (reached through 671): none, see record 5.
9 Stand-in: none.
10 Same rule: test 5 above.
11 Real: yes, a user pressing stop during the startup conversion.
Verdict: Rewrite. The only change is that the shared helper's line 543 goes (see record 5); the old verdict otherwise holds: Investigate, claim not checked only, needs real file (a).
Today it catches: a stopped conversion leaving partial state (kept).

## 7. a library file v2.12.1 wrote opens, gains the album column, and keeps its records (tests/test_store_open.cpp:898)

Delta round: changed, replaces records/A32.md record 18
Read: lines 833-930; `check_v2121_rows`, `text_on_file`; `testdata/store/README.md` file list.
1 Claim: As A32 record 18. Changed: line 925 also expects the profiles table to have the early-fill column after the open.
2 Owner: constructor with `OldFilePolicy::SetAside`, `add_album_column`, `add_early_fill_column`, `lookup`. The same open runs `add_early_fill_column` now, because the real file lacks both columns (library_file.cpp:104-114; the album is named first). Engine.
3 Real chart: Engine: the real file `testdata/store/v2.12.1_library.db`, written by release v2.12.1, classified LacksAlbumColumn. Claim: it opens, gains the album column and, in this build, the early-fill column, keeps its three records, reads each Stale. Checked on real input; raw SQL (`check_v2121_rows`) is the independent reader.
4 Direct: 871-889 (via `check_v2121_rows`, called before and after) pin 3 records, 7 path rows, 1 profile, "Expert Pro Drums, 2x Bass", sp_cap 4, hyversion "2.8.0+efill85", per chart pathcount, stars, notecount, best path, score, actcount; 910-922 record count, library count 0, each lookup Stale with `stale_build`, no ready records; 924 album column; 925 early-fill column; 926 Current; 928 the pinned rows again.
5 Reason: the typed per-chart numbers are the real file's contents (comment 837-838 says "read off the one run that made it"); as A32 said they are snapshots of the file, a typed copy. The Stale expectation comes from the stamp: the file's "2.8.0+efill85" is not accepted by `kResultsStamp` ("3.1.2+efillwindow", stored_versions.h:55).
6 Display: no.
7 Failure: as A32 record 18. Still possible.
8 Deleted: 928 (second call to `check_v2121_rows`) catches rewriting of records on the upgrade (reasoned); 910, 912, 919-922 the lookup and count results; 924 the album add; 926 the kind. 925: none, implied by 926 (`Current`), see the shared finding. The line goes.
9 Stand-in: counts at 871-873 stand in for "the same rows" (A32).
10 Same rule: test 3 above (album add on a hand-built file) and test 8 below (the early-fill rebuild on this same file).
11 Real: yes.
Verdict: Rewrite. A32's rewrite still stands: read every table's rows from the file before the open and compare after, instead of typing 20 values; delete line 925. Keep the real file and the album and kind checks.
Today it catches: records lost or rewritten on a v2.12.1 user's first open (kept); the typed per-chart values go.
Side note: the name holds commas (line 898).

## 8. the v2.12.1 file's profiles table is rebuilt with the early-fill window, keeping every row (tests/test_store_open.cpp:992)

Delta round: new
Read: lines 932-1031; `add_early_fill_column` (record_store.cpp:909-936) and its header comment (record_store.h:960-975); `Lens` (record_store.h:129-150) by grep; model.h 100-118; commit 97eb4f73 body and README hunk; D344 items 1-2; rulings lines 56-57; D250 head.
1 Claim: The first open of the real v2.12.1 library rebuilds its profiles table with the early-fill window column, keeping the profile row's id and every other column and all three records; files the row under the default window; reads the old record Stale (not Not analyzed) at the default window and Not analyzed at the 85 ms window; and two profiles that differ only in the window can then both be saved.
2 Owner: `add_early_fill_column` (record_store.cpp:909), called from `set_up_schema` (885-887) only. `Lens::efill_window_ms` default is `kDefaultEarlyFillWindowMs` (model.h:112); the 85 is `kMaxEarlyFillWindowMs` (model.h:115). Engine. The rule "old rows are filed under the default window" has one place, line 926.
3 Real chart: the input is real (`testdata/store/v2.12.1_library.db`) and the reader is independent (`older_profile_columns`, raw SQL before and after), so "every row and record kept" is checked on real input. The rule "an old row belongs under the default window" is not a fact of any file: the file records no window. Claim for that part: not checked; no ruling found. Searched: rulings lines 23-24, 56-57 and 69 (the user said "default 60ms, supports 0ms to 85ms" for the setting, nothing on old files), D250, D344 items 1-2 (every result reads Stale), `docs/adr` by grep, `docs/sessions/index.md` for session a3ba79d1 (no card).
4 Direct: 998 three records join a profile row before; 1003 every profile row kept column for column; 1004 one profile at the default window; 1005 three records still under a profile; 1013 old record reads Stale at the default window; 1017 Not analyzed at 85; 1026-1027 both windows' records read Ready; 1029 two 'mode' profile rows.
5 Reason: 3 and 1 are the real file's counts (README); 85 is `kMaxEarlyFillWindowMs`, a named owner constant; "2" at 1029 is the two profiles the test just made. The default window is read from `Lens{}` (not retyped). No ruling or decision I found says the rows go under the default; D344 item 2 says every saved result reads Stale, which is why the choice is invisible on screen today.
6 Display: the Stale and Not analyzed results are what the library's status column reads (`lookup_one`); the test compares with typed statuses. They come from the stamp, the store's own rule.
7 Failure: 97eb4f73: a v2.12.1 user's open losing the profile row or the records after the table is rebuilt, and results saved at two windows colliding. Still possible.
8 Deleted: 998 and 1005 the records detached from their profile on rebuild (reasoned); 1003 a column lost or an id changed; 1004 the row filed under another window; 1013 the lookup missing the rebuilt row; 1017 the window not part of the lookup key; 1026-1029 the unique key lacking the window so the second save overwrites the first.
9 Stand-in: counts at 998, 1005 and 1029 stand in for "the right rows"; 1003 compares all columns item by item. The test bundles three promises (rebuild keeps rows; window is part of the key; old rows read Stale), so a failure message names the line but not the promise.
10 Same rule: test 9 below (the same rebuild on a hand-built file); the last promise (two windows are two results) is the one commit 97eb4f73 also pinned in `tests/test_store_records.cpp` (DL1's file, not read): one verdict should cover both.
11 Real: yes, every v2.12.1 user, and any user of a build between.
Verdict: Investigate. The real-file rebuild checks (1003, 1005) are sound. The value "filed under the default window" has no ruling or decision I can find: the old records were searched under the older early-fill rules (D250's orange 85 and white 60, stamp "efill85"), not under the new 60. It is invisible today (the rows read Stale), but it decides which profile row the re-analysis writes. This goes to the user as a question: what window should an old record be filed under.
Side note: the name holds a comma (line 992). Line 1021 assigns `kHitWindowMs` (a double) where 1016 uses `kMaxEarlyFillWindowMs` (an int); both are 85 and both read the hit window.

## 9. a file whose profiles table lacks the early-fill window opens and gains it in place (tests/test_store_open.cpp:1033)

Delta round: new
Read: lines 932-990 and 1033-1057; `add_early_fill_column`; `seed_without_early_fill`, `drop_early_fill_column`.
1 Claim: A library whose profiles table has no early-fill window column, but whose other tables are this build's, opens, reads both its records as it did, ends as Current with the column, keeps every profile column, files both rows under the default window, keeps its records and path rows, and leaves no temporary table behind.
2 Owner: `add_early_fill_column` (record_store.cpp:909), `set_up_schema` (885-887). Engine.
3 Real chart: claim not checked. The input is a file this build wrote, then rewritten by SQL in the test (`drop_early_fill_column`, 962-975). Needed: a real library file from a release that has the album column and not the early-fill column; none is checked in.
4 Direct: 1037 kind before; 1039 two profiles; 1044-1046 both records Ready; 1048 Current; 1049 column present; 1050 every profile column kept; 1051 both rows at the default window; 1052 both records still under a profile; 1053 path rows unchanged; 1054-1055 the temporary table is gone.
5 Reason: 2 is the seed's own two profiles; 4 and 8 caps are free inputs. The default window comes from `Lens{}`. As in record 8, "default window" has no ruling.
6 Display: no.
7 Failure: 97eb4f73: the current-layout file without the column (a build from just before) losing rows on the rebuild. Still possible.
8 Deleted: 1044-1046 records detached; 1050 a column lost; 1051 wrong window; 1052-1053 records or paths lost; 1054 the old table not dropped (a leftover would make the next rebuild or open fail). 1049 is implied by 1048 (the shared finding): the line goes. 1048 itself: reasoned, catches a rebuild that leaves the kind unchanged.
9 Stand-in: counts 1039, 1052 and 1053 stand in for "the same rows"; 1050 compares them column by column.
10 Same rule: test 8 above covers the same rebuild on the real file; tests 2-4 cover the older kinds ending with the column. The rule "an old row is filed under the default window" is pinned here and in test 8 under one open question.
11 Real: yes in principle: any user whose build wrote the album column and not yet the window column. Whether such a release exists is not read.
Verdict: Investigate: claim not checked only, plus the shared question from record 8 (which window an old row belongs under). Needs a real library file from such a release. Delete line 1049 (redundant with 1048).

## 10. an open that cannot rebuild the profiles table fails as DatabaseUpgrade and changes nothing (tests/test_store_open.cpp:1059)

Delta round: new
Read: lines 1059-1088; `add_early_fill_column` 909-936; `in_transaction` by grep (record_store.cpp:592); A32 record 5 for the same shape.
1 Claim: When the rebuild of the profiles table fails partway, the open fails as a database upgrade error and the file is as it was: same table, same rows, same records and path rows, still the older kind.
2 Owner: `add_early_fill_column` and `in_transaction`. Callers: `set_up_schema` only. Engine.
3 Real chart: claim not checked. The failing state is a table the test creates with the temporary name (line 1065); the SQLite error and rollback are real, the file is hand-built. Needed: a real failure on a real file; for a real v2.12.1 copy, the same trick on the real file.
4 Direct: 1076 error kind; 1079 still LacksEarlyFillColumn; 1080 no column; 1081-1082 the table's SQL as before; 1083 every profile column as before; 1084 record rows; 1085 path rows; 1086 records still under a profile.
5 Reason: none typed. 
6 Display: no.
7 Failure: 97eb4f73: a half-done rebuild leaving a user with no profiles table or lost records. Still possible only if the transaction were removed.
8 Deleted: the failing statement is the first one in the transaction. `add_early_fill_column` runs `ALTER TABLE profiles RENAME TO profiles_before_efill` first (record_store.cpp:916), and that fails at once because the name is taken, before the new table, the copy or the drop. So nothing was changed when it fails, and every line from 1079 to 1086 holds even with `in_transaction` removed (reasoned). The comment at 1061-1062 says the rename "fails inside the transaction", but the rollback the claim is about is never needed. Same fault as A32 record 5.
9 Stand-in: lines 1079-1086 stand in for "a rollback undid the earlier steps" and hold on any failure at the first step.
10 Same rule: tests for the album add (test_store_open.cpp:403, locked connection) and the failure columns (308); A32 record 5 has the same fault.
11 Real: the half-way state cannot be reached by this code. The test says why the odd input is needed, but it does not reach the later steps.
Verdict: Rewrite. Make a later step fail (the copy of the rows or the drop of the old table), so the rename and the new table already exist when it fails; then the same lines 1079-1086 prove the rollback. Add that the new table is not left behind.
Today it catches: DatabaseUpgrade is thrown when the rebuild cannot start (kept); it does not catch a missing rollback (the rewrite adds that).

---

## 11. the reader lists a chart's copies as the writer does (tests/test_store_library.cpp:766)

Delta round: new
Read: lines 733-775; `check_same_list`, `timed_library`; `StoreReader::list_chart_library_copies` (record_store.cpp:2020-2025) and the writer's (2223); `StoreReads` comment (record_store.h:516-519); commit 883ac36f.
1 Claim: The store's reader gives a chart's copies in the same rows and order, field for field, as the store's own read.
2 Owner: `library_copies_on` (record_store.cpp:1938), called by both the writer (2223) and `StoreReader` (2024). The header says each method's rule lives in `RecordStore`'s comment of the same name (record_store.h:518), so the writer is the owner and the reader is compared with it. Callers in `src/` of the reader's call: `src/ui/app_state.cpp:197` and `613` (both `store->reader().list_chart_library_copies`). The writer's other callers: none outside the store found by grep (`src/ui/library_jobs.cpp:74` is the list, not the copies).
3 Real chart: claim not checked. The rows are `timed_library()`, three typed rows. Needed: a real library, a scan of `testdata/input` or a copy of the user's library, with raw SQL on the charts table as the independent reader.
4 Direct: 773 two copies; 774 reader equals writer by `check_same_list` (size, order, each field including the timing).
5 Reason: 2 is the fixture's own two rows for md5 "x".
6 Display: no (the callers read the result, they decide nothing here).
7 Failure: 883ac36f: a reader answering differently from the writer (for example dropping the timing the stamp says to read). Still possible.
8 Deleted: 773 (no copies at all, so the equality below it would pass on two empty lists); 774 a reader that returns other rows, another order, or no timing (reasoned).
9 Stand-in: the comparison is two Hydra functions, but the header names the writer as the reader's owner and both call one function on two connections, so it tests the connection, not the logic. The writer's own answer is judged elsewhere (A31).
10 Same rule: tests 12, 15 and 17 in this file compare other reads.
11 Real: yes; a library with a chart in two folders is ordinary.
Verdict: Investigate: claim not checked only. Needs a real library file (a scan of the corpus, or a copy of the user's library) read two ways.

## 12. the reader lists the library by name as the writer does (tests/test_store_library.cpp:776)

Delta round: new
Read: lines 776-804; `StoreReader::list_chart_library` (record_store.cpp:2011-2018); `kChartMetaStamp` (stored_versions.h:68).
1 Claim: The reader's by-name list, and its unsorted list as a set, give the writer's rows; and when the scan stamp is not current both give rows with no timing.
2 Owner: `list_library_on` (record_store.cpp:1904), called by the writer (2218) and the reader (2017). Callers in `src/` of the reader's list: `src/ui/app_state.cpp:235` and `874`; `src/ui/library_jobs.cpp:92` through `read_library_summaries`. Engine.
3 Real chart: claim not checked. Same hand-built `timed_library()`. Needed as record 11. The stamp case is also hand-built, by an SQL update of the meta row.
4 Direct: 786 three rows; 787 the first has timing; 788 reader equals writer; 792 the unsorted list holds the same md5s as a set; 797 then the stale stamp: 799-800 three rows with no timing; 801 reader equals writer; 802-803 copies equal.
5 Reason: '5' (line 797) is a value `kChartMetaStamp` does not accept (accepted list {6}, stored_versions.h:68); the number is the pre-album stamp by history, any non-accepted value would do. Say so in code.
6 Display: no.
7 Failure: 883ac36f: the reader skipping the timing check or reading it on another connection. Still possible.
8 Deleted: 787 (a reader and writer both dropping timing would agree); 792 the unsorted order losing rows; 800 the stale-stamp case.
9 Stand-in: the unsorted check is by set, which is right (the order is not promised).
10 Same rule: test 14 does the same for the chart index.
11 Real: yes.
Verdict: Investigate: claim not checked only. Needs a real library with timing rows.

## 13. a reader's chart list does not wait while a save group is open (tests/test_store_library.cpp:806)

Delta round: new
Read: lines 806-821; `HeldSaveGroup` (store_file_fixtures.h:38-82); `RecordStore::begin_save_group` comment (record_store.h:702-709); `StoreReader::list_chart_library`, `list_chart_library_copies` (record_store.cpp:2011-2025).
1 Claim: While another thread holds a save group, the reader's chart list and its copies come back at once, with the rows the writer read before, instead of waiting for the group to commit.
2 Owner: the reader's own connection and lock (`StoreReader`, `conn_.mutex`), apart from the store's lock that a save group keeps. Callers in `src/`: `src/ui/app_state.cpp:197, 235, 613, 874` run on the window's thread; `src/ui/library_jobs.cpp:92`. Engine.
3 Real chart: Engine: a real helper thread holds the store's lock inside a real save group while the main thread reads on the reader's own real SQLite connection. Claim: the reads return before the group is let go. Checked on the real mechanism (real thread, real lock, real second connection). The rows are the hand-built fixture, which matters only as proof that the reads returned data.
4 Direct: 818 `group.held()` was still true after both reads returned; 819-820 the two lists equal the ones read before the hold.
5 Reason: `0, -1` is offset 0 and no limit (comment at `src/ui/app_state.cpp:235`). "x" is a free input. The wait is bounded by `kWaitCap` (wait_util.h:20, D108).
6 Display: no.
7 Failure: 883ac36f and the window-freeze family: the library list read on the window's thread waiting for a batch's save group. Still possible if `reader()` returned the store itself.
8 Deleted: 818: reads that wait would only come back after the helper lets go at `kWaitCap` or the test does, and then `held()` is false (reasoned). 819-820: a reader that returns early with nothing.
9 Stand-in: one boolean covers both reads; a failure says "returned while held was false" without saying which read waited. A failure costs up to `kWaitCap` (30 s); it never hangs.
10 Same rule: test 16 below (the chart index), `tests/test_store_records.cpp:400` (a lookup under a held group, DL1's file), and the app-level `returns_while_held` cases in `test_app_library.cpp` (DL3).
11 Real: yes, a batch holds a save group while the window reads.
Verdict: Keep.
Side note: if `begin_save_group` itself throws on the helper thread, `HeldSaveGroup` swallows it and the constructor waits the full 30 s (see record 19).

## 14. a reader sees a rescan once it is saved (tests/test_store_library.cpp:823)

Delta round: new
Read: lines 823-835; `StoreReader` (record_store.cpp:1960-2046).
1 Claim: After the store rebuilds the library from a rescan, the reader's next read shows the new rows, not the ones it read before.
2 Owner: `StoreReader`'s one read transaction per call, nothing kept open between calls (record_store.h:548-552). Callers in `src/`: as record 12. Engine.
3 Real chart: claim not checked on a real library. The mechanism (a real second connection seeing a commit) is real, but the rows are the hand-built `timed_library()` and a two-row rescan. Needed: a real library, rescanned with a changed set of charts.
4 Direct: 831 the new md5s differ from the old (setup); 832 reader's list equals writer's; 833-834 reader's copies for "z" equal the writer's.
5 Reason: "z", "New Song" are free inputs.
6 Display: no.
7 Failure: a reader that keeps an old snapshot (a read transaction left open between calls). Still possible if `ReadTransaction` stopped ending.
8 Deleted: 832 (stale reader), 833-834 (copies cached by id).
9 Stand-in: none. `REQUIRE` at 831 checks the setup.
10 Same rule: test 11, 12 for equality; test 18 for a hold, which deliberately keeps one moment.
11 Real: yes.
Verdict: Investigate: claim not checked only. Needs a real library rescanned with different charts.

## 15. the reader builds the chart index the writer builds (tests/test_store_library.cpp:860)

Delta round: new
Read: lines 840-884; `check_same_index`; `StoreReader::chart_index` (record_store.cpp:2041-2046) and `chart_index_on` by grep.
1 Claim: The reader's chart index lists the same song ids in the same order, each with the same copies count and the same naming row (with timing under a current stamp, none under a stale one), as the writer's.
2 Owner: `chart_index_on` (record_store.cpp, around 1946), called by the writer and `StoreReader::chart_index`. Callers in `src/` of the reader's chart_index: `app::charts_with_result` and the plan paths at `src/ui/app_state.cpp:667, 669` (through `reads`), and `src/ui/dm_jobs.cpp:51` and `library_jobs.cpp:824` through an `OwnReader`. Engine.
3 Real chart: claim not checked. Hand-built `timed_library()`; the stale stamp is an SQL update. Needed: a real library.
4 Direct: 874-877 two song ids, "x" has two copies, "x" has a naming row with timing; 878 reader equals writer by `check_same_index`; 880-885 the stale-stamp case: naming row exists, no timing, equal.
5 Reason: '5' as in record 12; 2 song ids and 2 copies are the fixture's own.
6 Display: no.
7 Failure: 883ac36f/eef15e6a family: the index built by the reader differing from the writer's. Still possible.
8 Deleted: 874-877 (a vacuous equality on an empty index); `check_same_index` fields: copies, naming row, order.
9 Stand-in: `check_same_index` compares `lists(id)`, copies and the naming row for each id; the order check is `song_ids()` equality. Fine.
10 Same rule: tests 12 and 17.
11 Real: yes.
Verdict: Investigate: claim not checked only. Needs a real library.

## 16. a reader's chart index does not wait while a save group is open (tests/test_store_library.cpp:886)

Delta round: new
Read: lines 886-902; `HeldSaveGroup`; `StoreReader::chart_index` and `chart_library_count` (record_store.cpp:2034-2046).
1 Claim: While another thread holds a save group, the reader's chart index and chart count come back at once, equal to the ones read before.
2 Owner: `StoreReader`'s own connection and lock. Callers in `src/` of the reader's chart_index and count: as record 15; `chart_library_count` through the reader: `src/app/report.cpp:201` takes it from a store reference (not read whether that is the reader). Engine.
3 Real chart: Engine and Claim: as record 13: a real helper thread holds a real save group while the main thread reads on the reader's own real connection. Checked on the real mechanism.
4 Direct: 897 `group.held()` still true; 898 index equals the writer's; 899 count equals.
5 Reason: none typed; the count is read from the writer, not typed.
6 Display: no.
7 Failure: as record 13.
8 Deleted: 897 as record 13; 898 and 899 a reader returning an empty index or count.
9 Stand-in: as record 13 (one boolean for two reads).
10 Same rule: record 13.
11 Real: yes.
Verdict: Keep.

## 17. an own reader lists and indexes the library as the writer does (tests/test_store_library.cpp:905)

Delta round: new
Read: lines 905-925; `OwnReader`, `OwnReader::hold` (record_store.cpp:2050-2068); header comment (record_store.h:594-642); commits 95a499ca, 9da33c72.
1 Claim: A reader the caller owns gives the same chart list, copies, index and count as the writer, and gives the same rows again while a hold is open.
2 Owner: `OwnReader` and its `StoreReader`. Callers in `src/`: `src/ui/dm_jobs.cpp:51` and `src/ui/library_jobs.cpp:824` (both `store_.open_own_reader()`). Engine.
3 Real chart: claim not checked. Hand-built library; no real file. Needed: a real library.
4 Direct: 914 two copies (setup); 915-918 list, copies, index and count equal the writer's; 922-924 the same three reads under a hold.
5 Reason: none typed.
6 Display: no.
7 Failure: 95a499ca/9da33c72: the own reader answering differently, or a hold breaking reads. Still possible.
8 Deleted: 915-918 an own reader reading the wrong connection; 922-924 a hold that returns nothing. The hold's real promise (one moment of the file across calls) is not checked here: nothing changes the file between the reads, so a hold that did nothing would pass (reasoned). The hold object is also only a name (`hold`) kept alive; its read transaction is the claim.
9 Stand-in: lines 922-924 stand in for "a hold keeps one moment" and cannot show it. Whether `tests/test_store_records.cpp` (DL1) checks that: not read.
10 Same rule: the own-reader cases at `tests/test_store_records.cpp` 335-531 (DL1's file, not read for the hold).
11 Real: yes; `dm_jobs.cpp:51` and `library_jobs.cpp:824` use it.
Verdict: Investigate: claim not checked only. Needs a real library. The hold lines 922-924 prove only that reads still work under a hold; another test (named above, not read) must carry the one-moment promise.

---

## 18. StoreFile (tests/store_file_fixtures.h:22)

Delta round: new (helper)
Read: lines 1-36 of the header; `testtemp::temp_path` (temp_util.h:40-42 comment); callers by grep.
1 What it does: gives a test a library path in the temp folder, with its `-wal` and `-shm` side files removed when it is made and again when the case ends.
2 Callers: `tests/test_store_library.cpp` (7 new cases), `tests/test_store_records.cpp` (about 30 uses, DL1), `tests/test_preview_controller.cpp:366` (DL5). None in `src/`. Test code.
3 Real chart: not a chart or game claim; it is a fixture. Claim not checked as a claim.
4 Direct: n/a (no assertion).
5 Reason: the per-process name comes from `temp_path` (pid in the name), so two test processes do not collide.
6 Display: no.
7 Failure: 856dfbd2 merged two near-copies (`FileStore`, `LibraryFile`) the single-owner check flagged. Done.
8 Deleted: `remove_all` in the constructor (a leftover from a crashed run would be opened as an old file); in the destructor (files left in the temp folder).
9 Stand-in: it does not remove a set-aside file beside the path; none of the callers need that (not read for the preview test).
10 Same rule: `remove_db` in test_store_open.cpp:81 removes side files and the set-aside file for the same purpose (a second helper for the same job, a split noted here, not judged).
11 Real: yes.
Verdict: Keep (as a helper). Callers declare `StoreFile` before the `RecordStore`, so the store closes first, in the cases I read (test_store_library.cpp).

## 19. HeldSaveGroup (tests/store_file_fixtures.h:38)

Delta round: new (helper)
Read: lines 36-83; `begin_save_group`, `commit_save_group` comment (record_store.h:702-709); `wait_until`, `kWaitCap` (wait_util.h:20-40).
1 What it does: opens a save group on another thread and holds it until `let_go()`, the destructor, or `kWaitCap`; `held()` says whether the helper still holds it.
2 Callers: `test_store_library.cpp` (806, 886 above), `test_store_records.cpp:400`, `test_app_click.cpp:553`, `test_app_reports.cpp:961, 982`. Test code.
3 Mechanism: a real thread and the real store lock; checked on the real mechanism by the cases that use it.
4 Direct: n/a.
5 Reason: the 30 s cap is D108's (`kWaitCap`).
6 Display: no.
7 Failure: reads that wait on a save group; the helper must hold on its own thread because the lock is per thread.
8 Deleted: the `held_ = false` before commit (so `held()` is false the moment the helper starts to let go); the `try/catch` around `begin_save_group` (an exception would end the process; here the constructor times out instead).
9 Stand-in: if `begin_save_group` throws in the thread, `held_` never becomes true and the constructor waits the whole `kWaitCap` (30 s) before throwing; the failure is slow, not a hang, and it names "the helper's save group".
10 Same rule: see the shared split with `StoreLockHolder`.
11 Real: yes.
Verdict: Keep (as a helper), under the split above.

## 20. StoreLockHolder (tests/store_lock_holder.h:18)

Delta round: new (helper)
Read: lines 1-58; `begin_save_group`, `commit_save_group`; the callers' lines by grep.
1 What it does: holds the store's lock from a helper thread by a save group until `release()` or `kWaitCap`; `let_go()` says whether the helper has started to let go.
2 Callers: `test_library_jobs.cpp:698` (DL4), and `returns_while_held` below (five uses in `test_app_library.cpp`, DL3). Test code.
3 Mechanism: a real thread and the real store lock. The callers' claim is checked on the real mechanism.
4 Direct: n/a.
5 Reason: `kWaitCap` (D108).
6 Display: no.
7 Failure: five tests repeated the same lines (a311dde7); 5a998908 moved it out of two test files into this header.
8 Deleted: the `release_` wait loop at 1 ms (cap stops a build that never releases).
9 Stand-in: the helper thread's `begin_save_group()` and `commit_save_group()` calls sit outside any `try`. If either throws, the exception leaves the thread function and ends the whole test run through `std::terminate`, instead of failing one test. `HeldSaveGroup` handles both.
10 Same rule: the split above: `HeldSaveGroup` does the same job with different names and error handling. Callers cannot swap them, because `let_go()` means the opposite thing in each.
11 Real: yes.
Verdict: Rewrite. Merge it with `HeldSaveGroup` into one helper (production code already has one rule for how a thread holds the store lock: the save group), and catch the helper thread's exceptions as `HeldSaveGroup` does.
Today it catches: n/a (helper); it is what makes the five app-level no-wait cases and `test_library_jobs.cpp:698` fail instead of hang.
Production-code finding: none.

## 21. returns_while_held (tests/store_lock_holder.h:61)

Delta round: new (helper)
Read: lines 55-69.
1 What it does: runs a call while a save group holds the store's lock, then tells whether the call came back before the helper let go, that is, whether it did not wait for the lock.
2 Callers: `test_app_library.cpp:353, 1246, 1266, 1280, 1299` (DL3). Test code.
3 Mechanism: real thread, real lock, real call. Claim: a call that waits comes back only after the helper hits `kWaitCap`, and `let_go()` is true by then, so it returns false. Checked on the real mechanism by its callers (not read).
4 Direct: the returned boolean is the assertion each caller makes.
5 Reason: none typed.
6 Display: no.
7 Failure: a window call that waits on the store's lock freezes the window (callers' purpose, a311dde7).
8 Deleted: `holder.release()` after the check (otherwise the helper holds until the cap); `!holder.let_go()` (a call that waited would still report true).
9 Stand-in: none. If `call()` throws, the holder's destructor releases and joins, so no thread is left holding the lock.
10 Same rule: the split above; its holder is the unmerged one.
11 Real: yes.
Verdict: Keep (as a helper), to move onto the merged holder when the split above is closed.
