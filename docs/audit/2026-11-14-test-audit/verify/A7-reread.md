# Verify A7, re-read line by line: ten records

Verifier: agent afd9b69d406393613 (task T5-V-R3b). Read-only; nothing run. The round 2 verifier of A7 agreed with these records without reading their code line by line. I read the code behind each of the ten. Every mark is "by reading". Nothing is "proven by": no run of these tests exists in the records folders.

Records re-read: 10 of 10 (`tests/test_app_reports.cpp` lines 344, 608, 631, 736, 761, 777; `tests/test_app_shell.cpp` lines 247, 286, 350, 408). Verdicts I agree with: 9. Verdicts I dispute: 1 (record 44, line 408). Of the nine I agree with, one has a wrong Q8 claim inside it (record 35, line 247), and one (record 17, line 608) has a missing check I name. The six report records hold line by line.

Question 3 for all ten: the record says "claim not checked" because `records-screen/index.md` lists the path report window as not captured. I confirmed that line (`records-screen/index.md:66`: "the path report and the Path Builder" under "What is not captured"). The records name the capture needed. That rules Keep out; the verdicts are Investigate or Rewrite, which is right.

What I read for this file, beyond the test lines: `src/ui/app_state.cpp` 690-789, 828-1076 (`start_batch`, `update_background_jobs`, `settle_report_wait`, `request_path_report`, `start_path_report_batch`, `decline_report_batch`, `launch_path_report`, `cancel_path_report`, `show_path_report`, `close_path_report`, `path_report_offered`, `land_report`, `collect_path_report`); `src/ui/app_state.h:237-240` (`mark_cancelled`, which the A7 auditor did not read) and 319; `src/ui/library_jobs.cpp:440-445` (`BatchJob::stop`) and 641-686 (`ReportJob`, hook taken at construction); `src/app/job_base.h:40`; `tests/scoped_hook.h`; `tests/test_app_reports.cpp` 1-80, 520-562; `tests/app_state_fixtures.h:65-126`; `src/ui/app_shell.cpp` 60-344; `src/ui/app_shell.h:94-99`; `src/ui/library_view.cpp` 28-83; `docs/audit/2026-10-03-fix-decisions.md` line 1019 (D249 item 8, quoted below). Not read: `wait_batch_finished` and `start_redo_batch` (named in the tests, defined in a helper header I did not open), `run_guarded`, `app::plan_batch`.

---

## test_app_reports.cpp:344 "Cancel ends a path report build as cancelled and keeps the last report" (record 11). Agree: Investigate

I traced it. Lines 348-351 build a report and keep its pointer. Line 356 starts a second build under a held report job, so the job waits on `release`. The hook is taken when the job is constructed (`library_jobs.cpp:648`) and the `ScopedHook` is cleared at the end of the block, so there is no race. Line 358: `cancel_path_report` calls `report_job->cancel()` (`app_state.cpp:966`) and then returns at line 967 because `report_wait_` is `None`; it never reaches `mark_cancelled`. So the state at 361 comes from the job's landing: `collect_path_report` runs `land_report`, which for `JobOutcome::Cancelled` calls `slot.mark_cancelled()` (`app_state.cpp:1025-1027`). `mark_cancelled` (`app_state.h:237-240`) sets `last = Cancelled` and clears the during-build mark and touches nothing else. So 361 (Cancelled) and 362 (the rows pointer unchanged) both hold, and the record's owner list is right (it did not read `mark_cancelled`; I did).

Q8 per assertion. 361: if the `cancel()` at `app_state.cpp:966` were removed, the held job would finish normally and the state would be Ready, so 361 goes red (reasoned). 362: if a Cancelled landing replaced `slot.result`, 362 goes red (reasoned). Both can fail.

## test_app_reports.cpp:608 "the window's Cancel stops the report's own batch, and no report follows" (record 17). Agree: Investigate, one gap added

Traced. `show_path_report` with five charts waiting: the threshold is 500, so `request_path_report` starts the batch itself (`app_state.cpp:931`, `start_path_report_batch`, `start_batch(false)`, then `settle_report_wait` sets `report_wait_ = OwnBatch`). `cancel_path_report` stops the batch (`app_state.cpp:970`), clears the wait and marks Cancelled. 619: `BatchJob::stop` sets the cancel flag (`library_jobs.cpp:441`) and `is_cancelled` reads it (`job_base.h:40`), so 619 holds. 620 holds. After release, `update_background_jobs` sees the finish, reads `stopped` true and skips the report (`app_state.cpp:764-766`). 624 and 625 hold.

Q8 per assertion: 619 goes red if the `stop()` at line 970 is removed. 620 goes red if `mark_cancelled` at 972 is removed. 624 goes red if the `!stopped` guard at 766 is removed (a report would launch). 625 adds little beyond 620 and 624.

Gap the record did not name. Line 970 reads `if (report_wait_ == ReportWait::OwnBatch && batch_job) batch_job->stop();`, and the comment above it says "A batch the user started is the user's to stop". No test cancels while the report waits on the user's batch. I searched `tests/` for `cancel_path_report` and found four callers only: `test_app_reports.cpp:249, 358, 618, 827`. At 249 and 827 the cancel is of a report job, and at 358 and 618 the cases above. If someone dropped the `OwnBatch` condition, so that Cancel also stopped a user's batch, no test in `tests/` goes red. This is a missing check, not a fault in the record.

## test_app_reports.cpp:631 "more rows to run than the threshold open the batch confirm first" (record 18). Agree: Investigate (split into one test per step)

I traced all six steps against `request_path_report`, `start_path_report_batch`, `decline_report_batch`, `close_path_report`, `open_batch_confirm` (`app_state.cpp:692`) and `settle_report_wait`. The test's expectations hold at each step.

Step one (638-650): threshold 5 equals the five charts to run, and `todo_rows() <= report_confirm_above` (`app_state.cpp:931`) runs the batch at once, so 644 and 645 hold. If `<=` became `<`, the window would ask, `batch_job` stays null, and 644 goes red.

Step two (654-666): threshold 4, so 5 > 4, the window asks, nothing runs (656), `path_report_build()` reads Building because `report_wait_` is not `None` (`app_state.cpp:857`) (660).

Step three (669-684): Analyze starts the batch, which the report waits on. Step four (689-698): Cancel calls `decline_report_batch`, which closes the plan, and `settle_report_wait` launches the report from what is stored (`app_state.cpp:874`), so 694 holds. Step five (703-717): `close_path_report` closes the plan and ends the wait; reopening finds the rows and asks nothing. Step six (722-731): `open_batch_confirm` calls `decline_report_batch` first (`app_state.cpp:692`) and then opens the user's own popup, so 725-728 hold.

The record's Q5 is right and needs two additions. First, line 632 `CHECK(AppState::kReportConfirmAbove == 500)` has no D-number beside it; the comment at 628 says "The user's answer 8" and names no decision. D249 item 8 (`fix-decisions.md:1019`) reads: "when more than 500 charts need analyzing, the report shows the normal batch confirm first ("Analyze N charts that have no result yet?"); at 500 or fewer it just runs. The 500 is the number the question offered and the user chose." So the number is a rule number with a real source, but the test does not write the source where it types the number. Second, D249 says "charts" while the code counts library rows, each copy of a chart counted (`app_state.cpp:719-721`, D76); that is the same split the code comment at `app_state.cpp:718-719` already names, not a new one.

Line 663 types the question text ("Analyze 5 charts that have no result yet?"). Line 664 compares the same string with `batch_question(false)`, the owner. So 663 is the display's words typed, 664 is the owner's answer; the record is right.

## test_app_reports.cpp:736 "closing the path report while it asks ends the question, and reopening asks again" (record 19). Agree: Investigate, one weak line

Traced. Threshold 4, five charts, `show_path_report` asks (Confirm wait, plan open). `close_path_report` hides the window, sees the question open, closes the plan and clears the wait (`app_state.cpp:981-992`), so 747-753 hold: `path_report_build()` reads `path_report.last`, which is the default `None`. Reopening: no rows and build `None`, so `request_path_report(Open)` asks again (755-758).

Q8: 748 goes red if `report_wait_ = None` (line 991) is removed. 749 goes red if `close_batch_confirm()` (line 990) is removed. 751-753 would go red if the close started a batch or a report. 757 (the question text the same as before) can fail only if the plan changed between close and reopen, and nothing in `close_path_report` changes a plan; 756 (asks again) and 758 (nothing runs) already carry the behaviour. So 757 is nearly redundant.

D276 item 6, quoted in the record from `fix-decisions.md`, matches the code comment at `app_state.cpp:986-988`.

## test_app_reports.cpp:761 "a batch already running is waited on, and its finish builds the report" (record 20). Agree: Investigate

Traced, with `start_redo_batch` and `wait_batch_finished` taken as named by the test (not read). `show_path_report` with a batch running: `request_path_report` takes the `batch_job && !batch_finish_seen_` branch (`app_state.cpp:897-901`) and sets `report_wait_ = Batch` and the cause `Open`, with no report job. So 767, 768 and 769 hold. After release, `update_background_jobs` sees the finish, `waited` is true, and the report launches with the saved cause `Open` (`app_state.cpp:772-773`). 774 (`built_on_open`) is set only when the build ends Ready and the cause is `Open` (`app_state.cpp:1045-1046`), so 774 also proves the report ended Ready, which the record's Q8 does not say.

Q8: 774 goes red if the saved cause were lost (`waited ? report_wait_cause_ : Batch`, line 773). 767 and 768 go red if the wait branch were removed. The record's "report lists half the library" failure is not asserted by any line; the test checks no row count.

## test_app_reports.cpp:777 "the toolbar offers the path report whenever the library has rows" (record 21). Agree: Investigate

`path_report_offered()` is `!library.rows().empty() && !analysis_blocked()` (`app_state.cpp:994-996`; `analysis_blocked` is at `app_state.h:319`). Callers in `src/`: `path_report_button_shown` (`app_state.cpp:1003`), nothing else; the toolbar calls `path_report_button_shown` (`library_toolbar.cpp:129`), not `path_report_offered`. So the test reads the owner's function; the button the user sees also needs `store_ready()` and the other two conditions in `path_report_button_shown` (`app_state.cpp:1002-1003`). That matches the record's Q6.

779 uses `app_on`, which loads a hand-built seeded library (`tests/app_state_fixtures.h:65-73`), not a corpus chart. For a state claim this is fine. 781 uses `app_with(empty, {})`, whose library is empty. Q8: 779 goes red if the `!rows.empty()` test were inverted, and so does 781. The name leaves out the `analysis_blocked` half, as the record says; no assertion sets it.

---

## test_app_shell.cpp:247 "with no frame and no monitor list a report keeps the main window's rectangle" (record 35). Agree with Investigate; one Q8 claim is wrong

Traced `report_placement` (`app_shell.cpp:258-284`). First call: no saved rectangle, the main window maximized, one monitor `{0,0,1280,800}` equal to the main client, zero frame. `main_area` is that monitor; `max_area = main_area`; the result is `shrunk(*main_area, zero)`, which is `{0,0,1280,800}`, with maximized true. Second call: no monitors, so `area` is null and line 282 returns `{candidate, false}` where `candidate` is `main_client`. So 253, 254, 259 and 260 all hold.

Wrong claim in the record. Its Q8 says 253 catches a change to the `max_area ? shrunk : main_client` branch (`app_shell.cpp:279`). In this fixture `main_client`, the work area and the shrunk work area are all the same rectangle, `{0,0,1280,800}`, because the frame is zero. So 253 passes whether the code returns `shrunk(*max_area, frame)`, `*max_area` or `main_client`. It cannot catch a fault in that expression, and it does not test the frame at all, though the comment above it says it is "the case for the frame". Only 254 (maximized true) and 259 and 260 can fail. 259 does go red if the no-monitor branch (282) is removed, because the code would dereference a null `area`.

## test_app_shell.cpp:286 "hydra_ui.ini keeps each report's placement in its own section" (record 38). Agree: Rewrite (line 322)

Traced. The fixture ini holds a main window section and a `[Hydra][Window:pathreport]` section. `placement_read_open` (`app_shell.cpp:76-84`) hands the key `pathreport` its own entry, and `parse_window_placement_line` (149-163) builds `{8,31,2552,1384}` from Pos then Size, so 298 holds. 300 holds (no `dmreport` entry gives a default, invalid placement). 302 holds (`{100,100,1380,820}`). 309: `remember_report_placement` returns early when the new value equals the stored one (`app_shell.cpp:210`), so the dirty timer stays clear. 314: a new key is inserted and `mark_ini_dirty` runs. The writer (`placement_write_all`, 93-110) writes the main section first, then each report in key order (`dmreport` before `pathreport`), then the layout; the test looks for each chunk by `find`, so the order does not matter. 320-323 hold: `{2600,60,4400,1000}` gives `Pos=2600,60` and `Size=1800,940`.

Q5/Q4: the record is right that 322 types `1800,940` and the working (4400 - 2600, 1000 - 60) is absent. The rewrite (write the size as right minus left, bottom minus top) is the rubric's "rule applied to named inputs" form.

Q8 per assertion: 300 goes red if `report_window_placement` returned a valid default; 302 if the main section were mixed with the report's; 309 if the equality early return at 210 were removed; 314 if `mark_ini_dirty` at 212 were removed; 320-323 if the writer dropped a section or a field. The test reads ImGui's private `SettingsDirtyTimer` (`imgui_internal.h`), as the record says.

## test_app_shell.cpp:350 "hydra_ui.ini remembers the window placement" (record 41). Agree: Rewrite (line 374)

Traced. The fixture ini `Pos=-1700,40 / Size=1400,900 / Maximized=1` parses to `{-1700,40,-300,940}` (right = -1700 + 1400, bottom = 40 + 900), so 361 and 362 hold. `remember_window_placement` stores `{200,150,1480,870}` and marks the ini dirty (`app_shell.cpp:167-171`); `shutdown_imgui` writes the ini, and the text `Pos=200,150 / Size=1280,720 / Maximized=0` is right minus left (1480 - 200) and bottom minus top (870 - 150), so 374 holds with the sizes typed as raw results, as the record says. The test has no `ImGuiOnIni` guard, and it has no REQUIRE, so no check can abandon it before `shutdown_imgui`; the record's sentence is clumsy but its conclusion holds.

## test_app_shell.cpp:408 "hydra_ui.ini remembers the library split, not the child's own width" (record 44). DISPUTE: the claim and the rewrite both miss the real owner

Traced the round trip. 424, 425 and 431 hold: `remember_library_share(0.25f)` and `remember_library_hidden(true)` store in `g_layout`, `placement_write_all` writes `[Hydra][Layout]` with `LibraryShare=0.2500` (`%.4f`, `app_shell.cpp:288`) and `LibraryHidden=1`, and the second `setup_imgui` reads them back. That half is a genuine round trip, close to record 42's text round trip.

The other half does not do what the name says. Lines 412-415 write a `[Window][Hydra/##library_06E76512]` section with a size, and 417 asserts `library_share()` equals `kDefaultLibraryShare`. But `library_share()` just returns `g_layout.library_share` (`app_shell.cpp:313`), and the only handler that reads the file for that value is the `Hydra` handler, which never sees a `[Window]` section. So 417 cannot go red for any change to `app_shell.cpp`. What actually keeps the child's width out of the share is in the view: the library child is created with `ImGuiWindowFlags_NoSavedSettings` and the share changes only on a drag (`library_view.cpp:36-40` comment, `library_view.cpp:67-72`). That is a different file, and no assertion here touches it. With `NoSavedSettings` ImGui neither reads nor writes a `[Window]` section for that child, so the fixture's section is already inert today.

So the record's Q8 line "417 reading the child's size as share" is not a bug this test can catch. The record found the typed hash as a stand-in; the stronger fault is that the claim is about `library_view.cpp:68`, and this test checks `library_share()`, a getter. The record's rewrite (build the section name from the child's name) would keep an assertion that cannot fail. The verdict should not be that Rewrite. It is Investigate, with the finding stated: the half of the claim about the child's width is owned by the view's `NoSavedSettings` flag, which only a frame-level GUI test (`hydra_uitest`) can check; the round trip half could stay as a Rewrite of its own.

Split: none new.

---

Golden files: no record here proposes a saved expected-output file.
