# TOOL-AUDIT-T7-C12: closure check, sorting file TOOL-AUDIT-T6-S06.tsv, ledger TOOL-AUDIT-T5-L06.md

Paths: hooks repo = C:\Users\Patrick\.claude (gates\, agents\, tools\session_cards\); Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, current files).
Totals: PASS 6, REOPEN 1, GONE 0. Not reached: none.

### L-F18-06
**Verdict:** PASS (category P)
**Homes read:** gates\review_dispatch.py:16-17 (`_KEY`, 40 hex standing alone; comment says a 64-hex doc key is not a review key), :21 (`Review range` line, 40 hex); gates\review_rules.py:26-28 (waiver phrases 7-40, 7-64, 7-40), :56 (`_HEX` 40 or 64, used at :423 for raw ref values), :475 (`doc_key` is a document fingerprint, a different thing); gates\ref_judge.py:235 (40 or 64 for object ids); gates\README.md:39 ("40-character hash of the commit to review"); Hydra docs\agents\derive-once-review.md:17 ("key (a 40-character hash)"); doc-review.md:29 ("40-character hash of the branch's tip"); handoff.md:19,:35,:51 (full 40-character commit hash); .claude\commands\handoff.md:11-15; .claude\agents\reviewer.md and model-postures.md searched for "40", "hash", "key": no length claim (reviewer.md:31 only says write no hash after SIGN-OFF).
**Why:** Every prose home that gives a length for the review key says 40, which is exactly what `_KEY` accepts. The 7-64 and 40-or-64 patterns belong to other jobs (waiver prefixes, raw git ref values, document fingerprints) and no doc states them differently. A pointer in place of the repeated "40-character" would change no behaviour and needs no choice.

### L-F18-07
**Verdict:** PASS (category C)
**Homes read:** Hydra .derive-once-gate:3 ("Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"); .doc-review-gate:3 ("hooks\lib\doc_review_rules.ps1"); gates\review_rules.py:18-24 (CODE_MARKER, DOC_MARKER, BUILTIN_CODE "Code paths for a repo whose marker has no gate: lines (Hydra's holds only prose)", PUBLIC_DOCS); gates\README.md:138; Hydra doc-review.md:17 (names the same four pages as PUBLIC_DOCS); Glob/ls of C:\Users\Patrick\.claude\hooks: only `state\`, no `lib\`, no .ps1 files.
**Why:** The two marker comments name .ps1 files that no longer exist. The current owner is clear: review_rules.py BUILTIN_CODE (:22) for code paths and PUBLIC_DOCS (:24) for the pages; the gated steps are in doc-review.md:17 and publish_gate.py. Both sides of the split are a comment and a moved or missing file, so C holds. Note for the fixer: the marker is itself gated (every change to it needs its own review, review_rules.py:476), which is a process step, not a reason to ask.

### L-F18-08
**Verdict:** PASS (category P)
**Homes read:** gates\publish.ps1:1-8 (header "small-text rule"; ValidateSet of six kinds); gates\publish_gate.py:1-5 ("the small-text rule has this file as its one home"), :13-14 (`SMALL_LINES, SMALL_CHARS = 20, 2000`); gates\review_rules.py:67 area (searched "small"); gates\README.md:63 ("the one home of the short-text limit"), :65 (the six kinds: pr, release, release-edit, issue, issue-comment, pr-comment); Hydra doc-review.md:17 ("A short text passes the last three with no review; publish_gate.py ... owns that limit"), :19, :21 (the same six kinds); CLAUDE.md searched for "short text", "small text": no match.
**Why:** The six kind names are identical in all three homes. No document repeats the 20/2000 numbers; each points at publish_gate.py. The only difference is the name "small-text" in code against "short-text" in docs, which is wording. Using a pointer instead of the copied kind list needs no choice.

### L-F18-09
**Verdict:** PASS (category P)
**Homes read:** gates\ruling_rules.py:97-101 (`INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION = "File this ruling in {path}?"`); Hydra docs\agents\rulings\README.md:3 (File it), :41 ("File this ruling in docs/agents/rulings/<topic file>?" and the two labels), :51 ("INSTALL_DATE in ruling_rules.py, 2026-11-14"); agent-rules.md:33 ("File it"); gates\README.md:59 (points at ruling_rules.py).
**Why:** Labels, question wording and date are the same in code and every doc copy. The README's `<topic file>` stands for the `{path}` placeholder. Nothing disagrees. (The :394 comment about other gates was a separate F18 note and is not a split in this row.)

### L-F18-14
**Verdict:** REOPEN (should be a question, or the A part split from one doc-against-doc piece)
**Homes read:** gates\progress.py:13-15 (100/150, 20/30, "The one home of these numbers"), :92-100 (warn at 100 calls / 20 minutes: "Start wrapping up: commit what passes and report what is unfinished"), :129-133 (refuse at 150 / 30); agents\reviewer.md:11 ("at 100 calls or 20 minutes you stop fixing", and "each with the plan line it breaks (CLAUDE.md rule 5)"); tools\session_cards\writer-brief.md:101 ("At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report"); model-postures.md:5; Hydra agent-rules.md:13 (rule 7); brief-preamble.md:71,:73; derive-once-review.md:110 ("Write each finding you did not fix under Notes, with what is left to do").
**Why:** Two of the splits hold under A: "100 calls" against "100 tool calls" (agent file against progress.py's wording), and writer-brief.md:101 against the hook's warn-then-refuse (page against progress.py). The third split in the ledger row (finder note, second bullet) does not: reviewer.md:11 tells a reviewer who hits the budget to list each unfixed finding with "the plan line it breaks (CLAUDE.md rule 5)", while derive-once-review.md:110 and agent-rules.md:13 only say to list the rest under Notes "with what is left to do", and CLAUDE.md rule 5 itself holds no "plan line" requirement. Both sides are docs or an agent file; no program decides it (agent_stop.py reads only the SIGN-OFF line). A has no code side here, so the closure cannot cover that piece. The sorting file's `sides` column drops it. Either list it as its own question, or confirm it is no split (the page asks for less, reviewer.md for more).

### L-F18-15
**Verdict:** PASS (category P)
**Homes read:** gates\agent_stop.py:132-155 (`signed_tip` docstring: verdict is for the reviewer's newest commit on the recorded branch after the handed tip, or the handed tip when it committed nothing; no longer descends leaves nothing signed); gates\README.md:45 and the same passage (reads the recorded branch only; no record if branch gone or no longer descends; signs the newest `Agent:`-trailer commit, not the branch head); gates\ref_judge.py:184-200 area (pre-push, merge commits skipped for trailers per README:53); Hydra derive-once-review.md:112 ("checks that it still descends from the key you were handed, and records your verdict for your newest commit on it, or for the key when you committed no fix"), :151, :167, :200, :202; doc-review.md:95; agents\reviewer.md:19, :33 (same wording as :112); brief-preamble.md:65; integrate.md:20-30 searched (no claim on this).
**Why:** The earlier split no longer exists: reviewer.md:33 and derive-once-review.md:112 now state the same rule `signed_tip` implements (descends from the handed tip; newest own commit; handed tip if no fix). derive-once-review.md:151 omits the "descends" clause but does not contradict it. Replacing the copies with a pointer needs no choice.

### L-F18-19
**Verdict:** PASS (category P)
**Homes read:** Hydra derive-once-review.md:28 ("Seven are listed here; kind G is under 'One question beyond copies'"), :30-36 (A, B, C, D, E, F, H: seven kinds), :38; tools\precheck\checks.cpp:8-12 (labels kind C, B, D, E, F; line 38 of the page says the precheck covers B, C, E, D's new numbers and F's candidates); docs\agents\test-rubric.md:39,:148 (kind A = derive-once copy, matches page A); integrate.md:9 (kind A copy, matches).
**Why:** Letters and meanings agree in every home. "Seven" matches the seven bullets, and the precheck labels use the same letters as the page. No disagreement, and a pointer instead of a restated kind list needs no choice.

## Ids not reached
None.
