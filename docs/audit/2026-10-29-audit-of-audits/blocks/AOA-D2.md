# AOA-D2 results (derivation audit 2026-10-03 drift / double_reading findings)

### 2026-10-03 derivation 49: Multiplier squeeze check uses its own combo list, misses 4- and 5-note chords

- **Splits:** Does this chord straddle a multiplier step: `MultSqueeze::applies` fixed list (4-note only at combos 7, 17, 27; 5-note never) vs `to_multiplier` payout (4-note 6-8, 16-18, 26-28; 5-note 5-8, 15-18, 25-28), per the finding's table. Today's code has one side only.
- **How it left:** fixed (question first). Triage: "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)."
- **User's words:** Question text: "Today Hydra lists only 2- and 3-note chords, plus 4-note chords at combos 7, 17 and 27. It never lists a 5-note chord." ... "After: every chord that crosses a step is listed, whatever its size, and the points are the real gain." "Recommended: yes to both." (docs/audit/2026-10-04-phase-7-questions.md:22-26). Answered as D51: "The user answered "all recommended" to every question" plus the user's note "those should never appear in a real drums chart. but it's valid in clone hero so hydra should account for it correctly." (docs/audit/2026-10-03-fix-decisions.md:156-157).
- **Code today:** `MultSqueeze::applies` returns false unless `crosses_multiplier_step(combo, chord.count())`, which is `step.first < step.last` from `multiplier_step` (src/core/model.cpp:373-381, src/core/timing.cpp:24-27). No combo list remains. By reading.
- **Verdict:** PASS
- **Why:** The question named both the old list (2-, 3-note, 4-note at 7/17/27, no 5-note) and the new rule (every chord that crosses a step), and the code now asks `to_multiplier`'s step for every size.

### 2026-10-03 derivation 53: Generated fill length reads the meter one change late with two changes between chords

- **Splits:** Which meter sets a generated fill's length when two meter changes sit between chords: own `SongIter`-style walk, one change late (old src/parse/song.cpp:222-247) vs `MeasureIndex::section_at`, current. Today the walk is gone.
- **How it left:** settled_by_decision (D26). Closure's words: "D26 keeps today's rule as unverified against Clone Hero; ADR 0023 lists it." (docs/audit/2026-10-03-fix-triage.json:895). D26 itself: "No live Clone Hero check now. Generated-fill length and 6/8 beats keep today's rule, written down as unverified against Clone Hero." (docs/audit/2026-10-03-fix-decisions.md:63). ADR 0023:92-93: "A generated fill's length, when two or more meter changes fall between two chords. Hydra's walk reads the meter one change late there."
- **User's words:** D26 says "keep today's rule" and never names either meter (no "one change late", no "current"). The ADR names the late reading, but the ADR is not the user's words. Searched D26 in fix-decisions.md, the step2 handoff (docs/handoffs/2026-10-04-step2-done-handoff.md:51) and ADR 0023; no user quote with the two readings found.
- **Code today:** `Song::place_fills_ch10` takes the fill length from `grid.length_at(grid.span_at(chord.timecode.ticks()))` times `kCh10FillLengthMeasures` (0.5), the measure grid's span at the chord itself, no walk (src/core/song.cpp:163-164). `place_fills_ch11` starts a fill on a measure line, length = chord tick minus line tick (song.cpp:220). The `tpm_pos`/`bpm_pos` walk is gone: `git log -S"bpm_pos"` shows 6a98db3b (M61) and 5677f8d3; `git log -S"tpm_pos"` shows 5677f8d3 (2026-10-24, "Clone Hero 1.0 fills count measures on the shared grid"). By reading.
- **Verdict:** GONE
- **Why:** Commit 5677f8d3 removed the private walk, so both homes read the meter from MeasureIndex. Two meter changes between chords now give the meter in force at the chord, not one behind.
- **Side picked without a ruling:** the current-meter side won (grid span at the chord), replacing D26's "today's rule". Partial ruling exists: session a124ccd9 card, user (2026-10-24 23:38 UTC): "Yes. Move the 1.0 generator onto MeasureIndex/MeasureCount, keep only its own fill rule, and show me which test-corpus charts' 1.0 fills and scores move before merging." then "merge now, bump the stamp" (C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\cards\2026-10-24-a124ccd9.md:27,29). That approves the move after seeing which charts changed (11 of 97), but the words do not name the two-meter-changes case. Also D26's real user words were a "Not now" to a question about a live Clone Hero check ("Live Clone Hero check for fill length, 6/8 beats and whether authored fills stop generation ...", card 2026-10-04-bbf41dc4.md:26), which never described the two readings either. The main session decides whether the 2026-10-24 approval is enough.

### 2026-10-03 derivation 54: Four readers interpret the fill-rule stamp four ways

- **Splits:** What fill rule does a database file hold, for an unstamped file with rows: migration/hydra_batch call it 1.1, hydra_fillcompare silent, 1.0 side empty; and ch11-stamped file used for both sides warns while one file legitimately holds both. Today the stamp is gone.
- **How it left:** fixed. Triage: "phase 7: stamped_fill_rule in M7-2b (6e980ed); hydra_report and hydra_fillcompare decide with it in RP (3ecc9b8, D65), hydra_batch in LB1 (2c1c41e)."
- **User's words:** D115 item 6 (2026-10-14): "The file-wide fill-rule label is retired, one file holds both rules, and `hydra_batch` may write `--legacy-fills` results into the app's own hydra.db." (docs/audit/2026-10-03-fix-decisions.md:587). No phase question cited 54 (searched the three question files and fix-decisions.md).
- **Code today:** No `engine_mode`, `engine_mode_stamp`, `stamped_fill_rule` or `fill_rule_from_stamp` remains in src/. Only `kRetiredFillLabelKey = "engine_mode"` deletes the old meta row (src/store/record_store.cpp:862-866). Each record carries `Lens::legacy_fills`. The triage's "fixed" evidence (stamped_fill_rule) is itself removed, by 43276691 (2026-10-14, "Retire the whole-file fill-rule label"). By reading plus `git log -S`.
- **Verdict:** GONE
- **Why:** The question the four readers disagreed about no longer exists; every row answers it itself, and D115 item 6 recorded the retirement and one-file-both-rules.

### 2026-10-03 derivation 55: Each fill rule's name and explanation typed separately in about eight places

- **Splits:** What the 1.0 rule is called: "Clone Hero 1.0" (batch confirm, mismatch) vs "Clone Hero 1.0 (legacy)" (batch settings) vs "Clone Hero 1.0 fills" (text report) vs "CH 1.0" (fill report) vs "1.0 fills" (checkbox). Plus prose explained twice.
- **How it left:** a question the user answered. Triage: "AF1 (D74 item 4), merged to main 47a9eeb (2026-10-05): the 1.0 fills tooltip is built from fill_rule_description".
- **User's words:** "**13. One name for each fill rule.** (55) Today the old rule is "Clone Hero 1.0" in the batch confirm, "Clone Hero 1.0 (legacy)" a few lines later, "Clone Hero 1.0 fills" in the report subtitle and "CH 1.0" in the fill comparison. Recommended: "Clone Hero 1.0" in sentences and "CH 1.0" in narrow columns, both from one place. "(legacy)" goes. The checkbox stays "1.0 fills"." (docs/audit/2026-10-04-phase-3-5-questions.md:70-72). Answered as D48: "The user answered "all recommended" to all 33 questions ... Each question's "Recommended" line is the decision." (docs/audit/2026-10-03-fix-decisions.md:118). Prose half: D74 item 4 "The "1.0 fills" tooltip uses the shared fill-rule sentences (55)" (fix-decisions.md:325).
- **Code today:** `fill_rule_name(rule, style)` returns "CH 1.0" / "Clone Hero 1.0" (src/search/graph.h:62-64); `fill_rule_description` (graph.h:75). Callers: src/ui/library_dialogs.cpp:80, src/cli/batch.cpp:148, src/app/report.cpp:315, src/app/fill_report.cpp:203-205, src/ui/settings_bar.cpp:25-30. No "(legacy)" string in src/. By reading.
- **Verdict:** PASS
- **Why:** The question listed all four spellings and the one-place rule; the code has one name function with the two styles the answer named.

### 2026-10-03 derivation 58: Two measure-position functions read a meter-change tick differently

- **Splits:** Measure position at the exact tick of a mid-measure meter change: `Timecode` via `section_at` (earlier section, 1.25) vs `measures_at_tick_f` (new section, 1.3333). Today both read one table.
- **How it left:** fixed after a question. Triage: "phase 7 wave 2, merged to main c8c4196 (M7-2c, PV/SE2/TM, 2026-10-05)."
- **User's words:** "**22. The SP gauge on the exact tick of a mid-measure time-signature change.** (58) Today the gauge's drain line has a tiny kink there, because it measures that tick under the new time signature while the engine uses the old one. The end point doesn't move. Recommended: the gauge uses the engine's side, so the kink goes." (docs/audit/2026-10-04-phase-7-questions.md:123-126); D51 call 22: "The SP gauge measures a mid-measure meter-change tick under the engine's side, the earlier section (Q22, 58)." (fix-decisions.md:188).
- **Code today:** `Timecode` sets `measures_decimal_ = count.measures_at(ticks_)` (src/core/timing.cpp:292) and `SongTiming::measures_at_tick_f` returns `count_.measures_at(ticks)` (timing.cpp:337). The count is continuous (`elapsed_` accumulates `(starts_[i] - starts_[i-1]) / tpm`, timing.cpp:216-219), so at tick 2400 in the finding's map both give 1.25. Introduced by 801db20c "Clone Hero's measure grid and continuous measure count (D115 B2-1)". By reading.
- **Verdict:** GONE
- **Why:** Both functions now call the same continuous count, so there is no jump at the change tick and no side to pick.

### 2026-10-03 derivation 59: play_chart.py keeps its own chart parser, MIDI reader and tempo math

- **Splits:** Which notes a chart holds, in which lane, at what time: Hydra's parser and `MsIndex` vs play_chart.py's own parser (e.g. gem inside a tom span: Hydra tom, tool cymbal; `N 5`: Hydra no note, tool green; notes.mid vs notes.chart pick). Tool-only, no screen.
- **How it left:** code-only, fixed. Closure's words: "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, 86, 98, 103, 108, 109, 170, 174, 198 and 202." (docs/audit/2026-10-03-fix-decisions.md:194). Plan row: "PR2 play_chart reads Hydra | 59, and play_chart's share of 79, 80, 85" (docs/superpowers/plans/2026-10-04-phase-7.md:69).
- **User's words:** "**play_chart.py:** it reads the chart through `hydra_replay dump` instead of its own parser, so it plays exactly the notes Hydra analyzed (59)." (docs/audit/2026-10-04-phase-7-questions.md:164), approved as D51 (fix-decisions.md:194, quoted above). The finding itself is a developer-tool copy with no display. (The sheet says `hydra_replay dump`; the tool reads `hydra_replay score`, whose header says a dump file does not work, play_chart.py:3-8. The idea, read Hydra's parse, is the same.)
- **Code today:** `tools/ch_probe/experiments/play_chart.py` header: "The notes come from a `hydra_replay score` JSON, so the auto-player presses ..." (lines 3-5); no `parse_chart` / `parse_midi` / `ticks_to_seconds` remain (grep found none). Commit f3ea6681 "play_chart reads each chord's notes; the score file says the pads once". By reading.
- **Verdict:** GONE
- **Why:** The tool's own parser, MIDI reader and tempo walk are deleted; it consumes Hydra's `hydra_replay` dump, so both sides are one.
- **Side picked without a ruling:** Hydra's parser won; that is a tool-only choice covered by the D51 code-only approval, no display or score involved.

### 2026-10-03 derivation 60: Blank artist or charter shows blank for .ini/.sng but placeholder for .srb

- **Splits:** What a present-but-blank artist/charter is: song.ini and .sng blank vs .srb `<unknown artist>`. Today one helper decides after all three readers.
- **How it left:** question in chat (batch). Triage: "artist_or_unknown and charter_or_unknown sit beside title_or_unknown; discover_charts applies all three once. Fixed by af26153."
- **User's words:** Question (chat, session bbf41dc4): "Approve the small-items batch as recommended (60, 61, 258/319, 331, R7.5, R7.7, 77 fixes; 141 and 320 recorded only)?" chose "Approve all (Recommended)". Batch text: "Blank charter (finding 60). 56 library charts have `charter =` left blank. Today those show an empty Charter cell. `.srb` songs and blank titles already show a placeholder instead. The fix shows `<unknown charter>` for all of them, through one shared helper." (the batch text was not kept; the answer is on card `docs/sessions/2026-10-04-bbf41dc4.md`:27). D27: "blank artist or charter shows the placeholder through one helper next to `title_or_unknown` (60; 56 charter cells)" (docs/audit/2026-10-03-fix-decisions.md:65).
- **Code today:** `r->artist = artist_or_unknown(trim(r->artist)); r->charter = charter_or_unknown(trim(r->charter));` applied once to every scan item whatever the format (src/app/analysis.cpp:715-716); helpers return the placeholder when empty (src/parse/song.cpp:570-576). By reading.
- **Verdict:** REOPEN
- **Why:** The question the user saw described only the blank charter (56 cells); it never said a blank artist would now show `<unknown artist>` too, and blank-versus-placeholder for the artist is the same user-visible split. Low weight: the artist rule is the identical twin of the charter rule the user did see.
- **Examples:** Low end: song.ini `artist =` blank. Before: empty Artist cell. Now: `<unknown artist>` (shown as "(unknown)" by display_artist per D56 item 2, analysis.cpp:711). High end: song.ini `artist = X` shows X both ways. Gap: .sng with artist "" now shows the placeholder, where before it showed a blank; the question named only the charter column.

### 2026-10-03 derivation 61: A container's notes entry recognised by name in .sng but by extension in .srb

- **Splits:** What counts as a container's notes entry: .sng takes only the exact names notes.mid / notes.chart (`pick_notes_file`, src/parse/chart_files.cpp:21-36) vs .srb, whose odd names are decided by the stream's MThd bytes (src/parse/song.cpp:2005-2012). Both still exist today.
- **How it left:** fixed, with "accepted by the plan". Triage: "Still there: .srb still sniffs MThd bytes for an odd name by design; accepted by the plan."
- **User's words:** Chat batch (same "Approve all (Recommended)" as 60): "**One rule for container entries (finding 61).** `.sng` and `.srb` would find their notes file the same way." (the batch text was not kept; the answer is on card `docs/sessions/2026-10-04-bbf41dc4.md`:27). D27: "one container-entry rule (61)" (fix-decisions.md:65). The plan then kept the .srb byte sniff: "An `.srb` keeps its byte sniff as the fallback for a name outside the rule" (docs/superpowers/plans/2026-10-03-step2-parser-rules.md:3673).
- **Code today:** `.sng` odd entry name: `notes_file_format` gives None, so no notes file is picked and the load fails. `.srb` odd name: `is_midi_header(...)` decides (song.cpp:2005-2012, comment "Unexpected filename: sniff the payload instead"). By reading.
- **Verdict:** REOPEN
- **Why:** The words the user saw said the two containers "would find their notes file the same way". They do not: the name rule is shared, but the .srb still sniffs bytes and the .sng does not. "By design" and "accepted by the plan" are not a user ruling.
- **Examples:** End one: notes entry `notes.mid` loads in both. End two: no notes entry in either fails in both. Gap: an entry named `song.mid` with MIDI bytes: .srb loads it as MIDI, .sng fails with "No chart files found in SNG file." (finding text). Weight is low: the audit found no real container with such names.

### 2026-10-03 derivation 62: songmeta keeps one song length per chart, overwritten by the last difficulty analyzed

- **Splits:** How long the song is for the record viewed: Expert's last note vs Easy's last note, depending on which difficulty was analyzed last. Today no per-difficulty or last-note length exists.
- **How it left:** question, then reversed by a later ruling. Triage: "D69 then replaced that with one audio length per song: AL1 0711fd4 and AL2 2c4b63e, merged 2026-10-05."
- **User's words:** Q9: "Today Hydra stores one length per chart. If you analyze Expert and then Easy, the Expert timeline ends at Easy's last note, and late activation dots pile up on the right edge. Recommended: store a length per difficulty" (docs/audit/2026-10-04-phase-7-questions.md:55-57; D51 call 9). Then D69: "The user asked why Hydra derives a song's length from the chart's last note at all, and decided: use the audio length for everything. This replaces D51 call 9 (a length per difficulty)" and "What goes away. ... the per-chart overwrite rule finding 62 described." (fix-decisions.md:283,287). D255 later made the chart's end event the song's end (fix-decisions.md:898).
- **Code today:** `songmeta` no longer appears in src/store/record_store.cpp; the length is `song_length_ms(chart_timing_meta(...), song)` (src/app/analysis.cpp:862) and read, not derived from a difficulty's last note, by `build_activations` (src/app/path_view.cpp:205-226). By reading.
- **Verdict:** GONE
- **Why:** The per-chart overwritten last-note length is removed; one song-level length is used everywhere, by the user's ruling D69.

### 2026-10-03 derivation 63: Duplicate chart copies named by the first copy at scan, the last saved after analysis

- **Splits:** Which copy names a chart: first copy listed by the scan vs whichever copy was analyzed or saved last. Today only the first listed copy names it.
- **How it left:** question, answered "all recommended" (D51 call 10), refined by D63 and D76/D77. Triage: "phase 7 wave 2, merged to main 6e980ed (M7-2b, ST2, 2026-10-05)."
- **User's words:** Q10: "Today the copy that names the chart flips. A rescan shows the first folder's song.ini names. An analysis shows the names of whichever copy was analyzed last. A batch also analyzes the chart once per folder. Recommended: the first copy the scan lists always names it, and a batch analyzes each chart once." (docs/audit/2026-10-04-phase-7-questions.md:59-61). D51: "The user answered "all recommended" to every question" (fix-decisions.md:156); call 10: "The first copy the scan lists names a duplicate chart, and a batch analyzes each chart once (Q10, 63)." (line 170).
- **Code today:** `ChartIndex::from_rows` - "The first row of a chart is its naming copy." (src/store/chart_index.h:21-22); reports and the batch name charts through `index.naming(id)` (src/app/analysis.cpp:850, src/app/report.cpp:113, src/app/fill_report.cpp:248, src/app/dm_report.cpp:95); no `upsert_song` name overwrite remains. By reading.
- **Verdict:** PASS
- **Why:** The question named both rules (first folder's names at scan, last-analyzed copy's names after) and the recommended single rule; the code names every chart from its first scanned copy. (D76/D77 changed the batch count to every copy, which is a different question, with its own ruling.)

### 2026-10-03 derivation 64: A late dynamics tag leaves early notes plain while the tab says dynamics are on

- **Splits:** Is dynamics on for a MIDI chart: per-note gate at the note's tick vs chart-level flag the tab shows. Today the tab reports the late start.
- **How it left:** question in chat. Triage: "D24: is_dynamics_marker takes Clone Hero's two spellings in file order; the tab says 'Dynamics enabled: from <time> on (N earlier markings ignored)'. Fixed by a419b22."
- **User's words:** Question: "Dynamics tab when the tag comes after some marked notes: what should the 'Dynamics enabled' line say?" chose "'from 4:01 on (N ignored)' (Recommended)" (session bbf41dc4; this question and answer were not kept, and its card `docs/sessions/2026-10-04-bbf41dc4.md` does not hold them). D24: "When the tag comes after marked notes, the Dynamics tab says "Dynamics enabled: from <time> on (N earlier markings ignored by Clone Hero)" (3 library charts)" (docs/audit/2026-10-03-fix-decisions.md:59).
- **Code today:** `dynamics_enabled_text` returns "Dynamics enabled: from " + time + " on (" + count + " ignored by Clone Hero)" when `late_tag_ms` is set (src/ui/dynamics_tab.cpp:160-166); notes before the tag are priced Normal (`note.dynamictype = dynamics_enabled_ ? dyn : NoteDynamicType::Normal`, src/parse/song.cpp:825). By reading.
- **Verdict:** PASS
- **Why:** The user was shown the late-tag case and picked how the tab should tell it; the code prices early notes plain and the tab now says so, so display and score agree.

### 2026-10-03 derivation 65: Results made under other rules are deleted, though the docs promise they come back

- **Splits:** Does a result made under rules B delete the rules-A result: docs/ADR 0014 say no (comes back) vs `write_row` purges. Today the key keeps both.
- **How it left:** question, answered "all recommended". Triage: "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)."
- **User's words:** Q8: "You edit hydra_rules.ini, re-analyze three songs, then undo the edit. Those three songs read "Not analyzed", and their results at every other cap are gone too. ADR 0014 and the User Guide both promise the old results come back. After: results under other rules stay in hydra.db and read Ready again when the rules match. ... Recommended: keep them, as the docs already promise." (docs/audit/2026-10-04-phase-7-questions.md:50-53). D51 call 8: "Results made under other rules are kept, and read Ready again when the rules match (Q8, 65)." (fix-decisions.md:168).
- **Code today:** the profile table's unique key is `UNIQUE (chartmode, sp_cap, <lens columns>, hyversion, rules_fp)` (src/store/record_store.cpp:252), so a rules-A and a rules-B record sit side by side. By reading.
- **Verdict:** PASS
- **Why:** The question named both behaviours (delete today, keep after) and the code's key holds the rules fingerprint.

### 2026-10-03 derivation 66: Two INI line readers treat comments and bad lines differently

- **Splits:** (a) `key=Hard # practice`: settings file kept "# practice" (silently Expert) vs rules file stripped it. (b) A line with no `=`: settings skipped vs rules threw. (c) A whole-number key set to `abc`: settings `atoi` reads 0 vs rules file refuses.
- **How it left:** a question for (a) only, answered "all recommended". Triage: "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)."
- **User's words:** "**15. Two small settings-file changes.** (66, 140) A `# comment` after a value in hydra_settings.ini would be ignored. Today `view_difficulty=Hard # practice` silently becomes Expert. The hit window could also hold a decimal like 85.5, where today it's cut to 85. Recommended: yes to both." (docs/audit/2026-10-04-phase-7-questions.md:93-96); D51 call 15: "A `#` comment after a value in hydra_settings.ini is ignored, and the hit window may hold a decimal (Q15, 66 and 140)." (docs/audit/2026-10-03-fix-decisions.md:181). For (c) the concept audit's Q8 said of the settings file "junk reads as 0 by design" but asked only about the rules file (docs/audit/2026-10-11-concept-audit/questions.md:89,93; D114 Q8, fix-decisions.md:559).
- **Code today:** both files now call `split_ini_line` (src/app/config.cpp:263, src/app/rules_file.cpp:86), so (a) is one rule. Still different: settings skips a "="-less line ("A blank, comment-only or "="-less line is skipped." config.cpp:262) while the rules file throws `is not key = value` (rules_file.cpp:88-92); settings reads a whole number with `std::atoi(...)` "Junk reads as 0" (config.cpp:239-240) while the rules file refuses it. By reading.
- **Verdict:** REOPEN
- **Why:** The question named only the comment case. Rows (b) and (c) of the finding's own table are still two answers to one question and were never put to the user; the only words on them call junk=0 "by design".
- **Examples:** End one: `view_difficulty=Hard # practice` is Hard in the settings file now (asked, ruled). End two: a valid `key=value` line reads the same in both. Gap: a hand-edited line `some_whole_number_key=abc` becomes 0 and then the nearest edge of its range in hydra_settings.ini, but stops Hydra with an error in hydra_rules.ini; a stray line `junk` is silently skipped in the first and refused in the second.

### 2026-10-03 derivation 67: Library search and report-page search match rows by different rules

- **Splits:** Does this row match the typed text: Library `fold_for_search` / `query_matches` (accent folding, per word, field prefixes and quotes) vs the report pages' JS filter. Accents and word-by-word are now shared; quotes and field prefixes are not.
- **How it left:** question, answered "all recommended". Triage: "Phase 3 S1: report pages fold accents and match words like the Library (M_D 57dcf23)."
- **User's words:** "**31. Searching on report pages.** (67) Today typing "beyonce" on a report page misses "Beyoncé". "beyonce halo" misses a row that has one word in each field. The Library finds both. Recommended: the pages search the way the Library does." (docs/audit/2026-10-04-phase-3-5-questions.md:146-149); D48: "The user answered "all recommended" to all 33 questions" (fix-decisions.md:118).
- **Code today:** the page filter folds the typed text and requires every word in the row's pre-folded `search` text (src/app/html_page.cpp:341-345). Its comment: "Quotes and the Library's field prefixes (artist:, title:, charter:) are ordinary words here; only the Library reads its full query language." (src/app/html_page.cpp:337-339). By reading.
- **Verdict:** REOPEN
- **Why:** The Recommended line says the pages search "the way the Library does", but the pages do not read the Library's quotes or `artist:` / `title:` / `charter:` prefixes. The question's two examples (accent, one word per field) both hold; the broader promise does not, and nobody asked about the rest.
- **Examples:** End one: "beyonce" finds "Beyoncé" on both. End two: "beyonce halo" across title and artist finds the row on both. Gap: typing `artist:beyonce` in the Library filters the artist field; on a report page `artist:beyonce` is one ordinary word that no row's search text contains, so it finds nothing. Low weight: the audit found no user-facing text claiming the pages take prefixes.

### 2026-10-03 derivation 68: hydra_replay accepts 'true' and 'yes' for flags; the settings file accepts only '1'

- **Splits:** Which text counts as "on" for a yes/no option: `flag_bool` "1"/"true"/"yes" (tools/replay.cpp) vs settings file "1" only. Today one `parse_bool`.
- **How it left:** code-only list, approved as written. Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** "Its on/off flags take 0 or 1 only, as its help says (68)." (docs/audit/2026-10-04-phase-7-questions.md:151), approved as D51: "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, ..." (docs/audit/2026-10-03-fix-decisions.md:194).
- **Code today:** `parse_bool` returns true only for "1" and false only for "0", else nullopt (src/core/strutil.cpp:118-122). hydra_replay: `if (!on) throw ... " takes 0 or 1, not \""` (tools/replay.cpp:743-745); the settings file keeps the setting as it was for anything else (src/app/config.cpp:237). By reading.
- **Verdict:** PASS
- **Why:** The sheet line names the narrowing ("0 or 1 only"), which is exactly what the code does; "true" and "yes" are what stop working, and that is the visible change.

### 2026-10-03 derivation 69: Progress bars disagree on how full a bar is when the total is 0

- **Splits:** A progress bar at 0 of 0: scan modal full vs batch strip empty vs Preview loader 0.10. Today one helper.
- **How it left:** question, answered "all recommended". Triage: "progress_fraction makes 0 of 0 read empty on every bar (M_D 57dcf23)."
- **User's words:** "**19. A progress bar at 0 of 0.** (69) Today the scan dialog draws 0 of 0 as full, the batch strip draws it empty, and the Preview loader sits at 8%. Recommended: empty, meaning "nothing reported yet". The scan's Done line already says "0 charts found" in words." (docs/audit/2026-10-04-phase-3-5-questions.md:94-96); D48 (fix-decisions.md:118). (The finding's table says the Preview sat at 0.10; the question said 8%; the number is a small mismatch in the question, not in the rule.)
- **Code today:** `progress_fraction` returns 0 when `!(total > 0.0)` (src/app/display_format.h:65-68), called by widgets.h:87, library_dialogs.cpp:449, preview_load_job.cpp:261, report_window.cpp:179, version_bar.cpp:107. By reading.
- **Verdict:** PASS
- **Why:** The question named all three bars and the empty rule; every bar now reads its fill from one function that gives empty at 0 of 0.

### 2026-10-03 derivation 77: Probe scripts disagree on the total hit-window cap, 171.43 vs 170 ms

- **Splits:** Where Clone Hero caps the whole hit window: `watch_window` 171.43 ms vs `passive_probe` / `poll_windows` 170 ms (2 x back). Developer tools only.
- **How it left:** chat question (batch), then "already fixed". Triage: "phase 7 recheck (2026-10-04, D51): already fixed by 5df9139; one whole-window cap constant."
- **User's words:** Batch text: "**Probe tools (finding 77).** The probe scripts would use the measured hit-window cap of 171.43 ms. Today `poll_windows.py` would wrongly report "NO CLAMP" on real data." chose "Approve all (Recommended)" (the batch text was not kept; the answer is on card `docs/sessions/2026-10-04-bbf41dc4.md`:27). D27: "probe tools use the measured 171.43 ms cap (77)" (docs/audit/2026-10-03-fix-decisions.md:65); D28: "cap 171.43 ms, floor 96.29 ms, both gaps used" (line 67).
- **Code today:** `WINDOW_CAP_MS = 171.4313` once (tools/ch_probe/constants.py:242). The 170 survives only as `CAP_FROM_GAP_MS = 2 * EXPECT_NORMAL_BACK_MS  # 170`, the gap at which a note reads the cap (constants.py:252), a different question. `poll_windows.py` no longer exists (experiments/ listing). By reading.
- **Verdict:** GONE
- **Why:** The cap value has one home; the other 170 answers "how wide must the gaps be", not "what is the cap". The user's batch words named the 171.43 side and the wrong "NO CLAMP" result of the other.

### 2026-10-03 derivation 78: Experiment scripts each rewrite the live-engine pick their own way

- **Splits:** Which engine object is the live one: `find_live_engine` vs four older scripts' own filters. Developer tools only.
- **How it left:** code-only list approved as written, scripts deleted. Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** "The six old diagnostic scripts are deleted: hit_detect, find_clock3, poll_windows, find_engine, milestone2 and test_attach. walk_edges and watch_window replaced them, and test_attach can kill Clone Hero (78, 83, 84, 174)." (docs/audit/2026-10-04-phase-7-questions.md:159), approved in D51 (fix-decisions.md:194, "approved as written ... In particular, the six old probe scripts are deleted.").
- **Code today:** tools/ch_probe/experiments/ holds none of hit_detect, find_clock3, poll_windows, find_engine, milestone2, test_attach; `MODULE_SPAN` appears only in tools/ch_probe/engine_finder.py:27,192. By reading (directory listing and grep).
- **Verdict:** GONE
- **Why:** The four copies are deleted, so `find_live_engine` is the only pick.

### 2026-10-03 derivation 79: Probe runners wait for song time with separate loops; play_chart presses up to 2 ms early

- **Splits:** When the press fires and when a song counts as stopped: 4 separate loops (stall 5 s vs 8 s, 2 ms lead in play_chart) vs one. Today one `live.wait_until`.
- **How it left:** code-only list approved as written. Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** "The runners share one wait loop that gives up after 5 s of a frozen clock. The edge runners fire at the target time. play_chart keeps its 2 ms early press as a named constant, so its playing doesn't change." (docs/audit/2026-10-04-phase-7-questions.md:160); approved in D51 (fix-decisions.md:194).
- **Code today:** `live.wait_until` and `STALL_S = 5.0` (tools/ch_probe/experiments/live.py:60,117); walk_edges, active_probe, play_chart and watch_window call it (walk_edges.py:239, active_probe.py:135, play_chart.py:194, watch_window.py:325 `live.ClockFrozen`); play_chart passes `lead_ms=PRESS_LEAD_MS` with `PRESS_LEAD_MS = 2.0` (play_chart.py:66,195). By reading.
- **Verdict:** PASS
- **Why:** The approved sentence names the shared loop, the 5 s stall and the one deliberate leftover (play_chart's 2 ms lead), and the code is exactly that.

### 2026-10-03 derivation 80: Starting mid-song, three runners skip different notes

- **Splits:** Which notes are too late when a run joins mid-song: walk_edges "at least 150 ms ahead", active_probe "more than 150 ms ahead", play_chart "up to 50 ms behind". Today one cursor.
- **How it left:** code-only list approved as written. Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** "They share one start-cursor rule: skip notes less than 150 ms ahead." (docs/audit/2026-10-04-phase-7-questions.md:161); approved in D51 (fix-decisions.md:194).
- **Code today:** `START_LEAD_MS = 150.0` and `first_note_index` skip notes "less than this far ahead" (tools/ch_probe/experiments/live.py:151-162); used at walk_edges.py:227, active_probe.py:137, play_chart.py:175,203. By reading.
- **Verdict:** PASS
- **Why:** The approved sentence names the single 150 ms rule and the code has exactly it, in all three runners. (The text does not mention play_chart's old 50 ms-behind rule, but the rule it names is the one in the code, and the tools are developer-only.)

### 2026-10-03 derivation 84: Five scripts use five tolerances to decide the window value changed

- **Splits:** How far a stored window must move to count as changed: 1e-6 ms (watch_window) vs 0.001 (poll_windows) vs 0.1 (hit_detect) vs round-4 (find_engine) vs round-2 (milestone2). Developer tools only.
- **How it left:** code-only (scripts deleted, one helper). Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** Four of the five scripts are in the approved deletion line quoted at 78 ("hit_detect, find_clock3, poll_windows, find_engine, milestone2 and test_attach ... (78, 83, 84, 174)", docs/audit/2026-10-04-phase-7-questions.md:159). The text says "one hit-offset formula and one window-edge finder (79 to 82)" (line 162) and never names a tolerance value.
- **Code today:** `WINDOW_CHANGE_TOLERANCE_MS = 1e-6` and `window_changed(a, b)` in tools/ch_probe/constants.py:263 and tools/ch_probe/experiments/live.py:178-180 are the one comparison. By reading.
- **Verdict:** GONE
- **Why:** Four of the five readers are deleted and the survivor's 1e-6 is the only tolerance.
- **Side picked without a ruling:** watch_window's 1e-6 ms won. No quoted ruling names it; the approved sheet text names only the deletions. Developer tool only, no screen, score or record.

### 2026-10-03 derivation 85: hit_detect presses chords its own way and plays 2x kick on O, not L

- **Splits:** Which key plays a 2x kick and how long a chord is held: `input_driver` / play_chart L and 3 ms vs hit_detect O and 5 ms. Developer tools only.
- **How it left:** code-only list approved as written; hit_detect deleted. Triage: "phase 7 wave 1, merged to main 6c65208".
- **User's words:** "The 2x kick stays on L, since either pedal hits either kick (85)." (docs/audit/2026-10-04-phase-7-questions.md:163); approved in D51 (fix-decisions.md:194).
- **Code today:** `Lane.KICK: 0x4C,  # 'L' (orange)` with the comment that ch_probe presses L for the 2x kick too (tools/ch_probe/input_driver.py:62-70); play_chart maps `("Kick", False)` to `Lane.KICK` (play_chart.py:81); hit_detect.py no longer exists. By reading.
- **Verdict:** GONE
- **Why:** The O-key press loop was in a deleted script, so only the L / 3 ms side is left, and the approved sentence names that side. The reason given ("either pedal hits either kick") is a game fact I did not verify.

## Not reached

None. All 22 ids have a block. Items 77 to 85 were checked more lightly than 49 to 69: I did not diff every probe script, only confirmed by directory listing and grep that the named scripts are gone and the shared helpers exist in tools/ch_probe/experiments/live.py.

## Side notes

- Finding 61's split (extension vs exact name vs byte sniff for a container's notes entry) was re-found by the concept audit as M62 (docs/audit/2026-10-11-concept-audit/findings.md:1400) and is still open in code (src/parse/song.cpp:2005-2012 vs src/parse/chart_files.cpp:21-36).
- Finding 53 closure text is stale in docs: ADR 0023 still lists "A generated fill's length, when two or more meter changes fall between two chords. Hydra's walk reads the meter one change late there." under "Not verified against Clone Hero" (docs/adr/0023-chart-rules-follow-clone-hero-code.md:92-93), but the walk is gone (5677f8d3). The 1.1 generator was always on the measure line (b57a2155).
- Finding 54: the triage row cites functions (`stamped_fill_rule`) that 43276691 later deleted, so the "fixed" evidence in docs/audit/2026-10-03-fix-triage.json for 54 no longer points at code.
- D51 phase-7 sheet line 164 says play_chart reads `hydra_replay dump`; the tool actually requires `hydra_replay score` JSON (tools/ch_probe/experiments/play_chart.py:3-8).
- Finding 69's question said the Preview loader "sits at 8%"; the finding table says 0.10. Small number mismatch inside the question text (docs/audit/2026-10-04-phase-3-5-questions.md:95).
- D26's actual user words (card 2026-10-04-bbf41dc4, "Not now" to "Live Clone Hero check for fill length, 6/8 beats and whether authored fills stop generation ...") are a deferral of a check, not a ruling that names two meter readings; any closure leaning on D26 for a rule choice leans on that deferral only.

<!-- next -->
