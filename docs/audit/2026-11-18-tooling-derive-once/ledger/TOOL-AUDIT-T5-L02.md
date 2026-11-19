# TOOL-AUDIT-T5-L02 (ledger compiler L02, second run; finders F04, F05, F06)

Read: F04, V04, F05, V05, F06, V06 in the reports folder. No re-send pair (Fxxb / Vxxb) exists for any of the three. V06 now exists and is used. I compiled only; I judged nothing new. Where verifier and finder disagree, the verifier wins and I say so.

Short paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, MP = C:\Users\Patrick\.claude\model-postures.md, PRE = H\docs\agents\brief-preamble.md, MEM = the project memory folder. Rulings pages are H\docs\agents\rulings\.

Counts of verdicts over all 58 rows: CONFIRMED 27, CONFIRMED-CHANGED 6, REFUTED 2, NOT VERIFIED 23. (F04: 7 / 2 / 1 / 10. F05: 8 / 2 / 0 / 9. F06: 12 / 2 / 1 / 4.)

---
# Finder F04 (verifier V04)

### L-F04-01: How many tool calls, minutes and status-line minutes may an agent use, and who restates them?
- **Source:** I9-027, I9-028, I9-029, I9-084, I9-085, I9-086, I9-096, I8-247, I9-078; F04 group A; V04 "Candidates".
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED. V04 saw the K1 copies and the K3 change. V04 did not reopen test_docs_numbers.py; which docs the test covers rests on F04's reading.
- **Homes:** G\progress.py:14-16 `CALL_WARN, CALL_STOP = 100, 150`, `MINUTES_WARN, MINUTES_STOP = 20, 30`, `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10` (owner, V04 confirmed). Copies V04 saw: writer-brief.md:100-101 (C:\Users\Patrick\.claude\tools\session_cards\), reviewer.md:11 "at 100 calls or 20 minutes", MP:5, H\CLAUDE.md:69. Finder only: PRE:13,21,71,73; agent-rules.md:13; derive-once-review.md:9,110; doc-review.md:9,100; G\README.md:124; H\tools\effort_test_scorer_brief.md:1; MEM every-brief-carries-status-rule.md:15 and MEMORY.md:45; G\tests\test_progress.py:186-292 and 856-858 (a literal tuple of the six numbers); test_percall_rewrite.py:78 (`calls: 10_000` literal).
- **Splits:** None on the values; every home agrees at 100/150/20/30/5/10. K3 change of action (V04 confirmed): writer-brief.md:101 says finish the card in hand and report at 20 min or 100 calls, silent on 150/30 and on committing; PRE:73 says commit what passes and report, stopped at 150/30. MP:5 uses the numbers as a task-size rule ("too big for one executor inside 100 tool calls and 20 minutes, it is two tasks"), not a stop rule.
- **Ruling quotes:** review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time ... Agents have a hard cap of 30 minutes real time." (finder, file under Confirmed); :34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes"; agents-and-workflow.md Confirmed "i don't care if an agent is read only, no status updates is unacceptable". None names 100 or 150 calls; those appear in rulings only under Unconfirmed (review-and-hooks.md:60, an older 10/15 value). None names both sides of a split.
- **Tests:** test_docs_numbers.py holds five Hydra docs (CLAUDE.md, agent-rules.md, preamble, derive-once-review.md, doc-review.md) to progress.py (finder's reading; V04 did not reopen). test_progress.py NumbersLiveOnce. Nothing holds README.md, reviewer.md, MP, writer-brief.md, effort_test_scorer_brief.md or the memory notes.
- **Open:** Which docs the test covers (V04 did not reopen). Whether card writers (who skip PRE) follow writer-brief.md's version.
- **Hook gap:** none.

### L-F04-02: Does the test that forbids budget numbers on gates lines see only .py files, and does it hold a literal copy?
- **Source:** F04 side notes; no inventory row named; V04 "Not rechecked".
- **Kind:** K1, K6.
- **Verdict:** NOT VERIFIED (V04 did not recheck either claim).
- **Homes:** Finder only: G\tests\test_progress.py:835-864 scans gates\*.py only, so a number in a .ps1, .sh or .json is not seen; test_progress.py:856-858 pins the six numbers as a literal tuple.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** The test is itself the subject.
- **Open:** Both claims unchecked by the verifier.
- **Hook gap:** hook gap: yes, G\tests\test_progress.py (finder's label; not rechecked).

### L-F04-03: How long may a Bash command be, how long may a shell result be, and what does a trim keep?
- **Source:** I9-030, I9-053, I9-150, I9-186, I9-190, I9-034; F04 group B; V04 "Not reached".
- **Kind:** K1, K8-shaped.
- **Verdict:** NOT VERIFIED (V04 lists group B as not reached).
- **Homes:** Finder only: G\bash_windows_rule.py:16 `MAX_CHARS = 1000`; G\shell_output.py:15-18 `CAP = 4000`, `HEAD_CHARS, TAIL_CHARS = 1500, 1500`, `ERROR_CHARS = CAP - HEAD_CHARS - TAIL_CHARS`. Copies: PRE:33 "A command over 1,000 characters", :35 "A result over 4,000 characters"; G\tests\shell_output_sizes.py:18-21 `CAPS = (2000, 4000, 8000)`, `AT_FAILURE_CEILING = 9900` (no import from shell_output); G\tests\probe_results.md:104-236; H\docs\superpowers\plans\2026-11-07-shell-output-to-disk.md:13,140; test_docs_numbers.py:60-61 comment "The user ruled against raising this limit: long output is trimmed instead (shell_output.py)"; G\README.md:154.
- **Splits:** None on the value.
- **Ruling quotes:** None found under Confirmed for 1,000, 4,000, 1,500 or the output-limit ruling. The 1,000 appears only under Unconfirmed (review-and-hooks.md:43). The only user quote for "do not raise the limit" is in the plan, line 21, "so instead of saving you want to waste even more usage? no. unacceptable." (finder). Searched: rulings\*.md for shell output, trim, bashOutputMaxChars, output limit.
- **Tests:** test_docs_numbers.py:192-202 holds the preamble to MAX_CHARS and CAP. Nothing holds shell_output_sizes.py to shell_output.CAP.
- **Open:** All unverified. The 30,000 / 10,000 values are Claude Code's own limits (probe_results.md), not ours.
- **Hook gap:** none.

### L-F04-04: How long may a logged refusal reason be?
- **Source:** I9-008; F04 group C; V04 "Not reached".
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: G\common.py:46 `reason[:300]` (a literal, no name); G\README.md:81 "its first 300 characters"; G\tests\test_refusals.py:50 re-slices `reason[:300]`; test_bash_windows_rule.py:299-311 comment "the refusal log keeps the first 300 characters". Same number, different questions (finder): progress.py:31 `QUOTE_CHARS = 300`, ref_judge.py:57 `err[:300]`, prepass.py:33 `TOOLS_LINE_CAP = 300`.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_refusals.py recomputes the cut instead of naming a constant.
- **Open:** All.
- **Hook gap:** none.

### L-F04-05: When does the stall, wake and context watch fire, and do its numbers and claims have one home?
- **Source:** I9-112, I9-113, I9-114, I9-118, I9-119, I9-122, I9-123, I9-126, I9-191; F04 group D; V04 "Candidates" and split 5.
- **Kind:** K1, K8-shaped.
- **Verdict:** CONFIRMED. V04 confirmed 660 in both settings files, 500K typed twice in stall.py, and that no AUTOCOMPACT setting exists in the two settings files. V04 did not search the README or Claude Code's own docs; the "50% of the window" may be a Claude Code default, which only its docs or a run could settle. V04 did not recheck MP:15.
- **Homes:** G\stall.py:22-39 `STALL_SECONDS = 600`, `SCAN_EVERY = 60`, `CTX_THRESHOLD = 500000` (:27), `CTX_BUCKET`, `EMPTY_FLEET_SECONDS = 300`, `LIMIT_SECONDS = progress.MINUTES_STOP * 60`; stall.py:36 CTX_TEXT "Auto-compact should have fired at 500K (50% of the window)" (500K typed in text). G\settings.fragment.json:235 and live settings.json:245 `"timeout": 660` (V04 confirmed); wake.py:11-12 comment says the timeout is STALL_SECONDS plus SCAN_EVERY. Finder only: MP:15 "Keep this conversation under 500K tokens"; fixtures `SID = "sess-1"`, `T0 = 1_800_000_000.0` in test_stall.py, test_wake.py, test_posture.py; test_stall.py:72-131 literal 10/20/30 minute texts.
- **Splits:** The finder's split 5: stall.py CTX_TEXT claims an auto-compact at 500K / 50%, and nothing in settings.json env or the fragment sets it (V04: CONFIRMED for those two files). An unowned claim, not a value split.
- **Ruling quotes:** None found for the 10-minute stall, 5-minute fleet or 500K.
- **Tests:** test_wake.py:212-218 holds the fragment's 660 to `STALL_SECONDS + SCAN_EVERY`; test_docs_numbers.py `test_live_settings_carry_the_fragment` plus sync_settings.py hold the live file. Nothing holds the "500K" text to CTX_THRESHOLD (test_stall.py:410-456 pins literal 500,000).
- **Open:** Whether 50% / 500K is a Claude Code default (needs its docs or a run).
- **Hook gap:** none.

### L-F04-06: How often is the posture resent, and did the user answer that?
- **Source:** I9-079; F04 group E; V04 "Candidates" (K8 posture).
- **Kind:** K1, K8-shaped.
- **Verdict:** NOT VERIFIED beyond the comment's existence. V04: the comment exists at posture.py:16; "Whether any ruling backs it I did not search; F04's claim stands unchecked."
- **Homes:** G\posture.py:16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2` (V04 saw the comment). Finder only: G\README.md:35 (pointer, no value); H\docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:83; G\tests\test_posture.py:67-80 uses literals `61 * MIN` and `30 * MIN`. The Q8 in 2026-10-25-hooks-from-scratch.md:204 is a different question (finder).
- **Splits:** None.
- **Ruling quotes:** None found (finder searched rulings\*.md for resend, hour, posture).
- **Tests:** test_posture.py uses the constant at the edge and literals after.
- **Open:** Whether a ruling backs "the user's answer".
- **Hook gap:** none.

### L-F04-07: What is the hook code line budget, and is it a hard cap?
- **Source:** I9-179; F04 group F, splits 1 and 2; V04 splits 1 and 2.
- **Kind:** K2, K3, K8.
- **Verdict:** CONFIRMED (both splits). V04 did not recheck replay.py:489 (exit 1) or the 5,572-line count.
- **Homes:** G\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget for hook code; only the user changes it` (V04 quoted). Finder only: replay.py:24 docstring "The run fails (exit 1) when the hook code is over LINE_BUDGET"; plan 2026-10-25-hooks-from-scratch.md:21 "about 2,200 lines", :257 "fails over 3,000"; cards H\docs\sessions\2026-10-30-83eb8bac.md:16,25, 2026-10-31-adbda2b2.md:13, 2026-10-31-6777b546.md:17,22.
- **Splits:**
  - Split 1 (value): code says 3100; the only Confirmed line naming a number is review-and-hooks.md:13, chose "Raise cap to 3,050". 3,100 appears only at line 44 under `## Unconfirmed` (header line 37) and in a session card. Situation: any run over 3,050 and up to 3,100 lines.
  - Split 2 (meaning): replay.py:41 calls it a hard budget and exits 1 when over; Confirmed review-and-hooks.md:11 (2026-10-31) "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". The card 2026-10-31-6777b546.md:17 reads that as "the cap is only information". Situation: a hook total over the cap.
- **Ruling quotes:** review-and-hooks.md:13 (Confirmed, "Raise cap to 3,050") and :11 (Confirmed, quoted above). V04: no Confirmed line names both 3,050 and 3,100; the :11 line does not name both sides of split 2.
- **Tests:** None holds replay.py's number to any other home.
- **Open:** replay.py:489 exit 1; the line total.
- **Hook gap:** none.

### L-F04-08: May an agent type file set a turn cap?
- **Source:** I9-025; F04 group G; V04 "Candidates" (K7).
- **Kind:** K7.
- **Verdict:** CONFIRMED for README.md:128. V04's grep did not match the preamble wording.
- **Homes:** G\README.md:128 "No agent file in ..\agents\ sets `maxTurns`" (V04 saw). Finder only: PRE:73 "no agent type file sets a turn cap behind it"; test_docs_numbers.py:34-36,100-108 greps agents\*.md for maxTurns. Side note (finder only): settings.json:4 `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH: "1"`, not mentioned in README.md:128.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py (finder).
- **Open:** Preamble wording match; spawn-depth setting was not reached by V04.
- **Hook gap:** none.

### L-F04-09: How many builds may run at once, and who states it?
- **Source:** I10-031, I10-032, I10-033, I10-035, I10-036, I10-037, I10-046; F04 group H; V04 "Candidates" (three builds).
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED. V04: H\CLAUDE.md:76, agent-rules.md:20 and development.md:114 say "at most three builds run at once". PRE:57 says "You never build ... (agent rule 14)" and points to rule 14; it does not state three on that line. So the F04 claim of four docs is three plus a pointer. V04 did not reach the 15 s / 60 s build-slot values.
- **Homes:** H\tools\build_slot.ps1:3 "This file is the one place that knows the slot rule, the pass rule and their numbers." with `$BuildSlotCount = 3` (:75), `$BuildSlotPollSeconds = 15`, `$BuildSlotMessageSeconds = 60`, `$BuildPassUnclaimedSeconds = 120` (:80), `$BuildSlotFolderWaitMs = 5000`, `$BuildSlotYoungSeconds = 5`, and an unnamed `1..10` with `Start-Sleep -Milliseconds 200` (:288-291) (finder). Copies finder only: build_slot.ps1:74 cites the ruling; fix-decisions.md:1799 D314; H\tools\test_build_pass.ps1:115-119,129,256-358 (literals 3, 120, slot3.lock); MEM one-build-per-wave.md:13.
- **Splits:** None on three.
- **Ruling quotes:** agents-and-workflow.md:8 (Confirmed, 2026-11-07) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." (V04 confirmed: covers the three-builds limit only.) Names no split.
- **Tests:** test_build_pass.ps1 pins 3 and 120 as literals, not read from build_slot.ps1; nothing holds the doc quotes to the constant.
- **Open:** 15 s, 60 s, and the unnamed 1..10 / 200 ms values not reached.
- **Hook gap:** none.

### L-F04-10: Where did the 120-second unclaimed-pass value come from?
- **Source:** I10-032 area; F04 group H, split 4; V04 split 4.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** build_slot.ps1:79 "The user's answer in the same rulings page" (the claim). agents-and-workflow.md:35, under `## Unconfirmed` (header line 32): "Yes, all recommended" to seven questions, one being "(an unclaimed pass runs out after 2 minutes, 120 seconds)" (V04 quoted). test_build_pass.ps1:129 literal. H\tools\hydra_ui_mcp.py:92 `BUILD_PASS_TIMEOUT_S = 30` is a different wait (finder).
- **Splits:** An owner claim that points at text no Confirmed line holds; not a value split.
- **Ruling quotes:** agents-and-workflow.md:8 (Confirmed) covers the three-builds limit only (V04).
- **Tests:** test_build_pass.ps1 pins 120 as a literal.
- **Open:** None named.
- **Hook gap:** none.

### L-F04-11: Does the build-slot default folder path exist on this machine?
- **Source:** F04 side notes (build_slot.ps1:78; bench_run.ps1:8-9); V04 "Side notes".
- **Kind:** K1 (path).
- **Verdict:** REFUTED. Finder said C:\Users\Patrick\.claude\hooks does not exist. V04: it exists and holds `state\build_slots`, so build_slot.ps1:78 and the bench_run.ps1 comment are real paths.
- **Homes:** Finder's claim: build_slot.ps1:78 default `...\.claude\hooks\state\build_slots`; bench_run.ps1:9 same in a comment.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** None.
- **Hook gap:** none.

### L-F04-12: What are the helper's timing and size numbers in hydra_ui_mcp.py, and who copies them?
- **Source:** I10-050, I10-051, I10-052, I10-053, I10-054, I10-063; F04 group I; V04 "Not reached".
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: H\tools\hydra_ui_mcp.py:41 "This is the one home of these three numbers." `REPLY_MARGIN_S = 15.0`, `HELLO_TIMEOUT_S = 60.0`, `REPLY_CAP_CHARS = 12_000`, plus `LOG_TAIL_LINES`, `CONNECT_RETRY_S`, `GIT_TIMEOUT_S`, `BUILD_PASS_TIMEOUT_S`. ui-testing.md:280,284 name them with no values. H\tools\test_hydra_ui_mcp.py:293 expects the literal "[cut at 12,000 characters"; :289 uses `hm.REPLY_CAP_CHARS`. The timeout in force: H\tests\ui\uitest_commands.h:17-19 `double timeout = 30.0`, named by hydra_ui_mcp.py:46-47.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_hydra_ui_mcp.py:289 uses the constant; :293 holds the literal.
- **Open:** All.
- **Hook gap:** none.

### L-F04-13: Does the third-party REA skill quote the tool's defaults, and does anything check them?
- **Source:** I8-225, I8-226, I8-230, I8-233; F04 group J; V04 "Not reached".
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: C:\Users\Patrick\.claude\skills\reverse-engineer-anything\references\javascript-applications.md:36-40 "Defaults are 1024 MiB and 300000 ms", "512 MiB runtime reserve"; SKILL.md:133-134, 149-150, 186-187 (50 frames, 50 lines, 20 rows); references\native-and-artifacts.md:33 "samples 20 rows"; references\evidence-workflows.md:51-56. The owner (the REA server) is outside the readable scope.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All; the REA defaults could only be read from the installed server.
- **Hook gap:** none.

### L-F04-14: How many times does the handoff checker run, and what is done with a row still `missing`?
- **Source:** I8-242; F04 group K, split 3; V04 split 3.
- **Kind:** K3, K7.
- **Verdict:** CONFIRMED.
- **Homes:** H\.claude\commands\handoff.md:12 "If a row is still `missing` after that second run, leave it, and say which row and why in your report." H\docs\agents\handoff.md:65 "fixes every `missing` row it finds". Finder only: PRE:97 and handoff.md:67 (continuing session reports each MISSING or CHANGED SINCE line).
- **Splits:** Same command, two actions. The command leaves a row still missing after the second run and reports it; handoff.md:65 says the command fixes every missing row. handoff.md gives no number of runs; check_handoff.py has no owner for "twice" (finder).
- **Ruling quotes:** None found (V04: found no ruling on it).
- **Tests:** None (finder searched tools\test_check_handoff.py for "second").
- **Open:** None named.
- **Hook gap:** none.

### L-F04-15: How is the handoff date found, and how is the session-start pointer check run?
- **Source:** I10-022, I10-015; F04 group K; V04 "Not reached" (not listed; no V04 text covers them).
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: H\tools\check_handoff.py:398-413 `handoff_date` (file name date, else last git commit date, else today with a warning); handoff.md:7,59 state only the first step; .claude\commands\handoff.md:11. check_pointers.py registered at G\settings.fragment.json:104 and live settings.json:114, both `--session-start` with `"timeout": 15`; G\README.md:71.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py `test_live_settings_carry_the_fragment` and sync_settings.py hold the two settings files equal (finder).
- **Open:** All.
- **Hook gap:** none.

### L-F04-16: Where is the SP squeeze search window (500 ms) written, and do the pointers to it still hold?
- **Source:** I8-248; F04 group L and side notes; V04 "Side notes".
- **Kind:** K1, K8.
- **Verdict:** CONFIRMED for the two stale pointers (V04 read both). The other homes were not reached by V04.
- **Homes:** Owner (finder): H\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0`. V04 confirmed: H\docs\clone-hero\05-star-power.md:417 says "src/core/model.h line 111", but the constant is at model.h:121; C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 names src/search/graph.h, but the constant is in src/core/model.h:121. Finder only: MEM sp-squeeze-mechanics.md:13; 05-star-power.md:413-417 SP-38.
- **Splits:** None.
- **Ruling quotes:** H\docs\agents\rulings\clone-hero.md:12 (finder: Confirmed block starts line 7, Unconfirmed at 67) "ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." (2026-11-12). Names the 500 ms; no split.
- **Tests:** No test holds the memory or skill to the constant.
- **Open:** None named.
- **Hook gap:** none.

### L-F04-17: Where is the 3 ms backend leeway written?
- **Source:** I8-251; F04 group L; V04 "Not reached" (SP leeway).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;` and `core::counted_without_squeeze` (backend_value.h:39); MEM sp-end-is-one-point-leeway-is-delgars.md:13; MEMORY.md:7; 05-star-power.md:453-461 SP-42; model.h:98 comment "a user rule".
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** No test pins the doc's "3 ms" to rules.h (not searched beyond a name grep).
- **Open:** All.
- **Hook gap:** none.

### L-F04-18: How many charts does the library hold, as the memory notes and rulings state it?
- **Source:** I8-254; F04 group L, split 6; V04 split 6.
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED. Finder said five different counts. V04: four counts, each a dated example. search-before-clicking-a-song.md:17 holds only the prefix "of 19", a check string, not a count. The ruling-file figures were not rechecked.
- **Homes:** MEM no-full-library-runs.md:17 "18,811"; min-path-delta-proof.md:29 "18,610"; stored-versions-one-rule.md:11 "18,755"; measure-small-explain-first.md:11 "19,000-chart" (all V04 confirmed). Finder only: rulings\code-and-engine.md:27 "about 18,848 charts"; rulings\ui-and-display.md:20 "your 19,610 charts" (question text beside a Confirmed choice, not the user's words, per the finder). No owner in scope; the live count is the user's hydra.db.
- **Splits:** Several figures for one live quantity; none states itself as a rule.
- **Ruling quotes:** The two ruling lines are question text, not the user's words (finder; V04 did not recheck).
- **Tests:** none named.
- **Open:** The two ruling-file figures.
- **Hook gap:** none.

### L-F04-19: Do the test fixtures in test_common.py have one home?
- **Source:** I9-016, I9-020; F04 group M; V04 "Not reached".
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: G\tests\test_common.py:30 `RACERS, HOLD_SECONDS = 16, 0.02`; test_common.py:213-224 sets `_LOCK_WAIT = 0.05`; owner G\common.py:18 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

### L-F04-20: (side note) Is the subagent-depth setting covered by the turn-cap text?
- **Source:** F04 side notes; no verifier text.
- **Kind:** K7.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: C:\Users\Patrick\.claude\settings.json:4 `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH: "1"`; PRE:77 "You have no helper agents"; G\README.md:128 does not mention the setting.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

---
# Finder F05 (verifier V05)

### L-F05-01: What CPU limit does the quiet check use, and which plan still says 15?
- **Source:** I10b-001, I10b-002, I10b-003; F05 splits and rows; V05 section 1.
- **Kind:** K2, K1.
- **Verdict:** CONFIRMED-CHANGED. V05: the old 15 is in two places in the plan, line 74 and line 303 (premise row P22, "CPU over 15 percent ... checked 2026-11-12"); the finder named only line 74. V05 calls the plan a dated record: stale history, not a live reader.
- **Homes:** H\tools\ch_mcp\quiet.py:29 `QUIET_CPU_PCT = 30` (:30 `QUIET_SAMPLE_S = 5`). H\docs\superpowers\plans\2026-11-12-player-stall-fixes.md:74 and :303 say 15. rulings\clone-hero.md:72 (Unconfirmed: `## Unconfirmed` at line 67): "raise the limit to 30%... So `QUIET_CPU_PCT` in tools/ch_mcp/quiet.py is 30." Finder only: test_quiet.py:126-128 and test_game_tools.py:2928 use the constant by name.
- **Splits:** 15 (plan lines 74 and 303) against 30 (quiet.py and the Unconfirmed line 72). Situation: a machine using between 15 and 30 percent CPU while the check runs.
- **Ruling quotes:** None under Confirmed for either value (clone-hero.md has Confirmed at line 7, Unconfirmed at 67; line 72 is Unconfirmed).
- **Tests:** test_quiet.py:126 pins "exactly at the threshold is quiet".
- **Open:** None named.
- **Hook gap:** none.

### L-F05-02: How long does ch_start's quiet check take ("about 6 s")?
- **Source:** I10b-004; F05; V05 did not mention.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: tools\ch_mcp\server.py:128 (MCP instructions) and :158 (docstring) "about 6 s"; code side a 5 s sample (quiet.py:29) plus a folder scan capped at 3 s (quiet.py:46). A hand-derived value in prose; no code row says 6.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** None.
- **Open:** All.
- **Hook gap:** none.

### L-F05-03: Does a user ruling back the 3-second folder-scan cap that a code comment cites?
- **Source:** I10b-005; F05; V05 "Other lines".
- **Kind:** K8.
- **Verdict:** CONFIRMED. V05 grepped "2026-11-07" in docs\agents\rulings\*.md: hits only at agents-and-workflow.md:8, :9, :34, :35, none about a scan cap. Cards and question files were not searched.
- **Homes:** tools\ch_mcp\quiet.py:43 "The user's ruling of 2026-11-07: how long the folder scan may run", `SCAN_CAP_S = 3.0` (V05 quoted); comment also states "1.5 s for 31 folders" (finder).
- **Splits:** A comment claims a ruling no Confirmed line holds.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** Cards and question files not searched.
- **Hook gap:** none.

### L-F05-04: What caps do the bot-run and human-launch limits use, and do rulings back them?
- **Source:** I10b-009, I10b-011; F05; V05 did not mention.
- **Kind:** K8.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: bot_run.py:63-68, 85-91; comments cite a removed plan (git show 444631e6) and "the direct-start plan's question 1, answered by the user on 2026-10-31". docs\agents\clone-hero-mcp.md gives no values.
- **Splits:** None.
- **Ruling quotes:** None found under Confirmed (finder).
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

### L-F05-05: Timing and cap numbers in the Clone Hero tools (install, state reads, kit, frame probe, SP watch, trace, recorder, report, TypeScript agent) with one home each plus prose copies
- **Source:** I10b-024, I10b-027, I10b-031, I10b-036, I10b-038, I10b-040, I10b-042, I10b-043, I10b-047, I10b-048, I10b-049, I10b-050, I10b-052, I10b-055, I10b-056, I10b-063, I10b-065, I10b-085; F05; V05 did not mention.
- **Kind:** K1, K8-shaped.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: game_tools.py:45 `INSTALL_CAP_S = 60.0`; frida_tools.py:43-49 with tools\ch_mcp\README.md:85 "far inside the 5 s cap"; method_watch.py:47-48 restates 120 as "cap_s=120"; kit_tools.py:96-123 and :733-740 (comments quoting the user); sp_tools.py:49-66, trace_hits.py:57-74 ("approved by the user on 2026-10-28"); machine_recorder.py:41-60; machine_report.py:38-52; agent\clock.ts:14, kit.ts:72/82, trace.ts:37/43; ch_probe\constants.py:349-358. Pointers only (no value): clone-hero-mcp.md:55, server.py:356.
- **Splits:** None.
- **Ruling quotes:** Found by the finder (not homes): rulings\clone-hero.md:27 chose "Keep 60 s" (install cap, INSTALL_CAP_S); :20 chose "Accept all four (Recommended)" for the 5 s frame probe; :16 "Yes, those numbers (Recommended)" and :14 "Start them, 5 s cap (Recommended)" (stall hunt). No Confirmed line found for the kit watch cap (60 s), SP/trace caps, or the recorder numbers.
- **Tests:** none named.
- **Open:** All; single-name or number searches only for I10b-043, 048, 052, 055, 065, 085 (finder).
- **Hook gap:** none.

### L-F05-06: Do two code readers parse settings.ini?
- **Source:** I10b-040 / 042; F05; V05 section 5.
- **Kind:** K6.
- **Verdict:** CONFIRMED (V05: arguable). V05 did not open game_files.py.
- **Homes:** tools\ch_mcp\machine_report.py:49-52 says the lines "sit in [video], so game_files.get_game_setting, which reads [game], cannot" (V05 quoted); machine_report.py `_settings_lines` (250-262); game_files.py:217-256 (finder).
- **Splits:** Two readers of one file; each reads a different section, with a stated reason. Not the same question for the same section.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** game_files.py not opened by V05.
- **Hook gap:** none.

### L-F05-07: Is the menu step SEARCH_NEXT_ITEM = Blue a guess?
- **Source:** I10b-044; F05; V05 section 6.
- **Kind:** K4.
- **Verdict:** CONFIRMED.
- **Homes:** menu_tools.py:44-51, :48 "So Blue is a guess: live step LS-K3..." (V05).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** Only a live run on the real menu settles it.
- **Hook gap:** none.

### L-F05-08: Does the "never scroll the library unasked" ruling cover Clone Hero's own song list (SEARCH_DOWN_CAP = 10)?
- **Source:** I10b-044; F05; V05 "Other lines".
- **Kind:** K8.
- **Verdict:** NOT VERIFIED. V05 did not check that ui-and-display.md:27 sits under Confirmed and left coverage for the main session.
- **Homes:** menu_tools.py:39 `SEARCH_DOWN_CAP = 10` (V05 saw; it scrolls Clone Hero's list).
- **Splits:** The ruling is about Hydra's library; the cap acts on Clone Hero's list. Situation: ten scroll presses through the game's list.
- **Ruling quotes:** rulings\ui-and-display.md:27 (2026-10-30; finder gave the text, section unchecked by V05): "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes to reach the song you want with no explanation. this should never happenw tihout my approval." Whether it names both sides: it names Hydra's library, not the game's list.
- **Tests:** none named.
- **Open:** Whether the line is under Confirmed; whether it covers the game's list.
- **Hook gap:** none.

### L-F05-09: Are the Clone Hero menu class names held in two files?
- **Source:** I10b-044; F05; V05 did not mention.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: menu_tools.py:54-57 (the four names, "by the name agent/menu.ts sends") and agent\menu.ts:47-48 `MENU_CLASSES`; PLAYER_SCREEN_FRAME_RVA 0x32F590 (menu.ts:54) also in docs\clone-hero\code\6142\player_menu_on_ready_page_32F590.md.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** menu.ts:50 says tests\test_frida_tools.py checks the RVA against the dump's facts (finder; not opened).
- **Open:** All.
- **Hook gap:** none.

### L-F05-10: Is Hydra's hit window "flat 85 ms", and how many places hold 85?
- **Source:** I10b-060, I10b-061, I10b-062; F05 splits (K2/K3); V05 section 2.
- **Kind:** K1, K2, K3.
- **Verdict:** CONFIRMED-CHANGED (finder partly wrong). V05: the K2 drift is REFUTED. The README reads true of Hydra's rating model: kHitWindowMs = 85.0 is kept by ratings, tiers, badges and search, while the clamps kWindowFrontMs 37.5 / kWindowBackMs 85.0 and the formula are display only. V05 added a copy the finder missed: SPEC.md:302.
- **Homes:** H\src\core\model.h:106 `kHitWindowMs = 85.0`; H\src\core\note_window.h:8-11 says it is "a separate number (D270 call 8): the user ruled 'just show it alongside.'" (V05 quoted; D270 not opened by V05), clamps and formula at note_window.h:30-45. H\tools\ch_probe\README.md:101 "Hydra's model uses a flat 85 ms hit window"; ch_probe\SPEC.md:302 "Hydra's model uses a flat `kHitWindowMs = 85.0`"; ch_probe\constants.py:89-92,227,241-243 hold 85, 37.5, 171.4313, 96.2932 (K1 Python copy). Finder only: SPEC.md:107,151-152,166-167,277; docs\agents\test-rubric.md:21 and test-rubric-examples.md:179 "85 ms hit window"; test_trace_hits.py:222,249 "within 85 ms".
- **Splits:** Two engine homes for 85 ms (kHitWindowMs and kWindowBackMs); note_window.h describes them as separate on purpose (a quote; V05 did not find the cited ruling in a Confirmed line).
- **Ruling quotes:** rulings\code-and-engine.md:23-24 and :49 are on an 85 ms early fill (finder; section of :49 unchecked). The note_window.h quote "the user ruled 'just show it alongside'" was not found in a Confirmed line by V05 (D270 is in the decision log).
- **Tests:** tests\test_note_window.cpp:38,74,77 and tests\test_replay_schedule.cpp:79 pin 171.4313075 (finder). Nothing ties constants.py to the C++; test_s2_window_constants.py ties constants.py only to the measured CSV.
- **Open:** Whether tests link constants.py to note_window.h (V05 did not re-search).
- **Hook gap:** none.

### L-F05-11: Do the Python hit-window constants and the C++ owner have a test between them?
- **Source:** I10b-060, I10b-061; F05; V05 section 2 last line.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED. V05: "I did not re-search tests for a link between constants.py and note_window.h."
- **Homes:** See L-F05-10.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** Finder: none ties Python to C++.
- **Open:** Whether such a test exists.
- **Hook gap:** none.

### L-F05-12: Does the report-button script hold a hand-measured copy of a layout the UI computes, and are the capture waits copies?
- **Source:** I10b-069, I10b-070; F05; V05 did not mention.
- **Kind:** K4, K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: tools\capture\report_shot.ps1:5-28 `ClickFromRight 80` ("2,560 - 2,480 on the user's maximized window"); the UI places the control at src\ui\widgets.h:197 (used at library_toolbar.cpp:140); capture\steps.ps1:13-17, shot.ps1:7.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** All.
- **Hook gap:** none.

### L-F05-13: How many places hold the 12-second capture wait?
- **Source:** I10-076; F05; V05 section 3.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** tools\capture_window.ps1:56 `$Wait = 12`; tools\capture\report_shot.ps1:10 and steps.ps1:8 pass a literal `12` (V05). Finder: shot.ps1:4 passes its own $Wait; WaitForExit(10000) has one home.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** None named.
- **Hook gap:** none.

### L-F05-14: Do the doc picture limits (500 KiB and 4 MiB) have two homes that each call the other the same limit?
- **Source:** I10b-071; F05; V05 section 3.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** tools\doc_shots\shrink.py:17-21 (`500*1024`, `4*1024*1024`; comment "The user's limits (D300 ...). tests/test_docs_match_code.cpp checks the same two at merge"); tests\test_docs_match_code.cpp:2022-2025 (same numbers; comment names shrink.py) (V05: each comment names the other; neither reads the other). docs\agents\doc-screenshots.md:17 points to shrink.py (finder).
- **Splits:** None.
- **Ruling quotes:** None found (D300 is in the decision log; finder).
- **Tests:** None holds them equal.
- **Open:** None named.
- **Hook gap:** none.

### L-F05-15: Do tests hold literals copied from constants (12,000 characters; 85 ms) and does the stand-in session carry its own numbers?
- **Source:** I10b-074; F05 side notes; V05 did not mention.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: tools\test_hydra_ui_mcp.py:293 "cut at 12,000 characters" (see L-F04-12); fake_hydra_live.py:57-65 (BIG_TEXT sized to exceed REPLY_CAP_CHARS; SESSION_TIMEOUT_S 25 "Deliberately not the real exe's starting value"); ch_mcp\tests\test_trace_hits.py:222,249.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** Those tests are the subject.
- **Open:** All.
- **Hook gap:** none.

### L-F05-16: Do tests pin literals copied from the precheck constants?
- **Source:** I10b-077, I10b-078, I10b-079, I10b-081, I10b-086, I10-083, I10-085, I10-086; F05; V05 section 4.
- **Kind:** K1.
- **Verdict:** CONFIRMED. V05 did not open test_precheck_pick_exe.cpp strings.
- **Homes:** Owners (finder): tools\precheck\definitions.h:15-25 (30 / 200); items.h:45-50 (400 lines, "This is the one place it lives."); check_5.cpp:22-28 (12 / 3 / 4); test_pick.h:22-30 (30 / 40 / 20). Copies V05 saw: tests\test_precheck_git.cpp:200-202 pins `RangeSize{400,0}` and "over the 400-line threshold"; tests\test_precheck_pick.cpp:75-77 pin `kCommonNameLimit == 30`, `kPickSizeLimit == 40`, `kHeaderIncluderLimit == 20`. Finder only: test_precheck_pick_exe.cpp:195-203 strings "than 20", "than 40"; derive-once-review.md:21 points to the precheck (true).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** The tests are the copies; test_precheck_check1.cpp:193 and test_precheck.cpp:417 use the constants by name.
- **Open:** pick_exe strings.
- **Hook gap:** none.

### L-F05-17: Replay schedule numbers (frame 3.0, judge frame, press lead 2, SP-end lead 15, SP-out lag 15, order gap 8, room 3, window clamps) and the other "3 ms"
- **Source:** I10b-091 to I10b-097, I10b-098; F05; V05 did not mention.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: tools\replay_schedule.cpp:20-57 (comments cite plans, one removed: git show d7e47a20, and "the user chose 'One frame', D319 item 3"); :61-62 read kWindowFrontMs and kWindowBackMs from core\note_window.h (a true pointer); derive-once-review.md:64 worked example "3.00 ms is the one frame of judging the tool counts". Same number, other meaning: MEM sp-end-is-one-point-leeway-is-delgars.md:3,11 "3 ms" backend leeway; kFrameMs and kRoomMarginMs both 3.0.
- **Splits:** Three 3.0-ish numbers with three meanings (finder side note), not a split in value.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

### L-F05-18: What is the mutation time limit rule ("clean time plus 5 s"), and how many places state it?
- **Source:** I10b-102; F05; V05 section 3.
- **Kind:** K1.
- **Verdict:** CONFIRMED. V05 counts three live homes, not the finder's four. V05 did not open the tests pinning 5.0.
- **Homes:** tools\mutate\run.py:89-100 (rule and TIME_LIMIT_RULE); run.py:967 a print string "(each run's clean time plus 5 s)" not built from TIME_LIMIT_RULE; H\docs\development.md:732-734 states the rule while saying its one home is `test_timeout_seconds` in run.py (V05). Finder only: py_mutate.py:105,581 (pointer); tools\mutate\test_run.py:228,274,405-406 pin 5.0; card docs\sessions\2026-11-16-60c3a576.md:26 holds the user's quote (a card, not a ruling).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_run.py pins 5.0 (not opened by V05).
- **Open:** Tests pinning 5.0.
- **Hook gap:** none.

### L-F05-19: Is the three-builds rule and the build-pass tool stated in prose that a hook already owns?
- **Source:** I10-064; F05; V05 sections 3 and 7.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED. No test between the prose and the constant seen (V05).
- **Homes:** Owner H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`; hook percall.py refuses the pass tool to every agent (finder: percall.py:109 refusal message). Prose: G\README.md:15 explains the build-pass refusal and quotes the ruling (V05); PRE:57, H\CLAUDE.md:76, agent-rules.md rule 14 "At most three builds"; development.md:114. Finder only: tools\hydra_ui_mcp.py:87-93 `BUILD_PASS_TIMEOUT_S = 30`; build_slot.ps1:195,197 (one pass per session; refuses at count >= 3).
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:8 (Confirmed, 2026-11-07) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." Names the limit and who builds; no split.
- **Tests:** tools\test_build_pass.ps1 runs the script; V05 saw no test between prose and constant.
- **Open:** test_build_pass.ps1 not opened by the finder for a pinned 3.
- **Hook gap:** none.

---
# Finder F06 (verifier V06)

### L-F06-01: Which tool names are key tools, and which count only on route "keys"?
- **Source:** I1-001, I1-002; F06 group A; V06 group A.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED. V06 opened the code homes; the prose homes are the finder's.
- **Homes:** G\percall.py:16 `KEY_TOOLS = ("mcp__ch__ch_key", "mcp__ch__ch_press")` and :18 `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`; H\tools\ch_mcp\game_tools.py:71 `ROUTE_KEYS = "keys"` (V06: percall.py imports neither; test_progress.py:628 pins the percall copy to a typed literal; the finder cites :520). Finder only: server.py:148,389-391,466,477; H\docs\agents\clone-hero-mcp.md:29; G\README.md:13; percall.py:94-95 refusal text; MEM ch-runs-must-not-need-focus.md:17; G\review_rules.py:54 `KEY_ALLOW_TEXT = "allow key run"`; README.md:37.
- **Splits:** None; all homes agree today.
- **Ruling quotes:** None found (finder searched docs\agents\rulings for "key run", "key route", "SendInput").
- **Tests:** G\tests\test_progress.py pins percall.KEY_TOOLS to a typed copy; test_percall_key.py. No test compares percall's list to the tool list in server.py.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-02: Which tools are shell tools?
- **Source:** I1-018; F06 group B; V06 group B.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED.
- **Homes:** G\progress.py:20 `SHELL_TOOLS = ("Bash", "PowerShell")` (imported by percall.py at lines 47, 112, 133, 147; V06). G\delete_rule.py:168 own literal `("Bash", "PowerShell")`; G\tests\shell_output_sizes.py:20 typed copy (V06). Finder only: shell_output.py:35-36 `TAILS` keys; settings.fragment.json:192 matcher "Bash|PowerShell"; bash_windows_rule.py:235 judges Bash only; PRE:31-33.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** No test holds them equal.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-03: In what order do the hook refusals run, for a subagent and for the main session?
- **Source:** I1-008; F06 group B; V06 group B (named in the heading only).
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED at group level. V06's detail names only SHELL_TOOLS and S3; it gives no separate reading of the order.
- **Homes:** G\percall.py:1-6, :99-139 (owner; finder). G\README.md:11 restates the order in prose; finder compared and found them equal today (subagent: delete_rule, library, bash_windows, media(main=False), key; main: library, key, bash_windows, media, stall); PRE:5 lists rules without order.
- **Splits:** None. Order differs between the two sides (finder).
- **Ruling quotes:** None found.
- **Tests:** test_refusals.py and test_percall_rewrite.py test order; not compared to README (finder).
- **Open:** V06 did not itemise this row.
- **Hook gap:** none.

### L-F06-04: Which worktree folders does the delete rule trust?
- **Source:** I1-048; F06 group B, split S3; V06 group B.
- **Kind:** K3, K1, K5.
- **Verdict:** CONFIRMED.
- **Homes:** G\delete_rule.py:17-18 `WORKTREE_FOLDERS` holds two exact folders, `...hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees` (V06). MP:5 "each in its own worktree under a `worktrees` folder (the delete rule trusts only those)" (V06). Finder only: G\README.md:136; PRE:43 and derive-once-review.md:197 name the two exact folders; delete_rule.py:1-3, percall.py:112-119 (subagent only); PRE:43 spells the delete rule out (K5).
- **Splits:** The code trusts two exact folders; MP:5 and README:136 read as any folder named `worktrees`. Situation: a worktree at H\worktrees\ (this audit's own worktree is H\worktrees\tooling-audit-records, in neither folder; V06 saw it in its environment), where an agent could not delete its own files.
- **Ruling quotes:** review-and-hooks.md:17 (Confirmed, 2026-10-25) chose "Keep rule, move worktrees" to a question saying "a worktree that sits in a `worktrees\` folder". V06: names the folder generically, not the two exact paths; names one side, not both.
- **Tests:** none named; the delete rule's text is not held by test_docs_numbers.py (finder).
- **Open:** None named.
- **Hook gap:** none.

### L-F06-05: What tells an agent its id and status file, and does a brief carry the status rule?
- **Source:** I1-027, I5-062, I5-066, I1-092; F06 group C, split S1 and K8; V06 group C.
- **Kind:** K1, K2, K3, K8.
- **Verdict:** CONFIRMED (S1 and the K8 on the memory note). V06: K8 holds "with a reading caveat" (the note says "the dispatch gate", not the exact retired name).
- **Homes:** MP:11 "A brief names no status file and no id; it carries only the preamble's status rule." (V06). MEM every-brief-carries-status-rule.md:3 "the brief must carry the status-file sentence"; MEMORY.md "every brief ... asks for a status line ... via gates\status_append.ps1" (V06). Finder only: G\progress.py:88-90 first-call note; PRE:9,13-19; derive-once-review.md:9 and doc-review.md:9; writer-brief.md:100 (a different line form `HH:MM done <n> of <m> cards | next: <session>`); G\README.md:122-126; MEM note "the dispatch gate now denies briefs that lack it".
- **Splits:** S1: MP:11 says a brief carries no status file or id and only the preamble's status rule; the memory note says the brief must carry the status-file sentence. Situation: writing a brief (this brief's line 4 follows MP:11). K8: the memory note names a gate that review_dispatch.py cannot be that gate (count of "status" there is 0; settings.fragment.json:158 is the only Agent matcher, routing to review_dispatch, which judges reviewer keys only; test_docs_numbers.py:31 lists `reviewer_dispatch_gate` as RETIRED).
- **Ruling quotes:** agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05) and :26 (status in plain English), review-and-hooks.md:34 (finder). V06: say status is required; do not say how a brief carries it; they do not name both sides.
- **Tests:** test_docs_numbers.py:116 holds Hydra docs' "every N minutes" to 5 only.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-06: How many places decide whether an agent id is legitimate?
- **Source:** I5-066, I1-092; F06 group C (side note); V06 "Checked lightly".
- **Kind:** K6.
- **Verdict:** NOT VERIFIED. V06: "I did not open the code for them."
- **Homes:** Finder only: G\progress.py:23-26 id characters `[A-Za-z0-9_-]` with the call's id equal to the caller's own id; G\common.py:113-116 `check_id` (rejects only `/ \ :`); G\status_append.ps1:26-41 (name must have a record; progress.py:62-82 makes the record on every counted call).
- **Splits:** Three id rules with different tests (finder).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

### L-F06-07: Which commits carry trailers and which are exempt, and who reads them?
- **Source:** I3-006, I3-007, I3-020, I5-082, I5-084, I2-027, I2-048; F06 group D, S2; V06 group D (S2, S10).
- **Kind:** K1, K3, K4, K6.
- **Verdict:** CONFIRMED, extended by a new split S10 from V06.
- **Homes:** G\ref_judge.py:36 `TRAILER_KEYS`, :201, :225-232 refusal text; PRE:65-67. V06 added: ref_judge.py:186 reads trailers with `unfold,separator=%x2C`; agent_stop.py:117 reads the Agent trailer with `separator=%x09` (two code readers, different jobs: presence and identity). Finder only: G\agent_stop.py:112-124 `agent_commits` ("The one reader of the Agent trailer"), :127 `reviewer_commits`; stall.py:208-228 `_commits_fact`; G\README.md:5,53,166-168; derive-once-review.md:100,202; doc-review.md:95; integrate.md:27; MEM git-traps-in-shared-checkout.md:21; G\tests\test_ref_judge.py:22 typed `TRAILERS`; PRE:67,69 Co-Authored-By "Claude Opus 5.5".
- **Splits:** S2: PRE:65 names one exemption (merge commits); the refusal text (ref_judge.py:230-232) names two (merges and commits origin already has). S10 (V06): the code at missing_trailers (:188) `--not --branches ...known` also exempts commits already on any local branch, which the text and the preamble do not name. Situation: a commit already on a local branch. Also I3-006: only the hook message adds "or orchestrator" to `Agent:`. I2-027 / I2-048: "the agent's commits" has two scopes (stall counts everything since the first call in two repos; the reviewer path counts commits after the handed tip).
- **Ruling quotes:** review-and-hooks.md:30 (Confirmed, 2026-10-04) chose "Git-side hook (Recommended)" to "How should the review gate stop unreviewed changes reaching main?" (finder; not trailer-specific). review-and-hooks.md:57 is Unconfirmed (finder). Names no side of S2 or S10.
- **Tests:** test_ref_judge.py:22 typed copy; test_agent_stop.py exercises agent_commits; nothing holds ref_judge's parse equal to agent_stop's (finder).
- **Open:** K4 (V06): no hook checks the Co-Authored-By model line; PRE's default is "Claude Opus 5.5" and this session's attribution line names a different model (by reading ref_judge's TRAILER_KEYS use).
- **Hook gap:** none.

### L-F06-08: How many places decide whether a call or stop is the main session's?
- **Source:** I2-036, I2-037 (part), I2-067; F06 group E; V06 group E.
- **Kind:** K6.
- **Verdict:** CONFIRMED-CHANGED. V06: the finder's list is right but the tests differ in form.
- **Homes:** G\percall.py:163 tests `not data.get("agent_id")` only (V06). G\stall.py:421, :433, :469 test `agent_id or not session_id`; G\wake.py:97 (finder says :97-98) tests `agent_id or not session_id` (V06). Finder only: G\README.md:31,184; stall.py docstring "Main session only"; posture.py and prompt.py have no agent_id test.
- **Splits:** Two forms of one question: percall treats a hook input with neither agent_id nor session_id as the main session; stall and wake treat it as "not watched". Whether such input exists is open (V06).
- **Ruling quotes:** None found (finder searched rulings for Workflow, main session only).
- **Tests:** none named; stall's own test repeats percall's (finder).
- **Open:** Whether a hook input with neither id exists; a run would settle it.
- **Hook gap:** none.

### L-F06-09: Does a reviewer launched from a Workflow script count?
- **Source:** I2-037; F06 group E, split S4; V06 "CHANGED OR REFUTED".
- **Kind:** K1 (at most).
- **Verdict:** REFUTED as a split. Finder had left the merge path untraced; V06 traced it. agent_stop.py:200-203 looks for a handover record whose prompt_sha256 matches the brief, else raises `NoVerdict("no handover record matches its brief (was it dispatched through the Agent tool?)")`. Only the PreToolUse "Agent" hook in review_dispatch.py:170 writes a handover; a Workflow launch fires no Agent call. README:188 says so. The pages and the code agree.
- **Homes:** MP:5 "a reviewer from a Workflow script leaves no verdict"; derive-once-review.md:200 such a reviewer "gets no such record, so its sign-off counts for nothing"; G\README.md:180 (SubagentStop fires with agent_type reviewer for a Workflow agent), :188; agent_stop.py:238, :200-203; stall.py:436 `tool_name == "Workflow"`; prepass.py:252 (finder).
- **Splits:** None after V06's trace (the finder's S4 dropped).
- **Ruling quotes:** None found. V06: README:188 says the user approved it as "plan question Q2"; not looked up.
- **Tests:** No test found for the Workflow stamp equal to prepass's name list (finder).
- **Open:** Whether plan question Q2 is in the user's words.
- **Hook gap:** none.

### L-F06-10: Which agent type is a reviewer, what is a review key, and when is no key refused?
- **Source:** I2-040, I3-049, I3-050, I3-051, I3-052, I3-053, I5-107; F06 group F, S5; V06 group F.
- **Kind:** K1, K3, K5, K6.
- **Verdict:** CONFIRMED (S5 and the two REVIEWER_TYPE constants). V06 did not itemise the key-text (I3-050 to I3-052) or I5-107 homes.
- **Homes:** G\agent_stop.py:20 and G\review_dispatch.py:16 each declare `REVIEWER_TYPE` separately (V06). review_dispatch.py:155-159 returns None (allows) when no key is found and `_gated(...)` is false; denies only for a gated repo (V06). Finder only: review_dispatch.py:17-27,149-150,160-162 (key rules); G\README.md:39; derive-once-review.md:17,200,202; doc-review.md:29; MP:5; MEM git-traps-in-shared-checkout.md:17 and MEMORY.md; AG\reviewer.md; `judged_folder` for cross-repo briefs (MP:5, README.md:39, derive-once-review.md:200).
- **Splits:** S5: README:39 "The hook refuses a brief with no commit" and derive-once-review.md:17,200 "A hook refuses a reviewer dispatch with no key" read as unconditional; the code refuses only when the judged repo has a review marker on main (V06). Situation: a repo with no review marker on main. Also (finder): the pages say "more than one commit", the code counts 40-hex strings.
- **Ruling quotes:** review-and-hooks.md:18, :19 (Confirmed) concern the hand-back format, not the dispatch refusal; they name neither side of S5 (V06). review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done." (finder).
- **Tests:** none; no test compares the two REVIEWER_TYPE constants (finder).
- **Open:** `_key_problem` not read in full by the finder; I5-107 text not opened by V06.
- **Hook gap:** none.

### L-F06-11: Which tools may a card writer use, and what enforces it?
- **Source:** I4-018, I5-118, I5-119; F06 group G; V06 group G.
- **Kind:** K1, K5, K7.
- **Verdict:** CONFIRMED.
- **Homes:** C:\Users\Patrick\.claude\agents\card-writer.md:5 `tools: Read, Glob, Grep, Write, PowerShell`; writer-brief.md:33 limits PowerShell to one command (prose only) (V06); writer-brief.md:101 "20 minutes or 100 tool calls" is a typed copy of progress.py's numbers (V06). Finder only: card-writer.md:6,9 (`disallowedTools: Agent`; replaces the preamble); the card writer's rules live in writer-brief.md and card-writer.md.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** V06: neither test_docs_numbers.py (Hydra docs only) nor test_agent_files.py opens writer-brief.md.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-12: How is a card's agent count worked out, and how many places know where an agent transcript lives?
- **Source:** I4-085, I4-086, I4-087, I4-119; F06 group G; V06 "Checked lightly".
- **Kind:** K6.
- **Verdict:** NOT VERIFIED. V06 opened none of the three transcript-location homes; it confirmed only the `_TASK_ENDED` regex at agent_stop.py:51.
- **Homes:** Finder only: prepass.py:250-253, 143-170, 289-290; session_cards\common.py:68-70 (notice regexes); check.py:82-100; G\stall.py:175-185, 307; H\tools\agent_cost.py:160; agent_stop.py:51 `_TASK_ENDED` (V06: confirmed). Tool names "Agent", "Task" (prepass.py:250), review_dispatch.py:141, settings.fragment.json:158.
- **Splits:** None.
- **Ruling quotes:** None found (finder searched).
- **Tests:** test_session_cards.py:743-750 pins (1, 3, 7, 1, 8) to its own fixture, not to stall.py or agent_cost.py (finder).
- **Open:** All except the one regex.
- **Hook gap:** none.

### L-F06-13: Which model runs which role, and does the scout file match rule 2?
- **Source:** I5-032, I5-038, I5-105, I5-118, I5-120, I5-123, I5-129, I5-140, I5-143; F06 group H, S6; V06 group H.
- **Kind:** K1, K2, K3, K5, K6, K7.
- **Verdict:** CONFIRMED (S6).
- **Homes:** AG\scout.md:4 `model: sonnet` (V06); H\CLAUDE.md rule 2 and MP:5 say "Sonnet or Haiku" for scouts (V06). scout.md:5 lists Edit and Write in `tools:` while scout.md:3 says "read-only" and :9 "Change no files except your status line and files in your own scratch folder." (V06). The executor, planner, reviewer and scout files carry the identical ten-tool line, the card-writer five; all five have `disallowedTools: Agent` (V06). Finder only: agent-rules.md:8; derive-once-review.md:5,202; doc-review.md:5; four agent files say "You have no helper agents"; PRE:77; MP:5.
- **Splits:** S6: rule 2 and MP:5 allow a Haiku scout; scout.md fixes Sonnet, so a Haiku scout needs a per-dispatch override no page names. scout.md's tools line allows editing while its prose says read-only; the prose is the only enforcement.
- **Ruling quotes:** agents-and-workflow.md:12 (Confirmed, 2026-10-14) "use haiku for these simple transcript readers" (finder); :38 (Unconfirmed). Names neither side of S6.
- **Tests:** test_agent_files.py holds reviewer.md and planner.md to Premise-map phrases only, not tools or model. test_docs_numbers.py:100 checks maxTurns only, not disallowedTools.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-14: How many agents may run in parallel?
- **Source:** I5-042; F06 group I, S7; V06 group I.
- **Kind:** K2.
- **Verdict:** CONFIRMED.
- **Homes:** MEM lean-agent-fanout.md:21 "use 3-4 agents" for repeated discovery rounds; MEMORY.md:39 "3-4 agents per round" (drops "discovery") (V06). Finder only: H\CLAUDE.md:68, agent-rules.md:12, MP:5.
- **Splits:** S7: memory note and its index line carry a cap of 3-4 agents (note: for discovery rounds; index: no qualifier); Confirmed line "you dont need to stay limited to 10. run as many as you can." The scopes differ (jobs per agent or discovery rounds against agents overall). Situation: a round of parallel agents.
- **Ruling quotes:** agents-and-workflow.md:21 (Confirmed, 2026-10-03) "you dont need to stay limited to 10. run as many as you can."; also :17, :20, :23 (V06). V06: whether :21 covers discovery rounds is for the main session; memory notes are not the user's words. The line does not name both sides.
- **Tests:** none named.
- **Open:** Scope of :21.
- **Hook gap:** none.

### L-F06-15: What does a brief carry?
- **Source:** I5-103; F06 group I, S8; V06 group I.
- **Kind:** K3.
- **Verdict:** CONFIRMED (overlaps S1).
- **Homes:** MP:5 names the task id, session id, owned files, tests and the status rule; PRE:3 names the task id, session id, owned files, worktree or checkout, scratch folder and tests (V06). Each omits what the other names. MP:11 also bars status-file and id text (V06). Finder only: derive-once-review.md:197; G\README.md:136; MEM git-traps-in-shared-checkout.md:16; delete_rule.py:17-18.
- **Splits:** S8: MP:5 omits worktree and scratch folder; PRE:3 omits the status rule. Situation: writing a brief.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** None named.
- **Hook gap:** none.

### L-F06-16: How should a stuck agent be handled (nudge first or stop at once), and was the memory note's quoted choice made?
- **Source:** I5-111; F06 group I, S9; V06 "CONFIRMED-CHANGED" and "Checked lightly".
- **Kind:** K3, K8.
- **Verdict:** CONFIRMED-CHANGED. V06: a weaker split than the finder says. The hook text contains both words, in two different messages.
- **Homes:** MP:11 orders the steps: "nudge it once with SendMessage ... If it does not recover, stop it with TaskStop and re-dispatch" (V06). G\stall.py:124-125 and :143-144 tell the main session only "stop it with TaskStop and re-dispatch its remaining work", no nudge step; stall.py:460-461 (the check-in text) says "open its transcript, nudge it or stop it" (V06). Finder only: PRE:23; MEM relay-status-in-plain-english.md:15 quotes a 2026-11-16 choice ("Claim, plus disagreement").
- **Splits:** The stuck-agent notice skips the nudge step; MP:11 has it. Situation: the stuck-agent notice, not the periodic check-in.
- **Ruling quotes:** None found (V06: none for "supervis" or "nudge"). The finder found no Confirmed line for "Claim, plus" or "supervis"; V06 did not open the memory-note quote.
- **Tests:** none named.
- **Open:** The memory note's quote of the user's choice, unopened by V06.
- **Hook gap:** none.

### L-F06-17: Does a brief that names a commit in the other repo need that repo's root folder?
- **Source:** I5-107; F06 group I; V06 "Checked lightly" (cross-repo text not opened).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** Finder only: MP:5 "A brief may name a commit in the other repo if it writes that repo's root folder"; G\README.md:39; derive-once-review.md:200; owner G\review_dispatch.py `judged_folder`.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** All.
- **Hook gap:** none.

### L-F06-18: The reviewer's guides, the review range, and the handoff sentence
- **Source:** I5-121, I5-130; F06 group J; V06 group J.
- **Kind:** K1.
- **Verdict:** CONFIRMED as K1. V06 saw the handoff sentence in MP:13, PRE:95 and H\CLAUDE.md rule 9 (three); it did not open agent-rules.md for the fourth copy the finder counted.
- **Homes:** AG\reviewer.md:9; derive-once-review.md:17,53,202; doc-review.md:29; MP:5; AG\executor.md:9; PRE:95-97; MP:13; H\CLAUDE.md rule 9; agent-rules.md rule 9; handoff.md (owner page) (finder). The range `main...<tip>` word "main" is also at review_dispatch.py:21.
- **Splits:** None; the pages agree today.
- **Ruling quotes:** None found.
- **Tests:** test_agent_files.py holds reviewer.md to Premise-map phrases only.
- **Open:** The fourth copy of the sentence.
- **Hook gap:** none.

### L-F06-19: Prose that spells out a hook's rule, and nothing checking page wording against the hook (side notes)
- **Source:** F06 side notes; V06 closing hook-gap line.
- **Kind:** K5, K7.
- **Verdict:** NOT VERIFIED as a separate claim. V06's closing line repeats the general statement; V06 gave S5 and S4's earlier wording as examples (S4 is refuted).
- **Homes:** Finder only: PRE:33 and :43 spell hook rules out at length (delete rule roots and wildcard rules, the five Bash shapes, the 4000/30000 output limits) while naming the hook file as owner; test_docs_numbers.py:192-198 holds the Bash limit and output cap numbers but not the delete rule's text; writer-brief.md:101 typed copy (see L-F06-11); shell_output_sizes.py:20 and test_progress.py:520 typed copies.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:192-198 (limits only).
- **Open:** All beyond the general statement.
- **Hook gap:** hook gap: yes (finder's and verifier's general words; no single file named).

---
# Closing lists

**Candidates a verifier REFUTED (row ids):**
- L-F04-11 (the "hooks folder does not exist" side note; V04: it exists).
- L-F06-09 (finder's S4, a split between the Workflow-reviewer pages and the README; V06 traced the merge path and the pages agree with the code).
- Part of L-F05-10 (V05 refuted the K2 drift in the ch_probe README; the row as a whole is CONFIRMED-CHANGED, not REFUTED).

**Rows no verifier reached (NOT VERIFIED):**
- F04: L-F04-02, L-F04-03, L-F04-04, L-F04-06 (only the comment's existence seen), L-F04-12, L-F04-13, L-F04-15, L-F04-17, L-F04-19, L-F04-20.
- F05: L-F05-02, L-F05-04, L-F05-05, L-F05-08, L-F05-09, L-F05-11, L-F05-12, L-F05-15, L-F05-17.
- F06: L-F06-06, L-F06-12, L-F06-17, L-F06-19.
- Parts left open inside rows that are otherwise CONFIRMED or CONFIRMED-CHANGED are named in their Open lines (for example, V04 did not reach the build-slot 15 s / 60 s values in L-F04-09, and V06 did not itemise the key-text homes in L-F06-10).
