# TOOL-AUDIT-T5-V18 (verifier of finder F18). All claims "by reading"; I ran nothing.

## Candidate 1: whole-library run phrase (docs say "exactly", hook is tolerant; memory note says no hook) - CONFIRMED
Homes opened: agent-rules.md:14 and brief-preamble.md:49 ("sending exactly `allow library run`; each such message lets one command through"). review_rules.py:51-52 and :81-82: `LIBRARY_ALLOW_TEXT = "allow library run"`, `_LIBRARY_ALLOW = re.compile(re.escape(...) + r"[.!]?", re.IGNORECASE)`, `fullmatch(text.strip())`. percall.py:45-55 refuses a shell call holding the flag and says "they allow one run by sending exactly: allow library run". Memory no-full-library-runs.md:15: "No hook enforces it yet."
Situation: the user types "Allow Library Run!" (or with spaces round it). Docs say that is not the phrase; the hook lets it through.
Splits:
- S1: docs "exactly" vs review_rules.py:52 (case ignored, one trailing . or ! allowed, trimmed).
- S2: memory note "No hook enforces it yet" vs percall.py:45-55 (the hook refuses). The note is stale. By reading, the note is not the user's words.
Ruling: tests-and-runs.md:14 and review-and-hooks.md:16, both under Confirmed (review-and-hooks.md line 16 is before its `## Unconfirmed` at :37): "i want to be able to give explicit approval for a full library run." It names the approval, not the exact spelling, so it does not name both sides of S1. Nothing in the memory note's side (S2) is a user ruling.
Side: percall.py LIBRARY_FLAG vs Hydra kWholeLibraryFlag: I did not open batch_scope.h; not checked.

## Candidate 2: Hydra marker files name retired .ps1 hooks - CONFIRMED
Hydra `.derive-once-gate` line 3 names "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; `.doc-review-gate` line 3 names "(hooks\lib\doc_review_rules.ps1)". Glob of C:\Users\Patrick\.claude\hooks\** finds only state\build_slots\slot1.lock: no hooks\lib folder, no .ps1 rules. The live owner is review_rules.py:22 BUILTIN_CODE and :24 PUBLIC_DOCS (used at :184, :448). Also copied into several worktree copies of the markers. Kind K8 (a comment naming a wrong owner). Split S3: marker comment vs code owner.

## Candidate 3: "markdown" vs DOC_SUFFIXES - CONFIRMED
agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`, used at :172 (and the notice at :226). reviewer.md:33 "fingerprints every markdown file the range changes"; derive-once-review.md:151 same words; doc-review.md:29 "documents are every markdown file the range changes". Situation: an .html or .htm page in the range. Split S4: three docs say markdown only; the hook also takes html. I did not open gates/README.md:49 (finder says it says "a markdown file or an HTML page"); unchecked.

## Candidate 4: "Holds the merge:" Note vs BLOCKED, and the three-case exception - CONFIRMED-CHANGED
derive-once-review.md:106: Notes do not hold the merge "with three exceptions" (visible change marked "not asked", unfixable kind H, unfixable kind D); the last two start "Holds the merge:"; "You still sign off CLEAN ... the orchestrator holds the merge, not your sign-off line." derive-once-review.md:136 repeats that a `Holds the merge:` Note "is the one kind that does wait". But :149 and reviewer.md:31 say BLOCKED "when a finding stays that you could not fix and that must hold the merge", and :151 "Only `SIGN-OFF: BLOCKED` holds the merge." So :151 contradicts :106 and :136 inside one page; the finder cited only :149. I did not open the hook's reading of Notes beyond agent_stop.py:23 (it names only the two sign-off strings); no code read of a "Holds the merge" prefix found in agent_stop.py by my grep for SIGN-OFF only, so "no hook reads the prefix" is not fully proven (I searched agent_stop.py only).
Splits:
- S5: DOR:106 (sign off CLEAN, hold by Note) vs DOR:149 and reviewer.md:31 (write BLOCKED when it must hold the merge).
- S6: DOR:151 and doc-review.md:125 "Only SIGN-OFF: BLOCKED holds the merge [or the publish]" vs DOR:106/:136 (a Note holds it too).
- S7: agent-rules.md:11 "the merge does not wait for [Notes]" and "nothing goes back to the user" vs DOR:106 (three held cases, questions go to the user).
- S8: memory one-reviewer-rule-d107.md:11 "pass them on, don't hold the merge" (no exceptions, no D314) vs DOR:106. The note's own description says "nothing back to the user", same words as DOR:90 and agent-rules.md:11.
- S9: doc-review.md:98 "It does not hold the publish for them" (no exceptions) vs DOR:106 when one reviewer covers code and docs.
Ruling: see candidate 5.

## Candidate 5: citation "alright approve b" (new finding, finder said it could not find it) - CONFIRMED-CHANGED
derive-once-review.md:106 says "The user ruled this on 2026-11-12 ("alright approve b")". The words are in docs/agents/rulings/review-and-hooks.md:39, which sits under `## Unconfirmed` (heading at :37; `## Confirmed` is at :7). So DOR:106 cites an unconfirmed line as the user's ruling. The session card docs/sessions/2026-11-13-bed936f0.md:23 quotes it as "approves option b: a finding a reviewer cannot fix holds the merge". The words do not say CLEAN vs BLOCKED, so even on the card they do not name both sides of S5/S6. Not a Confirmed ruling.

## Candidate 6: "to the user" vs "to the executor" (model-postures.md:5) - CONFIRMED
model-postures.md:5: "Nothing goes back to the executor and no second round (D107), except that a reviewer's fix that fails your build goes to a fresh executor". agent-rules.md:11 and derive-once-review.md:90: "nothing goes back to the user". Different words, not the same sentence: the executor half is about the code author, the user half about findings. Split S10 (K3, wording). By reading, the situation: a Note with a question for the user (goes to the user, against DOR:90's words).

## Candidate 7: precheck "finds kind F mechanically" vs "candidates" - CONFIRMED-CHANGED (partly unchecked)
DOR:21: "It finds kinds B, C, E and F mechanically, and kind D's new numbers." I read DOR:38 only as a truncated match (the long line was omitted); I did not read its text, so I cannot confirm the finder's quote. Open: read DOR:38 in full. Marked unchecked, no split added.

## Candidates 8-10 (publish kinds, "File it" question text/date, hook shared-test list, earlyStart)
- Publish kinds: CONFIRMED in part. publish.ps1:8 ValidateSet is the six kinds; publish_gate.py:14 owns `SMALL_LINES, SMALL_CHARS = 20, 2000`; publish_gate.py:5 calls itself "the one home". publish.ps1:3 says "small-text rule". I did not read README.md:63/65, doc-review.md:17-21 (long lines omitted), so the other copies of the six kinds and "short-text" naming are unchecked.
- "File it": CONFIRMED. ruling_rules.py:99-100 `INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`. rulings/README.md:41 writes out the question `File this ruling in docs/agents/rulings/<topic file>?` and both labels; :51 is a long line I did not read, so the date copy there is unchecked. No code compares README:41 to the constants (by reading; I did not open the test file). Kind K1, no split.
- Hook shared-test list (K7) and earlyStart (K7): NOT REACHED.

## Not reached
Hook shared-test list (agent-rules.md:7, model-postures.md:11, README.md:156); earlyStart; I5-135, I5-137, I5-170, I5-171, I5-191, I6-013/015/016/024/026 (finder did not reach them either); family 8 (user's words, DOR:45 vs :52 vs PM:45 vs DR:63); families 10-12; the percall LIBRARY_FLAG vs kWholeLibraryFlag spelling.

## Splits added beyond the finder
- S6 (DOR:151 and doc-review.md:125 vs DOR:106/:136).
- Candidate 5: DOR:106 cites an Unconfirmed line as "the user ruled".
