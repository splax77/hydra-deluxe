# TOOL-AUDIT-T5-F04 (finder F04, agent adb2f4b8add62fa3d)

53 rows, grouped by the question they ask. Every row id is under exactly one group. Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, M = the memory folder.
No row was skipped. Depth is uneven: groups A, B, F, H are searched fully; J, L, M are single-home or light.
Searched in every group: all of scope in the brief (gates incl. README, tests, settings.fragment.json, live settings.json; agents, model-postures.md, CLAUDE.md both, skills, tools\session_cards, H docs\agents with rulings, .claude\commands, H tools, memory notes), with greps by number, by number written out in words, and by question.

## Group A. How many calls, how many minutes, how old a status line (I9-027, I9-028, I9-029, I9-084, I9-085, I9-086, I9-096, I8-247)
Question: how many tool calls and minutes may an agent use (warn and stop), and how old may its status line be (nag and deny)?
Owner: G\progress.py:14-16, `CALL_WARN, CALL_STOP = 100, 150`, `MINUTES_WARN, MINUTES_STOP = 20, 30`, `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`. progress.py's refusal and warning texts print the constants (`"%d minutes of %d"`), so those messages are not copies.

Homes (a home states the value):
- H\CLAUDE.md:69 "Agents write a status line every 5 minutes; only time counts, not calls."
- H\docs\agents\agent-rules.md:13 rule 7: "every 5 minutes ... At 100 tool calls ... 150 tool calls is a hard stop ... At 20 minutes ..." (full text)
- H\docs\agents\brief-preamble.md:13 "every 5 minutes"; :21 "When your last line is 5 minutes old ... 10 minutes old ... refuses"; :71 "Commit before your 100th tool call"; :73 "At 100 tool calls, or at 20 minutes ... At 150 tool calls or 30 minutes it stops you"
- H\docs\agents\derive-once-review.md:9 "every 5 minutes"; :110 "(at 100 tool calls or 20 minutes)"
- H\docs\agents\doc-review.md:9 "every 5 minutes"; :100 "(at 100 tool calls or 20 minutes)"
- G\README.md:124 "At 5 minutes old it adds a reminder. At 10 minutes old it refuses the call"
- C:\Users\Patrick\.claude\agents\reviewer.md:11 "at 100 calls or 20 minutes you stop fixing, sign off what you fixed"
- C:\Users\Patrick\.claude\model-postures.md:5 "too big for one executor inside 100 tool calls and 20 minutes, it is two tasks"
- C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:100 "One status line every 5 minutes"; :101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report."
- H\tools\effort_test_scorer_brief.md:1 "Write a status line every 5 minutes"
- M\every-brief-carries-status-rule.md:15 "a status line every 5 minutes"; M\MEMORY.md:45 "a status line every 5 minutes"
- H\docs\agents\rulings\review-and-hooks.md:34 (see rulings below)
- G\tests\test_progress.py:186-292, 856-858 pins `(CALL_WARN, CALL_STOP, MINUTES_WARN, MINUTES_STOP, STATUS_NAG_MINUTES, STATUS_DENY_MINUTES) == (100, 150, 20, 30, 5, 10)` (a literal copy of the owner in a test)
- G\tests\test_progress.py:835-864 forbids the numbers 100, 150, 20, 30, 5, 10 on any gates line that mentions calls, minutes or budget. It checks gates\*.py only.
- G\tests\test_docs_numbers.py:110-127, 182-190 holds the quoted numbers in five Hydra docs to progress.py. The five are CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md (DOCS, :25-26). Its regexes are `N tool calls` and `every N minutes`; the preamble check adds `Nth tool call` and the minute numbers.
- Plan H\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:108 says the numbers sit at the top of progress.py "and nowhere else in code; the docs quote them".

What each home tells its reader to do:
| home | main session | agent | at the limit | card writer (skips preamble) | worktree |
|---|---|---|---|---|---|
| progress.py | n/a (percall runs for subagents only) | warned once at call 100 / minute 20, refused at call 150 / minute 30 (`>=`) | call 100 warns, call 150 denied | same hook runs for any subagent | state is per agent id, not per checkout |
| preamble :73 | not addressed | "start wrapping up, commit what passes and report" at 100/20; stopped at 150/30 | "At 150 tool calls or 30 minutes it stops you" | not read by card writers | commit before 100th call |
| agent-rules rule 7 / derive-once-review / doc-review | rule is for "the main session and every agent" | reviewer: stop fixing, sign off | same | not read | none |
| writer-brief :100-101 | n/a | "stop starting new cards: finish the one in hand and report" at 20 min or 100 calls; does not say commit | silent about 150/30 | this is the card writer's own copy | none |
| reviewer.md :11 | n/a | stop fixing, sign off, list unfixed findings | same | n/a | n/a |
| model-postures.md :5 | main session reads it | "a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks" (a planning size rule, not a stop rule) | n/a | n/a | n/a |

Kinds: K1 for each doc quoting the numbers and still agreeing (CLAUDE.md, README, reviewer.md, model-postures.md, writer-brief.md, the memory notes, effort_test_scorer_brief.md). K3 for writer-brief.md:101 and model-postures.md:5: the numbers are kept but the action changes (writer-brief: finish one card and report; postures: split the task). K6 does not apply, since only progress.py computes. The numbers in docs outside the tested DOCS set (README.md, reviewer.md, model-postures.md, writer-brief.md, effort_test_scorer_brief.md, memory notes) are not held by any test. The doc-number test reads only the five DOCS. Test_progress.py:835-864 excludes non-.py files. test_progress.py:856-858 is itself a copy (K1).
Splits: none found today; every home agrees at 100/150/20/30/5/10.
Edge note, exactly at the limit: progress.py warns when calls `>=` 100 and denies at `>=` 150 (test: call 149 passes, 150 denied). The preamble says "At 150 tool calls ... it stops you", and derive-once-review says "at 100 tool calls", which agree.
Confirmed rulings (quoted, never a home):
- H\docs\agents\rulings\review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time, they should plan to wrap up soon and dispatch remaining work to a new agent with fresh context. Agents have a hard cap of 30 minutes real time." (2026-10-09)
- review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (2026-09-04)
- agents-and-workflow.md Confirmed: "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05)
No ruling gives 100 or 150 calls. The calls (100 warn, 150 stop) appear in rulings only in the Unconfirmed block (review-and-hooks.md:60, "status demanded every 10 tool calls (reject after 15)" - an old value, not a copy of today's).
Tests holding homes equal: test_docs_numbers.py (docs to progress.py, five docs only), test_progress.py NumbersLiveOnce (progress.py has the only assignment). None for the README, agents\, model-postures.md, writer-brief.md, memory.
I8-247: the memory notes every-brief-carries-status-rule.md:15 and MEMORY.md:45 carry "every 5 minutes" as K1 homes (and the relay note by link). Not tested.
I9-078: test_percall_rewrite.py:78 uses `calls: 10_000` as a record past the budget, a literal where progress.CALL_STOP exists (one test home; K1-ish, the value does not need to equal anything).

## Group B. Bash length limit and shell output cap (I9-030, I9-053, I9-150, I9-186, I9-190, I9-034)
Questions: how long may a Bash command be (I9-053); how long may a shell result be and what a trim keeps (I9-150, I9-030, I9-186, I9-190); may settings raise Claude Code's output limit (I9-034).
Owners: G\bash_windows_rule.py:16 `MAX_CHARS = 1000`; G\shell_output.py:15-18 `CAP = 4000`, `HEAD_CHARS, TAIL_CHARS = 1500, 1500`, `ERROR_CHARS = CAP - HEAD_CHARS - TAIL_CHARS` (derived). The refusal text prints `%d ... MAX_CHARS` (not a copy).
Homes:
- H\docs\agents\brief-preamble.md:33 "A command over 1,000 characters"; :35 "A result over 4,000 characters arrives as its start, its error lines, its end and the path ..." (no 1,500 or 1,000 error figure; only the cap)
- G\tests\test_bash_windows_rule.py:47-57 pins 1000 pass / 1001 refused; comment "the limit is the users number"
- G\tests\test_shell_output.py:19-25,138-218 pins 4000/4001, 1500, `ERROR_CHARS == 1000`
- G\tests\test_docs_numbers.py:192-202 holds the preamble to `MAX_CHARS` and `CAP` (formatted `{:,}`)
- G\tests\shell_output_sizes.py:18-21 `CAPS = (2000, 4000, 8000)`, `TABLE_CAPS = (4000, 8000)`, `AT_FAILURE_CEILING = 9900` (a second statement of the 4000 cap, with no import of shell_output)
- G\tests\probe_results.md:104-236 "the design uses a 4,000 cap with 1,500 characters at each edge" and Claude Code's own limits (29,000/31,000 saved, failed result excerpt "about 10,000")
- H\docs\superpowers\plans\2026-11-07-shell-output-to-disk.md:13 "4,000 characters is the recommendation", :140 "Recommended 1,500 characters each", :11,:68 the 30,000 and 10,000 facts (plan text; a home like any other)
- G\tests\test_docs_numbers.py:60-61 `OUTPUT_LIMIT_KEY = "bashOutputMaxChars"` with comment "The user ruled against raising this limit: long output is trimmed instead (shell_output.py)"; G\README.md:154 "Neither the fragment nor the live settings.json may raise the shell output limit."
Table:
| home | main session | agent | exactly at the limit | card writer | worktree |
|---|---|---|---|---|---|
| bash_windows_rule.py | refused the same | refused the same | 1000 passes, 1001 refused | refused too (hook, not preamble) | n/a |
| preamble :33 | applies (read by agents; main session reads CLAUDE.md, which has no copy) | write long content to a file | "over 1,000" | does not read it; hook refuses anyway | n/a |
| shell_output.py | trims the same (PostToolUse) | same | 4000 untouched | same | n/a |
| preamble :35 | n/a | read the saved file with Read | "over 4,000" | does not read it | n/a |
Kinds: K1 (preamble to constants, held by test_docs_numbers.py); K1 for shell_output_sizes.py (4000 restated, not held by a test, the file prints sizes only); K1 for probe_results.md/plan (research record of the design numbers). K8-shaped: the test comment at test_docs_numbers.py:60 and shell_output.py-side claim that "The user ruled against raising this limit". The only user quote for it is in the plan, line 21: "so instead of saving you want to waste even more usage? no. unacceptable." (quoted by the main session in a brief). No `## Confirmed` or `## Unconfirmed` line in rulings\ speaks to it (searched rulings\*.md for shell output, trim, bashOutputMaxChars, output limit). No Confirmed ruling names 1,000 or 4,000 or 1,500; the 1,000 appears only under Unconfirmed (review-and-hooks.md:43 "The user saw a measured 7,600 option and chose 1,000"; :40 the git-commit exemption).
Splits: none on the value.
Tests: test_docs_numbers.py holds preamble to the two constants; none holds shell_output_sizes.py to shell_output.CAP (the size study could silently drift).
I9-190/I9-191 (probe_results.md): the 30,000 / 10,000 values are Claude Code's own limits (not ours); AT_FAILURE_CEILING 9900 in shell_output_sizes.py is a second statement of "about 10,000" in probe_results.md:127 and the plan, and the preamble does not state the 30,000. I9-191 (wake and nudge probes) is a record of live results: homes are probe_results.md:238-248, and G\README.md:180 "What the probes found" paragraph; the wake timeout number is in Group D.

## Group C. How long a logged refusal reason may be (I9-008)
Owner: G\common.py:46 `reason[:300]` (a literal inside the function, no name).
Homes: G\README.md:81 "`reason` (its first 300 characters)"; G\tests\test_refusals.py:50 `reason[:300]` (test recomputes the cut); G\tests\test_bash_windows_rule.py:299-311 comment "the refusal log keeps the first 300 characters", requires every refusal to carry `Instead:` before character 300; see I9-056/I9-100 in other reports. Different questions with the same number: G\progress.py:31 `QUOTE_CHARS = 300`; G\ref_judge.py:57 `err[:300]`; tools\session_cards\prepass.py:33 `TOOLS_LINE_CAP = 300` (not the same question; named so they are not mistaken).
What they tell: all say "300"; the test builds the expected value with the same slice (the pattern the rubric forbids), and the bash test makes every refusal text depend on common.py's 300 without importing it.
Kinds: K1 (README, test_bash_windows_rule comment); K6 is NOT claimed (the test recomputes the slice, it does not decide a log).
Splits: none. Tests: test_refusals.py holds the log to 300 by re-slicing, not by naming the constant.

## Group D. Stall, wake and context watch (I9-112, I9-113, I9-114, I9-118, I9-119, I9-122, I9-123, I9-126, I9-191)
Owners: G\stall.py:22-39 `STALL_SECONDS = 600`, `SCAN_EVERY = 60`, `CTX_THRESHOLD = 500000`, `CTX_BUCKET = 50000`, `CTX_TAIL_BYTES`, `EMPTY_FLEET_SECONDS = 300`, `LIMIT_SECONDS = progress.MINUTES_STOP * 60` (derived from progress.py, good).
Homes beyond the owner:
- stall.py:36 `CTX_TEXT = "Context is {:,} tokens. Auto-compact should have fired at 500K (50% of the window)..."`: the 500K is typed inside text while `CTX_THRESHOLD = 500000` is a few lines above (two statements in one file; K1, inside one owner file).
- C:\Users\Patrick\.claude\model-postures.md:15 "Keep this conversation under 500K tokens; the context hook warns you above that." (K1, K3: tells the main session to end the phase and let compaction happen)
- No setting names an auto-compact percentage (searched settings.json env, fragment, README: no `AUTOCOMPACT`). So "Auto-compact should have fired at 500K (50% of the window)" claims a Claude Code behaviour that nothing in scope sets; it is a claim in stall.py text only.
- G\settings.fragment.json:235 and live settings.json:245 `"timeout": 660`, `"asyncRewake": true` for wake.py; wake.py:11-12 comment says the timeout is `stall.STALL_SECONDS plus stall.SCAN_EVERY`, i.e. 600+60 (a derived number typed as 660 in two JSON files; K1/K6 across two settings files). Held equal by G\tests\test_wake.py:212-218 (`timeout == stall.STALL_SECONDS + stall.SCAN_EVERY`) for the fragment, and by test_docs_numbers.py `test_live_settings_carry_the_fragment` plus G\sync_settings.py for the live file.
- G\README.md:29,31 point at `STALL_SECONDS` and `SCAN_EVERY` by name (pointers, no value).
- Fixtures: `SID = "sess-1"`, `T0 = 1_800_000_000.0` in G\tests\test_stall.py:18-19, test_wake.py:16-17, test_posture.py:15-16 (K1 fixtures, three files; I9-126). test_review_dispatch.py also uses "sess-1" literals.
- G\tests\test_stall.py:72-131 asserts the texts "silent for N minutes" with N=10,20,30 as literals (the 10 matches STALL_SECONDS=600 and 30 matches progress.MINUTES_STOP; the test does not derive them).
Table: stall notices say 10/20/30 minutes silent, third is "the last stall notice"; the time-limit notice (I9-113) says "past its time limit ... first call was N minutes ago" at more than 30; a main session receives all of them (agents never do); a card writer is covered like any subagent; exactly 600 s is "not yet stale", 300 s warns the empty fleet and 299 s is quiet.
Kinds: K1 (500K twice in stall.py/model-postures.md; 660 twice), K8-shaped for "Auto-compact should have fired at 500K (50% of the window)": no home in scope sets or reads the 50%.
Splits: none on value. Confirmed rulings: none speak to 10-minute stall, 5-minute fleet or 500K. (The 2026-10-09 Confirmed line for the 20/30 minute caps is Group A.)
Tests: test_wake.py holds the wake timeout to STALL_SECONDS+SCAN_EVERY; no test holds the "500K" text to CTX_THRESHOLD (test_stall.py:410-456 pins literal 500,000 and the text).
I9-191: only probe_results.md:238-248 and the posture/T4 note in G\README.md; one home plus the wake design (no number to copy).

## Group E. How often the posture is resent (I9-079)
Owner: G\posture.py:16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2`.
Homes: G\README.md:35 names `RESEND_MINUTES` and says "The number lives at the top of posture.py" (pointer, no value); H\docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:83 "T2. The posture goes out less often" (Q8 is in that plan); G\tests\test_posture.py:67-80 uses `61 * MIN` and `30 * MIN` literals, which assume RESEND_MINUTES is between 31 and 61 (the test copies a range).
Kinds: K1 (test). K8-shaped: the posture.py comment names "Q8 of the hooks plan HX-P2" as the user's answer. Note the Q8 in the other plan (2026-10-25-hooks-from-scratch.md:204) is a different question (publish gate). No ruling line speaks to 60 minutes (searched rulings\*.md for resend/hour/posture).
Splits: none. Tests: test_posture.py uses the constant for the edge and literals for "after".

## Group F. The hook code line budget (I9-179)
Owner: G\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget for hook code; only the user changes it`; exit 1 when over (replay.py:489).
Homes: replay.py:24 docstring "The run fails (exit 1) when the hook code is over LINE_BUDGET"; H\docs\superpowers\plans\2026-10-25-hooks-from-scratch.md:21 "about 2,200 lines", :257 "fails over 3,000"; session cards H\docs\sessions\2026-10-30-83eb8bac.md:16,25 ("rase cap to 3100"), 2026-10-31-adbda2b2.md:13 (trimmed to fit "exactly at 3,100"), 2026-10-31-6777b546.md:17,22.
SPLIT 1 (value): the only `## Confirmed` line that names a number says the cap is 3,050: H\docs\agents\rulings\review-and-hooks.md:13 "chose "Raise cap to 3,050" to "The merge-undo fix adds about 35 lines; hook code is at 2,999 of your 3,000 cap ..."" (2026-10-25). The 3,100 sits only in review-and-hooks.md:44 under `## Unconfirmed` ("Hook code cap raised by the user from 3,000 to 3,050 lines on 2026-10-25 ..., then to 3,100 on 2026-10-30"; "note ...; backs: no page yet") and in the session card 83eb8bac. replay.py says 3100.
SPLIT 2 (meaning): review-and-hooks.md:11 Confirmed (2026-10-31) "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized". The card 2026-10-31-6777b546.md:17 reads this as "the cap is only information". replay.py:41 still calls it "the user's hard budget ... only the user changes it", and the run exits 1 when over. At my `wc -l` of gates\*.py, *.ps1, *.sh and lib\*.ps1 the total is about 5,572 lines (replay counts files by its own `line_total`, which I did not run); the comparison is not mine to settle.
Kinds: K2 (3,050 Confirmed vs 3100 in code), K3 (hard budget vs "only information"), K8 (comment attributes a hard budget to the user; the Confirmed words of 2026-10-31 are weaker).
Tests: none holds replay.py's number to any other home.

## Group G. May an agent file set a turn cap (I9-025)
Question: may agent type files set Claude Code's turn cap?
Owner: no code decides it; the test G\tests\test_docs_numbers.py:34-36,100-108 greps `agents\*.md` for `maxTurns`.
Homes: G\README.md:128 "No agent file in ..\agents\ sets maxTurns ..."; H\docs\agents\brief-preamble.md:73 "no agent type file sets a turn cap behind it"; test_docs_numbers.py comment "progress.py is the only limit on an agent, so no agent type file may set Claude Code's turn cap"; plan 2026-10-25-hooks-from-scratch.md (progress is the budget home).
Table: README tells the maintainer not to set it; preamble tells every agent there is none; test refuses a file that does (main session vs agent: both read it as a fact, only the maintainer edits agent files). Card writers use no agent file (they are briefed from writer-brief.md), so the test cannot see a cap set there; I found none set.
Kinds: K7 (a prose rule stated in README, preamble, test comment; the test is its check, README names the test, so there is a home); no split. Confirmed rulings: none speak to it.
Side note: C:\Users\Patrick\.claude\settings.json:4 `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH: "1"` is another turn-structure setting; the preamble's "You have no helper agents" is a prose claim over it.

## Group H. Build slots and the build pass (I10-031, I10-032, I10-033, I10-035, I10-036, I10-037, I10-046)
Owner: H\tools\build_slot.ps1:3 "This file is the one place that knows the slot rule, the pass rule and their numbers." with `$BuildSlotCount = 3` (:75), `$BuildSlotPollSeconds = 15`, `$BuildSlotMessageSeconds = 60`, `$BuildPassUnclaimedSeconds = 120`, `$BuildSlotFolderWaitMs = 5000`, `$BuildSlotYoungSeconds = 5` (:76-86), and a literal `1..10` tries with `Start-Sleep -Milliseconds 200` (:288-291, I10-046; unnamed literals).
Homes of "three":
- H\CLAUDE.md:76, H\docs\agents\agent-rules.md:20, H\docs\agents\brief-preamble.md:57, H\docs\development.md:114 "at most three builds run at once"
- build_slot.ps1:74 cites "the user's limit of three is in docs/agents/rulings/agents-and-workflow.md"; H\docs\audit\2026-10-03-fix-decisions.md:1799 D314 ("a limit of three builds at once ... This takes over from D49's limit of 3 cold builds")
- H\tools\test_build_pass.ps1:115-119,256-358 pins "three passes", "all 3 build slots are taken", `slot3.lock`
- M\one-build-per-wave.md:13 quotes the user "3 builds at once?"
Homes of "120 s": build_slot.ps1:80 (value) and :207 text printed from the variable (not a copy); test_build_pass.ps1:129 literal `'runs out if nothing claims it in 120 s'`; rulings\agents-and-workflow.md:35 "an unclaimed pass runs out after 2 minutes, 120 seconds" (this line is under `## Unconfirmed`, header at line 32, so it is a home, not a ruling); the comment at build_slot.ps1:79 "The user's answer in the same rulings page" points at that Unconfirmed line. Also H\tools\hydra_ui_mcp.py:92 `BUILD_PASS_TIMEOUT_S = 30` (a different wait: how long the helper waits for the script to answer).
Homes of poll 15 / message 60: none outside build_slot.ps1 (test_build_pass.ps1 uses Start-Sleep -Seconds 60 as a bystander process, unrelated). I10-032, I10-033, I10-036, I10-037, I10-046: one home each, searched H\tools, H\docs\agents, docs\development.md, G, memory, test_build_pass.ps1.
Table (the 'three'): CLAUDE.md/agent-rules/preamble tell main session and agents "at most three at once; agents never build; main session builds one at a time" (the preamble tells agents to hand back uncompiled); development.md tells the main session how to ask for a pass; build_slot.ps1 enforces for any caller under Claude Code (CLAUDECODE=1), none elsewhere (CI, terminal); hook percall.py refuses the pass tool to every agent. Worktree vs main checkout: slot folder is machine-wide, default `C:\Users\Patrick\.claude\hooks\state\build_slots` (see side note), so a worktree shares the three slots with the main checkout. Card writers: not told (hook refuses the pass tool to them as to any agent).
Kinds: K1 (the "three" in 4 docs + test + D314); K8 (build_slot.ps1:79 says the user answered 120 s "in the same rulings page"; that line is Unconfirmed); K7 none.
Confirmed rulings: H\docs\agents\rulings\agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." (2026-11-07). Splits: none on the value of three. Tests: test_build_pass.ps1 pins 3 and 120 (literals, not read from build_slot.ps1); no test holds the doc quotes to the constant (the docs check "docs match code" is for docs/code pointers; I did not run it).
Side note: build_slot.ps1:78 default folder path says `...\.claude\hooks\state\build_slots` while the hooks live in `...\.claude\gates` and `C:\Users\Patrick\.claude\hooks` does not exist (checked); H\tools\bench_run.ps1:9 uses the same `hooks\state` path in a comment. The path is hard-coded with the user name.

## Group I. The helper's numbers in hydra_ui_mcp.py (I10-050, I10-051, I10-052, I10-053, I10-054, I10-063)
Owner: H\tools\hydra_ui_mcp.py:41 "This is the one home of these three numbers." `REPLY_MARGIN_S = 15.0`, `HELLO_TIMEOUT_S = 60.0`, `REPLY_CAP_CHARS = 12_000`; also `LOG_TAIL_LINES = 20`, `CONNECT_RETRY_S = 0.05` (:52-53), `GIT_TIMEOUT_S = 60` (:87), `BUILD_PASS_TIMEOUT_S = 30` (:92).
Homes: H\docs\agents\ui-testing.md:280 "The helper waits that long plus REPLY_MARGIN_S for each reply, and HELLO_TIMEOUT_S for the hello line. All its numbers live at the top of tools/hydra_ui_mcp.py." and :284 "A reply longer than REPLY_CAP_CHARS is cut ..." (pointers: names only, no values); H\tools\test_hydra_ui_mcp.py:293 expects the literal text `[cut at 12,000 characters: ...` (copy of 12,000 inside a test string, while :289 uses `hm.REPLY_CAP_CHARS`); the "timeout in force" (I10-053) is owned by H\tests\ui\uitest_commands.h:17-19 `double timeout = 30.0` ("The cap on wait-idle and wait-text"), which hydra_ui_mcp.py:46-47 names instead of copying; ui-testing.md:265 describes the hello line `timeout`. test_hydra_ui_mcp.py uses `reply_margin_s=1` overrides.
What they tell: ui-testing.md points an agent to the file; the helper enforces; main session vs agent same; card writers have no hydra tools; worktree: REPO = the checkout of the file. Kinds: K1 only for test_hydra_ui_mcp.py:293 ("12,000" in an expected string); I10-053 is a correct pointer (owner named, no value). I10-050, 051, 054, 063: one home plus pointers; I searched ui-testing.md, clone-hero-mcp.md, this-machine.md, tests\ui\*.h, test_hydra_ui_mcp.py. Splits: none. Confirmed rulings: none. Test holding equal: test_hydra_ui_mcp.py:289 uses the constant for the bound.

## Group J. Third-party skill quoting a tool's defaults (I8-225, I8-226, I8-230, I8-233)
Questions: the defaults of the REA tool (heap, timeout, runtime reserve, page sizes, capture limits).
Owner: not in scope; the skill states "When the connected schema advertises `max_heap_mb` and `analysis_timeout_ms`" and links github.com/morluto/rea docs. The values belong to the REA MCP server (installed plugin; not readable here).
Homes: C:\Users\Patrick\.claude\skills\reverse-engineer-anything\references\javascript-applications.md:36-40 "Defaults are 1024 MiB and 300000 ms", "512 MiB runtime reserve"; SKILL.md:133-134 "Output defaults to 50 matching frames", :149-150 "50 literal lines", :186-187 "Defaults are selected-table schema and 20 rows"; references\native-and-artifacts.md:33 "samples 20 rows" (second statement of the 20 rows); references\evidence-workflows.md:51-56 "V3 captures cannot be upgraded to V4", "SIGTERM ... SIGKILL once" (I8-233 cites runtime-observation.md:49-57 as well; in my check the V3/SIGTERM text is in evidence-workflows.md only, runtime-observation.md has a different passage). The skill's own version marker is `version: "42"` (SKILL.md:5). No other copy under skills\, plugins\ (searched "1024 MiB", "finalization_ms", "50 matching frames").
Kinds: K1 (the 20 rows twice inside the skill), K8 none (it names its owner as the schema). It is a copy of a tool's own rule held in prose, so it can go stale when REA updates; nothing checks it. No test, no Confirmed ruling. Splits: none.

## Group K. Handoff checker, its date, and the session-start hook (I8-242, I10-022, I10-015)
I8-242 (how many times the checker runs, and what to do with a row still `missing`): homes: H\.claude\commands\handoff.md:12 "Then run the checker once more. If a row is still `missing` after that second run, leave it, and say which row and why in your report."; H\docs\agents\handoff.md:65 "fixes every `missing` row it finds, and pastes what it printed under `## Checker output`"; brief-preamble.md:97 and handoff.md:67 for the continuing session ("reports each MISSING or CHANGED SINCE line").
SPLIT 3: the /handoff command tells the writer to leave a row that is still `missing` after the second run and say why; handoff.md:65 says the command "fixes every `missing` row it finds". Same command, two actions, no number of runs in handoff.md. Different owner for "twice": none in tools\check_handoff.py. Kind K3, K7 (no home for the two-run rule). Tests: none (searched tools\test_check_handoff.py by name for "second").
I10-022 (how the handoff date is found): owner H\tools\check_handoff.py:398-413 `handoff_date`: file name date, else the last git commit date for that file, else today with a warning. Homes: H\docs\agents\handoff.md:7 "The date in the name is the day it was written. The checker reads that date" and :59 "The date defaults to the date in the handoff's file name; `--since` overrides it" (omit the git and today fallbacks); .claude\commands\handoff.md:11 "with today's date" in the file name. Kind: K1/K3 (docs state only the first step; the second and third steps live only in the code). No split.
I10-015 (how the session-start pointer check is run): H\tools\check_pointers.py is registered at G\settings.fragment.json:104 and live settings.json:114, both `python.exe -I -S ...check_pointers.py --session-start` with `"timeout": 15`; G\README.md:71 names it. Held equal by test_docs_numbers.py `test_live_settings_carry_the_fragment` and sync_settings.py. check_pointers.py has no timeout of its own (searched). Kind: K1 (fragment + live), one test.

## Group L. Memory notes quoting engine facts (I8-248, I8-251, I8-254)
I8-248 (SP squeeze search window, 500 ms): owner H\src\core\model.h:121 `constexpr double kSqueezeWindowMs = 500.0` (strict edge, :130-134). Homes: M\sp-squeeze-mechanics.md:13 "strictly within kSqueezeWindowMs = 500 ms"; skills\analyzing-sp-squeezes\SKILL.md:10 "within 500 ms of a deactivation node (kSqueezeWindowMs, src/search/graph.h)" (names graph.h; the constant is in core\model.h: K8-ish, a wrong file named; graph.h:216 and graph.cpp:15 do point at it); H\docs\clone-hero\05-star-power.md:413-417 SP-38 "Hydra looks for squeezes only within 500 ms of the end" (quotes `kSqueezeWindowMs = 500.0` at "line 111", the constant is now at line 121 of model.h: stale line number). Confirmed ruling: H\docs\agents\rulings\clone-hero.md:12 (Confirmed block starts line 7, Unconfirmed at 67) "ok well i am ruling it now: hydra searching for squeezes up to 500 ms is an intentional deviation from the game, not a bug." (2026-11-12). Kinds: K1, K8 (graph.h named for model.h's constant). No test holds the memory/skill to the constant.
I8-251 (the 3 ms backend leeway): owner H\src\core\rules.h:32 `double backend_leeway_ms = 3.0;` and `core::counted_without_squeeze` (backend_value.h:39; strict edge D29). Homes: M\sp-end-is-one-point-leeway-is-delgars.md:13 "The leeway (Rules::backend_leeway_ms, 3 ms ...)"; M\MEMORY.md:7; H\docs\clone-hero\05-star-power.md:453-461 SP-42; model.h:98 comment "a user rule". K1, no split. No test pins the doc's "3 ms" to rules.h (not searched in tests\*.cpp beyond a name grep).
I8-254 (how many charts the library holds): no owner in scope; the real count is the user's hydra.db and the window text (`hydra_text contains="of 19"`). Homes disagree by date: M\no-full-library-runs.md:17 "18,811 charts", M\min-path-delta-proof.md:29 "18,610", M\stored-versions-one-rule.md:11 "18,755", M\measure-small-explain-first.md:11 "19,000-chart", M\search-before-clicking-a-song.md:17 `"of 19"` (a prefix, so it is a check string for the real count); H\docs\agents\rulings\code-and-engine.md:27 (Confirmed block) question text "about 18,848 charts"; H\docs\agents\rulings\ui-and-display.md:20 (Confirmed block) "your 19,610 charts". K2 (several figures for one live quantity, each a dated example; none states itself as a rule). Rulings found: the two lines above are the question text recorded beside the user's choice, not the user's words.

## Group M. Single-home test fixtures (I9-016, I9-020)
I9-016: G\tests\test_common.py:30 `RACERS, HOLD_SECONDS = 16, 0.02`: one home; used only in that file (searched G\tests, G\*.py).
I9-020: G\tests\test_common.py:213-224 sets `_LOCK_WAIT = 0.05` on common; the owner is G\common.py:18 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001` and the README names no value. The text "without the lock" and the agent id the test expects are produced by common.py:206 (not a copy). One home plus a test override; no split.

## Splits (one line each)
1. replay.py:41 `LINE_BUDGET = 3100` vs the only Confirmed line on the cap, review-and-hooks.md:13 "Raise cap to 3,050"; 3,100 is only Unconfirmed (review-and-hooks.md:44) and a session card.
2. replay.py:41 and :24 treat the cap as a hard budget that fails the run; Confirmed review-and-hooks.md:11 (2026-10-31) "im not too concerned about the exact number as much as efficiency and functionality ..." and the card 2026-10-31-6777b546.md:17 reads that as "the cap is only information".
3. .claude\commands\handoff.md:12 (leave a row that is still `missing` after the second run) vs H\docs\agents\handoff.md:65 (the command "fixes every `missing` row it finds").
4. build_slot.ps1:79 says "The user's answer in the same rulings page" for 120 s; the page holds that answer only under `## Unconfirmed` (agents-and-workflow.md:35). (An owner claim that points at text no ruling backs; not a value split.)
5. stall.py CTX_TEXT says auto-compact "should have fired at 500K (50% of the window)" while nothing in scope sets an auto-compact percentage (settings.json env, fragment, README searched). (A claim without an owner; not a value split.)
6. M notes give 5 different chart counts for the library (Group L).

## Side notes
- C:\Users\Patrick\.claude\gates\tests\test_progress.py:856-858 pins the six budget numbers as a literal tuple: a second home inside the test that says the numbers live once.
- G\tests\shell_output_sizes.py:18-21 states `CAPS = (2000, 4000, 8000)`, `AT_FAILURE_CEILING = 9900` with no import from shell_output.py.
- H\docs\clone-hero\05-star-power.md:417 names `kSqueezeWindowMs` "at line 111", the constant is at src\core\model.h:121.
- skills\analyzing-sp-squeezes\SKILL.md:10 names src/search/graph.h as the home of kSqueezeWindowMs; the definition is in src\core\model.h:121.
- build_slot.ps1:78 default folder `...\.claude\hooks\state\build_slots`; the `hooks` folder does not exist on this machine, the hooks are in `gates`.
- A way past a hook: G\tests\test_progress.py:835-864 only scans `*.py` in the gates folder, so a budget number stated in a `.ps1`, `.sh` or `.json` file is not seen.
- settings.json:4 sets `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH` "1" (a subagent-depth setting not mentioned in README.md:128's turn-cap paragraph).
- No test or ruling was run or changed; no file in either repo was edited.

## Rows not reached
None. Rows with a weaker search (single-home claims, searched by number and by question in the scope but not read line by line in the test files): I9-016, I9-020, I9-078, I9-126, I9-186, I9-191.
