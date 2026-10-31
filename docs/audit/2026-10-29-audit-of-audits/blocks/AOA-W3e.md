# AOA-W3e: concept audit verifier side notes (V5, V6, V7, V8, V9)

Code today = main 76ffb2a8 in `.claude/worktrees/aoa-w3`. Paths are relative to it. "V6.md:187" means `docs/audit/2026-10-11-concept-audit/findings/V6.md` line 187.

Checked first: card 2026-10-11-8eb32145 covers only five other side notes (Playhead return value, audio header wording, stem sharing mode, prev_tick, the progress hook). None of my ids are in it. The decisions file has nothing on the V6 to V9 notes by name (searched `note_in_span`, `ByteRateClock`, `write_report_file`, `CountedLength`, `restarts the measure`).

### V5-SN3: the derivation audit's finding 2 table says "-85 -> Insane"

- **Splits:** None: an old audit document quoting a tier that the user later ruled differently.
- **How it left:** nothing at all (V5.md:161 "The footnote there is out of date, not the code.").
- **User's words:** D48 (all recommended): "A timing on a tier edge counts as inside (Q3)." (docs/audit/2026-10-03-fix-decisions.md:121).
- **Code today:** `tier_for` tests `*ms <= *t.cutoff` (src/core/squeeze_rating.cpp:211) and Extreme's cutoff is `w` = 85 (:188), so 85 ms is Extreme. The audit text is at docs/audit/2026-10-03-derivation-audit.md:74 (`| -85 | Insane | Hard SqOut |`). At the audit's own date the tier test was `*ms < *t.cutoff` (`git show de332fbc^:src/app/report.cpp`, line 235), so 85 was Insane then; the edge ruling came the next day. By reading.
- **Verdict:** PASS
- **Why:** The table was true when written. The user's ruling D48 Q3 moved the edge, and the code follows it. The audit file is a dated record, not a spec.

### V6-SN1: the drag-to-right-end test passes the audio end as the song length

- **Splits:** None: a test whose name promised more than it did.
- **How it left:** nothing at all.
- **Code today:** tests/test_preview_transport.cpp:290-303 is now "scrubber: a song length past the audio ends playback there". It loads a 5000 ms playhead, sets a 9000 ms song length, seeks to `length_ms()` and checks `now_ms()` is 9000. That is the case the note said was missing. By reading.
- **Verdict:** GONE
- **Why:** Commit d0d91262 (2026-10-18, "Preview playback ends at the song's length, the one end (D255 answer 5)") made the song's length the one end and rewrote the test to match.

### V6-SN2: the `ChartNow` comment on an empty md5 against `ViewJob::run`

- **Splits:** None: a comment that disagreed with a throw.
- **How it left:** nothing at all (the audit's own tag was "Wording only (M54)").
- **Code today:** src/app/analysis.h:116-119 now says an empty md5 "means the file is unreadable: the Preview load treats that as no change (the parse error speaks for it), and the click's ViewJob fails with "could not read <path>"". The throw is at src/ui/library_jobs.cpp:488-489. They agree. By reading.
- **Verdict:** GONE
- **Why:** Commit dfbc31ff (2026-10-11, "M54: ChartNow::differs_from answers both callers' md5 comparison") reworded the comment to name both callers.

### V6-SN3: "those carrying a timecode" in the `build_preview_scene` header

- **Splits:** None: a comment that describes a filter the code does not have.
- **How it left:** nothing at all (V6.md:189 "I did not re-read it").
- **Code today:** src/app/preview_view.h:526-527 still says "its activations (those carrying a timecode) become the overlay". The loop at src/app/preview_view.cpp:413-434 takes every activation from `path->walk_activations()` and skips none. `Activation::timecode` is a plain member (src/core/model.h:411), so every activation carries one. By reading.
- **Verdict:** REOPEN
- **Why:** The words suggest some activations are left out. None are.
- **Code-only fix:** write "its activations" and drop the parenthesis. No display, stored record, score or count changes.

### V6-SN4: `note_in_span` is used only by a test

- **Splits:** None: dead production code kept alive by one test.
- **How it left:** nothing at all (V6.md:190 "(M56)").
- **Code today:** `note_in_span` is defined at src/render/track_state.cpp:373 and declared at src/render/track_state.h:310. The only caller is tests/test_track_state.cpp:600-605 (grep over src, tests, tools). The golden dump that used it was removed by commit 9fec86d2 (2026-10-10, "Test suite: drop dead weight found by the 2026-10-10 audit"). The plan that added it (docs/superpowers/plans/tasks/p6-j1-3.md:43) says it served "the golden dump's two hand compares". By reading.
- **Verdict:** REOPEN
- **Why:** The reason it was exported is gone, and the span rule it pins is also pinned by `build_track_state` tests.
- **Code-only fix:** delete `note_in_span` and its test case, or move it into the test file. Nothing the user sees changes.

### V6-SN5: `rules` is held by four objects and keyed by none

- **Splits:** None today: a latent gap, no input reaches it.
- **How it left:** noted under M19 (V2.md:198-203), where the audit wrote "Dormant: `settings.rules` is assigned once at start-up ... and no UI path writes it again". Not asked.
- **User's words:** None found; searched the decisions file and D114 for "rules" and "RulesStamp".
- **Code today:** `PreviewController::open` stores `rules_ = rules` and returns early when the song key, cap and path key match (src/ui/preview_controller.cpp:90-93); `PreviewSongKey` has no rules member (`{entry.md5, reading, fills}`, :88). `settings.rules` is assigned once, at start-up (src/ui/app_state.cpp:38); the only other write is a copy in src/app/config.cpp:413. So nothing can change rules while the app runs. By reading.
- **Verdict:** PASS
- **Why:** The score in the Preview is priced from rules fixed for the whole run, so a rules-only change cannot reach `open`. It becomes a real gap on the day a live rules reload is built; that build should put the rules stamp in the key.
### V7-SN3: the preview_view.cpp comment "a mid-measure meter change restarts the measure there"

- **Splits:** None: a comment worded differently from `MeasureIndex`.
- **How it left:** nothing at all (V7.md:154 "Worth a reword when someone is there").
- **Code today:** the comment is gone. The time box now says "The time signature is the grid's, so it switches where the measure lines restart" (src/app/preview_view.cpp:553-555), and `MeasureIndex` now does restart: "The span a tick sits in: the last one restarting at or before it, so a tick exactly on a restart is in the new span" (src/core/timing.h:124-125). Grep for `restarts the measure` over src finds nothing. By reading.
- **Verdict:** GONE
- **Why:** Commit 801db20c (2026-10-14, "Clone Hero's measure grid and continuous measure count (D115 B2-1)") made the grid restart at the first beat at or after a change and removed the old comment in the same change.

### V7-SN4: `Song::check_activations` walks the meter twice, by two rules

- **Splits:** None today: the second walk is gone. (The earlier version of this was finding 53, which D26 had kept as "unverified against Clone Hero".)
- **How it left:** nothing at all for the side note; finding 53 itself was "recorded as kept" (V7.md:55). Already checked in AOA-C1 (M61 part a, docs/audit/2026-10-29-audit-of-audits/blocks/AOA-C1.md:59-64) and AOA-D2 (AOA-D2.md:17).
- **User's words:** not needed for this note.
- **Code today:** `Song::check_activations` (src/core/song.cpp:105-116) only picks the generator; it has no `tpm_pos`, `pre_tpm` or `plusmeasure` text (grep over src finds none). `place_fills_ch10` reads `grid.length_at(grid.span_at(...))` from the one `MeasureIndex` (comment at song.cpp:100-104 and :127-128). By reading.
- **Verdict:** GONE
- **Why:** Commit 6a98db3b (2026-10-11, M61) dropped the dead tempo walk and commit 5677f8d3 (2026-10-24, "Clone Hero 1.0 fills count measures on the shared grid") removed the meter walk.
- **Side picked without a ruling:** the shared grid won. D26 had kept the old walk. The commit message of 5677f8d3 says the 1.0 fill rule itself is unchanged; AOA-C1 and AOA-D2 judge whether that choice needs the user, so I do not repeat it here.

### V8-SN1: `MaReader::skip` marks an error only when `got != 0`

- **Splits:** None between two parts: `read` and `CountedLength::seek` agree. The mismatch is between code and the contract comment.
- **How it left:** nothing at all (V8.md:201 "They agree; worth knowing for M69"). M69 itself left as a contract comment, not a question (questions.md:213 "a message for a damaged stem would be a new sentence nobody asked for, so it is not put to you here").
- **User's words:** None found; searched the decisions file and questions.md for M69, `failed_`, `CountedLength`.
- **Code today:** in `read`, a decode result of zero frames sets only `at_end_` (src/audio/ma_reader.cpp:210-211, `if (r == MA_AT_END || got == 0)`), and `failed_` is set only when frames came back with an error (:212-214). `skip` does the same: `step.error = r != MA_SUCCESS && r != MA_AT_END && got != 0` (:244). The contract says `failed()` is "True once any decode error has ended the stem early" (src/audio/stem_reader.h:56). The only reader of `failed()` is `decode_audio` (src/audio/decode.cpp:35), which has no caller outside tests and the fuzz harness; the Preview never asks (stem_reader.h:63-65). By reading.
- **Verdict:** REOPEN
- **Why:** An error that arrives with zero frames ends the stem as if it were the normal end, which the "any decode error" sentence does not allow.
- **Code-only fix:** test `r != MA_SUCCESS && r != MA_AT_END` before `got == 0` in `read` and drop `&& got != 0` in `skip`; or reword the stem_reader.h:56 comment to say a zero-frame error reads as the end. Nothing the user sees, no stored record, score or count changes, because only test code reads `failed()`.

### V8-SN2: `CountedLength::seek` leaves `pos` stale after a failed restart

- **Splits:** None.
- **How it left:** nothing at all (V8.md:202 "only at the end, where every read returns 0").
- **Code today:** src/audio/stem_reader.h:176-181: when `restart()` fails the code sets `at_end = true` and returns false without touching `pos`. The next `seek` takes the same branch (`if (frame < pos || at_end)`) and tries `restart()` again, and a read at `at_end` returns 0 frames. `pos_` in `MaReader` is read only by `seek` and `read` (ma_reader.cpp:217, 222, 225, 231). `Mp3Reader::seek` has the same shape (:430-431). By reading.
- **Verdict:** PASS
- **Why:** The stale position is never read while `at_end` is true, and the next seek rewrites it. No sound, count or record depends on it. (The same restart failure is not reported as a decode error either; that is the V8-SN1 question.)

### V8-SN4: `write_report_file` always uses the same `<path>.tmp` name

- **Splits:** None between two parts: a fixed temp name that two writers of one path would share.
- **How it left:** nothing at all. D114 item 20: "Writing a file through a temporary file and a rename becomes one helper, and the settings file is saved through it." (docs/audit/2026-10-03-fix-decisions.md:573). It does not name the temp file's spelling.
- **User's words:** the D114 line above (Q20 at docs/audit/2026-10-11-concept-audit/questions.md:195, "Recommended: the helper"). Nothing about two writers.
- **Code today:** `write_file_replacing` builds `tmp = outpath; tmp += ".tmp"` (src/app/report_files.cpp:128-129). Its two callers are the settings save (src/app/config.cpp:304, the user's `hydra.ini`) and `write_report_file` (used by the fillcompare tool only, src/cli/fillcompare.cpp:105). I found no one-instance lock in src (grep for `CreateMutex`, "single instance", "already running" finds nothing about the app), so two Hydra windows could save the settings at once. By reading; not run.
- **Verdict:** REOPEN
- **Why:** The shared name is still there and nothing stops two processes writing it. The harm is a failed save or a garbled temp file in one rare overlap.
- **Code-only fix:** give the temp name a per-process (or per-call) suffix, for example the process id. No display, stored record, score or count changes in the normal case.

### V9-SN2: "stopped" is read from the cancel flag after the batch finished

- **Splits:** "Did the batch stop early?": the Stop button's flag read after the run ended (src/ui/library_dialogs.cpp:489 `const bool stopped = app.batch_job->is_cancelled();`; src/ui/app_state.cpp:755 `const bool stopped = batch_job->is_cancelled();`) vs "every chart ran" (the snapshot's `finished`). They differ only when Stop lands on the frame the last chart finishes: the strip then says "Stopped" and no path report is built.
- **How it left:** a question the user answered. Q21 (M77) put the batch edge in its own text: "The batch has a third spelling: a Stop pressed on the frame the last chart finishes marks the run \"Stopped\" and builds no path report, though every chart ran." and its case-against ends "and the \"Stopped with every chart done\" edge stays." (docs/audit/2026-10-11-concept-audit/questions.md:201, 203).
- **User's words:** Q21's Recommended line: "Recommended: keep both; the panel's rule is backed by `close_details`" (questions.md:203). Answered as D114 item 21: "Both landing orders stay: a finished report keeps its result after a late Cancel, and a click's analysis is dropped." (docs/audit/2026-10-03-fix-decisions.md:574), under "go with recommended" (:553). The recorded answer names the two landing orders, not the batch edge by itself; the question's own text names the edge with both outcomes.
- **Code today:** unchanged. The Stop button disappears once the snapshot says finished (src/ui/library_dialogs.cpp:422), but a press on the final frame still sets the flag; `render_batch_done` and `update_background_jobs` (src/ui/app_state.cpp:752-757, comment "A stopped run keeps its results but builds no report") read it as before. By reading.
- **Verdict:** PASS
- **Why:** The user was shown the edge, with what happens on each side (label "Stopped" and no report, though every chart ran), and chose the recommended "keep both". The edge is one frame wide.

### V9-SN4: `StoreOpenProgress::time_left_text` forward-declares `ByteRateClock`

- **Splits:** None: a tidy-up, not a bug.
- **How it left:** nothing at all.
- **Code today:** `ByteRateClock` is defined only in src/ui/preview_load_job.h:72 and used there (:167, :204). `src/ui/library_jobs.h` no longer mentions it. By reading (grep over src).
- **Verdict:** GONE
- **Why:** Commit 59b77bc7 (2026-10-14, "WIP: one owner for old library files ...") removed the forward declaration and the clock member.

### V9-SN5: `start_batch` over an empty plan

- **Splits:** None: a direct call can start a batch over nothing; no screen can.
- **How it left:** nothing at all (V9.md:257 "only tests can reach it").
- **Code today:** `AppState::start_batch` (src/ui/app_state.cpp:722-740) has no empty-plan check. Its callers both check first: the confirm dialog enables "Start analyzing" only when `to_run > 0` (src/ui/library_dialogs.cpp:386, 401), and the report-window path closes the confirm and builds the report with no batch when `batch_plan.todo_rows() == 0` (src/ui/app_state.cpp:919-923). The confirm's own text for that case is "Every chart here already has a result." (:714). By reading.
- **Verdict:** PASS
- **Why:** Both ways a user reaches `start_batch` refuse an empty plan first, so the missing check in the function is not a user-visible gap.
## Not reached

None. All 14 ids have a block.

## Side notes

- `MaReader::read` and `Mp3Reader` leave `failed()` false for several damaged-stream cases, and `failed()` has one reader (`decode_audio`, src/audio/decode.cpp:35) that only tests call (src/audio/stem_reader.h:63-65). So a damaged stem in the Preview plays as silence with no message. That is M69's contract comment, not a ruling; the user was told "a message for a damaged stem would be a new sentence nobody asked for" (questions.md:213) but was not asked whether to add one. Not in my items; flagged for whoever owns M69.
- V5-SN3 above: the derivation audit file still reads "-85 -> Insane" because it was true under the old `<` edge. Nothing needs changing, but anyone reading it today needs the D48 Q3 line beside it.
<!-- next -->
