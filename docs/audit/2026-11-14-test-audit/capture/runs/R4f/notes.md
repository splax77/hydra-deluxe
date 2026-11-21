# R4f notes: Path report window after a real batch

Short names: `rep.cpp` is tests/test_app_reports.cpp (CLEAN.md and counts/T5-IR19.md say so). The test_app_reports.cpp lines in the brief match the TEST_CASEs listed below. Labels checked by grep in src/ui: `Open path report` / `Building path report...` (library_toolbar.cpp:141), `Analyze library...` (library_toolbar.cpp:87), `Analyze library` popup, `Start analyzing`, `##redo` (library_dialogs.cpp), `Open report`, `Open automatically` (library_dialogs.cpp:510-550), `Refresh`, `Close` (report_window.cpp:81-93), `Refresh##outofdate` (report_window.cpp:129), `Report cancelled.` (report_window.cpp:196), `##search` (report_window.cpp:295), `##timing`, `Optimal paths only`, `Clear filters` (path_report_window.cpp:160-166), `###pathreport` (path_report_window.cpp:195), `###analysissettings` (settings_bar.cpp:305), `##mslimit` (settings_bar.cpp:203), `All timing tiers` and `No timing needed` (app/path_report_view.cpp:55, 46). `Pitch` is the song `Pitch Dark` (testdata/input/test_pitch_dark_ratio/song.ini). Unverified: `click ##timing/No timing needed` (combo ref form copied from the `##status/...` example in ui-testing.md; whether that tier exists on this corpus is not known), `key Escape` closing the settings popup, and `check //$FOCUSED/**/##mslimit` (the documented ref form). If a line fails, the script stops there; later sections are independent and can be run from a trimmed copy.

## A7 #1 (line 81, request builds in memory, one at a time)
Claims: `request_path_report` builds in memory, a second request while one runs starts nothing, result has rows, window stays shut. Covers `AppState::request_path_report`. Script: the batch after Start analyzing builds the report itself (lines under "A7 #1"). Confirming text: the done strip reads `The path report is ready.` and the toolbar says `Open path report`, with `dump Path report` finding no window. The one-at-a-time half (a held build) cannot be shown on a screen.

## A7 #2 (109, show opens the window and builds only the first time)
Covers `show_path_report`. Script: `click Open path report` after the batch. Confirm: window text has rows at once, no `Building the report from your library...`. The first-time build half is in `Open path report` with nothing built, which this flow does not do (a batch already built it).

## A7 #3 (123, `--open-report` opens it once)
No screen shows this in a command file: it needs the Hydra.exe `--open-report` flag, which hydra_uitest does not take. Not shown.

## A7 #4 (150, no flag: stays shut)
Shown: after the batch, `dump Path report` finds no Path report window until the click.

## A7 #5 (161, closing keeps rows, reopen no build)
Script: `key Escape`, `dump Path report` (window gone), then `click Open path report`. Confirm: same count line and rows as before, no Building text. Covers `close_path_report` / `show_path_report`.

## A7 #6 (185, built-on-open keeps filters; Refresh's does not)
Script: filters set, Escape, reopen (filters kept: search box `{ Pitch }`, timing box and Optimal paths only as left), then `click Refresh` (filters reset). Internal `built_on_open` and `path_report_input().keep_filters` are not visible; only their effect is.

## A7 #7 (208, closing mid-build lets the build finish)
Needs a build held open while the window is closed. A real build over six charts is too fast to catch; not shown (the hold seam `set_report_hold_for_test` is test-only).

## A7 #8 (240, reopen after a cancelled build shows cancelled, Try again builds)
Best effort only at the end of the script: `click Refresh` then `click Cancel`. If the build lands first, the click fails and the script stops with `!! failed`. If it works, `text` shows `Report cancelled.` and `Try again`. Not reliably shown.

## A7 #9 (267, out of date on a batch or a setting)
Shown for settings: the Analysis settings panel's `##mslimit` is ticked, the window reopens with the strip `The settings changed since this report was built.` and `Refresh##outofdate`; then the setting is put back. The negative case (a setting the report does not read leaves it alone) is not scripted. The batch half (`Your library changed ... (a batch finished at HH:MM)`) cannot be seen: the batch's own rebuild makes it current at once. The "Open automatically" half is scripted at the end (redo batch, window opens by itself; confirm with `dump Path report`).

## A7 #10 (327, a setting changed during a build leaves it out of date)
Not shown: needs a setting change inside a held build. The after-build outcome is the same strip as #9.

## A7 #11 (344, Cancel keeps the last report)
Same best-effort lines as #8. Not reliably shown.

## A7 #43 (test_app_shell.cpp:401) and #47 (470)
These are `share_is_valid` (library split fraction check) and "a folder without font files loads no fonts". They are not about the Path report. The first has no screen in this flow (the split is dragged by hand); the second is a startup setup with no font folder, which no command file can set. Not shown. If the record lines were meant to be different tests, the numbers are wrong: please check.

## A47 2, 6, 7
test_open_path (1051): opens from the finished strip's `Open report`, the toolbar, and automatically. Script uses the toolbar button and auto-open; the finished strip's `Open report` is not clicked (the dump after the batch shows the strip). test_reopen (1345): script reopen section; confirm same count line before and after. test_reopen_keeps_filters (1419): filter section; confirm the search box text `{ Pitch }`, the timing box text and the count line the same before Escape and after reopening. The sort click (Song header) is not scripted.

## A25 P1 (test_report_page.cpp:62, write_report_file)
`write_report_file` (src/app/report_files.cpp:159) swaps an HTML page in with no .tmp left. Used by `hydra_batch` and fillcompare, not by the window. No screen shows this.

## A25 R25 (test_report_rows.cpp:902, generate_report with cancel flag set)
Internal: the cancel flag is set when Hydra closes during a build. Not shown on a screen.

## A25 R29 (962, PageSettings::of differs when chart mode changes)
The report reads the chart mode. To see it, change the chart mode (`##prodrums` in the Analysis settings panel, `view_prodrums`) and reopen; the same strip should appear. Not scripted: changing the mode also changes which row the library shows. `PageSettings::of` itself is internal.

## A25 R30 (976, settings_change_touches)
Covered by A7 #9's `##mslimit` change (the lens). SP cap (`##spcap`) and legacy fills (`##legacyfills`) would use the same lines; not scripted. The negative check, `auto_open_report`, is the Open automatically tick: ticking it with the window open must not raise the strip (the script ticks it with the window closed, so it cannot show this).

## DL3 32 (rep.cpp:952 report builds while a save group is open)
Internal: a held store write group. No screen shows it.

## DL3 38 (rep.cpp:1097 a cancelled report closes its connection)
Internal: the store connection's write-ahead file. No screen shows it; the cancel half is the A7 #8 best effort.

## DL3 39 (rep.cpp:272, the same stale test as A7 #9)
Same lines as A7 #9.
