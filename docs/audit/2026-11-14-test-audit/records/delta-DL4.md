# Delta records DL4: tests/test_library_jobs.cpp, tests/test_library_model.cpp, tests/test_model.cpp, tests/test_config.cpp

Range read: `git diff c10c7f54 5ec682c7` on each file, code read at `5ec682c7`. No test case was removed in these four files (the diff has no deleted `TEST_CASE` line). In scope: 28 test cases (jobs 8 new; library model 5 new; model 4 changed and 3 new; config 5 changed and 3 new) and the helpers listed per file. Nothing was run. Every Q8 answer is "reasoned".

Shared reads for the whole file: `docs/agents/test-rubric.md`, `docs/agents/rulings/code-and-engine.md` lines 56-58 (under `## Confirmed`: "make the early fill threshold an adjustable setting pinned to one number in both directions", "so we should use the hit window. and stop inventing our own numbers.", "analysis settings panel, default 60ms, supports 0ms to 85ms", all 2026-11-15, session a3ba79d1), `src/core/model.h` 108-150 and 296-385, `src/core/model.cpp` 473-475, 535, 851-855, `src/ui/library_jobs.cpp` 484-615, `src/ui/library_jobs.h` 340-361, `src/ui/library_model.cpp` 61-90, 262-330, `src/ui/app_state.cpp` 255-285, `src/app/analysis.cpp` 735-790, 852-925, `src/app/config.cpp` 160-215, 405-470, `tests/store_lock_holder.h` 18-43, `records-engine/index.md` lines 1-6 and `records-screen/index.md` (grep). Old records: `A16.md` (headings), `C5.md` (verdict lines), `A18.md` records 7, 8, 9, 20, `A9.md` records 1, 2, 7, 11, 12.

Charts and captures needed and not available: (a) a Library view capture of rows after a batch, with a chart in two folders, showing chips and row text (jobs 1-3 and library model 2-5 tie to it); (b) an engine dump of a real chart analysed at early-fill windows 0, 60 and 85 (the seven dumps in `records-engine/` were made before the setting existed, so they carry no window; their E0 on `beg` activation 4 at `e_offset` 0.0 holds at any window above 0); (c) a real chart with an E1 (one early fill passed over) on an all-0 path; (d) a capture of the settings button and Reset tooltip with the early-fill window off its default; (e) a real `hydra_settings.ini` copy that carries `early_fill_window_ms` and one that does not.

---

# tests/test_library_jobs.cpp

Helpers new in this file (each judged for what its callers rely on):
- `real_in_turn(released, real)` (line 408): the n-th started chart waits until `released > n`, then returns `real`. Does what callers rely on. The `started` counter is shared by all workers, so it is right only for `workers=1`; every caller passes 1. Callers: jobs 5, 6, 7.
- `always(real)` (line 419): returns `real` at once. Callers: jobs 1, 2, 3, 8.
- `same_answer(a, b)` (line 427): compares six fields of a `RecordLookup`. Does what callers rely on if `RecordLookup` has no other field; `same_answer` lists fields by hand, so a field added later is silently not compared (not checked against `src/store/record_store.h`: not read). Callers: `check_answers_match_store`.
- `answers_for(rows, md5)` (line 434): counts tray answers for one id. Callers: jobs 2, 3, 7.
- `check_answers_match_store(rows, store, run)` (line 440): each tray answer equals `store.lookup_one(md5, run.profile)` asked afterward. Right shape: the owner's answer, item by item. Callers: jobs 2, 3, 4, 5, 6, 7, 8.
- `charts_with_a_copy(n)` (line 450): `fake_charts(n)` plus chart 0 under a second folder. Callers: jobs 1, 3.

## 1. jobs: answered_count reaches the number of charts saved, and counts a chart in two folders once (tests/test_library_jobs.cpp:464)
Delta round: new
Read: lines 401-480; `BatchJob::answered_count`, `note_saved`, `answer_waiting` (library_jobs.cpp:499-543); library_jobs.h:340-361; `fake_charts` (67-78). Adding commit c28427c2 (subject only). Not read: `corpus::first_analyzed_with_paths` body, `plan_of`.
1 Claim: after a batch of two charts, one of them listed in two folders, the job reports three charts done and two answered, and taking the tray does not lower the count.
2 Owner: `BatchJob::answered_count` (library_jobs.cpp:499); only caller in `src/` is none (grep `answered_count` in `src/`: the header's own declaration; the comment says "so tests wait on this instead"). A count only tests read. Not a display rule: no screen shows it.
3 Real chart: claim not checked on a real library; the claim is about the batch job's own counter and its real worker thread and real in-memory store. Independent owner for "two folders is one chart": `plan.todo` size (the test REQUIREs it in test 3).
4 Direct: 477 `completed == 3` (three rows, from `charts_with_a_copy(2)`: named inputs). 478 `answered_count() == 2` (two charts). 480 count unchanged after the tray is taken. All follow from the fixture's own comment.
5 Reason: inputs are the test's own; 3 and 2 are the fixture's rows and charts, named in the comment at 470.
6 Display: not a display test; no screen reads `answered_count`.
7 Failure: c28427c2 added the tray; named failure: a chart in two folders counted twice, so a wait for "all answered" never ends or ends early. Possible.
8 Deleted: 477 (a row count; a completed that skipped the copy row, reasoned). 478 (a double count of the second folder: `answered_md5s_` check at library_jobs.cpp:508 removed, reasoned). 480 (the tray take resetting the count, reasoned).
9 Stand-in: none. `completed` and `answered_count` are separate owners.
10 Same rule: test 3 also checks `rows.answers.size() == 2` for the same chart-in-two-folders case.
11 Real: yes; a chart in two folders is `library_model.h:79-82` (audit finding 143).
Verdict: Keep.
Side note: the test name holds a comma (line 464).

## 2. jobs: a batch job's tray holds the store's answer for each chart it saved (tests/test_library_jobs.cpp:481)
Delta round: new
Read: lines 481-496; `answer_waiting` (library_jobs.cpp:518-543); `take_saved_rows` (492-497); helpers above.
1 Claim: after a batch, the tray holds one answer per saved chart, each equal to what the store says for that chart now.
2 Owner: `RecordStore::lookup` (called library_jobs.cpp:523), taken by `LibraryModel::set_summaries_for` through `app_state.cpp:271-277`. The engine's store owns the answer; the job copies it. Right place.
3 Real chart: claim checked on the real store (in-memory SQLite, `:memory:`) with a real corpus analysis result (`first_analyzed_with_paths`) saved under made-up ids "fake0", "fake1"; owner compared: `store.lookup_one`. No chart-play fact is claimed. Library screen side: claim not checked (capture (a)).
4 Direct: 491 `problem` empty. 492 two answers (two charts). 493-494 one answer per id. 495 each tray answer equals `lookup_one`: a property against the owner.
5 Reason: 2 is `fake_charts(2)`; no rule number.
6 Display: the tray feeds rows (`app_state.cpp:277`); the check compares against the store's record, not a typed value. Passes question 6.
7 Failure: c28427c2; named failure: rows after a batch showing a status the store does not hold. Possible.
8 Deleted: 491 (a lookup problem hidden, reasoned). 492 (a missing or doubled answer). 493-494 (an answer under the wrong id). 495 (the job deciding a status of its own, which its comment says it must not).
9 Stand-in: `same_answer` lists fields by hand (see helper note).
10 Same rule: tests 3, 4, 5, 6, 7, 8 call the same check.
11 Real: yes; every batch does this.
Verdict: Keep.

## 3. jobs: a chart in two folders is asked about once (tests/test_library_jobs.cpp:498)
Delta round: new
Read: lines 498-522; `note_saved`/`answer_waiting` (library_jobs.cpp:504-543); `set_lookup_hold_for_test` (library_jobs.h:361).
1 Claim: a chart listed in two folders gets one store question and one tray answer, not one per folder.
2 Owner: `BatchJob::note_saved` dedupes (`answered_md5s_` and `waiting_`, library_jobs.cpp:508). The only caller of the dedupe is the job's own two callbacks (581-586). Fine.
3 Real chart: claim checked with the real worker thread and real store; the lookup count is read through the job's own test seam, not an independent owner. Screen side: claim not checked (capture (a)).
4 Direct: 517-519 one answer for each id, two in all. 520 `lookups == 2`, one per chart, none for the second folder. 521 tray matches the store.
5 Reason: 2 is the charts of `charts_with_a_copy(2)`, named in the comment on 520.
6 Display: no.
7 Failure: c28427c2; named failure: the second folder's row triggers a second store read per batch chart, doubling the work for libraries with copies. Possible.
8 Deleted: 520 (the dedupe at 508 removed, reasoned). 517-519 (a duplicate answer).
9 Stand-in: a hold hook counts lookups; it counts calls to `answer_waiting` that reach the hook, so an empty `waiting_` skips it (library_jobs.cpp:520). That makes the count exact only because the early return precedes the hook. A reordering would change the count without a bug. Noted, not a fault today.
10 Same rule: test 1 checks the same dedupe through `answered_count`; one verdict covers 1 and 3.
11 Real: yes.
Verdict: Keep.

## 4. jobs: a failed chart whose failure was stored reaches the tray as the store's answer (tests/test_library_jobs.cpp:524)
Delta round: new
Read: lines 521-558; `prepare_failed_chart` (analysis.cpp:782-789), `save_result` (756-771), `report` (852-867); `store_lock`-free. Not read: `remembered_failure` body, `exec_on_file`, `NoNotesError` base class.
1 Claim: of three failed charts, only the one whose failure the store keeps gets a tray answer; one the store does not keep, and one whose failure record could not be saved, get none.
2 Owner: `remembered_failure(kind)` decides what is kept (analysis.cpp:787); the job asks only after `on_failure_saved`. Callers of `on_failure_saved`: analysis.cpp:862 only. Right place.
3 Real chart: real file store with a real SQLite trigger from a second connection (`exec_on_file`), real worker thread. Not a chart claim. Screen: claim not checked (capture (a)).
4 Direct: 546 `failed == 3`. 549 `problem` empty. 550 one answer. 551 it is "fake0". 552 tray matches store. 553 `answered_count == 1`.
5 Reason: kinds chosen by the test (SongFileMissing, NoNotes); each says why in the comment at 535-536. The claim that SongFileMissing is "a failure the store does not keep" rests on `remembered_failure`, which I did not open: not read.
6 Display: no.
7 Failure: c28427c2 (message only); named failure: a failed chart the store never saved showing a stale answer, or a failed save leaving a phantom row. Possible.
8 Deleted: 551 (answer for the wrong chart, reasoned). 553 (the count including the unsaved ones).
9 Stand-in: a count of 1 plus the id check; fine together.
10 Same rule: none found in `tests/` for the "failed save leaves no answer" half (searched `on_failure_saved` in tests).
11 Real: the trigger stands for a real write failure ("run_batch counts a failed save as a failed chart"), named in the comment at 528-530.
Verdict: Keep.
Side note: the comment cites "O7 and O13" (line 521); these labels name items in a plan not opened here.

## 5. jobs: the tray is empty after it is taken, and a stopped run's tray holds only what it saved (tests/test_library_jobs.cpp:560)
Delta round: new
Read: lines 560-585; `analysis.cpp:904, 921` (a chart that finishes after cancel is dropped); `take_saved_rows`.
1 Claim: after Stop, the tray holds the answer of the chart that was saved before Stop and none for the one that finished afterwards; a second take returns nothing and the count stays.
2 Owner: `run_batch`'s cancel drop (analysis.cpp:921) plus `take_saved_rows`. Right place.
3 Real chart: real thread and real store; the stop is real (`job.stop()` sets `cancel_`). Screen claim: not checked (capture (a)).
4 Direct: 576 one answer; 577 it is "fake0"; 578 matches store; 581-582 second take empty; 583 count stays 1.
5 Reason: inputs are the test's own.
6 Display: no.
7 Failure: c28427c2; named: a stopped batch showing a row as analyzed for a chart whose result was dropped. Possible.
8 Deleted: 576-577 (a post-stop chart answered, reasoned). 581-582 (a take that does not empty the tray). 583.
9 Stand-in: none.
10 Same rule: `A16.md` 8 ("stop while paused ends the run and fails nothing"): a different promise.
11 Real: yes.
Verdict: Keep.
Side note: the test name holds a comma (line 560).

## 6. jobs: a paused batch job's tray keeps its answers (tests/test_library_jobs.cpp:587)
Delta round: new
Read: lines 587-620; `pause`/`wait_while_paused` (not read).
1 Claim: while paused, the chart already running finishes and is answered; the next one never starts; the tray keeps that answer and the rest arrives after resume.
2 Owner: the pause gate (`wait_while_paused`, library_jobs.cpp:595). Right place.
3 Real chart: real thread; not a chart claim. Screen: claim not checked.
4 Direct: 608 `answered_count == 1` after a fixed 200 ms. 609 paused. 612-614 first take one answer "fake0". 618-619 after resume one more, "fake1".
5 Reason: 200 ms is the test's own wait. It stands for "long enough for the second chart to start if the pause failed". No source for that length.
6 Display: no.
7 Failure: c28427c2; named: pausing losing an answer, or the second chart sneaking in. Possible.
8 Deleted: 608 can fail only if the second chart's answer lands inside 200 ms; a slower bug passes (reasoned).
9 Stand-in: a fixed sleep stands in for "the second chart never starts". The test could read the analyzer's start count after `released = 2` instead; the sleep still decides when to look.
10 Same rule: `A16.md` 7 ("pause lets the chart in flight finish and starts no new one") makes the pause half of this promise; one verdict covers the pause half.
11 Real: yes.
Verdict: Rewrite. Keep 612-619 (the tray keeps the paused answer, the rest arrives after resume). Replace the 200 ms sleep with a check that does not depend on how fast the machine is (the analyzer's own start count, read after the paused chart's answer).
Today it catches: a pause that drops the finished chart's answer, and a resume that loses the second chart's.

## 7. jobs: a lookup that fails leaves its sentence in the tray and is tried again (tests/test_library_jobs.cpp:622)
Delta round: new
Read: lines 618-680; `answer_waiting` catch block (library_jobs.cpp:532-542); `run()` final `answer_waiting` (612).
1 Claim: if the store cannot answer for a saved chart, the tray gets the same sentence a failed store read shows, no answer, and the chart is asked about again with the next chart.
2 Owner: `app::plain_error` (the sentence), `BatchJob::answer_waiting`; screen consumer `app_state.cpp:278-281` (`set_problem`). Right place.
3 Real chart: real SQLite file; a second real connection renames the `records` table (a real fault). The expected sentence is made by calling the store itself (`lookup_one` at 650-655), an independent owner. Screen: claim not checked.
4 Direct: 651 answers empty. 652 count 0. 659 problem equals the store's own sentence. 667-669 no run error, no failed chart. 670 `lookups == 2`. 672-676 both answers. 677 tray matches store. 678 count 2.
5 Reason: inputs own; `lookups == 2` comment: chart 1's lookup asks about chart 0 again.
6 Display: the sentence is compared to the store's real sentence, not typed.
7 Failure: c28427c2; named: a transient store failure losing a chart's row update for the rest of the batch. Possible.
8 Deleted: 659 (a different sentence, reasoned). 670 (no retry). 672-676 (the retried chart never answered).
9 Stand-in: none.
10 Same rule: `app_state.cpp` set_problem path is tested elsewhere (not searched in DL4).
11 Real: yes, a locked or damaged library file.
Verdict: Keep.

## 8. jobs: a frame's snapshot does not wait while the batch job waits on the store (tests/test_library_jobs.cpp:682)
Delta round: new
Read: lines 682-715; `StoreLockHolder` (store_lock_holder.h:18-43); `snapshot`, `take_saved_rows` (library_jobs.cpp:484-497); `answer_waiting` comment at 522.
1 Claim: while the batch thread waits on the store lock for a lookup, the window's snapshot and tray take still return at once.
2 Owner: `BatchJob` (mu_ held only briefly, lookup run outside it, library_jobs.cpp:522-529). Callers of `snapshot`/`take_saved_rows`: `app_state.cpp:256-271` every frame. Right place.
3 Real chart: real mechanism: a real second thread holds the real store lock (`begin_save_group`), a real worker waits on it. Not a chart claim.
4 Direct: 702 `returned_while_held` (the holder had not let go). 703 not finished. 704 no answers yet. 712-714 after release the one answer arrives.
5 Reason: 50 ms (line 699) is the test's own wait for "the lookup now waits on the store lock". Nothing observable says the worker reached the lock.
6 Display: no.
7 Failure: c28427c2; named: the window freezing while a batch saves. Possible and visible.
8 Deleted: 702 (snapshot taking the job's lock during a lookup would hang until the holder's cap, then the check turns false, reasoned). The line can pass wrongly if the worker is slow to reach the lock within 50 ms.
9 Stand-in: a fixed sleep stands for "the worker is blocked".
10 Same rule: none.
11 Real: yes.
Verdict: Rewrite. Keep the shape (real second thread, real lock). The 50 ms wait must become something the worker's own state proves; today a busy machine lets a lock-held-under-mu_ bug pass.
Today it catches: a snapshot that blocks on the job's own lock while the lookup waits (on an idle machine).

---

# tests/test_library_model.cpp

Helpers new: `two_folders_and_two_more()` (line 730): builds one chart in two folders plus two more charts, all Not analyzed; callers: model tests 1, 2, 3 (below). `answer_for(answers, md5)` (line 740): finds the answer for an id, REQUIREs it exists; callers: model test 1. Both do what callers rely on.

Shared: `LibraryModel::set_summaries_for` (library_model.cpp:286-296) applies each answer by id with `insert_or_assign` and rebuilds once when any row changed; `set_summary_for` is a one-line call to it (line 283). Only caller of `set_summaries_for` in `src/`: `app_state.cpp:277` (the batch tray). Screen claim for every test here: not checked; capture (a) needed.

## 1. library model: set_summaries_for gives every row the answer set_summary_for would (tests/test_library_model.cpp:749)
Delta round: new
Read: lines 727-779; `set_summary_for` (library_model.cpp:283), `set_summaries_for`, `shows_same_as`/`apply_summary` (61-90). Adding commit 5079b7e9 (subject only).
1 Claim: giving the model many chart answers at once leaves every row, and every chip count, as giving them one at a time does.
2 Owner: `LibraryModel::set_summaries_for`. `set_summary_for` is defined as `set_summaries_for({{md5, lookup}})`, so the "one at a time" side is the same function. Caller of the many-at-once: `app_state.cpp:277`.
3 Real chart: claim not checked (capture (a): a library with a batch landing mid-run).
4 Direct: 768 changed counts equal (compares a function with its own wrapper). 769 changed equals row count (every row was Not analyzed: a property). 771-775 each row `shows_same_as` the answer for its id in both models. 776-779 five chip counts equal between the two models.
5 Reason: inputs ("same" two folders, "first" stale, "second" can't-analyze) are the test's own.
6 Display: rows compared to the answer given; chip counts compared only to the other model.
7 Failure: 5079b7e9 "applies many charts' answers with one rebuild"; named failure: the bulk path skipping rows or leaving chip counts stale. Possible.
8 Deleted: 768, 776-779 can never fail while `set_summary_for` calls `set_summaries_for` (true by construction). 771-775 catch a row left unchanged or given another chart's answer.
9 Stand-in: yes, 768 and 776-779 compare two Hydra functions, one calling the other. `shows_same_as` is also the predicate `apply_summary` uses to decide a change, so a field it ignores would pass here too.
10 Same rule: `C5.md` 36 (both rows of a chart in two folders updated, test_library_model.cpp:715 at the time) and tests 3 and 5 below.
11 Real: yes.
Verdict: Rewrite. Check each row and each chip count against the answers given directly (a count of rows whose answer is Analyzed, Stale, Can't analyze), not against `set_summary_for`.
Today it catches: a row skipped or wrong in the bulk path (kept).
Production finding: none. Note `shows_same_as` serves as both the model's change test and this test's oracle.

## 2. library model: set_summaries_for applies a chart's later answer when it is listed twice (tests/test_library_model.cpp:781)
Delta round: new
Read: lines 781-792; `set_summaries_for` (`insert_or_assign`). Adding commit 2c9006d8 (subject).
1 Claim: if the answer list names a chart twice, both of its rows show the later answer.
2 Owner: `set_summaries_for`. Only caller (`app_state.cpp:277`) passes the tray, where each chart appears at most once (`answered_md5s_`, library_jobs.cpp:508; `waiting_` dedupe at 509).
3 Real chart: claim not checked; also no real caller makes a duplicate.
4 Direct: 783 returns 2 (two rows). 787 each "same" row shows the later answer. 790 two rows seen.
5 Reason: inputs own.
6 Display: no typed value.
7 Failure: 2c9006d8 subject "test that a chart listed twice gets its later answer"; failure: the order of answers mattering for the final row. Not reachable from the tray.
8 Deleted: 787 (first answer winning, reasoned).
9 Stand-in: none.
10 Same rule: none.
11 Real: no. The tray never lists a chart twice (library_jobs.cpp:508-510).
Verdict: Remove. The scenario cannot come from the one caller.
Today it catches: later-wins on a duplicate id, which no player can trigger.

## 3. library model: set_summaries_for updates both rows of a chart in two folders (tests/test_library_model.cpp:794)
Delta round: new
Read: lines 794-809.
1 Claim: one answer for a chart in two folders updates both rows and moves two rows to Analyzed, leaving the others.
2 Owner: `set_summaries_for` (matches by `entry.md5`, library_model.cpp:290-293). Caller: `app_state.cpp:277`.
3 Real chart: claim not checked (capture (a)).
4 Direct: 796 returns 2. 800 both rows show the answer. 803 two rows seen. 805 `analyzed == 2`. 806 `not_analyzed == 2`.
5 Reason: inputs own.
6 Display: chip counts are what the library header draws.
7 Failure: audit finding 143 (two rows per chart).
8 Deleted: 796, 800 (a second folder row left alone; also caught by test 1 lines 771-775). 805-806 (a chip counting charts instead of rows; only here).
9 Stand-in: none.
10 Same rule: test 1 (rows) and `C5.md` 36 (`set_summary_for`, same code path). One verdict covers the row half.
11 Real: yes.
Verdict: Investigate. Claim not checked only: needs capture (a), the Library chips and both rows of a chart in two folders after a batch. Lines 796-803 repeat test 1.

## 4. library model: set_summaries_for holds the row order under the Best path sort until sort_if_out_of_date (tests/test_library_model.cpp:811)
Delta round: new
Read: lines 811-827; `summaries_changed` (library_model.cpp:299-306); `sample()` (73-89).
1 Claim: while a batch lands answers, rows do not jump under a Best path sort; the order updates only when the model is told to resort.
2 Owner: `LibraryModel::summaries_changed`/`sort_if_out_of_date`. Caller of the resort: not searched beyond `app_state.cpp` area. Right place.
3 Real chart: claim not checked (a screen claim: the list does not jump; capture needed).
4 Direct: 817 order unchanged. 818 analyzed 3. 821 resort reports true. 823-824 typed title order {"YYZ","Halo","Burnout","Chair","Acid Romance","Other"}: a raw result (highest score first, 500,000, 400,000, 378,315, then unscored in previous order). 825 second resort false.
5 Reason: the scores 500000 and 400000 are the test's own inputs; the comment states the working.
6 Display: expected order typed, not derived from the rows' own scores.
7 Failure: the D-number behind "A new summary never moves a row by itself" is in library_model.cpp:300 comment; not opened. Possible.
8 Deleted: 817 (the sort applying at once, reasoned). 823-824.
9 Stand-in: none.
10 Same rule: the single-answer path in `C5.md` (not searched by name).
11 Real: yes.
Verdict: Rewrite. State 823-824 as "rows with a score come first in descending score, the unscored keep their earlier order", not a typed list. Screen claim also not checked (capture (a)).
Today it catches: rows jumping mid-batch, and the resort order.

## 5. library model: set_summaries_for with answers that change nothing rebuilds nothing (tests/test_library_model.cpp:829)
Delta round: new
Read: lines 829-848; `apply_summary` (library_model.cpp:80-90), `sample()` 73-89 (burnout, chair, yyz and the answers match line 87's: `ready(378315,"3- 1 2",7,163.0)`, `stale()`, a default lookup).
1 Claim: answers that equal what the rows already show change nothing: zero rows reported, chip counts the same, the Best path order not marked out of date.
2 Owner: `apply_summary` returns false when `shows_same_as`; `set_summaries_for` rebuilds only when `changed > 0`. Right place.
3 Real chart: claim not checked (capture (a)).
4 Direct: 838-840 returns 0. 841 returns 0 for an empty list. 842-846 five chip counts equal. 848 resort not needed.
5 Reason: each answer is copied from `sample()` and says so in the comment at 838.
6 Display: compares to the model's own earlier counts: a property.
7 Failure: the rebuild on every tick of a batch (the refresh runs about every `kBatchRefreshSeconds`).
8 Deleted: 840 (a changed count where nothing changed, reasoned). 848 (the `changed > 0` guard removed sets `order_out_of_date_` under Best path, so this line fails).
9 Stand-in: `shows_same_as` is both production's change test and the oracle for "same" (see test 1).
10 Same rule: none.
11 Real: yes, every refresh after the first batch tick.
Verdict: Investigate. Claim not checked only: capture (a), the Library list during a batch whose later ticks change nothing.

---

# tests/test_model.cpp

Shared: the early-fill window is now one argument (`window_ms`) to `fill_refuses`, `is_e0`, `allzero_activation` and `fill_within_limit` (model.h:331-372), carried on each `Activation` as `e_window_ms` (model.h:139-141) and filled from the graph at `search/engine.cpp:2030`. The engine's record: `records-engine/index.md` line 29 and `records-screen/beg.txt`: `beg` path 0 "1- 0 0 E0", activation 3 (fourth), `e_offset` 0.0 ("Early fill: 0.0 ms (required)"). That record was made before the setting existed, so it carries no window; at any window above 0 the E0 holds.

## 1. squeeze_difficulty and is_e0: one owner for the engine and the model (tests/test_model.cpp:147)
Delta round: changed, replaces A18.md record 7
Read: lines 147-168; `is_e0`, `squeeze_difficulty`, `early_fill_difficulty` (model.h:335-340); `Activation::is_E0` (model.cpp:475).
1 Claim: a squeeze's difficulty is its offset or the negative with no negative zero; an early fill is an E0 when its offset is under the window and nothing was skipped; the activation's own answers equal the free functions'.
2 Owner: `is_e0` (model.h:335); callers in `src/`: `Activation::is_e_critical`/`is_E0` (model.cpp:473-475), engine.cpp:859, 863, 1007.
3 Engine: `beg` path 0 activation 3, E0, `e_offset` 0.0. Claim: `is_e0(0.0, 0, w)` true. They agree at any window above 0. The edge at the window is on no captured chart: claim not checked at the edge.
4 Direct: new lines 154-157 type `w - 0.1` true, `w` false, `10.0` with one skip false, with `w` the default constant. 162 `is_E0() == is_e0(10.0, 0, a.e_window_ms)` (a member against the free function it calls).
5 Reason: `kDefaultEarlyFillWindowMs` is 60 by the confirmed ruling above (lines 56-58 of the rulings page); the edges follow from `e_offset < window`.
6 Display: not a display test.
7 Failure: d074e04b (message only); still possible.
8 Deleted: 154, 155 (the window edge, reasoned). 156 (skips ignored). 162 true by construction (Activation::is_E0 calls is_e0).
9 Stand-in: 162-165 (a member against the function it calls, as the old record said of lines 150, 161, 163).
10 Same rule: records for the edge in `test_model.cpp` tests 2-4 below, and `test_fills_and_skips.cpp` (not read).
11 Real: yes.
Verdict: Rewrite. The old verdict still holds: only the window argument changed in the edited lines. Drop the member-versus-function lines; check the edge from the rule.
Today it catches: sign of difficulty, `-0.0`, the E0 edge at the default window.
Side note: the old record's "pending ruling" is now filed, so the old note about lines 152-154 is closed by this change.

## 2. allzero_activation: no skips, or an E1 (tests/test_model.cpp:171)
Delta round: changed, replaces A18.md record 8
Read: lines 171-204; `allzero_activation` (model.h:374-380), `Path::is_allzero` (model.cpp:851-856).
1 Claim: a path is all-0 when no fill was skipped, or the one skipped was its early fill, whatever the window; and `Path::is_allzero` reads each activation's own window.
2 Owner: `allzero_activation`; callers engine.cpp:863, 1936, `Path::is_allzero`, pather.cpp (A18 record 8 lists 55, 104).
3 Real chart: claim not checked: an E1 on a real all-0 path is on no captured chart; also none at windows 60 and 85.
4 Direct: 176-181 the six cases run at windows 60 and 85 with `w - 0.1`, `-w`, `w`: edges from the rule. 184-185 70 ms: an E1 at 85, a plain 1 at 60. 198-203 the path's own window decides.
5 Reason: 70 is between 60 and 85, named in the comment at 184 ("70 ms to spare"). Window values: ruling lines 56-58.
6 Display: no.
7 Failure: 9dd25638 (A18); D102 as quoted in A18 record 8.
8 Deleted: 177-181 (edge moved, reasoned). 184-185 (a side reading a constant). 198-203 (the path ignoring the activation's window).
9 Stand-in: none.
10 Same rule: tests 3 and 4 below.
11 Real: yes.
Verdict: Investigate. Claim not checked only (capture (c): a real all-0 path with an E1, at 60 and 85). The old verdict holds; the changed lines now follow the ruling.

## 3. early-fill window: the refusal and the E0 edges (tests/test_model.cpp:206)
Delta round: changed, replaces A18.md record 9
Read: lines 206-237; `fill_refuses`, `is_e0`, `fill_within_limit`, `timing_within_limit` (model.h:304, 331-372).
1 Claim: at the default window and at the widest one, a fill refuses only when SP came ready later than the window after the deadline, and an activation is E-critical only when it came ready less than the window before; between the two windows each side answers differently.
2 Owner: `fill_refuses`, `is_e0`, `fill_within_limit`; engine.cpp:858-869, 1007, 1224 and `Activation::within_ms_limit` (model.cpp:535).
3 Engine: `beg` activation 3 E0 at 0.0 ms. Claim: E at under the window. Agree at 0.0. The edges and the 70 ms case are on no chart: claim not checked at the edges.
4 Direct: 210-215 edge lines at `-w`, `-w - 0.1`, `w`, `w - 0.1` (from the rule), 218-225 the member `is_e_critical()` against `is_e0` (a member against the function it calls). 231-236: 70 ms at 85 and 60; 236-237 `fill_within_limit(70.0, 0, -80.0, w)`.
5 Reason: windows 60 and 85 follow the ruling lines 56-58. The limit `-80.0` has no stated reason: nothing says why -80 and not -70 or -100. `70.0` is named in the comment.
6 Display: no.
7 Failure: d074e04b; the two sides drifting apart (D344: "so we should use the hit window. and stop inventing our own numbers."). Possible.
8 Deleted: 210-213 (edge moved, reasoned). 218-225 true by construction. 236-237 depend on the typed -80.
9 Stand-in: 220, 222, 224 compare a member with the function it calls.
10 Same rule: tests 1, 2, 4.
11 Real: yes.
Verdict: Rewrite. Drop 218-225. Name the limit in `fill_within_limit(70.0, 0, limit, w)` from the offset (a limit just inside and just outside `early_fill_difficulty(70.0)`), not -80.
Today it catches: both sides reading the window argument instead of a constant, at 60 and at 85.
The old verdict (Investigate, claim not checked at the edges) still holds as a second finding: capture (b).

## 4. early-fill window: every fill that refuses is an E0 at the same window (tests/test_model.cpp:239)
Delta round: new
Read: lines 239-253; `fill_refuses`, `is_e0`; engine.cpp:858-863 (`is_e0` "holds for every fill that refuses").
1 Claim: at every window from 0 to 85, any offset where a fill refuses is also an E0.
2 Owner: `fill_refuses`/`is_e0`; the engine leans on it at engine.cpp:858-859.
3 Real chart: claim not checked (a real chart where a fill refuses, from the engine's `refused` count at engine.cpp:862, is not in the seven records).
4 Direct: 248-250 for each offset and window, refuses implies E0. 251 at least one refused offset.
5 Reason: windows 0, 1, 30, 60, 70, 85 are the test's own picks; offsets are a sweep -200 to 200.
6 Display: no.
7 Failure: named: someone narrows E0 so a fill the engine refuses is no longer an E0 and the engine's guard at 858 drops it. Possible.
8 Deleted: 250. For any window of 0 or more, `e < -w` already implies `e < w`, so the line is true by construction and catches nothing: a side that read a constant (kHitWindowMs 85) would still pass here, because refused offsets are under 0 and under 85. Reasoned.
9 Stand-in: a floor `refusing > 0` as a "the loop ran" guard (251, acceptable as the rubric's fix).
10 Same rule: test 3 pins both edges at 60 and 85, which already fails if either side stops reading its window.
11 Real: yes.
Verdict: Remove. Test 3 makes the promise (each side reads its own window).
Today it catches: nothing test 3 does not.

## 5. early-fill window: at 0 ms no spawning activation is an E (tests/test_model.cpp:255)
Delta round: new
Read: lines 255-273; model.h:331-380.
1 Claim: with the window at 0, a fill spawns only if SP was ready by the deadline, and nothing that spawns is an E, an E1 or in the limit's business.
2 Owner: the model.h functions with window 0; callers as above.
3 Real chart: claim not checked: needs an engine dump of a real chart at window 0 (capture (b)); at 0 ms `beg` activation 3 would lose its E0.
4 Direct: 258-259 refuse at -0.1, spawn at 0.0. 261-268 over offsets 0 to 200: never refuses, never E0, never an E1, always within the limit. 270-273 activation with window 0 has no E mark and no early-fill difficulty.
5 Reason: 0 is the ruling's lower bound (line 58); 200 is the sweep's own end.
6 Display: no.
7 Failure: the setting's lower end producing an E mark or a refusal at an offset the user said is off.
8 Deleted: 259 (`<` to `<=`, reasoned). 264-266 (a side reading a constant). 273 (`e_difficulty` returning a value with the window at 0).
9 Stand-in: none.
10 Same rule: test 3.
11 Real: yes, 0 is a setting value the user allowed.
Verdict: Investigate. Claim not checked only: capture (b).
Side note: the comment at line 253 cites "(O19)", a plan label not opened here.

## 6. early-fill window: every default is kDefaultEarlyFillWindowMs (tests/test_model.cpp:275)
Delta round: new
Read: lines 275-285; `kDefaultEarlyFillWindowMs` (model.h:112), `SearchSettings` (search/pather.h:45), `ScoreGraph` (graph.h:201, 277), `Lens` default (record_store.h:129).
1 Claim: the search settings, an activation, a saved record and the search graph all start from one default window, which sits inside 0 to 85.
2 Owner: `kDefaultEarlyFillWindowMs`; many in-`src` initialisers read it.
3 Real chart: claim not checked on a real run: no dump in `records-engine/` carries the window (capture (b)).
4 Direct: 278-281 four carriers equal the constant (a property against the owner). 283-284 inside range.
5 Reason: the number is the constant; its value is in test_config.cpp:255 (60, from the ruling).
6 Display: no.
7 Failure: one carrier defaulting to a different window (a wrong result for any chart analysed from a default).
8 Deleted: 278-281 (a carrier initialised to 85, reasoned). 283-284 (a default outside range; only here).
9 Stand-in: none. `Lens{}` and `Settings{}` are not checked here (test_config.cpp:255 checks Settings).
10 Same rule: test_config.cpp:254 (Settings default).
11 Real: yes.
Verdict: Investigate. Claim not checked only: a real run's record, with the window it was made at (capture (b)).

## 7. Activation notationstr: E prefix, skips, symbols (tests/test_model.cpp:654)
Delta round: changed, replaces A18.md record 20
Read: lines 654-686; `Activation::notationstr` (model.cpp:543-548, not re-read); `is_e0`.
1 Claim: a path token carries E when the activation is E-critical under its own window, then the skip count and one symbol per squeeze.
2 Owner: `Activation::notationstr`; callers (A18 record 20): `pathstring`, `pathstring_read.cpp`, path_view.cpp, pb_coach.cpp.
3 Engine: `beg` path 0 "1- 0 0 E0" (A18 record 20). Claim: E on the E-critical activation. Agree. The window edge and "E2+-" are on no chart.
4 Direct: 658 "2" at 300 ms. 661 "2" at the window. 664 "E2" at 50. 667 "E2" at `window - 0.1`. 671-673 with window 85 and 84.9 "E2". 675-676 back at 60, 84.9 gives "2". 683 "E2+-".
5 Reason: 300 and 50 are free inputs; 84.9 is `kHitWindowMs - 0.1`, working shown.
6 Display: the token is read back by `pathstring_read.cpp`, so it is a notation rule.
7 Failure: as A18 record 20; the new lines catch an E prefix that ignores the activation's window.
8 Deleted: 661, 667 (edge moved). 671-676 (a constant used in place of the activation's window). 683 (symbol order).
9 Stand-in: none; the typed token strings are the notation's own grammar.
10 Same rule: tests 1 and 3 (the edge).
11 Real: yes.
Verdict: Investigate. The old verdict holds: claim not checked only at the edge on a real chart.
Side note: the test name holds a comma (line 654).

---

# tests/test_config.cpp

Helper changed: `check_same_settings(r, s)` (line 53): gained `CHECK(r.early_fill_window_ms == s.early_fill_window_ms)` at line 68; it compares each field one by one, so the new field is covered. Callers: "settings round-trip" (line 101) and "settings: load and save name the same keys" (line 109). Does what its callers rely on.

## 1. settings round-trip through an INI file (tests/test_config.cpp:76)
Delta round: changed, replaces A9.md record 1
Read: lines 76-102; `keys()` (config.cpp:160-215, `early_fill_window_ms` at 201).
1 Claim: every setting saved to `hydra_settings.ini` reads back unchanged, now including the early-fill window.
2 Owner: `Settings::save_file`/`load_file` (config.cpp); callers `Settings::load`, `Settings::save` (A9 record 1).
3 Real chart: claim not checked on a real settings file (capture (e)); no real file carrying the key.
4 Direct: 91 sets 37; 101 `check_same_settings` compares field by field.
5 Reason: 37 is a free input inside 0..85 and differs from the default 60, so a dropped field shows.
6 Display: no.
7 Failure: a key saved under a name not read back.
8 Deleted: 91 (key missing from the key table: the loaded value would be 60, reasoned).
9 Stand-in: none.
10 Same rule: tests 2, 4 and 5 below.
11 Real: yes.
Verdict: Rewrite. The old verdict holds (give `mslimit_value` and `backendlimit_value` different values; add a real file copy). The new line is sound.

## 2. settings: load and save name the same keys (tests/test_config.cpp:104)
Delta round: changed, replaces A9.md record 2
Read: lines 104-135.
1 Claim: the key names `save_file` writes are the key names `load_file` reads.
2 Owner: the key table in config.cpp:160-215.
3 Real chart: claim not checked on a real file (capture (e)).
4 Direct: 128-132 a typed list of key names, now with `early_fill_window_ms` between `legacy_fills` and `auto_open_report`.
5 Reason: the list is a copy of the key table.
6 Display: no.
7 Failure: a rename in one side only.
8 Deleted: 128-132 only fail if someone edits the table without the list; it copies the code.
9 Stand-in: the typed list is a copy of `keys()`.
10 Same rule: test 1.
11 Real: yes.
Verdict: Rewrite. The old verdict holds: drop the typed list, load a real `hydra_settings.ini` copy, keep the default round trip.

## 3. a missing INI yields defaults (tests/test_config.cpp:235)
Delta round: changed, replaces A9.md record 7
Read: lines 235-252.
1 Claim: with no settings file, the app starts on defaults, the early-fill window among them.
2 Owner: `Settings::load_file` and the field initialisers in config.h:195; callers as record 1.
3 Real chart: claim not checked (a first run has no file; capture (e)).
4 Direct: new 249 `early_fill_window_ms == kDefaultEarlyFillWindowMs` (the owner's constant). 238-248 older typed lines 4, 10, 50, 40.
5 Reason: the new line takes its value from the constant, not a typed 60.
6 Display: no.
7 Failure: a first run with the wrong window.
8 Deleted: 249 (a default of 85, reasoned).
9 Stand-in: none for the new line; the typed lines are copies of the initialisers.
10 Same rule: test_model.cpp test 6, and the new config test 4 line 257.
11 Real: yes.
Verdict: Rewrite. The old verdict holds (compare every field to `Settings d`, drop the typed 4, 10, 50, 40); the new line already does it right.

## 4. early_fill_window_ms: round-trips, and a file without the line loads 60 (tests/test_config.cpp:254)
Delta round: new
Read: lines 252-277; `number("early_fill_window_ms", ..., 0, kMaxEarlyFillWindowMs)` (config.cpp:201); ruling lines 56-58.
1 Claim: the early-fill window is saved and read back at 0, 37 and 85; a file without the line gives 60; the line sets 72; text or a decimal refuses.
2 Owner: `Settings` and its key table. Right place.
3 Real chart: claim checked on hand-written INI text, not a real file: claim not checked on real input (capture (e)).
4 Direct: 257-258 default 60 (rule number: ruling "default 60ms"). 260 file with other keys loads 60. 262-270 round trip for 0, 37, 85 (the ruling's range, and one free input). 272 file line 72 loads 72. 274-277 `wide` and `60.5` refuse.
5 Reason: 60, 0, 85: the ruling's own numbers (`code-and-engine.md` lines 56-58). 37 and 72 are free inputs. Refusal rule is "as every number key does" (comment 273).
6 Display: no.
7 Failure: a window not saved, or a default of 85.
8 Deleted: 258 (a default change, reasoned). 262-270 (key dropped). 277 (decimals accepted).
9 Stand-in: none.
10 Same rule: test 3 (default) and test 5 (range).
11 Real: yes.
Verdict: Investigate. Claim not checked only: capture (e), a real settings file with and without the line.
Side note: the test name holds a comma (line 254).

## 5. early_fill_window_ms reaches the search and the result's key together (tests/test_config.cpp:279)
Delta round: new
Read: lines 279-306; `Settings::to_analysis_settings`/`lens`/`profile`/`batch_run` (config.cpp:405-440); `store::Profile`, `Lens` (not read beyond the use).
1 Claim: the window travels with the settings into the search and into the result's key, and moves nothing else in the key.
2 Owner: `Settings::lens`/`profile`; callers: `app_state.cpp` batch runs (not re-searched). Right place.
3 Real chart: claim not checked; no stored record made at two windows (capture (b)).
4 Direct: 284-287 settings carry 60.0 and 85.0 into the search and 60 and 85 into the lens. 291-299 the two profiles differ in the lens window and nowhere else. 302-305 a batch's run holds the window and equals the profile.
5 Reason: 60 and 85 from the ruling.
6 Display: no.
7 Failure: results saved under the wrong key after a window change (a stale or wrongly fresh library row).
8 Deleted: 285-287 (window not reaching the search). 296-299 (another field changing with it). 305 (the batch key differing from the settings' key).
9 Stand-in: `p60 != p85` compares profiles built by the same owner; fine.
10 Same rule: `A9.md` 4 (the 1.0 fills setting reaches the search and key together), a sibling; the two are one pattern.
11 Real: yes.
Verdict: Investigate. Claim not checked only: capture (b), a stored record of a real chart at two windows (store key rows).

## 6. settings: a value outside its range loads at the nearest edge (tests/test_config.cpp:370)
Delta round: changed, replaces A9.md record 11
Read: lines 370-401 (diff lines at 396-399).
1 Claim: an out-of-range number loads at its nearest edge; added: window 200 loads 85, -5 loads 0, 0 and 85 stay.
2 Owner: `pull_into_range` through the key table.
3 Real chart: claim not checked on a real file (capture (e)).
4 Direct: four lines 397-400: 200 gives 85, -5 gives 0, 0 gives 0, 85 gives 85.
5 Reason: 0 and 85 from ruling line 58. 200 and -5 are free inputs beyond each edge.
6 Display: no.
7 Failure: a window above the hit window slipping in.
8 Deleted: 397 (upper clamp missing). 398 (lower clamp missing). 399-400 (an edge moved inside).
9 Stand-in: none.
10 Same rule: test 7 below (`Settings::clamp`); the old verdict (fold test 12's `clamp` lines into a property) still holds.
11 Real: yes.
Verdict: Rewrite. The old verdict holds; the four new lines are sound.

## 7. Settings::clamp is the one range every caller asks (tests/test_config.cpp:403)
Delta round: changed, replaces A9.md record 12
Read: lines 403-420.
1 Claim: `clamp` gives the early-fill window's range, 0 to 85.
2 Owner: `Settings::clamp`; the Early fill window box asks it (settings_bar.cpp:156-160, not re-read).
3 Real chart: claim not checked.
4 Direct: 414 200 gives 85; 415 -5 gives 0; 416 42 gives 42.
5 Reason: ruling line 58.
6 Display: the box reads the range from the owner.
7 Failure: the box and the file disagreeing on the range.
8 Deleted: 414-415 (an edge moved, reasoned). 416 (clamp pulling an in-range value).
9 Stand-in: repeats test 6's lines 397-398 through another function.
10 Same rule: test 6.
11 Real: yes.
Verdict: Rewrite. The old verdict holds: state the range once, as a property over `clamp` and `load_file`.

## 8. settings summary: the early-fill window, off its default (tests/test_config.cpp:990)
Delta round: new
Read: lines 988-1005; `describe_settings` (config.cpp:461-470), `library_dialogs.cpp:80, 100, 363` (`kEarlyFillWindowName + " " + values.early_fill`); `format_ms_whole`; `settings_button_label`, `reset_settings_tooltip` not opened (only their use in `library_dialogs.cpp:100`).
1 Claim: the settings button and the Reset tooltip name the early-fill window as "Early fill window N ms" only when it is off 60, and "defaults" otherwise.
2 Owner: `describe_settings` and the label functions in `ui/library_dialogs.cpp`. Callers: the settings button and Reset tooltip.
3 Real chart: claim not checked: needs a capture of the button and tooltip with the window at 85 (capture (d)). `records-screen/beg.txt` shows only "Analysis settings: Score range 2 scores".
4 Direct: 993 "60 ms". 994 "Analysis settings: defaults". 996 "85 ms". 997 "Analysis settings: Early fill window 85 ms". 998 "Resets: Early fill window 85 ms". 1000 "Analysis settings: Early fill window 0 ms". 1001 `settings_at_defaults`. 1002 `with_analysis_defaults` window 60.
5 Reason: typed screen text; the name comes from `kEarlyFillWindowName`, which the test does not read.
6 Display: expected text typed, not read from `describe_settings` or the name constant. The values are typed copies of the display's own wording.
7 Failure: the setting off default not showing on the button, so the player cannot tell why results differ.
8 Deleted: 994 (default shown as non-default). 997-998 (window omitted). 1000 (0 treated as unset). 1001-1002 (Reset leaving the window).
9 Stand-in: typed strings are copies of the display's format.
10 Same rule: `A41.md` ("settings button reads ..."): the same label pattern for other settings; not re-read.
11 Real: yes.
Verdict: Investigate. Claim not checked only: capture (d), the real button and tooltip text at window 85 and 0.
Side note: the test name holds a comma (line 990).
