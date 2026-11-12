# The startup library read: measure it, then give it a counted bar

Date: 2026-11-12. Status: a plan, waiting for the user's answers under `## Questions`. No code changes until then.

## The plain version

The startup screen says "Opening your library..." and shows no bar while Hydra reads the chart list. The user asked why a line with no visible progress is good design, and whether several queries in parallel would be faster than one.

The short answers, from measurements on a copy of the user's own library file:

The database reads are already short. All the SQL the library read runs took about 215 ms in total on the copy (70 ms for the chart list, 144 ms for the saved results). Hydra shows the startup screen only once an open has run 0.15 s, so on this library the screen is up for a fraction of a second. The screenshot showed it for longer only because the test window held the read on purpose.

Parallel queries would not help. SQLite allows them, and four readers did cut SQLite's own part of the chart-list walk from 10.6 ms to 3.0 ms. That is 8 ms saved out of 215. The time is not in SQLite's scan. It is in the sort order the query asks for (37 ms), in reading 68,743 saved path rows (106 ms), and in Hydra's own code after the query, which nobody has timed yet.

So the plan has two halves. First it times Hydra's own code on the same copy (T1), because that part cannot be timed without a build. Then it gives the library read a counted bar (T2 to T4), so a slow open on any machine shows real progress. One cheap speed-up the measurements point to is offered as a question (T5), not decided here.

The "no bar" design was a ruling (D100 item 7). Changing it is the user's call, so every visible change is under `## Questions`.

## What was measured, and how

Every number below came from one script, run on 2026-11-12 on a copy of `C:\Program Files\Hydra\hydra.db` made with SQLite's backup call. The copy is 23,216,128 bytes. The script used Python's `sqlite3` module (SQLite 3.50.4; Hydra itself builds SQLite 3.46.0 from `third_party/sqlite`). Each figure is the median of 7 runs, each on a fresh read-only connection, with the file already in the operating system's cache (a "warm" read). The script and the copy are in the session's scratch folder, not in the repo.

The copy holds 19,610 chart rows (18,984 distinct charts), 19,066 result records, 68,743 saved path rows and 26 settings profiles. The 19,610 matches the Library header the user saw ("1 of 19,610 charts").

| What was timed | Median | Notes |
|---|---|---|
| The chart-list query exactly as Hydra runs it, rows handed to Python | 70.5 ms | `SELECT md5, name, artist, charter, album, path, folder, sig, stated_length_ms, delay_ms FROM charts ORDER BY name LIMIT -1 OFFSET 0` |
| The same walk kept inside SQLite, no rows handed out | 48.3 ms | SQLite's own share of the line above |
| The chart list with no `ORDER BY`, rows handed to Python | 33.0 ms | 37.5 ms less than the first line |
| The chart list with no `ORDER BY`, inside SQLite | 11.4 ms | so the sort order costs about 37 ms |
| The chart list with 7 columns instead of 10 | 61.5 ms | 9 ms less than the first line |
| `SELECT COUNT(*) FROM charts` | 1.15 ms | the bar's total |
| The saved-results read (Hydra's `lookup`), both parts | 143.9 ms | 28.6 ms for 18,984 records, 105.8 ms for 68,743 path rows |
| The walk split across 1, 2 and 4 connections by row range, inside SQLite | 10.6, 6.5, 3.0 ms | parallel readers |
| Opening a connection and reading the schema | 0.47 ms | |

SQLite walks the chart list through the `charts_by_name` index (`EXPLAIN QUERY PLAN` answered `SCAN charts USING INDEX charts_by_name`). An index walk fetches each row from a different place in the file, which is why the sort order costs more than the scan.

Three things these numbers do not say. They are Python's times, not Hydra's; Hydra's C++ turns rows into objects faster than Python does, so the SQL share in Hydra is likely lower. They are warm reads; a first start after a reboot reads the 23 MB file from disk. And they leave out everything Hydra does after the query: building each row's search text and sort keys, sorting 19,610 rows, and picking each chart's winning record. T1 measures those three.

## Is there a faster method than one query?

**Several queries in parallel: no.** SQLite does allow it. In WAL mode (the journal mode Hydra's file uses, where readers do not block each other), each thread needs its own connection, and each connection is single-threaded. The measurement shows it works: 10.6 ms down to 3.0 ms with four connections. But it speeds up only SQLite's scan, and the scan is about 11 ms of a 215 ms read. Hydra's store holds one connection behind one lock, so parallel readers would mean a second set of read-only connections and the code to merge their rows, all to save about 8 ms. Prior art agrees: SQLite forum threads and write-ups on WAL reader pools report that parallel readers help servers answer many small queries at once, and that splitting one bulk scan across threads gives little, because the cost is per-row work and I/O (sources at the end).

**Fewer columns up front: no.** Dropping the three columns the library table never keeps saved 9 ms.

**Dropping the sort order: maybe (T5, question 4).** The query sorts by name, then `LibraryModel::set_charts` sorts the rows again by the column the user has chosen. The first sort cost 37 ms of the 70. Whether 37 ms matters depends on T1's total.

**The saved path rows: a lead, not a task.** Reading 68,743 path rows is the largest single piece (106 ms), and `lookup` reads every profile's paths, then keeps only the winning profile's. Changing it touches which result a chart shows, so it is listed under "Out of scope" until T1 says the time is worth chasing.

**The recommendation.** One query per table is already near the floor for the SQL. If T1 shows Hydra's own code is the larger share, the speed work belongs there, and it is a new plan with its own measurements. This plan does not guess at it.

## What the user will see

The only screen that changes is the startup screen (`render_startup_screen`). It shows only when an open is still running after 0.15 s. The numbers below are the user's own library's.

Before, during the library read on a file that needs no upgrade:

```
Opening your library...
```

After, if the user takes the recommended answers (questions 1 to 3):

```
Reading your charts...
[=========>            ]  8,000 / 19,610
```

then

```
Reading your results...
[===============>      ]  14,000 / 19,610
```

While the file itself is being opened, before any row can be counted, the line stays "Opening your library..." with no bar. Nothing can be counted there: the wait is the disk.

On a file that is being upgraded, the third line reads "Finishing..." with the last step's bar left full today. After the change it reads the same two new lines with their own counts, under the same two heading lines. That is also a visible change, so it is part of question 1.

No other screen reads these counts. The Library header's total ("19,610 charts") comes from the same rows once they are loaded; the bar's total is `chart_library_count()` of the same table, so the two agree.

## Global constraints

No whole-library song scan anywhere in this plan. The timing task reads one database file, a copy.

Agents write code and tests; the main session builds and runs every test that needs a build (agent rule 14).

Every count on the bar comes from one place: the store's `chart_library_count()`. The bar is drawn by the existing `progress_bar_counted`. No second widget and no second count.

## The tasks

### T1: Time Hydra's own part of the open on the database copy

Owned file: `tests/test_library_jobs.cpp` (one new test case). No production code changes.

The test case is named "timing: the startup library read by part". It is skipped unless the variable `HYDRA_TIMING_DB` names a library file, so the ordinary suite never runs it. It opens that file with `RecordStore`, then calls, in the order `read_library` does, `list_chart_library(0, -1)`, `LibraryModel::set_charts`, `hashes()` with `lookup`, `set_summaries` and `sort_if_out_of_date`. It times each call with `steady_clock` and prints one line per part in milliseconds, plus the store's open and the total. It runs the sequence 5 times and prints the first run and the fastest run, because the first run is the nearest thing to a cold read.

The main session builds, copies the database with SQLite's backup into the scratch folder, sets `HYDRA_TIMING_DB` to the copy and runs `hydra_tests.exe "-tc=timing: the startup library read by part"`.

Acceptance criteria:

1. With `HYDRA_TIMING_DB` unset the case passes and prints nothing. Check: the main session's run of that one case, unset.
2. With it set, the output holds seven labelled lines (open, list, set_charts, lookup, set_summaries, sort, total) for run 1 and for the fastest run, and the row count it read equals `chart_library_count()`. Check: a `CHECK` in the case on the row count, and the printed lines.
3. The case calls the production functions and holds no copy of their SQL. Check: the reviewer reads the case.
4. The main session writes the numbers into this plan's table under "What was measured", each with its run and date, before T2 starts. Check: the plan's diff. This settles P9, P10 and P11.

### T2: The store reports rows as it reads the chart list

Owned files: `src/store/record_store.h`, `src/store/record_store.cpp`, `tests/test_store_library.cpp`.

`list_chart_library` gains an optional progress callback. `read_library_entries` calls it once per row with the rows read so far, the way the conversion reports once per row today. The callback returns false to stop the read, and a stopped read throws the store's existing cancel signal, so closing the window during a long read still ends it. Callers that pass no callback behave exactly as before.

Acceptance criteria:

1. A read of a store holding N charts calls the callback N times with 1, 2, ... N, and returns N rows. Check: new test "store: the chart list reports each row it reads", failing first (the parameter does not exist yet).
2. A callback that returns false at row 2 of 4 stops the read, and the store still answers a later read. Check: new test "store: a chart list read that is told to stop stops".
3. With no callback the rows and their order are unchanged. Check: the existing `tests/test_chart_index.cpp` and `tests/test_store_library.cpp` cases stay green.

### T3: The library read reports counted progress for both of its parts

Owned files: `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `tests/test_library_jobs.cpp`, `tests/test_app_library.cpp`. Depends on T2 and on the answers to questions 1 to 3.

`StoreOpenProgress::Step` gains two steps in place of `LoadingLibrary`: reading charts and reading results. `read_library` takes an optional progress callback. The job asks `chart_library_count()` once for the total and uses that one number for both steps. During the first step the count is the rows T2's callback reports. During the second, `read_library_summaries` asks `lookup` for the rows' results one slice at a time (the slice size is question 3's answer), reports the rows answered after each slice, then hands the joined answers to `set_summaries` in one call, so the table is filtered once as today. `StoreOpenJob::report` keeps publishing the counts before the step. `label()` returns the two new lines.

`AppState::reload_library` (the read after a scan) passes no callback and shows nothing new.

Acceptance criteria:

1. An open of a file that needs no upgrade reports `Opening`, then reading charts from 0 to N of N, then reading results from 0 to N of N, where N is `chart_library_count()`. Check: rewrite "jobs: the startup open reads saved paths with its own words, then the library" and "AppState opens the store on a worker" (`tests/test_app_library.cpp`, the case that pins the step list), failing first.
2. The total is the same number in both steps and equals the rows the model holds afterwards. Check: a `CHECK` in the same case comparing every report's total with `library.rows().size()` or its public equivalent.
3. The sliced results read gives every row the same answer as one whole read. Check: new test "jobs: results read in slices match one read", on the corpus store with a slice size of 1 and of the real size, comparing `set_summaries` outcomes row by row.
4. A chart listed in two folders gets its result in both rows when the two rows fall in different slices. Check: the same test, with slice size 1 and the corpus's duplicate (`chart("same", ...)` pattern in `tests/test_library_model.cpp`).
5. A cancel during either step ends the job as a cancel with no message. Check: extend "jobs: a stop while the startup open reads saved paths ends it as a cancel" with a stop at each new step.
6. A read that fails still leaves its sentence for the status line and the app opens. Check: "a library read that fails after the startup open is the status line's problem" stays green with its gate keyed to the first new step.
7. `label()` returns the exact lines the user chose. Check: rewrite "the startup screen's line names what the open is doing" with the chosen literals.

### T4: The startup screen draws the bar, and the hold gate holds once

Owned files: `src/ui/library_view.cpp`, `tests/ui/uitest_harness.cpp`, `tests/ui/uitest_library.cpp`, `docs/agents/ui-testing.md`, `docs/adr/0026-*.md` ("What the user sees"), `docs/audit/2026-10-03-fix-decisions.md` (a new D entry that replaces D100 item 7, in the user's words from the answers). Depends on T3 and on P12.

`render_startup_screen` already draws `progress_bar_counted` whenever `rows_total > 0`, so it needs no new drawing code; the task checks the box height still fits one text row plus the bar, and changes nothing else there unless it does not. The test gate `OpenGate` holds at every report of the library-read step today. With one report per row it would hold 19,610 times. The task makes it hold at the first report of the first library-read step only.

Acceptance criteria:

1. A held open on the test corpus shows the chosen first line and a bar reading "0 / N" in the startup box, N being the corpus's chart count. Check: new uitest "startup: the library read shows a counted bar", reading the box's text; failing first.
2. A `StartupDrawn` hold of S seconds delays the open by about S seconds once, not once per report. Check: the same uitest asserts the open finishes within the existing wait cap after one hold.
3. The Conversion hold still holds at the conversion. Check: the existing upgrade uitest (the one that checks `Step::ConvertingRecords`) stays green.
4. ADR 0026 and the new D entry say what the screen shows now, and `ui-testing.md` says what a held open shows. Check: `hydra_tests.exe "-tc=docs match code*"`.

### T5: Read the chart list unsorted at startup (only if question 4 is a yes)

Owned files: `src/store/record_store.h`, `src/store/record_store.cpp`, `src/ui/library_jobs.cpp`, `tests/test_library_model.cpp`. Depends on T1 and T3.

`read_library` asks the store for the chart list in table order instead of name order. The other callers of `list_chart_library` keep the name order. `set_charts` sorts the rows itself, with a tie-break that ends on the chart's file path, so the table the user sees does not change (P8, which this task's test settles).

Acceptance criteria:

1. The library's shown order is the same for both read orders, for every sort column in both directions. Check: new test "library: the shown order does not depend on the order the rows were read", feeding `set_charts` the corpus rows in name order and in reverse.
2. No two chart rows in the corpus store share a file path. Check: a `CHECK` in the same test; this is what makes the tie-break total.
3. T1's timing case, run again by the main session on the copy, shows the list part faster than before. Check: the two printed lines, written into this plan. If it is not faster, the task is dropped and the plan says so.

## The live steps

### LS1: Start the live window with the hold set

Run after T4 is merged and built. The main session sets `HYDRA_UITEST_HOLD_OPEN_S` to 5 and starts the live window on a snapshot copy of the user's library. The variable holds the open at the library read (P12). Pass: the start returns without an error. Cap: one start, 2 minutes. On failure, report the start's error text. This step settles P13.

Calls: `hydra_start` (never tried)

### LS2: Read the bar on the held startup screen

Starts only once LS1 has a passing run. Same start; while the open is held, read the screen's text. Pass: the text holds the chosen first line and "0 / 19,610", and the total equals the Library header's total read after the hold ends. Cap: 3 reads inside the 5 s hold; if none shows the startup box, report what the reads returned. This step settles P14.

Calls: `hydra_start` (seen: LS1's passing run), `hydra_text` (never tried)

## Out of scope (for the user to approve)

Reading fewer saved path rows in `lookup`. It is the largest SQL piece (106 ms measured), but it touches which result a row shows. It needs its own plan after T1.

Speeding up `set_charts` or the sort. T1 measures them; any change is a new plan.

Parallel reader connections. Measured at 8 ms saved; not worth new connection machinery.

A time-left line on the new bars. D100 item 4 gave the upgrade one; this plan adds none, because the read is too short for the 3 s wait that line uses.

## Questions

1. **Should the library read show a counted bar, replacing D100 item 7?** The rule today, in D100 item 7's words: "A file that needs no upgrade but takes over 0.15 s to open ... shows "Opening your library..." with no bar, for both the open and the library read." At one end, a library that opens in under 0.15 s shows no screen at all, before and after. At the other, a library ten times this size on a slow disk shows one unchanging line for seconds today, and would show a bar counting its charts. On a file being upgraded, the last line changes from "Finishing..." over a full bar to the new lines with their own counts. Recommended: yes.

2. **What are the words?** Recommended: "Opening your library..." while the file opens (unchanged, no bar), "Reading your charts..." while the chart list is read, and "Reading your results..." while saved results are read. The alternative is to keep "Opening your library..." over both bars, which changes fewer words but does not say why the bar starts again.

3. **One counted step or two, and how big a slice?** Measured, the chart list is 70 ms of the SQL and the results are 144 ms. With one counted step the bar fills during the first third and then sits full. With two, the results are read in slices and the bar moves once per slice. A slice size is a new number. Recommended: two steps, 1,000 charts per slice, which moves the bar 20 times on 19,610 charts and 1 time on a 500-chart library. Smaller slices mean more queries; T1's case can time the difference if the user wants a number first.

4. **Read the chart list unsorted at startup (T5)?** Measured saving: about 37 ms of 70 on this library, in Python's timing. Nothing visible changes if P8 holds. Recommended: decide after T1. Take it if the list is a real share of the total, skip it if Hydra's own code dwarfs it.

## Promise map

| Promise | Plan line | Task | The check that proves it |
|---|---|---|---|
| The timing case never runs in the ordinary suite | T1, criterion 1 | T1 | The one-case run with the variable unset passes and prints nothing |
| The timing case prints each part and reads every row | T1, criterion 2 | T1 | The `CHECK` on the row count and the seven printed lines |
| The timing case holds no copy of the SQL | T1, criterion 3 | T1 | The reviewer reads the case |
| T1's numbers go into this plan before T2 starts | T1, criterion 4 | T1 | The plan's diff |
| The store reports each row it reads | T2, criterion 1 | T2 | "store: the chart list reports each row it reads" |
| A read told to stop stops, and the store still works | T2, criterion 2; "closing the window during a long read still ends it" | T2, T3 | "store: a chart list read that is told to stop stops"; T3 criterion 5's cancel at each step |
| Callers with no callback are unchanged | T2, criterion 3; T3 `reload_library` line | T2 | Existing `test_chart_index.cpp` and `test_store_library.cpp` cases |
| The open reports both counted steps, 0 to N of N | T3, criterion 1 | T3 | The two rewritten step-list cases |
| The count is derived once: one total for both steps, equal to the rows loaded | Global constraints; T3, criterion 2 | T3 | The `CHECK` on every report's total |
| Sliced results equal one whole read, duplicates included | T3, criteria 3 and 4 | T3 | "jobs: results read in slices match one read" |
| The table is filtered once, as today | T3 body ("in one call") | T3 | The same test counts one `set_summaries` call's outcome; the reviewer reads the call site |
| A failed read is still the status line's problem | T3, criterion 6 | T3 | "a library read that fails after the startup open is the status line's problem" |
| The labels are the chosen words | T3, criterion 7 | T3 | "the startup screen's line names what the open is doing" |
| The bar is drawn by the existing `progress_bar_counted` | Global constraints; T4 body | T4 | "startup: the library read shows a counted bar" reads "0 / N"; the reviewer confirms no new widget |
| The hold gate holds once | T4, criterion 2 | T4 | The same uitest finishes inside the wait cap |
| The Conversion hold is unchanged | T4, criterion 3 | T4 | The existing upgrade uitest |
| The docs say what the screen shows now | T4, criterion 4 | T4 | `hydra_tests.exe "-tc=docs match code*"` |
| The shown order does not depend on the read order | T5, criteria 1 and 2 | T5 | "library: the shown order does not depend on the order the rows were read" |
| T5 is dropped if it is not faster | T5, criterion 3 | T5 | The two timing lines written into the plan |
| The bar's total agrees with the Library header | "What the user will see"; LS2 | LS2 | LS2's pass line compares the two |
| No whole-library song scan | Global constraints | T1 | T1 reads one file named by `HYDRA_TIMING_DB`; the reviewer confirms no task names a song folder |
| Each live step has a cap and a failure report | LS1, LS2 | LS1, LS2 | The caps written in each step |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: The startup screen draws a counted bar only when `rows_total > 0`, with `progress_bar_counted`. | code | `src/ui/library_view.cpp:134` and `:157` | checked 2026-11-12 |
| P2: The library read is one `list_chart_library(0, -1)` handed to `set_charts`, then one `lookup` of every hash, and it reports no count. | code | `src/ui/library_jobs.cpp:40` to `:61` and `:155` | checked 2026-11-12 |
| P3: The chart-list query is the ten-column select with `ORDER BY name LIMIT ? OFFSET ?`. | code | `src/store/record_store.cpp:1529`, `:1538` and `:1643` | checked 2026-11-12 |
| P4: `set_charts` builds search text and sort keys per row, then sorts and filters; its sort tie-breaks on title, folder, then file path. | code | `src/ui/library_model.cpp:159` to `:181` and `:257` to `:263` | checked 2026-11-12 |
| P5: `lookup` reads records in chunks of 10,000 ids, then every profile's path rows for the ready charts, keeping the winning profile's. | code | `src/store/record_store.cpp:362`, `:1281` and `:1334` to `:1346` | checked 2026-11-12 |
| P6: The conversion step reports once per row, so a per-row report has precedent. | code | `src/store/record_store.cpp:901` | checked 2026-11-12 |
| P7: The user's library file holds 19,610 chart rows, 18,984 distinct, and the SQL times in the table above. | data | `C:\Program Files\Hydra\hydra.db`, read through a backup copy by the session's `timing.py` | checked 2026-11-12, warm reads in Python only |
| P8: No two chart rows share a file path, so the sort's tie-break is total and the read order cannot change the shown order. | unchecked | `src/store/record_store.cpp:1613` keys the scan cache by path, which implies it but does not prove it | T5 |
| P9: How long `set_charts` and the sort take on 19,610 rows. | unchecked | `src/ui/library_model.cpp:159`; no timing exists | T1 |
| P10: How long Hydra's C++ `lookup` and `set_summaries` take on this file. | unchecked | `src/store/record_store.cpp:1254`; only Python's SQL time is known | T1 |
| P11: How long the whole open takes on this file, first run and repeat. | unchecked | `docs/adr/0026-*.md` gives only the upgrade's time | T1 |
| P12: `HYDRA_UITEST_HOLD_OPEN_S` exists on main and holds the open at every report of the library-read step. | unchecked | `claude/exec-live-unfreeze:tests/ui/uitest_harness.cpp:384` to `:403` shows it on that branch, not yet on main at `7edc33ec` | T4 |
| P13: The `hydra` MCP's `hydra_start` passes `HYDRA_UITEST_HOLD_OPEN_S` from the session's environment to the window it starts. | unchecked | `claude/exec-live-unfreeze:tools/test_hydra_ui_mcp.py:803` patches the variable in a test | LS1 |
| P14: `hydra_text` answers while the open is held. | unchecked | `claude/exec-live-unfreeze:docs/agents/ui-testing.md:269` says the window draws during the hold | LS2 |
| P15: The startup screen shows only after 0.15 s, and "no bar" for an ordinary open was a user ruling. | code | `docs/audit/2026-10-03-fix-decisions.md:514` and `:519`; `src/ui/app_state.h:430` | checked 2026-11-12 |
| P16: The user asked for this plan and for the parallel-query answer. | ruling | the user's message as the main session's brief quotes it: `"yes plan it. is there a faster method than one query? multiple queries loading different parts in parallel?"` | checked 2026-11-12, the main session confirms the quote to the reviewer |
| P17: Existing tests pin the exact list of reports and the label text, so T3 must rewrite them. | code | `tests/test_library_jobs.cpp:550` and `:574`; `tests/test_app_library.cpp:645` and `:936` | checked 2026-11-12 |
| P18: The store holds one connection behind one lock, and its file is in WAL mode. | code | `src/store/record_store.cpp:791` and `:1639` | checked 2026-11-12 |

## Sources for the prior art

- [Questions about running SQL queries in parallel for performance (SQLite forum)](https://sqlite.org/forum/forumpost/4a4b00e6d6?t=c): each connection is single-threaded; parallel reads need one connection per thread, and the gain depends on per-row work.
- [SQLite performance tuning (phiresky)](https://phiresky.github.io/blog/2020/sqlite-performance-tuning): WAL lets readers run alongside each other and a writer.
- [SQLite Optimizations For Ultra High-Performance (PowerSync)](https://www.powersync.com/blog/sqlite-optimizations-for-ultra-high-performance): reader pools help many concurrent queries.
- [Write-Ahead Logging (sqlite.org)](https://www.sqlite.org/wal.html): the source Hydra's own schema comment cites for WAL readers.
