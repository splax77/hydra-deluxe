# Kinds of flaw

## fake data

hit rows: 124
hit lines: 61
tests: 89
flagged records: 69

- test: fixtures: a Stale record for each cause | tests/display_fixtures.h:169 | no record
- test: charts_with_result counts a Ready record and not a Stale or sibling-profile one | tests/test_analysis.cpp:241 | flagged
- test: a Stale converted record is replaced on click | tests/test_app_click.cpp:162 | flagged
- test: clicking a Stale song turns its row Ready | tests/test_app_click.cpp:195 | flagged
- test: the chart-file check runs on open and then every two seconds | tests/test_app_click.cpp:1039 | flagged
- test: a batch result for the open chart refreshes its row, not the panel | tests/test_app_library.cpp:178 | says something else
- test: a batch result for an open Ready chart changes its row and keeps the panel | tests/test_app_library.cpp:211 | flagged
- test: a batch refresh fills scores in where the rows sit; the batch's end sorts them | tests/test_app_library.cpp:532 | flagged
- test: a batch whose last result was already read still sorts when it ends | tests/test_app_library.cpp:555 | flagged
- test: a batch's end sorts once, and the next batch sorts when it ends | tests/test_app_library.cpp:565 | flagged
- test: a batch dismissed before a tick saw it finish still sorts | tests/test_app_library.cpp:591 | flagged
- test: a tick during a batch does not wait for the store | tests/test_app_library.cpp:615 | flagged
- test: a tick during a batch re-reads no row the batch did not save | tests/test_app_library.cpp:632 | flagged
- test: a batch's end re-reads no row the batch did not save | tests/test_app_library.cpp:644 | says something else
- test: a batch that fails as a whole still re-reads every row when it ends | tests/test_app_library.cpp:665 | flagged
- test: a lookup the batch could not make shows the store's problem | tests/test_app_library.cpp:684 | says something else
- test: a chart in two folders gets both rows from one batch result | tests/test_app_library.cpp:723 | flagged
- test: a failed chart whose failure the batch stored shows on its row, and one it did not  | tests/test_app_library.cpp:755 | no record
- test: a paused batch's stored results reach their rows | tests/test_app_library.cpp:786 | flagged
- test: a stopped batch's stored results reach their rows, then the list sorts | tests/test_app_library.cpp:801 | flagged
- test: a scan cannot start during a batch, and the status line says so | tests/test_app_library.cpp:912 | flagged
- test: any_job_running counts a running batch, and a click's job until tick collects it | tests/test_app_library.cpp:1007 | no record
- test: a batch already running is waited on, and its finish builds the report | tests/test_app_reports.cpp:721 | flagged
- test: a DM report builds while a save group is open | tests/test_app_reports.cpp:929 | says something else
- test: a report whose connection cannot open fails with the store's sentence | tests/test_app_reports.cpp:1013 | says something else
- test: http_download calls the installed downloader in place of the network | tests/test_http_download.cpp:112 | flagged
- test: launch: a missing builder exe has no path, and launching it says why | tests/test_launch.cpp:59 | flagged
- test: jobs: a running batch's time left neither collapses after a group nor climbs in a gap | tests/test_library_jobs.cpp:431 | no record
- test: jobs: a frame's snapshot does not wait while the batch job waits on the store | tests/test_library_jobs.cpp:1040 | flagged
- test: jobs: a finished batch hands the memory its charts freed back to Windows | tests/test_library_jobs.cpp:1136 | flagged
- test: jobs: results read in slices match one read | tests/test_library_jobs.cpp:1520 | flagged
- test: library model: a row Stale from another Hydra version carries that cause | tests/test_library_model.cpp:423 | no record
- test: library model: a row Stale from other rules carries that cause | tests/test_library_model.cpp:432 | no record
- test: library model: a row Stale from both causes carries both | tests/test_library_model.cpp:441 | no record
- test: library model: a Stale row whose cause changes takes the new cause | tests/test_library_model.cpp:450 | flagged
- test: library model: a lookup that differs only in a field the library never reads changes nothing | tests/test_library_model.cpp:660 | flagged
- test: library model: every status has its own chip, and All counts them all | tests/test_library_model.cpp:729 | flagged
- test: library model: the search narrows the rows and the chip counts follow it | tests/test_library_model.cpp:868 | flagged
- test: library model: a status chip narrows the rows, and an emptied chip falls back to All | tests/test_library_model.cpp:903 | flagged
- test: library model: the search or a chip narrows the table, and nothing else does | tests/test_library_model.cpp:922 | flagged
- test: library model: Best path sorts by score, with unscored rows last both ways | tests/test_library_model.cpp:939 | flagged
- test: library model: a new score fills in where the row sits, and one call sorts it | tests/test_library_model.cpp:976 | flagged
- test: library model: a whole re-read of the summaries keeps the rows in place too | tests/test_library_model.cpp:992 | flagged
- test: library model: answers that change nothing leave the order up to date | tests/test_library_model.cpp:1002 | flagged
- test: library model: a new score under another sort leaves nothing to re-sort | tests/test_library_model.cpp:1012 | flagged
- test: library model: a held row still leaves a chip it no longer belongs to | tests/test_library_model.cpp:1021 | flagged
- test: library model: a search, a chip or a header click sorts a held order at once | tests/test_library_model.cpp:1033 | flagged
- test: library model: asking for the search, chip and sort already in place holds the order | tests/test_library_model.cpp:1057 | flagged
- test: library model: new rows leave no held order behind | tests/test_library_model.cpp:1066 | flagged
- test: library model: Best path sorts a Can't analyze row after Not analyzed both ways | tests/test_library_model.cpp:1074 | flagged
- test: library model: set_summaries_for gives every row the answer it was given | tests/test_library_model.cpp:1203 | no record
- test: library model: set_summaries_for holds the row order under the Best path sort until  | tests/test_library_model.cpp:1280 | no record
- test: library model: set_summaries_for with answers that change nothing rebuilds nothing | tests/test_library_model.cpp:1319 | flagged
- test: library model: library_cell_text is what each column shows | tests/test_library_model.cpp:1342 | flagged
- test: library model: only the Best path cell reads the row's summary | tests/test_library_model.cpp:1364 | flagged
- test: library model: rows_version moves when the rows are replaced and at no other time | tests/test_library_model.cpp:1575 | flagged
- test: library model: query_version moves when the search changes and at no other time | tests/test_library_model.cpp:1588 | flagged
- test: a row under an older stamp is left alone by the fill | tests/test_saved_figure_fill.cpp:488 | flagged
- test: a library chart keeps its rows for other chart modes, including Stale ones | tests/test_store_library.cpp:322 | flagged
- test: rows missing the hardest figure: a Ready row with an empty figure and a squeeze-in  | tests/test_store_open.cpp:1398 | no record
- test: the file store's reader gives the writer's answer for every lookup | tests/test_store_records.cpp:270 | flagged
- test: a store opened under other rules reads the same through its reader | tests/test_store_records.cpp:277 | flagged
- test: a reader holds nothing open between reads | tests/test_store_records.cpp:314 | flagged
- test: the reader counts records and library charts as the writer does | tests/test_store_records.cpp:359 | flagged
- test: an own reader gives the writer's answer for every read | tests/test_store_records.cpp:431 | flagged
- test: a hold open at a batch's end loses nothing, and the next batch's end empties the log | tests/test_store_records.cpp:525 | flagged
- test: the log keeps growing while a hold is open | tests/test_store_records.cpp:551 | says something else
- test: an in-memory store's own reader reads through the store | tests/test_store_records.cpp:631 | flagged
- test: store: every stamp this build accepts reads Ready and a retired stamp reads Stale | tests/test_store_records.cpp:1359 | no record
- test: a saved record stores its best path's star count | tests/test_store_results.cpp:648 | flagged
- test: installed_by_setup is false for the test runner and follows the test seam | tests/test_update_release.cpp:224 | says something else
- test: every start of the installed exe checks, and a copy never does | tests/test_update_state.cpp:180 | flagged
- test: a newer release is offered; Later keeps an Update button and writes nothing | tests/test_update_state.cpp:214 | flagged
- test: Skip this version names the release as skipped; a newer one is offered | tests/test_update_state.cpp:254 | flagged
- test: the same or an older release lands in the resting Up to date | tests/test_update_state.cpp:289 | flagged
- test: a failed start-up check shows in the resting status with its plain_error | tests/test_update_state.cpp:310 | flagged
- test: the Updated note's OK offers a found release, or gives way to the resting status | tests/test_update_state.cpp:354 | flagged
- test: Update downloads, checks and starts the installer; Hydra then waits | tests/test_update_state.cpp:400 | flagged
- test: an installer that ends while Hydra runs leaves decision 10's state and the file | tests/test_update_state.cpp:421 | flagged
- test: an installer that can't start leaves decision 10's state with Windows' reason | tests/test_update_state.cpp:444 | flagged
- test: a download whose signature doesn't verify is refused and deleted | tests/test_update_state.cpp:467 | flagged
- test: Cancel during the download goes back to the offer and leaves no part-file | tests/test_update_state.cpp:480 | flagged
- test: the first start after a version change says so once | tests/test_update_state.cpp:519 | flagged
- test: the next start deletes the kept download folder | tests/test_update_state.cpp:549 | flagged
- test: test_open_in_path_builder_greyed | tests/ui/uitest_details.cpp:849 | flagged
- test: test_open_in_path_builder_enabled | tests/ui/uitest_details.cpp:853 | says something else
- test: test_panel_headline | tests/ui/uitest_details.cpp:1019 | flagged
- test: test_version_bar | tests/ui/uitest_library.cpp:1996 | flagged
- test: test_preview_error_wraps | tests/ui/uitest_preview.cpp:896 | no record

- hit: tests/app_state_fixtures.h:196 via tests/test_app_library.cpp:636 | body: tests/test_app_library.cpp:346-355 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)}
- hit: tests/app_state_fixtures.h:196 via tests/test_app_library.cpp:651 | body: tests/test_app_library.cpp:346-355 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)}
- hit: tests/app_state_fixtures.h:196 via tests/test_app_library.cpp:689 | body: tests/test_app_library.cpp:346-355 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)}
- hit: tests/app_state_fixtures.h:196 via tests/test_app_library.cpp:790 | body: tests/test_app_library.cpp:346-355 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)}
- hit: tests/test_app_library.cpp:669 | body: tests/test_app_library.cpp:346-355 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return hydra::app::AnalysisResult{scored_record(int64_t{1000} * turn), hydra::Song(192)}
- hit: tests/test_library_jobs.cpp:465 | body: tests/test_library_jobs.cpp:467-472 | mark: waits, then hands back a test value | note: it returns a value that is not the real function's answer: return r
- hit: tests/test_update_state.cpp:88 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:181 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:215 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:255 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:290 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:311 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:355 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:401 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:422 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:445 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:468 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:481 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:520 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/test_update_state.cpp:550 via tests/test_update_state.cpp:96 | body: tests/test_update_state.cpp:97-104 | mark: can also throw | note: it returns a value that is not the real function's answer: return canned.reply
- hit: tests/ui/uitest_library.cpp:1937 via tests/ui/uitest_library.cpp:1946 | body: tests/ui/uitest_library.cpp:1947-1954 | mark: waits, then hands back a test value; can also throw | note: it returns a value that is not the real function's answer: return reply
- hit: tests/ui/uitest_library.cpp:1938 via tests/ui/uitest_library.cpp:1955 | body: tests/ui/uitest_library.cpp:1957-1962 | mark: waits, then hands back a test value | note: a downloader whose body does more than throw
- hit: tests/ui/uitest_library.cpp:1946 | body: tests/ui/uitest_library.cpp:1947-1954 | mark: waits, then hands back a test value; can also throw | note: it returns a value that is not the real function's answer: return reply
- hit: tests/ui/uitest_library.cpp:1955 | body: tests/ui/uitest_library.cpp:1957-1962 | mark: waits, then hands back a test value | note: a downloader whose body does more than throw
- hit: tests/ui/uitest_library.cpp:2002 via tests/ui/uitest_library.cpp:1946 | body: tests/ui/uitest_library.cpp:1947-1954 | mark: waits, then hands back a test value; can also throw | note: it returns a value that is not the real function's answer: return reply
- hit: tests/ui/uitest_library.cpp:2002 via tests/ui/uitest_library.cpp:1955 | body: tests/ui/uitest_library.cpp:1957-1962 | mark: waits, then hands back a test value | note: a downloader whose body does more than throw

## forced wait

hit rows: 72
hit lines: 68
tests: 71
flagged records: 55

- test: a batch result for the open chart refreshes its row, not the panel | tests/test_app_library.cpp:178 | says something else
- test: a batch result for an open Ready chart changes its row and keeps the panel | tests/test_app_library.cpp:211 | flagged
- test: a batch refresh fills scores in where the rows sit; the batch's end sorts them | tests/test_app_library.cpp:532 | flagged
- test: a batch whose last result was already read still sorts when it ends | tests/test_app_library.cpp:555 | flagged
- test: a batch's end sorts once, and the next batch sorts when it ends | tests/test_app_library.cpp:565 | flagged
- test: a batch dismissed before a tick saw it finish still sorts | tests/test_app_library.cpp:591 | flagged
- test: a tick during a batch does not wait for the store | tests/test_app_library.cpp:615 | flagged
- test: a tick during a batch re-reads no row the batch did not save | tests/test_app_library.cpp:632 | flagged
- test: a batch's end re-reads no row the batch did not save | tests/test_app_library.cpp:644 | says something else
- test: a lookup the batch could not make shows the store's problem | tests/test_app_library.cpp:684 | says something else
- test: a chart in two folders gets both rows from one batch result | tests/test_app_library.cpp:723 | flagged
- test: a failed chart whose failure the batch stored shows on its row, and one it did not  | tests/test_app_library.cpp:755 | no record
- test: a paused batch's stored results reach their rows | tests/test_app_library.cpp:786 | flagged
- test: a stopped batch's stored results reach their rows, then the list sorts | tests/test_app_library.cpp:801 | flagged
- test: a scan cannot start during a batch, and the status line says so | tests/test_app_library.cpp:912 | flagged
- test: any_job_running counts a running batch, and a click's job until tick collects it | tests/test_app_library.cpp:1007 | no record
- test: the startup open runs on its own thread and tick collects the store and library | tests/test_app_library.cpp:1172 | flagged
- test: an AppState closed during its startup open stops the open | tests/test_app_library.cpp:1213 | flagged
- test: a path report request builds it in memory, one build at a time | tests/test_app_reports.cpp:88 | flagged
- test: closing the path report mid-build lets the build finish and land | tests/test_app_reports.cpp:215 | flagged
- test: reopening the path report after a cancelled build shows it cancelled | tests/test_app_reports.cpp:247 | flagged
- test: a setting changed during a build leaves that build out of date | tests/test_app_reports.cpp:334 | flagged
- test: Cancel ends a path report build as cancelled and keeps the last report | tests/test_app_reports.cpp:351 | flagged
- test: opening the path report runs a batch over the charts with no result, then builds | tests/test_app_reports.cpp:509 | flagged
- test: the window's Cancel stops the report's own batch, and no report follows | tests/test_app_reports.cpp:560 | flagged
- test: at the threshold the path report runs its batch without asking | tests/test_app_reports.cpp:583 | no record
- test: more rows to run than the threshold open the batch confirm first | tests/test_app_reports.cpp:606 | flagged
- test: a batch already running is waited on, and its finish builds the report | tests/test_app_reports.cpp:721 | flagged
- test: report_started stays set through a cancel, a refresh and a close, and a new batch clears it | tests/test_app_reports.cpp:773 | no record
- test: a cancelled report closes its connection | tests/test_app_reports.cpp:1052 | flagged
- test: a burst of setting changes ends with one analysis of the final settings | tests/test_app_settings.cpp:120 | flagged
- test: jobs: the batch strip's time left waits for two rounds of its workers | tests/test_library_jobs.cpp:345 | flagged
- test: jobs: time left leaves a pause out of its window | tests/test_library_jobs.cpp:371 | flagged
- test: jobs: a running batch's time left neither collapses after a group nor climbs in a gap | tests/test_library_jobs.cpp:431 | no record
- test: jobs: pause lets the chart in flight finish and starts no new one | tests/test_library_jobs.cpp:546 | flagged
- test: jobs: stop while paused ends the run and fails nothing | tests/test_library_jobs.cpp:575 | flagged
- test: jobs: the snapshot names the chart being analyzed and freezes its clock at the end | tests/test_library_jobs.cpp:597 | flagged
- test: jobs: the progress line's artist reads (unknown) when it is only tags | tests/test_library_jobs.cpp:627 | flagged
- test: jobs: a batch that fails as a whole shows its error and counts no chart | tests/test_library_jobs.cpp:673 | says something else
- test: jobs: the snapshot's counts come from the batch in one piece | tests/test_library_jobs.cpp:702 | flagged
- test: jobs: the tray is empty after it is taken, and a stopped run's tray holds only what it  | tests/test_library_jobs.cpp:922 | no record
- test: jobs: a paused batch job's tray keeps its answers | tests/test_library_jobs.cpp:944 | flagged
- test: jobs: a lookup that fails leaves its sentence in the tray and is tried again | tests/test_library_jobs.cpp:978 | flagged
- test: jobs: a frame's snapshot does not wait while the batch job waits on the store | tests/test_library_jobs.cpp:1040 | flagged
- test: song panel: a running batch locks the settings as a batch, then unlocks | tests/test_song_panel_state.cpp:107 | flagged
- test: test_batch_strip_drift | tests/ui/uitest_batch_reports.cpp:93 | flagged
- test: test_batch_confirm | tests/ui/uitest_batch_reports.cpp:274 | flagged
- test: test_batch_pause_stop | tests/ui/uitest_batch_reports.cpp:313 | flagged
- test: test_batch_strip_workers | tests/ui/uitest_batch_reports.cpp:350 | flagged
- test: test_cap_switch | tests/ui/uitest_details.cpp:152 | flagged
- test: test_stars | tests/ui/uitest_details.cpp:467 | flagged
- test: test_panel_headline | tests/ui/uitest_details.cpp:1019 | flagged
- test: test_panel_close_cancels | tests/ui/uitest_details.cpp:1079 | flagged
- test: test_view_cancel_try_again | tests/ui/uitest_details.cpp:1099 | flagged
- test: test_no_analyze_button | tests/ui/uitest_details.cpp:1167 | flagged
- test: test_view_progress_delay | tests/ui/uitest_details.cpp:1203 | flagged
- test: test_view_setting_burst | tests/ui/uitest_details.cpp:1256 | says something else
- test: test_settings_lock | tests/ui/uitest_details.cpp:1289 | flagged
- test: test_startup_screen | tests/ui/uitest_library.cpp:1561 | says something else
- test: test_startup_library_read_bar | tests/ui/uitest_library.cpp:1617 | flagged
- test: test_rescan_on_open_cancel | tests/ui/uitest_library.cpp:1828 | flagged
- test: test_version_bar | tests/ui/uitest_library.cpp:1996 | flagged
- test: run_live | tests/ui/uitest_live.cpp:263 | no record
- test: test_live_reset_draws_while_opening | tests/ui/uitest_live.cpp:638 | flagged
- test: test_live_reset_open_outlasts_timeout | tests/ui/uitest_live.cpp:653 | flagged
- test: test_live_start_gives_up_at_cap | tests/ui/uitest_live.cpp:733 | flagged
- test: test_analyze_on_preview | tests/ui/uitest_preview.cpp:90 | no record
- test: test_preview_controls | tests/ui/uitest_preview.cpp:228 | no record
- test: test_settings_change_holds_layout | tests/ui/uitest_preview.cpp:1030 | no record
- test: test_open_path | tests/ui/uitest_report_windows.cpp:1195 | flagged
- test: test_hidden_lets_go | tests/ui/uitest_report_windows.cpp:1423 | flagged

- hit: tests/app_state_fixtures.h:165 via tests/app_state_fixtures.h:166 | body: tests/app_state_fixtures.h:169-175 | mark: waits and also calls on_progress | note: it waits, then returns the real function's answer
- hit: tests/app_state_fixtures.h:166 | body: tests/app_state_fixtures.h:169-175 | mark: waits and also calls on_progress | note: it waits, then returns the real function's answer
- hit: tests/test_library_jobs.cpp:485 | body: tests/test_library_jobs.cpp:485-500 | mark: waits and also calls take_reading | note: it waits
- hit: tests/test_song_panel_state.cpp:112 via tests/test_song_panel_state.cpp:113 | body: tests/test_song_panel_state.cpp:115-119 | mark: waits, then throws | note: it waits
- hit: tests/test_song_panel_state.cpp:113 | body: tests/test_song_panel_state.cpp:115-119 | mark: waits, then throws | note: it waits

## forced failure

hit rows: 28
hit lines: 28
tests: 135
flagged records: 113

- test: run_batch: a failed group COMMIT saves each chart alone, and only a chart whose own  | tests/test_analysis.cpp:1478 | no record
- test: a click on a chart with no notes stores the failure once | tests/test_app_click.cpp:797 | flagged
- test: a click whose song file can't be opened stores nothing | tests/test_app_click.cpp:824 | flagged
- test: a failed reader read shows the store's problem | tests/test_app_library.cpp:1694 | says something else
- test: a report whose connection cannot open fails with the store's sentence | tests/test_app_reports.cpp:1013 | says something else
- test: jobs: a scan that throws finishes with its error as a problem | tests/test_library_jobs.cpp:220 | says something else
- test: jobs: a failed leaderboard fetch says what to do | tests/test_library_jobs.cpp:1765 | flagged
- test: closing the Preview frees its render targets; reopening draws again (WARP) | tests/test_preview_controller.cpp:615 | flagged
- test: scrub marks: the Preview's scrubber ends at the song's length | tests/test_preview_controller.cpp:642 | flagged
- test: scrub marks: a stated length past the audio ends playback and the scrubber there | tests/test_preview_controller.cpp:664 | flagged
- test: the Preview's playback, time box and scrubber all end at the song's length | tests/test_preview_controller.cpp:680 | flagged
- test: shows_path: the same path at another cap, and a different path | tests/test_preview_controller.cpp:726 | flagged
- test: set_ghost_path: the ghost's fills land on the scene, and clearing it empties them | tests/test_preview_controller.cpp:767 | flagged
- test: set_ghost_path: a changed chart draws no ghost, as it draws no path | tests/test_preview_controller.cpp:829 | flagged
- test: the Preview hides the path when the chart file changed since its record | tests/test_preview_controller.cpp:1047 | flagged
- test: the Preview trusts the scan's fingerprint and does not re-hash an unchanged chart | tests/test_preview_controller.cpp:1075 | flagged
- test: the Preview's song key: every part changed alone makes a different song | tests/test_preview_controller.cpp:1156 | no record
- test: store: a chart list read on threads whose connection fails to open fails whole | tests/test_store_library.cpp:678 | flagged
- test: a reader that cannot open fails the read and the next read tries again | tests/test_store_records.cpp:330 | flagged
- test: an own reader that cannot open fails the read | tests/test_store_records.cpp:598 | flagged
- test: test_batch_strip_drift | tests/ui/uitest_batch_reports.cpp:93 | flagged
- test: test_settings_and_reports | tests/ui/uitest_batch_reports.cpp:142 | flagged
- test: test_report_buttons | tests/ui/uitest_batch_reports.cpp:177 | flagged
- test: test_batch_confirm_note_shuffle | tests/ui/uitest_batch_reports.cpp:234 | flagged
- test: test_batch_confirm | tests/ui/uitest_batch_reports.cpp:274 | flagged
- test: test_batch_pause_stop | tests/ui/uitest_batch_reports.cpp:313 | flagged
- test: test_batch_strip_workers | tests/ui/uitest_batch_reports.cpp:350 | flagged
- test: test_batch_done_strip | tests/ui/uitest_batch_reports.cpp:425 | flagged
- test: test_batch_open_failure | tests/ui/uitest_batch_reports.cpp:477 | flagged
- test: test_status_line | tests/ui/uitest_batch_reports.cpp:494 | flagged
- test: test_analyze_db_fails | tests/ui/uitest_batch_reports.cpp:520 | flagged
- test: test_compare_disabled | tests/ui/uitest_batch_reports.cpp:545 | flagged
- test: test_dialog_keys | tests/ui/uitest_batch_reports.cpp:572 | flagged
- test: test_batch_chip_count | tests/ui/uitest_batch_reports.cpp:600 | flagged
- test: test_analyze | tests/ui/uitest_details.cpp:106 | flagged
- test: test_cap_switch | tests/ui/uitest_details.cpp:152 | flagged
- test: test_legacy_fills | tests/ui/uitest_details.cpp:198 | flagged
- test: test_dynamics | tests/ui/uitest_details.cpp:249 | flagged
- test: test_dynamics_reopen | tests/ui/uitest_details.cpp:346 | flagged
- test: test_dynamics_hard | tests/ui/uitest_details.cpp:421 | flagged
- test: test_stars | tests/ui/uitest_details.cpp:467 | flagged
- test: test_panel_follows_rescan | tests/ui/uitest_details.cpp:566 | flagged
- test: test_preview_load_bar | tests/ui/uitest_details.cpp:617 | says something else
- test: test_panel_open_close | tests/ui/uitest_details.cpp:655 | flagged
- test: test_panel_split | tests/ui/uitest_details.cpp:686 | says something else
- test: test_panel_hide_library | tests/ui/uitest_details.cpp:750 | flagged
- test: test_open_in_path_builder_greyed | tests/ui/uitest_details.cpp:849 | flagged
- test: test_open_in_path_builder_enabled | tests/ui/uitest_details.cpp:853 | says something else
- test: test_layout_sweep | tests/ui/uitest_details.cpp:879 | flagged
- test: test_long_error_wraps | tests/ui/uitest_details.cpp:957 | says something else
- test: test_panel_prev_next | tests/ui/uitest_details.cpp:999 | flagged
- test: test_panel_headline | tests/ui/uitest_details.cpp:1019 | flagged
- test: test_panel_close_cancels | tests/ui/uitest_details.cpp:1079 | flagged
- test: test_view_cancel_try_again | tests/ui/uitest_details.cpp:1099 | flagged
- test: test_view_rules_broken | tests/ui/uitest_details.cpp:1134 | says something else
- test: test_no_analyze_button | tests/ui/uitest_details.cpp:1167 | flagged
- test: test_view_progress_delay | tests/ui/uitest_details.cpp:1203 | flagged
- test: test_view_setting_burst | tests/ui/uitest_details.cpp:1256 | says something else
- test: test_settings_lock | tests/ui/uitest_details.cpp:1289 | flagged
- test: test_scan | tests/ui/uitest_library.cpp:67 | flagged
- test: test_library_twin_rows | tests/ui/uitest_library.cpp:86 | says something else
- test: test_difficulty | tests/ui/uitest_library.cpp:174 | flagged
- test: test_note_shuffle_switch | tests/ui/uitest_library.cpp:246 | flagged
- test: test_settings_button | tests/ui/uitest_library.cpp:318 | flagged
- test: test_engine_wildcard_patches | tests/ui/uitest_library.cpp:347 | says something else
- test: test_settings_reset | tests/ui/uitest_library.cpp:364 | flagged
- test: test_settings_early_fill | tests/ui/uitest_library.cpp:409 | flagged
- test: test_rules_error | tests/ui/uitest_library.cpp:462 | flagged
- test: test_settings_error | tests/ui/uitest_library.cpp:500 | flagged
- test: test_library_state_per_app | tests/ui/uitest_library.cpp:532 | flagged
- test: test_view_settings | tests/ui/uitest_library.cpp:556 | flagged
- test: test_library_search | tests/ui/uitest_library.cpp:641 | flagged
- test: test_analyze_follows_chip | tests/ui/uitest_library.cpp:841 | flagged
- test: test_library_cant_analyze | tests/ui/uitest_library.cpp:891 | says something else
- test: test_library_sort_scroll | tests/ui/uitest_library.cpp:950 | flagged
- test: test_library_layout | tests/ui/uitest_library.cpp:1008 | flagged
- test: test_library_column_widths | tests/ui/uitest_library.cpp:1085 | flagged
- test: test_library_widths_hold | tests/ui/uitest_library.cpp:1163 | flagged
- test: test_library_click_keeps_row_in_place | tests/ui/uitest_library.cpp:1273 | flagged
- test: test_library_next_scrolls_just_enough | tests/ui/uitest_library.cpp:1337 | flagged
- test: test_library_column_order | tests/ui/uitest_library.cpp:1401 | flagged
- test: test_library_columns_at_start | tests/ui/uitest_library.cpp:1441 | says something else
- test: test_startup_screen | tests/ui/uitest_library.cpp:1561 | says something else
- test: test_startup_library_read_bar | tests/ui/uitest_library.cpp:1617 | flagged
- test: test_old_library_set_aside | tests/ui/uitest_library.cpp:1687 | flagged
- test: test_rescan_on_open | tests/ui/uitest_library.cpp:1765 | flagged
- test: test_rescan_on_open_cancel | tests/ui/uitest_library.cpp:1828 | flagged
- test: test_scan_missing_folder | tests/ui/uitest_library.cpp:1864 | flagged
- test: test_version_bar | tests/ui/uitest_library.cpp:1996 | flagged
- test: test_library_search_tips | tests/ui/uitest_library.cpp:2224 | flagged
- test: test_reset_app_blocks | tests/ui/uitest_live.cpp:721 | flagged
- test: test_paths_list | tests/ui/uitest_paths.cpp:153 | flagged
- test: test_paths_rows | tests/ui/uitest_paths.cpp:364 | flagged
- test: test_paths_backend_timings | tests/ui/uitest_paths.cpp:482 | flagged
- test: test_paths_folds_copy | tests/ui/uitest_paths.cpp:578 | flagged
- test: test_paths_uncounted | tests/ui/uitest_paths.cpp:667 | flagged
- test: test_paths_fit_narrow | tests/ui/uitest_paths.cpp:721 | flagged
- test: test_paths_long_path | tests/ui/uitest_paths.cpp:752 | flagged
- test: test_paths_backend_fit | tests/ui/uitest_paths.cpp:821 | flagged
- test: test_paths_row_layout | tests/ui/uitest_paths.cpp:968 | says something else
- test: test_paths_length_from_click | tests/ui/uitest_paths.cpp:1153 | flagged
- test: test_paths_hit_window | tests/ui/uitest_paths.cpp:1249 | flagged
- test: test_paths_hit_window_narrow | tests/ui/uitest_paths.cpp:1338 | flagged
- test: test_preview | tests/ui/uitest_preview.cpp:34 | no record
- test: test_preview_path_overlay | tests/ui/uitest_preview.cpp:151 | flagged
- test: test_preview_solo_box | tests/ui/uitest_preview.cpp:377 | no record
- test: test_preview_squeeze_out_gauge | tests/ui/uitest_preview.cpp:537 | no record
- test: test_preview_path_picker | tests/ui/uitest_preview.cpp:802 | flagged
- test: test_preview_mode_reload | tests/ui/uitest_preview.cpp:957 | no record
- test: test_settings_change_holds_layout | tests/ui/uitest_preview.cpp:1030 | no record
- test: test_preview_overlay_steady | tests/ui/uitest_preview.cpp:1094 | flagged
- test: test_preview_activation_jumps | tests/ui/uitest_preview.cpp:1133 | flagged
- test: test_sort | tests/ui/uitest_report_windows.cpp:316 | flagged
- test: test_filters | tests/ui/uitest_report_windows.cpp:345 | flagged
- test: test_states | tests/ui/uitest_report_windows.cpp:372 | flagged
- test: test_keys | tests/ui/uitest_report_windows.cpp:539 | flagged
- test: test_close_button | tests/ui/uitest_report_windows.cpp:581 | flagged
- test: test_hidden_new_result | tests/ui/uitest_report_windows.cpp:635 | flagged
- test: test_both | tests/ui/uitest_report_windows.cpp:660 | flagged
- test: test_chip_label | tests/ui/uitest_report_windows.cpp:680 | flagged
- test: test_chip_outline | tests/ui/uitest_report_windows.cpp:719 | flagged
- test: test_column_widths | tests/ui/uitest_report_windows.cpp:739 | flagged
- test: test_cells_left | tests/ui/uitest_report_windows.cpp:804 | flagged
- test: test_header_narrow | tests/ui/uitest_report_windows.cpp:872 | flagged
- test: test_fits_screen | tests/ui/uitest_report_windows.cpp:955 | flagged
- test: test_open_asks | tests/ui/uitest_report_windows.cpp:1123 | flagged
- test: test_open_path | tests/ui/uitest_report_windows.cpp:1195 | flagged
- test: test_open_analyzes | tests/ui/uitest_report_windows.cpp:1256 | flagged
- test: test_cant_analyze_list | tests/ui/uitest_report_windows.cpp:1302 | flagged
- test: test_row_click | tests/ui/uitest_report_windows.cpp:1331 | flagged
- test: test_reopen | tests/ui/uitest_report_windows.cpp:1395 | flagged
- test: test_hidden_lets_go | tests/ui/uitest_report_windows.cpp:1423 | flagged
- test: test_reopen_keeps_filters | tests/ui/uitest_report_windows.cpp:1466 | flagged
- test: test_fresh_state | tests/ui/uitest_report_windows.cpp:1546 | flagged
- test: test_out_of_date | tests/ui/uitest_report_windows.cpp:1589 | flagged

## test code run mid-step

hit rows: 11
hit lines: 11
tests: 26
flagged records: 19

- test: a batch result for the open chart refreshes its row, not the panel | tests/test_app_library.cpp:178 | says something else
- test: a batch result for an open Ready chart changes its row and keeps the panel | tests/test_app_library.cpp:211 | flagged
- test: a batch refresh fills scores in where the rows sit; the batch's end sorts them | tests/test_app_library.cpp:532 | flagged
- test: a batch whose last result was already read still sorts when it ends | tests/test_app_library.cpp:555 | flagged
- test: a batch's end sorts once, and the next batch sorts when it ends | tests/test_app_library.cpp:565 | flagged
- test: a batch dismissed before a tick saw it finish still sorts | tests/test_app_library.cpp:591 | flagged
- test: a tick during a batch does not wait for the store | tests/test_app_library.cpp:615 | flagged
- test: a tick during a batch re-reads no row the batch did not save | tests/test_app_library.cpp:632 | flagged
- test: a batch's end re-reads no row the batch did not save | tests/test_app_library.cpp:644 | says something else
- test: a lookup the batch could not make shows the store's problem | tests/test_app_library.cpp:684 | says something else
- test: a chart in two folders gets both rows from one batch result | tests/test_app_library.cpp:723 | flagged
- test: a failed chart whose failure the batch stored shows on its row, and one it did not  | tests/test_app_library.cpp:755 | no record
- test: a paused batch's stored results reach their rows | tests/test_app_library.cpp:786 | flagged
- test: a stopped batch's stored results reach their rows, then the list sorts | tests/test_app_library.cpp:801 | flagged
- test: a scan cannot start during a batch, and the status line says so | tests/test_app_library.cpp:912 | flagged
- test: any_job_running counts a running batch, and a click's job until tick collects it | tests/test_app_library.cpp:1007 | no record
- test: a library read that fails after the startup open is the status line's problem | tests/test_app_library.cpp:1230 | says something else
- test: a batch already running is waited on, and its finish builds the report | tests/test_app_reports.cpp:721 | flagged
- test: a path report reads one moment | tests/test_app_reports.cpp:951 | flagged
- test: a path report's separate store calls read one moment | tests/test_app_reports.cpp:979 | flagged
- test: jobs: a chart in two folders is asked about once | tests/test_library_jobs.cpp:842 | flagged
- test: jobs: a lookup that fails leaves its sentence in the tray and is tried again | tests/test_library_jobs.cpp:978 | flagged
- test: jobs: the startup open reads saved paths with its own words, then the library | tests/test_library_jobs.cpp:1248 | flagged
- test: jobs: a fill that fails still opens the library and leaves its sentence for the  | tests/test_library_jobs.cpp:1317 | no record
- test: jobs: a stop while the startup open reads saved paths ends it as a cancel | tests/test_library_jobs.cpp:1392 | flagged
- test: a reader's lookup reads one moment | tests/test_store_records.cpp:287 | flagged

## replaces nothing

hit rows: 18
hit lines: 18
tests: 15
flagged records: 11

- test: a path report reads one moment | tests/test_app_reports.cpp:951 | flagged
- test: a path report's separate store calls read one moment | tests/test_app_reports.cpp:979 | flagged
- test: http transport: cancel aborts a request the server never answers | tests/test_dm_report.cpp:1285 | flagged
- test: http transport: cancel aborts a body that stops after the headers | tests/test_dm_report.cpp:1303 | flagged
- test: http transport: a 200 reply is read in full, a 404 keeps its message | tests/test_dm_report.cpp:1335 | says something else
- test: jobs: a failed chart reads in plain words and keeps the raw text | tests/test_library_jobs.cpp:646 | flagged
- test: jobs: answered_count reaches the number of charts saved, and counts a chart in two  | tests/test_library_jobs.cpp:807 | no record
- test: jobs: a batch job's tray holds the store's answer for each chart it saved | tests/test_library_jobs.cpp:825 | flagged
- test: jobs: a chart in two folders is asked about once | tests/test_library_jobs.cpp:842 | flagged
- test: jobs: a failed chart whose failure was stored reaches the tray as the store's answer | tests/test_library_jobs.cpp:872 | says something else
- test: jobs: a cancelled leaderboard fetch is not an error | tests/test_library_jobs.cpp:1752 | says something else
- test: jobs: a failed leaderboard fetch says what to do | tests/test_library_jobs.cpp:1765 | flagged
- test: every open caps the WAL file the log keeps after a checkpoint | tests/test_store_open.cpp:705 | flagged
- test: the batch guard sets the checkpoint threshold and truncates the log at the end | tests/test_store_open.cpp:1342 | flagged
- test: a reader's lookup reads one moment | tests/test_store_records.cpp:287 | flagged

## unsorted

hit rows: 13
hit lines: 10
tests: 29
flagged records: 24

- test: a click's save does not stop the window while a save group is open | tests/test_app_click.cpp:554 | flagged
- test: a click's worker writes a changed chart's hash before it saves | tests/test_app_click.cpp:583 | flagged
- test: a click cancelled before its save saves nothing | tests/test_app_click.cpp:612 | flagged
- test: a click closed before its save saves nothing | tests/test_app_click.cpp:626 | flagged
- test: a click replaced before its save saves nothing | tests/test_app_click.cpp:641 | flagged
- test: a click cancelled during its save keeps the panel cancelled and gives the row the store's answer | tests/test_app_click.cpp:702 | flagged
- test: a click closed during its save gives the row the store's answer | tests/test_app_click.cpp:713 | flagged
- test: a click replaced during its save gives the row the store's answer | tests/test_app_click.cpp:725 | flagged
- test: a dropped click that wrote a new hash still reloads the library | tests/test_app_click.cpp:744 | flagged
- test: a dropped click's save marks both reports out of date | tests/test_app_click.cpp:763 | flagged
- test: a batch result for the open chart refreshes its row, not the panel | tests/test_app_library.cpp:178 | says something else
- test: a batch result for an open Ready chart changes its row and keeps the panel | tests/test_app_library.cpp:211 | flagged
- test: a batch refresh fills scores in where the rows sit; the batch's end sorts them | tests/test_app_library.cpp:532 | flagged
- test: a batch whose last result was already read still sorts when it ends | tests/test_app_library.cpp:555 | flagged
- test: a batch's end sorts once, and the next batch sorts when it ends | tests/test_app_library.cpp:565 | flagged
- test: a batch dismissed before a tick saw it finish still sorts | tests/test_app_library.cpp:591 | flagged
- test: a tick during a batch does not wait for the store | tests/test_app_library.cpp:615 | flagged
- test: a tick during a batch re-reads no row the batch did not save | tests/test_app_library.cpp:632 | flagged
- test: a batch's end re-reads no row the batch did not save | tests/test_app_library.cpp:644 | says something else
- test: a lookup the batch could not make shows the store's problem | tests/test_app_library.cpp:684 | says something else
- test: a chart in two folders gets both rows from one batch result | tests/test_app_library.cpp:723 | flagged
- test: a failed chart whose failure the batch stored shows on its row, and one it did not  | tests/test_app_library.cpp:755 | no record
- test: a paused batch's stored results reach their rows | tests/test_app_library.cpp:786 | flagged
- test: a stopped batch's stored results reach their rows, then the list sorts | tests/test_app_library.cpp:801 | flagged
- test: a scan cannot start during a batch, and the status line says so | tests/test_app_library.cpp:912 | flagged
- test: any_job_running counts a running batch, and a click's job until tick collects it | tests/test_app_library.cpp:1007 | no record
- test: a batch already running is waited on, and its finish builds the report | tests/test_app_reports.cpp:721 | flagged
- test: a mode change on the open chart reloads only the notes and keeps the playhead | tests/test_preview_controller.cpp:888 | flagged
- test: a mode with no notes pauses the Preview in place; switching back clears it | tests/test_preview_controller.cpp:950 | flagged

- hit: tests/app_state_fixtures.h:196 via tests/test_app_library.cpp:763 | body: tests/test_app_library.cpp:766-772 | note: it throws and does more, with no wait
- hit: tests/test_app_click.cpp:494 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:503 via tests/test_app_click.cpp:495 | body: none read | note: the member's initialiser gives it set_seam, not a hook word
- hit: tests/test_app_click.cpp:521 via tests/test_app_click.cpp:494 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:521 via tests/test_app_click.cpp:495 | body: none read | note: the member's initialiser gives it set_seam, not a hook word
- hit: tests/test_app_click.cpp:533 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:540 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:562 via tests/test_app_click.cpp:494 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:562 via tests/test_app_click.cpp:495 | body: none read | note: the member's initialiser gives it set_seam, not a hook word
- hit: tests/test_app_click.cpp:594 via tests/test_app_click.cpp:494 | body: none read | note: the hook word is passed as an argument, not called
- hit: tests/test_app_click.cpp:594 via tests/test_app_click.cpp:495 | body: none read | note: the member's initialiser gives it set_seam, not a hook word
- hit: tests/test_preview_controller.cpp:891 | body: tests/test_preview_controller.cpp:862-865 | note: it returns the answer of a name the lambda captured: return fail(channels, sample_rate, std::move(source))
- hit: tests/test_preview_controller.cpp:953 | body: tests/test_preview_controller.cpp:862-865 | note: it returns the answer of a name the lambda captured: return fail(channels, sample_rate, std::move(source))
