# TOOL-AUDIT-T5-L01 (ledger compiler, finders F01, F02, F03)

Compiled only from reports TOOL-AUDIT-T5-F01, F02, F03, F03b and V01, V02, V03, V03b. Nothing searched again, nothing run. Where a verifier and finder disagree the verifier's reading is used and said so. No exits chosen, no opinions on what should change.

Short names: G = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test; SC = C:\Users\Patrick\.claude\tools\session_cards; M = the memory folder; BP = H docs\agents\brief-preamble.md; AR = agent-rules.md; DOR = derive-once-review.md; DR = doc-review.md; MP = C:\Users\Patrick\.claude\model-postures.md; CH = clone-hero-mcp.md; UT = ui-testing.md; AM = audit-method.md; TR/TE = test-rubric.md / test-rubric-examples.md. Inventory row ids (I1-xxx etc.) are the T3 ids the finders name. "NOT VERIFIED" means no verifier opened the home; the finder's words are the only evidence.

Ruling files named in the reports: H docs\agents\rulings\review-and-hooks.md (Confirmed lines 7-36, Unconfirmed from 37), agents-and-workflow.md, code-and-engine.md (Confirmed from 7, Unconfirmed from 61), ui-and-display.md (Confirmed 7-101), clone-hero.md (Confirmed from 7, Unconfirmed from 67).

---------------------------------------------------------------------
## Finder F01 rows (gates, session cards, hook numbers)
---------------------------------------------------------------------

### L-F01-01: Where do the agent budget numbers (status every 5 min and refused at 10; warn at 100 calls / 20 min; stop at 150 calls / 30 min) live, and who repeats them?
- **Source:** I1-014, I1-015, I1-016; F01, F02 (Groups A and B, candidate 1), F03 (Group 6); V01, V02 (candidate 1).
- **Kind:** K1, K5. (K3 for the writer-brief copy, see L-F01-02.)
- **Verdict:** CONFIRMED (V01 for the homes it opened, V02 candidate 1). The preamble :71-73 and reviewer.md:11 homes are taken from the finder (V01 did not open them).
- **Homes:**
  - Owner in code: G\progress.py:14-16 `CALL_WARN, CALL_STOP = 100, 150`, `MINUTES_WARN, MINUTES_STOP = 20, 30`, `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`; comment at :13 "The one home of these numbers (the user's rulings on rule 7). Docs quote them; code reads them here." (V02 read it.)
  - H CLAUDE.md:69 and AR:13 "Agents write a status line every 5 minutes; only time counts, not calls." (V01 read CLAUDE.md rule 7.) AR:13 also states 100, 150, 20, 30 in full.
  - BP:13, :21 ("5 minutes old ... 10 minutes old"), :71 "Commit before your 100th tool call", :73 (finder only for :71-73).
  - DOR:9, :90, :110 and DR:9, :91, :100 (finder only).
  - H tools\effort_test_scorer_brief.md:1 "Write a status line every 5 minutes" (finder only).
  - SC\writer-brief.md:100-101 "One status line every 5 minutes" and "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report." (V01 read it.)
  - G\README.md:124 "At 5 minutes old it adds a reminder. At 10 minutes old it refuses the call ..." (V01 confirmed.)
  - MP:5 "if a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks." (V01, V02 confirmed.)
  - C:\Users\Patrick\.claude\agents\reviewer.md:11 "at 100 calls or 20 minutes you stop fixing, sign off ..." (finder; V01's read cut the long line.)
  - M\one-reviewer-rule-d107.md:11, :15 "the reviewer cap is 150 calls like everyone else"; M\every-brief-carries-status-rule.md:15; M\MEMORY.md:45 (finder; V02 saw the d107 note lines).
  - AR:27 stall text copies 10 / 20 / 30 minutes (see L-F01-14).
  - G\tests\test_progress.py:856-858 holds the six values as literals (second copy inside a test).
  - Hook messages progress.py:94-100, 130-133, 137-143 build their numbers from the constants (not copies).
- **Splits:** the values agree everywhere today. One words-only split added by V01 (S8): CLAUDE.md rule 7 "only time counts, not calls" against MP:5 sizing a task by "100 tool calls and 20 minutes", while progress.py still counts 100/150 calls. V01 read rule 7; the MP quote is the finder's; not run-checked. The split on what an agent is told to do at the warning is its own row, L-F01-02.
- **Ruling quotes:** review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time, they should plan to wrap up soon and dispatch remaining work to a new agent with fresh context. Agents have a hard cap of 30 minutes real time." (names the caps, not the action). review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes". review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire. send agents to compile a full plan". agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable". None names both sides of the S8 words split. No Confirmed line states 100 or 150 calls (finder). The older "status every 10 calls/5 min, soft cap 100 calls (hand off), hard cap 150" is at agents-and-workflow.md:43 under Unconfirmed (F02, F03).
- **Tests:** G\tests\test_docs_numbers.py (:110, :116, :122, :182) reads only CLAUDE.md, AR, BP, DOR, DR (DOCS at :25-26); it does not read writer-brief, MP, reviewer.md, README, AR:27, memory notes, or tools\*brief.md (V02 confirmed: its REVIEW_DOCS_HERE list is checked for doc keys only, not numbers). test_progress.py:835-858 NumbersLiveOnce holds the numbers once inside G code and pins the six values. test_docs_numbers.py reads docs from the main checkout (env GATES_HYDRA_ROOT or main), so a branch edit is not checked until merged (F02, finder only).
- **Open:** the card writer skips BP (card-writer.md:9), so it gets the numbers only from writer-brief.md; the writer-brief copy gives 20 min / 100 calls and not the 10-minute refusal (finder).
- **Hook gap:** none stated.

### L-F01-02: What is a reviewer or card writer told to do when the progress hook warns at 100 calls / 20 minutes?
- **Source:** I1-028, I1-029 (and I1-014 to I1-016 edge); F01 S1, F02 split 1 and 2, F03 note; V01 S1, V02 S1, S2, S5.
- **Kind:** K3, K5, K1.
- **Verdict:** CONFIRMED-CHANGED. V01 confirmed the split as three actions (writer-brief / hook / reviewer.md). V02 changed it: for a reviewer the hook text "report what is unfinished" is not a contradiction (it is the Notes list); the real difference is in the words around it (see Splits). The verifier's reading wins.
- **Homes:**
  - G\progress.py:94-100 hook text "Start wrapping up: commit what passes and report what is unfinished." (V01, V02). Same instruction at :130-133 for the spent budget (finder). No reviewer branch in the hook (V02).
  - BP:73 "start wrapping up, commit what passes and report what is unfinished, so a fresh agent can take the rest" (V02 quotes the "fresh agent" clause).
  - SC\writer-brief.md:101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report." (V01 read.)
  - reviewer.md:11 "stop fixing, sign off what you fixed, and list every unfixed finding under a Notes heading" (finder; V01 did not read the cut line; V02 names it as a home saying "No fresh agent picks it up" together with DOR:110 and DR:100).
  - AR:13 "A reviewer hands nothing on: it signs off what it fixed and lists the rest in the Notes" (V02 quote).
  - DOR:110, DR:100 "(at 100 tool calls or 20 minutes), stop fixing" (finder; V02: they say "No fresh agent picks it up").
- **Splits:**
  - S1: a card writer (reads only writer-brief) is told to stop starting new cards and report; any agent reading the hook or BP is told to commit and report; a reviewer reading reviewer.md is told to stop fixing and sign off. Same trigger (100 calls / 20 min), three instructions.
  - V02's reading (S5, folded in): BP:73 tells a reviewer the warning exists "so a fresh agent can take the rest"; AR:13, DOR:110, DR:100 and reviewer.md tell the same reader nothing is handed on. Same trigger, same reader, different instruction about who takes the rest.
  - V02 S2: review-and-hooks.md:24 speaks to all agents ("dispatch remaining work to a new agent"); the later lines :20 and :21 speak to a reviewer that runs out. No single line names both sides as a split.
- **Ruling quotes:** review-and-hooks.md:24 (quoted in L-F01-01). review-and-hooks.md:20 (2026-10-10) "i asked for the review agent to fix its findings directly, then we're done." review-and-hooks.md:21 "notes are fine, and yes do that for running out". D107 (H docs\audit\2026-10-03-fix-decisions.md:599, a decision-log line, not a ruling line) quotes "one reviewer who fixes his findings directly, then we're done. no more follow ups." V02: none of the three Confirmed lines names both sides of the split; whether the three together are enough is for the main session. V01: :24 names the caps, not the action.
- **Tests:** none holds the wording equal (finder).
- **Open:** reviewer.md:11 text taken from finder by V01. Not run-checked.
- **Hook gap:** none stated.

### L-F01-03: Where does a Bash command end a part (an operator `;`, `&&`, `|`) when the operator is inside quotes?
- **Source:** I1-041, I1-072; F01 S2; V01 S2.
- **Kind:** K1/K6 (two owners of one question), K3.
- **Verdict:** CONFIRMED (V01).
- **Homes:**
  - G\delete_rule.py:29 `OPERATORS = re.compile(r"&&|\|\||[;|&\r\n]")` and :77 `pieces()` split the raw text; docstring "the one reader of where a command is cut" (V01).
  - G\bash_windows_rule.py:5 "_scan is the one reader of shell quoting"; its `_scan` (line 40) cuts at OPERATOR_CHARS only in state BARE, so an operator inside quotes does not cut (V01). It imports delete_rule (line 14) but uses its own cut (finder).
  - G\media_rule.py:20,23 reads delete_rule.OPERATORS and adds its own quote handling ("an operator inside a quoted filter ("a;b") cannot split a tool from its output"); V01 did not re-read media_rule.
  - BP:43 "A delete inside quotes ... is not read" describes delete_rule's side only (finder).
- **Splits:** a command with `;` or `&&` inside quotes: delete_rule cuts it into pieces, bash_windows_rule does not.
- **Ruling quotes:** none (V01: no ruling).
- **Tests:** none holds them equal (finder).
- **Open:** media_rule's reliance on delete_rule not re-read by V01.
- **Hook gap:** none stated by the finder as a way past; the hooks themselves are the homes.

### L-F01-04: What does the main session do at 500K tokens of context, and is "50% of the window" a fact the hook can read?
- **Source:** I2-006, I2-011, I5-116; F01 S3, F02 candidate 6 and Group F; V01 S3 and I2-006/011 K4, V02 candidate 6 and split 3.
- **Kind:** K1, K3, K4, K6 (two statements of 500K in one file).
- **Verdict:** CONFIRMED (V01 S3 and K4; V02 candidate 6 CONFIRMED, split 3 "CONFIRMED but mild").
- **Homes:**
  - G\stall.py:27 `CTX_THRESHOLD = 500000`.
  - G\stall.py:36-37 `CTX_TEXT`: "Auto-compact should have fired at 500K (50% of the window). If it has not, finish the current step, then ask the user to run /compact before the next dispatch." The 500K and "50% of the window" are literals inside the message, not built from the constant (V01, V02).
  - G\stall.py:38 `USAGE_FIELDS` reads usage only, so the hook cannot read the window size (V01).
  - MP:15 "Keep this conversation under 500K tokens; the context hook warns you above that. End the phase and let compaction happen before the next dispatch ..." (V01, V02).
  - G\README.md:29 names the warning without the number (finder).
- **Splits:** when auto-compact has not fired: stall.py tells the main session to ask the user to run /compact; MP:15 tells it to end the phase and let compaction happen, with no mention of asking. V02: the hook text is conditional ("If it has not"), so the two differ only when auto-compact did not fire. Edge: exactly 500K warns in the hook (test pins it), "under 500K" in MP:15 (one-token difference, finder).
- **Ruling quotes:** none (V01: no ruling; V02: did not search for one; finder found none).
- **Tests:** test_stall.py:424-434 pin the literal "500K", 500000 and 499999, and "Context is 600,000 tokens." (copies of the text and number, not of the constant); none holds MP:15 equal to CTX_THRESHOLD.
- **Open:** "50% of the window" is stated nowhere either verifier could check; nothing in the live settings.json sets a window or auto-compact (F02, finder grep).
- **Hook gap:** none stated.

### L-F01-05: How long may the cleanup sweep hook run: its own git limit or its registered timeout?
- **Source:** I2-091; F01 S4; V01 S4.
- **Kind:** K2, K6 (a script limit against a registration).
- **Verdict:** CONFIRMED (V01).
- **Homes:**
  - G\cleanup_sweep.ps1:58 `$GitTimeoutSec = 600`, used at :125 in WaitForExit (V01).
  - G\settings.fragment.json:81 registers that script with "timeout": 30 (V01).
  - Contrast: session_cards_start.ps1 sets 25 to fit its registered 30 (see L-F01-20); cleanup_sweep.ps1 has no budget tied to its registered 30 (finder).
- **Splits:** a git call that runs past 30 seconds: the script allows 600 s, the registration allows 30 s.
- **Ruling quotes:** none.
- **Tests:** none holds them equal.
- **Open:** whether Claude Code ends the hook at 30 s is a runtime fact; only a run with a slow sweep settles it (V01).
- **Hook gap:** none stated.

### L-F01-06: Is the 1,000-character Bash limit, and the "git commit text does not count" exemption, a ruling of the user?
- **Source:** I1-072, I1-076; F01 S5; V01 S5.
- **Kind:** K8 (a ruling claimed with no Confirmed line), K1.
- **Verdict:** CONFIRMED-CHANGED (V01). The finder said code and README call the 1,000 and the exemption the user's. V01 read: code says "the user's" in both places; the README claims a ruling for the exemption only, not for the 1,000. So it is a ruling claimed with no Confirmed line, not two differing values.
- **Homes:**
  - G\bash_windows_rule.py:16 `MAX_CHARS = 1000  # the user's number` and :172 (and :172-174) "(the user's ruling)" on the commit exemption (V01).
  - G\README.md:27 "`MAX_CHARS` in that file is the one home of the number"; the exemption stands "because the user ruled on 2026-11-06 'Exempt git commit'" (V01 quote).
  - BP:33 "A command over 1,000 characters ... A `git commit`'s own text does not count toward that limit" (finder).
  - Hook message bash_windows_rule.py:211-214 uses MAX_CHARS (derived).
- **Splits:** none in value. Edge: exactly 1000 passes; "over" in both hook and BP (agree).
- **Ruling quotes:** review-and-hooks.md:40 ("Exempt git commit") and :43 ("length over 1000 and backslashes", "chose 1,000"), both under `## Unconfirmed` (heading :37; `## Confirmed` is line 7). No Confirmed line names either (V01).
- **Tests:** test_docs_numbers.py:192 (BP quotes the limit); test_bash_windows_rule.py:53 pins `MAX_CHARS, 1000` as a literal (second copy).
- **Open:** none.
- **Hook gap:** none stated.

### L-F01-07: What is "the at-once limit" that batch.py sizes a card wave to?
- **Source:** I2-117 (and I4-098); F01 S6; V01 S6.
- **Kind:** K8 (a wrong owner claim, or a limit named with no such limit).
- **Verdict:** CONFIRMED (V01).
- **Homes:**
  - SC\session_cards_start.ps1:277 "(batch.py sizes a wave to the at-once limit)" (V01).
  - SC\batch.py:23-26 `MAX_BATCH_BYTES` 200 KB, `MAX_BATCH_SESSIONS` 10, `BATCHES_PER_WAVE` 10 ("The plan's batch limits and wave size (user-approved with the plan)"); :70 `wave = i // BATCHES_PER_WAVE` (V01).
  - H AR:20 rule 14 "At most three builds run at once ... a build means compiling" (V01).
  - M\lean-agent-fanout.md "~8 jobs per agent, 3-4 agents per round" (finder; V01 did not re-search memory notes).
- **Splits:** a main session launching a wave reads "at-once limit" and finds 10 batches per wave in batch.py and 3 builds in rule 14; no agent-concurrency limit by that name exists in batch.py (V01).
- **Ruling quotes:** none.
- **Tests:** none holds the card-wave size to anything.
- **Open:** memory notes not re-searched by V01.
- **Hook gap:** none stated.

### L-F01-08: Which status rule did the user confirm: "every 10 tool calls, reject after 15", or "every 5 minutes, reject after 10 minutes"?
- **Source:** I1-016 neighbourhood; F01 (reported, not a split); V01 S7 (added by the verifier).
- **Kind:** K3 (two Confirmed lines in one file give different rules), K8.
- **Verdict:** CONFIRMED (V01's own addition; V01 read both lines).
- **Homes:**
  - H docs\agents\rulings\review-and-hooks.md:33 (Confirmed) "demand status every 10 tool calls and reject the next call after 15".
  - review-and-hooks.md:34 (Confirmed) "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes".
  - review-and-hooks.md:22 "minutes only".
  - progress.py follows :34 and :22 (code has no calls-based status rule).
- **Splits:** calls-based versus minutes-based status rule, both under `## Confirmed` in one file, sessions 48cdbb34 (:33, :34) and b5e8d95a (:22).
- **Ruling quotes:** the three lines above. V01: no Confirmed line is read as naming both sides.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none stated.

### L-F01-09: Which calls bypass the call budget and status deadline (`git ` commands, report tools)?
- **Source:** I1-025; F01.
- **Kind:** K5, K3 (edge only).
- **Verdict:** NOT VERIFIED (V01: "progress.py:119 not opened; the K3 edge on 'git' with no space is unchecked").
- **Homes (finder only):** G\progress.py:119 `exempt` = REPORT_TOOLS or `command.startswith("git ")` after lstrip (REPORT_TOOLS also lists StructuredOutput; "git" alone with no space is not exempt, F02). BP:21 "Two kinds of call still pass then: a command that starts with `git`, and your final report"; BP:73; G\README.md:124; progress.py:131 "Only your final report, git commands and your status line get through now."
- **Splits:** none stated; possible K3 only at the "git" no-space edge.
- **Ruling quotes:** none.
- **Tests:** test_progress.py (finder: not read in detail).
- **Open:** the K3 edge.
- **Hook gap:** none stated.

### L-F01-10: How much of a refused command is quoted back (QUOTE_CHARS 300)?
- **Source:** I1-022, I1-051; F01.
- **Kind:** K1 (weak: two code constants with the same value 300 and no link).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\progress.py:31 `QUOTE_CHARS = 300` (used at :128); G\common.py:46 truncates a logged reason to `[:300]`. They answer "how long is a quoted piece" for different outputs.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none holds them equal. **Open:** not reached in depth. **Hook gap:** none stated.

### L-F01-11: What sets the clock start for the time limit and the first status age?
- **Source:** I1-030; F01.
- **Kind:** K1 (30-minute clock).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\progress.py:147-152 and :87-90 (record created on the first call). BP:21, BP:73 "counted from your first tool call"; DOR:90 and DR:91 "the progress hook counts an agent's 30 minutes from its first call and never restarts the count." No prose says a status line sets the first status age to the first call.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none named. **Open:** not reached in depth. **Hook gap:** none stated.

### L-F01-12: How long does code wait for a lock (gates lock versus session-cards manifest lock)?
- **Source:** I1-054, I4-058, I4-126 (F02 Group G); F01, F02.
- **Kind:** K6 (two lock-wait helpers in two tool families), K1 (the 25 s copy).
- **Verdict:** NOT VERIFIED (V01 lists I1-054 and I4-058 as not reached).
- **Homes (finder only):** G\common.py:14,18 (100 x 0.01 s; 2.0 s / 0.001 s). SC\common.py:140-145 `LOCK_WAIT_SECONDS = 15`, `LOCK_STALE_SECONDS = 2 * 60`, 0.05 s; comment "15 s keeps the SessionStart hook inside its 25 s budget" (a copy of session_cards_start.ps1:49) and "The user's choices (2026-10-25)". Message text at common.py:207-208 is copied in a test.
- **Splits:** none by value; two owners for the same kind of question with different numbers (F01).
- **Ruling quotes:** none found. **Tests:** test_session_cards.py:1296 reads `common.LOCK_STALE_SECONDS` (derived); a test pins the message text. **Open:** not reached. **Hook gap:** none stated.

### L-F01-13: What is the Bash/PowerShell output cap (4000) and how much of a long result is kept (1500 + 1500)?
- **Source:** I1-060, I1-061; F01.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\shell_output.py:14 "The one home of these numbers and lists ... Docs point here." BP:35 "A result over 4,000 characters arrives as its start, its error lines, its end ..."; G\README.md:23 names `CAP` only; 1500 appears in no doc. At exactly 4000 the hook does not trim; BP says "over 4,000" (agree).
- **Splits:** none. **Ruling quotes:** none Confirmed (finder; the plan said the numbers were the user's answers). **Tests:** test_docs_numbers.py:198 holds BP to CAP; test_shell_output.py:143-203 pins 4000, 1500, 4001, 7008 as literals. **Open:** not reached. **Hook gap:** none stated.

### L-F01-14: What are the stall notice steps (10 / 20 / 30 minutes of silence), and who repeats them?
- **Source:** I2-001, I2-002, I2-003, I2-021, I2-023, I5-159; F01, F02 (I5-159); V01 (I2-001, I2-002 listed as not reached).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\stall.py:22-24 `STALL_SECONDS = 600`, `STALL_NOTICES = LIMIT_SECONDS // STALL_SECONDS`, :20,23 read progress.MINUTES_STOP. AR:27 "a stall notice when an agent has made no call for 10 minutes ... The notice repeats at 20 and 30 minutes of the same silence, and the third says it is the last. The step is STALL_SECONDS in `stall.py`..." G\README.md:29 (pointer only). test_stall.py:78-79 comments copy 600 and 30. At exactly 600 s no notice (`<=`).
- **Splits:** none. V02 notes (S4) the number 10 in `STATUS_DENY_MINUTES` (status age) and `STALL_SECONDS = 600` (silence) are two different questions with one number; not a split.
- **Ruling quotes:** none Confirmed. **Tests:** test_stall.py:148 and test_wake.py:118 pin the text "silent for 10 minutes"; none holds AR:27 equal to the code. **Open:** not reached. **Hook gap:** none stated.

### L-F01-15: stall.py's header names a plan as the source of its numbers; is that a rule's home?
- **Source:** stall.py:8-9 (F01 side note; K8); V01.
- **Kind:** K8.
- **Verdict:** CONFIRMED (V01, by reading) with an open point.
- **Homes:** G\stall.py:8-9 says the numbers "are the ones the plan sets (docs/superpowers/plans/2026-10-25-hooks-from-scratch.md, section 2)" and that the empty-fleet notice is "on the user's ruling of 2026-10-25". G\tests\test_progress.py:837 cites "Plan section 4" as the source of the one-home rule; the plan's line 108 says the numbers "sit at the top of progress.py and nowhere else in code; the docs quote them" (finder).
- **Splits:** none. **Ruling quotes:** none found for the 2026-10-25 ruling (V01 did not check it against a Confirmed line: open). **Tests:** none. **Open:** the claimed ruling of 2026-10-25 is unchecked. **Hook gap:** none stated.

### L-F01-16: How long may the wake hook run (660), and is it tied to the stall numbers?
- **Source:** I2-004, I2-062, I2-063, I2-064, I5-160; F01, F02; V01.
- **Kind:** K1 (held by a test).
- **Verdict:** CONFIRMED-CHANGED (V01 "PARTLY CONFIRMED": the 660 at settings.fragment.json:235 is confirmed; test_wake.py:218 not opened).
- **Homes:** G\stall.py:25 `SCAN_EVERY = 60`; wake.py:11-13 comment "The registered timeout is stall.STALL_SECONDS plus stall.SCAN_EVERY"; G\settings.fragment.json:235 `"timeout": 660` (V01); G\README.md:31 (pointer). wake.py:63-66 reads the constants. Edge (finder): wake's check-in at waited >= STALL_SECONDS against the agent notice at strictly more.
- **Splits:** none.
- **Ruling quotes:** none. **Tests:** test_wake.py:218 `hooks[0]["timeout"] == stall.STALL_SECONDS + stall.SCAN_EVERY` holds 660 equal (finder; not opened by V01). Live settings.json held to the fragment by test_docs_numbers.py:142-146, skipped in a worktree (F02). **Open:** none. **Hook gap:** none stated.

### L-F01-17: How many recent tool calls does the evidence line show (TOOL_TAIL 5)?
- **Source:** I2-010; F01.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V01 lists I2-010 as not reached).
- **Homes (finder only):** G\stall.py:34 `TOOL_TAIL = 5` ("display size"); BP:23 "(five is TOOL_TAIL in `stall.py`)" and "the names of your last five tool calls"; MP:11 "its last five tools". AR:27 describes the evidence line without the number.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none (test_docs_numbers does not read stall.py). **Open:** not reached. **Hook gap:** none stated.

### L-F01-18: How long must the fleet be silent before the empty-fleet notice (300 s)?
- **Source:** I2-013; F01.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\stall.py:39 `EMPTY_FLEET_SECONDS = 300`; FLEET_TEXT takes minutes from it (derived); G\README.md:29 "no agent has written for 5 minutes" (value copy). Same number 5 minutes as `STATUS_NAG_MINUTES`, a different question.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none holds README's 5 equal. **Open:** not reached. **Hook gap:** none stated.

### L-F01-19: What is the hook-context field cap (10,000) and who else states 10,000?
- **Source:** I2-076, I2-079, I5-161; F01, F02.
- **Kind:** K1 (the same number 10,000 for two different Claude Code caps).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\prompt.py:19-21 `FIELD_CAP = 10000` with the citation "code.claude.com/docs/en/hooks, 'capped at 10,000 characters'"; prompt.py:55-57 message built from the constant; G\README.md:33 names `FIELD_CAP` (pointer); G\tests\probe_results.md:127, :223 "About 10,000 characters" (a different measured cap: the failed-tool excerpt); test_posture.py:265-272 reads FIELD_CAP.
- **Splits:** none (different questions, same number). **Ruling quotes:** none. **Tests:** none holds probe_results equal. **Open:** not reached. **Hook gap:** none stated.

### L-F01-20: What are the session-cards start hook's numbers (claim 2 h, reminder 500 MB / 24 h, hook budget 25 s inside a registered 30 s)?
- **Source:** I2-103, I2-104, I2-105, I2-106; F01, F02 (Group G I4-126); V01.
- **Kind:** K1, K2 risk (25 against 30).
- **Verdict:** CONFIRMED (V01).
- **Homes:**
  - G\session_cards_start.ps1:43 `$ClaimHours = 2`, :45 `$RemovalReminderMb = 500`, :47 `$ReminderGapHours = 24`, :49 `$HookBudgetSec = 25` with the comment "The settings.fragment.json registration's 30 s timeout, less 5 s" (V01). Comments say "The user's choice (task SC-9, 2026-10-14)".
  - G\settings.fragment.json:93 registers session_cards_start with "timeout": 30 (V01).
  - ps1:338 "does not hold the work for 2 hours" (comment copy of 2); :46 "at most once a day" (word copy of 24) (finder).
  - SC\common.py:141 "15 s keeps the SessionStart hook inside its 25 s budget" (copy of the 25).
  - G\tests\test_h53_session_cards_start.ps1:212 `$took -lt 30` (a literal 30 in a test).
- **Splits:** the 25 is written in the ps1 and in SC\common.py; the 30 in the registration; the relation "30 less 5" is prose only (finder).
- **Ruling quotes:** "The user's choice (task SC-9, 2026-10-14)" is a code comment and not a Confirmed line (V01); finder grepped rulings for "SC-9", "claim", "reminder" and found none.
- **Tests:** none holds 25 and 30 equal (test_h53:212 only checks `< 30`).
- **Open:** none beyond the above.
- **Hook gap:** none stated.

### L-F01-21: What day did the ruling gate start (INSTALL_DATE 2026-11-14) and where else is it written?
- **Source:** I3-072; F01.
- **Kind:** K1, K8 (the user's words quoted in a README and a code comment, neither under `## Confirmed`).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\ruling_rules.py:97-99 (owner; comment quotes "the user's answer 'Yes, no click' (2026-11-13, session ca29fda5)"); H docs\agents\rulings\README.md:51 "the day the gate was installed (`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14)" and "The user approved it on 2026-11-13 ('Yes, no click')"; AR:25 and docs\sessions\index.md:503 mention 2026-11-14 for another thing.
- **Splits:** none. **Ruling quotes:** "Yes, no click" appears in README and in the comment; neither is a Confirmed line. **Tests:** test_ruling_rules.py:375 reads rr.INSTALL_DATE (derived); none holds README's date equal. **Open:** not reached. **Hook gap:** none stated.

### L-F01-22: What is the card line cap (60) and header shape, and who repeats it?
- **Source:** I4-024, I4-065, I4-113 (F02 Group G); F01, F02; V01.
- **Kind:** K1, K5 (writer-brief spells what check.py enforces).
- **Verdict:** NOT VERIFIED for the copy homes; the Confirmed ruling line itself is CONFIRMED by V01 (check.py and writer-brief:71 not re-opened).
- **Homes (finder only):** SC\check.py:28-30 `MAX_CARD_LINES = 60` ("The user raised it from the plan's 45 on 2026-10-14"); SC\writer-brief.md:71 "The whole card is at most 60 lines; aim for 20 to 40. A long session may use the room up to 60" and the header shape; H docs\agents\rulings\agents-and-workflow.md:39 (Unconfirmed note "Cap is 60 lines (user, 4f18f31)"). The card writer reads only writer-brief (card-writer.md:9 skips the preamble).
- **Splits:** none. Header shape not compared line by line (finder).
- **Ruling quotes:** agents-and-workflow.md:15 (Confirmed) "continue, and raise the cap to 60." (names the cap value, not the header shape; V01).
- **Tests:** test_session_cards.py:322 uses `check.MAX_CARD_LINES` (derived); no test holds writer-brief:71 equal.
- **Open:** header shape. **Hook gap:** none stated.

### L-F01-23: What is the running-session cutoff (2 hours) and where is it copied?
- **Source:** I4-080, I4-091, I4-115 (F02 Group G); F01, F02.
- **Kind:** K1 (literal 7200 in a test).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** SC\prepass.py:30-31 `RUNNING_CUTOFF_SECONDS = 2 * 3600` ("Plan decision 6 (user-approved)"); prune_list.py:20 comment "prepass is owner of the running-session cutoff", :114 reads it; test_session_cards.py:1007-1012 literals `OLD + 7200 - 1` (copy). A different "2 hours" with a different meaning: session_cards_start.ps1:43 `$ClaimHours = 2`.
- **Splits:** none. **Ruling quotes:** none Confirmed. **Tests:** test_session_cards.py literal 7200 (copy). **Open:** not reached. **Hook gap:** none stated.

### L-F01-24: How are card batches and waves named and sized, and who reads the name?
- **Source:** I4-098, I4-099; F01.
- **Kind:** K1/K6 (two code places know the file-name format).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** SC\batch.py:23-26, :71-73 (batch name shape and line shape `<session id> TAB <condensed file path> TAB <KB>`); writer-brief.md:26 states the same line shape; session_cards_start.ps1:257-262 reads batch names by pattern `wave-(\d+)-batch-(\d+)\.txt`. The "at-once limit" sentence is in L-F01-07.
- **Splits:** none. **Ruling quotes:** none ("user-approved with the plan" is a code comment). **Tests:** none found holding the ps1 regex equal to batch.py's format (finder grep not confirmed). **Open:** not reached. **Hook gap:** none stated.

### L-F01-25: Does the writer-brief copy of the status call equal what the hook accepts?
- **Source:** I4-035; F01.
- **Kind:** K1, K5, K3 (see L-F01-02 for the action at 20/100).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** SC\writer-brief.md:99-101 (status line `"HH:MM done <n> of <m> cards | next: <session>"` and the call path `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`); G\progress.py:22 `STATUS_PREFIX` and :58 `_helper_line`; BP:13; DOR:9, DR:9 carry the same call text.
- **Splits:** none (form agrees). **Ruling quotes:** none. **Tests:** test_docs_numbers.py:189 holds only the preamble's copy to STATUS_PREFIX, not writer-brief's. **Open:** not reached. **Hook gap:** none stated.

### L-F01-26: The status-line call shape is checked, but is its text?
- **Source:** F01 side note ("Hook gap (general words)").
- **Kind:** K7 (hook's job in prose).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\progress.py `is_status_call` / `_STATUS_CALL`: the call shape is checked only for the exact helper path; a status line is accepted whatever it says (finder wording). BP:15-19 says what the line should say.
- **Splits:** none. **Ruling quotes:** none (the related Confirmed lines are agents-and-workflow.md:25-26, "no status updates is unacceptable", "the status updates should be brief but relayed in plain english"). **Tests:** none named. **Open:** none.
- **Hook gap:** hook gap: yes, G\progress.py.

---------------------------------------------------------------------
## Finder F02 rows
---------------------------------------------------------------------

### L-F02-01: Is the status line written by "append one line to the status file" (DOR/DR) or only by the helper (BP)?
- **Source:** Group A; F02.
- **Kind:** K1/K3 (words only).
- **Verdict:** NOT VERIFIED (V02 checked the budget copies in candidate 1 but did not reach this wording).
- **Homes (finder only):** DOR:9 and DR:9 "Append one line to <status path>" then the helper call, "alone in its own call"; BP:15 "Do not write the file with Edit, a heredoc or `>>`" (the helper is the only accepted way). Also the status-line shape in progress.py `_STATUS_CALL`, BP:15-19, DOR:9/DR:9 and writer-brief:100 (`<n> of <m> cards` variant: same call shape, different text).
- **Splits:** none stated as a contradiction; the finder says a reader of DOR:9 alone could append directly.
- **Ruling quotes:** none Confirmed on the shape (finder). **Tests:** test_docs_numbers.py:116-120, :182-190 for the five docs only. **Open:** not reached. **Hook gap:** none stated.

### L-F02-02: A branch's doc edit is checked against progress.py only after merge (docs test reads the main checkout).
- **Source:** F02 Group A tests note and side note 6.
- **Kind:** K7 (a check that does not see a branch).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\tests\test_docs_numbers.py:22,69 reads the docs from env GATES_HYDRA_ROOT or the main checkout; the live-settings test (:142-146) is skipped in a worktree.
- **Splits:** none. **Ruling quotes:** none. **Tests:** the test itself. **Open:** only a run would show it. **Hook gap:** none stated.

### L-F02-03: Rule 4 "fail loudly": who states it and what checks it?
- **Source:** I5-040; F02 Group C.
- **Kind:** K1 (CLAUDE.md and AR:10 identical), K5, K7 (the "fail loudly" half has no hook).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H CLAUDE.md:66 and AR:10 (identical sentence "Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke."); BP:109-111 "When blocked ... stop and report ... Never wait on a prompt, never retry the same call hoping it passes"; AR:27 the stall notice "which helps with rule 4". tools\capture_window.ps1:13 and tools\test_effort_test_guard.ps1:110 use "fails loudly" for other things.
- **Splits:** none.
- **Ruling quotes:** agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (session a9a461da). **Tests:** none holds CLAUDE.md:66 equal to AR:10. **Open:** not reached. **Hook gap:** none stated.

### L-F02-04: Who states "every step that repeats or waits needs a cap", and what checks it?
- **Source:** I5-088, I6-129, I7-084 part; F02 Group E; V02 candidate 7.
- **Kind:** K7, K1.
- **Verdict:** CONFIRMED (V02).
- **Homes:** BP:79-85 "Any step that repeats or waits needs a cap, in runs or minutes, and a line saying what to report when the cap is hit"; AM:40 "Every stage that repeats needs a cap and a line saying what happens at the cap"; CH:37 "Every wait has a cap ... No tool retries on its own." (V02 read all three). Also BP:85, PM:61-68, CH:43 (finder). Code caps exist: bot_run.py:88-90, engine_finder.py:38, frida_tools.py:46-49, prep_inputs.py:50, trace_hits.py:70-74 (finder).
- **Splits:** none (same rule, different scope; BP is read by agents only, AM by the main session planning an audit, card writers by writer-brief).
- **Ruling quotes:** clone-hero.md:14 "Start them, 5 s cap (Recommended)" and :25 "Hydra's end of the chart (Recommended)"; agents-and-workflow.md:15 "continue, and raise the cap to 60." None states the general rule.
- **Tests:** none holds BP:81 or CH:37 (V02: no test or hook).
- **Open:** V02's grep for "retry|retries" in tools\ch_mcp\*.py (non-test) found no hit, consistent with CH:37 but a name search proves little.
- **Hook gap:** none stated (progress hook limits an agent's total only).

### L-F02-05: Does anything stop a Clone Hero tool being given the whole song library?
- **Source:** I7-084; F02 Group J.
- **Kind:** K5, K7, K1.
- **Verdict:** CONFIRMED (V02; the library-root check is open).
- **Homes:** CH:39 "Nothing runs over the user's song library. Each tool takes one song folder or one chart folder." No refusal of a library-root folder found in tools\ch_mcp\*.py (V02 grep "library" found only unrelated lines). The whole-library hook reads shell text only: G\percall.py:14 `LIBRARY_FLAG = "--whole-library"`, G\review_rules.py:50-52 `LIBRARY_ALLOW_TEXT = "allow library run"` (finder; V02 did not re-open). Prose copies: CLAUDE.md rule 8, AR:14, BP:49, M\no-full-library-runs.md:11, AM:125, this-machine.md:67, UT:99 (finder).
- **Splits:** none.
- **Ruling quotes:** tests-and-runs.md:36 is in an Unconfirmed-style note ("I did not approve a full library test..."); the Confirmed section of tests-and-runs.md was not opened (finder), so no ruling line is quoted.
- **Tests:** none holds the prose to the hook.
- **Open:** a run with a library root as a folder argument would show it (V02).
- **Hook gap:** hook gap: yes, G\percall.py (the whole-library hook reads shell text only; CH tools take a folder argument it does not read).

### L-F02-06: How many charts does the test corpus (testdata/input) hold, and which pages say so?
- **Source:** I7-012, I7-126, I7-129, I7-130, I8-115 (F03 Group 1); F02 candidate 2, F03 candidate 1; V02 candidate 2, V03 candidate 1.
- **Kind:** K2, K1, K6 (tool test types its own 98).
- **Verdict:** CONFIRMED-CHANGED. Two readings. F02's claim (UT's 98 has drifted from 102 folders) is REFUTED by V02 as a drift: 102 counts folders holding any .chart/.mid/.sng; the files a library scan takes (notes.chart, notes.mid, *.sng) are 98, and tests pin 98 (tests\test_engine_sweeps.cpp:656 `CHECK(charts == 98)`, tests\ui\uitest_library.cpp:550 "98 charts", uitest_batch_reports.cpp:389 "Analyze 98 charts"). F03's claim (memory notes say 115 and 97 against 98) is CONFIRMED by V03 (98 song.ini found under H\testdata\input). The verifiers' readings win. What stays is K1.
- **Homes:**
  - H docs\agents\ui-testing.md:46, :48 "song folder = `testdata/input`, 98 charts".
  - TR:11 "A 98th chart was added ... tests still expected 97" (story); TE:101 `CHECK(charts == 98);`, TE:113 `IM_CHECK_EQ(shown(), (size_t)98);` (bad examples).
  - H tools\test_hydra_ui_mcp.py:774, :1070 `assertIn("charts=98", ...)` (V03); H tools\mutate\test_run.py:65 "discovered 98 chart(s)" (finder).
  - M\no-full-library-runs.md:11 "`testdata/input` (about 115 charts)" (V03).
  - M\questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix" (V03).
- **Splits:** (V03) "about 115" (no-full-library-runs.md:11) against 98 (ui-testing.md:48, file count). (V03) "[N/97]" (questions-describe-the-real-rule.md:24) against 98. Situation: the main session reads the notes; a tool test in a worktree with an added chart would fail on its typed 98 (not run-checked).
- **Ruling quotes:** none found (V03; finders).
- **Tests:** none in G\tests; the C++ tests pin 98; the tool test holds the GUI count to the typed 98, not to the folder.
- **Open:** which file names the loader accepts (V02 did not read the loader); a run would settle the worktree failure.
- **Hook gap:** none stated.

### L-F02-07: How long does the "machine is quiet" check take ("about 6 s")?
- **Source:** I7-069; F02 candidate 3 (Group J); V02.
- **Kind:** K1, K2 risk.
- **Verdict:** CONFIRMED-CHANGED (V02): a copied value with a hand-added second, not a proven drift.
- **Homes:** tools\ch_mcp\quiet.py:29-31 `QUIET_CPU_PCT = 30`, `QUIET_SAMPLE_S = 5` (used at :197), `BUILD_RECENT_S = 60`; tools\ch_mcp\server.py:128 and :158 ("ch_start checks the machine is quiet (about 6 s, and it can refuse)"); CH:15 "adds about 6 s; `tools/ch_mcp/quiet.py` owns the check". None is built from the constant.
- **Splits:** 5 s (constant) against "about 6 s" (three text copies). V02: only a run of ch_start timing would show whether 6 is wrong.
- **Ruling quotes:** none. **Tests:** none found (V02 looked at the test_game_tools hit by name only). **Open:** a timing run. **Hook gap:** none stated.

### L-F02-08: Why does ch_sp_compare ask the agent for the chart's Resolution when the schedule names its chart?
- **Source:** I7-088; F02 candidate 4; V02.
- **Kind:** K4.
- **Verdict:** CONFIRMED (V02).
- **Homes:** tools\ch_mcp\sp_tools.py:317, :330-332 refuses with "compare refused: give resolution, the chart's Resolution from its [Song] section"; tools\ch_mcp\game_tools.py:470-487 `schedule_chart_folder` returns the folder of `doc["chart"]` (refuses a relative or missing path); no `resolution` in tools\replay_schedule.cpp (V02 grep). A .mid chart keeps resolution in its header, not [Song], which the refusal text does not say (V02). F02 also cites server.py:557 and the docstring "which no file here holds".
- **Splits:** a schedule with an absolute chart path (a tool could read the Resolution) against a relative or missing chart path (cannot).
- **Ruling quotes:** none. **Tests:** none named. **Open:** none beyond reading. **Hook gap:** none stated.

### L-F02-09: Where do the picture size limits (500 KB per file, 4 MB per folder) live?
- **Source:** I6-249, I7-224; F02 candidate 5, F03 and F03b; V02 candidate 5, V03b.
- **Kind:** K6, K8 (weak).
- **Verdict:** CONFIRMED (V02, V03b).
- **Homes:**
  - H tools\doc_shots\shrink.py:19-21 `MAX_FILE_BYTES = 500 * 1024`, `MAX_FOLDER_BYTES = 4 * 1024 * 1024`; comment "tests/test_docs_match_code.cpp checks the same two at merge."
  - H tests\test_docs_match_code.cpp:2024-2025 `kDocImageMaxBytes = 500 * 1024`, `kDocImagesMaxTotalBytes = 4 * 1024 * 1024`; :2022 names shrink.py (V02), :2021 cites D300 (V03b).
  - H docs\agents\doc-screenshots.md:17 "the constants at the top of shrink.py hold them" (V02: names shrink.py, which does hold them, so K8 is weak; the second owner is the point).
  - D300 in H docs\audit\2026-10-03-fix-decisions.md:1662 names both pairs.
- **Splits:** none in value today. V03b did not see the finder's wording ("shrink.py fails a capture run on the same limits") in the C++ file; it saw D300 instead.
- **Ruling quotes:** none Confirmed named (comment "The user's limits (D300 ...)" only).
- **Tests:** none holds the two equal (neither side reads the other; V03b grep).
- **Open:** none. **Hook gap:** none stated.

### L-F02-10: Star Power end lead (15 ms before an SP end): who states it?
- **Source:** I6-030; F02 Group H.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H tools\replay_schedule.cpp:40 `constexpr double kSpEndLeadMs = 15.0;` (comment "The user set it: question 3 of the optimal-score run plan, which has left the tree"), exported as `sp_end_lead_ms` (:70); DOR:108 "The schedule presses 15 ms before a Star Power end (`kSpEndLeadMs` ...), and no game rule sets that margin. If it had no ruling..." (example sentence, a value copy); tools\ch_probe\tests\test_play_chart.py:1621,1731,1737,1804 pin 15.0; tools\ch_mcp\tests\test_game_tools.py:120-122,3223,3240 ("15 ms before the end"); tests\test_replay_schedule.cpp:721.
- **Splits:** none. **Ruling quotes:** none under `## Confirmed` (finder grepped rulings for "15 ms" and "kSpEndLeadMs"); DOR:108 itself says "If it had no ruling". **Tests:** none holds DOR:108's 15 equal. **Open:** not reached. **Hook gap:** none stated.

### L-F02-11: What is "one frame" (3.00 ms) and how does it relate to the 3 ms backend leeway?
- **Source:** I6-031; F02 Group H.
- **Kind:** K1 (note: the number 3 is two different rules).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H tools\replay_schedule.cpp:26 `kFrameMs = 3.0` (comment "The user set it: question 1 of ...player-freak-fixes.md", "the user chose "One frame", D319 item 3"), :31 `kJudgeEarlyMs = 1 * kFrameMs`; DOR:64 "3.00 ms is the one frame of judging the tool counts" (copy); AM:13, :135 "3" in the backend ladder (the leeway default, a different question).
- **Splits:** none (two different rules with the same number: tool frame 3.0, engine leeway `Rules::backend_leeway_ms`).
- **Ruling quotes:** none for the frame. code-and-engine.md:12-13 (Confirmed) "the 3ms rule is for whether a backend is included in the optimal score or uncounted. it's an arbitrary number chosen by dragondelgar" speaks to the leeway, not the frame. **Tests:** none. **Open:** not reached. **Hook gap:** none stated.

### L-F02-12: What is the "large range" threshold (400 lines) and the Confirmed "over 20 lines" review ruling; is either applied?
- **Source:** I6-055; F02 Group H and side notes 1, 2.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H tools\precheck\items.h:50 `constexpr long long kLargeRangeLines = 400;` ("The user's decision is D82 item 2 ... a starting value to be tuned later. This is the one place it lives."); DOR:193 states no number (pointer); tests\test_precheck_git.cpp:202 pins the text "401 changed lines is over the 400-line threshold" literally (test_precheck.cpp:417 uses the constant). No gate found applying a 20-line threshold (finder searched ref_judge.py, review_rules.py, review_dispatch.py).
- **Splits:** none (400 and 20 answer different questions).
- **Ruling quotes:** review-and-hooks.md:22 (Confirmed) "only large items over 20 lines should go for review" (quoted in L-F01-01); :50 (Unconfirmed) ">20 lines". Neither names a split.
- **Tests:** test_precheck_git.cpp:202 literal; test_precheck.cpp:417 constant. **Open:** not traced further. **Hook gap:** none stated.

### L-F02-13: "Five or fewer questions per message": whose rule is it?
- **Source:** I6-141; F02 Group H.
- **Kind:** K1, K4 (the main session's own number shown as settled).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** AM:104 ("five or fewer per message is the main session's own suggestion"); docs\superpowers\plans\2026-11-14-test-suite-audit.md:194; 2026-11-08-clone-hero-docs-gaps-search.md:91,187; 2026-11-08-player-audit.md:102; docs\audit\2026-11-08-player-audit\questions.md:3 ("five or fewer at a time"). Two plans show it as settled ("Questions go out five or fewer per message").
- **Splits:** none.
- **Ruling quotes:** questions-and-audits.md:28 is in the Unconfirmed section ("yes both" as the note records it); not a Confirmed line. **Tests:** none. **Open:** not reached. **Hook gap:** none stated.

### L-F02-14: The "at most one (never tried) call per step" rule: prose and test.
- **Source:** I6-179; F02 Group H.
- **Kind:** K1/K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** H docs\agents\premise-map.md:68 "A step holds at most one `(never tried)` call. A step with two or more is a shape error"; H tests\test_docs_match_code.cpp:1850 `if (never_tried >= 2)` with the message "(never tried) calls; split it so each step tries one new piece"; plans 2026-11-07-ch-run-timing.md:413, :463; CH:58.
- **Splits:** none. **Ruling quotes:** none. **Tests:** test_docs_match_code.cpp:1850, :1945-1946 (the enforcement; PM does not name it). **Open:** not reached. **Hook gap:** none stated.

### L-F02-15: M39 numbers and history stated twice in the audit-method page.
- **Source:** I6-146, I6-147; F02 Group H.
- **Kind:** K1 (history, not a live rule).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** AM:13 and AM:135 ("-10 ms to under 3 ms", "-10 ms to under 10 ms", `kBackendInnerBandMs`, commit c9c00cf7); the finder says `kBackendInnerBandMs` no longer exists in src\ and the code has `Rules::backend_leeway_ms` (src\core\backend_value.h:37-56); AM:190 points to `BackendSqueeze::summarystr` in src\core\model.cpp:342 (exists). Counts of past audits at AM:140-178 ("33 questions", "219 closures", "one in nine", "44 more", "26, 14, 11 and 12") repeat in docs\audit\2026-10-29-audit-of-audits\blocks\AOA-*.md, 2026-10-03-fix-decisions.md and others.
- **Splits:** none.
- **Ruling quotes:** code-and-engine.md:12 "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted..." and :13; AM:135 "3 is the default of the leeway setting" agrees. **Tests:** none. **Open:** not reached. **Hook gap:** none stated.

### L-F02-16: Two source lists behind the public doc style rules (nine versus ten sources; two web searches).
- **Source:** I6-256, I7-189 (F03b); F02, F03b.
- **Kind:** K1/K7 (weak).
- **Verdict:** NOT VERIFIED (V03b did not reopen I7-189).
- **Homes (finder only):** H docs\agents\public-doc-style.md:7 "two web searches on 2026-10-10" and ten linked sources (the row says nine; F02 counted ten); H docs\research\ai-generated-docs-review.md (2026-10-08, sources [S1]..[S27]), which DR:11 names as "where this method comes from". H docs\clone-hero\06-scoring.md:127 has an unrelated "two web searches" (2026-11-11), same words, different fact (F03b); M\search-web-before-finalizing-plans.md is a related rule, not a copy.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none. **Open:** not reopened. **Hook gap:** none stated.

### L-F02-17: How long does the open/analysis screen wait before showing "Analyzing chart... / Cancel" (0.15 s), and what else is 0.15 s?
- **Source:** I7-025, I7-026 (F02 Group I); I8-012 (F03b); F02, F03b; V03b.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V03b, for I8-012 and the code lines; V02 did not reach Group I).
- **Homes:**
  - H src\ui\app_state.h:434 `static constexpr double kViewProgressDelaySeconds = 0.15;` (V03b); the comment at :298 and details_panel.cpp:200 (finder).
  - app_state.h:445 `kSearchThrottleSeconds = 0.15` (a different question, same number; V03b agrees).
  - UT:124 "once it has run 0.15 s" (a value copy; V03b saw the hit, did not read the line); UT:132 names the constant (pointer).
  - M\approved-work-is-not-reasked.md:11 "about 130 ms (under the 0.15 s screen delay)".
  - H docs\agents\rulings\ui-and-display.md:97 (Confirmed, F03b) the user chose "Yes, counted bar" to a question quoting "takes over 0.15 s to open".
  - H tests\test_app_click.cpp:1067 `CHECK(AppState::kViewProgressDelaySeconds == 0.15);  // D87 item 6`; tests\ui\uitest_details.cpp:1209 reads the constant; tests\single_owner\rules\ui.json:1517-1535 registers it as an owner (finder).
- **Splits:** none on the delay itself. (The T7 timing split is L-F03-13.)
- **Ruling quotes:** ui-and-display.md:97 (above); "D87 item 6" is a decision-log entry, not searched in full (finder). **Tests:** test_app_click.cpp:1067 pins the value. **Open:** none. **Hook gap:** none stated.

### L-F02-18: Clone Hero MCP page copies (gap over 50 ms, engine offset +0x1B8).
- **Source:** I7-088; F02 Group J.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** CH:48 "gaps over 50 ms between samples" (owner tools\ch_mcp\sp_tools.py:62 `GAP_MS = 50`, the output line built from the constant at :500); "`sp_half_u64`, the game's field at engine `+0x1B8`" (owner tools\ch_probe\constants.py:128 `OFF_SP_HALF = 0x1B8`; also H docs\clone-hero\04-fills.md:214, 05-star-power.md:107,169, 07-internals.md:309-312, where a game fact belongs); "`sp_tools.ready_lines` owns the line" (sp_tools.py:329,355 true).
- **Splits:** none. **Ruling quotes:** none. **Tests:** none holds CH:48's 50 to GAP_MS. **Open:** not reached. **Hook gap:** none stated.

### L-F02-19: Trace and wait caps (120 s twice, 30 s margin) and their pointers.
- **Source:** I7-094, I7-086, I7-100, I7-070; F02 Group J.
- **Kind:** K1 (test comments copy the arithmetic).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** CH:55 names `trace_hits.TRACE_CAP_MARGIN_S` (tools\ch_mcp\trace_hits.py:70 `= 30.0`, D331) and `prep_inputs.REPLAY_CAP_S` (tools\ch_probe\prep_inputs.py:50 `= 120.0`, "The user chose 120 s ... 2026-11-07, session d0a3868c"); trace_hits.py:74 `WALL_CAP_PAUSE_S = 120.0` ("approved by the user on 2026-10-28"); the two 120.0 values answer different questions. test_trace_hits.py:180-181, 462-463 copy "6,000 ms + 30 s margin = 36 s song cap; plus 120 s of pause room = 156 s". CH:45 names bot_run.py:88-90 `LAUNCH_ATTACH_CAP_S = 20.0`, `LAUNCH_ARM_CAP_S = 5.0`, `LAUNCH_PATCH_CAP_S = 30.0` (no numbers in CH); CH:68 names `engine_finder.ENGINE_SEARCH_CAP_S` (engine_finder.py:38 `= 120.0`).
- **Splits:** none.
- **Ruling quotes:** clone-hero.md:25 (Confirmed) chose "Hydra's end of the chart (Recommended)" for the trace cap. **Tests:** test comments only. **Open:** not reached. **Hook gap:** none stated.

### L-F02-20: Settings reload: one claim in four places.
- **Source:** I5-183; F02 Group B.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** G\README.md:118 "An edit to a watched settings file reaches every running session on its next call"; README.md:190 "reaches a running session on its next call; an edit to a `--settings` file does not"; G\tests\probe_results.md:30 and :51 (evidence).
- **Splits:** none (all agree). **Ruling quotes:** none. **Tests:** none. **Open:** not reached. **Hook gap:** none stated.

---------------------------------------------------------------------
## Finder F03 rows (memory notes, skills, test pages)
---------------------------------------------------------------------

### L-F03-01: Early-fill reach: one setting, or a fixed 60 ms slack plus an 85 ms orange reach?
- **Source:** I8-059, I8-060, I8-089, I8-194, I8-088, I7-134; F03 Group 3; V03 candidate 2.
- **Kind:** K8, K2.
- **Verdict:** CONFIRMED (V03).
- **Homes:**
  - H src\core\model.h:101-106 `kHitWindowMs = 85.0` ("also the widest early-fill window the setting allows"); :108-112 `kDefaultEarlyFillWindowMs = 60.0` ("Every default of the setting (Settings, SearchSettings, the search graph, Activation, HydraRecord) is this one constant."); :115 `kMaxEarlyFillWindowMs`; :338 `fill_refuses(double e_offset, double window_ms)` (V03). `kEarlyFillSlackMs` has no hits in H\src or H\tests (V03 grep).
  - M\early-fill-orange-85-d250.md:3 and :11 say the reach is kHitWindowMs read by fill_refuses (orange) and the slack is the fixed `kEarlyFillSlackMs` (60 ms) read by is_e0. V03 did not read is_e0's body.
  - M\MEMORY.md:22 "reach kHitWindowMs 85 / slack 60".
  - ~/.claude\skills\analyzing-sp-squeezes\SKILL.md:18 "W=85", :24 "`85*(1+r)`" (no owner named); H tools\ch_probe\README.md:101 and SPEC.md:302 "`kHitWindowMs = 85.0`"; TE:179 "the 85 ms hit window" (finder).
  - An old plan names `kEarlyFillSlackMs`: H docs\superpowers\plans\2026-11-12-early-fill-live-check.md:253,296 (finder).
- **Splits:** the note says orange reads kHitWindowMs and the white slack is a fixed constant of 60; model.h says there is one early-fill setting, default 60 and up to 85, on both sides. Situation: a session reading the note to change early-fill behaviour.
- **Ruling quotes (H docs\agents\rulings\code-and-engine.md, Confirmed from line 7):** :23 "change the early fill search to allow orange timing up to 85ms."; :24 chose "Orange only (Recommended)" to "Which side of the early-fill window should go to 85 ms?"; :49 "the hit window is +/- 85ms, the 70ms is an outdated figure. we can change that."; :57 (2026-11-15) "analysis settings panel, default 60ms, supports 0ms to 85ms". clone-hero.md:54 chose "85 ms both sides (Recommended)" (Confirmed, finder). V03: the later :57 line is the one model.h quotes; :24 and :57 describe different designs; neither line names both sides of the note-versus-code split, so the ruling does not close it.
- **Tests:** none in G\tests; q3.md:77 (docs\audit\2026-10-31-stale-comments) mentions tests/test_model.cpp rows for both names (finder). **Open:** is_e0's current body not read. **Hook gap:** none stated.

### L-F03-02: Where is kSqueezeWindowMs (the 500 ms squeeze reach), and who calls the one check?
- **Source:** I8-161, I8-191, I8-163 neighbour; F03 Group 4, F03b; V03 candidate 3, V03b.
- **Kind:** K8, K1, K6.
- **Verdict:** CONFIRMED (V03, V03b).
- **Homes:**
  - Owner: H src\core\model.h:117-121 `constexpr double kSqueezeWindowMs = 500.0;`; :129-135 `within_squeeze_window`, "The one window check: the search graph, the engine's tail rows, the stored rows ... and hydra_replay all ask it."
  - Callers (V03): src\search\graph.cpp:160, :329; src\search\engine.cpp:2044; src\core\model.cpp:845; src\core\sqout_chord.h:32, :123.
  - ~/.claude\skills\analyzing-sp-squeezes\SKILL.md:10 "(`kSqueezeWindowMs`, src/search/graph.h)" (V03 read; graph.h:216 only mentions the name in a comment; V03b: graph.h:222 mentions `within_squeeze_window` in a comment).
  - C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:34 `HYDRA_WINDOW_MS = 500.0     # kSqueezeWindowMs, src/search/graph.h`; :33 `HIT_WINDOW_MS = 85.0        # kDefaultHitWindowMs, src/core/model.h` (finder F03b; V03b confirmed :34 and the 500.0 / 85.0 copies).
  - M\sp-squeeze-mechanics.md:13-15 "kSqueezeWindowMs = 500 ms of the deactivation node (src/core/model.h; `within_squeeze_window` is the one check, used by src/search/graph.cpp)"; M\MEMORY.md:38 "within 500 ms"; M\min-path-delta-proof.md:27 "500ms+ clearance around deacts" (a different use of 500).
- **Splits:** (1) skill SKILL.md:10 and squeeze_scout.py:34 say src/search/graph.h; the owner and the memory note say src/core/model.h. (2) the note's "used by src/search/graph.cpp" narrows the callers (five caller sites listed above). (3) squeeze_scout.py holds its own 500.0 and 85.0 outside the Hydra repo (V03b split 3).
- **Ruling quotes:** none found for 500 ms.
- **Tests:** none in G\tests.
- **Open:** F03b: replay.cpp:26-33 `core::squeeze_window_phrases` not checked to call `within_squeeze_window`.
- **Hook gap:** none stated.

### L-F03-03: Copies in memory notes of the status interval, the 3 ms leeway, and max tied paths.
- **Source:** I8-066, I8-123, I8-158, I8-063; F03 Groups 5 and 6; V03 candidate 4.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED as K1 only, no drift found (V03). V03 re-read only the status note (M\every-brief-carries-status-rule.md:15 "every 5 minutes", agrees); the rules.h owner lines were not re-opened by V03 (finder's word).
- **Homes:** G\progress.py:14-16 (see L-F01-01); H src\core\rules.h:32 `double backend_leeway_ms = 3.0;`, :38 `int max_tied_paths = 4;` (finder); M\sp-end-is-one-point-leeway-is-delgars.md:11 quotes "3ms"; M\MEMORY.md:7 "3 ms"; M\engine-tie-merge-prune.md:14 "`Rules::max_tied_paths` (src/core/rules.h, default 4, ...)"; M\every-brief-carries-status-rule.md:15; M\MEMORY.md:45.
- **Splits:** none.
- **Ruling quotes:** code-and-engine.md:12-13 (see L-F02-15) speak to the leeway; F03 did not look up the line. agents-and-workflow.md:25-26 (see L-F01-26).
- **Tests:** G\tests\test_docs_numbers.py covers five Hydra docs and not the memory notes (finder).
- **Open:** not re-opened by V03. **Hook gap:** none stated.

### L-F03-04: Fan-out numbers in memory notes ("about 8 jobs per agent", "3-4 agents per round").
- **Source:** I8-008, I8-090, I8-091; F03 Group 8; V03 candidate 4 (accepted only as "K7 candidate, unchecked by me").
- **Kind:** K7, K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** M\lean-agent-fanout.md:3, :20 "Batch small uniform jobs (merges, rewrites) about 8 per agent.", :21 "use 3-4 agents ... More agents per round makes 'stop when a round finds nothing' harder to reach"; M\MEMORY.md:39. No hook or tool in G or H\tools reads these numbers (finder searched G\*.py, G\*.md, H docs\agents).
- **Splits:** none.
- **Ruling quotes:** none Confirmed. The nearest user words are at agents-and-workflow.md:48, under `## Unconfirmed` (from line 32): "do we really need 54 merge agents?" and "was running 8 agents per round really necessary?" (a home, not a ruling; does not carry the note's numbers). **Tests:** none. **Open:** not re-opened. **Hook gap:** none stated.

### L-F03-05: The note names a hook message ("say what changed since the last run and why this one will end differently"); does any hook carry it?
- **Source:** I8-097; F03 Group 13; V03 candidate 5.
- **Kind:** K8.
- **Verdict:** CONFIRMED (V03).
- **Homes:** M\measure-small-explain-first.md:11 "A hook also flagged the fifth bench_run launch: "say what changed since the last run and why this one will end differently"." V03 searched G\*.py, G\*.md, G\*.json for "differently" and "since the last run": the only hit is an unrelated line, G\tests\test_agent_stop.py:352 ("fixed differently"). The finder also searched "what changed since", "will end differently", "bench_run" in G.
- **Splits:** a note quotes a hook; no hook carries that text today.
- **Ruling quotes:** none. **Tests:** none. **Open:** whether the hook was retired is unknown (V03). **Hook gap:** none stated.

### L-F03-06: Does a gate deny briefs that lack the status sentence?
- **Source:** I8-066 (note); F03 Group 7; V03 candidate 6.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED (V03; V03 saw :15 start but did not re-quote the "No gate" sentence).
- **Homes:** M\every-brief-carries-status-rule.md:3 "the dispatch gate now denies briefs that lack it." M\every-brief-carries-status-rule.md:15 "No gate checks a brief for this sentence today, so the brief has to carry it." G\review_dispatch.py:1 "a reviewer dispatch writes the handover record, or is refused"; V03's grep for "status" found no text in it. G\tests\test_docs_numbers.py:31 lists `reviewer_dispatch_gate` as a retired hook word (finder). The finder's side note: a retired hook name is still in the note's description as if live.
- **Splits:** note :3 (a dispatch gate denies briefs) against note :15 (no gate checks).
- **Ruling quotes:** none. **Tests:** none. **Open:** the retired-name point was not re-checked by V03. **Hook gap:** none stated.

### L-F03-07: How many builds may run at once, and where is the number typed?
- **Source:** I8-119, I8-121; F03 Group 9; V03 candidate 7.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V03).
- **Homes:**
  - H tools\build_slot.ps1:71-75 "How many builds may run at once ... Decision D49 ... the user's limit of three is in docs/agents/rulings/agents-and-workflow.md." `$BuildSlotCount = 3`, used at :152, :197, :199, :201 (V03).
  - H tools\test_build_pass.ps1:115 `(Get-Locks).Count -eq 3`, :117 `'all 3 build slots are taken'`, :141 `Want = 3`, :199/:207 `Write-Lock 3`; the test never reads `$BuildSlotCount` (V03 grep).
  - H CLAUDE.md:76 rule 14 "At most three builds run at once"; BP:57; AR:20 (finder).
  - M\one-build-per-wave.md:13 quote "3 builds at once? what? i didnt accept doing lots of builds"; M\archive\parallel-by-default.md:25 (finder). "One combined build per wave" has one home, M\one-build-per-wave.md:11, description :3.
- **Splits:** none today. V03: the test and the owner agree; changing the owner breaks the test. The finder: the note says a wave gets one build, the ruling says one build per session at a time (different questions).
- **Ruling quotes:** agents-and-workflow.md:8 (Confirmed) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." and :9 "any tests that require a build are done by the main session, not agents." (V03 did not re-open :8; finder quote.)
- **Tests:** H tools\test_build_pass.ps1 with literal 3 (a second copy); none in G\tests holds CLAUDE.md or BP "three" to build_slot.ps1. **Open:** none. **Hook gap:** none stated.

### L-F03-08: How many charts are in the user's library (18,610 / 18,811 / 18,755)?
- **Source:** I8-107, I8-118, I8-171; F03 Group 11; V03 extra claim.
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED (V03): three numbers confirmed, but 18,755 is "results went Stale", not a chart count, so not necessarily a split.
- **Homes:** M\min-path-delta-proof.md:29 "18,610 charts"; M\no-full-library-runs.md:17 "18,811 charts"; M\stored-versions-one-rule.md:11 "18,755 results went Stale"; the finder also names 19,000 (no home given). No home for the library size in H docs\agents; no owner in code or DB was read (finder).
- **Splits:** 18,610 against 18,811 for "charts" (both may be different dates, unchecked). 18,755 is a different thing.
- **Ruling quotes:** none. **Tests:** none. **Open:** which count is right was not checked. **Hook gap:** none stated.

### L-F03-09: Worked-example numbers copied between the test-rubric pages (Pitch Dark 243 / 128520 / 128400, bank 13440 / 14400, Blight, combo steps 10/20/30).
- **Source:** I7-127, I7-128, I7-132, I7-134, I7-135; F03 Group 2; V03 (extra claim).
- **Kind:** K1.
- **Verdict:** CONFIRMED (V03 by grep for the Pitch Dark lines TR:48, :49, :130; TE and TR:175 not reopened).
- **Homes (finder):** TR:48-49 ("chord 243, tick 128520", "tick 128400"), TR:130, TR:175; TE:23; Blight chord 228, ticks 64800, 65280 at TE:74; 13440/14400 at TR:25, TR:185, TE:33, TE:42, TE:83-85; fill reach 7285 / 7200 ms / 85 ms at TE:179-181; combo steps 10/20/30 at TE:224-244, 255, 282-311 with the owner page H docs\clone-hero\06-scoring.md:53 "### SC-05 The multiplier is one plus a tenth of the streak, up to 4"; fill pages 04-fills.md:192 (FL-19), :232 (FL-23); `kCh11FillLeadBeats = 4` at src/search/graph.h:71.
- **Splits:** none found. TR:48-49 and TE:23 hold the same run's output twice, in the page that forbids copying one run's output (TR:23). Whether 128520 and 65280 still equal the engine's record today is unchecked (finder did not open the charts).
- **Ruling quotes:** none. **Tests:** none in G\tests; H\tools\check_pointers.py exists but the finder did not read what it checks. **Open:** unchecked against the engine. **Hook gap:** none stated.

### L-F03-10: The pointer "src/search/engine.cpp line 2015" for the bank-rise stamp.
- **Source:** I7-133; F03, F03b; V03b.
- **Kind:** K8/K2 (stale line pointer).
- **Verdict:** CONFIRMED (V03b read the line).
- **Homes:** TE:88 "The search stamps `bank_rise_ticks` (`src/search/engine.cpp` line 2015)" (and TE:83-85 for the check); H src\search\engine.cpp:2015 is `const ScoreGraphNode* node = en.nodes[(size_t)oa.act_node];`, not a bank-rise line (V03b). Test data lives in tests\test_sp_meter_curve.cpp:279, 306, 416-466; no 13440 or 14400 in H\src (F03b); TR:25, :185.
- **Splits:** the page says a line number the code at that line does not match.
- **Ruling quotes:** none. **Tests:** none. **Open:** whether line 2015 was ever a bank-rise line (git log would settle; V03b ran nothing). **Hook gap:** none stated.

### L-F03-11: Where does the `acts > 1000` floor (D43) live?
- **Source:** I7-131; F03, F03b; V03b.
- **Kind:** K2.
- **Verdict:** CONFIRMED (V03b), unconfirmed on the decision-log line text.
- **Homes:** TE:123 `CHECK(acts > 1000);  // a floor the user approved (D43)`; TR:13, :88; H tests\test_sp_end.cpp:793, :830; the same comment form at tests\test_fills_and_skips.cpp:305 (`skipped > 100`); H docs\audit\2026-10-03-fix-decisions.md:103 (finder quote) "D43 ... Approved as test limits, none an app setting or visible: `acts > 1000` and `skipped > 100` (test_search.cpp)" (V03b: line 103 not read; test_search.cpp appears 3 times in the log). M\labels-are-not-reasons.md:11 names "Approved under D43" as an example of a label that is not a check.
- **Splits:** the decision log says the check is in test_search.cpp; the code has it in test_sp_end.cpp.
- **Ruling quotes:** none Confirmed; decision-log lines are recorded decisions, not `## Confirmed` lines. **Tests:** n/a. **Open:** decision-log line text not read by V03b. **Hook gap:** none stated.

### L-F03-12: "Same goal failed twice means put the design to the user": who states it, what checks it?
- **Source:** I8-132; F03 Group 12.
- **Kind:** K7, K3 candidate.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** M\question-the-design-before-patching.md:3, :11 "When the same goal fails twice and each fix works around the last failure, stop and put the design itself to the user, with the alternatives"; M\MEMORY.md:17 "2+ failed runs on one goal"; AR:9 (a plan changing a display takes its design to the user first); BP:85 "If your last two status lines show the same step and no new result, you are not making progress. Stop and report" (different actors, similar trigger).
- **Splits:** none ("twice" and "2+" agree). **Ruling quotes:** none. **Tests:** none. **Open:** not reached. **Hook gap:** none (finder: no hook found in G for "twice", "same goal", "failed runs").

### L-F03-13: How long does the threaded chart list (T7) take: "about 8 ms" or "5.4 ms against 13.8 on 1"?
- **Source:** I8-012; F03b; V03b.
- **Kind:** K2.
- **Verdict:** CONFIRMED (V03b; whether the ruling-file line sits under Unconfirmed was not re-checked, finder says it does).
- **Homes:** M\approved-work-is-not-reasked.md:13 "(the threaded chart list, about 8 ms)"; H docs\agents\rulings\code-and-engine.md:63 "T7 (reading the chart list on 8 threads, measured at 5.4 ms against 13.8 on 1)" (under `## Unconfirmed`, line 61, per finder; a home, not a ruling). M\approved-work-is-not-reasked.md:11 "about 130 ms" and :15 "floor tint 0.45 ... or keep today's 0.2".
- **Splits:** T7 time, note "about 8 ms" against ruling-file line 5.4 ms on 8 threads / 13.8 ms on 1.
- **Ruling quotes:** none for the figures. Neither "do not re-ask" quote names a split (see L-F03-14).
- **Tests:** none. **Open:** none. **Hook gap:** none stated.

### L-F03-14: Who checks "after a yes, do not re-ask approved work"?
- **Source:** I8-012; F03b; V03b.
- **Kind:** K7.
- **Verdict:** CONFIRMED (V03b: no hook text found; it searched only the repo rulings and the note, not the gates folder again).
- **Homes:** M\approved-work-is-not-reasked.md; M\MEMORY.md:5; AR:9 rule 3 (work waits for a yes; "when the user asks for one specific edit, the request is the approval", finder).
- **Splits:** none (rule 3 points the other direction: work waits for a yes).
- **Ruling quotes:** code-and-engine.md:8 "why are you trying to convince me to skip something i already approved?" and ui-and-display.md:9 "yes i approved the mockup and asked for option B. so why are you offering what we have today as an option?" (V03b: neither names a split; they are the rule in the user's words).
- **Tests:** none. **Open:** none. **Hook gap:** none stated.

### L-F03-15: dmleaderboards latency and fetch rules (warm ~650 ms, cold 30 s+).
- **Source:** I8-050; F03b; V03b.
- **Kind:** K1 (weak).
- **Verdict:** CONFIRMED as K1, weak (V03b did not reopen the code).
- **Homes:** M\dmleaderboards-comparison.md:19-20 "warm ~650ms, cold 30s+"; H src\net\http.cpp:172-174 (real User-Agent; Cloudflare 403), :180-185 "tens of seconds", `WinHttpSetTimeouts(session_.h, 15000, 20000, 30000, 120000)`; H src\net\dmbot_client.h:3-12 (finder quotes).
- **Splits:** none ("30s+" in the note, "tens of seconds" and a 120 s timeout in code; "650 ms" has one home).
- **Ruling quotes:** none. **Tests:** none. **Open:** code lines not reopened. **Hook gap:** none stated.

### L-F03-16: Line-number pointers into src/parse/song.cpp have drifted (203/2009 against 205/2204).
- **Source:** I8-057 side note; F03b; V03b (not re-checked).
- **Kind:** K2.
- **Verdict:** NOT VERIFIED (V03b: the side note was not re-checked; the "4 of 50 lanes" count itself is one home).
- **Homes (finder only):** H docs\clone-hero\02-note-list.md:611, :631, :641 and questions.md:270 quote "src/parse/song.cpp line 203" and "called at line 2009"; the code has `light_drum_lanes` at src\parse\song.cpp:205 and the call at :2204 (song.cpp:2198-2204). M\drum-lanes-d112-2026-10.md:17 "4 of 50 lanes differ between the Onyx and CH pad rules" (one home).
- **Splits:** pointer lines against the code's line numbers.
- **Ruling quotes:** ui-and-display.md:117 (section not confirmed by F03b). **Tests:** none. **Open:** not re-checked. **Hook gap:** none stated.

### L-F03-17: How far behind was the executor worktree base (361 commits), and where is the rule?
- **Source:** I8-080; F03b; V03b.
- **Kind:** K7, K1.
- **Verdict:** CONFIRMED-CHANGED (V03b): the finder said one home; there are two. The "no hook" claim was not re-searched by V03b.
- **Homes:** M\git-traps-in-shared-checkout.md:13 "Four executors once started on a base 361 commits old"; M\git-traps-in-shared-checkout.md:16 (rule: cut an executor worktree from main by hand, executor checks its base commit first); archived M\archive\agent-worktree-base-is-origin-main.md:11 "An `Agent` call with `isolation: worktree` cuts its worktree from `origin/main`, not local `main`. On 2026-11-07 that was `cdc505eb`, 361 commits behind, and all four timing executors stopped at their start check." (gives the cause; the live note gives only the figure). MP:5, :7 "each in its own worktree", "Cut its branch from the blocker's executor branch" (finder). Only reviewer isolation is a hook: G\review_dispatch.py:24-26,149-150, tests test_review_dispatch.py:79-92, DOR:200 (finder).
- **Splits:** the live note gives the figure and not the cause; the archived note gives both. Situation: an executor started with worktree isolation against one cut by hand from main.
- **Ruling quotes:** none. **Tests:** none for executor base. **Open:** the "no hook or doc names the executor worktree base" claim is the finder's. **Hook gap:** none stated.

### L-F03-18: The "781k tokens and two refused merges" figures.
- **Source:** I8-126; F03b; V03b.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V03b read the note; the decision-log copy was not reopened).
- **Homes:** M\one-reviewer-rule-d107.md:13 "~781k tokens and two refused merges"; H docs\audit\2026-10-03-fix-decisions.md:599 "about 781k subagent tokens and two refused merges" (finder; cites a removed handoff). Restatements of the rule (not the figures): MP:5,7,9; reviewer.md:3; G\README.md:138; CLAUDE.md rule 5; BP:73, :89.
- **Splits:** none (they agree). **Ruling quotes:** none. **Tests:** none. **Open:** decision-log copy not reopened by V03b. **Hook gap:** none stated.

### L-F03-19: How long is a key held (KEY_HOLD_S 0.003)?
- **Source:** I8-148; F03b; V03b.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V03b).
- **Homes:** H tools\ch_probe\input_driver.py:41 `KEY_HOLD_S = 0.003` (users :195, :216); H tests\single_owner\rules\probe.json:506-535 (owner "KEY_HOLD_S in tools/ch_probe/input_driver.py", whitelist lines :527, :532; V03b: :508, :527, :532); H tools\ch_probe\tests\test_input_driver.py:117-118 `assertEqual(input_driver.KEY_HOLD_S, 0.003)` with the comment "D54 records the 3 ms hold as it is"; H tools\ch_probe\interfaces.py:233 (pointer). Same number, different questions: docs\clone-hero\03-timing.md:123,125; 05-star-power.md:467; tools\ch_probe\experiments\stall_harness.py:89.
- **Splits:** none (V03b: two homes held equal by the scan rule plus a pin).
- **Ruling quotes:** none. **Tests:** probe.json scan rule; the pin at test_input_driver.py:118. **Open:** none. **Hook gap:** none stated.

### L-F03-20: Never scroll the library unasked, against the render-checks step "Scroll to the middle of the list".
- **Source:** I8-154; F03b; V03b.
- **Kind:** K3, K5, K7.
- **Verdict:** CONFIRMED (V03b; the split is real in the words).
- **Homes:** M\search-before-clicking-a-song.md:3, :11, :13 ("1,300 wheel notches" and the user's quoted words); M\MEMORY.md:10; M\render-checks-use-the-corpus.md:19 "Scroll to the middle of the list, click a song, compare before and after, close, and compare again."; H docs\agents\ui-testing.md:99 (guards `item_ref`, `library_scroll_refusal`, `library_key_refusal`, `kPageKeys` in tests\ui\uitest_commands.cpp:239; "No guard here can stop a shell script that sends wheel or key messages ... no hook reads shell text, so the rule ... is the only thing that binds there"). No hook reads wheel messages (finder searched PostMessage, WM_MOUSEWHEEL, mouse wheel, SendInput in G).
- **Splits:** the render-checks note tells the step "scroll to the middle"; the search note says never scroll without asking in chat. V03b: whether the render-checks line means the guarded test-command scroll or a shell wheel post is not in the text; only the user can say.
- **Ruling quotes (ui-and-display.md, Confirmed 7-101):** line 27 (2026-10-30) "ive had enough of this shit. something like 10 times now you waste my time by scrolling thorugh the entire library for minutes ... this should never happenw tihout my approval." line 48 (2026-10-24) "why are you scrolling through the entire library??? why do you keep forgetting search exists?" Neither names the render-checks step, so neither names both sides (V03b).
- **Tests:** the uitest guards for the guarded channel only. **Open:** the meaning of the render-checks scroll step. **Hook gap:** hook gap: yes, G (no hook reads wheel or key messages posted from a shell; ui-testing.md:99 says so in general words).

### L-F03-21: The skill's SP rules (SqOut gap measured from the pre-collection end; phrase collected extends SP +2 measures).
- **Source:** I8-195, I8-196; F03b; V03b.
- **Kind:** K6 candidate, K1 (one home in tooling).
- **Verdict:** CONFIRMED (V03b: one home in tooling; light search).
- **Homes:** ~/.claude\skills\analyzing-sp-squeezes\SKILL.md:22 "A phrase whose completion note lands inside SP is collected -> SP extends +2 measures.", :23 and :31 (SqOut rule, Enigma Machine 4750 ms wrong against 2083 ms right, "Use the pre-collection end for SqOut (tool does this)"). Code owners of the question (F03b): H src\core\backend_value.h:14-29 (`sqout_position`, `paid_by_sp_walk`), src\core\model.h:276-295 ("Every row is measured from the one SP end (D269 call 4)"); `kMeasuresPerSpBar` at src\core\timing.h:254, named at H docs\clone-hero\05-star-power.md:109,129,363. V03b: a search of src, docs\clone-hero and tools for "pre-collection" found nothing, so the phrase has no code definition; the code's words are "the one SP end". The skill's tool squeeze_scout.py does the work, a second owner (F03b; unchecked).
- **Splits:** none stated as a contradiction; two words for one question ("pre-collection end" in the skill; "the one SP end" in code).
- **Ruling quotes:** none for these rules. code-and-engine.md:12 "there should never be two different answers for where star power ends. ever." (see L-F02-15). **Tests:** none. **Open:** the game's rule at 05-star-power.md:403 (SP-37) was not read. **Hook gap:** none stated.

### L-F03-22: When must the doc screenshots be re-run ("before each release; after a label or layout change")?
- **Source:** I7-226; F03b; V03b.
- **Kind:** K7.
- **Verdict:** CONFIRMED (V03b: one home; the gates folder was not searched).
- **Homes:** H docs\agents\doc-screenshots.md:21-23 (:23 per finder) "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows." H tools\doc_shots.ps1:1-11 only points at the page. V03b grepped doc_shots.ps1 and test_docs_match_code.cpp for those phrases and found nothing.
- **Splits:** none. **Ruling quotes:** none. **Tests:** none. **Open:** the gates folder search. **Hook gap:** none stated.

---------------------------------------------------------------------
## End list
---------------------------------------------------------------------

### Candidates the verifier REFUTED (row ids)
No row is wholly REFUTED. One finder candidate was REFUTED inside a row: F02's claim that UT's "98 charts" has drifted (finder counted 102), refuted by V02 as a drift (102 counts folders, the loader-style file count is 98 and tests pin 98). It sits in L-F02-06, whose verdict is CONFIRMED-CHANGED because F03's 115 / 97 memory-note split in the same row was CONFIRMED by V03.

### Rows no verifier reached (verdict NOT VERIFIED)
- F01: L-F01-09, L-F01-10, L-F01-11, L-F01-12, L-F01-13, L-F01-14, L-F01-17, L-F01-18, L-F01-19, L-F01-21, L-F01-22 (ruling line confirmed only), L-F01-23, L-F01-24, L-F01-25, L-F01-26.
- F02: L-F02-01, L-F02-02, L-F02-03, L-F02-10, L-F02-11, L-F02-12, L-F02-13, L-F02-14, L-F02-15, L-F02-16, L-F02-18, L-F02-19, L-F02-20.
- F03: L-F03-04, L-F03-12, L-F03-16.

### Single-home items the finders listed but gave no row (no verifier opened them except where noted)
F01: I2-005, I2-007, I2-008, I2-014, I2-033, I2-034 (stall.py constants), I3-008, I3-009 (ref_judge.py timeouts 30 s / 60 s; relation 30 < 60 not written anywhere), I3-054 (MAX_BRIEF_FOLDERS 16), I3-082 (65536 read block), I4-081 (TOOLS line caps). F02: I5-078 (BP:51 "If a timing test fails while the machine is busy, report it once. Do not rerun the suite to chase it."), I7-008, I7-010, I7-011, I7-013, I7-050, I7-051, I7-052, I7-054, I7-100 (UT pointers and measurements; "all owner claims were checked in code and are true" per F02; V02 did not reopen). F03: I8-172, I8-095, I8-027, I8-174, I8-180, I8-104, I8-105, I8-107 (singles, no other home); I8-163 (video offset 19995.46, one home, confirmed single by V03b); I7-155 (stall harness, not reopened by V03b); I7-189 (not reopened).

### Side notes carried (no verifier)
F02 side note 1 (the "over 20 lines" Confirmed ruling has no applied threshold found) and side note 4 (TE:101,113 pin 98 as examples) are in L-F02-12 and L-F02-06. F03 side note on M\one-build-per-wave.md modification time later than its text date is not carried as a row.

---------------------------------------------------------------------
Counts: 68 rows (F01 26, F02 20, F03 22). CONFIRMED 30, CONFIRMED-CHANGED 7, REFUTED 0 (one candidate refuted inside a row), NOT VERIFIED 31. Splits named in the rows: 22 (F01 9, F02 2, F03 11), counting V01's added S7 and S8 and V02's added reading of S1; a words-only or weak split is counted when the verifier listed it.
