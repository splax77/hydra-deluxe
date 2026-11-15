# Delta round DL1 records: tests/test_store_records.cpp and tests/test_store_results.cpp

Task T5-DL1, agent a69a201fe8b3f26d9. Diff read: `git diff c10c7f54 5ec682c7` on both files. Old records compared with: `records/A33.md` (test_store_records.cpp, headings T1 to T34) and `records/A34.md` (test_store_results.cpp, headings R1 to R21).

## What every record below shares

Read (all records): `tests/test_store_records.cpp` lines 1-135 (helpers) and each test's own lines; `tests/store_file_fixtures.h` whole; `src/store/record_store.h:490-645` (StoreConnection, StoreReads, StoreReader, OwnReader); `src/store/record_store.cpp:1095-1118` (BatchWrites), `:1120-1127`, `:1380-1400`, `:1538-1550`, `:1836-2066` (readers, hold); `src/store/stored_versions.h:55`; `git log c10c7f54..5ec682c7` subjects for both files. Not read: commit bodies, `tests/store_fixtures.h`, `tests/db_file_util.h` (the old records read them), `tests/wait_util.h` beyond `kWaitCap` (30 s, line 19).

Callers in `src/` for the reader seam (A33 F1 said it had none; that no longer holds): `RecordStore::reader()` is read in `src/ui/app_state.cpp:197, 205, 217, 235, 613, 667, 669, 874` and `src/ui/library_jobs.cpp:92`; `open_own_reader()` plus `hold()` in `src/ui/dm_jobs.cpp:51-52` and `src/ui/library_jobs.cpp:824-825`. `prepare_record` is called from `src/app/analysis.cpp:902` and `src/ui/library_jobs.cpp:770`. Every reader test below is a store-file or thread claim, not a chart-play claim, so `records-engine/` and `records-screen/` have nothing to compare; none of these tests is a display test (question 6 is "not a display test" for all of them, with the caller list above).

How I answered question 3 for these tests. Where the claim is about a thread, a lock, a moment or a held read, the real mechanism is a real SQLite file, a second real connection and (for locks) a real second thread (`HeldSaveGroup`, `tests/store_file_fixtures.h:38`). I wrote "checked on the real mechanism" for those. Where the claim is "a reader answers as the writer does", or about stamps and rules, the expected side is the writer's own answer from the same `lookup_on` code (`record_store.cpp:1390`), and the input is a fixture saved by the test, so I wrote "claim not checked" and named the real input (a real library file, a real second rules file, a real chart analyzed at two windows).

User's words found: `docs/agents/rulings/code-and-engine.md:56`, `2026-11-15`, "make the early fill threshold an adjustable setting pinned to one number in both directions" (session a3ba79d1), under Confirmed. It backs the setting. D344 (`docs/audit/2026-10-03-fix-decisions.md:1987`) says "so we should use the hit window. and stop inventing our own numbers." and bumped the stamp to `3.1.2+efillhitwindow`; the setting's default is 60 ms (`src/core/model.h:112`) and its top edge is 85 (`:115`). Both are filed; the setting is the later one. The test comment at `tests/test_store_records.cpp:1213-1218` quotes a user answer "Mark them Stale". I searched `docs/` (all files, phrase "Mark them Stale" and "Mark them") and found that phrase only in the test and an unrelated 2026-10-04 audit file; no session card or ruling holds it. So that quote is a claim, not evidence. Split: none (the setting is a later filed ruling than D344), but the quote needs a source.

## Helpers (test_store_records.cpp and store_file_fixtures.h)

Delta round: new (each function). What each does and its callers:

- `StoreFile` (`store_file_fixtures.h:22`): a temp `.db` path, with `-wal` and `-shm` removed at construction and destruction. Does what callers rely on. Callers: every store test below, plus `tests/test_preview_controller.cpp`, `tests/test_store_library.cpp`, `tests/test_store_open.cpp` (DL2, DL5).
- `HeldSaveGroup` (`store_file_fixtures.h:38`): opens a save group on another thread and holds it until `let_go()` or 30 s. The constructor waits for the group to open (`wait_until`, throws on timeout). `held()` is true while the helper holds it. It does what callers rely on; a reader that blocked on the writer's lock would leave `held()` false after 30 s, so a wait shows as a failed `CHECK`, not a hang. Callers here: tests 143 and 390; elsewhere `test_app_click.cpp:553`, `test_app_reports.cpp:961, 982`.
- `save_four_charts`, `four_chart_ids` (`test_store_records.cpp:78-89`): save a Ready record, a failure, an old-build record and no record for "never". Callers: 124, 171, 178, 215, 260, 334, 430, 536.
- `check_answers_as_store` (`:94`): compares a reader's `lookup`, `lookup_one`, `settled_song_ids` with the store's. The expected side is the writer's answer. Callers: 124, 171, 178, 334, 536.
- `check_reader_answers_as_store` (`:104`): the same on `store.reader()`. Callers: 124, 171, 178.
- `can_delete` (`:110`): `std::filesystem::remove` succeeds. On Windows a file with an open connection cannot be deleted; elsewhere it can, so on another platform it proves nothing. Callers: 250, 521.
- `moment_reads`, `check_same_moment`, `list_and_save_first`, `move_on`, `wal_size` (`:297-320`): helpers of the own-reader tests; `move_on` changes all three values `moment_reads` takes (record count, second chart's answer, chart list), which each caller `REQUIRE`s. Callers: 352, 376, 430, 456 (`wal_size` and the saves).

## Records

### 124. the store's reader answers every lookup as the store does (tests/test_store_records.cpp:124)
Delta round: changed, replaces A33 T1 (A33.md:21)
Read: lines 124-128, helpers above; `record_store.cpp:1540-1543`, `:819`.
1 Claim: asking an in-memory store for charts through `reader()` gives the store's own answers.
2 Owner: `RecordStore::reader()` returns `*this` when `reader_` is null (`record_store.cpp:1540`); `reader_` is made only for a store with a file (`:819`). Callers of `reader()` in `src/`: listed above, all on a file store.
3 Real chart: not a chart claim; claim not checked.
4 Direct: the helper's lines compare the store with itself.
5 Reason: ids are the test's own inputs.
6 Display: not a display test.
7 Failure: the old record named none; the helper's split moved the body to a shared helper (commit 5382b31c, subject only).
8 Deleted: every line is true by construction here (same object both sides, reasoned). They catch a bug only if `reader()` stopped returning the store, and 130 checks that directly.
9 Stand-in: a function compared with itself.
10 Same rule: 130 (the in-memory reader is the store), 171 (the same helper on a real second connection).
11 Real: production stores always have a file; no in-memory store reaches a player.
Verdict: Remove. Test 171 makes the same promise on a real second connection, and 130 pins that `reader()` is the store. It catches today: nothing 171 and 130 do not.
Old verdict (Investigate, no caller, F1) no longer holds: `reader()` now has callers; the old question about keeping the seam is answered by the code, though not by a filed ruling.

### 143. a reader's lookup does not wait while a save group is open (tests/test_store_records.cpp:143)
Delta round: new
Read: 143-155; `StoreReader::lookup` (`record_store.cpp:1994`), `HeldSaveGroup`.
1 Claim: while a batch holds the store's save group on another thread, a window read of a chart still answers at once with the answer from before the group.
2 Owner: `StoreReader::lookup` takes its own connection's lock only (`:1996`), not the writer's. Callers: `app_state.cpp:217` and the other `reader()` calls above.
3 Real chart: not a chart claim. Checked on the real mechanism: a real file, a real helper thread holding the writer's lock, the reader's own real connection. The saved record is `fixture().record`, the first corpus chart analyzed.
4 Direct: `returned_while_held` says the group was still held when the read returned; `same_answer(got, want)` says the answer is the committed one.
5 Reason: no numbers.
6 Display: not a display test.
7 Failure: a reader that waits on the writer's lock freezes the window while a batch runs (commit 5382b31c: "The store opens a second, read-only connection for its reader", subject only). Still possible if `StoreReader` took the writer's mutex.
8 Deleted: `CHECK(returned_while_held)` fails if the read blocked until the helper's 30 s cap (reasoned). `same_answer` fails if the reader saw the group's uncommitted state, but nothing is saved inside this group, so that half catches a wrong answer only.
9 Stand-in: none; the wait is on the real lock.
10 Same rule: 390 (own reader), `test_app_click.cpp:553` and `test_app_reports.cpp:961` (DL3) use `HeldSaveGroup` for the window.
11 Real: yes, a batch saves while the window reads.
Verdict: Keep.

### 157. a reader sees a save group's chart only once the group commits (tests/test_store_records.cpp:157)
Delta round: new
Read: 157-169.
1 Claim: a chart saved inside an open save group does not show to the window's reader until the group commits.
2 Owner: SQLite's snapshot of a separate connection; `StoreReader::lookup`. Callers as above.
3 Real chart: real mechanism: a real file, a second real connection. Record is the corpus fixture.
4 Direct: `during == before` (NotAnalyzed), then the reader equals the writer after commit.
5 Reason: none.
6 Display: not a display test.
7 Failure: a reader that shared the writer's connection would show half a batch. Possible.
8 Deleted: `same_answer(during, before)` fails for a shared connection (reasoned). The last line fails if the reader is stuck on an old moment.
9 Stand-in: none.
10 Same rule: 280 states it for counts.
11 Real: yes.
Verdict: Keep.

### 171. the file store's reader gives the writer's answer for every lookup (tests/test_store_records.cpp:171)
Delta round: new
Read: 171-176.
1 Claim: a file store's reader gives the same lookup answers as the writer for a Ready chart, a failure, an old-build record and a chart never analyzed.
2 Owner: both go through `lookup_on` (`record_store.cpp:1390`) with the store's own rules fingerprint (`StoreReader::lookup`, `:1999`). Callers as above.
3 Real chart: claim not checked. The expected side is the writer's answer from the same code, and the file is built by the test. Needed: a real library copy opened by the real store, its charts read through the writer and the reader.
4 Direct: each lookup equals, field by field (`same_answer`), as a property.
5 Reason: ids are the test's own.
6 Display: not a display test.
7 Failure: the reader built with the wrong rules fingerprint or without the build stamp; still possible.
8 Deleted: the line fails if `StoreReader` passes another fingerprint (reasoned).
9 Stand-in: two Hydra functions over one shared function; the shared code is checked by the other tests of the file.
10 Same rule: 124, 334, 536 run the same helper.
11 Real: yes.
Verdict: Investigate, claim not checked only. It needs a real library file read by the writer and the reader.

### 178. a store opened under other rules reads the same through its reader (tests/test_store_records.cpp:178)
Delta round: new
Read: 178-186; `core::default_stamp()` use.
1 Claim: a store opened under a rules fingerprint that differs from the one a record was saved under still answers the same through its reader as directly.
2 Owner: `store_.rules_fingerprint_` passed in `StoreReader::lookup` (`record_store.cpp:1999`).
3 Real chart: claim not checked. The "other rules" is `default_stamp().fixed + 1`, a number made by the test. Needed: a real second `hydra_rules.ini` (as old A33 T31/T32 asked).
4 Direct: the helper's property lines.
5 Reason: `+ 1` is a stand-in for "some other rules"; the name says so, nothing else needs it.
6 Display: not a display test.
7 Failure: the reader using the default fingerprint instead of the store's. Possible.
8 Deleted: fails if the reader read under the default fingerprint (the records would read Stale to one and Ready to the other, reasoned).
9 Stand-in: none.
10 Same rule: A33 T31, T32 (writer side).
11 Real: yes, a settings change.
Verdict: Investigate, claim not checked only: a real second rules file.

### 188. a reader's lookup reads one moment (tests/test_store_records.cpp:188)
Delta round: new
Read: 188-213; `set_between_lookup_reads_for_test` (`record_store.h:775`), `lookup_on` use of it (`:1390`), `ReadTransaction` (`:1880`).
1 Claim: a lookup that takes two reads sees one moment of the file even if the writer saves between them.
2 Owner: `ReadTransaction one_moment` in `StoreReader::lookup` (`:1998`). Callers as above.
3 Real chart: real mechanism: a real second connection, the hook saving a second record between the two reads. Input: corpus fixture plus a record with one path text changed to "second".
4 Direct: `saved` guard, `after != before` guard, `during == before`, then the reader catches up.
5 Reason: "second" is the test's own name for a record.
6 Display: not a display test.
7 Failure: a reader whose two reads see two moments could mix a status and its paths. Still possible if the transaction were dropped.
8 Deleted: `same_answer(during, before)` fails without the read transaction (reasoned).
9 Stand-in: none.
10 Same rule: 352 for the own reader's hold. One rule, two owners (call-level moment, hold-level moment).
11 Real: yes, across threads.
Verdict: Keep.
Production finding: a test seam member, `between_lookup_reads_for_test_`, sits in the production `lookup_on` (`record_store.cpp:1390`); noted, not judged.

### 215. a reader holds nothing open between reads (tests/test_store_records.cpp:215)
Delta round: new
Read: 215-229.
1 Claim: after the window's reader has read, nothing it holds stops the batch's end from emptying the log.
2 Owner: `ReadTransaction` ends the read with each call (`record_store.cpp:1883`); `BatchWrites::~BatchWrites` truncates (`:1107`).
3 Real chart: real mechanism: real file, real WAL, a real second connection.
4 Direct: `-wal` size is above zero before (guard), zero after.
5 Reason: zero is the claim (log emptied).
6 Display: not a display test.
7 Failure: a reader that keeps a read open blocks the checkpoint, so the log grows. Possible.
8 Deleted: `CHECK(file_size(wal) == 0)` fails if the reader kept a transaction open (reasoned).
9 Stand-in: file size stands for "nothing held open"; it is the real effect.
10 Same rule: 430 and 456 are the same mechanism with a hold open.
11 Real: yes.
Verdict: Keep.

### 231. a reader that cannot open fails the read and the next read tries again (tests/test_store_records.cpp:231)
Delta round: new
Read: 231-248; `open_if_closed` (`record_store.cpp:1970`), `open_reader` (`:1843`); D73 (`fix-decisions.md:360`, a failed read must fail, not answer "not found").
1 Claim: if the reader cannot open the file, the read throws a database-read error, and the next read tries again.
2 Owner: `StoreReader::open_if_closed` throws `KindedError(DatabaseRead)` from `open_reader`. Callers as above.
3 Real chart: real mechanism for the open (real SQLite fails opening a path that is not there); the failure is forced by `fail_next_reader_open_for_test` rather than a real locked or unreadable file.
4 Direct: kind equals DatabaseRead; the later read equals the writer's answer.
5 Reason: none.
6 Display: not a display test.
7 Failure: D73's quiet empty answer on a failing read. Possible.
8 Deleted: `CHECK(threw)` fails if the failure became an empty answer; the last line fails if the reader stayed closed.
9 Stand-in: the test hook stands for a real open failure.
10 Same rule: 503 (the own reader's open).
11 Real: yes (antivirus lock, deleted file).
Verdict: Keep.

### 250. closing the store closes its reader (tests/test_store_records.cpp:250)
Delta round: new
Read: 250-258; `StoreReader::close` (`record_store.cpp:1965`).
1 Claim: after `store.close()` the library file can be deleted.
2 Owner: `RecordStore::close` closing `reader_` (not opened here; `:819` makes it). Callers of `close()` in `src/` not searched.
3 Real chart: real mechanism on Windows: a real file with open connections cannot be deleted. On another platform `remove` succeeds while a connection is open, so the check proves nothing there.
4 Direct: `can_delete` true.
5 Reason: none.
6 Display: not a display test.
7 Failure: a leaked reader connection leaves the file locked. Possible.
8 Deleted: fails on Windows if the reader stayed open (reasoned).
9 Stand-in: deletion stands for "no connection open"; the `-wal` and `-shm` are not checked.
10 Same rule: 521 (the own reader).
11 Real: yes.
Verdict: Keep. Side note: `can_delete` is Windows-only evidence (tests/test_store_records.cpp:116).

### 260. the reader counts records and library charts as the writer does (tests/test_store_records.cpp:260)
Delta round: new
Read: 260-278.
1 Claim: the reader's record count and library chart count equal the writer's.
2 Owner: `record_count_on`, `chart_library_count_on` on each connection (`StoreReader::record_count`, `:2027`).
3 Real chart: claim not checked: a four-chart library made by the test; needed a real library copy.
4 Direct: `REQUIRE(records > 0)`, `REQUIRE(charts > 0)` guard; equality as a property.
5 Reason: none; zeros guarded.
6 Display: not a display test.
7 Failure: a reader counting on a stale connection. Possible.
8 Deleted: each CHECK fails for a reader that read nothing (reasoned); guards say so.
9 Stand-in: the writer's count is the expected value, from the same code.
10 Same rule: 334 repeats both checks for the own reader.
11 Real: yes.
Verdict: Investigate, claim not checked only: a real library copy.

### 280. a reader's counts see a save only once it commits (tests/test_store_records.cpp:280)
Delta round: new
Read: 280-295.
1 Claim: the reader's record count does not include a save inside an open group, and does after it commits.
2 Owner: SQLite snapshot; `StoreReader::record_count`.
3 Real chart: real mechanism (a second real connection); corpus fixture.
4 Direct: `during == before`; `REQUIRE(store.record_count() != before)` guard; the reader catches up.
5 Reason: none.
6 Display: not a display test.
7 Failure: a count shared with the writer's connection shows half a batch (the window's count line).
8 Deleted: `CHECK(during == before)` fails for a shared connection.
9 Stand-in: none.
10 Same rule: 157 for lookups.
11 Real: yes.
Verdict: Keep.

### 334. an own reader gives the writer's answer for every read (tests/test_store_records.cpp:334)
Delta round: new
Read: 334-351.
1 Claim: an own reader's answers, counts and chart list equal the writer's.
2 Owner: `OwnReader::reads()` is its own `StoreReader` (`record_store.cpp:2050`). Callers: `dm_jobs.cpp:51`, `library_jobs.cpp:824`.
3 Real chart: claim not checked; same as 171/260. Needed: a real library copy.
4 Direct: property lines; `REQUIRE(record_count() > 0)` guard. The chart list check compares only `song_ids()`; the comment points to `test_store_library.cpp` for field-for-field (DL2).
5 Reason: none.
6 Display: not a display test.
7 Failure: the own reader built with the wrong store state.
8 Deleted: each line fails for a reader that read nothing or under another fingerprint.
9 Stand-in: writer's answer from the same code.
10 Same rule: 171, 260 (one rule, "a second connection answers as the writer"; three tests state it).
11 Real: yes.
Verdict: Investigate, claim not checked only: a real library copy.

### 352. a hold makes every call see one moment (tests/test_store_records.cpp:352)
Delta round: new
Read: 352-374; `OwnReader::hold` (`record_store.cpp:2062`), `begin_read` (`:1858`).
1 Claim: while a hold is open, an own reader's second chart read, count and chart list all see the file as at the hold's first read, even after the writer moves on; once the hold ends it sees the new file.
2 Owner: `OwnReader::hold` opens a read transaction. Callers: `dm_jobs.cpp:52`, `library_jobs.cpp:825`.
3 Real chart: real mechanism: real file, real second connection; corpus fixture.
4 Direct: three `REQUIRE`s say the writer's move changed each value; `check_same_moment` compares held with before and later with after.
5 Reason: none.
6 Display: not a display test.
7 Failure: a report built from a chart list from one moment and a count from another. Possible if the hold were not a transaction.
8 Deleted: `check_same_moment(held, before)` fails without the transaction (reasoned).
9 Stand-in: none.
10 Same rule: 188.
11 Real: yes.
Verdict: Keep.

### 376. an own reader with no hold sees each commit (tests/test_store_records.cpp:376)
Delta round: new
Read: 376-388.
1 Claim: without a hold, each read sees what the writer committed.
2 Owner: per-call transactions. Callers as above.
3 Real chart: real mechanism.
4 Direct: first call equals before; after `move_on`, equals after, with a `REQUIRE` the values changed.
5 Reason: none.
6 Display: not a display test.
7 Failure: an own reader stuck on its first moment, so a job's later reads are stale.
8 Deleted: the last line fails for a reader that never ends a transaction (reasoned).
9 Stand-in: none.
10 Same rule: 352 (the contrast).
11 Real: yes.
Verdict: Keep.

### 390. a held own reader does not wait for a save group, and a save group does not wait for it (tests/test_store_records.cpp:390)
Delta round: new
Read: 390-414; grep `busy_timeout|sqlite3_busy` over `src/store/*.cpp`: no hit.
1 Claim: with a hold open, a read does not wait on a save group held on another thread, and a later save group on the writer starts without waiting on the hold.
2 Owner: SQLite WAL and the reader's own lock. Callers as 352.
3 Real chart: real mechanism: real thread holds the real lock, real second connection holds the real hold.
4 Direct: `returned_while_held`; answers equal; a save group then commits and the writer reads "h2" Ready.
5 Reason: none. The comment's "the store sets no busy timeout" matches my grep (no hit); not a rule.
6 Display: not a display test.
7 Failure: a hold that blocks the writer freezes a batch; the writer waiting on a hold would throw "database is locked".
8 Deleted: `CHECK(returned_while_held)` fails if the read waited; the last `CHECK` fails if the writer threw.
9 Stand-in: none.
10 Same rule: 143 states half of it (no hold).
11 Real: yes.
Verdict: Keep. Side note: the test name holds a comma.

### 416. an open hold does not stop the window's reader (tests/test_store_records.cpp:416)
Delta round: new
Read: 416-428.
1 Claim: while an own reader holds a moment, the window's `reader()` still reads the latest.
2 Owner: `reader_` and an own reader are two separate `StoreReader`s (`record_store.cpp:819`, `:1546`).
3 Real chart: real mechanism.
4 Direct: guard that the held answer differs from now; window reader equals the writer.
5 Reason: none.
6 Display: not a display test.
7 Failure: both sharing one connection would freeze the window on a job's moment. Possible.
8 Deleted: the last `CHECK` fails for a shared connection.
9 Stand-in: none.
10 Same rule: none other.
11 Real: yes (a report job runs while the window reads).
Verdict: Keep.

### 430. a hold open at a batch's end loses nothing, and the next batch's end empties the log (tests/test_store_records.cpp:430)
Delta round: new
Read: 430-454; `BatchWrites` (`record_store.cpp:1101-1118`).
1 Claim: if a hold is open when a batch ends, the log is not emptied but every result reads back; after the hold ends, the next batch's end empties the log, and a second connection reads the results.
2 Owner: `BatchWrites::~BatchWrites`: a truncating checkpoint that "loses nothing" if it cannot finish (`:1108-1112`). Callers: batch runs (`analysis.cpp` not read for its use).
3 Real chart: real mechanism: real WAL, real hold.
4 Direct: log size above zero while the hold is open; three lookups read Ready, CantAnalyze, Stale; log size zero after; the window reader reads Ready.
5 Reason: zero is the claim.
6 Display: not a display test.
7 Failure: a batch end that throws or loses results when a reader holds the log.
8 Deleted: `wal_size > 0` fails if the checkpoint passed the hold (reasoned); the three lookups catch lost results.
9 Stand-in: file size for "checkpoint blocked": the real effect.
10 Same rule: 215, 456.
11 Real: yes.
Verdict: Keep. Side note: the test name holds a comma.

### 456. the log keeps growing while a hold is open (tests/test_store_records.cpp:456)
Delta round: new
Read: 456-501; `set_wal_autocheckpoint_for_test` (`record_store.cpp:1124`).
1 Claim: during a hold the log grows with every save group, and without one it stays no larger than after the second group.
2 Owner: SQLite's checkpointing. No Hydra rule decides this; the hold only keeps a transaction open.
3 Real chart: real mechanism (real WAL), but nothing Hydra decides.
4 Direct: `held[i] > held[i-1]` for each group; `unheld[i] <= unheld[1]`; `unheld.back() < held.back()`. These pin SQLite's checkpoint timing, not a wanted behaviour.
5 Reason: "one page" autocheckpoint is the test's own input; why the second group is the reference is a reading of SQLite's folding, not a rule.
6 Display: not a display test.
7 Failure: nothing a player sees today. Whether growth during a hold is accepted or should be bounded is open.
8 Deleted: `held` lines fail if the hold stops holding (352 also catches that). The `unheld` lines can fail on a SQLite upgrade without any Hydra bug.
9 Stand-in: file size stands for "checkpoint blocked".
10 Same rule: 430, 215.
11 Real: a job that holds while a batch saves can happen; `dm_jobs.cpp:52` holds for a job.
Verdict: Investigate. It pins a cost (the log grows without bound while a hold is open), not a promise; whether that growth is accepted goes to the user. If it is, the test duplicates 352 and 430 and should go.

### 503. an own reader that cannot open fails the read (tests/test_store_records.cpp:503)
Delta round: new
Read: 503-519.
1 Claim: `open_own_reader` throws a database-read error when the file cannot be opened, and a later one works.
2 Owner: `OwnReader` ctor calls `open_now` (`record_store.cpp:2054`); callers `dm_jobs.cpp:51`, `library_jobs.cpp:824`.
3 Real chart: real SQLite failure by hook (as 231).
4 Direct: kind DatabaseRead; later reader equals writer.
5 Reason: none.
6 Display: not a display test.
7 Failure: D73's quiet empty answer (a report job treating every chart as not analyzed).
8 Deleted: `CHECK(threw)` fails if the open is lazy and the failure only shows later (reasoned).
9 Stand-in: hook for a real failure.
10 Same rule: 231.
11 Real: yes.
Verdict: Keep.

### 521. closing an own reader lets go of the file (tests/test_store_records.cpp:521)
Delta round: new
Read: 521-534.
1 Claim: after the hold and the own reader are gone and the store closed, the file can be deleted.
2 Owner: `StoreReader::~StoreReader` calls `close`.
3 Real chart: real mechanism on Windows only (see 250).
4 Direct: `can_delete`.
5 Reason: none.
6 Display: not a display test.
7 Failure: a leaked connection keeps a library file locked.
8 Deleted: fails on Windows if the own reader leaked (reasoned).
9 Stand-in: deletion for "no connection".
10 Same rule: 250.
11 Real: yes.
Verdict: Keep.

### 536. an in-memory store's own reader reads through the store (tests/test_store_records.cpp:536)
Delta round: new
Read: 536-555; `OwnReader` ctor (`record_store.cpp:2050`, `reads_` is the store when `own_` is null).
1 Claim: for an in-memory store the own reader answers as the store, a hold changes nothing, and inside an open save group it sees the group.
2 Owner: `OwnReader` with `path.empty()`. A production store always has a file: grep `":memory:"` in `src/` finds only `library_file.h`/`.cpp` comments and a Fresh-file case (`library_file.cpp:135`), no caller that opens one.
3 Real chart: claim not checked; the case is test-only.
4 Direct: most lines compare the store with itself and are true by construction (reasoned); they can fail only if an in-memory own reader opened a second, empty `:memory:` database.
5 Reason: none.
6 Display: not a display test.
7 Failure: a second empty in-memory connection answering "nothing". No player has an in-memory store.
8 Deleted: see 4.
9 Stand-in: none beyond 4.
10 Same rule: 124 (same pattern), 130.
11 Real: no.
Verdict: Investigate: whether the in-memory branch of `OwnReader` should exist at all, since only tests open an in-memory store. If the user keeps it, this test is the only one that pins it.

### 591. store: prepare_record refuses a profile at another early-fill window (tests/test_store_records.cpp:591)
Delta round: new
Read: 591-606; `prepare_record` guard (`record_store.cpp:695-699`); `kMaxEarlyFillWindowMs` (`model.h:115`), `kDefaultEarlyFillWindowMs` (`:112`); `src/app/analysis.cpp:902`, `src/ui/library_jobs.cpp:770` (callers); ruling `code-and-engine.md:56`.
1 Claim: a record analyzed at one early-fill window cannot be filed under a profile at another, and the matching pair files.
2 Owner: `prepare_record` compares `profile.lens.efill_window_ms` with `record.early_fill_window_ms`, the value the engine stamped. The two callers pass an analysis and its profile.
3 Real chart: claim not checked. The 85 ms record is `at_cap(4)` with its window field set by hand; no analysis ran at 85. Needed: a real corpus chart analyzed at 85 ms by the engine and filed.
4 Direct: first throw, second throw, third file; each compares two typed fields, and the matching pair's `efill_window_ms == kMaxEarlyFillWindowMs`.
5 Reason: the default and the max come from named constants in `model.h`; the ruling at `code-and-engine.md:56` backs a setting that varies.
6 Display: not a display test.
7 Failure: a record searched at one window filed as another's result, so the window shows another window's marks.
8 Deleted: each throw fails if the guard were removed or compares one side only (reasoned).
9 Stand-in: none.
10 Same rule: `test_store_results.cpp:339` line 751-754 pin that the window is in `Lens` equality; 608 stores the two apart.
11 Real: yes, a user changes the setting.
Verdict: Investigate, claim not checked only: needs a real chart analyzed by the engine at two windows.

### 608. store: one chart's results at two early-fill windows are two results (tests/test_store_records.cpp:608)
Delta round: new
Read: 608-639; `scalar_on_file`.
1 Claim: saving one chart at two windows keeps both; each lookup returns its own; the file holds two profile rows; a reopen reads both.
2 Owner: `profiles` rows keyed with `efill_window_ms` (`record_store.cpp:183, 414`). Callers as above.
3 Real chart: claim not checked: the second record is `no_paths_at_cap(4)` with its window set to 85 by hand, not an analysis. Needed: a real chart whose early-fill result differs between windows, analyzed at both.
4 Direct: `saved_default.record != saved_85.record` guard; each lookup equals its saved record; `record_count() == 2` (a count that goes with the equality checks); the file's profiles count is 2 (a typed 2; it is the claim: two profile rows).
5 Reason: 2 is two windows.
6 Display: not a display test.
7 Failure: one window's result overwriting another's, or a lookup returning the other window's record. Commit e066919d ("saves results per early-fill window", subject only).
8 Deleted: each lookup comparison fails if a save replaced the other's record.
9 Stand-in: the profile count for "two profiles".
10 Same rule: 591.
11 Real: yes.
Verdict: Investigate, claim not checked only: a real chart analyzed at two windows.

### 699. store: a save purges the chart's unreadable records in every mode, and the next open deletes their emptied profiles (tests/test_store_records.cpp:699)
Delta round: changed, replaces A33 T8 (A33.md:132)
Read: 699-722. Change: `FileStore` became `StoreFile("store_records")` (same temp path, leftover files cleared first); no other line changed.
Old verdict: Rewrite. It still holds: the emptied-profile lines it named (now the two checks at 714-715 and 718) are still there; `StoreFile` changes nothing about them. Q1 to Q11 are as A33 wrote them.
Verdict: Rewrite. What it catches today: the other mode reads NotAnalyzed after a save, and a reopen removes the emptied profile (A33 T8).
Side note: the test name holds a comma.

### 724. store: a profile the scan's purge empties reads as nothing, and the next open deletes it (tests/test_store_records.cpp:724)
Delta round: changed, replaces A33 T9 (A33.md:149)
Change: `StoreFile` only. Old verdict Rewrite still holds (the survival check and the three listings from `settled_song_ids`). Verdict: Rewrite. Side note: name holds a comma.

### 753. store: an open while another connection holds the write lock keeps the empty profiles, and a later open deletes them (tests/test_store_records.cpp:753)
Delta round: changed, replaces A33 T10 (A33.md:166)
Change: `StoreFile` only. Old verdict Rewrite (name the profiles by mode, not a count of two) still holds; the connection holding the lock is real, so it also stands on the real mechanism. Verdict: Rewrite. Side note: name holds a comma.

### 917. store: a failure under other rules reads Stale, and one a store with no rules refuses (tests/test_store_records.cpp:917)
Delta round: changed, replaces A33 T18 (A33.md:290)
Change: `StoreFile` only. Old verdict Investigate, claim not checked only (needs a real broken `hydra_rules.ini`) still holds. Verdict: Investigate, claim not checked only. Side note: name holds a comma.

### 963. store: a failure replaces the real record's path rows in the file (tests/test_store_records.cpp:963)
Delta round: changed, replaces A33 T20 (A33.md:322)
Change: `StoreFile` only. The old record checked the claim on a real SQLite file and a real corpus record against SQL counts from a separate connection (`scalar_on_file`), so Q3 holds. Verdict: Keep.

### 1005. store: a profile whose only record is a failure is kept (tests/test_store_records.cpp:1005)
Delta round: changed, replaces A33 T22 (A33.md:353)
Change: `StoreFile` only. Old verdict Rewrite (check the profile by chart mode, not a count of two) still holds. Verdict: Rewrite.

### 1023. store: a failure of a kind this build cannot name reads Stale for another build (tests/test_store_records.cpp:1023)
Delta round: changed, replaces A33 T23 (A33.md:369)
Change: `StoreFile` only. Old verdict Keep still holds (a real file, an `UPDATE` as a later build would leave it). Verdict: Keep.

### 1113. store: path rows measured on the corpus (tests/test_store_records.cpp:1113)
Delta round: changed, replaces A33 T27 (A33.md:431)
Change: `StoreFile` only. Old verdict Rewrite (name each chart that differs, require zero skips, drop the size measurement). Still holds; the test still loops over `corpus_analyses()` and prints sizes. Verdict: Rewrite.

### 1206. store: every accepted results stamp reads Ready, others Stale (tests/test_store_records.cpp:1206)
Delta round: changed, replaces A33 T29 (A33.md:463)
Read: 1206-1265; `stored_versions.h:55`; `docs/audit/2026-10-03-fix-decisions.md:1987` (D344); ruling `code-and-engine.md:56`.
Changes: the current stamp is now `3.1.2+efillwindow` (typed twice, at lines 1220-1221 and in the cases table at 1245); `3.1.2+efillhitwindow` joined the retired stamps (a new `CHECK_FALSE` and a new case "n"); the comment quotes the user's answer "Mark them Stale".
1 Claim: only the stamp the build writes reads Ready; every older stamp reads Stale and stays out of the skip list.
2 Owner: `kResultsStamp.is_current` (`stored_versions.h:55`), used at `record_store.cpp:471`; skip list `settled_song_ids`/`ready_song_ids` (`analysis.cpp:779`, `cli/batch.cpp:17`).
3 Real chart: claim not checked. Needs a real older-release `hydra.db` (stamps from real releases) read through the store.
4 Direct: 14 typed stamps and statuses. The current stamp typed as a literal is a copy of `stored_versions.h:55`, so a bump fails this test (A33's point).
5 Reason: each stamp is a real release; the comment cites D85, D105, D115, D337, D344 (D344 opened: "so we should use the hit window..."; it bumped to `3.1.2+efillhitwindow`, now retired). The quote "Mark them Stale" has no source in `docs/` (searched). The setting itself has the filed ruling at `code-and-engine.md:56`.
6 Display: not a display test.
7 Failure: an old result read as current after a rule change; a player sees marks from the wrong window.
8 Deleted: each case fails if the stamp rule accepted it.
9 Stand-in: none.
10 Same rule: `tests/test_store_open.cpp` store-version tests (DL2).
11 Real: yes.
Verdict: Rewrite, as A33 T29 said: take the Ready case from `kResultsStamp.written` and keep a few retired stamps as named inputs. The old verdict holds. The newly added lines (the `efillhitwindow` case and `CHECK_FALSE`) are still typed literals.
Finding: the test comment quotes a user answer ("Mark them Stale") that no session card, decision or ruling holds; it needs a source or removal.
Side note: the test name holds a comma.

### 1288. store: a record made under other rules reads Stale until the store runs those rules (tests/test_store_records.cpp:1288)
Delta round: changed, replaces A33 T31 (A33.md:496)
Change: `StoreFile` only. Old verdict Investigate, claim not checked only (a real second `hydra_rules.ini`) still holds. Verdict: Investigate, claim not checked only.

### 1328. store: the other rules' record a save kept reads Ready under those rules (tests/test_store_records.cpp:1328)
Delta round: changed, replaces A33 T32 (A33.md:511)
Change: `StoreFile` only. Old verdict Investigate, claim not checked only (a real analysis under a second real rules file) still holds. Verdict: Investigate, claim not checked only.

### Profile compares on every part of the identity (tests/test_store_results.cpp:339)
Delta round: changed, replaces A34 R8 (A34.md:139)
Read: 339-366 (diff hunk at 345-362); `Lens` (`record_store.h:129-150`); `model.h:112, 115`.
Changes: "five fields" became "six"; new check that a lens at `kMaxEarlyFillWindowMs` differs from `kLensA`; new checks that `Lens{}` and `kLensA` name `kDefaultEarlyFillWindowMs` as their window.
1 Claim: two profiles are the same only if each part of the identity, now including the early-fill window, matches.
2 Owner: `Lens::operator==` (`record_store.h:150`) includes `efill_window_ms`. Callers: store lookups and saves.
3 Real chart: claim not checked (as A34 R8: no real profile on a real library).
4 Direct: each `!=` names the field that differs; the two default checks compare with the named constant.
5 Reason: `kMaxEarlyFillWindowMs` and `kDefaultEarlyFillWindowMs` are the source's own constants (`model.h:112, 115`).
6 Display: not a display test.
7 Failure: a window change that does not separate results (608's problem at the key level).
8 Deleted: the `!=` fails if `operator==` dropped the field; the two default lines fail if `Lens` picked another default.
9 Stand-in: none.
10 Same rule: 591, 608.
11 Real: yes.
Verdict: Investigate, claim not checked only (as A34 R8). Old verdict holds; the new lines are direct and need nothing the old record did not.

## Removed test cases
None removed. The commits only moved the `FileStore` struct out of `test_store_records.cpp` into the shared `StoreFile` (commit 856dfbd2, "The store tests share one held save group and one temp library file"); no test case was deleted from either file.

## Counts
In scope: test_store_records.cpp 34 cases (22 new, 12 changed, 0 removed); test_store_results.cpp 1 case (changed). Verdicts, by test line: Keep = 143, 157, 188, 215, 231, 250, 280, 352, 376, 390, 416, 430, 503, 521, 963, 1023 (16). Rewrite = 699, 724, 753, 1005, 1113, 1206 (6). Remove = 124 (1). Investigate = 171, 178, 260, 334, 456, 536, 591, 608, 917, 1288, 1328, results 339 (12). Total 35.
