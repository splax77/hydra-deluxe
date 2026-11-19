# TOOL-AUDIT-T5-W03 (second-round verifier W03; ledger L02; 20 rows)

All claims are "by reading". I ran nothing. H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates. Rulings pages are H\docs\agents\rulings\. Hook-gap note: none found for any of these rows beyond what the rows already say.

### L-F04-02
**Verdict:** CONFIRMED (by reading).
**Homes as found:** G\tests\test_progress.py:847-849 walks `os.listdir(GATES)` and opens only names ending `.py`, so a budget number in a .ps1, .sh or .json is not seen. test_progress.py:856-858 pins the six numbers as the literal tuple `(100, 150, 20, 30, 5, 10)`. The scan regex (:844) also only fires on a line that mentions "calls", "minutes" or "budget", so a bare number on any other line passes.
**Splits:** None on value. Gap: the "one home" test sees a narrower set (py files, lines with those words) than the rule it guards.
**Ruling:** None found (searched rulings for budget, calls, minutes).
**Open:** none.

### L-F04-03
**Verdict:** CONFIRMED (by reading), with extra homes the row missed.
**Homes as found:** G\bash_windows_rule.py:16 `MAX_CHARS = 1000  # the user's number`. G\shell_output.py:15-18 `CAP = 4000`, `HEAD_CHARS, TAIL_CHARS = 1500, 1500`, `ERROR_CHARS = CAP - HEAD_CHARS - TAIL_CHARS`. Copies: brief-preamble.md:33 "over 1,000 characters", :35 "over 4,000 characters"; G\tests\shell_output_sizes.py:18-21 `CAPS = (2000, 4000, 8000)`, `AT_FAILURE_CEILING = 9900` (typed, no import). Missed by the row: test_bash_windows_rule.py:53 `assertEqual(rule.MAX_CHARS, 1000)` and test_shell_output.py:172 `assertEqual(shell_output.ERROR_CHARS, 1000)` (typed copies); G\tests\replay_extract.py:28 `REFUSAL_TEXT_CAP = 4000` (a 4,000 that is not named from shell_output.CAP); G\prompt.py:21 `FIELD_CAP = 10000` (different question). test_docs_numbers.py:60-61 comment states the user ruled against raising the limit; test_docs_numbers.py:192-202 holds the preamble to MAX_CHARS.
**Splits:** None on value. Typed 4,000 in replay_extract.py and shell_output_sizes.py is a second home for the same cap.
**Ruling:** None under `## Confirmed`. review-and-hooks.md:43 (the 1,000 choice) and :40 (commit text exempt) both sit under `## Unconfirmed` (header line 37). No Confirmed line names 1,000, 4,000 or 1,500. Neither side of any split named.
**Open:** Whether the trimmed-output "do not raise the limit" ruling exists as a Confirmed line: searched rulings for 1,000, 4,000, output limit, shell output, trim; found none.

### L-F04-04
**Verdict:** CONFIRMED (by reading).
**Homes as found:** G\common.py:46 `"reason": reason[:300]` (literal). G\README.md:81 states the 300 (long line, not quoted). G\tests\test_refusals.py:50, test_percall_key.py:207, test_publish.py:302 each re-slice `[:300]` by literal. test_bash_windows_rule.py:307 comment "keeps the first 300 characters". Same number, other questions: progress.py:31 `QUOTE_CHARS = 300` (quotes a refused command back), ref_judge.py:57 `err[:300]`, C:\Users\Patrick\.claude\tools\session_cards\prepass.py:33 `TOOLS_LINE_CAP = 300`.
**Splits:** None on value. The log's cut has no name; four tests retype it.
**Ruling:** None found.
**Open:** none.

### L-F04-06
**Verdict:** CONFIRMED-CHANGED (by reading). The claim that it is "the user's answer" is contradicted by the plan it cites.
**Homes as found:** G\posture.py:16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2`. H\docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:89 says "The 60 is my pick, not a user decision ... the user settles it in Q8 before the build". Same plan :306 "Q8. The posture resend interval. T2 picks 60 minutes ... Options: 30, 60, or once per session. Recommendation: 60." That is a question with a recommendation, not an answer. posture.py:6 also resends right after a compaction, which plan :89 says a time rule cannot see (a changed design from that plan text). Tests: test_posture.py:67-80 literals `61 * MIN` / `30 * MIN`, but :74-75 and :117 use `posture.RESEND_MINUTES`.
**Splits:** Code comment says user answered Q8; plan text says it was a pick awaiting an answer. I found no answer.
**Ruling:** None. Searched rulings for posture, resend, hour, "60 min" with no hit; sessions cards for "posture" hit only 2026-11-07-e3e9069c.md:35 (not opened).
**Open:** Whether the user answered Q8 in some chat; card 2026-11-07-e3e9069c.md not opened.

### L-F04-12
**Verdict:** CONFIRMED (by reading).
**Homes as found:** H\tools\hydra_ui_mcp.py:41 "This is the one home of these three numbers." :42-44 `REPLY_MARGIN_S = 15.0`, `HELLO_TIMEOUT_S = 60.0`, `REPLY_CAP_CHARS = 12_000`; :52 `LOG_TAIL_LINES = 20`; :53 `CONNECT_RETRY_S = 0.05`; :87 `GIT_TIMEOUT_S = 60`; :92 `BUILD_PASS_TIMEOUT_S = 30`. The cut text is built from the constant (:236 `[cut at {cap:,} characters`). ui-testing.md:280,284 names the constants, no values. H\tests\ui\uitest_commands.h:19 `double timeout = 30.0` (the timeout in force the helper waits on). Copy: H\tools\test_hydra_ui_mcp.py:293 expects literal "[cut at 12,000 characters" while :289 uses `hm.REPLY_CAP_CHARS`. fake_hydra_live.py:61 `SESSION_TIMEOUT_S = 25`.
**Splits:** None on value. The comment says "three numbers" while seven names live there (comment-count drift only). Two different 30s: BUILD_PASS_TIMEOUT_S (wait for a pass) vs uitest timeout (wait-idle cap); different questions.
**Ruling:** None found.
**Open:** none.

### L-F04-13
**Verdict:** CONFIRMED (by reading).
**Homes as found:** C:\Users\Patrick\.claude\skills\reverse-engineer-anything\references\javascript-applications.md:37 "Defaults are 1024 MiB and 300000 ms", :39 "512 MiB". SKILL.md:133 "Output defaults to 50 matching", :149 "Output defaults to 50 literal lines", :186 "20 rows". references\native-and-artifacts.md:33 "samples 20 rows". The owner (REA server) is outside what I can read.
**Splits:** None findable by reading. A skill file restates a third-party server's defaults; nothing checks them.
**Ruling:** None found.
**Open:** The server's real defaults need the installed server or a run (REA's own docs). Not run.

### L-F04-15
**Verdict:** CONFIRMED-CHANGED (by reading).
**Homes as found:** H\tools\check_handoff.py:398-413 `handoff_date`: docstring "its git author date"; the code runs `git log -1 --format=%as -- <name>`, which is the author date (the row said "last git commit date"). Fallback today's date, with a warning on stderr. H\docs\agents\handoff.md:7 says only "The date in the name is the day it was written. The checker reads that date" (no git or today fallback stated). H\.claude\commands\handoff.md:11 tells the writer to name the file with today's date. check_pointers.py: G\settings.fragment.json:98-104 and live C:\Users\Patrick\.claude\settings.json:108-114 both carry `-I -S ...check_pointers.py --session-start`, `"timeout": 15` (equal today).
**Splits:** Docs state one step of the date rule; the code has three. Situation: a handoff file whose name has no date, or a file name date older than its content.
**Ruling:** None found.
**Open:** README.md:71 not opened.

### L-F04-16
**Verdict:** CONFIRMED (by reading).
**Homes as found:** Owner H\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0;`, used strictly (:130-134 `fabs(...) < kSqueezeWindowMs`). Other C++ readers by name: config.cpp:158, batch.cpp:149, path_view.cpp:81-94, graph.cpp:15, replay.cpp:26. Stale pointers: H\docs\clone-hero\05-star-power.md:417 and :419 say "src/core/model.h line 111" (constant is at 121; the page's own :399 says `within_squeeze_window` model.h line 123, also stale); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 names "src/search/graph.h", but the constant is in src/core/model.h.
**Splits:** Two doc homes point at the wrong line/file; no value split.
**Ruling:** clone-hero.md:12 (under Confirmed, which runs lines 7-66): "ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." It names 500 ms; no split. A card quote on 05-star-power.md:420 ("they should reach the full 500 ms hydra supports") is a card, not a ruling.
**Open:** none.

### L-F04-17
**Verdict:** CONFIRMED-CHANGED (by reading). The 3 ms is a default of an editable setting, not a fixed number.
**Homes as found:** H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;`; rules.cpp:76 reads it from hydra_rules.ini (a rules-file entry, default 3.0). model.h:98 "a user rule". Engine/screens read it by name: engine.cpp:2174, path_view.cpp:262/383-403, replay.cpp:191, squeeze_rating.cpp:158-234, backend_value.h:37-39. Prose: H\CONTEXT.md:801-805 "3 ms ... by default" with markers `<!-- default: Rules::backend_leeway_ms -->`; 05-star-power.md:319-321 and :453-459 say "3 ms" flat (SP-42), and SP-31 "within 3 ms" is a different measurement (game's stored end vs Hydra's chord) with the same number; SP-39 (:351) another 3 ms frame. differences-from-public-hydra.md:31 lists hydra_rules.ini.
**Splits:** (1) SP-42 says the backend leeway is 3 ms; code lets hydra_rules.ini change it. Situation: a user who edits the rules file. (2) Three 3 ms numbers with three meanings (backend leeway, SP-31 end agreement, one frame).
**Ruling:** code-and-engine.md:12 (Confirmed, 2026-10-29): "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar and i am fine with keeping it ..." It names both sides (where SP ends versus whether a backend counts). :13 is the second half. Neither speaks to the rules-file override.
**Open:** none.

### L-F04-19
**Verdict:** CONFIRMED-CHANGED (by reading). No copy of the owner's number exists; the test overrides it by name.
**Homes as found:** G\common.py:19 (the row said :18) `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`. G\tests\test_common.py:30 `RACERS, HOLD_SECONDS = 16, 0.02` (test-only race setup). test_common.py:213-216 saves `common._LOCK_WAIT`, sets it to 0.05 and restores it with `addCleanup`, so the test uses the owner's name, not a retyped 2.0.
**Splits:** None.
**Ruling:** None found.
**Open:** none.

### L-F04-20
**Verdict:** CONFIRMED (by reading).
**Homes as found:** C:\Users\Patrick\.claude\settings.json:4 `"CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH": "1"`. G\README.md:128 "No agent file ... sets `maxTurns` ... the budget hook ... is the only limit." brief-preamble.md:73 "This hook is the only limit on an agent: no agent type file sets a turn cap behind it." :77 "You have no helper agents." I searched G\*.md and *.py for "spawn" and "depth": only unrelated bash scanner and multiprocessing hits. No gates doc names the spawn-depth setting.
**Splits:** Preamble/README say the budget hook is the only limit on an agent; the depth setting is a second, undocumented limit (it stops agents from spawning). Also stall.py has its own LIMIT_SECONDS. No value conflict.
**Ruling:** None found.
**Open:** whether Claude Code's depth default would differ; only its docs settle that.

### L-F05-02
**Verdict:** CONFIRMED (by reading).
**Homes as found:** H\tools\ch_mcp\server.py:128 and :158 "about 6 s". quiet.py:30 `QUIET_SAMPLE_S = 5`, :46 `SCAN_CAP_S = 3.0` (comment "1.5 s for 31 folders"), :31 `BUILD_RECENT_S = 60`. quiet.py:197 samples CPU over QUIET_SAMPLE_S. No code line says 6; it is hand-added (5 s plus about 1 s). If the scan runs to its cap the total is above 6 (5+3, not counting process listing and the power query).
**Splits:** Prose figure (6) vs code (5 + up to 3 + process calls). Situation: a slow folder scan.
**Ruling:** None found.
**Open:** The real duration needs a run (not done).

### L-F05-04
**Verdict:** CONFIRMED-CHANGED (by reading). The plan the comment cites says the opposite of the comment for the launch caps.
**Homes as found:** H\tools\ch_mcp\bot_run.py:64-68 `POLL_INTERVAL_S = 1.0`, `POLL_CAP_EXTRA_S = 60.0`, `EXIT_CHECK_S = 30.0` under "The user's caps ... question 5 of the feasibility plan". `git show 444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md` line 27: "5. The caps. Every waiting step needs a number and these are new, so they need your yes. Recommended: ... song's length plus 60 s; the exit check ... 30 s ...". That is a recommendation; I found no answer. bot_run.py:85-91 `LAUNCH_ATTACH_CAP_S = 20.0`, `LAUNCH_ARM_CAP_S = 5.0`, `LAUNCH_PATCH_CAP_S = 30.0`, `LAUNCH_POLL_S = 0.5` under "the direct-start plan's question 1, answered by the user on 2026-10-31". `git show 6804353f:docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` line 132: "The three launch caps. No ruling sets them, and none of the three has been measured; these are guesses LS-D0 then measures. Recommended: LAUNCH_ATTACH_CAP_S = 20 ... 5 ... 30 ... LAUNCH_POLL_S = 0.5."
**Splits:** Code comment says the user answered; the plan text says no ruling and a guess. I found no answer in rulings or cards (searched rulings for caps, polls; session cards for launch cap, LAUNCH_ATTACH).
**Ruling:** None under `## Confirmed` for either set (clone-hero.md Confirmed runs lines 7-66).
**Open:** Whether the user answered later in chat (not in the files I read).

### L-F05-05
**Verdict:** CONFIRMED for the homes I opened; the rest is not reached (by reading).
**Homes as found:** game_tools.py:45 `INSTALL_CAP_S = 60.0`. frida_tools.py:39-49 "three Frida caps of the plan's question 5, approved by the user on 2026-10-29": `ATTACH_DELAY_S = 5.0`, `ATTACH_CAP_S = 30.0`, `READ_CAP_S = 5.0`. method_watch.py:47-49 `METHOD_WATCH_CAP_S = 120.0` with comment "the plan's ch_method_watch_start (cap_s=120) and live step LS-M ("cap 120 s")" (the plan's value is restated in a comment). kit_tools.py:94-123 `KIT_HOLD_FRAMES = 2` ("user's answer to the plan's question 2, 2026-10-30"), `KIT_SETUP_CAP_S = 10`, `KIT_PRESS_CAP_S = 1` ("question 3"), `KIT_WATCH_MAX_S = 60.0` ("the plan's K7 question, 2026-10-30, 'Cap at 60 s (Recommended)'"), `KIT_RATE_WINDOW_S = 1.0`, `KIT_EVENT_WAIT_S = 0.05`. trace_hits.py:67-74 `TRACE_CAP_MARGIN_S = 30.0`, `WALL_CAP_PAUSE_S = 120.0` ("approved by the user on 2026-10-28"); its tests (test_trace_hits.py:180-181, 462-463, 545-554) retype "36 s ... 156 s". sp_tools.py reads caps by name (sp_watch.RUN_CAP_S, engine_finder.ENGINE_SEARCH_CAP_S, READ_CAP_S).
**Splits:** (1) Three different caps share 120: METHOD_WATCH_CAP_S (a watch), WALL_CAP_PAUSE_S (room for a paused song) and, in the plan, find_live_engine's 120 s and REPLAY_CAP_S 120; different questions. (2) trace_hits.py:73 says the 120 s pause room was approved 2026-10-28; the feasibility plan's Q5 line (which I read) lists song cap plus 30 s and find_live_engine 120 s but I did not find a "pause room" in it. Unconfirmed.
**Ruling:** Confirmed lines found: clone-hero.md:27 "Keep 60 s" (install cap, matches INSTALL_CAP_S), :20 "Accept all four" (frame probe 5 s), :14 and :16 (stall hunt). I found no Confirmed line for the attach caps, kit caps (10 s, 1 s, 60 s), trace caps or 120 s watch. All quoted "user's answer" comments in kit_tools.py and frida_tools.py are comments, not rulings.
**Open:** Not opened: machine_recorder.py:41-60, machine_report.py:38-52, agent clock.ts/kit.ts/trace.ts, ch_probe\constants.py:349-358, README.md:85, sp_tools.py:49-66 values.

### L-F05-08
**Verdict:** CONFIRMED (by reading).
**Homes as found:** H\tools\ch_mcp\menu_tools.py:39 `SEARCH_DOWN_CAP = 10`; _search (:387-405) presses the next-song item at most that many times and then stops with "the target was not selected after 10 ... presses". It scrolls Clone Hero's own song list with kit presses. menu_tools.py:48-50: "The checked-in menu_steps.json has no search step"; I searched menu_steps.json (case-insensitive) for "search" and found nothing, so the cap is dormant in the checked-in steps.
**Splits:** The ruling's subject is the song library listing ("scrolling through the entire library for minutes"); the cap acts on the game's list, at most 10 presses, with an error line when it stops. Situation: ten scroll presses in the game's list.
**Ruling:** ui-and-display.md:27 is under `## Confirmed` (lines 7-101; Unconfirmed starts 102): "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." (2026-10-30, session 83eb8bac). It names the library and "without my approval"; it does not name the game's list, a cap of 10 or the explanation case, so it does not name both sides.
**Open:** Whether it covers the game's own list is the user's reading, not mine.

### L-F05-09
**Verdict:** CONFIRMED (by reading).
**Homes as found:** menu_tools.py:53-57 `PROFILE_SCREEN = "PlayerProfileMenu"`, `MAIN_MENU = "MainMenu"`, `SONG_SELECT = "SongSelect"`, `PLAYER_SCREEN = "PlayerSelectionMenu"` ("by the name agent/menu.ts sends"). agent\menu.ts:47-48 `PLAYER_SCREEN = "PlayerSelectionMenu"`; `MENU_CLASSES = ["PlayerProfileMenu", "MainMenu", "SongSelect", PLAYER_SCREEN]`. Related: menu.ts:54 `PLAYER_SCREEN_FRAME_RVA = 0x32F590` is checked by tests\test_frida_tools.py:642-654 (reads menu.ts text with a regex). frida_tools.py:72-74 and menu.ts share menu_fields.json (one file, bundled; not a copy).
**Splits:** None today (two typed lists agree). Nothing I found holds the Python four names to the TypeScript list; I did not open test_menu_tools.py.
**Ruling:** None found.
**Open:** test_menu_tools.py not searched for a name check.

### L-F05-10
**Verdict:** CONFIRMED-CHANGED (by reading), agreeing with V05.
**Homes as found:** H\src\core\model.h:101-106 "One fixed number for the search and the display alike ... no setting changes it (D270 call 8)" `kHitWindowMs = 85.0`; :112-115 early-fill default 60 and max `static_cast<int>(kHitWindowMs)`. H\src\core\note_window.h:1-11 "The row is display only. Hydra's ratings, tiers, badges and search keep kHitWindowMs ..., a separate number (D270 call 8): the user ruled 'just show it alongside.'" :32-33 `kWindowFrontMs = 37.5`, `kWindowBackMs = 85.0` (a second 85.0 in C++), :37-40 formula constants. Python copies: tools\ch_probe\constants.py:227 `EXPECT_NORMAL_BACK_S = 0.085`, :232 `EXPECT_NORMAL_BACK_MS`, :250-251 `WINDOW_CAP_MS = 171.4313`, `WINDOW_FLOOR_MS = 96.2932`. tools\ch_mcp\trace_hits.py:265 and frida_tools.py:614 count "calls within 85 ms" using the Python `EXPECT_NORMAL_BACK_MS`, not Hydra's value. Prose: ch_probe\README.md:101 and SPEC.md:302 "flat 85 ms / flat kHitWindowMs = 85.0" (true of the rating model; the game's own window varies, per note_window.h). Tests: ch_probe\tests\test_s2_window_constants.py:55 `assertEqual(C.EXPECT_NORMAL_BACK_MS, 85.0)` (literal).
**Splits:** (1) kHitWindowMs 85 vs kWindowBackMs 85: one number is Hydra's model, the other the game's clamp; equal today, separate on purpose per the comment (a claim, not evidence). Situation: a change to one leaves the other. (2) constants.py 85.0 / 37.5 vs note_window.h 85.0 / 37.5 (two languages, no link). (3) trace tool's "85 ms" limit is the game's back clamp, not the engine's kHitWindowMs.
**Ruling:** ui-and-display.md:13 (Confirmed; section lines 7-101): "just show it alongside. try again with a maximum of 8 words for the new info." (2026-11-12, session 375e58cd); it asks to display the dynamic value beside Hydra's, which is the quote note_window.h trims. code-and-engine.md:49 (Confirmed, 2026-08-19) "the hit window is +/- 85ms, the 70ms is an outdated figure"; :23-24 (Confirmed) an 85 ms early-fill window, orange only; :57 (Confirmed, 2026-11-15) "default 60ms, supports 0ms to 85ms". None names the kHitWindowMs-vs-kWindowBackMs split.
**Open:** none beyond F05-11.

### L-F05-11
**Verdict:** CONFIRMED (by reading, search limited to tools\).
**Homes as found:** Searched H\tools (all .py .cpp .h .ps1 .ts) for note_window, kWindowBackMs, kWindowFrontMs, EXPECT_NORMAL_BACK, WINDOW_CAP_MS. Only tools\replay_schedule.cpp reads note_window.h; no Python file or test mentions it. ch_probe tests use constants.py only; test_s2_window_constants.py:36 ties WINDOW_CAP_MS to the measured csv and :55 pins 85.0 by literal. C++ tests (finder: test_note_window.cpp, test_replay_schedule.cpp) were not opened by me, and I did not search H\tests for constants.py.
**Splits:** Python constants and C++ constants are held equal by no test I found.
**Ruling:** None found.
**Open:** whether any H\tests C++ test reads the Python file (not searched).

### L-F05-12
**Verdict:** CONFIRMED (by reading).
**Homes as found:** H\tools\capture\report_shot.ps1:5-8 `ClickFromRight = 80`, `ClickY = 22`, "80 is 2,560 - 2,480 on the user's maximized window"; click point = client width minus 80 (:13). The UI places the button with `place_control_at_right_edge(report_w)` (src\ui\widgets.h:197-200, called at library_toolbar.cpp:140); report_w (:130) comes from `button_slot_width("Open path report")` (and the "Building path report..." label), a runtime width that follows font and DPI. The script's 80 px is a hand-measured offset for one window width. Waits in the script: `Start-Hydra $Dir 12`, a 30 x 1 s poll, `Start-Sleep -Seconds 3` (:23-25), 50 ms mouse steps.
**Splits:** Hand-measured pixel offset in the script vs width computed in the UI. Situation: a different DPI, font or button width.
**Ruling:** None found.
**Open:** A run only; not done. steps.ps1:13-17 not opened.

### L-F05-15
**Verdict:** CONFIRMED (by reading).
**Homes as found:** H\tools\test_hydra_ui_mcp.py:293 literal "cut at 12,000 characters" (the production string is built from `cap:,`, hydra_ui_mcp.py:236). H\tools\fake_hydra_live.py:57-65 `SESSION_TIMEOUT_S = 25` ("Deliberately not the real exe's starting value") and BIG_TEXT of 1,000 lines of 49 characters + newline, which exceeds REPLY_CAP_CHARS only by size, not by name. H\tools\ch_mcp\tests\test_trace_hits.py:94-100 and :222-254 hold literal "85 ms" strings and `diff_ms == 85.0`, beside `C.EXPECT_NORMAL_BACK_MS` use at :95 and :251, so the test mixes the named constant and typed 85.
**Splits:** None on value today. A change to REPLY_CAP_CHARS breaks :293 by text; a change to the constant 85 breaks the typed lines.
**Ruling:** None found.
**Open:** none.

## Extra finding for the ledger (not one of my ids)
L-F05-13 says shot.ps1 "passes its own $Wait". By reading, H\tools\capture\shot.ps1:2 declares `[int]$Wait = 10` and calls `Start-Hydra $Dir $Wait`, while capture_window.ps1:56 defaults to 12 and report_shot.ps1:10 and steps.ps1:8 pass 12. So the startup wait is 10 in one script and 12 in three: a real split (situation: a slow startup on shot.ps1). Not checked by a run.

## Ids not reached
None. All 20 reached. Within F05-05 the homes in machine_recorder.py, machine_report.py, agent\*.ts and ch_probe\constants.py:349-358 were not opened (listed under that row).
