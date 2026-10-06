Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task ER2: the screens and the batch show the kind's sentence, and the text matcher goes (finding 193 and R7.20, part 2 of 2)

Task id: ER2. Base: **ER1's commit**. ER1 forks from main after AL2 merges, and the main session names both hashes. You read ER1's `ErrorKind` and its switch in `plain_error`. Branch: claude/p7-er2 (worktree `.claude\worktrees\p7-er2`, made as the preamble says). Session for your trailers: 6fc5f64c-7b23-4264-912b-0b78bf5cec9d.

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, "Wave 4: errors carry their kind", split by folder under D59 item 4. Read `p7-er1.md` first: it explains the kinds, the owner and what ER1 left for you. Decision: D51 call 25 ("Errors carry their kind, and each kind has one plain sentence. A Preview asset problem reads 'Reinstall Hydra'"). Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 193.` and `#### R7.20`.

## Goal

Every screen that shows a failure shows its kind's sentence, from `app::plain_error`, with the raw text on a details line where the screen has room for one. The batch hands each failure over with its sentence already worked out, so nothing reads an error's words any more. The text matcher, `is_no_notes_message` and `is_timing_refusal` are deleted. A failed save during a batch becomes a failed chart instead of closing Hydra. No score, path or stored record changes; the results stamp stays "2.1.0".

## What the code does today

Line numbers are from main at e4833e0. AL2 moves lines in `analysis.cpp`, `library_jobs.cpp` and `app_state.cpp`.

**The app-folder and UI throwers, still untyped after ER1.** In `src/app/analysis.cpp`, `stream_md5` and its helpers throw "cannot open file: " and "MD5 hashing failed", and line 64 throws "BCryptOpenAlgorithmProvider(MD5) failed". `write_report_file` (`report_files.cpp`, lines 149, 155 and 163) throws "cannot write ". `RulesFileError` (`rules_file.h`) is matched by type. `collect_dm_rows` (`dm_report.cpp`, line 204) throws the rules gate's own sentence as a `std::invalid_argument`. In `src/ui`: `JobCancelled` (`job_base.h`) reads "cancelled", `DmReportJob` throws "this user has no scores to compare" (`dm_jobs.cpp`, line 52), and `ReportJob` throws `report.why_empty` or "no records stored yet" (`library_jobs.cpp`, lines 381 and 382).

**The batch hands over text.** `run_batch`'s worker keeps only `e.what()` (`analysis.cpp`, line 638). `BatchCallbacks::on_error` passes a title and that text (`analysis.h`, line 199). `BatchJob` turns it back into a sentence with `plain_error_text` (`library_jobs.cpp`, line 284), so the type is already lost. hydra_batch prints the raw text (`cli/batch.cpp`, line 229).

**A failed batch save closes Hydra.** `run_batch` calls `store.save_analysis` on the consuming thread with no catch (line 652). `run_work_pool` rethrows a consumer's exception (`work_pool.h`, line 98). `BatchJob::run` has no catch around `run_batch`, so the exception leaves the job's thread and the program ends.

**The Preview.** `PreviewController::poll` sets `error_` to `plain_error_text(job_->error())` (`preview_controller.cpp`, line 216). That works the sentence out again from text, though the job already holds it as `message()`. A failed overlay build stores the raw `scene_job_->error()` (line 159), and a renderer failure stores the raw `e.what()` (line 260). That covers the missing-asset throws, and the Direct3D failures from `check`. `preview_tab.cpp` (line 295) prints "Preview failed: %s" with no details line. So a missing or damaged Preview asset shows raw program text: finding 193's main point.

**The Dynamics tab.** `dynamics_tab.cpp` (line 98) prints "Dynamics failed: " plus the job's raw `error()`. `AppState::reap_dynamics` (`app_state.cpp`, line 445) builds "Counted, but saving failed: " plus the raw `e.what()`.

**Screens that already read a sentence.** The song panel's Analyze error (`details_panel.cpp`, `message()` plus a dimmed `error()`), the status line (`app_state.cpp`, lines 367 and 528), "Analyzed, but saving failed. " (line 593), the batch strip's report line, and the two leaderboard dialogs (`library_dialogs.cpp`). They need no change beyond what their jobs now hand them.

**Every reachable chart error is already matched.** The cymbal, ghost and accent marker errors and "Duplicate note." in `core/model.cpp` are swallowed by the two per-note catches in `song.cpp` (lines 877 and 1392), so they never reach a screen. Every chart error that does reach one has a matcher entry. Moving the batch from text to kinds therefore changes no batch sentence.

## What changes

1. **The last throwers get kinds.** HashFailed and SongFileMissing go on the `analysis.cpp` throws, ReportWrite on `write_report_file`, and RulesFile on `RulesFileError`. AlreadyPlain goes on `collect_dm_rows`'s refusal and on `ReportJob`'s `why_empty`, so the gate's and the report's own sentences pass through. Cancelled goes on `JobCancelled`, NoScores on `DmReportJob`, and NoRecords on "no records stored yet". All of them keep their words byte for byte.
2. **The batch carries the sentence.** `run_batch`'s worker catch keeps both the raw text and `plain_error(e)`, while the exception's type is still known. `on_error` hands both over (open question 7). `BatchJob` puts the sentence in `failures` and the raw text in `failure_details`, as today. hydra_batch prints the raw text, as today.
3. **A failed save is a failed chart.** `run_batch`'s consumer catches a throw from `save_analysis`. It counts that chart as failed, calls `on_error` with the database sentence and the raw text, and goes on to the next chart (open question 5).
4. **The Preview reads the jobs' sentences.** `poll` takes `job_->message()` and `scene_job_->message()`. `render`'s catch takes `plain_error(e)`. A new `PreviewController` accessor holds the raw text beside `error()`. `preview_tab.cpp` keeps "Preview failed: " plus the sentence, then prints the raw text on a dimmed, wrapped details line, the way the song panel's Analyze error does (open questions 1 to 3). The audio-device hover (`audio_warning`) stays raw: it is already a details hover under a plain line.
5. **The Dynamics tab.** It shows "Dynamics failed: " plus the job's `message()`, with `error()` on a dimmed, wrapped details line. `reap_dynamics` builds "Counted, but saving failed. " plus `plain_error(e)`, the same shape as "Analyzed, but saving failed. " (open question 4).
6. **The matcher goes.** Delete `plain_error_text`, `is_no_notes_message`, and `is_timing_refusal` with its declaration. The three refusal prefixes stay in `song.h`, because `song.cpp` builds its messages from them; fix their comment to name what uses them. `plain_error` keeps the kind switch, the `std::bad_alloc` type check, and the fallback. No string matching is left (open question 6). `user_messages.h`'s comments say a sentence comes from the error's kind, and name the switch.

## Owned files (only these may change)

- `src/app/user_messages.h`, `src/app/user_messages.cpp`
- `src/app/analysis.h`, `src/app/analysis.cpp` (the hashing throws, `BatchCallbacks::on_error`, and `run_batch`'s two catches)
- `src/app/report_files.cpp`, `src/app/rules_file.h`, `src/app/dm_report.cpp` (line 204 only)
- `src/parse/song.h`, `src/parse/song.cpp` (`is_timing_refusal` and the prefixes' comment only)
- `src/ui/job_base.h` (`JobCancelled`), `src/ui/dm_jobs.cpp`, `src/ui/library_jobs.cpp` (`ReportJob`'s throws and `BatchJob`'s `on_error`)
- `src/ui/preview_controller.h`, `src/ui/preview_controller.cpp`, `src/ui/preview_tab.cpp` (the error line only)
- `src/ui/dynamics_tab.cpp`, `src/ui/app_state.cpp` (`reap_dynamics`'s line only)
- `src/cli/batch.cpp` (the `on_error` lambda only)
- `tests/test_user_messages.cpp`, `tests/test_library_jobs.cpp`, `tests/test_analysis.cpp`, `tests/test_preview_controller.cpp`, `tests/test_app_state.cpp`, `tests/test_report.cpp` (line 509's check only)
- `tests/ui/uitest_preview.cpp` (`test_preview_error_wraps` only), `tests/ui/uitest_details.cpp` (`test_long_error_wraps` only)
- `tests/test_single_owner.cpp`: ER1's row and its `known_copies` lines only.

Not yours: ER1's files outside this list, `details_panel.cpp`, `library_dialogs.cpp` (both already read `message()` and `error()`), `src/ui/main.cpp`.

Notes on the base. AL2 added `read_song_length` to `BatchCallbacks` and a song-length read to `AnalyzeJob::start`, and it rewrote `update_song_length`'s catch. Stay out of those lines. Whichever lands second merges text only.

## Test cases with red lines

Write each red first, then green, and keep the red line. Pin sentences as literals. Build stores with the trigger technique from `test_store.cpp`'s `a save_analysis that fails leaves nothing behind`, and batches with the counting analyzer `test_analysis.cpp`'s dedupe case uses. No new helper.

In `tests/test_user_messages.cpp`:
1. New: "user_messages: an untyped error reads the fallback, whatever its words". `std::runtime_error("cannot write C:\\x\\hydra_paths.html")` and `std::runtime_error("cancelled")` read "Something went wrong. Try again, and if it keeps happening, report it with the details below." Red line: today they read the report-write sentence and "Stopped before it finished.".
2. Rewrite every existing case that throws an untyped error with a thrower's words to throw that thrower's kind. Delete a case once ER1's case 1 already pins the same sentence. Keep `a real refused load shows the tick sentence`, the `NoNotesError` and `RulesFileError` checks, and `std::bad_alloc` as they are.

In `tests/test_analysis.cpp`:
3. New: "run_batch counts a failed save as a failed chart and goes on". Two charts, with a trigger that refuses one chart's `results` insert. `run_batch` returns. `on_error` is called once, with the database-write sentence and the raw "add_row" text. The other chart is stored, and the last progress reads analyzed 1, failed 1. Red line: `run_batch` throws today.
4. New: "run_batch hands on_error the sentence its exception's kind names". An analyzer that throws a kinded StoredResult error reads "A saved result couldn't be read. Re-analyze this song to replace it." Red line: `on_error` takes no sentence.

In `tests/test_library_jobs.cpp`:
5. Re-pin `jobs: a failed leaderboard fetch says what to do`. Its fetcher throws the NetUnreachable kind with the same words. The default WinHTTP fetcher already throws kinds through `fail` (ER1). Red line: an untyped error reads the fallback after case 1.

In `tests/test_preview_controller.cpp`:
6. New: "a Preview asset failure reads Reinstall Hydra, with the raw text as its detail". Point `app::asset_dir` at a missing folder through `app::set_path_overrides`, then call `render(64, 64)` on a `PreviewController(nullptr, nullptr)`. `error()` reads "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.", and the new raw accessor starts "PreviewRenderer: missing". Red line: `error()` holds the raw "PreviewRenderer: missing ..." text.

In `tests/test_app_state.cpp`:
7. New: "a Dynamics save failure reads the database sentence". Build the app the way the existing Dynamics cases do, with a trigger refusing the `put_dynamics` insert. `dynamics_store_error` reads "Counted, but saving failed. Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and that no other copy of Hydra is running, then try again." Red line: "Counted, but saving failed: put_dynamics failed: ...".

In `tests/test_report.cpp`:
8. Line 509 throws an untyped error with `why_empty`. Re-pin it to the AlreadyPlain error `ReportJob` now throws, still pinning the sentence literal above it.

GUI scripts:
9. `long-error-wraps` (`uitest_details.cpp`): the long path is now on the details line under "Dynamics failed: " plus the sentence. Re-pin the line the test looks for to that details line, and keep its overflow checks as they are.
10. `preview-error-wraps` (`uitest_preview.cpp`): keep the "Preview failed:" check. Add a check that the details line holds `pc`'s raw text. Its overflow checks stay.

Scan row: in ER1's "Which plain sentence does this failure show?" row, delete every `known_copies` line ER1 added. The matcher is gone, so the row now flags any string match on an error's text anywhere in `src`, including `user_messages.cpp`.

Existing cases that must pass unchanged: `jobs: a failed job records the raw text, the plain message, not ok and finished`, `run_batch analyzes a chart found in two folders once`, `save_analysis writes the song, the result and the count together`, and the GUI scripts `preview-mode-reload` (it pins the no-notes sentence through `pc.error()`), `dynamics`, `rules-error`, `batch-done-strip`, `report-buttons` and `status-line`.

## Test filters

- `build-cpp\Release\hydra_tests.exe` with `-sf=*test_user_messages*`, `-sf=*test_analysis*`, `-sf=*test_library_jobs*`, `-sf=*test_preview_controller*`, `-sf=*test_preview_load_progress*`, `-sf=*test_app_state*`, `-sf=*test_report.cpp*`, `-sf=*test_dm_report*`, `-sf=*test_song.cpp*` (run only), `-tc="single-owner*"`, `-tc="hydra_batch reuses the GUI's scan cache"` (build `hydra_batch` warm first)
- `build-cpp\Release\hydra_uitest.exe --test <name>` for `long-error-wraps`, `preview-error-wraps`, `preview-mode-reload`, `dynamics`, `rules-error`, `batch-done-strip`, `report-buttons` and `status-line` (build `hydra_uitest` with `.\build_cpp.ps1 -Target hydra_uitest` first)

Nothing else. Never the full suite, never `hydra_uitest --all`.

## Stored results

None change. A batch that hits a failed save now keeps every result saved before and after it, and counts that one chart as failed.

## Not in this task

- The scan's problem list ("N problems during the scan:", with raw lines such as "Failed to write chart library: ...") and the rules-file line under the toolbar. Each line names the file or the line at fault, and D51 named no change to either.
- A database that can't open at startup. `AppState`'s constructor calls `open_store`, and nothing up to `src/ui/main.cpp` catches it, so Hydra closes with no message. No screen ever shows the database-open sentence. The store-open writes (`fill_missing_stars`, `delete_auto_results`) end the same way. A startup message is a new screen, so it is a follow-up for the user, not part of ER.
- The status line has no details line, so "Could not analyze X. Something went wrong. ... report it with the details below." points at details it does not show. That is a follow-up too.
- hydra_batch's, hydra_report's and hydra_fillcompare's raw stderr lines.

## Done when

- Nothing in `src` reads an error's words to pick a sentence: `plain_error_text`, `is_no_notes_message` and `is_timing_refusal` are gone, and the scan row has no known copies left.
- Every throw site with a sentence carries its kind, the batch hands sentences over, and a failed batch save is a failed chart.
- The Preview tab and the Dynamics tab show the kind's sentence with the raw text on a details line.
- The cases above pass with their red lines recorded, and the named existing cases and GUI scripts pass.
- `git diff --stat <base>..HEAD` lists only the owned files.

## Open questions (each with a recommended answer)

1. **A Preview asset problem on the Preview tab** (needs the user: displayed text; D51 call 25 already chose it). "Preview failed: PreviewRenderer: missing texture gem.png" becomes "Preview failed: Some of Hydra's Preview files are missing. Reinstall Hydra to restore them." The same goes for the damaged files in ER1's open question 4. Recommended: yes, as decided.
2. **The Preview error gains a details line** (needs the user: a new line on screen). The raw text shows dimmed and wrapped under "Preview failed: ...", the way the song panel's Analyze error does. Recommended: yes, for every Preview failure. Today a failed load shows the sentence with nothing under it, and the fallback sentence says "report it with the details below".
3. **Graphics-card failures and Hydra's own Preview bugs** (needs the user: displayed text). A failing Direct3D call (`check`, raw text like "PreviewRenderer: texture") and a failed overlay build (raw text like "build_track_state: ...") now read "Preview failed: Something went wrong. Try again, and if it keeps happening, report it with the details below.", with the raw text on the details line. Recommended: yes, the fallback. The alternative is a new sentence for a graphics-card failure, whose words would need drafting and your approval.
4. **The Dynamics tab's two lines** (needs the user: displayed text). "Dynamics failed: <raw text>" becomes "Dynamics failed: <sentence>", with the raw text on a dimmed details line. For a missing chart file that reads "Dynamics failed: Hydra couldn't open the song file. It may have been moved or deleted; run Scan library to update the library." "Counted, but saving failed: <raw text>" becomes "Counted, but saving failed. <sentence>", like "Analyzed, but saving failed. ". Recommended: yes. Both lines show raw program text today, the thing D51 call 25 removes.
5. **A failed batch save** (needs the user: a new line where Hydra used to close). Today a database error during a batch closes Hydra. After this, that chart joins "N charts failed" as "<title>: Hydra couldn't save to its database (hydra.db). Check that the disk isn't full and that no other copy of Hydra is running, then try again.", with the raw text under it. The batch goes on with the next chart, and hydra_batch prints its "FAILED" line instead of stopping. Recommended: yes, per chart. A locked database can clear by the next chart. If the disk is full, every later chart fails with the same sentence, which says what to do. The alternative stops the whole batch at the first failed save, which keeps the list short but throws away charts that would have saved.
6. **The text matcher is deleted outright** (code-only). The plan kept a matcher for third-party text, but ER1 found none that reaches it. sqlite's, WinHTTP's, JSON's and miniz's text always arrives inside Hydra's own words, and `std::bad_alloc` is matched by type. Recommended: delete it, so the scan row can forbid string matching everywhere.
7. **How the batch hands a failure over** (code-only). Recommended: `on_error` gets the sentence beside the raw text, worked out by `plain_error` at `run_batch`'s catch, the only place the exception still exists. The alternative passes the kind and lets each caller call the switch, which gives the GUI and hydra_batch two places to ask.
8. **The Preview takes the jobs' own `message()`** (code-only). Recommended: yes. `ResultJobBase::fail` already works out the sentence with the type in hand, and `poll` working it out again from text was a second answer.
9. **The leaderboard gate's refusal passes through** (code-only). The toolbar turns Compare off before `collect_dm_rows` can refuse, so no screen reaches it today. Recommended: AlreadyPlain, so it shows the gate's own sentence if it ever does, instead of the fallback.

No other displayed text changes. The batch strip, the status line, the song panel, the report strip and the leaderboard dialogs read the same sentences as before, because every chart, network and report error that reaches them was already matched.

## Commits

One commit for the throwers and the batch, and one for the screens and the matcher's removal, if you prefer. Trailers: `Task: ER2` plus the preamble's others, with the session above. Report as the preamble says, plus the line count of the matcher you deleted.
