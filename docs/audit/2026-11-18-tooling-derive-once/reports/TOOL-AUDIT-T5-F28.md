# TOOL-AUDIT-T5-F28 (finder F28, agent a8dc050d2a4fac6bd, session f6a384b5)

Read-only. All 53 rows read in their T3 reports and grouped by the question they answer, so one search covers several rows. Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, M = the memory folder of the Hydra project, GC = ~/.claude/CLAUDE.md, MP = ~/.claude/model-postures.md, PC = H\CLAUDE.md, AR = H\docs\agents\agent-rules.md, BP = H\docs\agents\brief-preamble.md.

Rows not reached: none. Rows that got only a light look (one home, searched by phrase, no deeper): I2-055, I2-057, I2-065, I2-066, I2-072, I2-074, I2-078, I2-101, I2-116 (grouped as Q4 and Q5), I5-007, I5-008, I5-045, I5-046, I5-060.

Rulings read first: hooks-from-scratch plan line 108 ("Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of progress.py and nowhere else in code; the docs quote them, and a test ... greps"). The `## Confirmed` lines of review-and-hooks.md and agents-and-workflow.md were read; the ones that bind these rows are quoted under each question.

---

## Q1. Who may take a build pass, and what do agents say about building (I1-003, I1-013, I5-031 in part, I5-050, I5-053 in part)

Question in game words: "Which tool hands out a build pass, who is refused it, and what is an agent told instead?"

Homes (value = the tool name, the limit three, or the "agents never build" rule):
- G\percall.py:19-20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`, comment "Hydra's tools/build_slot.ps1 owns what a pass allows." (pointer plus the tool name).
- G\percall.py:109-111 refusal text: "Agents never build, so hydra_build_pass is refused to every agent. Instead: commit your work and hand back; the main session builds your branch and runs any test that needs a build. Say in your report what to build and which tests to run."
- H\tools\hydra_ui_mcp.py:874 registers the tool as `hydra_build_pass` (the server name `hydra` is set elsewhere). Its docstring at :22-24 says "The rule is tools/build_slot.ps1's".
- H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`; :87 `$BuildPassHowTo = 'Agents never build: hand back, and the main session builds. A main session gets a pass from hydra_build_pass.'` (a third wording of the refusal); :195 "One build at a time per session." This is the code that enforces the limit.
- G\README.md:15 explains the refusal, quotes the 2026-11-07 ruling and says build_slot.ps1 "owns the limits".
- PC:76 (rule 14 short form), AR:20 (full text, states "at most three", "one build at a time", names hydra_build_pass), AR:27 (hook list), BP:57 ("At most three builds run at once ... through a pass from hydra_build_pass. A hook refuses that tool to every agent."), H\docs\development.md:114-120 ("at most three builds run at once", the ticket-booth text), MP:9 and MP:11 ("hydra_build_pass, one build at a time"), ~/.claude/agents/executor.md:11 and reviewer.md:21 ("You never build"), the claimed-in-comment at H\tools\new_worktree.ps1:8 and H\tools\compare_db.py:41-46.
- M\one-build-per-wave.md (a note, a home): "batch C++ branches into one combined build per wave".

What each home tells its reader:

| Home | Who is told | What they do | Limit three stated? |
|---|---|---|---|
| percall.py refusal | any agent that calls the tool | commit, hand back, say what to build | no |
| build_slot.ps1 text and code | anyone without a pass | read "agents never build", ask the tool | yes, in code (3) |
| PC:76, AR:20, BP:57, development.md:114 | all agents and the main session | never build (agents); main session builds one at a time | yes, four places state "three" |
| MP:9, MP:11 | the main session | own every build and build-needing test | no number |
| executor.md:11, reviewer.md:21 | those agent types | never build, never run hydra_tests/uitest/replay | no |
| card-writer (skips the preamble) | writer-brief.md and card-writer.md say "commit nothing and run no tests" | nothing about building | n/a |
| main session against an agent | the hook refuses by `agent_id` only; main session passes | | |
| worktree against main checkout | build_slot.ps1 takes `-Repo <worktree>`; the pass is per session, not per checkout | | |
| exactly at the limit (fourth pass) | build_slot.ps1:197-199 refuses with "all N build slots are taken"; test_build_pass.ps1:119 checks it | | |

Kinds: K1 (the number three stated in PC:76, AR:20, BP:57, development.md:114, ruling text; the value is only coded in build_slot.ps1:75); K1 (tool name in percall.py:20, hydra_ui_mcp.py:874, AR:20, G\README.md:15); K1/K3 (three wordings of the refusal text: percall.py:109, build_slot.ps1:87, BP:57).

Splits: none found in the value. Side line: percall.py:19 claims build_slot.ps1 owns what a pass allows, and build_slot.ps1:73-87 does hold the count and the per-session rule, so the owner claim is true (I checked it against the script).

Rulings: H\docs\agents\rulings\agents-and-workflow.md:8 (Confirmed) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." (session e3e9069c).

Tests holding homes equal: H\tools\test_build_pass.ps1 holds the count (three locks, a fourth refused) to build_slot.ps1 only. G\tests\test_percall_key.py:197-217 uses `percall.BUILD_PASS_TOOL`, so it does not check the string against the hydra server. H\tools\test_hydra_ui_mcp.py:55 lists `hydra_build_pass` as a registered tool. No test links the string in percall.py to the tool in hydra_ui_mcp.py, and none links the "three" in the docs to `$BuildSlotCount`. G\tests\test_docs_numbers.py holds only call counts and minute numbers.

Side note: build_slot.ps1:78 default folder is `C:\Users\Patrick\.claude\hooks\state\build_slots`; the hooks live under `gates\`, and `~\.claude\hooks\` now holds only `state`. Not checked whether the path is live on purpose.

---

## Q2. What the main session is told to do about a quiet, stuck or past-limit agent, and a status line (I2-009, I2-011, I2-016, I2-020, I2-022, I2-024, I2-038, I2-056, I5-054)

Question in game words: "When an agent goes quiet, runs long or writes a status line, what does the main session read and do?"

Homes:
- G\stall.py:30-33 `STATUS_HEADER`: check each line against the EVIDENCE line, "Relay the claim to the user briefly, in plain English. When the evidence disagrees, add one sentence that says so, and open the agent's transcript."
- MP:11: "Check each line against that evidence. Relay the claim to the user briefly, in plain English, and add a second sentence only when the evidence disagrees. ... When an agent is stuck, looping, silent or off its brief, act at once: look at its transcript tail and worktree log, then nudge it once with SendMessage ... If it does not recover, stop it with TaskStop and re-dispatch its approved task to a fresh executor on the same branch ... Never re-dispatch a brief unchanged."
- G\stall.py:116-125 (stall summary) and :137-144 (past-limit): "Check each one now. If an agent is stuck, stop it with TaskStop and re-dispatch its remaining work." (the same sentence twice in one file).
- G\stall.py:458-461 `CHECK_IN_HEADER`: "If an agent has gone quiet or looks off track, open its transcript, nudge it or stop it."
- G\stall.py:42-45 empty-fleet: "If the run is dead, re-launch its open tasks. If it has in fact finished, ignore this."
- G\stall.py:36-37 context text and :27 `CTX_THRESHOLD = 500000` (Q3).
- G\agent_stop.py:260-261: "marked finished and no longer watched for stalls".
- AR:27: "The main session also gets a stall notice when an agent has made no call for 10 minutes ... repeats at 20 and 30 minutes ... the third says it is the last. The step is STALL_SECONDS in stall.py, and the cap is ... MINUTES_STOP in progress.py. An agent that has not handed back 30 minutes after its first call gets one more notice."
- G\README.md:29 and :31 describe the same notices and name STALL_SECONDS and MINUTES_STOP without numbers (pointer). G\README.md:41 describes the parked-agent case (agent_stop).
- BP:23 and :73 tell agents about the evidence line and the 20/30 minute numbers.
- M\relay-status-in-plain-english.md (a note, a home): "check the line against the EVIDENCE line ... write one sentence naming the task in plain words".

What each home tells its reader:

| Home | Reader | Action |
|---|---|---|
| STATUS_HEADER | main session | check, relay briefly, add one sentence if evidence disagrees, open transcript |
| MP:11 | main session | same, then nudge once with SendMessage, then TaskStop and re-dispatch to a fresh executor; no approval needed |
| stall summary and past-limit notice (stall.py:124, :144) | main session | "Check each one now", then TaskStop and re-dispatch. No nudge step |
| CHECK_IN_HEADER | main session | open transcript, nudge, or stop |
| AR:27 | readers of the rules | states 10/20/30 minute numbers |
| agent (hook-refused or silent agent) | a stopped agent never reads these | not applicable; an agent only gets the progress.py warnings |
| card writers | skip the preamble; the notices still reach the main session about them | |
| exactly at the limit | stall.py:131 uses `now - first <= LIMIT_SECONDS` skip, so exactly 30 minutes is not yet past limit; AR:27 says "30 minutes after its first call gets one more notice" | one-minute edge between wording and code (not tested by me) |
| worktree against main checkout | not applicable (state is per session) | |

Kinds: K1 (the 10 / 20 / 30 numbers in AR:27, BP:73, test_stall.py comments :78-79 against stall.py:20-22 and progress.py:15); K3 (see split 2a); K5 (MP:11 spells out what STATUS_HEADER already tells the main session; the hook text is the one the main session gets every time).

Splits:
- 2a. Stuck-agent action: MP:11 says nudge once with SendMessage first and only then TaskStop; stall.py:124 and :144 tell the main session to stop with TaskStop and re-dispatch right away, with no nudge; stall.py:460 (check-in) says "nudge it or stop it".
- 2b. Relay length: STATUS_HEADER "add one sentence that says so" (only when evidence disagrees) and MP:11 "add a second sentence only when the evidence disagrees" agree. No split; recorded because it is the same sentence in two homes.
- 2c. MP:11 says re-dispatching "needs no approval"; AR rule 3 (AR:9) says "No work starts without the user's approval ... launching agents to do either" is a build. The posture's "no approval" for a re-dispatch of an approved task is not stated in AR:9 (AR:9 only exempts "compiling and test runs inside approved work"). Not checked whether a ruling covers it.

Rulings (Confirmed): H\docs\agents\rulings\agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable"; :26 "the status updates should be brief but relayed in plain english"; :18 "agents should fail loudly. freezing is unacceptable".

Tests: G\tests\test_stall.py holds the stall numbers to the constants (`test_stall.py:148` "silent for 10 minutes" is built from the code, not from AR). No test holds AR:27's numbers equal to stall.py; test_docs_numbers.py (:110-122, :182) covers only call counts and "every N minutes" status interval. No test holds STATUS_HEADER equal to MP:11.

---

## Q3. When does the main session get a context warning (I2-011)

Homes: G\stall.py:27 `CTX_THRESHOLD = 500000`; G\stall.py:36 text "Auto-compact should have fired at 500K (50% of the window)"; MP:15 "Keep this conversation under 500K tokens; the context hook warns you above that." Kind K1 (the figure is typed in three places; the text carries no format from the constant). "(50% of the window)" is a fact about Claude Code's auto-compact setting, which nothing in the scope reads (live settings.json holds no window or compact threshold; searched `compact` and `window`, found only the hook matcher at :119). K4 candidate: the text tells the main session to ask the user to run /compact when auto-compact "should have fired", a fact the hook cannot read.
Test: test_stall.py:421-434 pins "Auto-compact should have fired at 500K" and 500000 against the same file, so it holds the text and constant equal only to each other.
Ruling: none found.

---

## Q4. The wake watch: wake rule, crash rule, who decides, how it is registered (I2-057, I2-059, I2-065, I2-066)

Homes: G\wake.py:3-6 (asyncRewake, exit 2; points at tests\probe_results.md and the plan docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`) "task W1"), :9-10 and :20 (decides nothing; stall.py owns the words), :66-80 (`_crash`), :104-105 (crash text). G\README.md:31 states the same: Stop hook, exit 2 wakes, "Every word it hands over is stall.py's", crash wakes once with the error. G\settings.fragment.json:235-236 `"timeout": 660, "asyncRewake": true`; live settings.json:245 `"timeout": 660`.
Test holding two homes equal: G\tests\test_wake.py:212-218 `test_registered_timeout_is_stall_seconds_plus_one_scan` pins the fragment's 660 to `stall.STALL_SECONDS + stall.SCAN_EVERY`; wake.py:8-9 names that test. test_docs_numbers.py:142 `test_live_settings_carry_the_fragment` holds the live settings equal to the fragment.
Kinds: K1 (the 660 in two files, pinned by tests); K8 side note: wake.py:6 points at docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`), which is not in H\docs\superpowers\plans (listing grep for "supervisor" found nothing). The path is a pointer to a file that does not exist there now.
Splits: none. Rulings: none.

---

## Q5. What a hook does when it crashes (I2-055, I2-074, I2-101, I2-116, I2-078)

Question: "When a hook breaks, does it block, warn, or stay silent, and what does it exit with?"

Homes:
- G\common.py:52 `run(body, mode)`; each hook picks its word: agent_stop.py:297, posture.py:55, prompt.py:67 and :30, shell_output.py:201, waiver.py:50 are "loud"; delete_rule.py:187, percall.py:176, publish_gate.py:135, review_dispatch.py:178 are "closed".
- G\README.md:90-100 states the rule ("closed" is for hooks that can refuse; "loud" for hooks that only inform; "the script exits 0"), describing common.run.
- G\cleanup_sweep.ps1:31 and :699-700, G\session_cards_start.ps1:24-25 and :300: their own header text "fail loudly, never freeze", "Exit 0 always", "D46: this hook cannot refuse ..." (PowerShell code, not common.run). The D46 entry is H\docs\audit\2026-10-03-fix-decisions.md:109-110 ("D46 amendment, 2026-10-04: fail loudly, never freeze ... Hooks that cannot refuse ... show a visible warning (systemMessage)").
- G\prompt.py:49 "this prompt went through, but the part answered %s: %s"; G\README.md:33 repeats that a part's deny or ask is said in the systemMessage.
- AR:10 and PC:66 rule 4: "they fail loudly: stop and say what broke" (about agents, not hooks).
- G\posture.py:44-45 missing-posture text; README.md:35 "A missing or empty posture file is said out loud every time."

What each says:

| Home | Reader | Action on a crash |
|---|---|---|
| common.run "closed" | the hook | deny with the error text |
| common.run "loud" | the hook | print a systemMessage saying the hook did nothing, exit 0 |
| the two .ps1 hooks | themselves | their own try/catch, systemMessage, exit 0 (a second code place answering the same question) |
| README:97-100 | readers | same words as the code |
| AR:10 / PC:66 | agents | fail loudly (a different thing: the agent's own behavior) |
| main session vs agent | `percall.py` runs "closed" for both; posture.py "loud" only reaches the main session | |
| card writers | not applicable | |

Kinds: K6 (the crash rule is done by `common.run` in Python and again by hand in cleanup_sweep.ps1 and session_cards_start.ps1); K1 (the phrase "fail loudly, never freeze" in D46, two ps1 headers and two ps1 bodies); K3 (AR:10 reuses "fail loudly" for agent behavior while D46 uses it for hook behavior; the user's one words cover both).
Rulings: H\docs\agents\rulings\agents-and-workflow.md:18 (Confirmed) "agents should fail loudly. freezing is unacceptable" (session a9a461da).
Tests: G\tests\test_common.py (not read in detail) covers common.run; the ps1 tests (test_h52, test_h53) cover the ps1 side separately. No test holds the Python and PowerShell crash behavior equal.
Splits: none in value. I2-072 (posture file missing): single home plus README:35; test_posture.py:220 pins the text start. I2-078: single home plus README:33.

---

## Q6. Housekeeping scripts: test params and proof test (I2-102, I2-118)

Homes: G\cleanup_sweep.ps1:33-46 (param list and "Proof: tests\test_h52_cleanup_sweep.ps1"), G\session_cards_start.ps1:26-32 ("-ToolsDir ... -DataDir ... -CardsDir ... -StateRoot", "Proof: tests\test_h53_session_cards_start.ps1"), G\README.md:152 ("The housekeeping scripts have PowerShell tests (test_h52_cleanup_sweep.ps1, test_h53_session_cards_start.ps1, test_status_append.ps1)"). Kind K1 (file names in three places, which agree; both files exist). The param lists in the header comments (which say "-ScratchRoot, -StateRoot, -ClaudeRepo and -ClaudeHome point it at other folders ... -Foreground") restate the `param(...)` block in the same file (K1 inside one file). session_cards_start.ps1:26 says "The test uses all four"; I did not count which parameters the test passes. No test holds the header text equal to the param block. Rulings: none.

---

## Q7. What the main session does for unwritten session cards (I2-113, I2-115, I2-116 in part, I5-018)

Question: "At the start of a session, what does the main session do for past sessions with no card, and for a removal reminder?"

Homes:
- G\session_cards_start.ps1:269-291 builds the instruction text (launch one card-writer per batch, read writer-brief.md, never run check.py, then check.py, index.py, link_memory.py, the docs check, commit on main with trailers and `-- docs/sessions` last, "A commit that changes only docs/sessions needs no reviewer", "Do it alongside"). :265-266 comment: "The commit step relies on gates\review_rules.py explain_main_move: a commit on main that changes only docs/sessions needs no reviewer, but every commit needs the Task, Agent and Session trailers."
- PC:29 "A start-of-session hook asks the main session to write cards for finished sessions that have none. Do it alongside the user's request, as the hook's message says." and "A card marked `[write-back]` in the index names a Clone Hero fact its page lacks, and the next main session adds that fact to the page alongside the user's request".
- C:\Users\Patrick\.claude\agents\card-writer.md:9 (read writer-brief.md first, skip the preamble, write only card files, read the CH page list for `write-back:`).
- G\...\tools\session_cards\writer-brief.md (the writer's rules; lines 64, 78-94 touched).
- M\git-traps-in-shared-checkout.md: "Stage each file by name. Never git add -A ... trailers last".
- G\README.md:67 (one line naming the scripts); README:59 for ruling_rules reading cards.
- MP:5 "Card writers run on Haiku" and "Every dispatch names its agent type"; AR/PC rule 2 repeat the model.

What each says: the ps1 text tells the main session what to do step by step (K5 candidate: PC:29 states "do it alongside" which the hook text also states, and "[write-back]" handling has no hook text at all). Write-back has no home in code: session_cards_start.ps1 never mentions it (grep for "write-back" in the ps1: none). K7 (the write-back duty is prose in PC:29 only; check.py:39-49 and index.py:9,:51 produce and mark it but nothing tells the next main session).
Owner claim check (I2-113 owner claim, the K8 question): the ps1 comment says review_rules.explain_main_move decides "docs/sessions needs no reviewer". explain_main_move (G\review_rules.py:398) decides by whether the repo carries a marker and whether a changed path is code (`_is_code`, :180). I did not trace whether `docs/sessions/*` passes as not code in the H repo. Also, H\.derive-once-gate and H\.doc-review-gate both say the deciding rules are in `hooks\lib\derive_once_rules.ps1` / `hooks\lib\doc_review_rules.ps1`; `~\.claude\hooks\lib` does not exist (only `hooks\state`). That is a K8 wrong owner claim in two Hydra marker files, next to this row.
Edges: card-writer agents skip the preamble: card-writer.md:9 and writer-brief.md:78/94 carry their own copy of the plain-English rule and their own status rule. Main session against an agent: the commit step is for the main session only, but the hook text goes to the session that starts, which could be a main session in a worktree; the text says "in the main checkout" so a worktree session is told to commit in the main checkout. Exactly at the limit: batch.py "sizes a wave to the at-once limit" (not read).
I2-115 reminder text: single home (:350), plus the same "It never deletes anything; the user does" at :17 in the same file. I2-116: single home, see Q5 for the failure mode.
Rulings: none found for card commits.
Tests: G\tests\test_h53_session_cards_start.ps1 (not read) tests the ps1; tools\session_cards\test_session_cards.py covers the python tools. No test holds PC:29 equal to the ps1 text.

---

## Q8. Who may be named as the catcher of a stuck agent (I4-031, I4-068, I4-122)

Homes: tools\session_cards\writer-brief.md:80 and :91 (the three phrases and "Use `hook notice` when a hook's stall or limit notice raised it"), tools\session_cards\check.py:33-37 (`CATCHERS = ("main session", "hook notice", "user")`, comment cites the user's answer "Yes, record it" to question 3 of the supervisor plan, 2026-11-16), tools\session_cards\test_session_cards.py:477-498 (builds its messages from `check.CATCHERS`).
The user's answer exists: H\docs\sessions\2026-11-16-60c3a576.md:30 and 2026-11-16-e816e44f.md:25 ("chose "Yes, record it" to "Measurement: should session cards end each Roadblocks line ..."") . A card quote is not a filed ruling; nothing under `## Confirmed` says it (grep for "Yes, record it" in rulings: none). The supervisor plan named in check.py is no longer in H\docs\superpowers\plans.
What each does: writer-brief tells writers the three exact strings; check.py rejects any other; the test passes if each one passes and "the reviewer" fails.
Kinds: K1 (three strings typed in writer-brief.md:80 and :91 and in check.py:37; the test reads the constant). Edge: card writers skip the preamble and read only writer-brief.md; the writer-brief.md copy is therefore the one they see. "hook notice" is also tied to what stall.py sends (Q2): the writer is asked to judge from the condensed text which notice raised it.
Splits: none. Tests holding homes equal: none for writer-brief against CATCHERS (test_session_cards.py:477 uses the constant only; no check that writer-brief.md lists the same three).

---

## Q9. How to write: plain English (I5-001, I5-002, I5-003, I5-004)

Homes (value = the rule text):
- GC:3-9 and :11-29 (the rule and the BAD and GOOD example); the pre-show check "stop and check if it looks like the BAD example below" at :11 is only here.
- BP:121-123 "Plain English": "Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls of file:line references ... That file is the one home of those rules." (BP:123 is the claim that GC is the home, quoted from I5-097).
- BP:13 status line "in plain words".
- MP:13 "Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references." (main session; omits the gloss rule and the pre-show check).
- G\stall.py:32 STATUS_HEADER "in plain English".
- tools\session_cards\writer-brief.md:94 "Write in plain English, short sentences, no jargon without a gloss. Name files by name where it helps, but keep sentences around them." (a fourth wording, for card writers).
- agents\executor.md, planner.md, scout.md: "in plain English" (pointer-level; no rule text). reviewer.md: no mention found.
- H\docs\agents\doc-review.md:81 "Hold the document to 'How to explain things' in C:\Users\Patrick\.claude\CLAUDE.md. That file is the one home of the style rules" (pointer, correct).
- M\relay-status-in-plain-english.md (a note), MEMORY.md:46 index line.
- H\docs\agents\public-doc-style.md (a different, player-facing style).

Table:

| Home | Reader | Rule it gives |
|---|---|---|
| GC | every session | full rule with example and pre-show check |
| BP:121-123 | every agent | 4 of the 7 points, names GC as home |
| MP:13 | main session | 3 points |
| writer-brief.md:94 | card writers (skip BP) | 3 points |
| STATUS_HEADER | main session | "plain English" when relaying only |
| subagent files | agents | "in plain English" only |
| main against agent | main session gets GC through Claude Code context and MP; agents get BP | agents do not see GC unless their brief points there |
| worktree vs main checkout | GC is user-global, same in both | |
| exactly at limit | "40-word run-ons" is the only number (GC:6); no one checks it | |

Kinds: K1/K7 (the same rule in GC, BP, MP, writer-brief, no hook checks it; BP:123 names GC "the one home" while MP:13 and writer-brief:94 restate it, so BP's own claim has two copies beside it, K8-adjacent). K3: MP:13 and BP drop the gloss and pre-show-check parts, so a reader of only one of them gets a shorter rule.
Rulings (Confirmed): H\docs\agents\rulings\agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english". The wider plain-language lines sit under `## Unconfirmed` in docs-and-writing.md (e.g. :20 "how do I get you to listen"), which are not the user's words by the rulings README.
Tests: none pair these texts. Splits: none.

---

## Q10. When to ask the user a question (I5-005, I5-022 in part, I5-052)

Homes: PC:1 "Ask questions if something is unclear."; M\ask-only-game-or-permission-questions.md ("questions to the user are about what the game or Hydra does, or a permission; never wording, proof standards or which tool to use"); AR:23 "A plan never offers its own reading of a gap ... It never puts such a reading to the user as a question either."; AR:18/AR:12 (a visible change not asked for is a question); M\questions-describe-the-real-rule.md; AR:25 (a row with no owner goes to the user).
Split 10a (K3): PC:1 invites questions about anything unclear; the memory note and AR:23 limit questions to game or permission and ban invented-rule questions. They answer one question ("what may be asked") with different limits. The memory note is a session's note; it cites a user's reply "why are you presenting this to me? this isnt a fact about the game" (2026-11-08), which is not under `## Confirmed` (grep "presenting this" in rulings: none).
Ruling lines: H\docs\agents\rulings\questions-and-audits.md:9 (Confirmed) "i dont understand why you always try to invent insane edge cases, this one isnt even possible in clone hero. you cant just activate arbitrarily whenever you want"; :15 "what are we already doing and why are you asking me to re derive it". The AR:23 quotes "the correct rule is to use what hydra already knows. stop trying to invent new rules" and "anything that invents its own rule instead of reading what the engine already knows is a violation" (user words as AR gives them) were not found in any Confirmed line (grep "correct rule" and "invent new rules" in rulings/*.md: none). So that quoted text in AR:23 is not backed by a Confirmed line I could find. The same for AR:21's 2026-11-13 quote.
Test: none.

---

## Q11. Clone Hero: how to drive it and what to read first (I5-007, I5-008) and who defines a project word (I5-022, I5-058)

Clone Hero homes: PC:5 and :9; BP:105 ("When you learn a Clone Hero fact ... it belongs on its page under docs/clone-hero/, never in a memory note ..."); agents\card-writer.md:9 (read the CH page list); M\ch-runs-must-not-need-focus.md:17; H\docs\agents\premise-map.md:42,:78. Single statement of "read docs/clone-hero/README.md before any Clone Hero question" is PC:9; the others are about learning facts, not reading. No split. Searched: agents, MP, GC, M, gates, H\docs\agents, .claude\commands. Row has one home plus pointers.

CONTEXT.md homes: PC:41, AR:41-43, BP:99, H\docs\agents\derive-once-review.md:49-51 (step 8), M (none). Text of each:
- AR:41 "CONTEXT.md decides what a project word means." AR:43 "The code's usage settles the meaning, not a guess drawn from one sentence of a doc."
- derive-once-review.md:51 repeats both sentences word for word. BP:99 repeats the second.
Split 11a (K3): "CONTEXT.md decides what a project word means" against "The code's usage settles the meaning" in the same section (AR:41 and AR:43, again at derive-once-review:51). The pages tell the reader to ask the user when they disagree, so the two are meant for different cases, but the sentence "settles the meaning" is stated as a decision for the code. I do not decide.
Kinds: K1 (3-4 copies), K3 as above. No Confirmed ruling found that speaks to the definitions rule (searched "CONTEXT" in rulings: only Unconfirmed notes).

---

## Q12. The agent rules in short and long form (I5-031, I5-033, I5-034, I5-039, I5-045, I5-046, I5-047, I5-048, I5-049, I5-051, I5-052, I5-053, I5-054, I5-060)

PC:61 "the full text, and which hooks check which rule, is in docs/agents/agent-rules.md" and AR:3 "This page holds the full text of the fifteen agent rules ... The numbers never change". Each rule is stated in PC, AR, and often MP, BP, agents\*.md, M.

| Rule | Copies found (file:line) |
|---|---|
| 1 tests | PC:63, AR:7, BP:47, MP:9, MP:11, executor.md:11, reviewer.md:21, M\merge-tests-touch-changed-files.md:11-15, G\README.md:17 |
| 3 approval | PC:65, AR:9, MP:3 |
| 9 trust | PC:71, AR:15, BP:91-95, MP:13, handoff.md:3, M\derive-display-from-engine-truth.md:11 |
| 10 CI | PC:72, AR:16 |
| 11 "You will see" | AR:17, M\questions-describe-the-real-rule.md:11 (pointer plus extra), PC:73 short |
| 12 visible change | PC:74, AR:18, derive-once-review.md step 9 |
| 13 stray edits | PC:75, AR:19, G\ref_judge.py:339 (message "only your own uncommitted edits remain"), M\git-traps-in-shared-checkout.md:13 |
| 14 builds | Q1 |
| 15 owner | PC:77, AR:21, BP:115 |
| display decides nothing | PC:77, AR:23, agents\planner.md, agents\reviewer.md (owner-map lines), owner-map.md |
| owner map | PC:55, AR:25, owner-map.md, planner.md:19, reviewer.md:17, scout.md:11 |

Rule 1 detail (I5-034): the owner claim in AR:7 is that hydra_precheck "decides what counts as shared code and when the whole suite runs". M\merge-tests-touch-changed-files.md:15 says to run the lines on "its `tests:` and `always:` lines", while AR:7 first says the `tests:` line and later "prints them as three `always:` lines". The two agree in the end (AR:7 covers both); the note is shorter and says `always:` once. For hooks, AR:7 names gates\common.py, gates\lib, settings.fragment.json and gates\tests\lib as shared (a second list of shared code, K1/K6-adjacent: the precheck owns it for Hydra; the hook-side list lives only in AR:7). I did not read hydra_precheck's source for the hook list.
Rule 3 detail (I5-039): split 12a (K3): the word "build" means two things. AR:9 "In this rule a build means any piece of work: writing code, editing hooks, or launching agents"; AR:20 "In this rule a build means compiling, not rule 3's piece of work." MP:3 "Every plan and every build waits for it. A code-only change needs no approval of its design, but its build still waits" uses rule 3's meaning, while MP:9 "Any build, and any test that needs one, is yours" and MP:11 "build its branch (hydra_build_pass ...)" use the compile meaning. M\one-build-per-wave.md quotes the user using "builds" for compiles. No definition in either place is wrong; the word's two meanings sit in one file (MP).
Rule 12 detail (I5-048): the ruling is a click ("Yes, ban them (Recommended)"), Confirmed at H\docs\agents\rulings\questions-and-audits.md:20; rule 11 at :18; rule 13 at :19. The words quoted in AR as "Yes, require it" and "Yes, add it" are the user's clicks. AR:19 (rule 13) says reports the user; G\ref_judge.py:339 prints "only your own uncommitted edits remain" for a merge undo (different matter, same word "edits").
Rule 14 stall detail (I5-054): AR:27 states 10, 20 and 30 minutes and says "A hook keeps rule 14's 'agents never build'"; the numbers match stall.py:20-22 and progress.py:15 today (STALL_SECONDS 600, MINUTES_STOP 30). BP:73 states 100/150/20/30; progress.py:14-15 is the owner; test_docs_numbers.py:110-122 and :182 hold the call and minute numbers; it does not hold AR:27's "10 minutes" or "20 and 30 minutes of the same silence".
Rule 15/display/owner map (I5-051, I5-052, I5-053): PC:77, AR:21-25 and BP:115 state the same ideas in three lengths; planner.md/reviewer.md/scout.md carry rule fragments (e.g. scout.md: "A name that looks close is not an owner unless its own words claim the question"), and AR:25 says the brief "never adds the main session's own sentence". K1/K7: the owner-map shape is in H\docs\agents\owner-map.md (a named home); the copies point there in some lines and restate in others. K4 note on AR:23: it says a plan never puts a made-up rule to the user; the memory note (Q10) says the same; no hook checks it.
Rule 60 (I5-060): AR:3 "the fifteen agent rules ... The numbers never change". PC.rules run 1 to 15 (PC:63-77), so the count agrees today; no test checks the count (searched test_docs_numbers.py for "fifteen": none).
Rule 9 (I5-045) and rule 10 (I5-046): single-sentence rules, copies are word-for-word in PC and AR (rule 9 also at MP:13 and BP:91-95); no drift found. Rule 9's last sentence appears in PC, AR, MP:13 in the same words.
Rule 1 edges: main session against an agent: AR:7 lets the main session run precheck's tests; agents get "any other test is for its own change". Card writers: "run no tests" in card-writer.md:9. Worktree vs main checkout: AR:7 "runs ... precheck --repo <path>" from the main checkout. Exactly at the limit: not applicable.
Tests holding PC equal to AR: H's docs-vs-code test (`hydra_tests.exe "-tc=docs match code*"`) runs on docs; G\tests\test_docs_numbers.py:25-29 reads PC and AR for the call and minute numbers only (header comment line 24: "CLAUDE.md keeps each agent rule as one short line; agent-rules.md holds the full text"). No test pairs PC lines with AR lines for rules 1, 3, 9-13, 15.
Rulings (Confirmed): agents-and-workflow.md:8 (builds, Q1); questions-and-audits.md:18-20 (rules 11, 12, 13 clicks); tests-and-runs.md:14 "i want to be able to give explicit approval for a full library run." and :15 "Fix the doc line, and also add the repo-wide tests ... to every merge run." (session c965ee5e), which bears on AR:7. The 2026-11-13 quote in AR:21 (rule 15) was not found under Confirmed (searched "second owner", "copied from", "owner").

---

## Q13. How far may an agent edit (I5-074)

Homes: BP:41 "Touch only the files your brief owns, plus your scratch folder. Never delete, move or rewrite anything else"; G\delete_rule.py (enforces delete only; README.md:19 describes it: not read in detail); my own dispatch-level text: scout.md says change no files except your status line and scratch folder. Executors: "owned files" in MP:5. K1 plus K5 (BP:41 tells agents what delete_rule enforces only for deletes; "move or rewrite" has no hook). Worktree vs main: model-postures.md:5 says each task in its own worktree under a `worktrees` folder (the delete rule trusts only those). Card writers: card-writer.md:9 "Write only the card files for the sessions in your batch". Ruling: none found. Single main home (BP:41), nothing disagrees.

---

## Candidates (summary)

K1 copies: build limit three (Q1), tool name string (Q1), 500K (Q3), 660 timeout (Q4, pinned by a test), catchers (Q8), test file names (Q6), plain-English rule (Q9), rule short/long (Q12), CONTEXT.md rule (Q11), stall numbers 10/20/30 (Q2).
K2: none found.
K3 split-like: nudge-first against TaskStop-first (Q2); "build" two meanings in model-postures (Q12); "ask questions" against "ask only game questions" (Q10); "CONTEXT.md decides" against "code's usage settles" (Q11).
K4: context warning asks the main session to rely on a fact (auto-compact at 50% of window) no tool reads (Q3).
K5: model-postures.md:11 spells out what STATUS_HEADER already says (Q2); BP:41 spells out part of delete_rule (Q13).
K6: crash handling in common.run and again in two PowerShell scripts (Q5).
K7: write-back duty only in PC:29 (Q7); plain-English rule with no checker (Q9).
K8: H\.derive-once-gate and H\.doc-review-gate name `hooks\lib\*.ps1` as deciders; that folder is gone, the logic is in G\review_rules.py (Q7); wake.py:6 and check.py:33 cite a plan file not found under docs\superpowers\plans (Q4, Q8); AR:21/AR:23 quotes of the user not found under Confirmed (Q10).

Splits as separate lines:
- SPLIT: model-postures.md:11 (nudge once with SendMessage, then TaskStop) against stall.py:124 and :144 (stop with TaskStop and re-dispatch, no nudge).
- SPLIT: model-postures.md:3 and :9/:11 use "build" for different things; AR:9 and AR:20 each define their own meaning.
- SPLIT: PC:1 "Ask questions if something is unclear" against M\ask-only-game-or-permission-questions.md and AR:23.
- SPLIT: AR:41 "CONTEXT.md decides what a project word means" against AR:43 "The code's usage settles the meaning".

Side notes (outside my rows):
- H\tools\build_slot.ps1:78 default slot folder under `~\.claude\hooks\state`, while hooks now live in `gates`.
- A memory note in another project folder (terraria-mods\memory\workflow-agent-messaging.md:11, outside the scope listed) says never SendMessage a workflow agent; MP:11 tells the main session to nudge with SendMessage. I did not check if workflow agents are meant.
- G\stall.py:131 treats exactly 30 minutes as not yet past the limit; AR:27 says "30 minutes after its first call gets one more notice". Not run.
- A hook gap: the build refusal reads the structured tool name only (README.md:15), so a build through any other route than that tool is only stopped by build_cpp.ps1's own pass check (AR:27). No steps given.
