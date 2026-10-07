# Timing agent: status lines (it never wrote a final report)

The agent was interrupted at 23:32 before its last step. These are its own progress lines, verbatim. Its scripts and saved results are in ../scripts/timing/.

```
23:10 done read preamble, report, bench.cpp, app_state open path | next: trace get_record and details tabs, then bench runs
23:11 done traced select -> get_record, paths tab, length job | next: preview/dynamics lazy parse, then bench runs
23:12 done code trace for Q1 (preview, dynamics lazy), found replay dump/target | next: check uitest timing, start parse bench
23:15 done Q4 file checks (0 missing, 0/1000 md5 changed, 288 orphans); bench lock held by another session's JC run | next: parse bench under lock
23:21 done found main Release bench lacks --parse/--engine and folder mode dies on no-notes charts | next: check perf-h1 build matches main src, then use it read-only
23:24 done whole-library parse timing (median 0.49 ms, max 220 ms), record size list | next: per-chart engine sample under lock
23:25 done engine sample 448 charts (median total 1.8 ms, blink 202 ms) | next: record load/decode timing, determinism redo
23:26 done picked 19 folders (74 charts) for redo test; main moved to M1 mid-run, using 2.1.0 ship exes | next: batch --redo into scratch copy, compare_db
23:27 done Q3 redo of 74 charts = 0 differing rows in every table | next: tiny DB for record load/decode and replay/target timings
```
