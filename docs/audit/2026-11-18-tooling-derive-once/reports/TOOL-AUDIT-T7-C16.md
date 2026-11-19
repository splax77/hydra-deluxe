# TOOL-AUDIT-T7-C16 (sorting S08, ledger L08, ids L-F23-10..13, L-F24-03..07, 13..15)
Paths: G = C:\Users\Patrick\.claude\gates, SC = C:\Users\Patrick\.claude\tools\session_cards, HR = C:\Users\Patrick\Downloads\Hydra\hydra-test.

### L-F23-10
**Verdict:** PASS (category B)
**Homes read:** SC\batch.py:24-26 (`MAX_BATCH_BYTES`, `MAX_BATCH_SESSIONS = 10`, `BATCHES_PER_WAVE = 10`), used at :33,:70,:74; G\session_cards_start.ps1:277 "(batch.py sizes a wave to the at-once limit)"; memory lean-agent-fanout.md:21 "For repeated discovery rounds, use 3-4 agents ... More agents per round makes 'stop when a round finds nothing' harder"; :20 "Batch small uniform jobs ... about 8 per agent".
**Why:** The one split is a memory note (3-4 agents per round, for discovery rounds) against code (10 batches per wave). Memory note against code, so B holds. The note is about a different job. Side note: the ps1 comment "at-once limit" names no number, and batch.py holds no text naming that limit; a vague comment, not a second value.

### L-F23-11
**Verdict:** PASS (category P)
**Homes read:** G\session_cards_start.ps1:262-289 (wave line, trailers line, "A commit that changes only docs/sessions needs no reviewer"); SC\writer-brief.md:95-106 (status every 5 minutes, "Only time counts", "At 20 minutes or 100 tool calls"); HR docs\agents\brief-preamble.md (5-minute rule); HR CLAUDE.md rules 7 and 14.
**Why:** No home disagrees: the hook message and brief repeat rules the way their owners state them. A pointer instead of the copies changes nothing. I did not reopen the numbers in G\progress.py; searched writer-brief.md and session_cards_start.ps1 for other values of "20 minutes"/"100" and found none that differ.

### L-F23-12
**Verdict:** PASS (category P)
**Homes read:** SC\check.py:39 `IGNORED_LINES = ("proposed-tags:", "memory:", "write-back:")`.
**Why:** Searched IGNORED_LINES across C:\Users\Patrick\.claude (py, md, ps1, json) and HR tools and docs\agents: the only live-file hit is the definition at check.py:39 (other hits are worktree copies of the same file and workflow JSON logs). A dead value; deleting it changes no behaviour and needs no choice.

### L-F23-13
**Verdict:** PASS (category A), with a note
**Homes read:** SC\writer-brief.md:75 ("Never quote CLAUDE text, TOOLS lines, a COMPACTION-SUMMARY, or anything in the `unsent` list"); SC\check.py:140,164; SC\common.py:324-335 (`users_words` = messages plus `answers[].typed`; `is_users_words` = substring of one); SC\prepass.py:110,173 (`unsent` kept apart); G\session_cards_start.ps1:40-50 (`$HookBudgetSec = 25`, comment "30 s timeout, less 5 s"); G\settings.fragment.json:91-93 (session_cards_start.ps1, timeout 30); SC\link_memory.py:24 and SC\common.py:42 (`INDEX_FILE` for MEMORY.md and index.md); SC\index.py:50 (marks).
**Why:** Splits: a brief (doc) against check.py (code) on the unsent list; a ps1 comment against the settings file the hook reads. Each is doc/comment against code; none is code against code or doc against doc. Today they do not really disagree (check rejects anything not in messages or typed, which excludes `unsent`; 25 = 30 less 5). The two INDEX_FILE constants name different files. A holds.

### L-F24-03
**Verdict:** PASS (category P)
**Homes read:** HR docs\agents\owner-map.md:3,21-29 (four columns; `found`, `split`, `carry out`, `none`), :35-47; C:\Users\Patrick\.claude\agents\scout.md:11 (names `carry out` and `none`, then points to owner-map.md); agents\planner.md:19; agents\reviewer.md:17; HR CLAUDE.md:55; docs\agents\agent-rules.md:25; docs\agents\brief-preamble.md:111.
**Why:** scout.md gives two of four statuses and points to the page; the others only point. No home contradicts another; a pointer changes no behaviour.

### L-F24-04
**Verdict:** PASS (category P)
**Homes read:** HR docs\agents\premise-map.md:15-17,23,31,35,37-47 (six Kind words, `## Premise map`, `Waits for:`); HR tests\test_docs_match_code.cpp:1520-1530 (`kPremiseMapHeading`, `kWaitsFor`, `kKindWords` = code, data, chart, run, ruling, unchecked).
**Why:** Kind words, heading and `Waits for:` agree word for word. The cut-off dates (2026-11-01, 2026-11-13) are stated only in the test; the page names none (grepped premise-map.md for 2026-1x dates: only an example at :31 and a user-quote date at :66).

### L-F24-05
**Verdict:** PASS (category P)
**Homes read:** HR docs\agents\premise-map.md:42; HR docs\clone-hero\README.md:23-26,118 ("Proven in the game's code", "Proven in a live run", "Hydra assumes it, never checked", "Unknown").
**Why:** The four labels are identical word for word; the README owns the meanings.

### L-F24-06
**Verdict:** PASS (category P)
**Homes read:** premise-map.md:29 ("A source must be for the exact thing"); derive-once-review.md:53 (step 10); doc-review.md:63 ("Sending a press and reading a value are live parts. Deciding how far a read moves a later press is not."); agents\planner.md:15 ("A step that moves or converts a quantity the engine owns is never a live part").
**Why:** Two rules, each stated at its home and restated at most once, no contradiction, no value to choose.

### L-F24-07
**Verdict:** PASS (category P)
**Homes read:** G\agent_stop.py:25,113-120 (`verdict_word`: whole stripped lines; BLOCKED, else CLEAN, else CLEAN plus a missing-line problem); HR derive-once-review.md:146-151; doc-review.md:120-125; agents\reviewer.md:28-33.
**Why:** Docs and code say the same: whole-line match, no line records CLEAN with a note, only BLOCKED holds the merge.

### L-F24-13
**Verdict:** PASS (category P)
**Homes read:** HR docs\agents\brief-preamble.md:127 ("Your brief says what to return. If it does not, return: ..."); derive-once-review.md:126-151; doc-review.md:106-118; SC\writer-brief.md:102-106; agents\reviewer.md:24.
**Why:** The preamble list is a default by its own first sentence; the other homes are briefs that override it. No home claims to be the default while saying something else.

### L-F24-14
**Verdict:** PASS (category P)
**Homes read:** G\README.md:9; G\common.py:16-19 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`, :168-172, :201-207 (on timeout the change still goes in and a note is returned).
**Why:** 2 seconds, writes anyway, says so, never refuses: README and code agree exactly.

### L-F24-15
**Verdict:** PASS (category A)
**Homes read:** G\README.md:91-104 ("Every Python hook works the same way ... `common.run(body, mode)`"; "The refusing hooks carry `onFailure: block`"); G\wake.py:90-110 (own `main` and try/except, no `common.run`); G\ref_judge.py:492-538 (own `main`); `common.run(` is called in agent_stop, delete_rule, percall, posture, prompt, publish_gate, review_dispatch, shell_output, waiver; G\settings.fragment.json:153,169,185 (onFailure block on percall.py, review_dispatch.py, publish_gate.py).
**Why:** The split is a README sentence against code (wake.py and ref_judge.py do not use `common.run`). Doc against code: A holds. Note for the main session: delete_rule.py runs `common.run(..., "closed")` but has no `onFailure` entry in settings.fragment.json while the README says refusing hooks carry it; I did not check whether it is registered elsewhere. Still README against settings, so A would hold for that too.

## Not reached
None.
