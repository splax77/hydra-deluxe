# TOOL-AUDIT-T5-F05 (finder F05, 52 rows)

Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout); G = C:\Users\Patrick\.claude; M = G\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory.

Searched for every row: H\docs\agents (with rulings), H\CLAUDE.md, H\.claude\commands, H\tools (py, ps1, ts, cpp, h, json, md), H\tests (for pinning tests), G\gates (hooks, README, percall.py), G\agents, G\skills, G\CLAUDE.md, M\*.md. I searched by constant name, by the number, and by the question in other words ("about 6 s", "clean time plus 5", "one frame", "three builds"). "One home" below means I searched those places and found no second. I did not search H\docs\clone-hero or H\docs\superpowers systematically (outside the scope list); where they came up I name them.

Ruling-section rule: H\docs\agents\rulings\clone-hero.md has `## Confirmed` at line 7 and `## Unconfirmed` at line 67, so its line 72 is Unconfirmed (a home, not the user's words).

## Splits and drift (own lines)

- SPLIT/K2 (a plan): H\docs\superpowers\plans\2026-11-12-player-stall-fixes.md:74 says the quiet check "refuses for a running compiler, CPU over 15 percent or a recent build folder". quiet.py:28 has QUIET_CPU_PCT = 30. Row I10b-001.
- K2/K3: H\tools\ch_probe\README.md:101 says "Hydra's model uses a flat 85 ms hit window." The engine owner H\src\core\note_window.h:32-40 holds clamps (37.5 and 85.0) plus a formula, so the window is not flat; kHitWindowMs in core/model.h is a separate number (note_window.h:10). Rows I10b-060/061.
- K1 with no test between the two homes: Python ch_probe\constants.py:229-267 (85, 37.5, 171.4313, 96.2932) against C++ note_window.h. tests\test_note_window.cpp:38,74,77 and tests\test_replay_schedule.cpp:79 pin 171.4313075. ch_probe\tests\test_s2_window_constants.py ties constants.py only to the measured CSV, not to the C++.

## Rows

### I10b-001 / 002 / 003 (quiet check: CPU 30, sample 5 s, build-recent 60 s)
- Homes: tools\ch_mcp\quiet.py:28-30 (the values, with a comment). Rulings\clone-hero.md:72 (Unconfirmed): "raise the limit to 30%... So `QUIET_CPU_PCT` in tools/ch_mcp/quiet.py is 30." The 15 in the plan above is the older value. Tests use the constant by name (test_quiet.py:127-128, test_game_tools.py:2928), so they are not copies.
- Kinds: K2 (plan), K1 (the Unconfirmed note restates 30).
- Edges: exactly at the limit: test_quiet.py:126 pins "exactly at the threshold is quiet". Main vs agent: ch_start is the same call for both. Card writers never call it. Worktree: no effect.
- Confirmed ruling: none found for 30%.

### I10b-004 ("about 6 s")
- Homes: tools\ch_mcp\server.py:128 (MCP instructions every agent sees: "ch_start checks the machine is quiet (about 6 s, and it can refuse)") and server.py:158 (docstring "about 6 s"). Code side: a 5 s sample (quiet.py:29) plus a folder scan capped at 3 s (quiet.py:46).
- Kind: K1 (a value derived by hand from two constants, stated in prose; no code row says 6). No test.

### I10b-005 (scan cap 3.0 s)
- One home in code: quiet.py:43-46, comment "The user's ruling of 2026-11-07: how long the folder scan may run". I searched the Confirmed sections of all ruling files for a scan-cap line and found none. So a code comment claims a user ruling that no Confirmed line holds (K8-adjacent; main to check). The same comment states the measure "1.5 s for 31 folders" (a measured copy).

### I10b-009 / 011 (bot and human-launch caps)
- Homes: bot_run.py:63-68 and 85-91 only. Comments cite a plan removed from the tree (git show 444631e6:...) and "the direct-start plan's question 1, answered by the user on 2026-10-31". docs\agents\clone-hero-mcp.md gives no values. No Confirmed line found for these caps. One home plus a claim in a comment.

### I10b-024 (install cap 60 s)
- Homes: tools\ch_mcp\game_tools.py:45 INSTALL_CAP_S = 60.0. docs\agents\clone-hero-mcp.md:55 names prep_inputs.REPLAY_CAP_S with no value (pointer).
- Confirmed ruling: docs\agents\rulings\clone-hero.md:27: chose "Keep 60 s" to "...may take up to 120 s (REPLAY_CAP_S)... the install itself is capped at 60 s (INSTALL_CAP_S)...". Ruling found, not a home.

### I10b-027 (state read caps)
- Homes: frida_tools.py:43-49; tools\ch_mcp\README.md:85 "far inside the 5 s cap, `READ_CAP_S` in `frida_tools.py`" (K1, value in prose); docs\clone-hero\07-internals.md:553 names ATTACH_DELAY_S (pointer). Kind K1.

### I10b-031, 043, 047, 048, 052, 055, 065, 085 (single-home timing and name rows)
- One home each in code. Extra: 047 METHOD_WATCH_CAP_S 120 is restated in a comment at method_watch.py:47-48 ("cap_s=120", "cap 120 s") (K1). 052, 055, 065, 085 and 043, 048 were searched by name and number only.

### I10b-036 (kit timing)
- Homes: kit_tools.py:96-123. server.py:356 names KIT_WATCH_MAX_S with no value (pointer). The comment quotes the user ("Cap at 60 s (Recommended)", 2026-10-30); no Confirmed line has it.

### I10b-038 (frame probe 5 s)
- Homes: kit_tools.py:733-740 (comment "the 5 s the user accepted for LS-G1 (D339 item 3)").
- Confirmed ruling: rulings\clone-hero.md:20: chose "Accept all four (Recommended)": "(1) The LS-G1 frame probe runs 5 s. (2) Its frame-rate floor is 322.2...". Line 70 (Unconfirmed) records that the frame-rate floor was dropped.

### I10b-040 / 042 (recorder and report numbers)
- Homes: machine_recorder.py:41-60, machine_report.py:38-52. K6 candidate: machine_report.py:49-52 and _settings_lines (250-262) read settings.ini lines on their own because game_files.get_game_setting reads only [game] (game_files.py:217-256). Two code readers of one file; the comment gives the reason. ALWAYS_LISTED_NAMES is built from quiet's lists (a true pointer).

### I10b-044 (menu steps)
- Homes: menu_tools.py:37-57. The four menu class names appear in menu_tools.py:54-57 ("by the name agent/menu.ts sends") and agent\menu.ts:47-48 MENU_CLASSES (two homes, K1). PLAYER_SCREEN_FRAME_RVA 0x32F590 (menu.ts:54) is also in docs\clone-hero\code\6142\player_menu_on_ready_page_32F590.md; menu.ts:50 says tests\test_frida_tools.py checks it against the dump's facts (a test that holds it).
- K4: menu_tools.py:44-51 says of SEARCH_NEXT_ITEM "So Blue is a guess".
- Confirmed ruling that may speak to SEARCH_DOWN_CAP 10 (ten scroll presses through the game's song list): rulings\ui-and-display.md:27: "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." That ruling is about Hydra's library; this scroll is in Clone Hero's list. Main to read; I grade nothing.

### I10b-049 / 050 (SP watch and trace caps)
- Homes: sp_tools.py:49-66 (GAP_MS 50, comment cites the user), trace_hits.py:57-74 (TRACE_CAP_MARGIN_S 30, WALL_CAP_PAUSE_S 120, "approved by the user on 2026-10-28"). docs\agents\clone-hero-mcp.md:55 names TRACE_CAP_MARGIN_S and D331 with no value (pointer). No Confirmed line found for either number.

### I10b-056 (TypeScript agent numbers)
- Homes: agent\clock.ts:14, kit.ts:72/82, trace.ts:37/43. The agent\*.js files are git-ignored build outputs with stamps (tools\ch_mcp\README.md:79), so not homes. No Python copy of 250/2/20/200 found.

### I10b-060 / 061 / 062 (hit-window constants)
- Homes in tools: ch_probe\constants.py:89-92,166-171,229-267; ch_probe\SPEC.md:107,151-152,166-167,277 ("normal 85/37.5, precision ~40/25"); ch_probe\README.md:6,101 (split above). ch_mcp\trace_hits.py:265 and frida_tools.py:601-616 read C.EXPECT_NORMAL_BACK_MS (pointer). Strings "within 85 ms" at ch_mcp\tests\test_trace_hits.py:222,249. docs\agents\test-rubric.md:21 and test-rubric-examples.md:179 state "85 ms hit window" (K1 as examples).
- Owner of the engine rule: H\src\core\note_window.h:30-40 ("The game's normal-mode clamps on one half-gap: TM-18 on docs/clone-hero/03-timing.md (0.0375 s and 0.085 s)").
- Kinds: K1 (constants.py vs note_window.h), K2/K3 (README:101).
- Tests: none ties Python to C++ (see above). 062 probe timings: one home.
- Confirmed rulings on 85 ms: rulings\code-and-engine.md:23-24 and :49 (I did not check their section for :49; :23-24 read as rulings of 85 ms early fill).

### I10b-063 (stall hunt)
- One home (constants.py:349-358). Confirmed rulings restate its numbers: rulings\clone-hero.md:16 chose "Yes, those numbers (Recommended)" (40 ms stall, 10 minutes) and :14 "Start them, 5 s cap (Recommended)".

### I10b-069 / 070 (capture waits, report button)
- Homes: tools\capture\steps.ps1:13-17, shot.ps1:7, report_shot.ps1:7-28. report_shot.ps1:5-8 says ClickFromRight 80 is "2,560 - 2,480 on the user's maximized window" and names place_control_at_right_edge in library_toolbar.cpp; the placing function is src\ui\widgets.h:197 (used at library_toolbar.cpp:140; its owner row is tests\single_owner\rules\ui.json:1715). The script holds a hand-measured copy of a layout the UI computes (K4/K1). No test.

### I10b-071 (doc picture size limits)
- Homes: tools\doc_shots\shrink.py:18-21 (500*1024 and 4*1024*1024; comment "The user's limits (D300 ...). tests/test_docs_match_code.cpp checks the same two at merge") and tests\test_docs_match_code.cpp:2022-2025 (same two numbers; comment "tools/doc_shots/shrink.py fails a capture run on the same limits"). docs\agents\doc-screenshots.md:17 says "the constants at the top of shrink.py hold them" (pointer).
- Kind: K1 (two homes, each calls the other the same limit, neither reads the other). Test that holds them equal: none.
- Confirmed line: none found (D300 is in the decision log).

### I10b-074 (stand-in hydra session)
- Homes: fake_hydra_live.py:57-65. SESSION_TIMEOUT_S 25 is commented "Deliberately not the real exe's starting value". BIG_TEXT (1,000 lines) is sized to exceed hydra_ui_mcp.py:44 REPLY_CAP_CHARS 12_000; tools\test_hydra_ui_mcp.py:293 pins "cut at 12,000 characters" as a literal (K1, test copy). docs\agents\ui-testing.md:284 names REPLY_CAP_CHARS (pointer).

### I10b-077 / 078 / I10-086 (body floor 30, look-back 200)
- Homes: tools\precheck\definitions.h:15-25 (D49). tests\test_precheck_check1.cpp:193 uses the constant by name. One home.

### I10b-079 / I10-083 (large range, 400 lines)
- Homes: tools\precheck\items.h:45-50 ("This is the one place it lives."); items.cpp:102-104 builds the message from it. docs\agents\derive-once-review.md:21: "The precheck owns that threshold and which files count; this page never restates them." (pointer, true).
- Test copies: tests\test_precheck_git.cpp:200-202 pins `RangeSize{400, 0}` and the text "over the 400-line threshold" (K1). tests\test_precheck.cpp:417 uses the constant by name.

### I10b-081 / I10-085 (repeat sizes 12 / 3 / 4)
- Homes: tools\precheck\check_5.cpp:22-28 ("This is the one place they live."). I searched tests and docs by name and number and found no second home.

### I10b-086 (test picker limits 30 / 40 / 20)
- Homes: tools\precheck\test_pick.h:22-30. Test copies: tests\test_precheck_pick.cpp:75-77 pin `kCommonNameLimit == 30`, `kPickSizeLimit == 40`, `kHeaderIncluderLimit == 20`; tests\test_precheck_pick_exe.cpp:195-203 hold the strings "than 20" and "than 40" (K1).

### I10b-091 to 097 (replay schedule: frame 3.0, judge 1 frame, press lead 2, SP-end lead 15, SP-out lag 15, order gap 8, room 3)
- Homes in code: tools\replay_schedule.cpp:20-57 only; comments cite plans (one removed: git show d7e47a20:...) and "the user chose 'One frame', D319 item 3".
- Prose copy: docs\agents\derive-once-review.md:64 worked example "3.00 ms is the one frame of judging the tool counts" (K1 of kFrameMs).
- Same number, other meaning: memory sp-end-is-one-point-leeway-is-delgars.md:3 and :11 hold "3 ms" backend leeway (Delgar's rule), a different question from kFrameMs and kRoomMarginMs (both 3.0).
- No copy of 15, 8 or 3 found in ch_probe or ch_mcp Python (searched lead, lag, gap and margin names).

### I10b-098 (window clamps)
- replay_schedule.cpp:20 points at core\note_window.h; lines 61-62 read kWindowFrontMs and kWindowBackMs. A pointer that reads the owner; true.

### I10b-102 (mutate time limit, clean time plus 5 s)
- Homes (all state the rule): tools\mutate\run.py:86-96 (code and docstring, quotes the user); run.py:100 TIME_LIMIT_RULE text; run.py:966-967 a print string "(each run's clean time plus 5 s)" (a literal of the rule not built from TIME_LIMIT_RULE); docs\development.md:732-734 ("Its one home is `test_timeout_seconds` in `run.py`" yet it states the rule); py_mutate.py:105,581 call it (pointer); tools\mutate\test_run.py:228,274,405-406 pin 5.0. Card docs\sessions\2026-11-16-60c3a576.md:26 holds the user's quote (a card, not a ruling).
- Kind: K1. Confirmed ruling: none found.

### I10-064 (build-pass tool)
- Homes: tools\hydra_ui_mcp.py:87-93 (BUILD_PASS_TIMEOUT_S 30; session id from CLAUDE_CODE_SESSION_ID); points at tools\build_slot.ps1 ($BuildSlotCount = 3 at :75; one pass per session at :195). The rule "at most three builds, one per session, agents never build" is stated in: docs\agents\agent-rules.md:20 (rule 14), H\CLAUDE.md:76, docs\agents\brief-preamble.md:57, docs\development.md:114, G\gates\README.md:15 and percall.py:109 (refusal message). Owner value: build_slot.ps1:75.
- brief-preamble.md:57 and gates\README.md:15 explain a hook's rule in prose (K5).
- Table (what each tells its reader): agent-rules/CLAUDE.md/preamble tell agents never to build and main to use a pass; percall.py refuses the tool to every agent (card writers included, since it reads the tool name); build_slot.ps1 gives three slots, one per session. Main session: allowed. Worktree vs main checkout: pass is per session, not per checkout. Exactly at the limit: build_slot.ps1:197 refuses at count >= 3.
- Confirmed ruling: rulings\agents-and-workflow.md:8: "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session."
- Test: tools\test_build_pass.ps1 runs the script; I did not open it for a pinned 3.

### I10-076 (capture waits)
- Homes: tools\capture_window.ps1:56 `$Wait = 12`; the same 12 as a literal in tools\capture\report_shot.ps1:10 and steps.ps1:8 (shot.ps1:4 passes its own $Wait). K1 (three places for "12 s after launch"). WaitForExit(10000): one home.

## Confirmed lines that speak to these rows (sections checked)
- rulings\agents-and-workflow.md:8 (build limit).
- rulings\clone-hero.md:27 (install cap 60 s), :20 (frame probe 5 s), :16 and :14 (stall hunt numbers).
- rulings\ui-and-display.md:27 (no scrolling the library without approval).
- Not Confirmed: rulings\clone-hero.md:72 (30%, Unconfirmed).

## Rows not reached
None skipped. Only name and number searched, not read in the code body: I10b-043, 048, 052, 055, 065, 085.

## Side notes
- tools\ch_mcp\tests\test_trace_hits.py:222,249 and tools\test_hydra_ui_mcp.py:293 hold literals of constants (85 ms, 12,000).
- Three 3.0-ish numbers with three meanings: kFrameMs, kRoomMarginMs, and the memory note's 3 ms leeway.
- Hook gap: none noticed.
