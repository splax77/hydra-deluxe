# TOOL-AUDIT-T5-F01 (finder F01, agent a747f37212c3e7688)

Paths: G = C:\Users\Patrick\.claude\gates\ ; H = C:\Users\Patrick\Downloads\Hydra\hydra-test\ ; SC = C:\Users\Patrick\.claude\tools\session_cards\ ; M = the memory folder.
Searched for every row: the number and the question's own words across G (py, ps1, README, tests), SC, agents\, model-postures.md, CLAUDE.md (both), H docs\agents (with rulings), H tools\*.md, .claude\commands, M (not archive). Grep patterns used the number, the constant name, and the plain words (e.g. "silent for", "wrapping up", "at once"). Searches that print the whole long line were cut by the tool, so a home found only inside a very long line may be named without its exact column.

Required reading done: plan hooks-from-scratch line 108 says "The numbers (100, 150, 20, 30, 5, 10) sit at the top of progress.py and nowhere else in code; the docs quote them, and a test in T11 greps the preamble and CLAUDE.md ...". Confirmed lines of review-and-hooks.md (8-34) and agents-and-workflow.md (8-29) read.

## Confirmed rulings that speak to these rows (quoted, never a home)
- H docs\agents\rulings\review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time, they should plan to wrap up soon and dispatch remaining work to a new agent with fresh context. Agents have a hard cap of 30 minutes real time." (session e82ae000). Speaks to I1-015.
- review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (session 48cdbb34). Speaks to I1-016.
- review-and-hooks.md:33 "demand status every 10 tool calls and reject the next call after 15" (session 48cdbb34). This is a Confirmed line that states a calls-based status rule the code no longer has (code is minutes only). It sits beside line 34 and line 22 ("minutes only"). Report only; no Confirmed line anywhere states 100 or 150 calls.
- review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire. send agents to compile a full plan" (session b5e8d95a).
- agents-and-workflow.md:15 "continue, and raise the cap to 60." (session 205c56ae). Speaks to I4-024 and I4-065 (card line cap 60).
- agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (session e81b56eb).
- No Confirmed line found for: the 1,000-character Bash limit, the git-commit exemption, the 4,000 output cap, 500K context, the 10-minute stall step, the 2-hour claim, the 500 MB / 24 h reminder, the 2-hour running cutoff, the batch sizes, INSTALL_DATE. The lines that carry the 1,000 limit and the commit exemption are under `## Unconfirmed` (review-and-hooks.md:40, 43; heading at line 37). See split S5.
- agents-and-workflow.md:35 (under Unconfirmed, a home): pass limit "2 minutes, 120 seconds" (outside my rows).

## Tests that hold homes equal (named once)
- T-A G\tests\test_docs_numbers.py: test_every_call_count_is_a_budget_number (line 110), test_status_interval_is_the_nag_minutes (116), test_agent_rules_rule_7_quotes_the_budget (122), test_preamble_quotes_the_budget (182), test_preamble_quotes_the_bash_limit (192), test_preamble_quotes_the_output_cap (198). They read DOCS = CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md only (line 25-26). They do NOT read writer-brief.md, model-postures.md, agents\reviewer.md, gates\README.md, the memory notes, or H tools\*brief.md.
- T-B G\tests\test_progress.py NumbersLiveOnce (835-858): bars budget numbers on lines about calls/minutes/budget in other G\*.py files, and pins the six values (100,150,20,30,5,10) as literals at line 856-858 (a second copy of the numbers inside a test).
- T-C G\tests\test_wake.py:218 `hooks[0]["timeout"] == stall.STALL_SECONDS + stall.SCAN_EVERY` holds the registered 660 in settings.fragment.json equal to stall.py.
- T-D G\tests\test_bash_windows_rule.py:53 `MAX_CHARS, 1000` pins 1000 as a literal; G\tests\test_shell_output.py:143-203 pin 4000, 1500, 4001 and 7008 as literals (copies of CAP, HEAD/TAIL).
- T-E G\tests\test_stall.py:148, test_wake.py:118 pin the text "silent for 10 minutes"; test_stall.py:424-434 pin "500K" and 500000 / 499999 as literals.
- T-F SC\test_session_cards.py:322 uses check.MAX_CARD_LINES (derived, not a copy); :1009-1011 pin 7200 (copy of RUNNING_CUTOFF_SECONDS).
- No test found that holds equal: writer-brief.md line 71/101 to check.py / progress.py; model-postures.md 100/20/500K to progress.py/stall.py; agent-rules.md:27 stall steps (10, 20, 30) to stall.py; README "5 minutes" empty fleet to EMPTY_FLEET_SECONDS; rulings README INSTALL_DATE to ruling_rules.py; session_cards_start.ps1 HookBudgetSec 25 / common.py "25 s" to the registered 30; the card-wave size to anything.

## Rows

### I1-014 / I1-015 / I1-016 (call budget 100/150; minutes 20/30; status 5/10)
Owner claim: G\progress.py:13 "The one home of these numbers (the user's rulings on rule 7). Docs quote them; code reads them here."
Homes (value stated, not pointer):
- H CLAUDE.md:69 "Agents write a status line every 5 minutes; only time counts, not calls."
- H docs\agents\agent-rules.md:13 rule 7: 5 minutes, 100 tool calls, 150 hard stop, 20 and 30 minutes (full text).
- H docs\agents\brief-preamble.md:13 "every 5 minutes"; :21 "5 minutes old ... 10 minutes old"; :71 "Commit before your 100th tool call"; :73 "At 100 tool calls, or at 20 minutes ... At 150 tool calls or 30 minutes it stops you."
- H docs\agents\derive-once-review.md:9 "every 5 minutes", :90 "30 minutes", :110 "(at 100 tool calls or 20 minutes)"; H docs\agents\doc-review.md:9, :91, :100 same.
- H tools\effort_test_scorer_brief.md:1 "Write a status line every 5 minutes".
- SC\writer-brief.md:100 "One status line every 5 minutes"; :101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report."
- G\README.md:124 "At 5 minutes old it adds a reminder. At 10 minutes old it refuses the call ..."
- C:\Users\Patrick\.claude\model-postures.md:5 "if a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks."
- C:\Users\Patrick\.claude\agents\reviewer.md:11 "at 100 calls or 20 minutes you stop fixing, sign off ..."
- M\every-brief-carries-status-rule.md:15 "every 5 minutes"; M\one-reviewer-rule-d107.md:11 "wrap-up warning at 100 calls or 20 minutes", :15 "the reviewer cap is 150 calls like everyone else"; M\MEMORY.md:45 "every 5 minutes".
- G\tests\test_progress.py:856-858 literals.
- Hook messages: progress.py:94-100, 130-133, 137-143 build their numbers from the constants (derived, not copies).
What each home tells its reader to do (edges):
| home | at the limit | main session vs agent | skip-preamble agent (card writer) | worktree vs main |
| progress.py | warn at >= 100 calls / 20 min; refuse at >= 150 / 30 (calls >=, minutes >=); status nag at >=5, refuse at >=10 | agents only (percall calls it for subagents) | applies (hook is per agent) | state keyed by session/agent, not by checkout |
| preamble:73 | "At 100 tool calls ... it warns you once"; "At 150 ... stops you" | agent-facing | the card-writer.md says to skip the preamble, so this text never reaches it | n/a |
| agent-rules:13 | 100 start wrapping up, hand rest to fresh agent; reviewer hands nothing on | main session and agents | n/a | n/a |
| writer-brief:101 | "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report" (warning text of hook says "commit what passes and report") | agent only | this is the only copy a card writer reads; it says 20 min OR 100 calls | n/a |
| model-postures:5 | sizing rule for the main session: split a task that will not fit 100 calls and 20 minutes | main session | n/a | n/a |
| reviewer.md:11 | stop fixing at 100/20 | reviewer | n/a | n/a |
| README:124 | 5 min reminder, 10 min refuse, git and final report still pass | all | n/a | n/a |
Kinds: K1 (many copies agree today). K3 for writer-brief:101: the copy keeps 20/100 but changes the action ("stop starting new cards: finish the one in hand") against the hook text ("commit what passes and report what is unfinished") and the preamble. K5 (prose spelling out what the progress hook enforces): preamble:21, :73, writer-brief:100-101, doc-review:100, derive-once-review:110.
Kind K7-flavoured note: none; the rule has a home (progress.py) and a partial test (T-A, T-B), but T-A skips writer-brief, model-postures, reviewer.md, README, memory notes.
Splits: none of values today (all agree). The one difference is S1 below (the action at 100/20).
S1 writer-brief.md:101 ("stop starting new cards: finish the one in hand and report") vs progress.py:94-100 hook text and preamble:73 ("commit what passes and report what is unfinished") vs reviewer.md:11 ("stop fixing, sign off"): three different actions at the same limit.
Rulings: review-and-hooks.md:24, :34, :33, :22 above. Tests: T-A, T-B.

### I1-022 QUOTE_CHARS = 300 (how much of a refused command is quoted)
One home for this question in the hook that quotes (progress.py:31, used at 128). Related but a different question: common.py:46 truncates a logged reason to `[:300]` (I1-051). Searched G\*.py, README for "300" and "quote". Another place where a refusal quotes a command back: none found beyond these two. Two code constants with the same value 300 and no link: K1 between progress.py:31 and common.py:46 only if they answer the same question; they answer "how long is a quoted piece" for different outputs. No test holds them equal. No Confirmed ruling.

### I1-025 `git ` commands and report tools are exempt from the budget
Homes: progress.py:119 (rule). Prose: preamble:21 "Two kinds of call still pass then: a command that starts with `git`, and your final report"; preamble:73; G\README.md:124; progress.py:131 message "Only your final report, git commands and your status line get through now."; M archive note not counted. Preamble:21 states the rule for the status deadline, preamble:73 for the budget. K5 (prose spelling out hook rule). Edge: `command.startswith("git ")` after lstrip; "git" alone with no space is not exempt. The preamble says "a command that starts with `git`". Possible K3 only at that edge; preamble gives no detail. Main session: progress only runs for agents. Card writers: PowerShell only per card-writer.md, and the exemption applies to their `git` calls but they commit nothing. Test: test_progress.py (not read in detail). No Confirmed ruling.

### I1-028 / I1-029 warning and spent-budget text
Homes of the same instruction: progress.py:94-100, 130-133 (hook), preamble:73 "start wrapping up, commit what passes and report what is unfinished, so a fresh agent can take the rest", agent-rules:13, writer-brief:101 (different action, see S1), reviewer.md:11, derive-once-review.md:110, doc-review.md:100. K5 and K3 as in I1-014. Test: none holds the wording equal.

### I1-030 the clock start (first call stands in for status time)
Homes: progress.py:147-152 and 87-90 (record created on first call). Prose: preamble:21 "When your last line is 5 minutes old"; preamble:73 "counted from your first tool call"; doc-review.md:91 and derive-once-review.md:90 "the progress hook counts an agent's 30 minutes from its first call and never restarts the count." No prose says a status line sets the first status age to the first call. One code home; prose copies the time-limit start only. K1 for the 30-minute clock. Searched "first call", "first tool call".

### I1-054 lock wait numbers (common.py 100 x 0.01 s; 2.0 s / 0.001 s)
One code home in G\common.py:14,18. Searched G docs and README for "lock" and the numbers: the progress.py replies mention "without the lock" (text only, no numbers); tests hold_lock helpers. A separate lock in SC\common.py:140-145 (15 s, 2 min, 0.05 s) answers "how long to wait for a lock" with other numbers: two owners of the same kind of question in two tool families (K6), values differ on purpose by claim only. See I4-058.

### I1-060 / I1-061 shell output cap 4000, keeps 1500 + 1500
Owner claim: shell_output.py:14 "The one home of these numbers and lists ... Docs point here."
Homes: H docs\agents\brief-preamble.md:35 "A result over 4,000 characters arrives as its start, its error lines, its end ..." (value stated); test_docs_numbers.py:198 holds the preamble to CAP (T-A); G\tests\test_shell_output.py literals 4000/1500 (T-D). G\README.md:23 names `CAP` only (pointer). The shell_output.py message text TRIMMED_LINE prints lengths from the data. 1500 appears nowhere in docs. ERROR_CHARS derived (line 18).
Table: preamble:35 tells every agent a >4,000 result is cut; the hook applies to main session and agents (PostToolUse on Bash|PowerShell); worktree irrelevant; card writers skip the preamble but the hook still applies to their PowerShell calls. At exactly 4000 the hook does not trim (`len(text) > CAP`), preamble says "over 4,000" (agree).
Kinds: K1 (preamble:35, test literals). Splits: none. Rulings: none Confirmed (the plan says the numbers were the user's answers; no Confirmed line).

### I1-072 Bash length 1000 (git commit text excluded) and I1-076 commit exemption
Owner claim: bash_windows_rule.py:5 "This module owns the length limit and the five refused shapes"; :16 "the user's number"; :172-174 "(the user's ruling)"; G\README.md:27 "MAX_CHARS in that file is the one home of the number".
Homes: H docs\agents\brief-preamble.md:33 "A command over 1,000 characters ... A `git commit`'s own text does not count toward that limit"; test_docs_numbers.py:192 (T-A) and test_bash_windows_rule.py:53 literal 1000 (T-D); H docs\agents\rulings\review-and-hooks.md:40 (Unconfirmed, "Exempt git commit") and :43 (Unconfirmed, "length over 1000 and backslashes", "chose 1,000"). Hook message at bash_windows_rule.py:211-214 uses MAX_CHARS (derived).
Edges: exactly 1000 passes (`> MAX_CHARS`); the hook says refuse "over" and the preamble says "over 1,000" (agree). Main session and agents: both (README:27). Card writers: use PowerShell only; Bash rule irrelevant. Worktree: no difference.
Kinds: K1 (preamble, test). K8 candidate: the code and README say "the user's number/ruling" but the only line carrying either is Unconfirmed (see S5). Same operator question as I1-041: see S2.
S2 (code): two code places cut a command into pieces. G\delete_rule.py:29,75-77 `OPERATORS = re.compile(r"&&|\|\||[;|&\r\n]")` splits on the raw text, not quote-aware (pieces: "the one reader of where a command is cut"); G\bash_windows_rule.py:19,55-77 `OPERATOR_CHARS = ";|&\r\n"` inside its own quote-aware `_scan` ("_scan is the one reader of shell quoting"). media_rule.py:20,23 reads delete_rule.OPERATORS ("an operator inside a quoted filter ("a;b") cannot split a tool from its output" - so media_rule has its own quote handling on top). bash_windows_rule.py imports delete_rule (line 14) but uses its own piece cut. Two owners of "where does a Bash command end a part"; they answer an operator inside quotes differently (delete_rule cuts, bash_windows_rule does not). Preamble:43 "A delete inside quotes ... is not read" describes delete_rule's side only. No test holds them equal.
S5: bash_windows_rule.py:16, :172-174 and G\README.md:27 call the 1,000 and the exemption the user's; review-and-hooks.md lines 40 and 43 sit under `## Unconfirmed`; no `## Confirmed` line states either.

### I2-001 stall seconds 600; I2-003 notices per silence; I2-021 when a notice is due
Owner: stall.py:22-24 (`STALL_SECONDS = 600`, `STALL_NOTICES = LIMIT_SECONDS // STALL_SECONDS`).
Homes: H agent-rules.md:27 "a stall notice when an agent has made no call for 10 minutes ... The notice repeats at 20 and 30 minutes of the same silence, and the third says it is the last. The step is STALL_SECONDS in `stall.py` ..." (value and derived count stated, with pointer); G\README.md:29 "each time its silence passes the next step of `STALL_SECONDS`" (pointer only); stall.py:8-9 docstring "The numbers are the ones the plan sets (hooks-from-scratch plan, section 2)" (a plan stated as the source of the numbers; the plan is deleted/old); tests test_stall.py:148 and test_wake.py:118 pin "10 minutes" text (T-E). wake.py:51,63,66 read stall.STALL_SECONDS (derived). Hook message stall.py:116 builds minutes from time (derived).
Table: agent-rules:27 tells the main session notices come at 10/20/30 minutes; stall.py sends at `now-last > 600` and then each multiple, cap 3 at the 30-minute limit. At exactly 600 s: no notice (`<=`); agent-rules says "no call for 10 minutes" (edge differs by one second only). Agents: never see these notices. Card writers: same. Worktree: no.
Kinds: K1 (agent-rules:27 states 10/20/30 beside the pointer). K7 none. Split: none today. Mention K8 weak: stall.py:8 says the plan sets the numbers while the plan is not the owner. No Confirmed ruling; no test holds agent-rules:27 equal to the code (T-A does not read it for stall).

### I2-002 time limit; I2-023 past the limit
stall.py:20,23 reads progress.MINUTES_STOP (derived; claim "owns the agents' time limit" matches progress.py:13-16). agent-rules:27 "An agent that has not handed back 30 minutes after its first call gets one more notice" (copy of 30). K1. Message stall.py:137-140 uses the constant. I2-023 single code home (stall.py:134); prose is the agent-rules sentence. No test holds the 30 in agent-rules equal (T-A's rule-7 test reads other phrases).

### I2-004 scan every 60 s; I2-064 / I2-062 / I2-063 check-in and watch timing
Homes: stall.py:25 (60); wake.py:11-13 comment "The registered timeout is stall.STALL_SECONDS plus stall.SCAN_EVERY ... tests/test_wake.py checks the two stay equal"; settings.fragment.json "timeout": 660 for wake.py (G\settings.fragment.json near line 235; 660 = 600 + 60, a copy of two constants); the live settings.json carries it by sync_settings (test_live_settings_carry_the_fragment); G\README.md:31 "Once `stall.STALL_SECONDS` have passed ... `stall.SCAN_EVERY` seconds" (pointer). Test holds 660 equal: T-C (test_wake.py:218). K1 with a test. wake.py:63-66 read the constants (derived).
Table: wake sleeps `min(SCAN_EVERY, STALL_SECONDS - waited)`; hook timeout 660 gives 60 s slack. At exactly STALL_SECONDS wake.py wakes with check_in; stall's own rule needs `now-last > STALL_SECONDS` (stall.py:105): the check-in lands at waited >= STALL_SECONDS while the agent notice needs strictly more; test_wake.py:110-120 sets last call "a second before the turn ended". Main session only. Card writers/worktrees: no effect.
I2-062: single code home wake.py:48-66; README:31 describes it in words without numbers.

### I2-005 STATUS_MAX_LINES 12; I2-007 CTX_BUCKET 50000; I2-008 CTX_TAIL_BYTES 262144; I2-010 TOOL_TAIL 5; I2-014 JOURNAL_SLACK 60; I2-033 open Workflow rule; I2-034 context notice repeat
I2-010: stall.py:34 TOOL_TAIL = 5 (comment "display size"). H docs\agents\brief-preamble.md:23 "(five is TOOL_TAIL in `stall.py`)" and "The second is the names of your last five tool calls ... " and model-postures.md:11 "its last five tools" (value copies, K1). H agent-rules.md:27 describes the evidence line without the number. No test holds them equal (test_docs_numbers does not read stall.py). Main session reads model-postures; agent reads preamble.
I2-005, I2-007, I2-008, I2-014, I2-033, I2-034: one home each in stall.py (lines 26, 28, 29, 40, 303-338, 370-378). Searched G\README.md, H docs\agents, model-postures, M for "12 lines", "50,000", "262144", "journal", "slack": README:29 mentions the empty-fleet and "launch" in words with no number. I2-033 rule is described in README:29 as "a Workflow is open and no agent has written for 5 minutes" (see I2-013). No other homes.

### I2-006 context 500K threshold; I2-011 context text
Homes: stall.py:27 CTX_THRESHOLD = 500000; stall.py:36 CTX_TEXT "Auto-compact should have fired at 500K (50% of the window)" (literal inside the message, not the constant); C:\Users\Patrick\.claude\model-postures.md:15 "Keep this conversation under 500K tokens; the context hook warns you above that. End the phase and let compaction happen before the next dispatch ..." ; tests test_stall.py:424-434 literals.
Table: stall.py: at >= 500000 the notice fires (test pins exactly 500K warns, 499,999 does not); model-postures says "under 500K" (so exactly 500K is already over; edge differs by one token only). Hook text tells the main session: "If it has not [auto-compact], finish the current step, then ask the user to run /compact before the next dispatch." model-postures tells the main session: "End the phase and let compaction happen" (does not say to ask the user to run /compact). S3.
S3: stall.py:36-37 (ask the user to run /compact) vs model-postures.md:15 (let compaction happen; no mention of asking). Also "50% of the window" is a claim about the window size held only in stall.py:36 (K4: a model window size the hook cannot read; stall.py:38 USAGE_FIELDS reads usage only). Kinds: K1 (500K in 3 places), K3 (action differs), K4 candidate. No Confirmed ruling. No test holds model-postures equal to CTX_THRESHOLD.

### I2-013 empty-fleet 300 s
Homes: stall.py:39 (300); stall.py:42 FLEET_TEXT takes minutes from the number (derived); G\README.md:29 "no agent has written for 5 minutes" (value copy). Test: none found holding README's 5 equal. Also unrelated same number 5 minutes (status nag, progress.STATUS_NAG_MINUTES) answers a different question. K1.

### I2-076 FIELD_CAP 10000; I2-079 over-cap message
Homes: prompt.py:19-21 (value, with citation "code.claude.com/docs/en/hooks, 'capped at 10,000 characters'"); prompt.py:55-57 message builds from the constant; G\README.md:33 names `FIELD_CAP` (pointer, no value); G\tests\probe_results.md:127, :223 state "About 10,000 characters" for a different measured cap (the failure-cut excerpt of a tool result; same number, different question, not the additionalContext cap); test_posture.py:265-272 reads FIELD_CAP. Other places that cap what Claude Code shows: shell_output.py "Above 30,000 characters it arrives as Claude Code's own notice" (preamble:35). So 10,000 appears for two different Claude Code caps (hook context field; failed-tool excerpt) in prompt.py and probe_results.md. Single owner for the context cap: prompt.py. K1 only if probe_results counts. Test: none holds probe_results equal.

### I2-091 GitTimeoutSec 600 (cleanup_sweep.ps1:58)
One home. Searched G for "600", "timeout" in README/ps1/py: settings.fragment.json registers cleanup_sweep with "timeout": 30 (line 81), a registration that is smaller than the 600 s git limit (the sweep may be killed first). Same question family as I2-106 (hook time budget). Unlike session_cards_start.ps1 (which sets 25 to fit 30), cleanup_sweep.ps1 has no budget tied to its registered 30. K2/K6 candidate: two timeouts for one hook (script 600 s git vs registration 30 s). No test holds them equal.

### I2-103 claim 2 hours; I2-104 500 MB; I2-105 24 hours; I2-106 hook budget 25 s
Homes: session_cards_start.ps1:43-50 (owner, "The user's choice (task SC-9, 2026-10-14)"); :338 comment "does not hold the work for 2 hours" (copy of the number in a comment, I2-103); :16-17 header comment names `$ReminderGapHours`, `$RemovalReminderMb` by name (pointers); :46 comment "at most once a day" (a word copy of 24). settings.fragment.json:93 registers session_cards_start with "timeout": 30; common.py (SC):141 "15 s keeps the SessionStart hook inside its 25 s budget" (copy of 25 in another tool). test_h53 line 212 `$took -lt 30` (a literal 30 in a test). No Confirmed ruling for any of these: the plan SC-9 is cited; no ruling lines found by grep for "SC-9", "claim", "reminder" in rulings\ (only the 2 minutes pass limit at agents-and-workflow.md:35, a different row).
I2-106: the 25 is stated in session_cards_start.ps1:49, SC\common.py:141 and (30) settings.fragment.json:93; the relation "30 less 5" is stated in prose at ps1:48-50 ("the registration's 30 s timeout, less 5 s"). No test holds 25 and 30 equal (test_h53:212 checks only `< 30`). K1/K2 risk: change one and the others drift. Hook budget is in the main session's SessionStart only; agents unaffected; card writers unaffected.

### I2-117 "batch.py sizes a wave to the at-once limit"
Home: session_cards_start.ps1:277 (text printed to the main session). Owner claim: that batch.py holds "the at-once limit". Search for any "at-once" limit: batch.py:26 `BATCHES_PER_WAVE = 10` ("The plan's batch limits and wave size (user-approved with the plan)"). The only limit in the rules is agent rule 14 (H CLAUDE.md; agent-rules.md:20, 20 lines): "At most three builds run at once ... In this rule a build means compiling", which is about builds, not agents. M\lean-agent-fanout.md says "~8 jobs per agent, 3-4 agents per round" (a memory note, a home for a different fan-out number). So the "at-once limit" the sentence names: batch.py says 10 batches per wave; the rule's at-once number is 3 builds; no other agent concurrency limit found. K8 (wrong owner claim, or a limit named without a limit). Searched: agent-rules.md, CLAUDE.md both, model-postures.md, memory (not archive), SC\*.py, ps1.

### I3-008 ORIGIN_TIMEOUT 30 s; I3-009 WATCHDOG 60 s
One home each: ref_judge.py:37-38 (uses at 171, 390-393). Searched G README, install_ref_gate.ps1, settings.fragment.json, H docs\agents, tests for "30 s", "60 s", "30 seconds", "60 seconds", "watchdog": none state these; test_ref_judge.py:404 overrides WATCHDOG for a test. README:53-57 describe ref_judge without numbers. No other home. One home (searched as above). Relation 30 < 60 (ls-remote must finish inside the watchdog) is not written anywhere.

### I3-054 MAX_BRIEF_FOLDERS 16
One home: review_dispatch.py:36 (used at 87). Tests read the constant (test_review_dispatch.py:272, 281). Searched H docs\agents (derive-once-review.md:200, doc-review.md) for "folders named in a brief": they describe the rule ("write that repo's root folder once in the brief") without the number 16. Single home.

### I3-072 INSTALL_DATE 2026-11-14
Homes: ruling_rules.py:97-99 (owner; comment quotes "the user's answer 'Yes, no click' (2026-11-13, session ca29fda5)"); H docs\agents\rulings\README.md:51 "the day the gate was installed (`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14)" (value copied beside the pointer; also "The user approved it on 2026-11-13 ('Yes, no click')" - a quote of the user's words in a README, not under `## Confirmed`); H docs\agents\agent-rules.md:25 and docs\sessions\index.md:503 mention the date 2026-11-14 for another thing. Test: test_ruling_rules.py:375 reads rr.INSTALL_DATE (derived); no test holds README's 2026-11-14 equal. K1. The ruling README:51 line and ruling_rules.py:97-99 both quote "Yes, no click" with date 2026-11-13: neither is a Confirmed line.

### I3-082 transcript read block 65536
One home in ruling_rules.py:192-204 (min(65536, pos)). Searched G for 65536/block: none other. (Hydra tools\hydra_ui_mcp.py:538 and fake_hydra_live.py:34 use 65536 for unrelated reads; out of scope.) Single home.

### I4-024 / I4-065 card line cap 60; header shape
Homes: SC\check.py:28-30 MAX_CARD_LINES = 60 (comment: "The user raised it from the plan's 45 on 2026-10-14"); SC\writer-brief.md:71 "The whole card is at most 60 lines; aim for 20 to 40. A long session may use the room up to 60"; H docs\agents\rulings\agents-and-workflow.md:39 (Unconfirmed note: "Cap is 60 lines (user, 4f18f31)"); SC\test_session_cards.py:322 (derived). Ruling found: agents-and-workflow.md:15 Confirmed "continue, and raise the cap to 60." (quoted above). Table: check.py rejects `> 60`; writer-brief says "at most 60" (agree at the limit). Card writers read only writer-brief (card-writer.md skips the preamble), so the 60 reaches them only through the brief's own copy. Main session/worktrees: check.py runs from the main session. K1 (no K2). K5: writer-brief spells out what check.py enforces. No test holds writer-brief:71 equal to MAX_CARD_LINES.
Header shape (six parts joined by " | ", six headings in order): writer-brief.md:71 spells it; check.py enforces it; I did not open check.py's own rule to compare line by line (not reached in depth).

### I4-035 writer limits and status line
Homes: SC\writer-brief.md:99-101 (status line shape and 20 minutes / 100 tool calls); also the same numbers in progress.py (see I1-014/015). Writer brief status line text `"HH:MM done <n> of <m> cards | next: <session>"` vs preamble:13 `HH:MM done ... | next: ...` and progress.py:58 `_helper_line` (form agrees; the hook accepts any quoted line). The writer-brief copy of the call path `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"` copies progress.py:22 STATUS_PREFIX (a text a hook reads exactly); test_docs_numbers.py:189 holds only the preamble's copy equal to STATUS_PREFIX, not writer-brief's (T-A). Same copies in doc-review.md:9, derive-once-review.md:9. K1, K5, K3 (S1 for the action at 20/100). Writer's final report content is single-homed (writer-brief:103-105).

### I4-058 manifest lock timing (common.py SC:140-145)
Homes: SC\common.py:141-144 (15 s, 2 minutes, 0.05 s); comment "15 s keeps the SessionStart hook inside its 25 s budget" (copy of 25 from session_cards_start.ps1:49, itself a copy of the 30 registration less 5); comment "The user's choices (2026-10-25)". G\common.py:14-18 is a different lock with other numbers (I1-054). Test SC\test_session_cards.py:1296 reads common.LOCK_STALE_SECONDS (derived). No Confirmed ruling found. K1 (25), K6 (two lock-wait helpers).

### I4-080 running cutoff 2 hours; I4-091 stale card rule
Homes: prepass.py:30-31 (owner; "Plan decision 6 (user-approved)"); prune_list.py:20 comment "prepass is owner of the running-session cutoff", :114 reads prepass.RUNNING_CUTOFF_SECONDS (derived); session_cards_start.ps1:43 `$ClaimHours = 2` (a separate "2 hours" about claim time, a different question with the same number); test_session_cards.py:1009-1011 literal 7200 (copy). I4-091: prepass.py:400-405 only; index.py shows [stale] (reads the flag); H docs\sessions\index.md shows [stale] on cards. No prose states the rule outside the plan. Kinds: K1 for 7200 in the test; no split.

### I4-081 TOOLS line caps 300 / 600
One home prepass.py:32-34 ("inherited from the planner's measured prototype"). Searched SC *.py, writer-brief for "300", "600", "TOOLS": writer-brief mentions TOOLS lines in the shape of the input only. Single home.

### I4-098 batch size and wave size; I4-099 batch names and retry rule
Homes: batch.py:23-26 (200 KB, 10 sessions, 10 batches; "(user-approved with the plan)"); writer-brief.md:26 states the line shape `<session id> TAB <condensed file path> TAB <KB>` (the shape batch.py:71-73 writes); session_cards_start.ps1:257-262 reads batch names by pattern `wave-(\d+)-batch-(\d+)\.txt` (a copy of batch.py:71's name shape, K1/K6: two code places know the name format); session_cards_start.ps1:277 names "wave" ordering. See I2-117 for the "at-once limit" claim. No test holds the ps1 name regex equal to batch.py's format (searched test_h53 for "wave-": not found by my grep; not confirmed). Retry rule (`--failed` once, old batch files deleted at each run, largest first): single home batch.py.

## Splits (own lines)
- S1 Action at the 100-call / 20-minute limit: writer-brief.md:101 ("stop starting new cards: finish the one in hand and report") vs progress.py:94-100 and preamble:73 ("start wrapping up: commit what passes and report what is unfinished") vs reviewer.md:11 ("stop fixing, sign off what you fixed, list unfixed under Notes").
- S2 Where a Bash command is cut into pieces: delete_rule.py:29,75-77 (regex split, not quote-aware) vs bash_windows_rule.py:19,55-77 (its own quote-aware scan); media_rule.py:20-23 leans on delete_rule's OPERATORS and adds its own quote handling.
- S3 What the main session does at 500K context: stall.py:36-37 (ask the user to run /compact before the next dispatch) vs model-postures.md:15 (end the phase and let compaction happen; no mention of asking).
- S4 Hook timeouts: cleanup_sweep.ps1:58 `$GitTimeoutSec = 600` vs settings.fragment.json:81 registration "timeout": 30 for the same script.
- S5 Claim of a user's ruling with no Confirmed line: bash_windows_rule.py:16, 172-174 and gates\README.md:27 vs review-and-hooks.md:40, 43 (under `## Unconfirmed`, line 37).
- S6 "At-once limit": session_cards_start.ps1:277 ("batch.py sizes a wave to the at-once limit") vs batch.py:26 (10 batches per wave) vs agent-rules.md:20 rule 14 (three builds at once, builds only).
- (Not a split) Confirmed review-and-hooks.md:33 "every 10 tool calls ... reject the next call after 15" is the user's own words and the code has no calls-based status rule; reported only.

## Candidates by kind (ids, no exits)
- K1: I1-014, I1-015, I1-016, I1-060, I1-072, I1-076, I2-001, I2-002, I2-006, I2-010, I2-013, I2-064, I2-103, I2-104, I2-105, I2-106, I3-072, I4-024, I4-035, I4-058, I4-065, I4-080, I1-025.
- K2: none found with a different value today (S4 is a script limit against a registration).
- K3: I1-028/I1-029 (S1), I2-011 (S3).
- K4: I2-006 / I2-011 (the 50% of the window claim; a window size the hook cannot read).
- K5: I1-014/015/016 (preamble:21, :73, writer-brief:100-101, doc-review:100, derive-once-review:110), I1-025, I1-028, I4-024 (writer-brief:71 spells out check.py).
- K6: I1-054 / I4-058 (two lock-wait helpers), I1-041 / I1-072 (S2), I4-098/I4-099 (batch file name format in batch.py and the ps1).
- K7: none found. K8: I1-072 and I1-076 (S5), I2-117 (S6), stall.py:8-9 (plan named as the source of the numbers).

## Rows not reached in depth (searched by number and name only)
I1-022, I1-030, I2-005, I2-007, I2-008, I2-014, I2-033, I2-034, I3-008, I3-009, I3-054, I3-082, I4-081, I4-091, I4-099 (single-home or near-single-home findings above rest on the grep list at the top, not on reading every candidate file in full). Rows not reached at all: none.

## Side notes
- G\README.md:154 says the preamble must quote `shell_output.CAP`; the test that enforces it is test_docs_numbers.py:198 (T-A).
- H docs\agents\brief-preamble.md:23 says "five is TOOL_TAIL in `stall.py`" (a pointer that also states the value: a home).
- G\tests\test_progress.py:837 cites "Plan section 4" as the source of the one-home rule; H plan 2026-10-25-hooks-from-scratch.md:108 also says a T11 test greps the preamble and CLAUDE.md for `110` and `every 10 tool calls`; the test that does it is test_docs_numbers.py (RETIRED at 29), which scans five docs only.
- Hook gap (general words): the status-line call shape is checked only for the exact helper path; a status line is accepted whatever it says.
- M\one-reviewer-rule-d107.md:15 states "the reviewer cap is 150 calls like everyone else" (a memory-note copy of CALL_STOP).
- Memory note lean-agent-fanout.md gives "~8 jobs per agent, 3-4 agents per round", a fan-out number no hook or batch.py reads.
