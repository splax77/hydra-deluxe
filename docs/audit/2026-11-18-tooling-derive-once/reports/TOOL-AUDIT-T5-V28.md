# TOOL-AUDIT-T5-V28 (verifier of F28). All claims by reading; nothing run.
G = ~/.claude/gates, H = Hydra main checkout, AR = H/docs/agents/agent-rules.md. AR line numbers in F28 do not all match what I saw (rule 3 is line 9, rule 14 line 20, hooks list line 27, CONTEXT.md paragraph line 43); I cite what I read.

## Verdicts
1. Build limit "three" typed in many places (K1): CONFIRMED. build_slot.ps1:75 `$BuildSlotCount = 3`; AR:20 "At most three builds run at once"; CLAUDE.md rule 14 "At most three builds run at once". Not re-opened: BP:57, development.md.
2. Tool name (K1): CONFIRMED. percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`; hydra_ui_mcp.py:874 `def hydra_build_pass()`; AR:20 names it. The prefix is server name plus tool name, so the string is built by hand from two other places.
3. Three wordings of the refusal (K1/K3): CONFIRMED. percall.py:109 "Agents never build, so hydra_build_pass is refused to every agent..."; build_slot.ps1:87 `$BuildPassHowTo = 'Agents never build: hand back, and the main session builds...'`.
4. 500K (K1): CONFIRMED. stall.py:27 `CTX_THRESHOLD = 500000`; stall.py:36 text "fired at 500K (50% of the window)"; model-postures.md:15 "under 500K tokens".
5. K4, "50% of the window" is a fact no code reads: CONFIRMED. It is a literal in stall.py:36 and does not come from the constant. I did not re-search settings.json.
6. 660 timeout (K1): CONFIRMED that settings.fragment.json:235 has `"timeout": 660`. I did not open test_wake.py or the live settings, so "pinned by a test" is carried from F28, unchecked.
7. Stall numbers 10/20/30 (K1): CONFIRMED in part. stall.py:21-23 `STALL_SECONDS = 600`, `LIMIT_SECONDS = progress.MINUTES_STOP * 60`; AR:27 states "10 minutes" (the rest of line 27 was cut in my view; 20/30 not re-read). progress.py not opened.
8. Catchers (K1): CONFIRMED, with a path correction. Files are ~/.claude/tools/session_cards/check.py:36 `CATCHERS = ("main session", "hook notice", "user")` and writer-brief.md:91 (same three phrases); test_session_cards.py:486-488 reads `check.CATCHERS`. F28 wrote H\tools\session_cards; that folder does not exist in H. The writer-brief copy is a typed list; nothing ties it to CATCHERS.
9. K5 model-postures.md:11 spells out what STATUS_HEADER says: CONFIRMED. stall.py:30-33 and model-postures.md:11 give the same relay-briefly / second-sentence-only-on-disagreement instruction.
10. K6 crash handling in common.run and two .ps1 hooks: NOT REACHED.
11. K7 [write-back] duty only in CLAUDE.md: CONFIRMED. session_cards_start.ps1 has 0 matches for "write-back". (writer-brief.md:64 and :81 carry the `write-back:` line the writer writes; the next main session's duty is CLAUDE.md only.)
12. K8 .derive-once-gate and .doc-review-gate name `hooks\lib\derive_once_rules.ps1` / `doc_review_rules.ps1`: CONFIRMED. Both marker files say so; ~/.claude/hooks holds only `state`; ~/.claude/hooks/lib does not exist.
13. K8 plan files not found: CONFIRMED. wake.py:5 cites docs/superpowers/plans/2026-11-16-agent-supervisor.md; nothing in H/docs/superpowers/plans matches "supervis" and ~/.claude/plans has none. check.py:34-35 says "the supervisor plan" with no path (a pointer to a doc I could not find).
14. AR user quotes (rules 15 and display) not under `## Confirmed`: CONFIRMED. grep for "correct rule is to use", "invent new rules", "invents its own rule" in docs/agents/rulings/*.md: no hits. In agents-and-workflow.md, Confirmed starts line 7, Unconfirmed line 32. "Yes, record it" (cited in check.py) also has no hit in rulings.

## Splits (each a separate line)
A. CONFIRMED. model-postures.md:11 "nudge it once with SendMessage ... If it does not recover, stop it with TaskStop" against stall.py:124-125 and :143-144 "If an agent is stuck, stop it with TaskStop and re-dispatch its remaining work" (no nudge). Situation: a stuck agent after a stall or past-limit notice. stall.py:460 (check-in) says "nudge it or stop it". Rulings: agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" and :25 do not name nudge vs stop; no Confirmed line names both sides.
B. CONFIRMED. AR:9 "In this rule a build means any piece of work"; AR:20 "In this rule a build means compiling, not rule 3's piece of work." model-postures.md:3 uses rule 3's meaning, :9/:11 the compile meaning. The ruling at agents-and-workflow.md:8 uses "builds" for compiles. No ruling on the word in rule 3.
C. CONFIRMED-CHANGED. CLAUDE.md:1 "Ask questions if something is unclear." The other side F28 names is a memory note (not opened by me, and a note is not the user's words). AR:23 is narrower than F28 says: it bars a plan from putting a rule it made up to the user as a question; it is not a game-or-permission-only limit. No Confirmed line seen that names both sides (questions-and-audits.md:9 and :15 not re-read).
D. CONFIRMED-CHANGED. My grep did not find "CONTEXT.md decides" in AR. AR:43 says: "The code's usage settles the meaning ... If a definition's text and the code's usage disagree, that is a question for the user, in game terms with both readings". So the same paragraph already routes the disagreement to the user. A split only if AR:41 does carry "CONTEXT.md decides" (re-open AR:41).
E. (missed by finder) stall.py:134 skips when `now - first <= LIMIT_SECONDS`, so the past-limit notice comes only after more than 30 minutes; AR:27 says "30 minutes after its first call gets one more notice". One-edge difference between prose and code.

## Side notes confirmed
- build_slot.ps1:78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'`; hooks live in gates and `hooks` holds only `state`. Whether that path is live is open; a run of build_slot.ps1 would settle it.
- Hook gap: percall.py:108 refuses by `tool_name` only; AR:27 says build_cpp.ps1 is the second stop.

## Not reached
Q5 K6 (common.run vs the two .ps1 hooks), Q6 housekeeping file names, Q9 plain-English copies (BP:121-123, writer-brief:94 not opened), Q12 rule-copy table beyond rules 3 and 14, Q13 BP:41 / delete_rule, the "tests hold homes equal" claims (test_wake.py, test_docs_numbers.py, test_build_pass.ps1 not opened), memory-note quotes.
