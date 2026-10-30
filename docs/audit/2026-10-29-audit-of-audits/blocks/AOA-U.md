# AOA-U: duplicate/doc findings and non-"fixed" rows of the 2026-10-03 derivation audit

### 2026-10-03 derivation audit 136: Preview shows activations in two different golds

- **Splits:** "Which gold marks an activation on the Preview": scrubber marks `kBestPathColor` (250,210,0) (src/ui/theme.h:24, used src/ui/preview_tab.cpp:199) vs next-activation header number, formerly the typed (255,204,51); today the header also uses `kBestPathColor` (src/ui/preview_tab.cpp:723). So the pair the finding names no longer differs. A second, unnamed pair remains: the "SP" label and gauge fill use `kStarPowerColor` (255,204,51) (src/ui/theme.h:26, src/ui/preview_tab.cpp:797, :821), a different gold from the activation gold on the same Preview. See Side notes.
- **How it left:** question. Triage "fixed": "Phase 3 C4b/K4a: kStarPowerColor names the gold and the marks and header read the best-path gold (M_C 16c836c, M_D 57dcf23)." (docs/audit/2026-10-03-fix-triage.json, row 136 status_evidence).
- **User's words:** Q26 "Today the highway floor is teal while Star Power runs, but the drain box text is gold at the same moment. The scrubber's activation marks use one gold (250,210,0) and the next-activation header another (255,204,51)." / "Recommended: teal means "SP running" everywhere, so the drain box text turns teal like the floor (the Onyx look, ADR 0008). Activation marks and the header both use the best-path gold." (docs/audit/2026-10-04-phase-3-5-questions.md:125-127). Answered as D48: "The user answered "all recommended" to all 33 questions in `docs/audit/2026-10-04-phase-3-5-questions.md`." (docs/audit/2026-10-03-fix-decisions.md:118) and "The Preview items Q22 to Q27 take theirs. Position is measured against the last note, the beat lines run to the audio end, teal means SP running, and a chord on the playhead counts as hit." (:134).
- **Code today:** marks and header both read `kBestPathColor`, by reading (preview_tab.cpp:199, :723). The label and gauge use the separately named `kStarPowerColor` whose comment says "A different gold from kBestPathColor." (theme.h:25).
- **Verdict:** PASS
- **Why:** The question quoted both golds with their numbers and the recommendation names which one the marks and header take; the code does that. The label/gauge gold is a different thing (Star Power, not an activation) and the finding itself said it arguably means Star Power; it was not what the question asked, so it is a side note, not a reopen of this row.

### 2026-10-03 derivation audit 138: Backend and Path limit boxes type 500 instead of the engine window

- **Splits:** "What value does a hand-edited out-of-range Backend limit become": the INI loader (then bare `atoi`, -50 stayed -50, shown as 50 rows) vs the box (then clamped -50 to 0). Today both go through one key table: `number("backendlimit_value", ..., 0, kWindowMs)` and `number("mslimit_value", ..., -kWindowMs, kWindowMs)` (src/app/config.cpp:194-196), with `kWindowMs = static_cast<int>(kSqueezeWindowMs)` (config.cpp:156). The three typed 500s agreed at 500 and were a copy, not a split.
- **How it left:** question. Triage "fixed": "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)." (fix-triage.json, row 138). Also D41 for the Path limit ceiling.
- **User's words:** Phase 7 Q14 "Today a bad value in hydra_settings.ini is handled four different ways." ... "`backendlimit_value=-30` shows -30 and hides rows past 30 ms." ... "After: every number setting has one allowed range, the one its box already enforces. A value outside it is pulled to the nearest edge when the file loads, and when you type it." / "Recommended: clamp, matching the boxes." (docs/audit/2026-10-04-phase-7-questions.md:86-91, :84 for the title naming 138). Answered as D51 "The user answered "all recommended" to every question in `docs/audit/2026-10-04-phase-7-questions.md`, including 4b." (fix-decisions.md:156) and call 14 "A hand-edited setting outside its range is pulled to the nearest edge of the range its box enforces, both on load and on typing." (:180). D41: "The Path limit box's ±500 ms ceiling is the engine's squeeze window. The box reads `kSqueezeWindowMs`" (:95).
- **Code today:** the range comes from `kSqueezeWindowMs` for both keys and load/typing share the table, by reading (config.cpp:156, :194, :196). 
- **Verdict:** PASS
- **Why:** The question names the Backend limit example with its real number and says "nearest edge, on load and on typing"; the code clamps exactly that way. (The hit-window key does not follow "nearest edge"; see Side notes.)

### 2026-10-03 derivation audit 139: smallest SP cap of 1 checked in five places

- **Splits:** (a) "May the SP cap be 1?": all five places say yes. No split. (b) "What does a cap below 1 become?": INI file `sp_cap=0` or junk becomes 4 (src/app/config.cpp:133-136, `Outside::DefaultFromFile`, :203); typing 0 in the Settings box becomes 1 (src/ui/settings_bar.cpp:133-135, `Settings::clamp`); `hydra_replay --cap 0` is refused with an error (tools/replay.cpp:164-175). The finding's own table lists these three answers and called it "a policy difference, not drift".
- **How it left:** question for (a); (b) was never asked. Triage "fixed": "The cap floor of 1 is owned by the sp_cap row in Settings' field table, read through Settings::clamp everywhere; replay's message prints kCloneHeroSpCap; engine 2s are kSpActivationBars. Fixed by c8c4196." (fix-triage.json, row 139).
- **User's words:** Phase 7 Q16 "A 1-bar cap can never activate SP, so every path comes back with no activations. Today it's allowed, and nothing explains why there are no activations." / "Recommended: keep it allowed as a what-if, and add one line on the Paths tab: "A 1-bar cap can never activate Star Power."" (phase-7-questions.md:98-100); D51 call 16 "A 1-bar SP cap stays allowed, and the Paths tab says "A 1-bar cap can never activate Star Power." (Q16, 139)." (fix-decisions.md:182). For (b) the only words are the D51 addendum "SE1, the SP cap. 0, junk and `auto` still read as 4. That is the earlier user rule with its own test." (fix-decisions.md:199), whose "earlier user rule" is ui-redesign decision 6, "just drop auto completely, it's unnecessary and confusing. leave the default as 4." (docs/superpowers/plans/2026-09-27-ui-redesign.md:231). That rule is about dropping Auto and the default; it does not say what a typed 0 or a negative number in the box becomes, nor that the box and the file differ. Q14's text ("every number setting ... pulled to the nearest edge ... when the file loads, and when you type it", phase-7-questions.md:106) is the opposite rule for the same kind of input.
- **Code today:** file reads 4, box clamps to 1, replay refuses; the 1-bar line exists (src/app/path_view.cpp:532). By reading.
- **Verdict:** REOPEN
- **Why:** (a) is covered, but the finding's own table holds a second split, what a cap below 1 becomes, and the cited words (drop Auto, default 4; Q14 saying nearest edge) do not name the file-says-4 versus box-says-1 difference.
- **Examples:** INI `sp_cap=0` (old Auto) -> the app runs at 4 bars; type 0 in the SP cap box (`Settings::clamp`) -> 1 bar, which never activates SP; INI `sp_cap=-3` -> 4 bars, box -3 -> 1 bar; `hydra_replay --cap 0` -> refused. At the other end `sp_cap=1` is 1 bar everywhere and `sp_cap=5` is 5 bars everywhere. Code's predicate: `number("sp_cap", &Settings::sp_cap, 1, kNoCeiling, Outside::DefaultFromFile)` (config.cpp:203).

### 2026-10-03 derivation audit 140: hit window default cast to int in three places

- **Splits:** "What is the hit window when the INI says 85.5": Settings/ReportOptions/ReportJob defaults and INI `atoi` gave 85 vs the double defaults in report.h/squeeze_rating.h gave 85.5. Today `double hit_window_ms = kDefaultHitWindowMs;` (src/app/config.h:167) and no `static_cast<int>(kDefaultHitWindowMs)` remains in src/ or tools/ (searched src and tools, .h/.cpp/.py). The probe's two 85 spellings: `EXPECT_NORMAL_BACK_S = 0.085` (tools/ch_probe/constants.py:218) and `EXPECT_NORMAL_BACK_MS = s_to_ms(...)` (:223), now one source.
- **How it left:** question. Triage "fixed": "phase 7: ReportOptions double in RP (3ecc9b8); the setting, key table and ReportJob in LB1 (2c1c41e, D67)." (fix-triage.json, row 140).
- **User's words:** Q15 "The hit window could also hold a decimal like 85.5, where today it's cut to 85." / "Recommended: yes to both." (phase-7-questions.md:94-95), answered as D51 call 15 "A `#` comment after a value in hydra_settings.ini is ignored, and the hit window may hold a decimal (Q15, 66 and 140)." (fix-decisions.md:181).
- **Code today:** the setting is a double read through the key table (config.cpp:199), by reading.
- **Verdict:** PASS
- **Why:** The question named the cut-to-85 and the 85.5 decimal and the code now keeps the decimal everywhere.

### 2026-10-03 derivation audit 218: dimmed grey and hover teal kept as parallel values

- **Splits:** (a) "Which grey is dimmed text": `kDimTextColor` 160,160,160 (src/ui/theme.h:65) vs `kDisabledInputTextColor`, now a reference to the same value (theme.h:82). Same value, no split. (b) "Which teal is the hover colour": frame/header (0,100,100) vs button (0,104,104). Today `kFrameHoveredColor = kButtonHoveredColor` (theme.h:54), used for `FrameBgHovered` and `HeaderHovered` (src/ui/theme.cpp:68, :71).
- **How it left:** question asked in chat (D74). Triage "fixed": "AF1 (D74 item 1), merged to main 47a9eeb (2026-10-05): frames and headers hover in kButtonHoveredColor (0,104,104)." (fix-triage.json, row 218).
- **User's words:** card quotes the question as "Finding 218: frame and header hover is teal 0,100,100 ..." and the user "chose "Match at 0,104,104 (Recommended)"" (session card 2026-10-05-18216767.md:19; the full question text is not in the card). D74: "One hover teal (218). Frames and headers hover in the button teal, 0,104,104, instead of 0,100,100." (fix-decisions.md:322).
- **Code today:** one hover teal, by reading (theme.h:45, :54).
- **Verdict:** PASS
- **Why:** The user chose the option that named both teals with their numbers, and the code uses the one chosen. Note finding 218 is listed under Q26 in the phase 3-5 file, but Q26 says nothing about it; the real ruling is D74 (Q26's heading lists 218 by mistake).

### 2026-10-03 derivation audit 141: .chart solo/disco events match raw text, section names strip quotes

- **Splits:** "How is a quoted `E "solo"` read": gameplay events (solo, soloend, disco) compare the raw word, so `E "solo"` is a plain event (src/parse/song.cpp:1252-1259) vs generic events used for practice-section names strip one pair of quotes (song.cpp:1264-1269). These are two rules for two jobs (gameplay vs section label), and Hydra's gameplay rule is the same as Clone Hero's.
- **How it left:** settled by a ruling, "recorded only". Triage "settled_by_decision": "D27 records it; ADR 0023 states .chart solo/disco events match raw text and section names strip one pair of quotes." (fix-triage.json, row 141 status_evidence).
- **User's words:** the question was asked in chat: "Q: Approve the small-items batch as recommended (60, 61, 258/319, 331, R7.5, R7.7, 77 fixes; 141 and 320 recorded only)?" chose "Approve all (Recommended)" (session card condensed file bbf41dc4-b650-434b-a047-ce3abe26e342.condensed.md:338-339). The batch text shown before it: "**Finding 141.** Gameplay events like `solo` match the raw text, as in Clone Hero. Section names strip their quotes. I'd reject the audit's "one cleanup for every event" idea, because it would turn a quoted `E "solo"` into a solo, which Clone Hero doesn't do." (same file:327). Recorded as D27: "Recorded only: gameplay events match raw text and section names strip quotes, with no shared normaliser (141)" (docs/audit/2026-10-03-fix-decisions.md:65). Clone Hero side: "**Answer: it does not recognise it.**" with "Strength: CH code." (docs/audit/ch-evidence.md:152).
- **Code today:** unchanged, `t.w[1] == "solo"` only for an unquoted word (song.cpp:1252). By reading.
- **Verdict:** PASS
- **Why:** The words the user approved name both behaviours (raw match for gameplay events, quote strip for section names) and say why; ch-evidence.md backs the Clone Hero side with game code, so the closure is a verified rule, not just "kept".

### 2026-10-03 derivation audit 143: which library row is the selected one is decided in three places

- **Splits:** (a) "Which row is the selected one": `relative_row` and two table sites each compared notespath; no disagreement. (b) "Same chart in two folders, A analyzing, open B": `analyze_job_shown` compares notespath so B shows "Wait for <title> to finish analyzing." vs `update_song_length` compares md5 (different question, store key). 
- **How it left:** question for (b), per triage. Triage "fixed": "phase 7 wave 3, merged to main 4868f44 (M7-3d, LB2, 2026-10-05)." (fix-triage.json, row 143). Not cited by id in any question heading.
- **User's words:** None found; searched docs/audit/2026-10-04-phase-3-5-questions.md, -phase-6-questions.md, -phase-7-questions.md, fix-decisions.md for "143" and for "two folders" / "Wait for" (only the 143 triage row and finding text matched).
- **Code today:** (a) one helper: `AppState::is_selected_row` compares `row_key(row) == row_key(*selected)` (src/ui/app_state.cpp:557-560), `row_key` is the notespath (src/ui/library_model.h:82-83), and the table uses it (src/ui/library_table.cpp:486, :509). (b) `analyze_job_shown` and the "Wait for ... to finish analyzing" text appear nowhere in src (searched src/ui and src for both); `git log -S"analyze_job_shown"` last touches it in a82f74ee (2026-10-07, "a click analyzes the song on a worker and saves its summary (D87, D90)"). By reading.
- **Verdict:** GONE
- **Why:** The three row compares are one helper, and the in-progress panel whose identity edge the finding raised was removed in a82f74ee.
- **Side picked without a ruling:** Row identity by notespath (the same chart in two folders is two rows) stays with no ruling; the only source is an agent-written plan comment, as the finding says.

### 2026-10-03 derivation audit 154: activation badge re-works out which part is hardest

- **Splits:** "Which part is hardest when a squeeze and an early fill tie, and how many decimals the ms shows": badge used to round to whole ms ("13") while the copied text truncated ("12"); a tie named the squeeze. Today `Activation::hardest()` picks squeezes first and only a strictly larger value takes over, so a tie keeps the squeeze (src/core/model.cpp:509-514), and both badge and notation text call `format_ms_whole` (src/app/path_view.cpp:74, src/core/model.cpp:556-557; defined :919).
- **How it left:** question. Triage "fixed": "Phase 3 O1/K2: Activation::hardest() gives the badge its part and value (M_D 57dcf23)." (fix-triage.json, row 154).
- **User's words:** Phase 3-5 Q2 "**2. How does a squeeze timing print?** (4, 18, 36, 154)" / "Today one squeeze can read "13 ms" on the activation badge, "12 ms" when you copy the path, and "12.6 ms" on the path button." / "After: whole-ms text rounds to nearest everywhere, so the badge and the copied text agree." / "Recommended: yes. When a squeeze and an early fill tie exactly, the badge keeps naming the squeeze, as today." (docs/audit/2026-10-04-phase-3-5-questions.md:20-24). Answered as D48 (fix-decisions.md:118, :120 "Whole-ms text rounds to nearest everywhere (Q2).").
- **Code today:** by reading, as above.
- **Verdict:** PASS
- **Why:** The question names the tie rule and the rounding with real values, and the code does both.

### 2026-10-03 derivation audit 164: running combo walked separately by four places

- **Splits:** None: four copies of "combo before/after a chord" that gave the same answer (the finding says "There is no edge input where they differ"). Today `category_scores` owns the walk (src/core/scoring.cpp:41, :87 `out.combo_after`), `ScoreGraph::build` reads `sg.combo_before` (src/search/graph.cpp:137-138) and `replay_path` reads `sg.combo_before` / `sg.combo_after` (src/core/replay.cpp:136, :206); the test copy in tests/test_engine_sweeps.cpp:292-294 now reads `chord_score_table`.
- **How it left:** fixed. Triage: "phase 6, merged to main: J2-7 in M6-J2 a1d4c28 (2026-10-05); J3-2 in M6-J3 e9e2eeb (2026-10-05)." (fix-triage.json, row 164). The open question about test oracles was answered by D42.
- **User's words:** chat "Q: Can tests keep frozen copies of old code to compare the new code against (audit R7.30)?" the user chose "Not allowed" (session card 2026-10-04-a9a461da.md:29); D42 "Tests may not keep frozen copies of old code as a reference." (fix-decisions.md:97).
- **Code today:** by reading, one owner.
- **Verdict:** PASS
- **Why:** No behaviour differed, the copies are folded into one owner, and the user's "Not allowed" is the matching ruling for test copies.

### 2026-10-03 derivation audit 174: probe scripts hard-code engine field offsets

- **Splits:** None: literals copied the `constants.py` offsets and all matched. Today tools/ch_probe holds no raw `0x20`/`0x94`/`0x30` offsets outside constants.py/engine.py/interfaces.py comments (searched tools/**/*.py), and the scripts find_engine, poll_windows, find_clock3 and hit_detect are not in tools/ch_probe at all (Glob of tools/ch_probe/**/*.py).
- **How it left:** a question inside the code-only list; the scripts are deleted. Triage "fixed": "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)." (fix-triage.json, row 174).
- **User's words:** Phase 7 "- The six old diagnostic scripts are deleted: hit_detect, find_clock3, poll_windows, find_engine, milestone2 and test_attach. walk_edges and watch_window replaced them, and test_attach can kill Clone Hero (78, 83, 84, 174)." under "## Code-only calls I'll make unless you say otherwise" (docs/audit/2026-10-04-phase-7-questions.md:159, :145). D51: "The user answered "all recommended" to every question in `docs/audit/2026-10-04-phase-7-questions.md`" (fix-decisions.md:156) and "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, 86, 98, 103, 108, 109, 170, 174, 198 and 202. In particular, the six old probe scripts are deleted." (:194).
- **Code today:** by reading (Glob, Grep above).
- **Verdict:** PASS
- **Why:** The finding's open question (delete or keep the old scripts) was put to the user by name and the code matches the answer.

### 2026-10-03 derivation audit 229: old diagnostics rebuild the engine search pattern and module span

- **Splits:** None between the deleted scripts. One remaining pair: `play_chart.py:285`, `poll_cost.py:137` and `find_checksum.py:370` search only `normal_pattern` while the runners use `all_patterns` (tools/ch_probe/experiments/walk_edges.py:214, watch_window.py:300, sp_watch.py:159, active_probe.py:160, passive_probe.py:65). The finding called the precision-mode effect "unproven".
- **How it left:** fixed, with the deletion of the scripts (D51 line 194 quoted under 174). Triage "fixed": "phase 6, merged to main: J3-7 in M6-J3 e9e2eeb (2026-10-05)." (fix-triage.json, row 229).
- **User's words:** same as 174 (docs/audit/2026-10-04-phase-7-questions.md:159; fix-decisions.md:194).
- **Code today:** `MODULE_SPAN` appears only in engine_finder.py (:27, :192); by reading and grep of tools/**/*.py.
- **Verdict:** PASS
- **Why:** The inline copies went with the scripts the user approved deleting; the normal-versus-all-patterns choice left in three dev scripts is a developer tool detail, listed in Side notes.

### 2026-10-03 derivation audit 193: error messages become plain text by matching wording

- **Splits:** None: a design choice (match message text vs carry a kind). Preview failure showed raw exception text; now `preview_controller.h:162` says the panel prints the sentence `app::plain_error` gives for the failure's kind, and "Reinstall Hydra" text exists (src/app/user_messages.cpp:83).
- **How it left:** question. Triage "fixed": "phase 7 wave 4: ER1 merged to main 65b6be5 (errors carry an ErrorKind, one switch in plain_error) and ER2 fe95e6a" (fix-triage.json, row 193).
- **User's words:** Phase 7 Q25 "**25. Plain words for every error.** (193)" / "Today a missing or damaged Preview asset shows raw program text. Other errors become plain sentences only when their exact wording matches a list, and that list keeps falling behind." / "Recommended: every error carries its kind, and each kind has one plain sentence. A Preview asset problem reads "Reinstall Hydra"." (phase-7-questions.md:137-139). D51 call 25: "Errors carry their kind, and each kind has one plain sentence. A Preview asset problem reads "Reinstall Hydra" (Q25, 193)." (fix-decisions.md:191).
- **Code today:** by reading (user_messages.cpp:83; preview_tab.cpp:433 prints `pc->error()`, preview_controller.h:162-163).
- **Verdict:** PASS
- **Why:** The question described the real rule and the fix; the code carries the kind and has the sentence.

### 2026-10-03 derivation audit 202: the report file name is typed twice (GUI and CLI)

- **Splits:** "Where does a report page land": GUI writes into `reports_dir()` (Documents\Hydra, or the db folder) via `report_html_path` vs `hydra_report` with no `--out` writes `hydra_paths.html` in the current folder. The finding: "The file name agrees today, by reading. The folder does not."
- **How it left:** "code-only". Triage "fixed": "phase 7: page-name constants in RP (3ecc9b8); the GUI harness reads them in LB2 (4868f44)." (fix-triage.json, row 202). Not a numbered question.
- **User's words:** the code-only list: "**hydra_report:** `--paths 200000000` says "top 200,000,000 paths" and not "every path" (86). Its default output stays the current folder; the file name comes from one constant (202)." (docs/audit/2026-10-04-phase-7-questions.md:155), approved as D51 "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, 68, 78 to 85, 86, 98, 103, 108, 109, 170, 174, 198 and 202." (fix-decisions.md:194). The words say the CLI stays in the current folder; they never say the app saves its pages in Documents\Hydra, so the user was not shown the two folders side by side.
- **Code today:** not re-read for the file-name constant (see Not reached).
- **Verdict:** REOPEN
- **Why:** The words name only one side (the CLI folder). A low-weight question, since the CLI is a command-line tool.
- **Examples:** `hydra_report chart.mid` with no `--out` -> `.\hydra_paths.html` in whatever folder you are in; the same chart's report from the app -> `Documents\Hydra\hydra_paths.html`. Gap: running the CLI from Documents\Hydra gives the same file as the app, from anywhere else it does not.

### 2026-10-03 derivation audit 207: two 2-second file re-check constants

- **Splits:** None: both constants were 2.0 s and the caching code agreed on every input, including the exact 2.0 edge. Today one constant and one helper: `kFileCheckSeconds = 2.0` and `cached_file_check` (src/ui/app_state.h:394-401), used by `selected_file_ok` (src/ui/app_state.cpp:545-548); `kReportCheckSeconds` appears nowhere in src/ui (grep of the constant names in app_state.h shows only kFileCheckSeconds).
- **How it left:** a code-only call, approved with the phase 6 questions. Triage "fixed": "phase 6, merged to main: J2-4 in M6-J2b 1967e93 (2026-10-05)." (fix-triage.json, row 207).
- **User's words:** D54: "Approved with D53: the "Recorded as recommended" list in `docs/superpowers/plans/2026-10-04-phase-6.md`, as written." and "207: the chart-file and report-file checks share one 2-second interval and one caching helper." (fix-decisions.md:211, :213). D53: "The user answered "use recommended answers to everything" to the four questions" (:205).
- **Code today:** one helper, by reading.
- **Verdict:** PASS
- **Why:** Not a split (same value, same behaviour); the code matches the recorded call.

### 2026-10-03 derivation audit 219: status-line fade is a bare 6.0 seconds

- **Splits:** None: one production place; the test only bounds it. Today `kStatusFadeSeconds = 6.0` (src/ui/app_state.h:409) used at src/ui/library_toolbar.cpp:28.
- **How it left:** fixed (named constant). Triage: "phase 6, merged to main: J2-4 in M6-J2b 1967e93 (2026-10-05). ... Spot-checked in the code 2026-10-05 at 993df71: holds." (fix-triage.json, row 219).
- **User's words:** None found; searched fix-decisions.md and the three question files for "219", "fade" and "6 s"/"6 seconds" (the phase 3-5 Q33 list of UI timings names "0.5 s "Done!", 2.0 s "Copied!", 0.15 s search refilter, 1.0 s batch refresh" and not the 6 s fade, docs/audit/2026-10-04-phase-3-5-questions.md:172).
- **Code today:** the 6.0 got a name; the value is unchanged, by reading.
- **Verdict:** REOPEN
- **Why:** This is a number nobody chose, not a split: the finding's own question, "Is 6 seconds the fade time you want for a news line in the status bar?", was never put to the user, and Q33 confirmed the neighbouring UI timings but left this one out. Low weight.
- **Examples:** a "Saved" news line stays 6.0 s (hidden at `> kStatusFadeSeconds`); a problem line never fades. Gap: Q33's confirmed "Copied!" badge is 2.0 s, the status line 6.0 s, both unasked as a pair.

### 2026-10-03 derivation audit 222: '100 means base speed' typed four times

- **Splits:** "Does a non-100% speed count as a normal run": the percent column hid for speed != 100 while the status/delta lines ignored speed (an off-speed score still counted "above optimal"). Today `kBaseSpeedPercent = 100` with `is_base_speed` (src/net/dmbot_client.h:35-36), used at dmbot_client.cpp:58, dm_report.h:69, dm_report_view.cpp:26, and a separate `kStatusOtherSpeed = "other speed"` status (src/app/dm_report.h:48).
- **How it left:** question in chat. Triage "fixed": "phase 7 recheck (2026-10-04, D51): already fixed by D25; off-speed scores have their own status." (fix-triage.json, row 222).
- **User's words:** chat "Q: Leaderboard page: how should off-speed scores count?" chose "Own 'other speed' status (Recommended)" (bbf41dc4 condensed file:294-295). The text before it: "Clone Hero's points have no speed factor, but it ranks each speed separately. Today an off-speed score can still count as "above optimal". It also counts toward Matched and Points left on table." (same file:275). D25: "Off-speed leaderboard scores get their own "other speed" status and leave the Matched, Above optimal and Points left on table totals." (fix-decisions.md:61). Game side: "**Answer: not through the scoring math.**" (docs/audit/ch-evidence.md:173).
- **Code today:** by reading (grep above).
- **Verdict:** PASS
- **Why:** The question named today's behaviour (off-speed counts as above optimal) and the alternative, and the code does what the user picked.

### 2026-10-03 derivation audit 298: database schema number is written every open but never read

- **Splits:** None: a second statement of the schema level that nothing read. Today `user_version` appears nowhere in src/store (grep of src/store).
- **How it left:** question. Triage "fixed": "phase 6, merged to main: J4-2 in M6-J4 e4833e0 (2026-10-05)." (fix-triage.json, row 298).
- **User's words:** Phase 6 Q1 "**After, option A:** Hydra stops writing the slot. A new database carries 0 there, an existing one keeps its 3, and nothing on any screen, report or stored result changes." / "**Recommended: A.**" (docs/audit/2026-10-04-phase-6-questions.md:9-18); D53 item 1: "The database's `user_version` slot (finding 298): Hydra stops writing it." (fix-decisions.md:206).
- **Code today:** by reading, no write.
- **Verdict:** PASS
- **Why:** Both options were written out with their effects and the code does option A.

### 2026-10-03 derivation audit R7.12: "is this path too long?" asked with two cut-offs

- **Splits:** "When is a path long": `kPlainPathLimit = MAX_PATH - 12` = 248 for file calls (src/core/winstr.cpp:49, :73) vs `fits_shell` = under 260 (:96-97). They answer different questions (file calls need the prefix from 248; the shell takes up to 259). The scattered `MAX_PATH` checks now all go through `fits_shell` (src/app/report_files.cpp:69, :79).
- **How it left:** question. Triage "fixed": "fits_shell in winstr.cpp is the one predicate for shell_path, open_in_browser and copy_to_short_temp; a scan test guards it. Fixed by 419a14d." (fix-triage.json, row R7.12).
- **User's words:** Phase 3-5 Q33 "- Two path cut-offs that answer different questions: 248 characters, past which file calls need the long-path prefix, and 260, the most the Windows shell accepts. Both go in ADR 0020 (R7.12)." (phase-3-5-questions.md:173); D48: "Every number in Q33 is kept as it is and recorded. That includes the 10 ms Path limit on, 50 ms backend hide off, and both path cut-offs (248 and 260)." (fix-decisions.md:137). ADR 0020 records 248 (docs/adr/0020-every-file-path-goes-through-win32-path.md:16).
- **Code today:** by reading.
- **Verdict:** PASS
- **Why:** The question names both cut-offs with their values and what each answers; code and ADR agree.

### 2026-10-03 derivation audit R7.22: the "at least one worker" floor is written three times

- **Splits:** None: the floor of 1 appeared three times with the same answer; the cap of 8 was already decided. `batch_worker_count` is the owner (used at src/app/analysis.cpp:576).
- **How it left:** a recorded call. Triage "fixed": "phase 6, merged to main: J1-6 in M6-J1a 7a4e59e (2026-10-04); J2-4 in M6-J2b 1967e93 (2026-10-05)." (fix-triage.json, row R7.22).
- **User's words:** the phase 6 plan's list "The developer-tool and pool numbers are recorded as they are, D49-style: ... R7.22's worker rule (hardware threads minus one, 1 when Windows reports 0, the same count for hashing)" (docs/superpowers/plans/2026-10-04-phase-6.md:180), approved by D54: "Approved with D53: the "Recorded as recommended" list in `docs/superpowers/plans/2026-10-04-phase-6.md`, as written." (fix-decisions.md:211). I could not confirm the user was shown that plan list itself; D53's trigger was the answer "use recommended answers to everything" to the four questions (:205).
- **Code today:** not read to the end (see Not reached).
- **Verdict:** PASS
- **Why:** No behaviour differs between parts; the three undecided parts of the worker rule are named in the recorded text.

### 2026-10-03 derivation audit R7.30: equivalence tests keep frozen copies of replaced loops

- **Splits:** None: tests, not product. Today `reference_replay_path`, `old_scan` and `reference_build` are absent from tests/ (count 0).
- **How it left:** question. Triage "fixed": "Phase 6 recheck d: 8c537c2 (D42) deleted reference_replay_path, old_scan and reference_build from tests/." (fix-triage.json, row R7.30).
- **User's words:** chat "Q: Can tests keep frozen copies of old code to compare the new code against (audit R7.30)?" chose "Not allowed" (condensed a9a461da:1024, card 2026-10-04-a9a461da.md:29); D42 (fix-decisions.md:97).
- **Code today:** by reading (grep count 0 in tests).
- **Verdict:** PASS
- **Why:** Question and answer name the exact rule and the code matches.

### 2026-10-03 derivation audit R7.39: "Chained files: each link plays in turn" is not what the Opus reader does

- **Splits:** "Which links of a chained Opus file play": the ADRs said each link; the reader keeps links only up to the first with another channel count or no audio page. Today ADR 0006 says "Chained files: the links that match the first link's channel count" (docs/adr/0006-ogg-opus-decoded-by-hand-with-libogg-and-libopus.md:31) and ADR 0019 "plays only the links that match the first link's channel count" (docs/adr/0019-preview-audio-streams-from-the-compressed-file.md:83-84).
- **How it left:** question inside a "record these" list; the docs were corrected. Triage "fixed": "phase 4 (merge ae60cc5, 2026-10-04): ADR 0006 and 0019: only links matching the first link channel count play." (fix-triage.json, row R7.39).
- **User's words:** Phase 3-5 Q33 "- A chained Opus file plays only the links that match its first link's channel count; the ADRs get corrected to say so (R7.39)." (phase-3-5-questions.md:174), D48 "Every number in Q33 is kept as it is and recorded." (fix-decisions.md:137).
- **Code today:** reader code not re-read (see Not reached); ADR text matches the question.
- **Verdict:** PASS
- **Why:** The question states the real behaviour and its fix (correct the ADRs); the ADRs now say it.

### 2026-10-03 derivation audit 341: scrubber marks copy ImGui's hidden 2 px slider padding

- **Splits:** None: one Hydra copy. `draw_scrub_marks` places the marks with `pad = 2.0f` and half a grab at each end (src/ui/preview_tab.cpp:187-199) and nothing else in src places slider marks (searched src for `GrabMinSize`: only preview_tab.cpp:191). The other side is ImGui's internal, which the finding says is hardcoded in third_party and exposes no accessor (I did not re-open third_party/imgui).
- **How it left:** closed, exit 4 "not a problem". Triage: "Phase 6 plan (2026-10-04-phase-6.md): closed as not a fold. The scrubber pad copies an ImGui internal ImGui does not expose, so there is one Hydra copy." (fix-triage.json, row 341 status_evidence). Plan: "341: closed as "single owner, ImGui internal". The scrubber's 2 px pad has one Hydra copy and nothing to call." (docs/superpowers/plans/2026-10-04-phase-6.md:179); D54 (fix-decisions.md:218).
- **User's words:** None needed for a one-copy item. Covered by D54's "use recommended answers" list as a record only.
- **Code today:** by reading, one Hydra copy (above).
- **Verdict:** PASS
- **Why:** One sentence holds: Hydra's marks are the only code placing them, and the ImGui side is third-party code that cannot be called. The finding's suggested GUI test against the real grab rect was not checked.

### 2026-10-03 derivation audit 53: generated fill length reads the meter one change late

- **Splits:** "Which meter sets a generated fill's length when two meter changes fall between two chords": Song::check_activations' own walk (one change behind) vs `MeasureIndex::section_at` (current). The old walk is gone. Today `place_fills_ch11` (src/core/song.cpp:182-225) sets the fill's length as chord tick minus the measure-line tick and reads no meter, and `place_fills_ch10` (src/core/song.cpp:135-167) takes the length from `grid.length_at(grid.span_at(chord.timecode.ticks()))`, the current grid measure at the chord. Both read `MeasureIndex`, so the two-change input now gives the "current" answer in 1.0 mode.
- **How it left:** "settled_by_decision", D26. Triage: "D26 keeps today's rule as unverified against Clone Hero; ADR 0023 lists it." (fix-triage.json, row 53 status_evidence).
- **User's words:** chat question "Live Clone Hero check for fill length, 6/8 beats and whether authored fills stop generation: when?" chose "Not now" (bbf41dc4 condensed file:298-299). The text before it asked about "which time signature sets a generated fill's length" (same file:277-278). D26: "No live Clone Hero check now. Generated-fill length and 6/8 beats keep today's rule, written down as unverified against Clone Hero." (fix-decisions.md:63). ch-evidence.md:92: "**Answer: unknown.** I could not find Clone Hero's fill generator."
- **Code today:** the one-change-late walk and its `pre_tpm` are gone (grep of src for `pre_tpm`, `bpm_pos`, `section_at` finds no parse/song.cpp walk; only src/core/song.cpp:105-225). The dead tempo walk was removed by 6a98db3b (2026-10-11); the whole meter walk was replaced by `MeasureIndex` lines in b57a2155 (2026-10-14, 1.1 generator) and 5677f8d3 (2026-10-24, 1.0 generator, "Clone Hero 1.0 fills count measures on the shared grid"). By reading.
- **Verdict:** GONE
- **Why:** Commits b57a2155 and 5677f8d3 removed the private walk; every fill length now comes from the one grid.
- **Side picked without a ruling:** In 1.0 mode the "current meter" side won (src/core/song.cpp:163). 5677f8d3's message says "The 1.0 fill rule itself is unchanged", which is not true for a chart with two meter changes between chords (old: one change behind; now: current). D26 said to keep the old rule. No user words name this choice. ADR 0023:92-93 still says "Hydra's walk reads the meter one change late there." (stale).

### 2026-10-03 derivation audit 307: an exact tie with the cap ceiling is not a clamp

- **Splits:** None: one place decides. `ScoreGraph::extend_deacts` counts a clamp only when the ceiling is strictly earlier: `const bool clamped = ceiling.ticks() < ext.ticks();` (src/search/graph.cpp:314). Nothing else copies it.
- **How it left:** question, "settled_by_decision". Triage: "D51 call 6 keeps strict-earlier clamp counting; the graph.cpp comment states the tie rule." (fix-triage.json, row 307).
- **User's words:** Phase 7 Q6 "**6. A phrase that fills the SP meter exactly to the cap: is that an overfill?** (307)" / "Today it isn't. The meter is full, nothing is lost, and no overfill note appears. No decision records this." / "Recommended: keep, and record it." (docs/audit/2026-10-04-phase-7-questions.md:40-43). D51 call 6: "A phrase that fills the meter exactly to the cap is not an overfill (Q6, 307)." (fix-decisions.md:166).
- **Code today:** strict `<` at graph.cpp:314, comment at :310 "min(ext, ceiling): ceiling only when it is strictly earlier". By reading.
- **Verdict:** PASS
- **Why:** The question states the exact-tie rule in game terms and the user took "keep, and record it"; the code does exactly that.

### 2026-10-03 derivation audit 314: a beat is always a quarter note in every meter

- **Splits:** None inside Hydra for the deadline: `activation_fill_deadline_ms` subtracts `kCh11FillLeadBeats * timing.tick_resolution()` (src/search/graph.cpp:68-69, `kCh11FillLeadBeats = 4` at src/search/graph.h:71). The open part is against Clone Hero, not between two parts of Hydra. Note: the code's grid "Beat" is now denominator-based, so the Preview beat lines in 6/8 are eighth notes while the fill deadline counts quarter notes (CONTEXT.md:731-737, "Two rules count quarter notes in every meter instead"); that difference is written down.
- **How it left:** "settled_by_decision", D26. Triage: "D26 keeps a quarter-note beat in every meter as unverified; ADR 0023 and CONTEXT.md record it." (fix-triage.json, row 314).
- **User's words:** same chat question as 53, "whether beats count as quarter notes in 6/8" and the answer "Not now" (bbf41dc4 condensed file:277-279, :298-299); D26 "6/8 beats keep today's rule, written down as unverified against Clone Hero." (fix-decisions.md:63). ch-evidence.md:207 "**Answer: unknown.** I did not find the code that checks the 1.1 fill deadline ("4 beats")."
- **Code today:** as above, by reading. ADR 0023:94-96 still lists it under "Not verified against Clone Hero". CONTEXT.md:735-737 says the two quarter-note rules are "both as Clone Hero's code does", which contradicts the ADR (see Side notes).
- **Verdict:** PASS
- **Why:** No two parts of Hydra answer differently; the user was told the exact open question (quarter notes in 6/8) and chose to leave it unverified, and it is recorded as unverified. The CONTEXT.md sentence overstates it.

### 2026-10-03 derivation audit 317: Auto cleanup recognises only default-ladder rows

- **Splits:** "What happens to an old Auto row": default-ladder rows deleted at first start vs custom-ladder rows read Stale until re-analysed. Today neither exists: `retired_auto_fingerprint` and `delete_auto_results` appear nowhere in src (searched src for `retired_auto`, `delete_auto_results`, `auto_cap_ladder`; the only hit is the ignored-key list src/app/rules_file.cpp:24).
- **How it left:** question, "settled_by_decision". Triage: "D51 call 13 records that custom-ladder Auto rows read Stale until re-analyzed." (fix-triage.json, row 317).
- **User's words:** Phase 7 Q13 "- Old Auto results made with a custom cap ladder read Stale until re-analyzed. Only default-ladder ones are deleted at first start." / "Recommended: record all five as they are." (phase-7-questions.md:76, :80; the list has six bullets, the Recommended line says "five"). D51 call 13: "Custom-ladder Auto rows read Stale until re-analyzed (317)." (fix-decisions.md:175).
- **Code today:** the Auto cleanup was deleted in 59b77bc7 (2026-10-14), message: "The copy upgrade, its recovery, the schema 2/3 rebuild, the Auto cleanup and retired_auto are gone." A library file older than 2.2.0 is now set aside as a whole (`OldFilePolicy`), by reading the commit message.
- **Verdict:** GONE
- **Why:** Commit 59b77bc7 removed the cleanup; no Auto rows survive either way.
- **Side picked without a ruling:** Neither of the two old behaviours won; an old library file is set aside whole (D115, the user's one-record-per-chart design). I did not find the user's words naming that Auto rows go with it.

### 2026-10-03 derivation audit 320: .chart files always count ghosts and accents

- **Splits:** None: single place, `song.dynamics_enabled = true` for a .chart (src/parse/song.cpp:1893). The open part was whether Clone Hero does the same.
- **How it left:** "settled_by_decision", D27 "recorded only". Triage: "D27 records that .chart ghosts and accents always count; CONTEXT.md states it." (fix-triage.json, row 320).
- **User's words:** batch shown in chat: "**Finding 320.** `.chart` ghosts and accents always count, as in Clone Hero." then "Approve all (Recommended)" (bbf41dc4 condensed file:328, :338-339). D27: "`.chart` ghosts and accents always count (320)." (fix-decisions.md:65). Evidence: "**Answer: yes.** Strength: CH code." (docs/audit/ch-evidence.md:249).
- **Code today:** by reading, ungated for .chart (song.cpp:1893); the triage text "Clone Hero's reading is unverified" is out of date for this row.
- **Verdict:** PASS
- **Why:** The user was told the rule and Clone Hero's code was read to confirm it; Hydra matches.

### 2026-10-03 derivation audit 321: the scan reads .sng and .srb names from only the first 1 MB

- **Splits:** "Where does a .sng/.srb's title and artist come from": scan's first-1-MB prefix (`kSngHeadCapture`) vs Preview's `sng_delay_ms` and the loader's `load_songpath_srb`, which read the whole metadata. The scan's prefix cap is gone. Today the scan calls `sng_read_head` (src/app/analysis.cpp:293, :441) and `srb_read_metadata` (:302), the same readers the loader uses (src/parse/song.cpp:1986, :1995) and the Preview uses (src/app/preview_source.cpp:212).
- **How it left:** question, "settled_by_decision". Triage: "D51 call 13 records the 1 MB name capture for .sng/.srb; stored_versions.h cites it." (fix-triage.json, row 321).
- **User's words:** Phase 7 Q13 "- The scan reads a .sng or .srb's title and artist from its first 1 MB." / "Recommended: record all five as they are." (phase-7-questions.md:77, :80). D51 call 13 "The scan reads .sng and .srb names from the first 1 MB (321)." (fix-decisions.md:176). The question did not say the Preview and the loader read the whole block.
- **Code today:** `sng_read_head` keeps reading until the file table fits (src/parse/sng.cpp:115-127), no 1 MB cap; `.srb` metadata is capped at `kSrbMaxMetadata = 1 << 20` (src/parse/srb.h:47) for every caller alike. `kSngHeadCapture` appears nowhere in src. The commit that removed it is 36be3a70 (2026-10-10, "A chart's id is Clone Hero's song id for .sng and .srb too"; found with `git log -S"kSngHeadCapture"`).
- **Verdict:** GONE
- **Why:** The scan now uses the same readers as the loader and Preview, so the scan-versus-Preview difference is gone.
- **Side picked without a ruling:** the "read the whole metadata block" side won. 36be3a70 changed it as part of the song-id change; its message gives the user's call only for dropping old .sng/.srb results ("user's call"), not for removing the 1 MB prefix. For an .srb the 1 MB cap still stands, now for all readers.

## Not reached

Every id has a block. Three code checks were left short, none changes a verdict by my reading: R7.22 (I did not re-read `batch_worker_count` and `run_work_pool` for the single floor; I only confirmed `batch_worker_count` is the one called), R7.39 (I did not re-read `OpusReader`, only the ADR text), and 202 (I did not look for the page-name constant in `src/app/report_files.h`; the REOPEN rests on the user's words, not on the constant).

Tally: PASS 21 (136, 138, 140, 141, 154, 164, 174, 193, 207, 218, 222, 229, 298, 307, 314, 320, 341, R7.12, R7.22, R7.30, R7.39); REOPEN 3 (139, 202, 219); GONE 4 (53, 143, 317, 321).

## Side notes

- ADR 0023 lines 92-93 still say "A generated fill's length, when two or more meter changes fall between two chords. Hydra's walk reads the meter one change late there." (docs/adr/0023-chart-rules-follow-clone-hero-code.md). The walk is gone (commits b57a2155, 5677f8d3), so the sentence is false, and the unverified list is stale.
- CONTEXT.md:735-737 says the CH 1.1 fill deadline and the generated fill's half-beat distance count quarter notes in every meter "both as Clone Hero's code does", while ADR 0023:94-96 and ch-evidence.md:207 ("**Answer: unknown.**") say the 6/8 beat count is not verified. One of them is wrong.
- `hit_window_ms` below 1 reads the default 85 from the INI (src/app/config.cpp:197-199, `Outside::Default`, comment "has no edge to land on") although the key has a floor of 1.0, while Q14 promised "pulled to the nearest edge" for every number setting. So `hit_window_ms=0.5` -> 85, not 1. Not asked.
- The "SP" label, gauge fill and stars use `kStarPowerColor` (255,204,51) while activation marks and the header use `kBestPathColor` (250,210,0) (src/ui/preview_tab.cpp:356, :797, :821 vs :199, :723). Q26 only unified marks and header.
- Phase 3-5 Q26's heading lists finding 218, but its text says nothing about hover teal; 218 was settled by a separate chat question (D74). Phase 7 Q13 says "record all five" over a list of six bullets (docs/audit/2026-10-04-phase-7-questions.md:76-80).
- 5677f8d3's message says "The 1.0 fill rule itself is unchanged", but for two meter changes between chords the 1.0 length changed from one-change-late to current meter (see block 53). A silent change in score-affecting behaviour (1.0 mode only).
- tools/ch_probe: `play_chart.py:285`, `poll_cost.py:137` and `find_checksum.py:370` search only `normal_pattern`, while the other runners use `all_patterns` (see block 229).
- Triage rows 53/314/320/141 still say "Clone Hero's reading is unverified"; docs/audit/ch-evidence.md answers 141 and 320 from game code (:152, :249).

