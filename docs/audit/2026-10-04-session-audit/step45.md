# Draft: step 4 (docs) and step 5 (shell path), checked at main 50e4b0b

Short version. Of the 45 findings in this slice, 4 are already fixed, 5 are partly fixed, and 36 are untouched. Steps 1 and 2 rewrote ADR 0011, 0013 and 0014 and the cap-clamped doc, which closed 46 and 48, and D31 reworded the fill-slop guide line (96). The rest of the ADR 0019 and ADR 0012 text, the User Guide sentences and the CLI messages still say what the audit found. Nothing in this slice changes a stored record. 13 findings change something a user reads (guide text, a tooltip, a CLI line); the other 28 are comments, ADR text, and "write the decision down" items. Two of those 28 (R7.31, R7.39) only stay code-only if the user confirms the current behaviour.

Every status below was checked by reading the file at head, and by `git log -S` on the old text where it had gone.

## Status table

| id | status | evidence |
|---|---|---|
| 6 | open | `src/core/model.h:624` "Points per scored note"; `src/app/report.cpp:108` tooltip "Points per note on average" |
| 39 | open | `docs/UserGuide.md:162` "how long a full meter would last from here"; `build_drain_box` still uses the bar length at the playhead |
| 40 | open | `docs/UserGuide.md:118` "The more negative the value, the earlier you have to hit" and `0ms` |
| 41 | open | `src/cli/batch.cpp:134` "not tagged as legacy", `:169-170` "The rule is not stored on each result"; `tools/replay.cpp:130` "Every stored row is a 1.1"; comments `src/search/graph.cpp:76-77`, `src/core/timing.h:47-53` |
| 42 | open | `docs/UserGuide.md:122` "`3ms` to `85ms` range (the upper edge follows the hit-window setting)"; line 131 already names `backend_leeway_ms` |
| 43 | open | `docs/UserGuide.md:170` "shows the counts but notes that the game won't apply them" |
| 44 | open | ADR 0012 line 35 "pitch 96 (the difficulty's own kick)", line 45 `allows_dynamics()` (gone from src since a244301) |
| 45 | partly | ADR 0011 lines 4-6 still "two more measures for every phrase" with no cap; the model.h `transfer_pre` comment is gone, and the `sp_end_steps` comment at model.h:317-323 names the Clamped step |
| 46 | fixed | 2919869 rewrote `docs/cap-clamped-squeeze-frontend-anchor.md`; it now names `extend_deacts`, `clamp_tick()` and ADR 0021 only; `git log -S sp_end_shift_ms` on the doc ends there |
| 47 | partly | `src/core/model.h:645-646` still promises "An older blob reads back kNoRulesFingerprint"; `path_codec.cpp:351-355` throws instead (and `structure_is_current` reads the row Stale before that); ADR 0014 lines 31 and 79-80 still name `kRowReadySql` and `IN (?, ?)`; today it is `row_ready_sql()` at `record_store.cpp:350-356` |
| 48 | fixed | ADR 0011 ends with "Correcting note (finding 48)" (2919869, step 1 Task 22) |
| 91 | partly | Task 22 edited `UserGuide.md:126` but it still says the warning shows whenever a phrase "fills the meter up to the SP cap"; the second gate (a listed squeeze or an uncounted row, `squeeze_rating.cpp:143-151`) and "a tie does not clamp" are still missing there and in `CONTEXT.md:174-180` |
| 92 | open | ADR 0013 lines 6-7 "nothing collected after that note can push the end out any further"; `CONTEXT.md:178-179` same; `extend_deacts` keeps extending once the meter drains |
| 93 | open | `src/app/path_view.cpp:218` shows `act.sp_meter()`; `path_view.h:82` and `UserGuide.md:112` call it bars you "spend" |
| 96 | fixed | dbc6a1d (D22/D31) rewrote `UserGuide.md:253`: "how close after a fill's end a note must be for the fill to land on it... It applies only to fills written in the chart", which is the D30 rule; `git log -S "for the fill to count"` ends there |
| 102 | open | ADR 0012 lines 80-83 "a ghost kick now draws as a narrowed bar"; `highway_draw.cpp:350-353` narrows ghosts only when `!g.kick` |
| 104 | open | `UserGuide.md:203` "It follows the current analysis settings"; `docs/development.md:38` "at the same chart mode"; subtitle `report.cpp:394-397` "top N paths per chart" while the cut is per chart and mode |
| 106 | open | `batch.cpp:264` "Store now holds N records across M songs" from `counts()` (`SELECT COUNT(*) FROM results`, line 1609); `report.cpp:401` counts rank-1 Ready rows |
| 107 | open | `tools/replay.cpp:551-556` prints `a.cap`, `a.ms`, `a.depth_mode` (the typed text), not the parsed `Settings` |
| 110 | open | `src/core/replay.cpp:377` "high by that note's first-hit share"; `replay.h:259`; `ambiguous_window_warnings(song, result, windows)` takes no rules |
| 114 | fixed | a82aea9 "Name the frontend-decided row rule once": the variable is now `decided`, with `is_frontend_decided` the helper; CONTEXT.md keeps "Frontend squeeze" for the glossary meaning |
| 115 | open | `src/app/preview_view.cpp:308-309` "where the ms index does not reach; the timing's own timecode extrapolates instead" |
| 118 | open | `src/render/preview_config.h:64` "relative to the gem's bottom centre"; `highway_draw.cpp:101` and `.h:54,100` say top centre |
| 326 | open | `tools/bench.cpp:193` `ms_filter = nullopt`, `:207` label "cap4 d4", literal 4 |
| 327 | open | no ADR or CONTEXT line; magenta still twice in `preview_config.cpp:20,24`; "OpusHead" in `decode.cpp:79` and `opus_reader.cpp:181` |
| 336 | open | `stars.cpp:10` float multiply; `CONTEXT.md:120-122` "Star cutoff" says nothing about 32-bit |
| 337 | open | `track_state.cpp:26` `kSpanEndTicks = 0.5`, no record |
| 338 | open (decided) | `preview_source.cpp:322-327` is the one owner; CONTEXT "Transport" (line 292) is silent; the decision exists (2026-09-24 plan decision 9, Task 17 gate) |
| 339 | partly | the 4/4 default got one owner under D27 (findings 258, 319); `kOnActivationMs` (preview_view.cpp:610) and the BPM 0.0 fallback still unrecorded |
| 348 | open | `preview_view.cpp:610` `kOnActivationMs = 0.5` |
| 349 | open | `rules.cpp:16` `fnv1a64`, 17 digits, 0 to 1; ADR 0014 still says only "the fingerprint of the rules" |
| 351 | open (decided) | `library_jobs.h:88` `kEtaMinFinished = 3`; the approved 2026-09-27 design spec line 81 is the decision; no ADR/CONTEXT line |
| R7.12 | partly | the scan test has the rule "Does the Windows shell take a path this long?" (`test_single_owner.cpp:86-110`) and lists the two `report_files.cpp` lines as known copies "removed by fix shell-path-owner (audit R7.12)" (`:749-754`); `winstr.cpp:42` still has `kPlainPathLimit = MAX_PATH - 12` beside `shell_path`'s `MAX_PATH`; no `fits_shell`; ADR 0020 says only 260 |
| R7.31 | open | `config.h:68-69,75-76` defaults 10 ms on, 50 ms off; no record |
| R7.32 | open | `preview_load_job.cpp:28-30` shares, `:34-35` MB rounding, `:271` 59.5 s; no record |
| R7.34 | open | ADR 0019 unchanged on the reader numbers |
| R7.36 | open | `app_state.h:445`, `engine.cpp:1723` (0.005), `pather.cpp:31`; no record |
| R7.37 | open | ADR 0019 lines 81-84 and `CONTEXT.md:289-290` "decode error"; `stream_mix.cpp:244` silences on any short read |
| R7.38 | open | ADR 0019 line 61 "exactly the old samples" |
| R7.39 | open | ADR 0006 lines 31-32 and ADR 0019 lines 71-72 "each link plays"; `opus_reader.cpp:222-236` keeps links only up to the first with another channel count or no audio |
| R7.40 | open | ADR 0020 names 260 only and misses the "short name exists but the viewer launch fails" copy at `report_files.cpp:118-121` |
| R7.41 | open | ADR 0019 lines 66-67 "1e-3 per sample after the first 20 ms"; `test_stream_mix.cpp` allows 0.013 for odd rate pairs |
| R7.42 | open | ADR 0019 line 51 "checks every seek"; `Mp3Reader::landed` is unchecked past the scanned frames |
| R7.43 | open | ADR 0019 line 43 says nothing about the 2 GiB Vorbis limit |
| R7.44 | open | ADR 0020 lines 38-44 promise "any std::filesystem call"; the rule at `test_single_owner.cpp:197` lists 15 names and matches `::directory_iterator(`, so `recursive_directory_iterator`, `absolute`, `status`, `equivalent`, `space`, `current_path` slip past; `src/cli/report.cpp:100` and `fillcompare.cpp:112` call `std::filesystem::absolute` with no `os_path` |

Counts: fixed 4 (46, 48, 96, 114). Partly fixed 5 (45, 47, 91, 339, R7.12). Open 36. Still to do: 41.

## Open findings in detail

### A. User Guide sentences that contradict the app (39, 40, 42, 43, 91, 93, 104)

These are all one kind of fix: the code is right and the guide describes something else. The owner in every case is the code that already exists; the guide only has to describe it.

The drain box (39) measures a full meter at the tempo under the playhead, which the user chose in the 2026-09-27 drain-box plan (line 482, "yes do the snap"). The guide line 162 should say "at the current tempo". Test: none needed, `tests/test_preview_view.cpp` (lines 1155-1166) already pins the box number.

The early-fill sign (40) is owned by `early_fill_difficulty` in `src/core/model.h`; positive means hit early (decision 3 of the 2026-09-24 plan). Line 118 of the guide should say "a positive number means you must hit that many ms early; negative is slack" and write `0.0ms`. `tests/test_path_view.cpp` (lines 152-176) pins the sign.

The uncounted-backend edge (42) is `Rules::backend_leeway_ms`. Line 122 should say "from the backend leeway (3 ms by default, `backend_leeway_ms`) up to the hit window". `tests/test_rules.cpp` covers the leeway.

Untagged MIDI dynamics (43): `MidiParser::op_note` turns every note Normal without the tag (D24 confirmed the tag spellings). Line 170 should say the tab shows no ghost or accent counts and says why. `tests/test_dynamics_breakdown.cpp` and the D24 tests in `tests/test_song.cpp` cover the parser.

Overfill warning (91): `rate_activation` fires only when the window is clamped and the activation lists a SqIn/SqOut or an uncounted/squeezed-out row (`squeeze_rating.cpp:143-151`), and a phrase that only ties the cap does not clamp. Line 126 and `CONTEXT.md:174-180` should say both. Fixture `clamped_no_squeeze` in `tests/test_squeeze_rating.cpp` pins the no-warning case.

Bars on the row (93): the row reads `act.sp_meter()`, the bank at the activation. The engine's SP end can run longer when a phrase is collected mid-SP (`extend_deacts`). This one needs the user's call (question Q1 below) because either the words or the number changes. If the words change: `path_view.h:82` and guide line 112 say "bars banked when you activate". If the number changes: `build_activations` reads the stored SP-end steps (`collected_phrase_ticks()`, `deact_tick()`) and shows bars spent; `tests/test_path_view.cpp` gets a new case on the "mid-activation phrase extends the end" fixture from `tests/test_search.cpp` (around line 469).

Path report scope (104): `collect_rows` lists every chart mode at the current cap and lens, top N per chart and mode (user decision 2, 2026-09-26 plan). Guide line 203, `development.md:38` and the subtitle at `report.cpp:396-397` ("top N paths per chart and mode") should say so. `tests/test_report.cpp` has the subtitle checks; one assertion changes with the wording.

### B. The report tooltip (6)

`Path::avg_mult` divides the score without solos by the 1x base score. The tooltip at `report.cpp:108` and the comment at `model.h:624` open with "Points per note". Both should open with "Average multiplier: the score without solo bonuses divided by the base score (every note at 1x)". `tests/test_report.cpp` greps column tooltips; one string changes.

### C. CLI messages (41, 106, 107, 110, 326)

Fill-rule messages (41). Since 1f3efdf each result carries its rule in its key (`store::Lens.legacy_fills`, ADR 0010 note, `CONTEXT.md:135-140`). The two `hydra_batch` refusals and the `hydra_replay` usage text still give the old reason. The guards stay (ADR 0010 note: "kept because the request was for the GUI only"); only the reason changes, to "hydra_batch keeps 1.0 results out of hydra.db by design; use the app's 1.0 fills setting for that" and "This file is stamped with the other rule". The two comments (`graph.cpp:76-77`, `timing.h:47-53`) lose "never writes the database the GUI reads" and the "exact squeeze solver" mention. `tests/test_batch_text.cpp` is the place for a message test; none exists for these two refusals today, so add one that runs the guard function on a temp db. Verify first that `timing.h:51`'s claim "`frontend_transfer_scales` calls `tick_at_ms`" is true today; the audit said that call does not exist.

Batch closing line (106). Two lines say "records across" and count different things. Needs Q2. Either way the owner is one scope. Cheapest: the batch line says "rows for every setting" (`batch.cpp:264`). Test: `tests/test_batch_text.cpp`.

Dump's not-analyzed line (107). `cmd_dump` should print from the parsed `Settings` (`s.sp_cap`, `s.mslimit_*`, `s.depth_mode`) with the app's own words (`batch_settings_summary` in `src/ui/library_dialogs.cpp:87` already formats "10 ms"/"off"). Then it can only describe what it looked up. No test covers the line today; add one in `tests/test_replay.cpp` or a small CLI text test.

Squeeze-out warning (110). `ambiguous_window_warnings` should take the rules and quote that chord's `sqout_reduction` under `sqout_rule`, so a `whole_chord` user reads the real overstatement. Touches `replay.h:259,268`, `replay.cpp:334-377`, the one caller `tools/replay.cpp:346`, and the tests at `tests/test_replay.cpp:420-907` (signature change). Add one case with `whole_chord` on a two-note chord.

Bench label (326). Keep corpus mode fixed (it is the before/after yardstick); label it "cap4 d4 no-ms" and use `kCloneHeroSpCap`. Needs Q3 only because the label is output. No test.

### D. ADR and comment corrections (44, 45, 47, 92, 102, 115, 118, R7.37, R7.38, R7.40, R7.41, R7.42, R7.43, R7.44)

All code-only. Each is a sentence that names a function that no longer exists or describes a rule the code does not follow. The fix is to point the sentence at the owner:

- 44: ADR 0012 points at `MidiParser::optype` and `difficulty_base_pitch` (96/84/72/60), drops `allows_dynamics()`.
- 45: ADR 0011 lines 4-6 add "or earlier, when the cap pins the end (ADR 0013)"; `extend_deacts` owns it.
- 47: `model.h:645-646` says an older structure reads Stale and is never decoded (`structure_is_current`); ADR 0014 names `row_ready_sql` and says the rules part compares one fingerprint.
- 92: ADR 0013 and `CONTEXT.md:178-179` say the end is pinned while the meter is full and later phrases extend it from there; the test "a later unclamped extension keeps the earlier clamp_tick" pins it.
- 102: ADR 0012's closing paragraph records f3ff7ec: a ghost kick keeps full width and takes only the overlay; `build_highway_draws` owns it; the "Clone Hero's own rendering" claim is marked unverified.
- 115: the preview_view comment says the ms index covers every tick (`MsIndex::at` extrapolates) and that `timecode(t).ms()` is the same call.
- 118: `preview_config.h:64` says top centre.
- R7.37, R7.38, R7.41, R7.42, R7.43: ADR 0019 sentences reworded to what the readers do (any short read silences a stem; the straight-read promise is per reader, and a resampled mix is within 1e-6; odd rate pairs restart within one input frame, tolerance 0.013; MP3 seeks are checked inside the scanned frames and huge files take the exact slow path; Vorbis over 2 GiB refuses at open, a limit that was already there). R7.37 waits on the R7.8/R7.9 calls (FLAC length 0, Opus seek after damage), which are in another slice; write it after them.
- R7.40: ADR 0020 adds the 248 edge (see E) and the third fallback (short name exists but the viewer launch fails, so copy to temp).
- R7.44: keep the ADR's promise and widen the test. The regex at `test_single_owner.cpp:197` gains `recursive_directory_iterator`, `absolute`, `status`, `equivalent`, `space`, `current_path`, and matches `::name(` as a whole word. Then `src/cli/report.cpp:100` and `src/cli/fillcompare.cpp:112` wrap their `absolute` call in `os_path`, and `tests/test_single_owner.cpp:909`'s own walker is already a listed owner line. Test: the scan itself (`-tc="single-owner rules hold*"`). This is the memory rule "one-place promise covers every question".

### E. The shell path (R7.12, step 5)

What ADR 0020 already covers: the 260 shell edge and `shell_path` as its owner, and the scan rule exists (commit c0acb26 wrote the "Does the Windows shell take a path this long?" rule and listed the two `report_files.cpp` copies as known copies waiting for "fix shell-path-owner (audit R7.12)"). So the gate is in place; the fix itself is not.

What is left, and it is smaller than the audit framed it. The 248 and the 260 are two different Windows facts, not one question asked twice. 248 is `MAX_PATH - 12`, the longest path the wide file calls take before the `\\?\` prefix is needed (the margin leaves room for an 8.3 name; the older CreateDirectory docs stated 248 outright, and today's page at learn.microsoft.com says only "limited to MAX_PATH" and "prepend \\?\"). 260 is what the shell accepts, measured on 2026-10-03. So the user's call (Q7) is really "write both down", not "pick one". The fix: `winstr.h` exports `fits_shell(const std::wstring&)` (`size() < MAX_PATH`), `shell_path`, `open_in_browser` and `copy_to_short_temp` call it, the scan rule's known-copies entries for `report_files.cpp` are deleted (so a new copy fails the build), and the prefix literals in `shell_path:79-80` reuse one constant with `win32_path`. ADR 0020 gets a paragraph naming both numbers. Tests: `tests/test_long_paths.cpp` (lines 232-257 pin the short-name rule) and `tests/test_single_owner.cpp` (the rule's owner lines move to `fits_shell`).

### F. Decisions to write down (327, 336, 337, 338, 339, 348, 349, 351, R7.31, R7.32, R7.34, R7.36)

For each, is the decision already made?

- 338 (Preview audio offset): yes, 2026-09-24 plan header decision 9 and the Task 17 gate measured at the game, extended to .sng/.srb by the 2026-09-26 plan decision 4. Only the CONTEXT "Transport" entry is missing. No question.
- 351 (ETA after three charts): yes, the approved 2026-09-27 design spec line 81. Only a CONTEXT line is missing. No question.
- 336 (float star cutoffs): the multipliers and rounding are decided (CONTEXT "Star cutoff", stars-tab plan); the 32-bit multiply is from the decompiled game and only in the plan's Claude-written text. Needs the user's one-word confirm (Q6).
- 327, 337, 339, 348, 349, R7.32, R7.34, R7.36: no decision anywhere (checked docs/adr, CONTEXT.md, the fix-decisions file D1-D47 and the plan headers). One bundled question (Q6). If accepted, each gets one sentence in CONTEXT.md or its ADR (0019 for the audio numbers, 0014 for the fingerprint, 0008 for the Preview numbers). While there, the magenta literal in `parse_hex_color` becomes one constant and the "OpusHead" tag one constant in `decode.h`, both code-only.
- R7.31 (10 ms on, 50 ms off): no decision; the 10 came from the upstream author. Needs the user (Q4), and it is the only item here whose answer could change stored paths.

## Questions for the user

Each one says what you see today, what you would see after, and my recommendation.

**Q1 (93).** Today a Paths row says "2 bars" when you banked 2 bars at the activation, even if a phrase collected during SP makes it last 3. Option A: the guide and comment say "bars banked when you activate" and the number stays. Option B: the row shows bars actually spent (3). I recommend A: the bank at activation is the number you see on your meter when you hit the activation, and the gauge already shows the spend.

**Q2 (106).** Today `hydra_batch` ends with "Store now holds 3 records across 2 songs" while the report says "2 records across 2 charts" for the same file, because batch counts every row at every setting and the report counts Ready rows at the current settings. Option A: the batch line says "rows for every setting". Option B: batch asks the store for the same Ready count the report uses. I recommend A: the batch line is about the file it just wrote to, and naming the scope costs one word.

**Q3 (326).** `hydra_bench` corpus mode prints "cap4 d4" but runs with no timing limit, unlike the app's 10 ms default. Keep it fixed and label it "cap4 d4 no-ms"? I recommend yes: it is the timing yardstick the 2026-09-26 plan compares against, and moving it would break every before/after number.

**Q4 (R7.31).** The Path limit starts on at 10 ms and "Hide backend rows beyond" starts off with 50 ms in the box. Confirm both as your defaults so they can be written in the guide? I recommend confirming: the 10 ms came from the upstream author and has been the number behind every analysis so far; changing it would make every new analysis keep a different set of alternate paths.

**Q5 (R7.39).** A chained Opus file (several streams glued end to end) plays only its links that match the first one's channel count; the ADRs say every link plays. Option A: correct the ADRs, code unchanged. Option B: make the reader play every link. I recommend A: a mixer can't switch channel count mid-stem, and no library file was found to hit this.

**Q6 (327, 336, 337, 339/348, 349, R7.32, R7.34, R7.36).** None of these change anything on screen. Do you accept the current numbers as they are, so they can be written into CONTEXT.md and the ADRs? They are: the renderer, audio and leaderboard literals (overlay min scale 0.6, 50 ms cancel poll, flat-line epsilon, magenta fallback colour, 64-byte Ogg tag window, WinHTTP timeouts, status 200 only); star cutoffs multiplied as 32-bit floats as the decompiled game does; Preview spans ending half a tick past their last note; the half-millisecond "on an activation" slack and the BPM 0.0 fallback; the rules fingerprint as FNV-1a 64 over name=value text with 17 digits and 0 mapped to 1; the loading bar's 8/82/4.5 percent shares, MB rounding, one-second pacing and the switch to minutes at 59.5 s; the audio reader numbers (120 ms for a zero-byte Opus packet, 512-packet queue, MP3 seek points every 0.5 s, 1 s hop, 4096-frame blocks, 4 GiB MP3 and 2 GiB Vorbis limits); 16 parked lookups, engine progress every 0.005, 0.9 of the bar for the main pass. I recommend accepting all; each is a tuning value with one owner and tests, and recording them stops the next audit re-reading them.

**Q7 (R7.12).** Two path cut-offs exist: 248 for when a file call needs the `\\?\` prefix, 260 for what the shell accepts. They answer different questions, so I recommend keeping both and writing both into ADR 0020, with one `fits_shell` helper for the 260 side. Say so if you would rather have one number.

**Q8 (wording approvals, 6, 39, 40, 41, 42, 43, 91, 104, 107, 110).** These change text you read, so by the confirm-before-coding rule they are listed, not assumed. Each fix makes the text say what the code already does: the Avg multiplier tooltip says it is a ratio, not points; the drain box line says "at the current tempo"; the early-fill line says positive means hit early; the backend line names the leeway setting instead of 3 ms; the dynamics line says untagged MIDI charts show no counts; the overfill line says the warning needs a listed squeeze; the report pages say every mode at the current cap, top N per chart and mode; the two batch refusals and the replay usage give the real reason for the guard; the dump line echoes the settings it used; the squeeze-out warning names the real cost under your `sqout_rule`. I recommend approving all as one batch.

## Proposed tasks

Tasks run in parallel worktrees. No file is owned by two tasks in the same wave.

**Wave 1, code-only, can start now.**

- T-A "ADR and comment corrections": owns `docs/adr/0011`, `0012`, `0013`, `0014`, `0019` (except the R7.37 paragraph), `0020` (R7.40 only, after T-D lands the 248 text), `src/core/model.h` (comment 645-646), `src/app/preview_view.cpp` (comment 308-309), `src/render/preview_config.h:64`, `CONTEXT.md:174-180`. Findings 44, 45, 47, 92, 102, 115, 118, R7.38, R7.41, R7.42, R7.43. No tests change. Done when every named function in those files exists in src (the new docs-match-code check in T-E proves it).
- T-B "write the made decisions down": owns `CONTEXT.md` "Transport" and a new "Batch time left" line. Findings 338, 351. Done when both entries cite the plan decision.
- T-C "std::filesystem scan widened" (R7.44): owns `tests/test_single_owner.cpp:192-205`, `src/cli/report.cpp:100`, `src/cli/fillcompare.cpp:112`, ADR 0020 lines 38-44. Done when `hydra_tests -tc="single-owner rules hold*"` passes with the six new names and the two `absolute` calls wrapped.
- T-D "one shell-path owner" (R7.12, R7.40): owns `src/core/winstr.h/.cpp`, `src/app/report_files.cpp`, `tests/test_long_paths.cpp`, `tests/test_single_owner.cpp:86-110,749-754`, ADR 0020's shell section. Depends on Q7 only for the ADR wording (default: both numbers). Done when `-tc="*long path*"` and the scan pass and the known-copies entries for `report_files.cpp` are gone.
- T-E "docs match code check" (the mechanical guard, below): owns new `tests/test_docs_match_code.cpp` and its CMake line. Done when it fails on a doc naming a deleted function and passes on head after T-A.

T-C and T-D both edit `tests/test_single_owner.cpp` but different rules (lines 192-205 vs 86-110 and 749-754); run T-C first or merge with care.

**Wave 2, after the user answers.**

- T-F "User Guide and tooltip wording" (Q8, Q1 option A, Q3): owns `docs/UserGuide.md`, `docs/development.md`, `src/app/report.cpp` (tooltip and subtitle), `src/core/model.h:624`, `src/app/path_view.h:82`, `tools/bench.cpp` label, `tests/test_report.cpp` strings. Findings 6, 39, 40, 42, 43, 91 (guide half), 93, 104, 326. Done when `-tc="*report*"` passes and the guide's rules sample matches `Rules{}` (T-E checks it).
- T-G "CLI messages" (Q8, Q2): owns `src/cli/batch.cpp`, `tools/replay.cpp`, `src/core/replay.h/.cpp` (warning takes rules), `src/search/graph.cpp:76-77`, `src/core/timing.h:47-53`, `tests/test_replay.cpp`, `tests/test_batch_text.cpp`. Findings 41, 106, 107, 110. Done when `-tc="*replay*,*batch*"` pass, with one new `whole_chord` warning case and one refusal-message case.
- T-H "record the tuning numbers" (Q6, Q4, Q5): owns `CONTEXT.md` (new lines), ADR 0008, 0014 and 0019 (the sentences Q6 names, plus the R7.39 and R7.37 paragraphs), ADR 0006 line 31, `src/render/preview_config.cpp` (one magenta constant), `src/audio/decode.h` (one OpusHead tag). Findings 327, 336, 337, 339, 348, 349, R7.31, R7.32, R7.34, R7.36, R7.37, R7.39. R7.37 waits on the R7.8/R7.9 calls from the other slice. Done when every number named in Q6 appears once in a doc and `-tc="*preview_config*,*decode*"` pass.

T-A and T-H both touch ADR 0019 and CONTEXT.md, so they are in different waves on purpose.

## The mechanical check (so docs stop drifting)

The problem in this slice is one shape repeated: a doc names a function, a constant or a default, and the code moves. The fix is a test that reads the docs the way `test_single_owner.cpp` already reads the source.

`tests/test_docs_match_code.cpp`, one doctest case, three rules:

1. Every backticked identifier in `docs/adr/*.md`, `CONTEXT.md`, `docs/UserGuide.md`, `docs/development.md` and `docs/cap-clamped-squeeze-frontend-anchor.md` that looks like code (`name(`, `Type::name`, `kName`, `snake_case_name` with an underscore) must occur in some file under `src/`, `tools/` or `tests/`. Sections headed "Superseded" or "Kept as history" are skipped, and a short allow-list holds names that are gone on purpose (ADR 0011's `deact_tick_from_rows()` sentence says it is gone). This would have failed on 44, 46 and 47 the day they drifted.
2. The `hydra_rules.ini` sample block in the User Guide (lines 236-242) parses with `app::rules_file` and every value equals `Rules{}`'s default. This catches 42-style drift in the one place the guide states numbers as settings.
3. Documented defaults carry a marker the test can find, e.g. `<!-- default: Settings::mslimit_value -->` beside "10 ms", and the test compares the number in the sentence to the value in code. Use it for the two R7.31 defaults, the 3 ms leeway, and the 500 ms squeeze window.

The test fails with the doc path, line and the missing name, so the fixer knows which sentence to edit. It runs in the normal suite; it reads about 30 files and takes well under a second, like the single-owner scan.

## Prior art

- Rust's documentation tests run the code examples in docs as tests, and `#[doc = include_str!("../README.md")]` with `#[cfg(doctest)]` extends that to a Markdown file, so the README fails the build when it drifts: https://doc.rust-lang.org/rustdoc/write-documentation/documentation-tests.html (opened).
- Python's `doctest.testfile()` runs the examples in a plain text document and the manual recommends it as "executable documentation": https://docs.python.org/3/library/doctest.html (opened). Hydra has no doc examples to run, so rule 1 above checks names instead of outputs, which is the same idea in a C++ project.
- Microsoft's CreateDirectoryW page today says a path "is limited to MAX_PATH characters" unless `\\?\` is prepended: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createdirectoryw (opened). The 248 in `winstr.cpp` is the older documented figure (MAX_PATH minus room for an 8.3 name); the page no longer states it, which is why ADR 0020 should.
- In this repo, `tests/test_single_owner.cpp` is the pattern for a test that reads source files and names the line that breaks a rule; the docs check copies its walker and its "known copies with a removed_by" shape.

## Notes for the main session

- My status file is `C:\Users\Patrick\.claude\hooks\state\status\ad1d87c0561828d3f.md` (the hook named it; one early line also went to `step45.md` before the id was known).
- Nothing in this slice was run; every claim is from reading head and `git log -S`.
- The `timing.h:51` sentence about `tick_at_ms` via `frontend_transfer_scales` was not verified either way; T-G should read `squeeze_rating.cpp` before rewording it.
