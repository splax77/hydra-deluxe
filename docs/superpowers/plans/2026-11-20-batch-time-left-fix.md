# Batch time left: stop the spikes (2026-11-20)

Written at main c71f745a. Nothing here is built until the user answers question 1.

## The plain version

While a batch runs, the strip under the toolbar says how long is left. In the user's screenshot it said "about 2:19 left". The same strip said the batch had done 12,667 of 19,698 rows in 0:08. At that pace the 7,031 rows left needed about 4 or 5 seconds, not 2 minutes 19. So the figure was wrong by about thirty times. The user asked "how is estimated finish time calculated? why does it spike so high?" and then said "Yes, plan a fix".

Here is why it spikes. The strip works out a speed from the last 16 finished rows only (16 is twice the 8 workers). It divides 16 by the time from the oldest of those 16 rows up to this very moment. Results reach the strip in bursts, because a chart is reported only after its save group of up to 16 charts is written to disk. So right after a burst the "last 16 rows" are that one burst, and the clock is really measuring "how long since the last burst landed". Then it multiplies. With 7,031 rows left, the figure is 7,031 divided by 16, which is 439, times that gap in seconds. Every tenth of a second with no burst adds 44 seconds to the figure. A gap of 0.32 s reads "about 2:19 left". The strip takes one such reading a second and holds it on screen for the whole second, so one unlucky instant is shown for a full second.

Decision D283 chose this rule on 2026-10-31. It was picked from one library run that finished in 2.8 s, where every rule on the table read "about 0:00 left" or "about 0:01 left", so that run could not show the fault. The session card says so in its own words: the window "could not be told apart on one library run".

The fix replaces D283's rule for the speed. It keeps everything else D283 and D276 item 8 set: the figure still waits for 2W finished rows, still changes at most once a second, and still leaves paused time out. Which new rule to use is the user's choice (question 1). The recommended one measures the speed over the last 15 seconds instead of the last 16 rows, and falls back to today's 16 rows only when a batch is so slow that fewer than 16 rows landed in those 15 seconds. The Song Preview's loading bar is not touched by any option.

One thing the brief asked about: why did D283's run take 2.8 s while the user's batch "shows minutes left"? The user's batch was not minutes long. Its own strip shows 12,667 rows in 8 seconds, so the whole batch needed about 12 seconds. The "minutes left" was the bug itself. What is still not known is why the app's batch ran about four times slower than the command-line run (12 s against 2.8 s), and what made results pause for 0.3 s. Both are `unchecked` rows P16 and P17 below. Neither blocks the fix, because the fix does not depend on what causes a pause.

## What was checked, and how

I read the code myself and did not take the explanation in the brief on trust (agent rule 9). The explanation holds, with two sharper points.

First, the rule. `TimeLeftClock::read` in `src/app/time_left.h` computes `units_ / (now - finishes_.front())` and then `(total - done_) / rate`. `BatchJob` builds the clock with `kTimeLeftWorkerRounds * workers_`, which is 2 times 8. `run_batch` in `src/app/analysis.cpp` reports a group's rows one after another only after `commit_save_group`, and `BatchJob`'s `on_progress` stamps each row with the batch clock at that moment. So a group's rows carry times a few microseconds apart. That is the burst.

Second, a group is not always 16. `run_batch` hands `flush_group` to the work pool as its idle call, and `run_work_pool` runs it whenever the consumer finds no result waiting. So when results trickle in, groups are small, and the last 16 rows span several groups. A full group of 16 forms when the saving thread falls behind the workers. Either way the figure is "rows left, times the seconds since the 16th-last row, over 16", and any pause in results is multiplied by rows left over 16.

Third, the measurement behind D283. It replayed the arrival times of `hydra_batch`'s `[i/N]` lines. Those lines are printed from the same `run_batch` callbacks the strip uses, so the bursts were in the data. The run was simply too short to show them: 19,499 charts in 2.8 s.

## Prior art

Progress bars usually smooth the speed over time, not over the last few items.

- indicatif (Rust) uses an exponentially weighted average of the rate with a time constant of 15 seconds: progress older than 15 s gets a total weight of 0.1 ([indicatif commit c670350](https://github.com/console-rs/indicatif/commit/c670350c7c8a61cbf5dc492cd317cc0e1e6808bb), [PR 539](https://github.com/console-rs/indicatif/pull/539), [issue 534](https://github.com/console-rs/indicatif/issues/534)). The issue says the estimator it replaced was thrown off by uneven steps, which is this bug's shape.
- tqdm (Python) uses an exponential moving average of the speed with a smoothing factor of 0.3 per update, where 0 means the average speed of the whole run and 1 means the instantaneous speed ([tqdm source](https://plafosim.readthedocs.io/latest/_modules/tqdm/std.html)).
- progressbar2 (Python) offers `AdaptiveETA`, a window of the last 10 samples, and `SmoothingETA` ([adaptive eta](https://progressbar-2.readthedocs.io/en/latest/widgets/adaptive-eta.html), [smoothing eta](https://progressbar-2.readthedocs.io/en/latest/widgets/smoothing-eta.html)). The D283 measurement already tried the 10-sample window as its candidate F.

What this plan takes from them: the window is a length of time, long compared with the gaps between bursts, and the 15 s in question 1 is indicatif's default. A plain window is offered instead of exponential weighting because Hydra's Preview clock is already a plain window, and a plain window can be stated on screen terms ("the last 15 seconds").

## Owner map

Written by scout eta-owner-map at main c71f745a, 2026-11-20. Unchanged from its report.

| Question | Owner | Its own words | Status |
|---|---|---|---|
| O1: How many charts is the batch through, and how many are still to go? | Counts: `BatchProgress` (`completed`, `total`) in `src/app/analysis.h:229`, written only by `run_batch`'s `report()` (`src/app/analysis.cpp:852`). The strip's copy is `BatchJob::snap_` (`src/ui/library_jobs.cpp:561`). Time left also keeps its own "done" in `TimeLeftClock::done_`, fed from the same `p.completed` (`library_jobs.cpp:570`). | `analysis.h:225`: "How far a batch run has got. run_batch is the only writer of every count here; a reader copies them rather than counting its own callbacks. Every count is in library rows, each copy of a chart counted (D76)". `time_left.h:57`: "Seconds left of `total` at the window's rate". | `split`. `run_batch` owns the counts, and `TimeLeftClock::done_` is a second "most work done" fed from the same number. They hold the same value today. |
| O2: When does a chart count as finished, as the strip sees it? | `run_batch`'s `report()` and `flush_group` in `src/app/analysis.cpp:852-890`. A chart is reported only after its save group commits, each of its library rows one by one. | `analysis.cpp:869`: "A group's charts wait here, in order, and are reported only once their group is committed, so a reported chart is always on disk." `analysis.cpp:847`: "Each of the chart's rows is counted and reported". | `found` |
| O3: How many workers does a run use? | `batch_worker_count()` (`src/app/analysis.cpp:712`, declared `analysis.h:255`). `BatchJob::workers_` copies it (`library_jobs.cpp:380`) and passes it to `run_batch`. | `analysis.h:252`: "The default batch pool size: one core is left for the UI (or shell) and for whatever else the machine is doing; capped so peak memory ... stays bounded." The pool itself runs `min(count, worker_count)` threads (`src/app/work_pool.h:80`). The strip's window uses `workers_`, not the pool's real thread count. | `found` |
| O4: How are charts grouped when saved and reported? | `store::kSaveGroupSize` = 16 (`src/store/record_store.h:400`), used by `flush_group` (`analysis.cpp:943`). | `record_store.h:398`: "The most charts one save group holds (D86 item 2). When a group closes sooner: see run_batch's flush_group". `analysis.cpp:872`: a group "closes when it is full, when a save lost it, and whenever the workers have nothing waiting". | `found` |
| O5: What clock does the batch use, and how are pauses left out? | `BatchClock` (`src/ui/library_jobs.h:254`, `library_jobs.cpp:337-365`). `BatchJob` feeds `time_left_.record(clock_.elapsed_s(...), p.completed)` and reads with `s.elapsed_s`. | `library_jobs.h:254`: "How long a batch has been working, with paused stretches left out." `time_left.h:23`: "Times are seconds on the job's own clock: the clock's caller decides what counts as time (the batch leaves paused stretches out of it)." | `found` |
| O6: How often may the time-left figure change? | `kTimeLeftStepSeconds` = 1.0 (`src/app/time_left.h:20`), applied in `TimeLeftClock::read` (`time_left.h:58`). | `time_left.h:17`: "A time-left figure changes at most once in this many seconds (D276 item 8), and the Preview's byte window spans at least this long." | `found` |
| O7: When does the batch's time left first show? | `TimeLeftClock::read` with the window size `kTimeLeftWorkerRounds * workers_` (`library_jobs.h:274`, `library_jobs.cpp:370`). It stays empty until `finishes_` holds that many entries (`time_left.h:68`). Empty also while paused or finished (`library_jobs.cpp:488`). The strip uses no age gate. | `time_left.h:30`: "Nothing shows until that many have finished." `library_jobs.h:321`: "Empty until that window is full, while paused, and once finished." | `found`. The Preview's 3 s gate, `kTimeLeftAfterSeconds` (see O10), is not used by the batch. |
| O8: How fast is the batch going, and so how long is left? | `TimeLeftClock::over_last_units` (`time_left.h:33`): the rate is `units_ / (now - finishes_.front())`, and the figure is `(total - done_) / rate`. `record` stamps every newly finished row, up to `units_` of them per reading, with that reading's time. | `time_left.h:31`: "The window is the last `units` units finished: the batch strip's charts (D283) ... The rate is `units` over the time from the `units`-th last finish to now." `time_left.h:35`: "Units that finish in one reading share its time." | `found` |
| O9: How is the time left written on screen? | Words: `app::time_left_text` (`src/app/display_format.h:61`), which uses `format_duration`. The strip's line is built in `src/ui/library_dialogs.cpp:457-458`. The gate-plus-words helper `app::time_left_line` (`time_left.h:100`) is Preview-only. | `display_format.h:59`: "How long a job has left, as the batch strip and the Preview loader say it: 'about 1:30 left'. The time itself is format_duration's." | `split`, though not a real conflict. The strip calls `time_left_text` directly and the Preview calls `time_left_line`, which calls `time_left_text`. That is one wording owner with two entry points. |
| O10: What does the Preview loader share with the batch's time left? | The shared parts are `TimeLeftClock`, `kTimeLeftStepSeconds` and `time_left_text`. The Preview-only parts are `ByteRateClock` (`src/ui/preview_load_job.h:72`, built on `over_last_second()`), `kTimeLeftAfterSeconds` = 3.0 (`preview_load_job.h:84`) and `time_left_line`. | `time_left.h:1`: "The batch strip and the Preview loader both read their time left from here; each brings its own unit (charts or bytes) and its own window." | `found` |
| O11: Does the startup screen show a time left? | `none` in the code. | `CONTEXT.md:310` ("UI timings"): "The startup screen shows no time left; its time-left line went with the copy upgrade (D115 task A1-6a)." | Nothing to own: the startup screen shows no time left by design. Grepped `src/ui` and `src/app` for `time_left`, `eta_s`, "left" strings and `TimeLeftClock`. |
| O12: How often does the strip redraw or refresh? | `AppState::kBatchRefreshSeconds` = 1.0 (`src/ui/app_state.h:447`). It governs how often saved rows reach the library table, not the time-left figure. | `app_state.h:348` and `CONTEXT.md:316`: "a running batch refreshes the library at most once a second". | `found`. Not the time-left hold (O6). The strip text is read every frame from `snapshot()`. |

How the plan uses the map. The only owner that changes is O8, the speed, and it changes inside its own home, `TimeLeftClock`. O2, O3, O4, O5, O6, O7, O9, O10 and O12 are used as they are. O11 needs nothing. The two `split` rows are handled in T1.

## Questions

**1. Which rule should the batch strip's time left use?** This replaces D283's rule, so it is the user's call. All three options keep these parts as they are today: the figure shows only once 2W rows have finished (16 on this machine), it changes at most once a second, paused time never counts, and the words stay "about m:ss left".

The examples use real code values: 8 workers, save groups of up to 16, and the screenshot's batch of 19,698 rows. Two cadences are assumed for the examples and are not measured. The fast one is this machine's: 16 rows land every 10 ms, which is 1,600 rows a second, close to the screenshot's 12,667 rows in 8 s; at the 8 s mark, with 7,031 rows left, nothing lands for 0.3 s. The slow one is a machine or setting where each chart takes 4 s, so 8 workers land 2 rows a second one at a time; with 1,000 rows left, all 8 workers hit long charts and nothing lands for 20 s. The true time left is about 4.4 s in the fast case and about 8:20 in the slow one.

- **A. Keep D283 as it is.** The speed is the last 16 rows over the time since the 16th-last. Fast case: the strip reads about 0:04 left, and a reading that falls at the end of the 0.3 s gap reads about 2:12 left for a full second (7,031 rows times 0.3 s over 16 is 132 s). Slow case: about 8:20 left, climbing to about 28:39 by the end of the 20 s lull (1,000 rows times 27.5 s over 16). Nothing is built.
- **B. The last 15 seconds, or the last 2W rows when fewer than 2W landed in them (Recommended).** The speed is the rows that landed in the last 15 s of batch time, over 15 s. A batch younger than 15 s uses the whole run so far. If fewer than 2W rows landed in the last 15 s, the window reaches back to the 2W-th last row, which is today's rule, so a slow batch never loses its figure. Fast case: about 0:04 left before the gap and about 0:04 or 0:05 after it (12,667 rows in 8.3 s is 1,526 a second, so 4.6 s). Slow case: about 8:20 left; during the 20 s lull it climbs as rows drop out of the 15 s, and from the point fewer than 16 are left in it the figure is today's, reaching about 28:39. So B cures the fast case and matches today in a long lull on a slow batch. The 15 s is a new number and is part of this question. It is indicatif's default (see Prior art). The other value offered is 5 s: it follows a real change of speed three times sooner, and a 0.3 s gap then moves the fast case from 4.4 s to 4.7 s instead of to 4.6 s. Say "B, 15" or "B, 5".
- **C. The average since the batch started.** The speed is rows done over batch time so far. This is the rule the strip had before D283; the session card records the D283 pick as made "over keeping today's average since the start". Fast case: about 0:04 left before and after the gap, the same as B, because the batch is younger than 15 s. Slow case: about 8:20 left, and after the 20 s lull about 8:37 if the batch had run ten minutes (1,200 rows in 620 s). C never spikes and adds no number. Its cost shows on a long batch whose speed really changes: after an hour at one speed, a drop to half speed takes many minutes to show.

What the user will see, by screen (agent rule 11). The values come from the worked cadences above, not from a recorded run; T6 is the real-library look if question 2 allows it.

| Screen | Today | After B or C |
|---|---|---|
| Batch strip while analyzing, fast case, reading taken right after a burst | "about 0:00 left" to "about 0:04 left", depending on the instant | "about 0:04 left" |
| Batch strip, fast case, reading taken at the end of a 0.3 s gap | "about 2:12 left" for one second | "about 0:04 left" or "about 0:05 left" |
| Batch strip before 2W rows have finished, while paused, when finished | no time left | no time left (unchanged) |
| Song Preview loading bar | its own last-second rule and 3 s gate | unchanged |
| Startup screen | no time left | unchanged |
| `hydra_batch` | prints no time left | unchanged |

Recommendation: B with 15 s. The strongest case against it: C is simpler, adds no number, and gives the same answer on every batch this machine runs, since they end inside 15 s. B earns its extra rule only on batches that run for minutes, and nobody has measured one. B also still climbs during a long lull on a slow batch, exactly as today.

**2. May the main session run one batch over the whole library in the real app, on a scratch copy of the library file, to watch the strip after the fix is built?** This is a permission question (agent rule 8). The fix's tests force the burst pattern with a fake pace, so they prove the rule. They cannot show the real pace. A yes also settles the two open facts, P16 and P17: why the app's batch is about four times slower than `hydra_batch`'s 2.8 s, and what pauses results for 0.3 s. A no means the fix merges on its forced tests and those two facts stay unknown; nothing else waits on them.

## Tasks

T1, T2 and T3 share files, so they are one branch (`claude/eta-fix`), one executor on Opus, three commits in this order. T4 and T5 are documents and run in parallel with it once question 1 is answered. If the answer is A, only T5 runs, and it records that D283 stands.

### T1: one count of rows done, and one wording owner (Opus; after question 1's answer)

Owner rows: O1, O9.

Files: `src/app/time_left.h`, `src/ui/library_jobs.cpp`, `src/ui/preview_load_job.cpp`, `tests/single_owner/rules/ui.json`, `tests/test_library_jobs.cpp`, `tests/test_preview_load_progress.cpp`.

O1 is split because `TimeLeftClock` keeps its own "most work done" (`done_`) beside `run_batch`'s count. The merge: `read` takes the count from its caller, so "rows left" is the total minus the count the strip copied from `run_batch` (`snap_.completed`). The clock keeps earlier readings only as its window's samples, never as how far the batch has got. `update` passes its `done` through, so the Preview's call does not change.

O9 needs no merge. `time_left_text` is the one place the words are made. `time_left_line` adds the Preview's age gate and then calls `time_left_text`. The strip has no age gate (O7), so it calls `time_left_text` directly. That is one owner with two callers, not two owners. T1 changes nothing for O9.

Acceptance criteria:
- `TimeLeftClock` has no member that holds the job's progress apart from its window samples. The single-owner rule at `tests/single_owner/rules/ui.json` (the "How long does a job have left" entry) names the new owner line, and its reader example is the new `read` call. Check: `hydra_tests.exe "-tc=single owner*"` passes; the reviewer reads the class.
- The Preview loader's figures are unchanged. Check: "ByteRateClock measures over a second and answers at most once a second" and "Preview load progress: time left waits 3 s, then reads m:ss" in `tests/test_preview_load_progress.cpp` pass with their expected values untouched. A diff that edits an expected value in either test fails this criterion.

### T2: the new speed rule (Opus; same branch, after T1)

Owner rows: O8 (changed), O3, O5, O6, O7 (used as they are).

Files: `src/app/time_left.h`, `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `tests/test_library_jobs.cpp`.

Written for answer B. The batch's mode of `TimeLeftClock` keeps its readings as (time, rows done) samples on the batch clock. The speed is the rows that landed in the last N seconds over N. While the batch is younger than N it is rows done over batch time. When fewer than 2W rows landed in the last N seconds, the window starts at the 2W-th last row, as today. N is one named constant beside `kTimeLeftStepSeconds`, with the user's answer and its reason in the comment. The 2W gate (O7), the one-second hold (O6), the batch clock (O5) and the "no rate, no figure" rule stay as they are. `over_last_second()`, the Preview's mode, is not edited.

For answer C the same task builds rows done over batch time instead, with no N, and the window tests below shrink to the steady, burst and gate cases.

Tests, all in `tests/test_library_jobs.cpp`, all calling `TimeLeftClock` with plain numbers so each expected value is one division a reader can do:
- Steady pace. Groups of 16 land once a second from 1 s to 20 s, total 1,600. At 16 rows a second, the 1,280 rows left take 80 s. The test reads three fresh clocks at 20.0 s, 20.05 s and 20.9 s and expects 80 s from each. Today's rule gives no answer, 4 s and 72 s for those three, so the test is red on today's code.
- A gap. The same feed, then nothing until 21.3 s: the figure stays within the change that one missing group makes to a 15 s window, stated as "320 rows done, 224 landed in the last 15 s". The executor writes the expected value as that division, not as a tolerance.
- Slow batch. Fewer than 2W rows in the last N seconds: the window starts at the 2W-th last row. The existing case "jobs: time left is the rate over the last 2W finished charts" is rewritten into this one, because its feed (8 rows by 1 s, 4 at 11 s, 4 at 21 s, read at 25 s) now has 8 rows inside the last 15 s and so no longer falls back.
- Young batch. Before N seconds have passed, the figure is rows left over (rows done over batch time).
- Kept as they are: "jobs: time left shows nothing until 2W charts have finished, then shows" (its expected value is rewritten for the new speed, its gate checks stay) and "jobs: the batch strip's time left waits for two rounds of its workers".
- "jobs: time left leaves a pause out of its window" changes its last check. It now asserts the figure is under the pause length, after requiring that all run time before and after the pause is under the pause length. The old check, `*eta <= run_s`, compared against run time since the resume only; a rule that averages the whole young run also counts the run time before the pause, which is right, and would fail that old line.

Acceptance criteria:
- Each test above passes, and the steady-pace test fails on main's `time_left.h`. The reviewer reads it against main's rule and says so.
- No number in `time_left.h` or `library_jobs.h` is new except N, and N's comment quotes the user's answer to question 1. Check: `hydra_precheck.exe` on the branch, and the reviewer's read.

### T3: the batch driven the way it really runs (Opus; same branch, after T2)

Owner rows: O2, O3, O4 (used as they are, through the real `run_batch`).

Files: `tests/test_library_jobs.cpp`.

One test runs a real `BatchJob` on a real `RecordStore`, with `set_analyzer_for_test(..., /*workers=*/8)` and at least 96 rows, so six full save groups. It makes real groups of 16 by holding the batch thread with `set_lookup_hold_for_test`: while that thread is held, the 8 workers fill the pool's waiting line of 16 (`kWorkPoolWaitingPerWorker` times 8), so the next group fills without the line going empty. The test holds for a little over `kTimeLeftStepSeconds` at each group, so each of its readings is a fresh one. It proves the groups are full by checking the snapshot's `completed` at each hold. P14 and P15 are the two facts this test design rests on, and the executor checks them first; if either fails, it stops and reports.

The test states two behaviours and uses no tolerance number:
- Right after a group lands, with at least two groups still to come, the figure is at least one gap long. Two more groups cannot arrive in less than one gap. Today's rule reads close to zero there.
- Across one gap in which nothing lands, in the second half of the batch, the figure rises by less than the gap itself. Today's rule rises by rows left over 16 gaps, which is at least two gaps.

Acceptance criteria:
- The test passes on the branch. The brief asks for no red run, so the reviewer reads the test against main's rule and writes down which check fails there and why.
- The test takes under 15 s of wall time (six gaps of about 1.1 s plus the corpus analysis). If it cannot, the executor stops and reports; it does not shorten the gap below `kTimeLeftStepSeconds`.

### T4: the words that describe the rule (docs; Sonnet writer; after question 1's answer; parallel with T1 to T3)

Files: `CONTEXT.md`.

The "Batch time left" entry is rewritten to say the new rule, quoting the user's answer to question 1 as the authority, and naming the new D-number from T5. Only the user changes what a `CONTEXT.md` term means; the answer to question 1 is that change, and T4 writes nothing the answer does not say.

The source comments that describe the rule are owned by T1 and T2, because they sit in those tasks' files: `src/app/time_left.h` lines 1 to 33, `src/ui/library_jobs.h` lines 271 to 274, 321 to 324 and 408 to 411, and `src/ui/library_jobs.cpp` lines 369 and 568 to 569. Each must describe the code as it is after the change and name no old value.

`docs/UserGuide.md:418` stays as it is under B and C: it says when the estimate appears, and that gate does not change (P12). `docs/adr/0008-preview-is-a-faithful-port-of-onyx.md` stays as it is: it describes the Preview only (P13). `docs/releases/release-3.0.0-notes.md:47` is history and is not rewritten.

Acceptance criteria:
- `CONTEXT.md`'s entry names the same rule the code has. Check: `hydra_tests.exe "-tc=docs match code*"` passes, and the reviewer reads the entry against `time_left.h`.
- No comment in the five places above still says "the last `units` units", "D283" as the window's source, or "2W" as the window. Check: the reviewer greps the branch for those three strings in `src/`.

### T5: record the decision (docs; main session; only after the user has answered question 1)

Files: `docs/audit/2026-10-03-fix-decisions.md`.

A new entry under the next free D-number. It says plainly that it replaces D283's window rule, and why: D283 was picked from one 2.8 s run that could not show the fault, and the user's batch read "about 2:19 left" with about 5 s to go. It quotes the user's answer to question 1 exactly, with the option text they chose. It says what stays from D283 and D276 item 8: the 2W gate, the one-second hold, paused time left out, and the Preview's own rule. It ends with the stamp line every entry carries: nothing stored changes, so no stamp moves. If the answer is A, the entry records that D283 stands and that the user saw this plan's examples.

Acceptance criterion: the entry exists, quotes the answer word for word, and names D283 as replaced or kept. Check: the plan's reviewer at merge reads it against the question box's answer.

### T6: the real-library look (main session; only on a yes to question 2, with the user's `allow library run`)

Rests on P16 and P17. Files: none in the repo.

The main session runs one batch over the library in the real dev app on a scratch copy of the library file, as "Seeing it in the real app" on `docs/agents/ui-testing.md` sets up, and notes the strip's time left once a second beside the rows done and the time elapsed. One run, capped at 5 minutes; if the batch has not finished by then it is stopped and the notes so far are reported. The report gives the user the table and says whether any reading was more than the batch's real remaining time by a minute or more.

Acceptance criterion: the table is shown to the user. This task proves no promise; it is the user's own look.

## Which GUI check this needs

Case 1 of `docs/agents/ui-testing.md`, "Behaviour and text". The change is a value the strip prints in a place and in words that do not move, so there is no layout to see and case 2 is not owed. The value is checked in `hydra_tests` (T2 and T3), because the figure depends on real time and a headless UI test cannot pace a batch. No `hydra_uitest` test pins the time-left text today, and this plan adds none. T6 is extra, and only on the user's yes.

## Builds

One build. The main session builds `hydra_tests` once on `claude/eta-fix` after T3 and runs "jobs: *time left*", the T3 test, "ByteRateClock*", "Preview load progress*", "single owner*" and "docs match code*". T4 and T5 are documents and need none. A second build happens only if the first fails and a fresh executor fixes it. T6 uses that same build.

## Out of scope (needs the user's yes to leave out)

- A line in the next release's notes saying the time left no longer spikes. It is written when that release is cut, by `docs/agents/public-doc-style.md`.
- Finding and fixing whatever pauses results for 0.3 s (P17). The fix makes the figure steady through such a pause; it does not remove the pause.
- Making the app's batch as fast as `hydra_batch` (P16).
- Any change to the Preview loader's time left.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| The figure no longer collapses right after a group lands | T3, first behaviour; T2 steady-pace test | T2, T3 | T2: three clocks at 20.0, 20.05 and 20.9 s all read 80 s. T3: figure at least one gap right after a landing. Both fail on main's rule. |
| The figure no longer climbs before the next group lands | T3, second behaviour | T3 | Across one gap the figure rises by less than the gap; main's rule rises by at least two gaps. |
| A short pause in results no longer multiplies into minutes | T2, "A gap" | T2 | The gap test's expected value is the 15 s window's division; main's rule gives rows left times the gap over 16. |
| A slow batch never loses its figure (B) | T2, "Slow batch" | T2 | Fewer than 2W rows in N seconds: the figure is the 2W-row rate, not empty. |
| The figure still waits for 2W finished rows | T2, kept tests | T2 | "shows nothing until 2W charts have finished" and "waits for two rounds of its workers" pass. |
| The figure still changes at most once a second | "The plain version"; T2 | T2 | The hold check inside the rewritten slow-batch case (a second reading inside the same second returns the first) stays. |
| Paused time never counts | T2, pause test | T2 | "time left leaves a pause out of its window", with its last check restated. |
| The Preview loader's behaviour is unchanged | T1 criterion 2 | T1 | Both Preview tests pass with expected values untouched. |
| One count of rows done (O1 merged) | T1 | T1 | Single-owner scan passes with the new owner line; reviewer reads the class. |
| O9 needs no merge | T1 | T1 | The reasons are in T1; the reviewer opens `time_left_line` and `time_left_text`. |
| No new number except the one the user chose | T2 criterion 2 | T2 | `hydra_precheck.exe`, and the constant's comment quotes the answer. |
| The driven test uses 8 workers and real groups of 16 | T3 | T3 | `completed` checked at each hold; `set_analyzer_for_test(..., 8)`. |
| The driven test never shortens its gap below the hold | T3 criterion 2 | T3 | Reviewer reads the gap constant; the executor stops if 15 s cannot be met. |
| `CONTEXT.md` and the comments say the new rule | T4 | T4, T1, T2 | Docs check passes; reviewer greps `src/` for the three old strings. |
| The decision is recorded, and says it replaces D283 | T5 | T5 | The entry quotes the answer and names D283. |
| Nothing is built before the user answers | header; each task's "after question 1's answer" | all | The main session dispatches no task before the answer (agent rule 3). |
| The library run happens only on the user's yes, once, capped at 5 minutes | T6 | T6 | The whole-library guard hook, and T6's cap line. |
| One build | Builds | main session | The build count in the merge report. |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The batch's speed is the window size over the time from the oldest window entry to now, and the figure is rows left over that speed. | code | `src/app/time_left.h:57-77` | checked 2026-11-20 |
| P2: Each newly finished row is stamped with the time of the reading that reported it, and only the last `units_` stamps are kept. | code | `src/app/time_left.h:37-52` | checked 2026-11-20 |
| P3: The batch's window is 2 times its worker count, and the worker count is at most 8 and one less than the machine's threads. | code | `src/ui/library_jobs.h:271-274`, `src/ui/library_jobs.cpp:369-381`, `src/app/analysis.cpp:707-716` | checked 2026-11-20 |
| P4: The strip stamps a row inside `on_progress`, on the batch clock, and reads the figure in `snapshot()` unless paused or finished. | code | `src/ui/library_jobs.cpp:484-490`, `src/ui/library_jobs.cpp:561-571` | checked 2026-11-20 |
| P5: `run_batch` reports a group's rows one by one only after the group commits; a group closes at 16, when a save lost it, or when the pool's idle call runs. | code | `src/app/analysis.cpp:852-890`, `src/app/analysis.cpp:915-946`, `src/store/record_store.h:398-400` | checked 2026-11-20 |
| P6: The pool runs the idle call whenever the consumer finds no result waiting, and lets at most 2 results per worker wait. | code | `src/app/work_pool.h:29-29`, `src/app/work_pool.h:58-60`, `src/app/work_pool.h:80-125` | checked 2026-11-20 |
| P7: The figure changes at most once in `kTimeLeftStepSeconds`, 1.0 s. | code | `src/app/time_left.h:17-20`, `src/app/time_left.h:58-58` | checked 2026-11-20 |
| P8: The strip prints the figure through `time_left_text` after the elapsed time, and the Preview prints its own through `time_left_line`. | code | `src/ui/library_dialogs.cpp:457-458`, `src/app/display_format.h:59-63`, `src/app/time_left.h:97-103`, `src/ui/preview_load_job.h:70-84` | checked 2026-11-20 |
| P9: D283 set the 2W rule from one run of 19,499 charts in 2.8 s where every candidate read "about 0:00 left" or "about 0:01 left"; D276 item 8 set the one-second hold; D278 item 1 allowed that one run. | code | `docs/audit/2026-10-03-fix-decisions.md:1399-1399`, `docs/audit/2026-10-03-fix-decisions.md:1418-1418`, `docs/audit/2026-10-03-fix-decisions.md:1437-1437` | checked 2026-11-20 |
| P10: The user's pick for D283 was "E: last 2W charts". | ruling | `docs/sessions/2026-10-31-e3dd3907.md`, the Rulings line stamped 14:52 UTC | checked 2026-11-20 |
| P11: `hydra_batch` runs the same `run_batch` with `batch_worker_count()` and the default analyzer, and prints its `[i/N]` lines from `run_batch`'s callbacks; it does no per-row store lookup. | code | `src/cli/batch.cpp:183-216` | checked 2026-11-20 |
| P12: The User Guide says only when the estimate appears (after twice the worker count), not how the speed is worked out. | code | `docs/UserGuide.md:418-418` | checked 2026-11-20 |
| P13: ADR 0008 describes the Preview's time left only. | code | `docs/adr/0008-preview-is-a-faithful-port-of-onyx.md:37-40`, `docs/adr/0008-preview-is-a-faithful-port-of-onyx.md:64-70` | checked 2026-11-20 |
| P14: Holding the batch thread in `set_lookup_hold_for_test` makes the next save group a full 16 when 8 workers run. | unchecked | Worked out from `src/app/work_pool.h` and `src/ui/library_jobs.cpp`; never run | T3 (the executor checks `completed` at each hold first, and stops if groups are not full) |
| P15: The test corpus holds at least 96 charts that analyze, enough for six groups. | unchecked | Reported as 97 by the removed handoff `docs/handoffs/2026-10-30-d271-eta-window-measurements.md` (`git show 057689de:docs/handoffs/2026-10-30-d271-eta-window-measurements.md`), and `docs/agents/test-rubric.md` mentions a 98th | T3 (the executor counts `corpus_rows` before writing the test) |
| P16: Why the app's batch (12,667 rows in 8 s) runs about four times slower than `hydra_batch` (19,499 in 2.8 s). The app adds a store lookup per reported row on the batch thread; whether that is the whole difference is not known. | unchecked | `src/ui/library_jobs.cpp` lines 504 to 530 show the lookup; no timing exists | question 2 |
| P17: What makes results pause for about 0.3 s in the user's batch. | unchecked | No source; the 0.3 s is worked back from the screenshot's own numbers | question 2 |
| P18: The screenshot read "about 2:19 left", "12,667 of 19,698" and "0:08 elapsed". | ruling | The user's screenshot as the main session quotes it in this plan's brief; the main session confirms the quote in the reviewer's brief, per `docs/agents/premise-map.md` | checked 2026-11-20 |
| P19: The existing tests that pin today's rule are the five cases at lines 287 to 393 of the jobs test file, and the single-owner entry that names the owner line. | code | `tests/test_library_jobs.cpp:284-393`, `tests/single_owner/rules/ui.json:1479-1503` | checked 2026-11-20 |
| P20: The Preview's two tests pin its figures with plain expected values. | code | `tests/test_preview_load_progress.cpp:237-263` | checked 2026-11-20 |
| P21: `CONTEXT.md`'s "Batch time left" entry states the 2W window, the gate, the hold and the pause rule. | code | `CONTEXT.md:274-283` | checked 2026-11-20 |
| P22: Case 1 of the GUI checks covers behaviour and text; case 2 is for layout, width, spacing and alignment. | code | `docs/agents/ui-testing.md:5-13` | checked 2026-11-20 |
| P23: The library table's once-a-second refresh reads the batch's tray and never the store, so it is not what pauses the batch thread. | code | `src/ui/app_state.cpp:256-282` | checked 2026-11-20 |
