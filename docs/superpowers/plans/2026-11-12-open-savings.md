# Every saving in the library open

## The plain version

Opening the library takes 250.9 ms today on a copy of your library. That number, and every other timing here, is a median of 7 warm runs of Hydra's own C++ (`hydra_bench --open`), run by the main session on 2026-11-12. The full output is `open_bench.txt`, named in the Premise map (P1). No number in this plan comes from Python or a SQL shell. Where Hydra's C++ has not timed something yet, the plan says "not measured" and names the task that measures it.

Three savings are already measured. The counted bar costs 66.7 ms more than the same read without it. The chart list's `ORDER BY name` costs 33.4 ms that the library's own sort then repeats. Reading the chart list on threads would save a further 4.4 ms (2 threads) or 7.3 ms (4 threads).

One more is large but not split yet. `set_charts` spends 47.1 ms folding text (24.6 ms for the search text plus 22.5 ms for the sort keys). The code shows it folds nearly the same text twice. How much each fix saves is not measured, so a task measures each before anything is built.

Two questions are yours at the end of this plan: threads (question 1) and which hidden copy of a chart opens (question 2).

## Every saving, in one table

"Available" is the measured cost today, read from `open_bench.txt`. "Saving" is what a fix would give back, and it is a number only where Hydra's C++ has timed the faster way. Rounded medians can differ by 0.1 ms from the differences the bench prints; the plan quotes the bench's own figures.

| What | Available today | Saving | Task |
|---|---|---|---|
| The counted bar: results read in 500-chart slices | 66.7 ms (the bench's difference of 250.9 counted and 184.1 uncounted) | not measured; T2 times the fix's prototype, T3 builds it | T2, T3 |
| Chart list read with no `ORDER BY name` | 46.6 ms sorted, on the bench's own connection | 33.4 ms (46.6 minus 13.2) | T4 |
| Chart list on 2 threads, after T4 | 13.2 ms unsorted on one thread | 4.4 ms (13.2 minus 8.8) | T7, only on your yes (question 1) |
| Chart list on 4 threads, after T4 | 13.2 ms | 7.3 ms (13.2 minus 5.9) | T7, only on your yes (question 1) |
| Sort keys reuse the search text | 22.5 ms (sort keys pass) | not measured; T2 times a prototype | T2, T5 |
| Fold each field without copying it first | part of 22.5 ms | not measured; T5 times it by building it | T5 |
| A faster path for plain ASCII text in the fold | part of 47.1 ms (24.6 plus 22.5) | not measured; T5 times it by building it | T5 |
| Fold rows on 2 or 4 threads | 47.1 ms | not measured; T2 times a prototype | T2, T6 only on your yes (question 1) |
| Store the folded text in the database at scan time | 47.1 ms | not measured; no task builds it | Out of scope |
| The results statement compiled again for each slice | part of 66.7 ms | not measured; T2 times it | T2 |
| A cold start (first open after a reboot) | never timed | not measured; T8 tries to time it | T8 |
| Read only the columns `set_charts` keeps | 13.2 ms | 0.9 ms (13.2 minus 12.3) | none: too small |
| Read only the winning profile's path rows | 42.4 ms | 0.3 ms (42.4 minus 42.1) | none: nothing to save |
| Opening the store | 2.0 ms | nothing planned | none |
| The chart count the bar needs | 0.6 ms | nothing planned | none |
| Shown names, the sort and the filter | 4.7 ms and 3.0 ms | nothing planned | none |
| The rest of `lookup` (75.5 ms minus its 42.4 ms path read) | 33.1 ms, not split into parts | not measured | Out of scope |

## Why the slices cost more (found in the code, not yet measured)

The results step costs 141.2 ms in slices and 75.5 ms as one call. Both read the same rows. The difference is the order in which they touch the database's pages.

SQLite keeps recently read pages of the file in a small memory area (its page cache). Hydra never sets its size, so it is SQLite's default of 2,000 KiB, about 2 MB (P7, P8). The library copy is 23.2 MB on disk (P9), so the cache holds under a tenth of it.

One call sorts every chart's hash and asks for them in that order (`lookup` sorts `distinct`, P5). The results tables are keyed by hash (P6). So one call walks each table once, front to back, and reads each page once.

The counted read slices the rows in the order `set_charts` was given them, which is the chart list's stored order (P4, P10): today the store's `ORDER BY name`, and after T4 the table's own order. Neither has anything to do with hashes. The 500 charts of one slice have hashes scattered across the whole table. `lookup` sorts them, but the slice still reaches from one end of each table to the other. Forty slices mean forty sweeps over both tables. Each sweep pushes the last sweep's pages out of the 2 MB cache, so most pages are read from Windows again on every slice.

That is the mechanism the code supports. It is `unchecked` (P11) until T2 times it. The fix, if T2 confirms it, is small: sort the rows by hash once, cut that sorted list into slices of 500, and put each answer back on its own row. Each slice then covers one stretch of each table, and all forty together read each page once, like the single call.

**Does that keep your ruling?** Yes. D336 item 3 says the bar moves once per slice, and item 4 sets the slice at "500 charts" (P3). The slices are still 500 charts each and the bar still moves once per slice, counting 500, 1,000 and so on up to the total. Only which 500 charts come first changes, and the screen never showed which. No words on the screen change. So this plan asks no question about the ruling.

Two other per-slice costs are in the code. T2 times one of them. First, the SQL text holds one `?` per hash, and Hydra keeps one compiled statement per distinct text (P12). The second read's count is the slice's Ready charts, which differs from slice to slice, so most slices compile a new statement. Second, each slice builds its own winner table and answer map; this is small work and is not timed separately.

`ready_profile_ids`, named in the brief as a suspect, is not called by `lookup` (P13). `ready_rules_fp` is called once per slice and only copies eight bytes (P13).

Prior art, searched 2026-11-12. SQLite's own page-cache default is 2,000 KiB per connection, and write-ups agree that a larger `cache_size` or an `mmap_size` helps read-heavy work only when the working set fits, with gains reported from under 3% to about 8%, so it must be measured (sources: sqlite.org's mmap page and its forum; dev.to and oneuptime.com tuning posts). A search for sorted-key batched `IN` lookups found one mailing-list benchmark where sorting the keys changed per-row lookups by about 2% and batching all the keys into one statement was the large win; it found no SQLite source that shows sorting helps. So the sorted-slice idea has no support or contradiction in prior art, which is why T2 measures it before T3 is built. The fold's ASCII fast path (T5 change 3) matches what GitHub's engineering blog reports for case folding code at memory speed: a branchless ASCII run, with the per-character table only for other text. Nothing found suggests another cheap way to fold search text.

If T2 shows hash-order slices are no faster, T3 does not start. The main session reports T2's numbers to you with the options left: a larger page cache (a new number, yours to pick) or progress reported from inside one `lookup` call (a change to how the slices are made, yours to rule on).

## What the user will see

Nothing changes on any screen. The startup screen keeps D336's text: "Opening your library..." with no bar while the file opens, then "Reading your charts..." over a bar, then "Reading your results..." over a bar, both counting out of the library's chart count (P3). T3 keeps the bar's steps of 500. T4 keeps the Library table's order for every sort column (its test proves it). T5 keeps every search and sort result (its tests prove it).

One thing is faster, so one thing is seen less often. The startup screen shows only when the open is still running after 0.15 s (D100 item 7 as D336 replaced it, P3). A faster open crosses that line less often. That is the existing rule working, not a new one.

Any visible change an executor finds it cannot avoid is a question to you before merge, never a line in a report.

## Global constraints

1. Every timing that goes to the user comes from `hydra_bench --open`, built and run by the main session. Agents time nothing. Checked by: each task's acceptance line is a `hydra_bench` line, and the reviewer of each task confirms no other timing is quoted.
2. The bench runs on the scratch copy of the library file, never the real one. It reads the database only and analyses no chart. If the main session reads agent rule 8 as covering it, it asks you first.
3. No screen text changes. Checked by T3's and T4's tests and by the existing startup-screen uitest, which the main session runs for T3 and T4.
4. Each fix is kept only if its bench line is lower after than before, same command, same copy. If not, the task's branch is not merged and the plan's table is updated to say so.

## The tasks

Order: T1 first, alone. Then T2 and T4 together (they own different files; T4 also waits for your answer to question 2). Then T3 and T5 together, once T2's numbers are in this plan and T4 has merged. Waiting for T4 is what stops T3 and T4 from both editing `library_jobs.cpp` at once: T3's branch is cut from `main` after T4's merge, so no hand merge is needed. If question 2 stays open, T3 does not wait for it: T3's branch is cut from `main` without T4, and the main session merges whichever of T3 and T4 comes second by hand (they edit different functions of one file). T6 and T7 run only on your yes to question 1. T8 can run beside T3 and T5 only if T2 has merged, because T2 and T8 both own `tools/bench.cpp`. T9 is last.

The bench's "parts" lines (section 1) copy `read_library`'s steps by hand until T9 updates them (P15), so after T3 or T4 the "total of the parts above" line may no longer match the real read. T3 and T4 do not check it; T9 does.

The bench command, used by every task below, is the one the main session ran today:

`build-cpp\Release\hydra_bench.exe --open <copy>\hydra.db --runs 7 --ini "C:\Program Files\Hydra\hydra_settings.ini" --as-ready`

### T1: Review and merge `hydra_bench --open`

Branch `claude/exec-open-bench`, tip ee02e710. A fresh reviewer (Sonnet) reads it once, fixes what it finds and signs off; then the main session merges it. Files in the change: `tools/bench.cpp`, `src/store/record_store.h`, `src/store/record_store.cpp`, `CMakeLists.txt` (P14).

The reviewer looks hardest at three things. First, the store header now shows seven helpers that were private to the store (`Stmt`, `prepare_read`, `step_row`, `library_entry_select`, `read_library_entries`, `path_row_columns`, `read_path_row`) and `ready_rules_fp` (P14); each must still have one definition and no second copy in the bench. Second, `--as-ready` rewrites profile rows, so the check that runs before the file is opened (`open_mode`, P15) must refuse the real library. Third, `hydra_bench` now links `hydra_ui`; it must open no window.

`hydra_precheck.exe`, run on this plan's branch on 2026-11-12, lists five items in `tools/bench.cpp`: two places that compile and step a statement themselves (lines 904 and 912), two that move a chart's path and folder (lines 1377 and 1378), and one bare number (line 1455). It also warns that the bench branch is 1,027 lines without this plan, over its 400-line mark. The reviewer settles each item, by calling the owner or by the scan's own way of recording an allowed use.

Acceptance criteria:

1. `hydra_bench` builds. Check: the main session builds the `hydra_bench` target on the reviewed branch.
2. The bench reproduces today's output shape. Check: the bench command above prints sections 1 to 6 and no line holds the word "differs".
3. The docs check passes. Check: `hydra_tests.exe "-tc=docs match code*"`, run by the main session.
4. The store's tests still pass, since store helpers moved. Check: the list `hydra_precheck.exe --tests` prints for the branch, run by the main session.
5. The precheck lists no item for `tools/bench.cpp`. Check: `hydra_precheck.exe` on the reviewed branch.

### T2: Time the candidates that have no number yet

Owned file: `tools/bench.cpp`. Depends on T1. An executor (Opus) adds lines to `--open`; the main session builds, runs the bench command and writes the numbers into this plan's table.

Every new line calls Hydra's public functions and checks its rows against today's rows on the first run, the way the existing prototypes do (the `differs` check). No line copies a rule: the fold is `app::fold_for_search`, the tag strip is `strip_rich_tags`, the lookup is `RecordStore::lookup`.

New lines under section 5:

- "results in 500-chart slices, stored order": today's loop, on its own store, over the hashes in the order `read_library` leaves them. This is the before figure.
- "results in 500-chart slices, hash order": the row indexes sorted by hash, cut into `ui::kResultsSliceCharts` slices, each sent to `lookup`, answers put back by index. This settles P11.
- "results in 500-chart slices, stored order, second pass on the same store": the same loop run again without reopening. Every statement is compiled by then, so the first pass minus this is the cost of compiling (P12's share).

New lines under section 6:

- "sort keys reusing the search text": for each of the five columns, copy the search text's folded field when the shown text equals the tag-stripped stored text, otherwise fold as today. Settles P17.
- "search text and sort keys on 2 threads" and "on 4 threads": the two passes over a share of the rows per thread.

Acceptance criteria:

1. Each new line prints a median and a minimum, and none says "differs". Check: the bench command's output.
2. The numbers are in this plan's table, replacing "not measured" for those rows. Check: this file's diff.
3. No new line holds a copy of a fold, strip or lookup rule. Check: the reviewer reads the diff.

### T3: The counted results read asks in hash order

Owned files: `src/ui/library_jobs.cpp` (only `read_library_summaries`), `src/ui/library_jobs.h` (its comment), `tests/test_library_jobs.cpp`. Depends on T2, and on T4 having merged unless question 2 is still open (see the order above). Uses P11: it starts only if T2's "hash order" median is lower than its "stored order" median. If it is not, stop and report, as the section above says.

`read_library_summaries`, when it has an `on_slice`, sorts the row indexes by hash, sends the store `kResultsSliceCharts` of them at a time, and writes each answer to its own row's place. It still tells `on_slice` the rows answered so far after each slice, and still hands the model all answers in one call. The read with no callback is untouched. `kResultsSliceCharts` stays 500.

Acceptance criteria:

1. Every row gets the same answer as the one-call read, when the rows' hash order differs from their row order and one hash sits on rows in two different slices. Check: the existing sliced-against-whole case in `tests/test_library_jobs.cpp`, extended with such a fixture at a slice size of 2. It fails if an answer lands on the wrong row.
2. The bar's counts are unchanged: one report per slice, each `slice_charts` more than the last, the final one equal to the row count. Check: a `CHECK` on the reported values in the same case. It fails if a slice is skipped or reported twice.
3. A false return from `on_slice` still stops the read. Check: the existing cancel case, unchanged and passing.
4. The counted read costs less. Check: the bench command; the line "the counted bar's cost (1's read_library minus 2's)" is below the 66.7 ms of P1, and "read_library as one call, counted" is below 250.9 ms. If T4 merged first, the main session first re-runs the bench on `main` and compares against that run's two lines instead, because T4 changes the order the slices start from. It writes both figures into this plan.
5. The startup screen's text is unchanged. Check: the existing held-open startup uitest, run by the main session.

### T4: Read the chart list unsorted at startup

Owned files: `src/store/record_store.h`, `src/store/record_store.cpp`, `src/ui/library_jobs.cpp` (only `read_library`), `src/ui/app_state.cpp` (only `select_chart`, if question 2 asks for it), `tests/test_library_model.cpp`, `tests/test_store_library.cpp`, `tests/test_app_reports.cpp` (the file that holds the `select_chart` cases). Depends on T1 and on your answer to question 2. T3 and T4 both edit `library_jobs.cpp`, in different functions; T4 merges first so T3 starts from it (see the order above).

`read_library` asks the store for the chart list in table order. The store's other two callers (`app_state.cpp:258` and `:905`, P18) keep the name order. `set_charts` already sorts the rows itself, by the chosen column, then title, then folder, then file path (P10).

The tie-break ends on the file path, so the shown order is fixed whenever no two rows share a path. On your library none do: the bench prints "0 rows with a path shared by another row" (P1). The `charts` table has no rule that forbids it (P19). Two rows with the same path, title and folder would tie, and their order would follow the read order; the task says so in its comment and does not claim more.

Four places in `app_state.cpp` walk the model's rows in stored order, not shown order. The reviewer of this plan read all four (P20, P25). Three find a row by its file path (`row_key`), which is one row each, so the read order cannot change their answer: `follow_selected_row` (line 205), `library_row_for` (line 214) and `library_matches` (line 256, whose answer follows the shown order). The fourth, `select_chart`'s second loop (line 646), picks the first copy of a chart in stored order when every copy is hidden by the search or the chip. Today stored order is name order; after T4 it is the table's own order. A chart's copies can carry different names: on the copy of your library, 601 charts have more than one copy and 47 of them have copies with different names (a count of rows, read from the scratch copy, not a timing). So which hidden copy opens could change for those. That is question 2, and T4 does not start until you answer it.

Acceptance criteria:

1. The shown order is the same for both read orders, for every sort column in both directions. Check: new case "library: the shown order does not depend on the order the rows were read" in `tests/test_library_model.cpp`, giving `set_charts` the corpus rows in name order and reversed. Failing-first is not possible here, because today's sort already has the tie-break; the reviewer says what would make it fail (dropping the path from the tie-break).
2. No two corpus rows share a file path. Check: a `CHECK` in the same case.
3. The unsorted read returns the same rows as the sorted one, and still reports each row and stops when told. Check: new case in `tests/test_store_library.cpp` comparing the two as sets, with the existing per-row and cancel cases run against the unsorted read too.
4. The other two callers still get name order. Check: the existing `test_store_library.cpp` order case, unchanged and passing.
5. `select_chart` follows your answer to question 2. Check: a new case in `tests/test_app_reports.cpp` with two copies of one chart in two folders, both hidden by a search, that selects the copy your answer names. If you answer "accept scan order", the case pins the scan-order copy. It fails if the loop picks the other copy.
6. The read is faster. Check: the bench command; "read_library as one call, no callback" is below 184.1 ms (P1). The main session writes it into this plan.

### T5: Fold each row's text once

Owned files: `src/ui/library_model.cpp`, `src/app/library_query.cpp`, `src/app/library_query.h`, `tests/test_library_query.cpp`, `tests/test_library_model.cpp`. Depends on T2 and, for `test_library_model.cpp`, on T4 having merged.

Today each row is folded ten times (P16). `make_searchable` strips the tags from five stored fields and folds each. Then the sort keys fold the five shown texts, and each shown text is the same stored field with its tags stripped and its ends trimmed. `library_cell_text` also returns a fresh copy of each text just to be folded.

Three changes, each its own commit, each kept only if its bench line drops:

1. The sort key copies the search text's folded field when the shown text equals the tag-stripped stored text, and folds as today otherwise. Built only if T2's "sort keys reusing the search text" is below 22.5 ms. Uses P17.
2. The sort-key pass reads each shown text in place instead of through a copy.
3. `fold_into` handles a run of plain ASCII bytes without the per-byte checks it needs for other characters, when no source map is asked for. The output is byte-for-byte the same.

Acceptance criteria:

1. Every row's search text and sort keys are byte-for-byte what they are today. Check: new case in `tests/test_library_model.cpp` that pins, as literals, the keys and search text of rows with rich tags, leading spaces, an empty title (shown as "(unknown)"), accents and full-width letters. The executor writes it first against today's code, so it passes before and after; it fails if a reused key differs from a folded one.
2. The fold's output is unchanged for every character it changes. Check: the existing fold-table cases in `tests/test_library_query.cpp`, unchanged and passing, plus one new case folding a mixed ASCII and non-ASCII string with whitespace runs against a pinned literal.
3. The traced fold (the one that maps matches back to shown text) is unchanged. Check: the existing highlight cases, unchanged and passing.
4. Each kept change is faster. Check: the bench command after each commit; "pass: search text (make_searchable)" against 24.6 ms, "pass: sort keys" against 22.5 ms, and "set_charts as one call" against 57.2 ms (P1). The main session writes each into this plan, and a change whose line did not drop is reverted by a fresh executor.

### T6: Fold rows on threads (only on a yes to question 1)

Owned files: `src/ui/library_model.cpp`, `tests/test_library_model.cpp`. Depends on T5 and on T2's thread lines.

`set_charts` builds its rows on the thread count you choose in question 1, each thread taking a share of the rows, then sorts and filters on the calling thread as today.

Acceptance criteria:

1. The rows are identical to the one-thread rows, in the same order. Check: T5's pinned case, run through the threaded path, and the T4 order case.
2. A library smaller than the thread count still loads. Check: new case with 0, 1 and 3 rows.
3. It is faster. Check: the bench command; "set_charts as one call" below T5's figure.

### T7: Read the chart list on threads (only on a yes to question 1)

Owned files: `src/store/record_store.h`, `src/store/record_store.cpp`, `tests/test_store_library.cpp`. Depends on T4.

What it costs. The store holds one connection behind one lock (P21). Threads need one read-only connection each, opened while the store's own connection is open: the bench found that first reads with no other connection open race to rebuild the WAL index and fail "database is locked" (P22). The rows are split by row id range and joined afterwards. The bar's per-row count has to be one shared counter, reported from one thread, and a stop has to reach every thread.

Acceptance criteria:

1. The threaded read returns the same rows as the one-thread read. Check: new case comparing them as sets.
2. The reported count reaches the row count exactly once and never goes down. Check: new case recording every report.
3. A false return stops every thread, and the store answers the next call. Check: new cancel case that stops at the first report and then calls `chart_library_count()`.
4. A thread whose connection fails to open fails the whole read with the store's plain error, and no partial list is returned. Check: new case with the open forced to fail on the second connection.
5. It is faster. Check: the bench command; "read_library as one call, no callback" below T4's figure.

### T8: Try to time a cold open

Owned file: `tools/bench.cpp`. Depends on T2 having merged.

A cold start has two parts. One is Windows loading Hydra's program files. `hydra_bench` is a different program, so it cannot time that; it is out of scope. The other is reading a library file that Windows has not cached. That part may be timeable without a reboot, and this task finds out.

The method is unproven (P23). Windows is reported to drop a file's cached pages when the file is opened with buffering off (`FILE_FLAG_NO_BUFFERING`) while nothing else has it open. The other known way, emptying the whole machine's standby list, needs administrator rights and slows everything else you are running, so the plan does not use it.

The executor adds `--cold`: before each timed open, with no store open, it opens and closes the copy once with buffering off, then times "open + read_library, no callback". The cap is one bench run of 7. If the cold median is not above the warm median, the trick did not empty the cache: the task reports that a cold open cannot be timed in C++ here without a reboot or administrator rights, the branch is not merged, and this plan says so.

Acceptance criteria:

1. `--cold` prints a cold line beside the warm line. Check: the bench command with `--cold`.
2. The plan states the outcome either way. Check: this file's diff, with the two medians.

### T9: The bench follows the merged read, and the plan leaves

Owned files: `tools/bench.cpp`, `docs/audit/2026-10-03-fix-decisions.md`, `docs/superpowers/plans/2026-11-12-open-savings.md`. Last.

Section 1 of the bench copies `read_library`'s steps by hand to time them one by one (P15). After T3 and T4 it must call what they call, so its parts still add up to the real read. The executor updates it, adds a D entry with the final before and after figures, and removes this plan in the same merge.

Acceptance criteria:

1. The parts match the real read. Check: the bench command; "total of the parts above" and "read_library as one call, counted" carry no "differs" line.
2. Nothing still points at this plan. Check: `hydra_tests.exe "-tc=docs match code*"`.

## Out of scope (for the user to approve)

Storing the folded text in the database at scan time. It could remove most of the 47.1 ms, but it adds columns, a stored-version stamp and a rescan whenever the fold changes, and the chart list would read more bytes. Not measured. Worth a plan of its own only if T5's figure is still large.

Splitting the other 33.1 ms of `lookup`. Timing its parts needs a timing hook inside the store. The code shows two candidates: the answers are copied, paths and all, into the result at the end, and the winner table is keyed by two strings. Not measured.

A larger SQLite page cache. It is a new number and a memory cost. It comes back as a question only if T3 cannot start.

The program-load part of a cold start (see T8).

## Questions

1. **Should the open use extra threads at all (T6 and T7)?** Today the whole read runs on one background thread, with one database connection behind one lock; that is the real rule (P21). At one end, T7 gives a measured 4.4 ms on 2 threads or 7.3 ms on 4, out of a read of about 184 ms, and costs extra connections, a shared counter for the bar and a new way for the open to fail. At the other end, T6 works on 47.1 ms, needs no database connection at all, and its saving is not measured until T2 runs. My recommendation: no to T7, because 4 to 7 ms does not pay for the new failure paths. Decide T6 after T2 prints its two thread lines; if you say yes then, the thread count (2 or 4) is yours, from those lines.

2. **When a chart has several copies and the search hides every one, which copy should it open?** The rule today (`select_chart`, `src/ui/app_state.cpp:646`): after the table's visible rows, take the first copy in the order the store handed the rows over, which is `ORDER BY name`. T4's faster read hands them over in the table's own order, the order the scan wrote them. The two orders differ only for charts whose copies carry different names: 47 charts in the copy of your library. Everything else is unchanged. Example from the first end: two copies, "Song" in folder A and "Song (Remaster)" in folder B, both hidden. Today the name order opens "Song". With the faster read, the scan order opens whichever copy the scan wrote first. Example from the other end: two copies with the same name open the same copy either way. The options: (a) accept scan order and pin it with a test (my recommendation: nothing promises a hidden copy's pick, and scan order is stable); (b) skip T4 and keep the 33.4 ms; (c) open the copy the table would show first with no search, a new small rule that needs the model to keep its unfiltered order. The strongest case against (a): the 47 charts' hidden pick changes without any word on screen saying so.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| Every timing shown to the user comes from `hydra_bench` | Global constraint 1 | every task | Each task's bench criterion; each reviewer confirms no other timing is quoted |
| The bench runs only on the scratch copy | Global constraint 2; T1 | T1 | The reviewer reads the refusal in `open_mode`; criterion 2's command names the copy |
| No screen text changes | "What the user will see"; Global constraint 3 | T3, T4 | T3 criterion 5 (startup uitest); T4 criterion 1 (shown order) |
| A fix is kept only if its bench line drops | Global constraint 4 | T3, T4, T5, T6, T7 | T3 criterion 4; T4 criterion 6; T5 criterion 4; T6 criterion 3; T7 criterion 5 |
| The bench builds and its checks pass before anything else uses it | T1 | T1 | T1 criteria 1 to 4 |
| Each exposed store helper has one definition | T1 | T1 | The reviewer reads the diff; T1 criterion 4 |
| The five precheck items in the bench are settled | T1 | T1 | T1 criterion 5 |
| New bench lines copy no rule and check their rows | T2 | T2 | T2 criteria 1 and 3 |
| T2's numbers replace "not measured" in the table | T2 | T2 | T2 criterion 2 |
| T3 starts only if hash-order slices measure lower | "Why the slices cost more"; T3 | T2, T3 | T2's two slice lines, written into the plan before T3's branch is cut |
| Slices stay 500 charts and the bar moves once per slice | "Does that keep your ruling?"; T3 | T3 | T3 criterion 2 |
| Each row gets its own answer after the reorder | T3 | T3 | T3 criterion 1 |
| A stop still stops the counted read | T3 | T3 | T3 criterion 3 |
| The counted read costs less than today | T3 | T3 | T3 criterion 4 |
| The Library's shown order does not depend on read order | T4 | T4 | T4 criteria 1 and 2 |
| The unsorted read returns the same rows, reports each, and stops when told | T4 | T4 | T4 criterion 3 |
| The other callers keep name order | T4 | T4 | T4 criterion 4 |
| Anything shown in stored-row order is a question, not a silent change | T4; question 2 | T4 | T4 criterion 5 |
| Search text and sort keys are byte-for-byte unchanged | T5 | T5 | T5 criteria 1 to 3 |
| A change whose line did not drop is reverted | T5 | T5 | T5 criterion 4 |
| Threaded folding gives identical rows, and small libraries load | T6 | T6 | T6 criteria 1 and 2 |
| The threaded chart list returns the same rows, counts once, stops everywhere and fails whole | T7 | T7 | T7 criteria 1 to 4 |
| T6 and T7 run only on the user's yes | The tasks' headings; question 1 | T6, T7 | The main session quotes the user's answer in each task's brief |
| A cold figure is reported only if the cache was really emptied | T8 | T8 | T8 criterion 2: cold above warm, or the plan says it cannot be timed |
| The bench's parts match the real read after the fixes | T9 | T9 | T9 criterion 1 |
| The plan leaves the repo with its last merge | T9 | T9 | T9 criterion 2 |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The bench printed the figures this plan quotes: 250.9 and 184.1 ms for the counted and uncounted read, 66.7 ms between them, 141.2 ms for the sliced results, 75.5 ms for one `lookup`, 46.6, 13.2, 12.3, 8.8 and 5.9 ms for the chart list variants, 42.4 and 42.1 ms for the path read, 57.2, 24.6, 22.5, 4.7 and 3.0 ms inside `set_charts`, 2.0 and 0.6 ms for the open and the count, 19,610 rows, and "0 rows with a path shared by another row" | code | `C:\Users\Patrick\AppData\Local\Temp\claude\b8c40ac9d5b73501\c8b70230-d47d-4a13-ba2e-f4c7048d9ab4\scratchpad\startup-bar\bench\open_bench.txt` lines 5 to 58 | checked 2026-11-12, the file read whole |
| P2: The bench's own-connection and thread reads ran beside an open store | code | `tools/bench.cpp:1285` to `:1298` | checked 2026-11-12 |
| P3: D336 rules the startup text, two counted steps out of `chart_library_count()`, and slices of "500 charts" held in `kResultsSliceCharts` | code | `docs/audit/2026-10-03-fix-decisions.md:1918` to `:1923` and `src/ui/library_jobs.h:100` | checked 2026-11-12 |
| P4: With a callback, `read_library_summaries` sends the store the model's hashes in stored row order, a slice at a time; with none it makes one call | code | `src/ui/library_jobs.cpp:93` to `:113` | checked 2026-11-12 |
| P5: `lookup` sorts and de-duplicates the hashes it is given, reads `records` then `record_paths` for them, up to 10,000 hashes per statement | code | `src/store/record_store.cpp:1264` to `:1365` and `:368` (the sort is line 1273 on this branch, 1263 on `main` until T1 merges) | checked 2026-11-12 |
| P6: `records` and `record_paths` are keyed by song id first and store their rows inside that key | code | `src/store/record_store.cpp:254` to `:277` | checked 2026-11-12, the key lines |
| P7: The store sets journal mode, synchronous and journal size, and no cache size or memory map | code | `src/store/record_store.cpp:801` to `:805`, and a search of `src/store` for `cache_size` and `mmap` that found nothing else | checked 2026-11-12 |
| P8: The vendored SQLite 3.46.0 defaults to a 2,000 KiB page cache, 4,096-byte pages and no memory map | code | `third_party/sqlite/sqlite3.c:13864`, `:13917` and `:15125` | checked 2026-11-12 |
| P9: The library copy is 23,216,128 bytes | code | `C:\Users\Patrick\AppData\Local\Temp\claude\b8c40ac9d5b73501\c8b70230-d47d-4a13-ba2e-f4c7048d9ab4\scratchpad\startup-bar\bench\hydra.db`, its size in a directory listing | checked 2026-11-12 |
| P10: `set_charts` keeps the rows in the order it is given and sorts an index by the chosen column, then title, folder and file path | code | `src/ui/library_model.cpp:159` to `:181` and `:246` to `:293` | checked 2026-11-12 |
| P11: The slices cost more because each one sweeps both result tables and the 2 MB cache cannot hold a sweep | unchecked | `src/store/record_store.cpp:1273` shows the per-call sort the reasoning rests on; nothing has timed it | T2 |
| P12: The store keeps one compiled statement per distinct SQL text, and `lookup`'s text holds one `?` per hash | code | `src/store/record_store.cpp:83` to `:91` and `:1293` to `:1351` | checked 2026-11-12 |
| P13: `lookup` does not call `ready_profile_ids`; it calls `ready_rules_fp`, which copies one 64-bit value | code | `src/store/record_store.cpp:1264` to `:1365` and `:401` to `:413` | checked 2026-11-12 |
| P14: The bench branch changes four files and adds seven helpers and `ready_rules_fp` to the store's header | code | `git diff main...claude/exec-open-bench`, read for `src/store/record_store.h` and `CMakeLists.txt` | checked 2026-11-12 |
| P15: `open_mode` runs a check before the file opens, and section 1 of the bench repeats `read_library`'s steps by hand | code | `tools/bench.cpp:934` to `:940` and `:1156` to `:1204` | checked 2026-11-12; the refusal (`refused_db`, `tools/bench.cpp:846` to `:861`) rejects a file in the installed Hydra's folder or the library beside the exe; T1's reviewer reads it too |
| P16: Each row is folded ten times: five in `make_searchable` after `strip_rich_tags`, five for the sort keys through `library_cell_text`, which returns a copy | code | `src/app/library_query.cpp:390` to `:400`, `src/ui/library_model.cpp:135` to `:176` and `src/parse/song.cpp:558` to `:596` | checked 2026-11-12 |
| P17: Reusing the search text for the sort key is faster than folding again | unchecked | `src/ui/library_model.cpp:175` is the fold it would skip; nothing has timed the reuse | T2 |
| P18: `list_chart_library` has three callers in `src` | code | `src/ui/library_jobs.cpp:69`, `src/ui/app_state.cpp:258` and `:905` | checked 2026-11-12 |
| P19: The `charts` table has no uniqueness rule on `path` | code | `src/store/record_store.cpp:807` to `:829` | checked 2026-11-12 |
| P20: Three of the four `app_state.cpp` row walks find a row by file path, and the fourth, `select_chart`'s second loop, picks a hidden copy by stored order | code | `src/ui/app_state.cpp:205`, `:214`, `:256` and `:646`, and `src/ui/library_model.h:83` (`row_key` is the notes path) | checked 2026-11-12, all four read |
| P26: On the copy of the library, 601 hashes sit in more than one `charts` row and 47 of those rows carry more than one distinct `name` | code | `C:\Users\Patrick\AppData\Local\Temp\claude\b8c40ac9d5b73501\c8b70230-d47d-4a13-ba2e-f4c7048d9ab4\scratchpad\startup-bar\bench\hydra.db`, a read-only count by `md5` | checked 2026-11-12 |
| P25: Today's stored order is the store's `ORDER BY name`, with ties in an order SQLite does not promise (the index `charts_by_name` would give rowid order) | unchecked | `src/store/record_store.cpp:1658` and `:826` | unchecked: the query and index were read, the plan SQLite picks for ties was not run; question 2 |
| P21: The store reads through one connection behind one lock | code | `src/store/record_store.cpp:1290` and `:1654`, and `src/store/record_store.h:674` | checked 2026-11-12 |
| P22: Parallel first reads with no other connection open fail "database is locked" | unchecked | `tools/bench.cpp:1285` to `:1289` is a comment that reports it; no run output was read | T7 |
| P23: Opening a file with buffering off makes Windows drop its cached pages | unchecked | `tools/bench.cpp` holds no such code today; the behaviour is reported, not documented | T8 |
