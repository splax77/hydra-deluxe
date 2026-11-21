# TOOL-AUDIT-T5-L10 ledger (finders F28, F29, F30)

Reports read: F28, V28, F29, V29, F30, V30, F30b, V30b. F28b, F29b, V28b, V29b do not exist. Where a verifier and finder disagree, the verifier's reading is used and said so. Short names as the finders use them: PC = Hydra CLAUDE.md, AR = docs/agents/agent-rules.md, BP = brief-preamble.md, DOR = derive-once-review.md, OM = owner-map.md, AM = audit-method.md, CH = clone-hero-mcp.md, UT = ui-testing.md, PD = public-doc-style.md, GR = gates README.md, MP = model-postures.md, IN = integrate.md, G = ~/.claude/gates, H = Hydra main checkout. Inventory row ids are given as the finders gave them.

Totals: 65 rows. CONFIRMED 35, CONFIRMED-CHANGED 12, REFUTED 1, NOT VERIFIED 17. Splits: 29.

---------------------------------------------------------------------
## Finder F28 rows
---------------------------------------------------------------------

### L-F28-01: Which tool hands out a build pass, how many builds at once, and what is an agent told when it asks?
- **Source:** I1-003, I1-013, I5-031 (part), I5-050, I5-053 (part); F28 Q1; V28 verdicts 1-3.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED. V28 did not re-open BP:57 or development.md.
- **Homes:** G\percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`; percall.py:109 "Agents never build, so hydra_build_pass is refused to every agent. Instead: commit your work and hand back; ..."; H\tools\hydra_ui_mcp.py:874 `def hydra_build_pass()` (V28: the prefix is server name plus tool name, so the string is built by hand from two places); H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`; build_slot.ps1:87 `$BuildPassHowTo = 'Agents never build: hand back, and the main session builds...'`; AR:20 "At most three builds run at once"; PC rule 14 "At most three builds run at once". Finder only: BP:57, development.md:114-120, MP:9/:11, executor.md:11, reviewer.md:21, G\README.md:15.
- **Splits:** None in the value. The refusal text has three wordings (percall.py:109, build_slot.ps1:87, BP:57).
- **Ruling quotes:** H\docs\agents\rulings\agents-and-workflow.md:8 (Confirmed, as F28 quotes it): "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." Names no split.
- **Tests:** test_build_pass.ps1 holds the count to build_slot.ps1 only (finder only, not opened by V28). No test links the percall.py string to hydra_ui_mcp.py, and none links the docs' "three" to `$BuildSlotCount`.
- **Open:** none beyond the unchecked homes.
- **Hook gap:** see L-F28-22.

### L-F28-02: When an agent is stuck after a stall notice, does the main session nudge it first or stop it at once?
- **Source:** I2-009, I2-011, I2-016, I2-020, I2-022, I2-024, I2-038, I2-056, I5-054; F28 Q2 split 2a; V28 split A.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** MP:11 "nudge it once with SendMessage ... If it does not recover, stop it with TaskStop"; G\stall.py:124-125 and :143-144 "If an agent is stuck, stop it with TaskStop and re-dispatch its remaining work."; stall.py:460 (check-in) "nudge it or stop it".
- **Splits:**
  - MP:11 answers "nudge first, then TaskStop"; stall.py:124/:143 answers "TaskStop and re-dispatch" with no nudge. They differ for a stuck agent after a stall or past-limit notice. stall.py:460 gives both.
- **Ruling quotes:** agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" and :25 "i don't care if an agent is read only, no status updates is unacceptable". V28: neither names nudge versus stop. None names both sides.
- **Tests:** none holds them equal.
- **Open:** none.
- **Hook gap:** none.

### L-F28-03: What does the main session do with a status line (relay wording)?
- **Source:** I2-009 and neighbours; F28 Q2; V28 verdict 9.
- **Kind:** K5 (K1).
- **Verdict:** CONFIRMED.
- **Homes:** G\stall.py:30-33 STATUS_HEADER "Relay the claim to the user briefly, in plain English. When the evidence disagrees, add one sentence that says so, and open the agent's transcript."; MP:11 "Check each line against that evidence. Relay the claim to the user briefly, in plain English, and add a second sentence only when the evidence disagrees."
- **Splits:** None (F28 2b: the two agree).
- **Ruling quotes:** agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english" (finder only, not re-read by V28).
- **Tests:** test_stall.py:305 pins the phrases in stall.py (F29/F30 report); MP:11 is not held.
- **Open:** none.
- **Hook gap:** none.

### L-F28-04: What are the stall steps (10, 20, 30 minutes), and does a notice arrive at exactly 30 minutes?
- **Source:** I2-056, I5-054; F28 Q2 and Q12 rule 14 detail; V28 verdict 7 and split E.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED in part. V28 re-read stall.py:21-23 and AR:27 "10 minutes" only; the 20/30 text of AR:27 and progress.py were not re-read.
- **Homes:** stall.py:21-23 `STALL_SECONDS = 600`, `LIMIT_SECONDS = progress.MINUTES_STOP * 60`; stall.py:134 skips when `now - first <= LIMIT_SECONDS`; AR:27 "10 minutes" and (finder) "30 minutes after its first call gets one more notice"; BP:73 (finder).
- **Splits:**
  - AR:27 says a notice at "30 minutes after its first call"; stall.py:134 sends it only after more than 30 minutes. They differ at exactly 30 minutes.
- **Ruling quotes:** None found.
- **Tests:** test_stall.py builds its cases from the constants; no test holds AR:27's numbers to stall.py (finder only); test_docs_numbers.py covers call counts and the status minutes only (finder only).
- **Open:** one-minute edge not run.
- **Hook gap:** none.

### L-F28-05: When does the main session get a context warning, and is "50% of the window" read from anywhere?
- **Source:** I2-011; F28 Q3; V28 verdicts 4 and 5.
- **Kind:** K1, K4.
- **Verdict:** CONFIRMED. V28 did not re-search settings.json for a window setting.
- **Homes:** stall.py:27 `CTX_THRESHOLD = 500000`; stall.py:36 "fired at 500K (50% of the window)" (V28: a literal, not built from the constant); MP:15 "under 500K tokens".
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_stall.py:421-434 pins the text and the constant to each other (finder only).
- **Open:** none.
- **Hook gap:** none.

### L-F28-06: The wake watch timeout (660) and the plan file its comment cites
- **Source:** I2-057, I2-059, I2-065, I2-066; F28 Q4; V28 verdicts 6 and 13.
- **Kind:** K1, K8.
- **Verdict:** CONFIRMED that settings.fragment.json:235 has `"timeout": 660` and that wake.py:5 cites docs/superpowers/plans/2026-11-16-agent-supervisor.md (`git show 80b310a9:docs/superpowers/plans/2026-11-16-agent-supervisor.md`), which V28 did not find in H/docs/superpowers/plans or ~/.claude/plans. "Pinned by a test" is NOT VERIFIED (V28 did not open test_wake.py).
- **Homes:** G\settings.fragment.json:235 `"timeout": 660, "asyncRewake": true`; live settings.json:245 (finder only); wake.py:5 the plan path; check.py:34-35 "the supervisor plan" with no path.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_wake.py:212-218 pins the 660 to `STALL_SECONDS + SCAN_EVERY` (finder only).
- **Open:** none.
- **Hook gap:** none.

### L-F28-07: What does a hook do when it crashes (block, warn, silent; exit code)?
- **Source:** I2-055, I2-074, I2-101, I2-116, I2-078; F28 Q5.
- **Kind:** K6, K1, K3.
- **Verdict:** NOT VERIFIED (V28 did not reach it).
- **Homes (finder only):** G\common.py:52 `run(body, mode)` with "closed" and "loud" modes; G\README.md:90-100; G\cleanup_sweep.ps1:31 and :699-700 and G\session_cards_start.ps1:24-25 and :300 ("fail loudly, never freeze", "Exit 0 always"); D46 at H\docs\audit\2026-10-03-fix-decisions.md:109-110.
- **Splits:** None named by the finder.
- **Ruling quotes:** agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (finder only).
- **Tests:** test_common.py covers common.run; test_h52 and test_h53 cover the PowerShell side; no test holds them equal (finder only).
- **Open:** whether the Python and PowerShell crash behaviours agree.
- **Hook gap:** none.

### L-F28-08: Housekeeping scripts: test file names and parameter lists in header comments
- **Source:** I2-102, I2-118; F28 Q6.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** cleanup_sweep.ps1:33-46; session_cards_start.ps1:26-32; G\README.md:152.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none holds the header text equal to the param block.
- **Open:** session_cards_start.ps1:26 says "The test uses all four"; not counted.
- **Hook gap:** none.

### L-F28-09: Who tells the next main session to add a `[write-back]` Clone Hero fact to its page?
- **Source:** I2-113, I2-115, I2-116 (part), I5-018; F28 Q7; V28 verdict 11.
- **Kind:** K7.
- **Verdict:** CONFIRMED. V28: session_cards_start.ps1 has 0 matches for "write-back".
- **Homes:** PC "Past sessions" paragraph ("the next main session adds that fact to the page alongside the user's request"); writer-brief.md:64 and :81 carry the `write-back:` line the writer writes; check.py:39-49 and index.py:9,:51 produce and mark it (finder only).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none holds PC equal to the ps1 text.
- **Open:** none.
- **Hook gap:** none.

### L-F28-10: The two Hydra gate marker files name a folder of deciding rules that no longer exists
- **Source:** I2-113 neighbours; F28 Q7 K8; V28 verdict 12.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** H\.derive-once-gate and H\.doc-review-gate name `hooks\lib\derive_once_rules.ps1` and `hooks\lib\doc_review_rules.ps1`; V28: ~/.claude/hooks holds only `state`, ~/.claude/hooks/lib does not exist; the logic is in G\review_rules.py (finder).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F28-11: Does `explain_main_move` really decide that a commit changing only docs/sessions needs no reviewer?
- **Source:** I2-113 owner claim; F28 Q7.
- **Kind:** K8.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** session_cards_start.ps1:265-266 comment ("a commit on main that changes only docs/sessions needs no reviewer"); G\review_rules.py:398 `explain_main_move` and :180 `_is_code`.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** the finder did not trace whether docs/sessions/* passes `_is_code`.
- **Hook gap:** none.

### L-F28-12: Who may be named as the catcher of a stuck agent in a session card?
- **Source:** I4-031, I4-068, I4-122; F28 Q8; V28 verdict 8.
- **Kind:** K1, K8.
- **Verdict:** CONFIRMED-CHANGED. Path correction by V28: the files are ~/.claude/tools/session_cards/check.py and writer-brief.md; F28's H\tools\session_cards does not exist.
- **Homes:** check.py:36 `CATCHERS = ("main session", "hook notice", "user")`; writer-brief.md:91 (same three phrases, a typed list; V28: nothing ties it to CATCHERS); test_session_cards.py:486-488 reads `check.CATCHERS`. check.py:34-35 cites "the supervisor plan" (see L-F28-06).
- **Splits:** None.
- **Ruling quotes:** None found. "Yes, record it" (cited in check.py) has no hit in rulings (V28); it appears as card quotes in docs/sessions/2026-11-16-60c3a576.md:30 and 2026-11-16-e816e44f.md:25 (finder), which are not filed rulings.
- **Tests:** none holds writer-brief.md equal to CATCHERS.
- **Open:** none.
- **Hook gap:** none.

### L-F28-13: How to write: the plain-English rule and its copies
- **Source:** I5-001 to I5-004; F28 Q9.
- **Kind:** K1, K3, K7.
- **Verdict:** NOT VERIFIED (V28 did not open BP:121-123 or writer-brief:94). See L-F30-17 for a related verified row.
- **Homes (finder only):** ~/.claude/CLAUDE.md:3-29; BP:121-123; MP:13; writer-brief.md:94; stall.py:32.
- **Splits:** None named (K3 note: MP:13 and BP drop the gloss and pre-show-check parts).
- **Ruling quotes:** agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english" (finder only).
- **Tests:** none pairs these texts.
- **Open:** all of it.
- **Hook gap:** none.

### L-F28-14: May an agent or session ask the user a question when something is unclear?
- **Source:** I5-005, I5-022 (part), I5-052; F28 Q10 split 10a; V28 split C.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED. V28: AR:23 is narrower than F28 said; it bars a plan from putting a rule it made up to the user as a question, and is not a game-or-permission-only limit. The other side is a memory note, which V28 did not open and which is not the user's words.
- **Homes:** PC:1 "Ask questions if something is unclear."; AR:23 (plan never puts its own reading of a gap to the user); memory ask-only-game-or-permission-questions.md (finder only).
- **Splits:**
  - PC:1 invites a question about anything unclear; the memory note limits questions to game or permission. They differ for a question about wording, proof standard or tool choice.
- **Ruling quotes:** questions-and-audits.md:9 and :15 (as F28 quotes them) do not name both sides; V28 did not re-read them.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F28-15: Who decides what a project word means: CONTEXT.md or the code's usage?
- **Source:** I5-022, I5-058; F28 Q11 split 11a; V28 split D.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED-CHANGED. V28's grep did not find "CONTEXT.md decides" in AR; AR:43 itself sends a disagreement to the user ("that is a question for the user, in game terms with both readings"). It is a split only if AR:41 carries "CONTEXT.md decides"; V28 says to re-open AR:41. (F29's V29 split 7 and F30 Family F read AR:41 as carrying it; see L-F29-13 and L-F30-09.)
- **Homes:** AR:41 and AR:43 (as quoted by F28); DOR:51; BP:99.
- **Splits:**
  - AR:41 "CONTEXT.md decides what a project word means" against AR:43 "The code's usage settles the meaning". They differ when a definition's text and the code's usage disagree.
- **Ruling quotes:** None found (finder: searched "CONTEXT" in rulings, only Unconfirmed notes).
- **Tests:** none.
- **Open:** AR:41's text.
- **Hook gap:** none.

### L-F28-16: What does "build" mean in rule 3 and rule 14?
- **Source:** I5-039; F28 Q12 split 12a; V28 split B.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** AR:9 "In this rule a build means any piece of work"; AR:20 "In this rule a build means compiling, not rule 3's piece of work."; MP:3 (rule 3 meaning); MP:9 and :11 (compile meaning).
- **Splits:**
  - Rule 3's "build" means any piece of work; rule 14's means compiling. MP:3 uses the first, MP:9/:11 the second, inside one file.
- **Ruling quotes:** agents-and-workflow.md:8 uses "builds" for compiles. V28: no ruling on the word in rule 3.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F28-17: User quotes in AR rule 15 and the display rule are not under `## Confirmed`
- **Source:** I5-051, I5-052; F28 Q10 and Q12; V28 verdict 14.
- **Kind:** K8.
- **Verdict:** CONFIRMED. V28: grep for "correct rule is to use", "invent new rules", "invents its own rule" in docs/agents/rulings/*.md has no hits. In agents-and-workflow.md Confirmed starts at line 7, Unconfirmed at line 32.
- **Homes:** AR:21 and AR:23 quote the user (as F28 gives them: "the correct rule is to use what hydra already knows. stop trying to invent new rules").
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F28-18: The agent rules in short and long form (rules 1, 3, 9 to 15, owner map)
- **Source:** I5-031, I5-033, I5-034, I5-039, I5-045 to I5-049, I5-051 to I5-054, I5-060; F28 Q12.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED beyond rules 3 and 14 (V28 did not reach the table).
- **Homes (finder only):** PC:63-77, AR, BP, MP, agent files, memory; hook-side shared-code list only in AR:7.
- **Splits:** None beyond L-F28-16.
- **Ruling quotes:** questions-and-audits.md:18-20 (rules 11-13 clicks) and tests-and-runs.md:14-15 (finder only).
- **Tests:** none pairs PC lines with AR lines for rules 1, 3, 9-13, 15 (finder).
- **Open:** whether copies drift.
- **Hook gap:** none.

### L-F28-19: How far may an agent edit (BP:41) against what delete_rule enforces
- **Source:** I5-074; F28 Q13.
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** BP:41; G\delete_rule.py (deletes only).
- **Splits:** None. K5: "move or rewrite" has no hook.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** none stated beyond the K5 note.

### L-F28-20: Where does the build-pass tool keep its slot folder?
- **Source:** I1-013 side note; F28 Q1 side note; V28 side note; V29 split 3.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED.
- **Homes:** H\tools\build_slot.ps1:78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'`; V28: hooks live in gates and `hooks` holds only `state`. See also L-F29-04.
- **Splits:** None named by F28.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** whether that path is live; "a run of build_slot.ps1 would settle it".
- **Hook gap:** none.

### L-F28-21: Clone Hero: what to read first, and how to drive it
- **Source:** I5-007, I5-008; F28 Q11.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** PC:5 and :9; BP:105; card-writer.md:9; memory ch-runs-must-not-need-focus.md:17; premise-map.md:42,:78. One home plus pointers; no split.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** all.
- **Hook gap:** none.

### L-F28-22: A build through any route other than the build-pass tool
- **Source:** F28 side note; V28 side note.
- **Kind:** K5.
- **Verdict:** CONFIRMED. V28: percall.py:108 refuses by `tool_name` only; AR:27 says build_cpp.ps1 is the second stop.
- **Homes:** G\percall.py:108.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** hook gap: yes, G\percall.py.

---------------------------------------------------------------------
## Finder F29 rows
---------------------------------------------------------------------

### L-F29-01: When in a wave does the main session build: each executor branch before review, or one combined branch after review?
- **Source:** I5-076, I5-079, I5-080, I5-110, I5-112, I5-122, I5-136, I5-152, I6-045, I6-113, I6-062; F29 Group 1 SPLIT 1; V29 split 1.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED. V29: real difference, but the "build once" side lives only in a memory note, which is not the user's words.
- **Homes:** MP:11 build each executor's branch and run its tests "before review"; DOR:198 the same; IN:41 builds the joined branch once; memory one-build-per-wave.md:11-15 "review each branch, then put the reviewed ones on one combined branch, build it once ... Never plan one build per branch."
- **Splits:**
  - MP:11 and DOR:198 build per executor branch before review; the memory note builds one combined branch after review. They differ for a wave of many small C++ branches.
- **Ruling quotes:** agents-and-workflow.md:8-9 say nothing about when a wave is built (V29). None names both sides.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F29-02: May an agent run a test that needs a build, and which program names are banned?
- **Source:** same rows as L-F29-01; F29 Group 1; V29 "Group 1 K1/K3/K7".
- **Kind:** K1, K5, K7.
- **Verdict:** CONFIRMED.
- **Homes:** PC rule 14; AR:7; AR:20; BP:47; MP:9; executor.md:11; reviewer.md:21; DOR:116; GR:17 "No hook enforces that part ..."; percall.py:109 refuses only hydra_build_pass.
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." and :9 "any tests that require a build are done by the main session, not agents." Neither names a split.
- **Tests:** none holds the pages equal. build_slot.ps1's 3 is held by tools/test_build_pass.ps1 (finder, not opened).
- **Open:** none.
- **Hook gap:** hook gap: yes, G\percall.py (the test-program part has no hook; GR:17 says so).

### L-F29-03: Agents running hook tests: allowed by the hooks README, narrowed by the preamble
- **Source:** same rows; F29 SPLIT 2; V29 split 2.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED. V29: a narrowing, not an opposite answer.
- **Homes:** GR:17 "The hook tests on this page need no build, so agents still run them."; BP:47 "Run only the tests your brief names." and "Never run all the hook tests."
- **Splits:**
  - GR:17 permits running hook tests; BP:47 permits only the ones the brief names. They differ for a hook test the brief does not name.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F29-04: Two state folders for hook data (hooks\state and gates\state)
- **Source:** same rows; F29 SPLIT 3; V29 split 3. Same fact as L-F28-20.
- **Kind:** K2, K6.
- **Verdict:** CONFIRMED.
- **Homes:** tools/build_slot.ps1:78 `$BuildSlotDefaultDir = '...\.claude\hooks\state\build_slots'`; `~\.claude\hooks\state` holds only `build_slots`; `~\.claude\gates\state` holds agents, posture, reviews, status etc. and no `build_slots`; hooks plan 2026-10-25-hooks-from-scratch.md:78 and :94 move state to `gates\state`.
- **Splits:**
  - hooks\state\build_slots (build_slot.ps1) against gates\state (every other hook state). They differ for where a pass is stored.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** whether the live pass tool reads the hooks path; needs a run.
- **Hook gap:** none.

### L-F29-05: Code and source data are the truth; a search that finds nothing proves nothing; when blocked
- **Source:** I5-090, I5-091, I5-093, I6-033; F29 Group 2.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V29 did not reach Group 2).
- **Homes (finder only):** PC:71, AR:15, BP:93-95, MP:13, handoff.md:3 (the handoff sentence word for word); BP:103, DOR:76, doc-review.md:65, AM:124 (absence rule); BP:109-111 pointing at owner-map.md "The stops" (finder checked the claim true: owner-map.md:3 and :35).
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:10 "so this whole time we've been accepting another agent's framing without verifying any of this is actually necessary." (finder only).
- **Tests:** none.
- **Open:** all.
- **Hook gap:** none.

### L-F29-06: What decision must stand behind a new number?
- **Source:** I5-095, I5-096, I5-097 (number part); F29 Group 3 SPLIT 4; V29 split 4.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** BP:117 "a new number ... needs a user decision your brief names and the reason for that value written beside it"; DOR:45 lists four places (an ADR, CONTEXT.md, a `## Confirmed` line, a plan's "User decisions" header) and says a code comment or task steps do not count.
- **Splits:**
  - BP:117 says the brief names the decision; DOR:45 lists four places. Nothing says a brief-named decision must sit in one of the four.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F29-07: Comment rule, owner search list, and the "test calls production" rule
- **Source:** I5-095, I5-096, I5-097 (comment and search-list parts); F29 Group 3.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V29 did not reach the comment rule).
- **Homes (finder only):** BP:115-119; PC rule 15; AR:21; DOR:46, DOR:104; memory comments-explain-current-code.md.
- **Splits:** None beyond L-F29-06.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** all.
- **Hook gap:** none.

### L-F29-08: Plain English (BP:123, MP:13, global CLAUDE.md) and where it is stated
- **Source:** I5-097a, I5-117; F29 Group 4.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED by V29. Related verified row: L-F30-17.
- **Homes (finder only):** ~/.claude/CLAUDE.md:3-9; BP:123; MP:13; PC:1; stall.py:30-33.
- **Splits:** None (K3 note: BP and MP narrower than the global file).
- **Ruling quotes:** agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english".
- **Tests:** test_stall.py:305 pins the stall.py phrases only.
- **Open:** all.
- **Hook gap:** none.

### L-F29-09: May the main session start compiles and test runs without a new yes from the user?
- **Source:** I5-101, I5-102, I5-115, I6-103; F29 Group 5 SPLIT 5; V29 split 5.
- **Kind:** K3, K7.
- **Verdict:** CONFIRMED. The handoff-checker, main-session-role and library-run parts of Group 5 were not reached by V29.
- **Homes:** AR:9 "The main session's compiling and test runs inside approved work need no separate yes (rule 14)."; memory one-build-per-wave.md:15 "before launching work that will need builds, say how many builds it needs and get a yes." (memory, not the user's words); MP:3.
- **Splits:**
  - AR:9 says no separate yes for the main session's compiles; the memory note says to get a yes before work that needs builds. They differ for work approved as a plan that then needs many builds.
- **Ruling quotes:** agents-and-workflow.md:19 "if anything changes a display in the app or a record, run it by me. if it only changes code you can use your recommendation without input." and :24 "you can't just go straight into writing code without confirming first."; review-and-hooks.md:16 (library run). None names both sides of this split.
- **Tests:** none.
- **Open:** I did not find a session card for 8e6a2908 (finder: not searched).
- **Hook gap:** none.

### L-F29-10: "Build" in rules 3 and 14 inside the model-postures file
- **Source:** I5-102; F29 Group 5 K3; V29 Group 1 K3.
- **Kind:** K3.
- **Verdict:** CONFIRMED. Same question as L-F28-16.
- **Homes:** AR:9 glosses build as "any piece of work: writing code, editing hooks, or launching agents"; PC rule 3 does not gloss it; MP:3 uses rule 3's meaning and MP:9/:11 the compile meaning.
- **Splits:** None (listed as K3 only by F29; the split is stated in L-F28-16).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F29-11: How many jobs do the hooks do: five (plan) or six (README)?
- **Source:** I5-100, I5-145, I5-182, I5-189; F29 Group 6 SPLIT 6; V29 split 6.
- **Kind:** K2.
- **Verdict:** CONFIRMED. V29: K2 drift in a plan, low weight.
- **Homes:** hooks-from-scratch plan line 11 "The new system does five jobs and nothing else."; GR:3 "The hooks do six jobs and nothing else." (GR:3 adds "where that plan and the code differ, the code wins").
- **Splits:**
  - Plan line 11 answers five; GR:3 answers six. They differ in the count of hook jobs.
- **Ruling quotes:** review-and-hooks.md:15 and :11 (finder only); plan line 108 "Each rule has one home..." (finder only). Neither names both sides.
- **Tests:** test_wake.py:212-218, test_docs_numbers.py:142,:148,:155 hold the registration rule (finder only).
- **Open:** none.
- **Hook gap:** none.

### L-F29-12: Lists of what hooks enforce (BP:5, AR:27, GR) and the one-registration rule
- **Source:** I5-100, I5-145, I5-182, I5-189; F29 Group 6.
- **Kind:** K7.
- **Verdict:** NOT VERIFIED (V29 checked only the five-versus-six split).
- **Homes (finder only):** BP:5; AR:27; GR:116; settings.fragment.json:124-236; stall.py:22, :25. F29: nothing holds BP:5's list or AR:27's list equal to anything.
- **Splits:** None.
- **Ruling quotes:** None beyond L-F29-11.
- **Tests:** see L-F29-11.
- **Open:** whether BP:5's list omits the build-pass refusal, key-route check, media guard, ruling check, publish gate (finder).
- **Hook gap:** none.

### L-F29-13: Who decides a project word's meaning; what if the definition and the code's usage disagree?
- **Source:** I6-021, I6-022; F29 Group 7 SPLIT 7; V29 split 7. Same question as L-F28-15.
- **Kind:** K1, K7.
- **Verdict:** CONFIRMED. V29 reads AR "Project words and rulings", DOR:51 and BP:99 as F29 did and calls whether it is two answers or one rule "a reading call for the main session".
- **Homes:** AR:41 "`CONTEXT.md` decides what a project word means."; AR:43 "The code's usage settles the meaning, not a guess drawn from one sentence of a doc. If a definition's text and the code's usage disagree ... ask the user ... and change neither."; DOR:51 (both sentences); BP:99 (second sentence); RR:22, RR:34 (finder only).
- **Splits:**
  - AR:43 "The code's usage settles the meaning" against its own next sentence "If ... disagree ... ask the user, change neither". Same disagreement, two answers; DOR:51 repeats both, BP:99 has only the second.
- **Ruling quotes:** None quoted for CONTEXT.md in rulings; finder did not grep every ruling file.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F29-14: DOR's worked example says the later press moves "one for one"; the tool scales it
- **Source:** I6-028, I6-029; F29 Group 8 SPLIT 8; V29 split 8 (first part).
- **Kind:** K2.
- **Verdict:** CONFIRMED.
- **Homes:** DOR:66 "`retime_from_end` ... moves each later press by the read end minus the planned end, one for one."; tools/ch_probe/experiments/play_chart.py:361-369 `own_end_now_ms` = `own_stored_end_ms + conversion[side] * (read_end_ms - read_stored_end_ms)`; :441 moves `aim_ms` by `own_now_ms - p.own_end_ms`.
- **Splits:**
  - DOR:66 answers "one for one"; play_chart.py scales by the file's conversion. They differ whenever the conversion is not 1.
- **Ruling quotes:** None found.
- **Tests:** tests/test_replay_schedule.cpp:1716-1730 pins the C++ side and tools/ch_probe/tests/test_play_chart.py:1692-1726 the Python side with fixed numbers; none holds the two equal (finder; V29 did not open them).
- **Open:** none.
- **Hook gap:** none.

### L-F29-15: Is DOR:57-64's worked example still true of the code (where the read end's ratio comes from)?
- **Source:** I6-028; F29 Group 8; V29 split 8 (second part).
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED. V29 answers F29's open point: replay_schedule.cpp:437 and :636 still read `SPSqueeze::transfer` / `transfer_post`, but for the squeeze's own ratio (`g.ratio = *ratio`); the read end after the anchor takes its scale from `step_transfer_scale(act, *rs, timing)` at :656. So DOR's example no longer matches that path, and DOR says the fix would call `transfer_scale_between` while the code calls `step_transfer_scale`.
- **Homes:** DOR:57-66; src/core/squeeze_rating.h:50-73 (`transfer_scale_between`, `step_transfer_scale`, `frontend_transfer_scales`); tools/replay_schedule.cpp:437, :636, :656.
- **Splits:** None named.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** whether replay_schedule.cpp:746, :775, :791 agree with the Python; "not traced".
- **Hook gap:** none.

### L-F29-16: Two code homes of the end-conversion sum (C++ inverse, Python forward)
- **Source:** I6-029; F29 Group 8 K6; V29 K6.
- **Kind:** K6.
- **Verdict:** CONFIRMED (replay_schedule.cpp:267-276 `end_conversion`, `read_at_own_ms`; play_chart.py:361-369).
- **Homes:** tools/replay_schedule.cpp:267-276; tools/replay_schedule.h:139-141; play_chart.py:361-369, :390-442.
- **Splits:** None named.
- **Ruling quotes:** None found.
- **Tests:** V29 did not open the two tests, so F29's "no test holds the two equal" is unverified.
- **Open:** none.
- **Hook gap:** none.

### L-F29-17: How the scan test and the docs test are named in briefs
- **Source:** I6-105 neighbours (integrator); F29 Group 9 SPLIT 9; V29 split 9.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED-CHANGED. V29 refutes F29's "no owner found" for the filter string: tools/precheck/test_pick.cpp:789-791 writes `always: "-tc=docs match code*"` and `always: "-sf=*single_owner*"`. The prose in IN, DOR, AR, PC is copies of that.
- **Homes:** IN:43 and DOR:99 `-tc="single-owner*"`; IN:44 `-sf=*docs_match_code*`; AR:7 `"-sf=*single_owner*"`; PC:57 `"-tc=docs match code*"`; tools/precheck/test_pick.cpp:789-790. Test cases named "single-owner: ..." exist (tests/single_owner/clones.cpp:282, scan.cpp:155).
- **Splits:**
  - IN and DOR name the scan test with `-tc=` and the docs test with `-sf=`; the precheck prints the reverse. They differ in the filter form.
- **Ruling quotes:** None found.
- **Tests:** none holds the prose equal to the precheck.
- **Open:** whether both filter forms select the same cases; "needs a run".
- **Hook gap:** none.

### L-F29-18: Integrator copies (stop on a fold, never loosen a scan row, no integrator agent type)
- **Source:** I6-105, I6-106, I6-108, I6-109, I6-111, I6-116, I6-119; F29 Group 9.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V29 did not reach the integrator copies).
- **Homes (finder only):** IN:3, :5, :21, :23, :29, :48, :60; DOR:106; AR:11-12; MP:5 lists no integrator; ~/.claude/agents holds five files (card-writer, executor, planner, reviewer, scout).
- **Splits:** None named (IN:3 "one suite run" against IN:41 builds before review: F29 says "not an opposite answer").
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** I6-116 not searched in test-rubric.md.
- **Hook gap:** none.

### L-F29-19: No helper agents, no background jobs, read the preamble first
- **Source:** I6-002, I6-003; F29 Group 10.
- **Kind:** K1, K5, K7.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** DOR:5; doc-review.md:5; BP:77; `disallowedTools: Agent` in all five agent files; "background jobs" prose only.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** nothing holds `disallowedTools` against the prose; test_agent_files.py checks premise-map phrases only.
- **Open:** finder side note: no hook refuses `run_in_background` for an agent (shell_output.py:75 only skips its rewrite; progress.py:77 comment about agents parked on background work).
- **Hook gap:** hook gap: yes (finder only, not verified), G\progress.py and G\shell_output.py.

### L-F29-20: Audit-method rules (AM) and their copies; the "four exits" cross-reference at DOR:106
- **Source:** I6-120 to I6-134; F29 Group 11.
- **Kind:** K1, K7, K8.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** AM:19-26, :28, :34-36, :44, :54, :121-124; PC:33. Finder: the inventory's cross-reference to DOR:106 for "four exits" is wrong (DOR:106 holds no four-exit list). AM:44 calibration: commit 707c285 exists and holds src/core/squeeze_rating.cpp.
- **Splits:** None.
- **Ruling quotes:** None matched.
- **Tests:** none.
- **Open:** all; AM:36's card `2026-10-11-8bffee2b` not opened.
- **Hook gap:** none.

---------------------------------------------------------------------
## Finder F30 rows (finder and re-send F30b; verifiers V30, V30b)
---------------------------------------------------------------------

### L-F30-01: The ban on agents running hydra_tests, hydra_uitest and hydra_replay is copied across pages; only the pass tool is hook-refused
- **Source:** I7-005, I7-034, I7-064, I6-250, I6-181, I7-063; F30 Family A; V30 item 1 and S1.
- **Kind:** K1, K5, K7.
- **Verdict:** CONFIRMED. V30 quoted PC rule 14, AR:20, executor.md:11, reviewer.md:21, BP Tests section, GR:17, percall.py ~108-111 and doc-screenshots.md. Not re-opened by V30: DOR:116, UT:17/27/185, TM:95, CH:7.
- **Homes:** as above; percall.py refuses only hydra_build_pass ("Agents never build, so hydra_build_pass is refused to every agent"); GR:17 "No hook enforces that part ...".
- **Splits:** None (V30 changed S1 to a plain K1 copy; the pages agree word for word today).
- **Ruling quotes:** agents-and-workflow.md:8 and :9 (as in L-F29-02); V30 did not re-open the file.
- **Tests:** none holds the exe lists equal.
- **Open:** none.
- **Hook gap:** hook gap: yes, G\percall.py.

### L-F30-02: A whole-library run needs the user's `allow library run`; the hook tests only the flag text, the prose says any whole-library run
- **Source:** I7-039, I6-145 (part); F30 Family B S3; V30 item 2; V30b gates-tests section.
- **Kind:** K1, K5, K7.
- **Verdict:** CONFIRMED-CHANGED (V30: S3 CONFIRMED-CHANGED). V30: BP describes the hook exactly (flag text), while AR:14 and BP's first sentence name runs wider than the flag; a library-wide compare or timing run without the flag is prose-only.
- **Homes:** G\percall.py:14 `LIBRARY_FLAG`, :48; G\review_rules.py:50-51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :81; AR:14 "any run over the user's whole song library (for example the hydra_batch flag, or a whole-library compare or timing run). The user allows one run by sending exactly `allow library run`; each such message lets one command through."; BP Tests section; (finder only) TM:91, UT:203, AM:125, GR:13/37, memory no-full-library-runs.md.
- **Splits:**
  - The hook answers "a command holding the flag text"; AR:14 answers "any run over the whole library". They differ for a library-wide compare or timing run that lacks the flag.
- **Ruling quotes:** tests-and-runs.md:14, :18, :21 (finder; V30 did not re-open). V30: none names a flag-versus-wider split.
- **Tests:** none compares hook text with the docs (V30b opened test_review_rules, test_percall_rewrite, test_progress, test_review_dispatch).
- **Open:** UT:203 "narrow the library with a search first" not re-opened.
- **Hook gap:** hook gap: yes, G\percall.py (flag-text only). V30 also saw the hook refuse its own report-writing command because the text held the flag; the hook tests command text, not intent.

### L-F30-03: Is the allow phrase held equal between the constant and its tests?
- **Source:** I7-039; F30b gates-tests section; V30b split 5 and gates-tests correction.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED. V30b corrects F30b: test_posture.py also holds the literal (lines 197, 226, 234, 246); F30b said it had none.
- **Homes:** G\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`; literal in G\tests\test_review_rules.py:839-845, test_progress.py:427, :437, :455, :577, :584, test_review_dispatch.py:309, test_posture.py:197, :226, :234, :246.
- **Splits:**
  - The constant in review_rules.py against the same literal typed in four test files. They differ only if the constant changes (the tests then go red).
- **Ruling quotes:** None found.
- **Tests:** the tests hold the literal, not the constant.
- **Open:** whether any test also imports the constant; V30b did not open it.
- **Hook gap:** none.

### L-F30-04: Owner map and premise map: the rule is restated outside its owner pages
- **Source:** I6-150, I6-151, I6-156, I6-157, I6-158 to I6-162, I6-170, I6-180, I6-181, I6-182; F30 Families C and D; V30 item 3.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED (partly checked). V30 confirmed OM:11-13 carries a carve-out ("A plan that only moves files, edits docs or changes hooks needs none.") that PC's Owner map paragraph lacks. planner.md, reviewer.md, scout.md not re-opened by V30 (V30b read reviewer.md:17 and scout.md:11).
- **Homes:** OM:11-13; PC:55; AR:25; BP:111; planner.md:19; reviewer.md:17; scout.md:11; PM (owner of the premise-map shape); CH:43, CH:58; PM:78.
- **Splits:** None (V30: wording shorter, not opposed).
- **Ruling quotes:** None found.
- **Tests:** tests/test_docs_match_code.cpp holds the Premise-map shape (:1509, :1887); no test checks OM.
- **Open:** F30's unchecked parts: I6-156 (`SPSqueeze::needs_no_timing` comment).
- **Hook gap:** none.

### L-F30-05: Does every live step end with ch_stop's restore lines?
- **Source:** I7-085, I7-063; F30 Family D S4; V30 S4.
- **Kind:** K7.
- **Verdict:** CONFIRMED as a rule nothing checks. V30: CH:43 states it; test_docs_match_code.cpp has `ch_stop` once (line 1978, inside a sample Calls line); premise-map.md has it only in its own example Calls line (:61); gates not searched.
- **Homes:** clone-hero-mcp.md:43 "Each live run names its pass and fail lines before it starts, has a cap on how many times it runs, and ends with ch_stop's restore lines."
- **Splits:** None (V30: not a two-answer split).
- **Ruling quotes:** None found.
- **Tests:** none requires a ch_stop step.
- **Open:** none.
- **Hook gap:** none.

### L-F30-06: Does the SP-squeezes skill tell an agent to run hydra_replay without rule 14?
- **Source:** F30 Family A S2; V30 S2.
- **Kind:** K5.
- **Verdict:** REFUTED as a split. V30: SKILL.md:18 and :44 describe what the script `fcvideo.py` does; they do not tell an agent to run hydra_replay. SKILL.md:35 is a table row saying not to launch the GUI / hydra_uitest.
- **Homes:** skills\analyzing-sp-squeezes\SKILL.md:35, :44.
- **Splits:** None (refuted).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** whether an agent running the script would run hydra_replay; only a run settles it. V30 did not read video-tools\fcvideo.py.
- **Hook gap:** none.

### L-F30-07: Which pages are "the pages outsiders read"?
- **Source:** F30 side note 2, Family I; V30 item 4.
- **Kind:** K1.
- **Verdict:** CONFIRMED. They agree today.
- **Homes:** G\review_rules.py:23-24 PUBLIC_DOCS = README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md ("The pages outsiders read (the user's call, 2026-10-10). Each is reviewed once."); PC "the four public pages"; PD:3 (not re-opened by V30).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F30-08: Triage label table against PC's "each label string equal to its name"
- **Source:** I6-237; F30 Family G; V30 item 5.
- **Kind:** K1.
- **Verdict:** CONFIRMED. Agree today; stale if the right-hand column is edited.
- **Homes:** triage-labels.md table (every row's two strings identical; last line "Edit the right-hand column to match whatever vocabulary you actually use."); PC "each label string equal to its name".
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F30-09: A project word means what CONTEXT.md says (five copies)
- **Source:** I6-217, I6-241; F30 Family F; V30 item 6.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED (V30 "NOT REACHED"). See L-F28-15 and L-F29-13 for the same question from other finders.
- **Homes (finder only):** RR:22, RR:34; DOR:51; AR:41; DR:63; BP:99; PC; DM:43, DM:45.
- **Splits:** finder only: RR:34 ("CONTEXT.md wins and the session asks the user") against BP:99/DOR:51 ("change neither"); unverified.
- **Ruling quotes:** None searched.
- **Tests:** none.
- **Open:** all.
- **Hook gap:** none.

### L-F30-10: When to refresh the doc screenshots
- **Source:** I6-251; F30 Family H S5; V30 S5.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED.
- **Homes:** doc-screenshots.md "When to run it": "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows."; docs/development.md:977-983 "Run it before each release" (V30: points to doc-screenshots.md for the rest).
- **Splits:**
  - doc-screenshots.md gives two triggers (release, and a label or layout change); development.md gives one (release). They differ for a label or layout change between releases.
- **Ruling quotes:** None found.
- **Tests:** tests/test_docs_match_code.cpp:2028 `kPicturePages` holds that linked pictures exist, not the run schedule (finder).
- **Open:** none.
- **Hook gap:** none.

### L-F30-11: Public-doc-style: "keep the names of things at the time" against "check every name against the code"
- **Source:** I6-261 to I6-268; F30 Family I S6; V30 S6.
- **Kind:** K2, K3.
- **Verdict:** CONFIRMED.
- **Homes:** PD:77 a release-note rewrite keeps "the names of things at the time, even ones that changed later"; PD:5 "check every name and number against the code before you write it"; PD:13 carves out the app name before 2.0.0.
- **Splits:**
  - PD:77 keeps old labels; PD:5 checks against current code. They differ for an old release note naming a control whose label has since changed. The page does not say which wins.
- **Ruling quotes:** None found.
- **Tests:** none found (finder: no test of em dashes, emoji, first person, headings).
- **Open:** none.
- **Hook gap:** none.

### L-F30-12: Where are the user's words to be looked for (three lists)?
- **Source:** I6-139; F30b; V30b.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED.
- **Homes:** AM:126 (decision log, question files, plans, session cards; no rulings/ folder); PC:41 (only a `## Confirmed` line may be cited); premise-map.md:45 "The quote must sit in a line under `## Confirmed` in a file under `docs/agents/rulings/`, in a session card's quoted words under `docs/sessions/`, or in the user's message as the main session quotes it in the reviewer's brief and confirms."; AM:89, AM:108.
- **Splits:**
  - AM:126 omits rulings/.
  - PC:41 omits decision log, plans, cards.
  - premise-map.md:45 adds the chat-quote path and leaves out the decision log and plans.
  They differ in which places a closure checker and a plan's reviewer search.
- **Ruling quotes:** none names a side.
- **Tests:** none holds them equal.
- **Open:** none.
- **Hook gap:** none.

### L-F30-13: Question-writing rules: always give a recommendation, against never recommend keeping one side
- **Source:** I6-140; F30b; V30b.
- **Kind:** K1, K3, K7.
- **Verdict:** CONFIRMED-CHANGED. V30b adds a home F30b missed: doc-review.md:61 holds its own list for decision questions.
- **Homes:** AM:95 to AM:102 (AM:98 "Name the options without leaning. Then give a recommendation and the strongest case against it."); PC:73; AR:17; doc-review.md:61; memory questions-describe-the-real-rule.md:3,:16; memory counts-on-screen-must-agree.md:21 "Never recommend keeping one side 'unchanged' or 'as today'."
- **Splits:**
  - AM:98 asks for a recommendation on every question; the memory note says never recommend keeping one side. The cases differ (a general question against a split in the code) and AM:98 does not say so.
  - doc-review.md:61 against AM:96-98: two lists for the same question-writing step.
- **Ruling quotes:** questions-and-audits.md `## Confirmed`: "Yes, require it" (rule 11) and "Yes, ban them (Recommended)" (rule 12). Neither backs AM:96-102.
- **Tests:** none; no hook (finder searched G\*.py and lib).
- **Open:** none.
- **Hook gap:** none.

### L-F30-14: What every audit brief carries (AM:118-127)
- **Source:** I6-145; F30b; V30b.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED (V30b: finder wrong that test_posture has no literal; see L-F30-03).
- **Homes:** AM:118-127; AM:26; AM:121-125; PC:33; BP:13; BP:49; AR:14; test-rubric.md:98, :189; "exact ids for a second wave" at AM:127 only.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** see L-F30-03.
- **Open:** none.
- **Hook gap:** none.

### L-F30-15: Who supplies a missing owner: propose one, or stop and ask?
- **Source:** I6-157; F30b; V30b.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED.
- **Homes:** OM:33 "A question can have only one owner."; PC:77 and AR:21 rule 15; memory derive-display-from-engine-truth.md:18 "If none exists, propose one rather than copying."; scout.md:11 "Never propose a new owner."; OM "The stops" "Nobody writes a new owner for it, because a missing answer to a game question is the user's call, not an agent's."
- **Splits:**
  - The memory note says propose an owner; scout.md:11 and OM's stops say nobody writes one. They differ when no owner exists. V30b: the cases differ a little (a calculation in code against a question in a plan) and the note does not say so. V30b: the finder's "rule 15 against OM `none` row" point is text only, a K3 candidate at most.
- **Ruling quotes:** None quoted.
- **Tests:** test_agent_files.py holds planner.md and reviewer.md phrases against the Premise map page only.
- **Open:** none.
- **Hook gap:** none.

### L-F30-16: Who checks the owner map, and what does a reviewer do?
- **Source:** I6-161; F30b; V30b.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** OM:47; reviewer.md:17 (restates OM:47's two steps; adds sign off BLOCKED on a contradiction); planner.md:19; scout.md:11; AR:25; PC:55; BP:111.
- **Splits:**
  - OM:47 against reviewer.md:17: two homes of the same two checks. V30b lists this as a split only in the sense of two homes.
- **Ruling quotes:** None found.
- **Tests:** none holds reviewer.md against OM; no hook checks that a plan has an owner map (finder).
- **Open:** none.
- **Hook gap:** none.

### L-F30-17: Internal docs follow the global "How to explain things": which pages restate it, and with which parts
- **Source:** I6-254; F30b; V30b.
- **Kind:** K1, K7.
- **Verdict:** CONFIRMED. Related to L-F28-13 and L-F29-08, which are NOT VERIFIED.
- **Homes:** PD:3; BP "Plain English" ("That file is the one home of those rules."); doc-review.md:81 (also says the global file is the one home, lists a different set: gloss on first use, short sentences, no wall of bullets, no chat leftovers); the global CLAUDE.md (also "prefer short sentences", "when I ask why give the reason", "analogy before the definition", "cut throat-clearing", BAD/GOOD example); card-writer.md has no plain-English line.
- **Splits:**
  - BP's list, doc-review.md:81's list and the global file are three different sets of the same rule. They differ in which parts a reader of one page sees.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F30-18: Keyboard and mouse off during a Clone Hero run: where the device list and reply keys live
- **Source:** I7-074; F30b; V30b.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED.
- **Homes:** tools/ch_mcp/kit_tools.py:129 `KIT_DEVICES = ("keyboard", "mouse")`; kit_tools.py:533-543 `devices_problem` (refuses unless the read-back is false; reads f"{d}_now"); tools/ch_mcp/agent/kit.ts:199 `type Device = "keyboard" | "mouse"`, :380-381, :425-427, :759-760 (keyboard_now / mouse_now); CH:25, CH:27, CH:47 (finder). CH:25's "both must read back off else setup refuses" holds.
- **Splits:**
  - Device list and `_now` key names in kit_tools.py and in kit.ts. They differ only if one is edited alone.
- **Ruling quotes:** the 2026-10-30 answers are quoted in a kit_tools.py:125-128 comment; the Confirmed lines behind them were not re-read.
- **Tests:** none in tools/ch_mcp/tests mentions KIT_DEVICES or keyboard_now (finder).
- **Open:** none.
- **Hook gap:** none.

### L-F30-19: Auto-player timing words (`retimed=`) and `path_index` default
- **Source:** I7-089; F30b; V30b.
- **Kind:** K1, K2, K6.
- **Verdict:** CONFIRMED-CHANGED. `path_index_refusal` at tools/ch_mcp/game_tools.py:451 is confirmed (refuses a non-integer and a schedule made from another path). Stronger than F30b's "possible K2": tools/ch_probe/experiments/play_chart.py:99 `RETIMED_RANGE = "range"`, applied at :418-438 to every press read outside read_range_ms, asserted in test_play_chart.py:1782-1886; CH:49 names only retimed=late/inside/plain.
- **Homes:** CH:49; game_tools.py:451 (called :1110, :1191); `kept_gems`/`declined` at game_tools.py:219-225; retimed words at play_chart.py:96-99, overlay_logs.py:23,127, tools/ch_mcp/agent/press_schedule.ts; "default 0" at game_tools.py:1074, :1141, server.py:385, :414 and CH:49 (not re-opened by V30b).
- **Splits:**
  - CH:49's word list (late, inside, plain) against play_chart.py's (late, inside, plain, range). They differ for a press read outside the read range.
  - "default 0" in four signatures plus CH:49 (V30b "not reopened").
- **Ruling quotes:** None found.
- **Tests:** none holds the word set equal (finder: only schedule fixtures contain "retimed").
- **Open:** whether the live ch_play_chart path emits `range` in a real run; only a play run settles it.
- **Hook gap:** none.

### L-F30-20: Does DOR:66's `retime_from_end` description match what ch_mcp's kit route runs?
- **Source:** I7-089; F30 Family K.
- **Kind:** K3, K8.
- **Verdict:** NOT VERIFIED (finder: unchecked; see L-F29-14 for the DOR:66 wording itself).
- **Homes (finder only):** CH:49 and :51 (kit agent hooks GameManager.Update; Python sends a `retime` only when an end moves later presses); DOR:66, DOR:108.
- **Splits:** finder only: CH:49 against DOR:66 describe different tools; unchecked.
- **Ruling quotes:** None found.
- **Tests:** test_server.py `test_a_leftover_timing_argument_is_dropped_without_a_word` tests the `timing` argument, not the rule.
- **Open:** all.
- **Hook gap:** none.

### L-F30-21: Where is the live ui-test code built: the `ship` preset leaves out HYDRA_UITEST_ATTACHED
- **Source:** I7-041; F30b; V30b.
- **Kind:** K1.
- **Verdict:** CONFIRMED.
- **Homes:** CMakePresets.json:20-26 preset "ship" sets HYDRA_UITEST_ATTACHED OFF; CMakeLists.txt:593-597 (default ON, "off for the installer"); UT:175, UT:205; docs/adr/0030-hydra-owns-every-live-command-rule.md:139 (not re-opened). They agree today.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none found (V30b did not search).
- **Open:** none.
- **Hook gap:** none.

### L-F30-22: Who checks exit-3 PASS quotes, and with what?
- **Source:** I6-138; F30b; V30b.
- **Kind:** K1 (single home).
- **Verdict:** CONFIRMED (one home). K8 not shown: AM says checkquotes.py "is one", not "owns".
- **Homes:** AM:87; docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py exists.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F30-23: Rows not reached by any verifier (issue tracker, domain docs, this-machine, ui-testing traps, C++ owns every command rule, some audit-method rules, doc-style single homes)
- **Source:** I6-136, I6-212, I6-213, I6-228, I6-233, I6-239, I6-242, I6-262 to I6-266, I7-055, I7-057, I7-060; F30 Families E, G, J, L.
- **Kind:** K1, K8 (not ruled out).
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** TM:3, TM:95; IT:20, IT:44-45; DM:11, DM:49-51; UT:288-290 with ADR 0030:19, tools/hydra_ui_mcp.py:12, tools/test_hydra_ui_mcp.py:5; UT:317-327; UT:360-367; PD single homes. Finder checked I6-212's owner claim true: src/app/path_view.cpp:225 `build_activations` and :529 `build_path_buttons` exist.
- **Splits:** None.
- **Ruling quotes:** rulings/ui-and-display.md:64 (Confirmed, as F30 quotes it) "you created the server, you figure it out"; not re-read by a verifier.
- **Tests:** none.
- **Open:** all; /triage and /wayfinder skills not in ~/.claude/skills.
- **Hook gap:** none.

---------------------------------------------------------------------
## End list
---------------------------------------------------------------------

Candidates the verifier REFUTED (row ids): L-F30-06 (whole). Parts refuted inside other rows: L-F29-17 (F29's "no owner found" for the scan and docs test filter string; V29 found tools/precheck/test_pick.cpp:789-791); L-F30-03 and L-F30-14 (F30b's "test_posture has no literal"); L-F30-01 (S1 reduced from split to plain K1 copy).

Rows no verifier reached (NOT VERIFIED): L-F28-07, L-F28-08, L-F28-11, L-F28-13, L-F28-18, L-F28-19, L-F28-21, L-F29-05, L-F29-07, L-F29-08, L-F29-12, L-F29-18, L-F29-19, L-F29-20, L-F30-09, L-F30-20, L-F30-23.
