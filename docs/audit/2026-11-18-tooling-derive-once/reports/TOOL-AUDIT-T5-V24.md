# TOOL-AUDIT-T5-V24 (verifier of F24). All claims by reading. HR = worktree tooling-audit-records, G = C:\Users\Patrick\.claude\gates.

Scope: I checked the load-bearing homes of groups A, B (owner map, premise-map code home), D, G, and the F-group and side-note claims that rest on them. Not re-opened: group E line by line, ref_judge.py, publish_gate.py, docs/clone-hero/README.md, the decision log, the triage JSON.

## Group A (user's words)
- K1 "Confirmed rule in ~10 places, all agree": CONFIRMED. Seen in CLAUDE.md, agent-rules.md:35-41, derive-once-review.md:45/52/55, doc-review.md:63, premise-map.md:45, test-rubric.md:76.
- SPLIT A1: CONFIRMED-CHANGED. Lists differ as claimed (DOR:52 adds D-record, "User decisions", brief's words; DR:63 = Confirmed, card, brief's quote; PM:45 = Confirmed, card, user message quoted in brief and confirmed by the main session; TR:76 = only "a line under `## Confirmed` ... that names the value"). Change: DOR:45 does not omit the card. Its first sentence lists ADR, CONTEXT.md, Confirmed, "User decisions"; its later sentence says a decision entry "counts only when it quotes the user's words as they are found under `## Confirmed` or in a session card". Situation where they differ: a reviewer at step 4 vs step 9 vs a test reviewer (TR:76 has no card). AM:126 omitting the rulings folder is true (it lists decision log, question files, plans, cards).
- SPLIT A2: CONFIRMED-CHANGED, weaker. DOR:52 says to quote the user's words "and say where they are: a D-record, a plan's "User decisions", a line under `## Confirmed`..., a session card". AR:41 says a decision entry "counts only when it quotes the user's words as found under `## Confirmed` or in a session card". These can be read as one rule. DOR:52 does not say the D-record must itself quote a Confirmed line or card. A wording gap, not a clear contradiction.
- SPLIT A3: CONFIRMED-CHANGED. CLAUDE.md "Past sessions" says a card keeps the user's quoted words, "a card quote is not a filed ruling", and "A card is a pointer, not proof: check what it says against the code". writer-brief.md:67 says the section holds the user's exact words, "not filed rulings: a ruling is only a line filed under `## Confirmed`". DR:63, DOR:52, DOR:55 and PM:45 accept a card's quoted words as the user's ruling words; PM:52 says a card is only the place a ruling's quote sits. "Pointer, not proof" is about claims on code, so the live split is "a card quote is not a filed ruling" against "a ruling counts when it quotes ... a session card".
- Confirmed lines naming both sides of A1 to A3: none. review-and-hooks.md:19 (under `## Confirmed`, which starts at :7; `## Unconfirmed` at :37): "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." It covers sign-off only. The finder's claim that questions-and-audits.md:28 is Unconfirmed was not re-opened.

## Group B
- Owner-map shape held by instruction only (K7): CONFIRMED by search. I searched HR tests, tools, src, .claude for "Owner map" with no hit, and G/*.py for `O1:` and `carry out` with no hit. owner-map.md:21-29 gives the heading, four columns, `O1:` ids, statuses `found`, `split`, `carry out`, `none`.
- K1/K5 (rules restated in planner.md:19, reviewer.md:17, scout.md:11): CONFIRMED. scout.md:11 restates the game-words rule, quote-the-comment, `carry out`, `none`, "Never propose a new owner"; it names two of the four status words. A partial copy, no disagreement.
- Premise-map code home: CONFIRMED. tests/test_docs_match_code.cpp:1520 `kPremiseMapSince = "2026-11-01"`, :1525 `kDeferredPremiseSince = "2026-11-13"`, :1528 `kQuestionsHeading`, :1529 `kLiveStepsHeading`, :1530 `kKindWords[] = {"code","data","chart","run","ruling","unchecked"}`. premise-map.md:37-46 lists the same six. Nothing holds the page's list to the test's list (K1/K6). premise-map.md:76 says "the comment above it says which plans count"; the dates are in the test only (S1 CONFIRMED).
- I6-173 (four Clone Hero labels copied at PM:42): OPEN. Needs a read of docs/clone-hero/README.md; not reached.

## Group D (sign-off)
- K1 CONFIRMED. Homes: derive-once-review.md:146-151 ("Write no hash, key or path with it. The hook reads nothing else in your report."); reviewer.md:28-31 ("Write no hash, key or path after it; nothing else in the report is read."); G/README.md:43 ("a whole line that is exactly `SIGN-OFF: BLOCKED`... A report with neither line is recorded CLEAN too"); G/agent_stop.py `verdict_word` (`lines = {s.strip() for s in report.splitlines()}`; BLOCKED if the exact string is a whole line, else CLEAN if so, else CLEAN with a missing-line problem).
- S5 hook gap: CONFIRMED. A line `SIGN-OFF: BLOCKED` followed by any text is not an exact whole line, so it matches neither and the report is recorded CLEAN. Gap exists in agent_stop.py `verdict_word`. The Confirmed line above covers "no sign-off line records CLEAN"; it does not name the added-text case.
- S4 (reviewer.md:31 vs :33): both sentences are there. They do not contradict by reading (verdict word from the report, tip and fingerprints from git) but the wording invites misreading. Not a split.
- Report section lists (DOR vs doc-review): only the DOR sign-off section re-read; the rest not re-verified.

## Group G
- TOOL_TAIL "five": CONFIRMED (K1). G/stall.py:34 `TOOL_TAIL = 5`, used :207. Doc copies: brief-preamble.md:23 area and model-postures.md:11 ("its last five tools"). No test for the doc copy found (test_docs_numbers.py not opened).
- 2-second lock wait: CONFIRMED (K1). common.py:18 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`; README:9.
- Eight refusal fields: CONFIRMED (K1/K6). README:81 lists eight; common.py log_refusal writes time, hook, event, tool, session, agent, decision, reason[:300]; tests/test_refusals.py has a `FIELDS` set compared with `set(line)`. The test is a third copy.

## Group F and side notes
- F1, F2: CONFIRMED-CHANGED as readings. BP:3 lists task id, session id, owned files, worktree or checkout, scratch folder, tests; AM:120 lists task id, session id, files or batch, caps, status rule. Different readers, no pointer between them. F1 (AM:50 vs the finder brief) not re-opened.
- S2 (memory note): CONFIRMED-CHANGED. The note's description line says "the dispatch gate now denies briefs that lack it". My grep for "No gate" or "gate checks" in the note found only its line 15; I could not confirm the finder's quoted body sentence. G/README.md:39 names review_dispatch.py for reviewer briefs only, so a gate for the status sentence is unconfirmed. The Confirmed line "i don't care if an agent is read only, no status updates is unacceptable" is at agents-and-workflow.md:25 (Confirmed starts :7, Unconfirmed at :32). It covers the status rule, not the brief-list split.
- S3 (hash lengths): CONFIRMED. tools/check_handoff.py:28 exactly 40 hex; tools/doc_tokens.py:39 and :50 7 to 40; G/review_rules.py:56 40 or 64 hex (`{40}([0-9a-f]{24})?`). review_dispatch.py `_KEY` pattern not re-opened.

## Splits (own lines)
- A1 (CONFIRMED-CHANGED: DOR:45 does not omit the card)
- A2 (CONFIRMED-CHANGED, weaker)
- A3 (CONFIRMED-CHANGED: "a card quote is not a filed ruling" vs "a ruling counts when it quotes a session card")
- F1, F2 (as above)
- New splits added: none.

## Not reached
Group E rows (I6-091 to I6-102) beyond the hash-length check; I5-017 single-home claim beyond CLAUDE.md and writer-brief.md; I6-173; I6-142; I5-098 and I5-099 beyond reading BP:3 and AM:120; test_docs_numbers.py coverage of TOOL_TAIL; ref_judge.py and publish_gate.py calls to log_refusal.
