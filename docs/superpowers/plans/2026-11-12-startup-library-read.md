# The startup library read: measure it, then give it a counted bar

Date: 2026-11-12. Status: a plan, waiting for the user's answers under `## Questions`. No code changes until then.

## The plain version

The startup screen says "Opening your library..." and shows no bar while Hydra reads the chart list. The user asked why a line with no visible progress is good design, and whether several queries in parallel would be faster than one.

The short answers, from measurements on a copy of the user's own library file:

**Would several queries in parallel be faster? No.** SQLite allows them, and four readers did cut SQLite's own part of the chart-list walk from 10.6 ms to 3.0 ms. That is about 8 ms saved out of a read of about 215 ms. The time is not in SQLite's scan. It is in the sort order the query asks for (about 37 ms), in reading 68,743 saved path rows (about 106 ms), and in Hydra's own code after the query, which nobody has timed yet. The numbers and where they came from are in "What was measured, and how".

**How long is the screen up on this library? A fraction of a second.** All the SQL the library read runs took about 215 ms in total on the copy (70 ms for the chart list, 144 ms for the saved results). Hydra shows the startup screen only once an open has run 0.15 s. So on this library the screen shows, if at all, for roughly the last tenth of a second or so of the open. That is an estimate from SQL times alone; T1 times the whole open in Hydra's own code and settles it. The screenshot the user saw showed it for longer only because the test window held the read on purpose.

So a counted bar would help a bigger library, a slower disk or a first start after a reboot, and it would flash by on this machine. That is the honest size of what is being asked for, and question 1 says so. The user's complaint was about a screen that gives one generic line and no sign of progress. The bar answers that complaint where the screen lasts long enough to read. Leaving the screen as it is stays a fair answer, and question 1 offers it.

The plan has two halves. First it times Hydra's own code on the same copy (T1), because that part cannot be timed without a build. Then, if the user says yes, it gives the library read a counted bar (T2 to T4). One cheap speed-up the measurements point to is offered as a question (T5), not decided here.

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

A second run of the same script by the plan's reviewer on the same copy gave medians of 75.2 ms for the chart list, 49.0 ms inside SQLite, 34.3 ms with no `ORDER BY`, 1.55 ms for the count, 160.6 ms for the saved-results read and 2.9 ms for four readers, with the machine a little busier. The story is the same. The table keeps the author's first run.

SQLite walks the chart list through the `charts_by_name` index (`EXPLAIN QUERY PLAN` answered `SCAN charts USING INDEX charts_by_name` for the ten-column query; the index holds only `name`, so a query for the name alone would say `USING COVERING INDEX`). An index walk fetches each row from a different place in the file, which is why the sort order costs more than the scan.

Three things these numbers do not say. They are Python's times, not Hydra's; Hydra's C++ turns rows into objects faster than Python does, so the SQL share in Hydra is likely lower. They are warm reads; a first start after a reboot reads the 23 MB file from disk. And they leave out everything Hydra does after the query: building each row's search text and sort keys, sorting 19,610 rows, and picking each chart's winning record. T1 measures those three.

### T1's numbers (Hydra's own code)

The main session ran T1's case on 2026-11-12, built from commit be27ce80, with `HYDRA_TIMING_DB` set to a file copy of the same backup, as `hydra_tests.exe "-tc=timing: the startup library read by part" -s`. The case uses the default settings profile (`hydra::app::Settings{}`), not the user's saved one, so the `lookup` line may differ from a real start. The open is warm.

| Part | Run 1 | Fastest (run 5) |
|---|---|---|
| open | 2.4 ms | 1.6 ms |
| list (`list_chart_library`) | 50.2 ms | 45.2 ms |
| set_charts | 57.3 ms | 57.2 ms |
| lookup | 23.2 ms | 21.5 ms |
| set_summaries | 0.34 ms | 0.32 ms |
| sort | 0.0001 ms | 0 ms |
| total | 133.4 ms | 125.8 ms |

So on a warm start Hydra reads this library in about 126 to 133 ms, under the 0.15 s after which the startup screen first shows (`kViewProgressDelaySeconds`). On this library, warm, the screen does not show at all. The largest single part is `set_charts`, Hydra's own work after the query, not the SQL. A cold first start was not timed.

## Is there a faster method than one query?

**Several queries in parallel: no.** SQLite does allow it. In WAL mode (the journal mode Hydra's file uses, where readers do not block each other), each thread needs its own connection, and each connection is single-threaded. The measurement shows it works: 10.6 ms down to 3.0 ms with four connections. But it speeds up only SQLite's scan, and the scan is about 11 ms of a 215 ms read. Hydra's store holds one connection behind one lock, so parallel readers would mean a second set of read-only connections and the code to merge their rows, all to save about 8 ms. Prior art agrees: SQLite forum threads and write-ups on WAL reader pools report that parallel readers help servers answer many small queries at once, and that splitting one bulk scan across threads gives little, because the cost is per-row work and I/O (sources at the end).

**Fewer columns up front: no.** Dropping the three columns the library table never keeps saved 9 ms.

**Dropping the sort order: maybe (T5, question 5).** The query sorts by name, then `LibraryModel::set_charts` sorts the rows again by the column the user has chosen. The first sort cost 37 ms of the 70. Whether 37 ms matters depends on T1's total.

**The saved path rows: a lead, not a task.** Reading 68,743 path rows is the largest single piece (106 ms), and `lookup` reads every profile's paths, then keeps only the winning profile's. Changing it touches which result a chart shows, so it is listed under "Out of scope" until T1 says the time is worth chasing.

**The recommendation.** One query per table is already near the floor for the SQL. If T1 shows Hydra's own code is the larger share, the speed work belongs there, and it is a new plan with its own measurements. This plan does not guess at it.

## What the user will see

The only screen that changes is the startup screen (`render_startup_screen`). It shows only when an open is still running after 0.15 s. In the examples below the total is the user's own library's (19,610 charts) and the done counts are sample moments.

Before, during the library read on a file that needs no upgrade:

```
Opening your library...
```

After, if the user takes the recommended answers (questions 1 to 4):

```
Reading your charts...
[=========>            ]  8,000 / 19,610
```

then

```
Reading your results...
[===============>      ]  14,000 / 19,610
```

While the file itself is being opened, before any row can be counted, the line stays "Opening your library..." with no bar. Nothing can be counted there: the wait is the disk. When the first count arrives the box grows by one row to hold the bar, and the centred box moves up by half a row once. T4 checks the box still fits.

On a file that is being upgraded (an old library file, once per version), the screen today reads, after the fill step's "Reading saved paths..." bar:

```
Updating your library file for this version of Hydra
This happens once. Your charts and results are kept.
Finishing...
[==============================]  1,200 / 1,200
```

The bar there is the fill step's last count left full. After the change the same box reads the same two heading lines, then "Reading your charts..." over a bar counting charts from 0 to N, then "Reading your results..." over a bar counting from 0 to N. That is also a visible change, so it is part of question 1.

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

Owned files: `src/ui/library_jobs.h`, `src/ui/library_jobs.cpp`, `tests/test_library_jobs.cpp`, `tests/test_app_library.cpp`. Depends on T2 and on the answers to questions 1 to 4.

`StoreOpenProgress::Step` gains two steps in place of `LoadingLibrary`: reading charts and reading results. `read_library` takes an optional progress callback. The job asks `chart_library_count()` once for the total and uses that one number for both steps. Before the chart list is read the job reports reading charts at 0 of N, so the bar and its first line appear before the first row. During the first step the count is the rows T2's callback reports. During the second, `read_library_summaries` asks `lookup` for the rows' results one slice at a time (the slice size is question 4's answer), reports the rows answered after each slice, then hands the joined answers to `set_summaries` in one call, so the table is filtered once as today. `StoreOpenJob::report` keeps publishing the counts before the step. `label()` returns the two new lines.

One trap in `read_library`: it wraps both of its reads in `catch (const std::exception&)` and turns the exception into the status line's sentence, and the job then ends as done. The cancel the store throws when the callback returns false is a `KindedError(ErrorKind::Cancelled)`, which is a `std::exception`, so as written a stop during the read would be swallowed and shown as a library problem. `read_library` must let a `Cancelled` pass through before its generic catch, and the job then ends as a cancel (criterion 5).

`AppState::reload_library` (the read after a scan, `src/ui/app_state.cpp:193`) passes no callback and shows nothing new. `read_library_summaries` is also called alone by the settings re-read (`src/ui/app_state.cpp:229`); with no callback it keeps its one whole `lookup`, so that caller is unchanged too.

Acceptance criteria:

1. An open of a file that needs no upgrade reports `Opening`, then reading charts from 0 to N of N, then reading results from 0 to N of N, where N is `chart_library_count()`. Check: rewrite "jobs: the startup open reads saved paths with its own words, then the library" (`tests/test_library_jobs.cpp:533`, which pins `LoadingLibrary` at `:560` and `:576`) and "the startup open runs on its own thread and tick collects the store and library" (`tests/test_app_library.cpp:605`, which pins the step list at `:645`), failing first.
2. The total is the same number in both steps and equals the rows the model holds afterwards. Check: a `CHECK` in the same case comparing every report's total with `library.rows().size()` or its public equivalent.
3. The sliced results read gives every row the same answer as one whole read. Check: new test "jobs: results read in slices match one read", on the corpus store with a slice size of 1 and of the real size, comparing `set_summaries` outcomes row by row.
4. A chart listed in two folders gets its result in both rows when the two rows fall in different slices. Check: the same test, with slice size 1 and the corpus's duplicate (`chart("same", ...)` pattern in `tests/test_library_model.cpp`).
5. A cancel during either step, including one that lands inside the chart-list read, ends the job as a cancel with no message. Check: extend "jobs: a stop while the startup open reads saved paths ends it as a cancel" (`tests/test_library_jobs.cpp:614`) with a stop at each new step and one at row 2 of the chart list, failing first for the row-2 stop (as written, `read_library` would swallow it).
6. A read that fails still leaves its sentence for the status line and the app opens. Check: "a library read that fails after the startup open is the status line's problem" (`tests/test_app_library.cpp:667`, its gate keyed to `LoadingLibrary` at `:675`) and "jobs: a fill that fails still opens the library and leaves its sentence for the status line" (`tests/test_library_jobs.cpp:582`, `:607`) stay green with their gates keyed to the new steps.
7. `label()` returns the exact lines the user chose. Check: rewrite "the startup screen's line names what the open is doing" (`tests/test_app_library.cpp:936`, `LoadingLibrary` at `:937` and `:943`) with the chosen literals.
8. `read_library_summaries` with no callback is one whole `lookup`, as today. Check: the criterion 3 test also calls it with no callback and compares the outcomes with the sliced call's; no existing test names `AppState::read_summaries_in_place`, so the reviewer also reads that call site.

### T4: The startup screen draws the bar, and the hold gate holds once

Owned files: `src/ui/library_view.cpp`, `tests/ui/uitest_harness.h`, `tests/ui/uitest_harness.cpp`, `tests/ui/uitest_library.cpp`, `docs/agents/ui-testing.md`, `docs/adr/0026-*.md` ("What the user sees"), `docs/audit/2026-10-03-fix-decisions.md` (a new D entry that replaces D100 item 7, in the user's words from the answers). Depends on T3.

`render_startup_screen` already draws `progress_bar_counted` whenever `rows_total > 0`, so it needs no new drawing code; the task checks the box height still fits one text row plus the bar, and changes nothing else there unless it does not.

The test gate `OpenGate` (`tests/ui/uitest_harness.cpp:382` to `:408`, merged with 6a0b3539) holds the open at the library-read step: `Step::LoadingLibrary` for the `LibraryRead` and `StartupDrawn` holds, `ConvertingRecords` for the `Conversion` hold. T3 removes `LoadingLibrary`, so the gate must name the first new step instead. There is also a second change to make. Today the step reports once, so the gate holds once. A `StartupDrawn` hold lets go on its own timer, not through `open()`, so the gate's open flag stays false, and the gate's check runs again at the step's next report and holds a fresh `min_hold_s` each time (`held_from` is set inside the call). With one report per row that would hold 19,610 times, and a 5 s hold would be hours. The `LibraryRead` hold is not affected, because `open()` ends it for good. The task makes a `StartupDrawn` gate hold at the first report of the first library-read step only, then let every later report through. That first report is the 0 of N one T3 adds, so the held screen shows "0 / N". The harness header's comment on `OpenHold` says "At the library read"; it keeps that meaning with the new step names.

Acceptance criteria:

1. A held open on the test corpus shows the chosen first line and a bar reading "0 / N" in the startup box, N being the corpus's chart count. Check: new uitest "startup: the library read shows a counted bar", reading the box's text; failing first.
2. A `StartupDrawn` hold of S seconds delays the open by about S seconds once, not once per report. Check: the same uitest asserts the open finishes within the existing wait cap after one hold; and the existing `test_a_held_open_keeps_drawing_and_the_hello_waits_for_the_library` in `tools/test_hydra_ui_mcp.py:799`, which needs a built `hydra_uitest` and which the main session runs, stays green (it would run long if the gate re-held per row).
3. The held startup box can be looked at. Check: the same uitest saves a screenshot of the held box with the harness's `screenshot` helper (`tests/ui/uitest_engine.h:220`), and the main session views it and shows it to the user before merge. P21 says this is unchecked until T4 runs it.
4. The Conversion hold still holds at the conversion. Check: the existing upgrade uitest (the one that checks `Step::ConvertingRecords`) stays green.
5. ADR 0026 and the new D entry say what the screen shows now, and `ui-testing.md` says what a held open shows. Check: `hydra_tests.exe "-tc=docs match code*"`.

### T5: Read the chart list unsorted at startup (only if question 5 is a yes)

Owned files: `src/store/record_store.h`, `src/store/record_store.cpp`, `src/ui/library_jobs.cpp`, `tests/test_library_model.cpp`. Depends on T1, and on T2 and T3 if question 1 is a yes (both edit the same read).

`read_library` asks the store for the chart list in table order instead of name order. The other callers of `list_chart_library` keep the name order. `set_charts` sorts the rows itself, with a tie-break that ends on the chart's file path, so the table the user sees does not change. On the user's library copy all 19,610 rows have distinct file paths (P8, checked), but the `charts` table has no uniqueness rule on `path` (P19), so the tie-break is total only for libraries whose scan wrote one row per path. Two rows with the same path and the same title and folder would tie, and their order would follow the read order. That case has no visible difference between the two rows except their other columns, and this task's test (criterion 2) pins the corpus, not every library; the task states that gap in its comment rather than claiming a total order.

Acceptance criteria:

1. The library's shown order is the same for both read orders, for every sort column in both directions. Check: new test "library: the shown order does not depend on the order the rows were read", feeding `set_charts` the corpus rows in name order and in reverse.
2. No two chart rows in the corpus store share a file path. Check: a `CHECK` in the same test; this is what makes the tie-break total.
3. T1's timing case, run again by the main session on the copy, shows the list part faster than before. Check: the two printed lines, written into this plan. If it is not faster, the task is dropped and the plan says so.

## Why there is no live step

An earlier draft had two live steps that started the `hydra` MCP window with `HYDRA_UITEST_HOLD_OPEN_S` set and read the held startup screen with `hydra_text`. Neither can work, and both reasons are in code the reviewer read on main.

The MCP helper starts Hydra with `subprocess.Popen` and passes no environment (`tools/hydra_ui_mcp.py:380`), so the window inherits the helper's own environment, which was fixed when Claude Code started the helper (`.mcp.json` sets no `env`). A main session cannot set the hold for one `hydra_start` call (P13).

Worse, a held open sends no hello line until the hold ends: `start_live_session` returns only once the library is open, and the hello is written after it (`tests/ui/uitest_live.cpp:252` and `:324`). `hydra_start` waits for the hello, so no `hydra_text` call can run while the open is held, and the helper test `tools/test_hydra_ui_mcp.py:799` pins that wait (P14). A live step could only read the screen after the startup box was gone.

The held startup box is seen instead by T4's uitest, which runs headless on the test corpus, reads the box's text and saves a screenshot of it (T4, criteria 1 and 3). That is the picture the main session shows the user. A live window on the user's own library copy would need a new way to hold and read the open, which is out of scope here and listed below.

## Out of scope (for the user to approve)

Reading fewer saved path rows in `lookup`. It is the largest SQL piece (106 ms measured), but it touches which result a row shows. It needs its own plan after T1.

Speeding up `set_charts` or the sort. T1 measures them; any change is a new plan.

Parallel reader connections. Measured at 8 ms saved; not worth new connection machinery.

A time-left line on the new bars. D100 item 4 gave the upgrade one; this plan adds none, because the read is too short for the 3 s wait that line uses.

A way to hold and read the startup screen of the live `hydra` MCP window on the user's own library copy (see "Why there is no live step"). The uitest in T4 covers the screen on the test corpus.

## Questions

1. **Should the library read show a counted bar, replacing D100 item 7?** On this library the screen shows for a fraction of a second, so on this machine you would rarely see the bar; it is for a bigger library, a slower disk or a first start after a reboot. The rule today, in D100 item 7's words: "A file that needs no upgrade but takes over 0.15 s to open, say on a drive waking from sleep, shows "Opening your library..." with no bar, for both the open and the library read." At one end, a library that opens in under 0.15 s shows no screen at all, before and after. At the other, a library whose read takes seconds shows one unchanging line today, and would show a bar counting its charts. On a file being upgraded, the third line changes from "Finishing..." over a full bar to the new lines with their own counts (the exact before and after text is under "What the user will see"). Answers: yes, build the bar (recommended, because it answers "no visible progress"); or no, leave D100 item 7 as it is and drop T2 to T4 (T1 and T5 stand alone). T1, the timing, is worth doing either way.

2. **What are the words?** Recommended: "Opening your library..." while the file opens (unchanged, no bar), "Reading your charts..." while the chart list is read, and "Reading your results..." while saved results are read. The alternative is to keep "Opening your library..." over both bars, which changes fewer words but does not say why the bar starts again.

3. **One counted step or two?** Measured, the chart list is 70 ms of the SQL and the results are 144 ms. With one counted step the bar fills during the first third and then sits full. With two, the results are read in slices and the bar moves once per slice. Recommended: two steps.

4. **How big is a results slice (your number, if you chose two steps)?** The slice size is a new number and the plan does not pick it for you. The suggestion is 1,000 charts, which moves the bar 20 times on 19,610 charts and once on a 500-chart library. Smaller slices mean more queries, each with its own fixed cost; T1's case can time the difference if you want a number first. If you chose one step, this question does not apply.

5. **Read the chart list unsorted at startup (T5)?** Measured saving: about 37 ms of 70 on this library, in Python's timing. Nothing visible changes if P8 holds. Recommended: decide after T1. Take it if the list is a real share of the total, skip it if Hydra's own code dwarfs it.

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
| The hold gate holds once | T4, criterion 2 | T4 | The same uitest finishes inside the wait cap; the helper test `test_a_held_open_keeps_drawing_and_the_hello_waits_for_the_library` stays green |
| The Conversion hold is unchanged | T4, criterion 4 | T4 | The existing upgrade uitest |
| The docs say what the screen shows now | T4, criterion 5 | T4 | `hydra_tests.exe "-tc=docs match code*"` |
| The shown order does not depend on the read order | T5, criteria 1 and 2 | T5 | "library: the shown order does not depend on the order the rows were read" |
| T5 is dropped if it is not faster | T5, criterion 3 | T5 | The two timing lines written into the plan |
| The bar's total agrees with the Library header | "What the user will see"; T3, criterion 2 | T3 | The `CHECK` that every report's total equals the rows the model holds afterwards |
| A cancel inside the chart-list read is a cancel, not a status-line sentence | T3 body ("One trap in `read_library`"); T3, criterion 5 | T3 | The row-2 stop in the extended cancel case, failing first |
| The held startup box can be seen | T4, criterion 3 | T4 | The screenshot the uitest saves, viewed by the main session |
| No whole-library song scan | Global constraints | T1 | T1 reads one file named by `HYDRA_TIMING_DB`; the reviewer confirms no task names a song folder |

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
| P8: On the user's library copy, all 19,610 chart rows have distinct file paths, so there the sort's tie-break ends on a unique key. | data | the copy's `charts` table: `SELECT count(*), count(DISTINCT path) FROM charts` gave 19610 and 19610, run by the plan's reviewer on `hydra-copy.db` in the scratch folder | checked 2026-11-12, this one file only |
| P19: In every library file no two chart rows share a file path, so the read order can never change the shown order. | unchecked | `src/store/record_store.cpp:797` to `:809` creates `charts` with no unique rule on `path`; `:1613` keys the scan cache by path, which implies one row per path without enforcing it | T5 |
| P9: `set_charts` takes about 57 ms and the sort under 0.001 ms on 19,610 rows. | run | T1's case `timing: the startup library read by part` on a copy of the user's library, 2026-11-12, table "T1's numbers" | checked 2026-11-12 |
| P10: Hydra's C++ `lookup` takes about 21 to 23 ms (default profile) and `set_summaries` about 0.3 ms on this file. | run | the same run, table "T1's numbers" | checked 2026-11-12 |
| P11: The whole open takes 133 ms on run 1 and 126 ms at its fastest, warm. | run | the same run, table "T1's numbers" | checked 2026-11-12 |
| P12: On main, `OpenGate`'s `StartupDrawn` hold waits at `Step::LoadingLibrary` until a startup frame is drawn and `min_hold_s` has passed, and it re-checks, and holds a fresh `min_hold_s`, at every later report of that step, because letting go that way never sets the gate's open flag. The step reports once today. `HYDRA_UITEST_HOLD_OPEN_S` feeds this hold. | code | `tests/ui/uitest_harness.cpp:382` to `:408`, `tests/ui/uitest_live.cpp:248` and `:285` to `:297`, `src/ui/library_jobs.cpp:155` | checked 2026-11-12, merged code at 6a0b3539 |
| P13: The `hydra` MCP helper starts Hydra with no `env` argument, so the window inherits the helper's own environment, set when Claude Code started the helper; `.mcp.json` sets none. A main session cannot set `HYDRA_UITEST_HOLD_OPEN_S` for one `hydra_start` call. | code | `tools/hydra_ui_mcp.py:380` to `:382` and `.mcp.json` | checked 2026-11-12 |
| P14: A held open sends no hello line until the hold ends, so `hydra_start` returns and `hydra_text` can run only after the startup screen is gone. | code | `tests/ui/uitest_live.cpp:252` to `:260` and `:324`, and `tools/test_hydra_ui_mcp.py:799` to `:807` | checked 2026-11-12 |
| P15: The startup screen shows only after 0.15 s, and "no bar" for an ordinary open was a user ruling. | code | `docs/audit/2026-10-03-fix-decisions.md:514` and `:519`; `src/ui/app_state.h:430` | checked 2026-11-12 |
| P16: The user asked for this plan and for the parallel-query answer. | ruling | the user's message as the main session's brief quotes it: `"yes plan it. is there a faster method than one query? multiple queries loading different parts in parallel?"` | checked 2026-11-12, the main session confirms the quote to the reviewer |
| P17: Existing tests pin the exact list of reports and the label text, so T3 must rewrite them. | code | `tests/test_library_jobs.cpp:560`, `:576` and `:607`; `tests/test_app_library.cpp:645`, `:675`, `:937` and `:943`; and the gate in `tests/ui/uitest_harness.cpp:390` | checked 2026-11-12, the lines the reviewer found by searching `LoadingLibrary` in `src` and `tests` |
| P18: The store holds one connection behind one lock, and its file is in WAL mode. | code | `src/store/record_store.cpp:791` and `:1639` | checked 2026-11-12 |
| P20: `read_library` catches `std::exception` around both its reads and returns the sentence, `Cancelled` is a `KindedError` and so a `std::exception`, and the job's `run_guarded` calls a run that returned true done even if cancelled, so a cancel thrown inside the read would end as a status-line sentence unless `read_library` lets it through. | code | `src/ui/library_jobs.cpp:42` to `:55`, `src/app/job_base.h:81` to `:85` and `:101` to `:109`, `src/store/record_store.h:427` | checked 2026-11-12 |
| P21: The harness's `screenshot` helper saves a picture of the headless window during a held open. | unchecked | `tests/ui/uitest_engine.h:220` declares the helper; `docs/agents/ui-testing.md:228` lists the `screenshot` verb; nothing says it works with the startup box up | T4 |
| P22: `read_library_summaries` is also called by `AppState::read_summaries_in_place`, and no test names that function. | code | `src/ui/app_state.cpp:228` to `:230`; a search of `src` and `tests` for `read_summaries_in_place` found only `app_state.cpp` and `app_state.h` | checked 2026-11-12 |

## Sources for the prior art

- [Questions about running SQL queries in parallel for performance (SQLite forum)](https://sqlite.org/forum/forumpost/4a4b00e6d6?t=c): each connection is single-threaded; parallel reads need one connection per thread, and the gain depends on per-row work.
- [SQLite performance tuning (phiresky)](https://phiresky.github.io/blog/2020/sqlite-performance-tuning): WAL lets readers run alongside each other and a writer.
- [SQLite Optimizations For Ultra High-Performance (PowerSync)](https://www.powersync.com/blog/sqlite-optimizations-for-ultra-high-performance): reader pools help many concurrent queries.
- [Write-Ahead Logging (sqlite.org)](https://www.sqlite.org/wal.html): the source Hydra's own schema comment cites for WAL readers.
