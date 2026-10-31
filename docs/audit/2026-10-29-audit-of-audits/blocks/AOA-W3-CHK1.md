# AOA-W3-CHK1 closure checks

### concept-audit M03 (fill-page claim)
- **Scout said:** GONE
- **Checked:** `git show 74f40bd3` exists; html_page.cpp:354-362 (no localeCompare, compares `a[field]` as plain strings); fill_report.cpp:105-115 and 367-379 (`folded_key` per text column); questions.md:39-49 (Q3); fix-decisions.md:553,556 (D114 item 3); card `docs/sessions/2026-10-11-ec019463.md`:21 ("go with recommended", 16:25 UTC; the prompt before it was not kept).
- **Verdict:** GONE
- **Why:** The fill page now sorts text on the windows' own folded key, so the Etude/Etude and "+"/"-" cases come out as in the windows, and Q3 named both sides. Side-picked claim is true for the text order. Left over and shown in Q3's text: one sort key against the windows' two, and two null cells compare as 1 either way (html_page.cpp:358).

### concept-audit M05 (fill-page claim)
- **Scout said:** GONE
- **Checked:** commit 1db64042 exists; html_page.cpp:412-413 (`' of ' + PAGE.total`); fill_report.cpp:100,383-387 (`counted(..., "chart", "charts")`); no `PAGE.noun` left in src/app; D114 item 4 (fix-decisions.md:557); Q4 questions.md:51-55.
- **Verdict:** GONE
- **Why:** The total is worded by `counted`, so one chart reads "1 of 1 chart" like the windows. Q4 quoted both wordings and the user took the recommendation.

### concept-audit M06 (posted sink)
- **Scout said:** PASS
- **Checked:** dm_report_view.cpp:146-158 (posted empty gives empty key, cell dash); dm_report.cpp:119-121 and report.cpp:52-54 (song, artist, charter go through `display_*`); song.cpp:468-475,554-566 (empty becomes "(unknown)"); fix-decisions.md:471 (D103 item 17); card `docs/sessions/2026-10-08-c4589620.md`:21 (user chose "Sink like the others (Recommended)"); V1.md:182-203.
- **Verdict:** PASS
- **Why:** One side is an optional date that can be empty, the other is text that is never empty after `display_*`, so they are different questions. The user's chosen option names the Posted side and the old blank-text behaviour.

### concept-audit M07 (colour tokens)
- **Scout said:** GONE
- **Checked:** commit fa2c16d8 exists; html_page.h:34-40 (`kChipRgb*`); theme.cpp:22-38 (chip_color reads them, comment true); html_page.cpp:139-140,303-308 (dark scheme injects T0, T3, TN); report.h:75-79 (stale sentence still there, as the scout said); Q5 questions.md:57-61, D114 item 5.
- **Verdict:** GONE
- **Why:** The hand-kept hex copies are one constant set now and the theme.cpp comment is true. The `tn` token being used by three reports for three words is a colour pick, not one question answered twice.

### concept-audit M17, second half
- **Scout said:** GONE
- **Checked:** commit e6140776 exists and adds `is_clone_hero_cap` (core/model.h:40); callers library_dialogs.cpp:79 and dm_report.cpp:57; settings_bar.cpp:125-127 is fixed prose built from `kCloneHeroSpCap`; grep of src shows no hand `== kCloneHeroSpCap` outside the owner.
- **Verdict:** GONE
- **Why:** The three places that say a cap is Clone Hero's all agreed (cap 4 yes, anything else a what-if), and the two code tests now share one function. The Compare-button half was a separate question (D271 item 8) and is not the claim here.

### concept-audit M21 (blank `sig`)
- **Scout said:** GONE
- **Checked:** commit f8199781 exists (message: `scan_item_of` and `to_library_entry` removed, ScanItem alias); analysis.h:30-33 (`using ScanItem = store::ChartLibraryEntry`); grep of src, tests and tools finds no `scan_item_of` or `to_library_entry`.
- **Verdict:** GONE
- **Why:** The copy that blanked `sig` is deleted, so a batch row now is the library entry. No stored row or display changes.

### concept-audit M23 (blank or padded charter)
- **Scout said:** GONE
- **Checked:** commit a8bd88d7 exists; analysis.cpp:708-718 (trim, then `*_or_unknown`, for every format and cache hit); stored_versions.h:95-99 (stamp 4 note, now 6); Q15 questions.md:143-151 shows both stored forms; D114 item 15 (fix-decisions.md:568).
- **Verdict:** GONE
- **Why:** A blank or padded charter now gets the same stored value from .sng, .srb and song.ini. Q15 showed both stored forms and the stamp cost, and the user took the recommendation.

### V4 item 1 (M38, Ch10 sentence and the Ch11 "4 beats")
- **Scout said:** PASS
- **Checked:** commit 57c5da7b exists; graph.h:68-81 (`kCh11FillLeadBeats = 4`, `fill_rule_description`); graph.cpp:60-91 (Ch11 uses the constant; Ch10 deadline is fill start minus `max(250, min(fend - pad, 10000))`, where `fend - pad` is the fill's length plus a res/16 pad); UserGuide.md:156; questions.md:217-219.
- **Verdict:** PASS
- **Why:** Sentence and code answer one question ("how long before the fill must SP be ready") and agree: Ch11 is 4 beats from one constant, Ch10 is about one fill-length, with the 0.25 s and 10 s clamp only biting extreme fills. Nothing is a split. No user ruling is needed, and none is claimed beyond the veto-by-id code-only list.

### V8 item 2 (M73, number grammar)
- **Scout said:** PASS
- **Checked:** commits 357b4a2e and 5273332b exist; rules_file.cpp:44-53, strutil.cpp:80-92, ini_errors.h:44-51, config.cpp:237-240; test_rules.cpp:120-146; Q8 questions.md:87-93; D114 items 8 and 24 (fix-decisions.md:561,577); card 2026-10-11-ec019463 ("Keep: 4.0 loads as 4 (Recommended)").
- **Verdict:** PASS
- **Why:** One reader (`parse_finite_number`, with `parse_whole_number` on top) now serves both ini files. Q8 named all three readers and the nan/inf/hex examples, and the user separately picked "4.0 loads as 4".

### V8 item 3 (M73, junk policy) and the real M72
- **Scout said:** PASS
- **Checked:** fix-decisions.md:1058 (D270 call 4); card `docs/sessions/2026-10-30-9a82ff65.md` lines 19 and 22 ("Both refuse (Recommended)", "refuse anything bad"); config.cpp:237-240; rules_file.cpp:44-53; user_messages.cpp:161,200-202; user_messages.h:42-44; D114 item 11 (fix-decisions.md:564).
- **Verdict:** PASS
- **Why:** The user's quoted words, with the question's text naming "junk reads as 0" in settings against refuse in rules, settle the split, and both files refuse in code. M72's wording (tools print "has an error:", one app sentence) is now in `user_messages`. The scout's loose end (no upper bound on rules numbers) is a separate number nobody chose; it is real but outside this closure.

### V9 M77 (cancel landing on a finished job)
- **Scout said:** PASS
- **Checked:** commit 5f134210 exists; job_base.h:62-82 (`outcome()`: ok first); app_state.cpp:450-462 (click's exception beside it), :755, :1010-1021; Q21 questions.md:197-204; D114 item 21.
- **Verdict:** PASS
- **Why:** Q21 laid out both orders, the click window, the batch "Stopped with every chart done" edge, and the case against keeping two orders. The user took "keep both" and the code does exactly that, with the exception written beside the owner. The batch edge is named only in the example and case-against text, as the scout said, but it is named.

### V9 M81 ("a batch is running")
- **Scout said:** GONE
- **Checked:** commit fe86522d exists; library_toolbar.cpp:54 and app_state.h:515 (one constant); grep of src finds no "Busy:" string; Q12 questions.md:125-129; D114 item 12.
- **Verdict:** GONE
- **Why:** The tooltip and the status line read one constant now. Q12 quoted both sentences and the user took the plain one.

### V9 M82 ("analysis is off")
- **Scout said:** GONE
- **Checked:** commit 8fd72bb2 exists; details_panel.cpp:173 and library_toolbar.cpp:149 (same notice function, `with_errors` false/true); grep finds no "Analysis is off until" in src; Q13 questions.md:131-135; D114 item 13.
- **Verdict:** GONE
- **Why:** The song panel shows the toolbar's sentence without the file's error, which is the recommended option the user took with both texts quoted.

### V9 M83 (failure sentences)
- **Scout said:** PASS
- **Checked:** commit 4cc6e3c3 exists; user_messages.h:1-8,157-167; the uses at app_state.cpp:1055-1057, library_dialogs.cpp:537-541, path_report_window.cpp:233, dm_report_window.cpp:137-138; Q14 questions.md:137-143; D114 item 14.
- **Verdict:** PASS
- **Why:** Q14 named the status line against the strip and window, and "Could not build the report." against the path report's shape; the user took the recommended pair and the code matches. The header comment now says only what is true (shared sentences live here).

### concept audit M16 (which key parts the store checks)
- **Scout said:** PASS
- **Checked:** record_store.cpp:612-653 (`check_profile_fits`: cap, ms limit both ways, fill rule); model.h:864-890 (`HydraRecord` has no chart-mode or score-range field); config.cpp:453 and config.h:318-325 (`batch_run` builds profile and analysis settings from one Settings); app_state.cpp:398-402 (ViewJob takes `settings.profile()` and `settings.to_analysis_settings()` together) and :542; the other `prepare_record` callers (analysis.cpp:945, tools/bench.cpp).
- **Verdict:** PASS
- **Why:** Nothing answers one question two ways. The record simply does not carry the chart mode or score range, and every saving caller derives key and search settings from one Settings object. Adding a check would need a new stored field, which no screen needs today.

### concept audit M34 (rows inside the squeeze window)
- **Scout said:** PASS
- **Checked:** model.h:110,119-124 (`within_squeeze_window`, strict `< 500`); callers graph.cpp:155 and :324, engine.cpp:1990, sqout_chord.h:32 and :123, model.cpp:819; grep for 500.0 and `< 500` finds no second hand-written test; config.cpp:160 only narrows a display choice by the same constant.
- **Verdict:** PASS
- **Why:** One predicate with six call sites, so there is no second answer. The Backend-limit display filter asks a different question (how many stored rows the user wants shown).

### concept audit M36 (tied-path count)
- **Scout said:** PASS
- **Checked:** engine.cpp:1284-1306 (running count for the limit), :2094-2105 (rebuild recounts each root and throws `logic_error` if the numbers differ); model.cpp:634-641 (`recount_tied_paths`); pather.cpp:179,193; the scout's side finding that `tied_pathcount()` has no production reader (grep of src finds it only at model.h:826).
- **Verdict:** PASS
- **Why:** The mid-search count answers "may this variant still fold under the limit" and the tree recount answers "how many are in the finished tree"; the build throws if they ever differ. That makes the two counts agree by guard, not by hope.

### concept audit M37 (every timing inside the ms limit)
- **Scout said:** PASS
- **Checked:** model.h:284-298,330-340,380-385 (`timing_within_limit`, `fill_within_limit`, `SPSqueeze::within_limit`); engine.cpp:1178-1189; model.cpp:528-532; library_query.cpp:424-431; d37a79b7 (pinned bypass at engine.cpp:1182); Path Builder spec line 91; the only `Path::within_ms_limit` caller is pather.cpp:55.
- **Verdict:** PASS
- **Why:** One edge rule (`ms <= limit`) feeds the engine, the record walk and the Library filter. The newer pinned-window bypass exists only in the engine, but the record walk is asked only about ordinary analysis records, so no screen shows the two disagree; the bypass is the Path Builder design's own stated rule.

### concept audit M48 (is Star Power running now)
- **Scout said:** PASS
- **Checked:** commit 2dbac10a exists; preview_view.h:114-127,384-395 and preview_view.cpp:270-277 (`sp_running_at` is the one owner); track_state.cpp:218-226 and :78-98 (highway builds from the same `sp_window()`); tests/test_track_state.cpp:250-284 pins the sweep to the owner at every edge of two back-to-back windows (read, not run); tests/single_owner/rules/app/d115-sp-running.json.
- **Verdict:** PASS
- **Why:** The "running" test now has one home, and the highway's separate interval walk is pinned to it by a test at start, join and end edges. Nothing the user sees can differ.

### D114 item 17 (Q17, M47, scrubber range)
- **Scout said:** GONE
- **Checked:** commit d0d91262 exists ("Preview playback ends at the song's length, the one end (D255 answer 5)"); preview_transport.cpp:56-62,74-76; preview_controller.cpp:418-422; preview_tab.cpp:545-556; fix-decisions.md:911 (D255 item 5); card `docs/sessions/2026-10-18-c433b17f.md`:25 keeps the first sentence of the quoted words, and fix-decisions.md:911 (D255 item 5) quotes the rest ("we already know how clone hero decides the end of the song"); UserGuide.md:332.
- **Verdict:** GONE
- **Why:** The split Q17 asked about (playback end L against the scrubber's edge E) is gone because playback end and scrubber edge are now both the song length. The user's own words in D255 item 5 chose one rule and name D48's playback range as replaced.

### D114 item 18 (Q18, M50, Preview end labels)
- **Scout said:** GONE
- **Checked:** same D255 item 5 text and code as item 17; preview_view.cpp:539-549; path_view.cpp:408-409; UserGuide.md:342 (clock counts to the song's end); fix-decisions.md:236 (D56 item 3, the clock-to-audio-end explanation, now replaced).
- **Verdict:** GONE
- **Why:** The three ends Q18 set side by side are one now (`app::song_length_ms`), so there is no pair of labels to tell apart. The answer's clause that the guide's D56 item 3 explanation "stands" is outdated, as the scout said, and replaced by D255 item 5.

### Side note 1 (`Playhead::read_frames` return value)
- **Scout said:** GONE
- **Checked:** commit 15f2e917 exists (player.cpp, player.h, and a new test); player.cpp:37-79 returns `got`; player.h:60-67 says the same; side-notes.md line 5.
- **Verdict:** GONE
- **Why:** The function now returns the frames the source really gave, matching its header, and a test pins a short source returning 2 of 4.

### Side note 2 (audio headers say `runtime_error`, code throws `KindedError`)
- **Scout said:** GONE
- **Checked:** commit 97278c68 exists and edits decode.h, mapped_file.h, song_audio.h and stream_mix.h; grep of src/audio headers: only device.h says `std::runtime_error` and device.cpp:61,78 throws it; every other thrown type matches its header. The scout's `mixer.cpp` leftover (two `runtime_error` throws not in mixer.h's comment) is real and test-only.
- **Verdict:** GONE
- **Why:** The headers that named the wrong type were rewritten the same day and no header names a wrong type today.

## Counts
23 blocks checked: 11 PASS, 12 GONE, 0 REOPEN. The one REOPEN block in the input (V8 item 4, M75) was skipped as the brief says. No block changed its verdict between PASS and GONE.

## Not reached
None.

## Side notes
- M03: the second half of D114 Q3 (the Library sorts on the text it shows) was outside the block and not checked here.
- M03 leftovers the scout listed are real and not a split: the fill page has one sort key against the windows' two (html_page.cpp:331), and two null cells compare as 1 either way (html_page.cpp:358-359). Q3's text stated the one-key difference, but its recommended option did not promise to close it.
- M37: Engine::act_within_limit has a pinned-window bypass (engine.cpp:1182) that Activation::within_ms_limit lacks. Harmless today because no pinned path reaches the record walk, but the two walks now differ by design.
- Rules file numbers have no upper bound (rules_file.cpp:37-41); D270 call 3 covers floors only. Same as the scout's note.
- tests were read, not run (test_track_state.cpp:250, test_sp_meter_curve.cpp:543).
- `mixer.cpp` throws `std::runtime_error` (lines 37, 52) for test-only `mix_stems`; mixer.h does not say so. Code-only.
