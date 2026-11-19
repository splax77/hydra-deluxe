# TOOL-AUDIT-T7-C03 (closure checker C03; batch 2: sorting S02, ledger L02)
Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout), PRE = H\docs\agents\brief-preamble.md.
Counts: PASS 22, REOPEN 3 (L-F04-13, L-F04-18, L-F05-06), GONE 0. Not reached: none.

### L-F04-01 (P)
**Verdict:** PASS
**Homes read:** G\progress.py:14-16 (100/150, 20/30, 5/10, "The one home of these numbers"); PRE:13,21,71,73; H\docs\agents\agent-rules.md:13; derive-once-review.md:110 and doc-review.md:100 (100 calls or 20 minutes); G\README.md:124; effort_test_scorer_brief.md:1; reviewer.md:11; model-postures.md:5; H\CLAUDE.md:69; writer-brief.md:100-101; memory one-reviewer-rule-d107.md.
**Why:** Every number agrees. The writer-brief says "stop starting new cards: finish the one in hand and report" and is silent on 150/30; that is an omission for a different role (it writes files, does not commit), not a different value. No choice of value is needed.

### L-F04-02 (P)
**Verdict:** PASS
**Homes read:** G\tests\test_progress.py:835-864 (loops `os.listdir(GATES)` for names ending ".py" only; a literal tuple of the six numbers is asserted after it).
**Why:** Confirmed as a test gap with no value split. Widening the scan changes no behaviour of the hooks.

### L-F04-03 (P)
**Verdict:** PASS
**Homes read:** G\bash_windows_rule.py:16 `MAX_CHARS = 1000`; G\shell_output.py:15-18 `CAP = 4000`, 1500/1500; PRE:33 ("over 1,000 characters"), PRE:35 ("over 4,000 characters"); G\tests\shell_output_sizes.py:18-24.
**Why:** The typed copies equal the owners. The 2000/4000/8000 tuple is a probe sweep, not a restatement of CAP.

### L-F04-04 (P)
**Verdict:** PASS
**Homes read:** G\common.py:46 `reason[:300]`; G\README.md:81 "its first 300 characters"; G\tests\test_refusals.py:50 `reason[:300]`.
**Why:** All say 300; no split.

### L-F04-08 (P)
**Verdict:** PASS
**Homes read:** G\README.md:128; PRE:73 ("no agent type file sets a turn cap"); G\tests\test_docs_numbers.py:34-36 (TURN_CAP = maxTurns over the agents dir); a grep of C:\Users\Patrick\.claude\agents\ for `maxTurns` found nothing.
**Why:** Docs, test and files agree today. The spawn-depth setting is its own row (L-F04-20).

### L-F04-09 (P)
**Verdict:** PASS
**Homes read:** H\tools\build_slot.ps1:3,75-80 (`$BuildSlotCount = 3`, owner comment); H\CLAUDE.md:76; agent-rules.md:20; development.md:114; PRE:57 (points at rule 14); H\tools\test_build_pass.ps1:112-119 (literal 3 and "all 3 build slots are taken"); memory one-build-per-wave.md.
**Why:** Three everywhere. The test pins 3 by literal, equal to the owner. The memory note is a dated user quote, not a restatement of a limit.

### L-F04-12 (P)
**Verdict:** PASS
**Homes read:** H\tools\hydra_ui_mcp.py:41-45 ("one home of these three numbers": 15.0, 60.0, 12_000); H\tools\test_hydra_ui_mcp.py:289 (uses hm.REPLY_CAP_CHARS), :293 (literal "cut at 12,000 characters"); H\docs\agents\ui-testing.md:280,284 (names, no values).
**Why:** The one literal copy equals the owner; docs carry no values.

### L-F04-13 (P)
**Verdict:** REOPEN
**Homes read:** C:\Users\Patrick\.claude\skills\reverse-engineer-anything\references\javascript-applications.md:34-40 (1024 MiB, 300000 ms, 512 MiB reserve); SKILL.md:131-135,147-151,185-188 (50 frames, 50 lines, 20 rows); references\native-and-artifacts.md:31-34 (20 rows); references\evidence-workflows.md:49-57. The skill's own copies agree with each other.
**Why:** P needs every home to say the same thing today, including the owner. The owner is the third-party REA server's own defaults. I searched for its tool schema (ToolSearch "rea analysis_timeout_ms max_heap_mb") and found nothing; the server is not readable here, and no file in either repo holds its defaults. So "no disagreement" cannot be shown, and a "pointer to the owner" has no readable target. It is not a closed P; the server's defaults must be read first (or a question asked whether a third-party skill is ours to edit).

### L-F04-15 (A)
**Verdict:** PASS
**Homes read:** H\tools\check_handoff.py:398-413 (name date, else `git log -1 --format=%as`, else today with a warning); H\docs\agents\handoff.md:7 (file-name date only), :59 ("defaults to the date in the handoff's file name; --since overrides"); H\.claude\commands\handoff.md:11.
**Why:** Doc and command file are docs against a program file; the doc states fewer steps than the code. Code wins; no side is a doc against a doc or a ruling.

### L-F04-16 (C)
**Verdict:** PASS
**Homes read:** H\src\core\model.h:121 `kSqueezeWindowMs = 500.0`, :133 `within_squeeze_window`; H\docs\clone-hero\05-star-power.md:417 and :419 ("model.h line 111", "within_squeeze_window line 123"); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 ("src/search/graph.h").
**Why:** Both are moved or wrong pointers and the current place is clear: src/core/model.h:121 for the constant and :133 for the function (the doc's line 123 is stale too).

### L-F04-17 (A)
**Verdict:** PASS
**Homes read:** H\docs\clone-hero\05-star-power.md:453-461 (SP-42: "less than 3 ms ... exactly 3.0 ms does not"); H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;`; H\src\core\rules.cpp:76 (listed as a settable rule); H\src\core\model.h:98-100 ("user rule ... hydra_rules.ini").
**Why:** A doc line against program files (the rules file is read by rules.cpp). The doc gives the default as if flat; the code makes it editable. Code wins. No other doc or ruling is a side.

### L-F04-18 (B)
**Verdict:** REOPEN
**Homes read:** memory no-full-library-runs.md:17 (18,811), min-path-delta-proof.md:29 (18,610), stored-versions-one-rule.md:11 (18,755), measure-small-explain-first.md:11 (19,000-chart); H\docs\agents\rulings\code-and-engine.md:27 ("about 18,848 charts", under `## Confirmed` from line 7, in the question text); H\docs\agents\rulings\ui-and-display.md:20 ("your 19,610 charts", under `## Confirmed` from line 7).
**Why:** B holds only if every split is a memory note against docs or code. Here the differing figures are memory notes against other memory notes, plus two lines in ruling files (question text beside a chosen answer). No side is a doc or code; the live count sits in the user's hydra.db, which no file states as a rule. It does not fit B. It is either not a split at all (each figure is a dated snapshot of a changing count) or needs a question; it should not stay closed as "Note is wrong".

### L-F04-19 (P)
**Verdict:** PASS
**Homes read:** G\common.py:15-18 (`_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`); G\tests\test_common.py:30 (`RACERS, HOLD_SECONDS = 16, 0.02`), :213-216 (sets `common._LOCK_WAIT = 0.05` by name and restores it).
**Why:** The test overrides the owner by name; the other numbers are the test's own fixture sizes with no copy elsewhere. No split. (Close to GONE: it names no duplicate.)

### L-F04-20 (A)
**Verdict:** PASS
**Homes read:** PRE:73 ("This hook is the only limit on an agent"); G\README.md:128 ("so it is the only limit"); C:\Users\Patrick\.claude\settings.json:4 `"CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH": "1"`; PRE:77.
**Why:** Docs against a settings file Claude Code reads; the setting adds a nesting limit the docs do not mention. Code wins.

### L-F05-01 (A)
**Verdict:** PASS
**Homes read:** H\tools\ch_mcp\quiet.py:27-31 `QUIET_CPU_PCT = 30`; H\docs\superpowers\plans\2026-11-12-player-stall-fixes.md:74 and :303 (both say 15 percent); rulings\clone-hero.md:72 is Unconfirmed and agrees with 30 (not a split side).
**Why:** Plan doc against program code. Code wins.

### L-F05-02 (A)
**Verdict:** PASS
**Homes read:** H\tools\ch_mcp\server.py:128 (instructions string "about 6 s") and :158 (docstring "about 6 s"); H\tools\ch_mcp\quiet.py:29-31 (`QUIET_SAMPLE_S = 5`), :44-47 (`SCAN_CAP_S = 3.0`, "1.5 s for 31 folders").
**Why:** Prose figure against the code's constants. Code wins. Note: 5 s plus a measured 1.5 s scan is about 6.5 s, so "about 6" is roughly right; still a hand-added figure.

### L-F05-06 (P)
**Verdict:** REOPEN
**Homes read:** H\tools\ch_mcp\machine_report.py:49-52,250-262 (`_settings_lines` opens the file itself and reads every `framerate` and `vsync` line in any section, prints all of them or "no X line"); H\tools\ch_mcp\game_files.py:70 `GAME_SECTION = "game"`, :215-256 (`get_game_setting` reads exactly one line in [game], raises on none or several). Real settings.ini: `[game]` line 27, `[video]` line 106, `framerate`/`vsync` lines 121-122, so the comment's claim is true.
**Why:** No value disagreement, but P needs "replacing with a pointer changes no behaviour and needs no choice". The two readers do different things (section scope; handling of duplicate or missing lines and an unreadable file). A single reader needs a choice on those, and no existing owner serves both. Not P; either not a finding or a design question.

### L-F05-09 (P)
**Verdict:** PASS
**Homes read:** H\tools\ch_mcp\menu_tools.py:53-57 (PlayerProfileMenu, MainMenu, SongSelect, PlayerSelectionMenu; comment names agent/menu.ts); H\tools\ch_mcp\agent\menu.ts:47-48 `MENU_CLASSES` (same four).
**Why:** The two lists agree and the first comment names the second. A Python file cannot import a TypeScript file, so "one list" would be a pin test; that changes no behaviour.

### L-F05-10 (P)
**Verdict:** PASS
**Homes read:** H\src\core\model.h:101-106 (`kHitWindowMs = 85.0`, "no setting changes it"); H\src\core\note_window.h:8-11,29-33 (`kWindowBackMs = 85.0`, "a separate number"); H\tools\ch_probe\README.md:101; H\tools\ch_probe\SPEC.md:302; H\tools\ch_probe\constants.py:85-92,243-251; H\src\app\report.cpp:92; src\core\squeeze_rating.cpp:133,242.
**Why:** All homes say 85 and the docs read true of the rating model (ratings, tiers, badges and search read kHitWindowMs; the clamps are display only). No disagreement.

### L-F05-11 (P)
**Verdict:** PASS
**Homes read:** H\tools\ch_probe\constants.py:250-251 (171.4313, 96.2932); H\tests\test_note_window.cpp:38,74-78 (171.4313075, 96.2932). I searched tools\ch_probe\tests and tests\*.cpp for a test reading both sides and found none linking them (only a doc check on RVA_ names, test_docs_match_code.cpp:728-735).
**Why:** Values agree; the missing link test is a test gap only.

### L-F05-14 (P)
**Verdict:** PASS
**Homes read:** H\tools\doc_shots\shrink.py:17-21 (500 KiB, 4 MiB, names the C++ test); H\tests\test_docs_match_code.cpp:2022-2025 (same numbers, names shrink.py); H\docs\agents\doc-screenshots.md:17 (points to shrink.py).
**Why:** Same numbers, each names the other.

### L-F05-15 (P)
**Verdict:** PASS
**Homes read:** H\tools\test_hydra_ui_mcp.py:293; H\tools\fake_hydra_live.py:58-65 (SESSION_TIMEOUT_S 25 deliberately differs from the real exe, with the reason; BIG_TEXT sized to exceed the cap); H\tools\ch_mcp\tests\test_trace_hits.py:222,249 ("85 ms", names EXPECT_NORMAL_BACK_MS).
**Why:** The retyped 12,000 and 85 equal the constants. The 25 is a stand-in's own value, not a copy of a real one.

### L-F05-16 (P)
**Verdict:** PASS
**Homes read:** H\tools\precheck\definitions.h:13-26 (30, 200); items.h:43-50 (400, "the one place it lives"); check_5.cpp:20-28 (12, 3, 4); test_pick.h:20-31 (30, 40, 20); H\tests\test_precheck_git.cpp:198-203; test_precheck_pick.cpp:73-77; test_precheck_pick_exe.cpp:195-203.
**Why:** Test literals equal the constants. They pin the user's numbers on purpose; switching them to names is a test choice with no product change.

### L-F05-17 (P)
**Verdict:** PASS
**Homes read:** H\tools\replay_schedule.cpp:15-62 (kFrameMs 3.0, kJudgeEarlyMs, kPressLeadMs 2.0, kSpEndLeadMs 15.0, kSpOutLagMs 15.0, kChordOrderGapMs 8.0, kRoomMarginMs 3.0; window values come from note_window.h); H\docs\agents\derive-once-review.md:60-64 (3.00 ms is one frame, a worked example); a grep of the main checkout for these names finds them only in replay_schedule.cpp (the other copy is in .claude\worktrees\plan-agent-supervisor, a separate worktree).
**Why:** The three 3.0 values answer three separate questions and each constant lives once.

### L-F05-18 (P)
**Verdict:** PASS
**Homes read:** H\tools\mutate\run.py:86-98 (`return baseline_seconds + 5.0`, `TIME_LIMIT_RULE`), :967 (a print string "each run's clean time plus 5 s"); H\docs\development.md:730-734 (states the rule, names test_timeout_seconds as its one home); H\tools\mutate\test_run.py:228,405-406 (pins 5.0).
**Why:** All say clean time plus 5 s. Replacing the print string and tests with the constant changes no behaviour.

## Ids not reached
None.
