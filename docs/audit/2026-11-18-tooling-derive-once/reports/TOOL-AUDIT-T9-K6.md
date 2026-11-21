# TOOL-AUDIT-T9-K6 closure check (18 ids)

Hooks repo = C:\Users\Patrick\.claude (gates\, tools\session_cards\). Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test. Line numbers below are today's. Search for a ruling used `docs\agents\rulings\*.md` greps; no card was needed to close any row.

### L-F19-06
**Verdict:** PASS (one note)
**Homes read:** gates\review_rules.py:255-266 `_carries` (scans base..new for a commit that passes) and :269-274 `_code_review`; gates\ref_judge.py:417-428 `made_from_existing` (two parents, parents[0]==cur, or any other ref contains new; no pass check). Its comment still says "the same carry rule as review_rules._code_review".
**Why:** Both are code answering "is this merge made only of existing work" two ways, so "Merge, I pick" is the right kind. Note: `made_from_existing` is now called only from `restore_checkout` (ref_judge.py:445), so the main session may find the two answer different questions. That is part of the pick, not a reason to fail the closure.

### L-F20-02
**Verdict:** PASS
**Homes read:** memory no-full-library-runs.md:15 still ends "No hook enforces it yet."; gates\percall.py:14 `LIBRARY_FLAG`, :45-55 `_library_refusal`; agent-rules.md:14 (rule 8, `allow library run`); src\cli\batch_scope.h (`batch_scope`: no flag and no folders = `Refused`, flag plus folders = `Ambiguous`); tools\compare_db.py:29-31 (says the flag and batch_scope.h own the rule).
**Why:** Runs half (exit 4): the program still makes the flag the only door, so the docs' "any run over the whole library" and the hook's flag check answer one question. Note half: the note's "No hook enforces it yet" is false, and its "unless truly necessary" rule has its home in CLAUDE.md rule 8, agent-rules.md:14 and the preamble, so pruning loses nothing.

### L-F20-09
**Verdict:** PASS
**Homes read:** memory fc-video-workflow.md:17 ("Keep the reviewer gate ... reproduced every done-when"); gates\review_rules.py:18 `.derive-once-gate`; `ls -a` of Downloads\Hydra\video-tools: own .git, no marker; gates\README.md:140 names only gates/, agents/, tools/session_cards/ for the hooks repo. Greps of Hydra docs\agents\*.md and gates\README.md for "reviewer gate", "reproduce every done" found nothing.
**Why:** "Reproduce every done-when" lives only in the note, and nothing contradicts it as a rule. The hook being off in video-tools is the gap the fix plan's home covers.

### L-F20-11
**Verdict:** PASS
**Homes read:** memory new-screens-start-from-a-clickable-mockup.md:15 ("Every plan for a screen includes one GUI test..."). Searched Hydra docs\ (minus sessions and audit), .claude\agents, ~\.claude\CLAUDE.md, model-postures.md for "by clicking", "clickable", "main task": only the rulings pages' unrelated mockup lines, a UserGuide line and old handoffs. docs\agents\rulings\ui-and-display.md has no Confirmed line for the GUI-click-test rule.
**Why:** One home, a memory note, no doc or hook, nothing contradicting it.

### L-F20-12
**Verdict:** GONE
**Homes read:** memory dir listing (54 files) has no render-checks-use-the-corpus.md, though MEMORY.md still indexes it; `find C:\Users\Patrick\.claude -name "render-checks*"` finds nothing; grep for "2x|zoom" in the memory dir, ui-testing.md, doc-screenshots.md, docs\agents\*.md finds no zoom step.
**Why:** The note that held the 2x-zoom rule no longer exists, so there is no rule left to give a home (the rule is simply lost, not homeless). MEMORY.md still points at the missing file. Main session decides whether the rule is wanted and whether the MEMORY.md line stays.

### L-F21-02
**Verdict:** PASS
**Homes read:** gates\review_rules.py:89-113 `human_prompt_text` (:103 `turnOrigin` == "human"); tools\session_cards\common.py:372-402 `classify_user` (:397-398 `origin.kind == "human"`), :433.
**Why:** Both code, two fields for "did the user type this". Unchanged.

### L-F21-03
**Verdict:** PASS
**Homes read:** gates\review_dispatch.py:18 and agent_stop.py:23 both `REVIEWER_TYPE = "reviewer"`; review_dispatch.py:203 `sha256(prompt)`; agent_stop.py:315 `sha256(first)` (first = agent_brief of the transcript).
**Why:** Both code, two fingerprint computations and two constants. Unchanged.

### L-F22-02
**Verdict:** PASS
**Homes read:** gates\ref_judge.py:235 accepts length 40 or 64, lowercase only; review_dispatch.py:20,24 `{40}` hex either case; review_rules.py:26-28 waiver phrases `{7,40}` / `{7,64}`, :56 `_HEX`.
**Why:** Both code, different length and case rules. Unchanged.

### L-F22-08
**Verdict:** PASS
**Homes read:** gates\ruling_rules.py:105 `SESSION_NOTE`, :144 ("it ends with a note other than (session <id>)"); tools\session_cards\common.py:94-99 `RULING_NOTE` (any one-level parenthesised note).
**Why:** Both code, strict against loose tail. Unchanged.

### L-F22-11
**Verdict:** PASS
**Homes read:** tools\session_cards\writer-brief.md:16-21 ("The user ruled on four kinds of content on 2026-10-31"). Grepped docs\agents\rulings\*.md for "no money", "token totals", "dollar", "local: yes", "security stays", "vague": no hit.
**Why:** A place says the user ruled and no Confirmed line holds it, so "Drop the claim" applies.

### L-F22-22
**Verdict:** PASS
**Homes read:** gates\status_append.ps1:23-37 (any agent record in any session passes); gates\progress.py:3-4,22,57-64 (id tied to the caller for subagent calls only).
**Why:** Both code, two readings of the id check. Unchanged.

### L-F23-08
**Verdict:** PASS
**Homes read:** review_rules.py:89-113 (turnOrigin) against common.py:372-459 (origin.kind plus command args and attachments).
**Why:** Both code, unchanged. Same pair as L-F21-02.

### L-F23-09
**Verdict:** PASS
**Homes read:** tools\session_cards\number_rulings.py:57-64 (`DNUM` hits under 1000 only); common.py:80 `DNUM = \bD(\d{1,4})\b`; check.py:168-172 (any D number must be in the decision log).
**Why:** Both code, different ceilings. Unchanged.

### L-F24-01
**Verdict:** PASS
**Homes read:** derive-once-review.md:52 (accepts D-record, plan "User decisions", Confirmed, a card, brief quotes); doc-review.md:63 (Confirmed, a card, or the brief's quote); premise-map.md:45 (same three); audit-method.md:126 (decision log, question files, plans, cards); agent-rules.md:35 (only Confirmed is the user's words).
**Why:** The pages still list different places and still admit cards and brief quotes. "Confirmed only" covers the split and the fix is still to be made.

### L-F24-02
**Verdict:** PASS
**Homes read:** CLAUDE.md "Past sessions" ("a card quote is not a filed ruling"); tools\session_cards\writer-brief.md:67 (same); doc-review.md:63 and derive-once-review.md:52 still accept a card's quoted words; agent-rules.md:41 accepts "words as found under ## Confirmed or in a session card".
**Why:** Still split, same answer covers it.

### L-F24-08
**Verdict:** PASS (one note)
**Homes read:** gates\agent_stop.py:26 `_BLOCKED/_CLEAN`, :166-173 `verdict_word` (whole-line match; a BLOCKED line with text after it matches neither and is recorded CLEAN with a problem message); derive-once-review.md:149 and doc-review.md:123 ("Write no hash, key or path with it"), reviewer.md:31.
**Why:** The docs state the rule and the hook still turns a BLOCKED with added text into CLEAN, flagged in its problem line. "Give each a home ... add a hook only where cheap and exact" covers a whole-line check here. Note: this is a fail-open path past the review gate, so the fix must be in the hook, not only the docs.

### L-F24-10
**Verdict:** PASS (one note)
**Homes read:** tools\check_handoff.py:28 `_FULL_HASH` exactly 40; tools\doc_tokens.py:39 and :50 `{7,40}`; check_handoff.py:33 `TABLE_STATUSES` and :368 returns "unknown" for a kind outside the seven; handoff.md:48-57 names four statuses plus `manual`.
**Why:** The hash split is code against code, which "Merge, I pick" covers. Note: the status split is smaller than the ledger said ("unknown" appears only for an unrecognised kind) and is doc against code, which follows Q1.

### L-F25-01
**Verdict:** PASS
**Homes read:** premise-map.md:45 (admits a card's quoted words and a brief quote the main session confirms); agent-rules.md:35 and CLAUDE.md "Rulings" (only Confirmed lines).
**Why:** Still split; "Confirmed only" says the premise-map page gets fixed.

## Ids not reached
None.
