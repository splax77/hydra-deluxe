# Handoff: make the path report fast (2026-10-11)

The user wants three changes combined into one piece of work: stop re-analyzing the whole library every time the path report opens, read each chart file once instead of once per mode, and make the open report window cheaper to draw. No code was changed in the session that wrote this. The next session plans the work and brings the plan to the user before anything is built.

## Where the numbers come from

On 2026-10-11 the main session profiled the real Hydra.exe GUI (build-cpp\Release at b951fc5d), not a command-line tool. A scratch copy of the app ran beside copies of the user's `hydra.db`, `hydra_settings.ini` and `hydra_ui.ini` from `C:\Program Files\Hydra`, under Visual Studio's sampling profiler, for about 5 minutes. A script clicked through what a user does: open songs, step through them, Preview with playback, the tabs, search, scroll, sort, then build and browse the path report. The recording holds 73,097 CPU samples from Hydra over 306 s. The profiler samples the whole machine, so other processes are in the trace too.

Everything from that run is in `build-cpp/prof/gui-2026-10-11/` (gitignored): `hydra_gui_flame.html` (timeline by thread, flame graph and tables, filterable by time range or action), `folded.txt` (one call chain per line), `summary.txt`, `actions.log` (each action's time since Hydra started) and the scripts that made them (`drive.ps1`, `fold.py`, `flame_template.html`). The recipe is in the main session's memory as `gui-profile-recipe-2026-10`.

Sampling sees CPU work only. A thread waiting on the disk, a lock or another process records nothing, so every claim below about waiting is an inference, and says so.

## What the profile found

The path report is the one expensive thing a user does. Building it took 41.6 of the session's 62 CPU-seconds (the 225–243 s window of the recording; the rest of the session was 20 CPU-seconds over 225 s). It took about 17 s of wall time. Song clicks cost almost nothing: the 11 clicks together ran under 0.1 s of analysis.

The report re-analyzes every chart because of D87 (ADR 0026, "the store keeps summaries, the engine gives details"): the database keeps one summary row per chart, so the report opens each chart file and runs the engine again (`collect_rows` in `src/app/report.cpp`). On the user's library that was 37,691 analyses of 18,907 distinct chart files. The count is two modes per chart, because the user has results stored with and without Note Shuffle and the report lists every mode at the current cap.

Inside Hydra's share of the build, 57% was reading and parsing chart files (`load_songpath_with_notes`: 10.7 s in the MIDI reader, 6.5 s in the .chart reader, 2.8 s opening files). The path search was 21% (`run_search`, 8.7 s) and graph building 12% (`ScoreGraph::build`, 5.0 s).

The workers were mostly idle. The pool has 8 threads (`kBatchMaxWorkers`, `src/app/analysis.cpp`), but per second of the build they averaged about 2 busy cores, with one 5.7-core second at the start. The coordinator thread and the UI thread were nearly idle too, so nothing inside Hydra was holding them back. Over the same 16.8 s window, Windows Defender (`MsMpEng.exe`) used 55.2 CPU-seconds, more than Hydra's 40.9. In an equal-length quiet window earlier (160–176.8 s) Defender did not appear in the top six processes. The likely reading is that Defender's real-time scan checked each chart file as Hydra opened it, and Hydra's workers waited for it. That is unproved: the trace cannot show the waits.

The 2026-10-07 D87 measurement (`docs/handoffs/2026-10-07-summary-storage-results.md`) put the report at 2.52 s on 8 workers. Today's 17 s is not comparable as it stands. That run's library, its mode count and whether Defender had already seen the files are all unknown. A warm second build in the scratch copy would separate the Defender cost from the rest. It was proposed but not run, because it reads the whole library twice and needs the user's yes in chat (CLAUDE.md rule 8).

While the report window stays open and nothing changes, it uses about 0.15 of a core redrawing (the UI thread over 263–278 s). The library view alone is about 0.05. Of the report's drawing, about 28% goes to fitting cell text into columns (`text_ellipsized`, called from the table cell code in `src/ui/report_window.cpp`), which is redone every frame.

## The three parts the user chose

**Part 1: don't re-analyze the library on every open.** Two forms came up, and the user has not picked one. Storing the rows the report shows in `hydra.db` makes opening it a database read. It partly reverses D87 and grows the file, by an amount nobody has measured; the report listed 136,087 paths at 5 per chart and mode. A stored record means stored-version stamps (ADR 0018, `src/store/stored_versions.h`), so an engine change marks the rows stale the same way results go stale. The lighter form keeps the last built report in memory until Hydra closes, so a reopen is instant. That reverses D103 item 28, where the user chose that closing the window frees its rows. Either form needs a rule for when the rows go stale: a chart re-analyzed on click, a batch, a settings change, a rescan that adds or removes charts. The batch already hands its rows to the next report through `ReportSeed` (D87 item 5), which is the nearest existing piece. Both forms change a stored record or what the user sees, so the plan takes the choice to the user first (CLAUDE.md rule 3).

**Part 2: read each chart file once, not once per mode.** Today each chart and mode is its own slot, and each slot calls `analyze_chart_file`, which opens and parses the file. Grouping the slots by file would open each file once. That halves the file opens, and so halves Defender's scans if the inference above is right. Parsing can also be shared when the modes differ only in Note Shuffle. The shuffle is the parser's last step (`apply_shuffle_switch` in `src/parse/song.cpp`, after the drum-lane pass), so one parsed song can be copied and shuffled for the second mode. Modes that differ in difficulty, Pro Drums or 2x Bass (`ChartReading` in `src/parse/chart_reading.h`) parse differently and can share only the file's bytes. The rows must come out identical. A row-by-row compare on `testdata/input` before and after shows that, with the D87 rule that tied-score rows may change order.

**Part 4: make the open report window cheaper.** Remember each cell's fitted text until its column width or the rows change, instead of fitting it every frame. This is code-only and small.

The user did not pick part 3, a Defender exclusion for the Clone Hero folder. That is a Windows Security setting, theirs to make.

## Things to know before planning

A session started 2026-10-11 ("Report progress bar: count records like the header", `local_1e9a5d1b-d9b1-4df3-9f9d-398030cfd8f8`) is changing the report's progress count in `collect_rows`. Part 2 rewrites the same function, so the plan forks from that change's merge, not from today's main. Read its merged code first.

The bar it fixes is a counting mismatch. During a build the bar read "of 37,691 records", but the finished header read "38,883 records across 19,444 charts". The bar counted analyses run; the header counts library copies (D76, D77). The gap, 1,192, is 596 duplicate library rows × 2 modes. Part 1 and part 2 both change what the bar counts, so the plan has to keep it agreeing with the header.

Library runs need the user's yes in chat, every time, naming it a "full library run" with the run count and time (memory `no-full-library-runs`). Test on `testdata/input` (97 charts after dedup) or a few charts. The corpus is too quick to time the report: the 2026-10-11 profiler plan measured a whole corpus batch at 0.04 s.

Earlier speed work is in `docs/handoffs/2026-10-06-perf-exploration/REPORT.md` and the 2026-10-07 results. Cross-group dominance pruning was measured unsound there and must not come back. The profiler plan, `docs/superpowers/plans/2026-10-11-profiler-flame-graph.md` (untracked), covers the tooling.

## Open questions for the user

For part 1, does the user want rows stored in the database, kept in memory until Hydra closes, or both? Next, may the plan include one measurement that builds the report twice in a row in a scratch copy, cold then warm? That is two full library runs of roughly 20 s each, and it would tell how much of today's 17 s is Defender. Last, when a chart is re-analyzed on click, should its rows in a kept report update right away, or should the report show itself out of date, the way the existing out-of-date strip does?
