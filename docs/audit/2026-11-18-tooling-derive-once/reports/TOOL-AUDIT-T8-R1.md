# TOOL-AUDIT-T8-R1 (themes 11, 12, 13)

Paths: Hydra docs are under C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\ (main). Hooks repo is C:\Users\Patrick\.claude. No T7 report mentions any of these 14 ids (grep of reports\TOOL-AUDIT-T7-C*.md found none), so none was reopened by T7.

### L-F28-15
**Verdict**: REAL (weak: the two lines mostly agree, and differ only on the case where text and code disagree).
**Homes read**:
- agent-rules.md:41 "`CONTEXT.md` decides what a project word means."
- agent-rules.md:43 "The code's usage settles the meaning, not a guess drawn from one sentence of a doc. If a definition's text and the code's usage disagree, that is a question for the user ... and the agent changes neither the text nor the code to match the other."
- rulings/README.md:34 "If the reading and `CONTEXT.md` disagree, `CONTEXT.md` wins and the session asks the user."
- brief-preamble.md:99 repeats the line-43 version ("change neither").
**Kind**: doc-vs-doc.
**What changes**: when a word's text and the code's use disagree, line 41 and the rulings README say CONTEXT.md wins (and ask); line 43 and the preamble say neither side wins and the user is asked, so an agent reading a ruling uses the CONTEXT.md meaning on one reading and holds both readings open on the other.

### L-F29-13
**Verdict**: REAL, the same split as L-F28-15 (same homes, same question; one question to the user covers both).
**Homes read**: agent-rules.md:41 and :43, as above.
**Kind**: doc-vs-doc.
**What changes**: same sentence as L-F28-15.

### L-F30-09
**Verdict**: REAL for the CONTEXT.md-wins versus change-neither part (same split as L-F28-15). The domain.md:11 side is NOT A SPLIT.
**Homes read**:
- rulings/README.md:34 "`CONTEXT.md` wins and the session asks the user."
- brief-preamble.md:99 "if the definition's text and the code's usage disagree, ask the user in game terms with both readings and change neither."
- derive-once-review.md:51 is the same text as agent-rules.md:41 ("`CONTEXT.md` decides what a project word means").
- domain.md:11 only says the `/domain-modeling` skill "creates them lazily when terms or decisions actually get resolved". It answers whether to create the files, not who changes a term's meaning, so it does not conflict with "only the user changes a term" (agent-rules.md:43).
**Kind**: doc-vs-doc (README/DOR:51 "wins" against preamble:99 and agent-rules:43 "change neither").
**What changes**: as L-F28-15.

### L-F28-18
**Verdict**: NOT A SPLIT (reconciled in the text).
**Homes read**: agent-rules.md:21 "If none does, the new owner goes in core". agent-rules.md:25 right after it: "Under an owner map, 'if none does, the new owner goes in core' above waits for the user: a question with no owner goes to the user first, and only a value the engine already works out and keeps unreadable is carried out." owner-map.md:37 "Nobody writes a new owner for it". CLAUDE.md rule 15 (the project file) never says "new owner goes in core"; it says "name the search that found none", and its owner-map paragraph says a row with no owner "stops the work and goes to the user".
**Why**: line 21 covers ordinary code and briefs, line 25 says outright which one wins once an owner map is in play, and owner-map.md:37 agrees with line 25.

### L-F30-04
**Verdict**: NOT A SPLIT, for the same reason as L-F28-18.
**Homes read**: agent-rules.md:21 and :25 (above); owner-map.md:37; C:\Users\Patrick\.claude\agents\scout.md:11 "Mark a question with no answer anywhere `none`... Never propose a new owner." The "carve-out missing from CLAUDE.md" is not a conflict: CLAUDE.md does not state the "goes in core" side at all, so it has nothing to carve out of.

### L-F30-15
**Verdict**: REAL, one side only (the memory note against the docs).
**Homes read**:
- C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\derive-display-from-engine-truth.md:18 "Before writing any calculation, find its one owner and call it. If none exists, propose one rather than copying."
- scout.md:11 "Never propose a new owner."
- owner-map.md:37 "Nobody writes a new owner for it, because a missing answer to a game question is the user's call, not an agent's."
- CLAUDE.md rule 15 (last sentence): "A rule a plan makes up is never put to the user as a question."
**Kind**: memory-vs-other.
**What changes**: a session that follows the memory note proposes an owner when none exists; a session that follows scout.md, owner-map.md and rule 15 sends the "none" row to the user without a proposal. The memory note is read by the main session and scout.md by scouts, so a main session can do the thing the scout was told not to do.

### L-F31-01
**Verdict**: REAL for the memory note; the agent-rules.md:21 side is reconciled by agent-rules.md:25 (see L-F28-18).
**Homes read**: memory derive-display-from-engine-truth.md:18 "propose one rather than copying" against owner-map.md:37 and scout.md:11 (quoted above).
**Kind**: memory-vs-other (same split as L-F30-15; one question covers both).
**What changes**: as L-F30-15.

### L-F18-02
**Verdict**: REAL.
**Homes read**:
- derive-once-review.md:106 for a kind H or kind D finding the reviewer cannot fix: "Start the Note for either of the last two with the words `Holds the merge:`. You still sign off CLEAN ... the orchestrator holds the merge, not your sign-off line."
- derive-once-review.md:149 "Write `SIGN-OFF: BLOCKED` instead only when a finding stays that you could not fix and that must hold the merge."
- C:\Users\Patrick\.claude\agents\reviewer.md:31 "Write `SIGN-OFF: BLOCKED` instead when something you could not fix must hold the merge."
- Hook: C:\Users\Patrick\.claude\gates\agent_stop.py:116 reads only `SIGN-OFF: BLOCKED`; a search of the gates folder for "Holds the merge" found no reader (only two old workflow JSON files).
**Kind**: doc-vs-doc (the hook-side gap is that no program reads the `Holds the merge:` prefix; only the main session reading Notes holds the merge).
**What changes**: an unfixable kind H finding signed CLEAN with a prefixed Note is held only if the main session reads the Note; signed BLOCKED it is held by the ref gate (review_rules.py:238).

### L-F18-04
**Verdict**: NOT A SPLIT.
**Homes read**: model-postures.md:5 (hooks repo) "Nothing goes back to the executor and no second round (D107)". agent-rules.md:11 "no second round, and nothing goes back to the user (D107)", then "the main session passes those Notes to the user".
**Why**: one line says findings do not return to the executor, the other that the review does not return to the user for another round; both mean "no second round" and neither tells anyone to act differently. The real tension with agent-rules.md:11 is Notes that hold the merge (derive-once-review.md:106), which is the L-F18-02 and L-F19-01 question.

### L-F18-12
**Verdict**: REAL (low stakes).
**Homes read**:
- derive-once-review.md:21 "It finds kinds B, C, E and F mechanically".
- derive-once-review.md:38 "kind F's candidates ... For kind F it lists only candidates ... step 8 below is where you judge them"; :49 "The precheck's F lines list the candidates."
- tools/precheck/check_5.cpp:30 the F line's own text: "a second home for an idea that already has one".
**Kind**: doc-vs-doc (line 21 against 38/49); the program text also states the F line as fact, not as candidate (doc-vs-code).
**What changes**: a reviewer who reads line 21 may quote a precheck F line as a finding; one who reads line 38 judges it first and may drop it. I did not look at the "CLEAN versus BLOCKED" part of the row; it is L-F18-02.

### L-F18-20
**Verdict**: REAL.
**Homes read**:
- derive-once-review.md:38 "A change that adds an entry to `known_clones` is a finding."
- docs/adr/0029-the-single-owner-rules-are-data.md:134-142 "Every entry also carries the day it was added, who added it, and a `remove_by` date ... The dated entry lets an agent that can't edit a file another task owns still land its change, while the clock runs."
- tests/single_owner/engine.cpp:207 accepts `known_clones` as a baseline key; tests/single_owner/baseline.json:3 holds `"known_clones": []` today.
**Kind**: doc-vs-doc (review page against ADR); also doc-vs-code in that the loader accepts dated entries.
**What changes**: a reviewer who follows the review page rejects a dated clone entry; the ADR and the loader let one in for up to 14 days.

### L-F19-01
**Verdict**: REAL.
**Homes read**:
- memory one-reviewer-rule-d107.md:11 "Findings that would need the user (new number, visible/stored change) go in the reviewer's Notes; pass them on, don't hold the merge".
- derive-once-review.md:106 holds the merge for a "not asked" visible change, an unfixable kind H and an unfixable kind D rule.
- doc-review.md:98 "It does not hold the publish for them." (a different subject: publishing documents; no carve-out there).
- The ruling DOR:106 cites ("The user ruled this on 2026-11-12 ('alright approve b')") sits at docs/agents/rulings/review-and-hooks.md:39, which is under `## Unconfirmed` (header at line 37), not `## Confirmed`.
**Kind**: memory-vs-other (memory against DOR:106); claimed-ruling (DOR:106 says the user ruled; the only filed line is Unconfirmed). The doc-review.md side is not a split: it governs publishing, and DOR:106 governs merges.
**What changes**: on the memory note no Note ever holds a merge; on derive-once-review.md three kinds of Note do.

### L-F24-09
**Verdict**: REAL (a gap with two competing lists).
**Homes read**:
- derive-once-review.md:130-139 report sections: Questions this change answers, Findings, (precheck lines), Visible changes, Precheck lines dropped, Notes, Touched, Scan rows added, Where I looked.
- doc-review.md:110-115: Restatement, Findings, Numbers checked, References opened, Notes, Where I looked.
- derive-once-review.md:90 says that when the range changes documents "you follow [doc-review.md] for every document"; doc-review.md:123 and derive-once-review.md:151 agree that one SIGN-OFF line signs both. Neither page says which section list a combined report uses.
**Kind**: doc-vs-doc.
**What changes**: a reviewer of a range with code and documents writes either one list, both, or a merge of the two; nothing says which, so reports differ.

### L-F31-03
**Verdict**: NOT A SPLIT.
**Homes read**: integrate.md:23 "If a fold would change a displayed text, a score or a stored record, stop and report it. Do not pick." derive-once-review.md:96 "Fold every copy into the owner" is inside step 3, and derive-once-review.md:106 says a change to a displayed text, a score or a stored record is not fixed by the reviewer: "Do not make those changes. Write the finding under Notes as a question for the user." The Confirmed line at rulings/agents-and-workflow.md:19 ("if anything changes a display in the app or a record, run it by me") covers both.
**Why**: integrate.md covers the integrator's join and the review page covers the reviewer; both stop on a display or score change.

## Ids not reached
None.
