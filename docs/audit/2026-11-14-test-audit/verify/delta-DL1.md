Agent a63a7630732ddad61 (T5-V-DL1). Verified `records/delta-DL1.md` against the code at `5ec682c7`. Nothing run; every mark below is "by reading" unless it says otherwise.

# Scope

I ran `git diff c10c7f54 5ec682c7` on both files and walked every hunk. In scope: 34 cases in `tests/test_store_records.cpp` and 1 in `tests/test_store_results.cpp`, 35 in all. The record file has 35 records, one per case. I found no case without a record and no record for an unchanged case. Hunk check: the 11 "StoreFile only" cases (699, 724, 753, 917, 963, 1005, 1023, 1113, 1206, 1288, 1328) each have the `FileStore` to `StoreFile("store_records")` line inside their body (1206 also has the stamp edits). Tests 130, 1039, 1067, 1094, 1185 and the other unchanged cases have no hunk. No test was removed; the diff shows only the `FileStore` struct leaving the file.

Line anchors in the record are the `TEST_CASE` lines at `5ec682c7`; they match. Some `record_store.cpp` line cites are off by one or two (for example `StoreReader::lookup`'s lock is at 1996 and its `lookup_on` call is 1998-2000). Not a fault.

# Verdict tally

I agree with 34 verdicts and dispute 1 (test 1023). Four agreed Keeps carry a caveat the record should state (231 and 503 forced open, 188 forced interleaving, 250 and 521 Windows-only).

# Shared claims checked

- Callers of the reader seam: `src/ui/app_state.cpp:197, 205, 217, 235, 613, 667, 669, 874`, `src/ui/library_jobs.cpp:92`, and `open_own_reader()` plus `hold()` at `src/ui/dm_jobs.cpp:51-52` and `src/ui/library_jobs.cpp:824-825`. Holds by reading. `prepare_record` callers: `src/app/analysis.cpp:902`, `src/ui/library_jobs.cpp:770`, and also `tools/bench.cpp:125, 335`, which the record leaves out.
- D344 (`fix-decisions.md:1987`) and the filed lines at `code-and-engine.md:55-57` are quoted correctly. The record cites line 56 but not line 57, "analysis settings panel, default 60ms, supports 0ms to 85ms", which is the filed ruling for the 60 default and the 0 to 85 range (`model.h:108-115` quotes it). So there is no split between D344's "use the hit window" and the 60 default: line 57 is later and names both. The record's "split: none" stands; it should cite line 57.
- "Mark them Stale" (test comment at 1213-1218): I searched `docs/` for "Mark them". The only hits are the test, this records file and an unrelated 2026-10-04 audit line. The nearest real user words are D344 item 2, "Bump the stamp". So the quote in the test comment is not backed as worded. Agree with the record's finding.
- No record proposes a saved expected-output file. Nothing to flag against `code-and-engine.md:50`.

# Records

124 (in-memory store's reader answers as the store). The body runs `save_four_charts` then `check_reader_answers_as_store` on `:memory:`. `reader()` returns `*this` when `reader_` is null (`record_store.cpp:1540-1542`); `reader_` is made only for a store with a file (`:819`). Both sides are one object. Agree: Remove. Test 130 (`&store.reader() == &store`) and 171 (same helper on a real file) cover it. The old Investigate (T1, no caller) no longer holds because callers now exist, as the record says.

143 (reader does not wait while a save group is open). Real file, real helper thread holding the real save group, the reader's own connection. Q8: `CHECK(returned_while_held)` goes red if the read blocked on the writer's lock, because the helper holds until its 30 s cap and `held()` is then false. `same_answer(got, want)` catches only a wrong answer, as the record says. Q3 meets the thread rule. Agree: Keep. Q2 callers confirmed (`app_state.cpp:217`).

157 (reader sees a group's chart only on commit). Second real connection. Q8: `same_answer(during, before)` goes red if the reader shared the writer's connection. The last line goes red if the reader stays on an old moment (the writer reads Ready, a stuck reader NotAnalyzed). Agree: Keep.

171 (file store's reader gives the writer's answer). Both sides run `lookup_on`; the expected side is the writer's own answer; the file is built by the test. Q3 "claim not checked" is right. Q8: red if `StoreReader::lookup` passed another rules fingerprint. Agree: Investigate, claim not checked only (a real library file read by writer and reader).

178 (store under other rules, same through reader). `RulesStamp{default_stamp().fixed + 1}` is a number the test made up. Q8 checked: records saved under default rules read Stale to the writer. A reader on the default fingerprint would read Ready, and CantAnalyze for the failure, so `same_answer` goes red per chart. Agree: Investigate, claim not checked only (a real second rules file).

188 (a reader's lookup reads one moment). The hook `set_between_lookup_reads_for_test` fires at `record_store.cpp:1464`, between the first and second read of `lookup_on`. The writer saves a record with different paths there. Without `ReadTransaction` (`:1998`) the reader would pair the first read's record columns with the new path rows, and `same_answer(during, before)` goes red at that `CHECK`. The interleaving is forced by a production seam on one thread, not raced between two threads; the save and both connections are real. Agree: Keep, with that caveat added to Q3. The record's production finding (a test seam inside `lookup_on`, `:1385-1390`) is accurate.

215 (reader holds nothing open between reads). Real WAL. Q8: `CHECK(file_size(wal) == 0)` goes red if a reader call left a transaction open, since the truncating checkpoint cannot finish. The `> 0` guard blocks a vacuous pass. Agree: Keep.

231 (reader that cannot open fails the read, next read retries). `open_if_closed` (`:1970`) forces the open onto a path that is not there, so the failure is a real SQLite open error on a real missing file; only the path swap is forced. Q8: `CHECK(threw)` goes red if the failure became an empty answer. The last line goes red if the reader stayed closed. Agree: Keep. Caveat for the record: the claim was not checked on a real unreadable library file.

250 (closing the store closes its reader). Q2: the record says callers of `close()` were not searched. I searched `src` and `tools`: `src/cli/fillcompare.cpp:93` calls `store->close()`. Q8: on Windows a leaked reader connection makes `remove` fail, so the `CHECK` goes red. On other platforms it is vacuous, as the record says. Agree: Keep.

260 (reader counts equal the writer's). The four-chart library is built by the test; `REQUIRE(records > 0)` and `REQUIRE(charts > 0)` stop a vacuous pass. Agree: Investigate, claim not checked only (a real library copy).

280 (counts see a save only on commit). Second real connection. `during == before` goes red for a shared connection; `REQUIRE(store.record_count() != before)` is a guard. Agree: Keep.

334 (own reader gives the writer's answer). Same input and owner as 171 and 260. The chart list check compares only `song_ids()`; field for field is in `test_store_library.cpp` (DL2). Callers `dm_jobs.cpp:51` and `library_jobs.cpp:824` confirmed. Agree: Investigate, claim not checked only.

352 (a hold makes every call see one moment). Three `REQUIRE`s prove the writer's move changed each value. `check_same_moment(held, before)` goes red if `hold` is not a read transaction (`begin_read`, `:1858`). Real file, real second connection. Agree: Keep.

376 (own reader without a hold sees each commit). The last line goes red for a reader that never ends its transaction. Agree: Keep.

390 (held own reader and a save group do not wait on each other). Real helper thread and a real hold. `CHECK(returned_while_held)` goes red if the read waited; the final `CHECK` goes red if the writer threw. The comment's "no busy timeout" is a comment, not a rule; my grep of `src/store` for `busy_timeout` also finds nothing. Agree: Keep.

416 (an open hold does not stop the window's reader). Guard `REQUIRE_FALSE(same_answer(now, held_before))`, then the window reader equals the writer. Red if the window reader shared the own reader's connection. Agree: Keep.

430 (a hold open at a batch's end loses nothing; the next batch's end empties the log). Real WAL. `BatchWrites::~BatchWrites` (`:1107-1114`) runs `wal_checkpoint(TRUNCATE)` and ignores the result, so a hold leaves the log non-empty; `wal_size > 0` is the real effect. Q2: the record did not read `analysis.cpp`. The production caller is `src/app/analysis.cpp:845` (`const store::RecordStore::BatchWrites batch_writes(store);`), the only one in `src` and `tools`. Agree: Keep.

456 (the log keeps growing while a hold is open). The assertions pin SQLite's checkpoint folding (`held[i] > held[i-1]`, `unheld[i] <= unheld[1]`, `unheld.back() < held.back()`), not a Hydra promise. Production sets a journal size limit (`record_store.cpp:850`) that applies only after a checkpoint resets the log, so growth during a hold is real. Whether it is accepted is the user's call. Agree: Investigate.

503 (own reader that cannot open fails the read). Same hook as 231. The `OwnReader` constructor opens eagerly (`:2050`), so `CHECK(threw)` goes red if the open became lazy. Agree: Keep, with the 231 caveat.

521 (closing an own reader lets go of the file). Windows-only evidence, as 250. Agree: Keep.

536 (in-memory store's own reader reads through the store). Q2 correction: the record greps `src/` only and says no caller opens `":memory:"`. `tools/bench.cpp:103` opens `store::RecordStore store(":memory:", ...)` in a shipped tool. It calls neither `open_own_reader` nor `reader()` (my grep over `src` and `tools`), so the in-memory own-reader branch is still reached by tests only. Most lines compare the store with itself. Agree: Investigate. Add the bench caller to the question for the user.

591 (prepare_record refuses a profile at another early-fill window). The guard is `record_store.cpp:690-699`. Q8 per assertion: the first `CHECK_THROWS_AS` (record at 60, profile at 85) goes red if the guard is dropped or checks one side. The second (record at 85, default profile) catches a one-sided guard in the other direction. The third `CHECK` pins that a matching pair files. The 85 ms record is `at_cap(4)` with the window set by hand, so no engine run at 85 backs the claim. Agree: Investigate, claim not checked only (a real chart analysed by the engine at two windows).

608 (one chart's results at two windows are two results). The second window is hand-set (`no_paths_at_cap(4)` with `kHitWindowMs`). The file's `profiles` count of 2 is the claim and is read on a separate connection (`scalar_on_file`). `record_count() == 2` is a typed count. Agree: Investigate, claim not checked only.

699, 724, 753, 917, 1005, 1113, 1288, 1328 (StoreFile only). The diff confirms a one-line change in each body and nothing else. The old verdicts stand: A33 T8 Rewrite, T9 Rewrite, T10 Rewrite, T18 Investigate, T22 Rewrite, T27 Rewrite, T31 Investigate, T32 Investigate. The constructor now clears leftovers first, and nothing the old records faulted moved. Agree with all eight.

963 (a failure replaces the real record's path rows). Corpus record (`fixture().record`), real file, SQL counts from a separate connection (`scalar_on_file`, `record_rows_in`), with a `REQUIRE(... > 0)` guard before. Q3 meets the rule. Agree: Keep.

1023 (a failure of a kind this build cannot name reads Stale for another build). I dispute the Keep. The input is a failure saved by the test, then `UPDATE records SET failure_kind = 'LaterKind'` run by hand. No real file from a later build is read. The claim is about a file another build wrote, and a hand-edited column is a hand-built fixture. The verdict should be Investigate, "claim not checked only"; it needs a real database written by a build that knows one more error kind. Assertions: `status == Stale`, `stale_build`, no `failure_kind`, empty settled list; all go red if the unknown kind read as CantAnalyze.

1206 (every accepted stamp reads Ready, others Stale). `stored_versions.h:55` is `{"3.1.2+efillwindow", {"3.1.2+efillwindow"}}`, matching the literal in the test. The current stamp is typed twice, so a bump fails the test, as A33 T29 said. Agree: Rewrite. The new case "n" and the new `CHECK_FALSE(is_current("3.1.2+efillhitwindow"))` are typed literals again. Finding agreed: "Mark them Stale" has no source (see above).

Results 339 (Profile compares on every part of the identity). `Lens::operator==` (`record_store.h:146-150`) lists six fields and `efill_window_ms` is the sixth, so "six fields" is right. The `!=` against `Lens::from(10, 0, 20, false, kMaxEarlyFillWindowMs)` goes red if `operator==` drops the field. The two default lines compare `Lens{}.efill_window_ms` with `static_cast<int>(kDefaultEarlyFillWindowMs)`, the same expression as the member initializer (`record_store.h:125`). They go red only if the initializer moves off that constant; they do not check 60 against the ruling. Agree: Investigate, claim not checked only (old R8 verdict stands).

# Splits

1. Test 456 pins a log that grows while a hold is open, while production sets a journal size limit (`record_store.cpp:850`). The record's open question (growth accepted or bounded) is a real split for the user.
2. "A second connection answers as the writer" is stated in 171, 260 and 334 (own reader), and 124 and 536 (in-memory). One rule, five tests. The record lists it.
3. In-memory store: tests (124, 536) and `tools/bench.cpp:103` open `:memory:`; the app never does. The record's question about the `OwnReader` in-memory branch stands, and bench is a caller it missed.
4. Early-fill 60 versus 85: no split (line 57 names the 60 default and the 0 to 85 range, later than D344).
5. No split the auditor missed beyond items 1 to 3's added detail.

# Not reached

None. Every record was opened. For the eight "StoreFile only" records I checked the diff hunk and the old record's verdict, but did not re-read their bodies line by line, since nothing in them changed.
