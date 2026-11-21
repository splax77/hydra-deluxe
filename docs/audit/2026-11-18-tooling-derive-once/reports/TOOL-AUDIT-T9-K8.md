# TOOL-AUDIT-T9-K8 closure check (18 ids)

Summary: REOPEN L-F29-06, L-F32-03, L-F33-02. GONE L-F33-10. PASS the other 14. All 18 reached.

### L-F28-19
**Verdict:** PASS (weak)
**Homes read:** docs/agents/brief-preamble.md:41 ("not other worktrees"); :43 and gates/delete_rule.py:14-18 (comment: no hook input says which worktree an agent works in, so any worktree counts).
**Why:** The rule is in a doc with no hook for "move or rewrite", and the delete hook openly cannot tell worktrees apart. Theme 3 ("add a check where cheap, else instruction only") fits. The doc already says the hook's limit, so it is not a contradiction. Not a strict "homeless" row, since BP:41 is a home.

### L-F28-20
**Verdict:** PASS
**Homes read:** tools/build_slot.ps1:10 and :78 (`...\.claude\hooks\state\build_slots`); `C:\Users\Patrick\.claude\hooks\state` holds only build_slots; `gates\state` holds agents, cleanup_sweep, posture, status etc. and no build_slots. Searched .claude for "build_slots": no gate reads it.
**Why:** Both sides are code (a path constant against every other hook's state folder). "Merge, I pick" fits the kind.

### L-F29-04
**Verdict:** PASS
**Homes read:** same as L-F28-20 (same fact).
**Why:** Same as above.

### L-F29-06
**Verdict:** REOPEN
**Homes read:** brief-preamble.md:117 (brief names the decision); derive-once-review.md:45 (four places: ADR, CONTEXT.md, a Confirmed line, a plan's "User decisions" header; "counts only when it quotes the user's words as found under Confirmed or in a session card"); premise-map.md:45.
**Why:** The ledger's split is whether a decision the brief names must sit in one of the four places. "Confirmed only" answers whether a card or chat quote counts as the user's words. It does not say what backs a new number, nor whether ADR, CONTEXT.md and plan-header entries (which are not Confirmed lines) still count. Only the card half is covered (DOR:45 still lets a card count). It should be "Merge, I pick" (both are docs), with the card half following "Confirmed only".

### L-F29-16
**Verdict:** PASS
**Homes read:** tools/replay_schedule.cpp:267 `end_conversion`, :273 `read_at_own_ms`; tools/replay_schedule.h:141-162; tools/ch_probe/experiments/play_chart.py:361 `own_end_now_ms`, :390 `retime_from_end`.
**Why:** Both sides are code (C++ inverse, Python forward). Merge-I-pick kind holds.

### L-F30-12
**Verdict:** PASS (note)
**Homes read:** audit-method.md:126 (decision log, question files, plans, cards; no rulings/) and :89 (search cards before "no ruling found"); premise-map.md:45 (accepts Confirmed, card quote, or chat quote the main session quotes); project CLAUDE.md "Rulings" section (only Confirmed may be cited).
**Why:** The split is which places count as the user's words, which is exactly what "Confirmed only" decides. Note for the fix: AM:126 still omits rulings/, and AM:89 says to search cards, so the page must keep "search cards, count only Confirmed" distinct.

### L-F31-22
**Verdict:** PASS
**Homes read:** memory approved-work-is-not-reasked.md:15; rulings/ui-and-display.md:9 (Confirmed, 2026-11-14: "yes i approved the mockup and asked for option B..."); :123 (under Unconfirmed, note-derived: row-layout consistency matters more than an approved mock). Searched docs/agents (outside rulings) for "mockup": nothing.
**Why:** The rule has no doc or hook. :123 is an Unconfirmed note summary, not a home, so it is not a competing answer. The fix should still settle that hint when the rule is written.

### L-F32-03
**Verdict:** REOPEN
**Homes read:** src/core/squeeze_rating.cpp:172-188 (squeezed-out row: core::paid_by_sp_walk; other rows: counted_without_squeeze with leeway); rulings/code-and-engine.md:12-13 (under `## Confirmed`, 2026-10-29): "there should never be two different answers for where star power ends. ever. the 3ms rule is for whether a backend is included in the optimal score or uncounted."
**Why:** A Confirmed line names both sides: one SP end, and the 3 ms only decides counted or uncounted backends. The code matches that. So the row should leave by that ruling, not by "Merge, I pick", whose "stricter reading" could undo the ruling by changing the leeway.

### L-F32-07
**Verdict:** PASS
**Homes read:** memory new-screens-start-from-a-clickable-mockup.md:11-17; rulings/ui-and-display.md:24 (Confirmed, "way too many options..."); :112 (Unconfirmed). grep of docs (outside sessions, audit, plans) for "mockup": only rulings and two old handoffs.
**Why:** No instruction doc or hook holds the mockup rule. The note's own parenthetical (four toggles accepted, ten too many) is inside the same note, not another home.

### L-F32-12
**Verdict:** PASS
**Homes read:** memory search-web-before-finalizing-plans.md:11-15; rulings/docs-and-writing.md:11 (Confirmed, 2026-09-03 "search github and stackoverflow for solutions before finalizing the plan"); audit-method.md:36 (left-alone rule for audits). Searched docs for "prior art": no instruction page.
**Why:** The rule lives in the note and a ruling, with no agent-rules or preamble line or hook. AM:36 covers a different thing (audit scope), so there is no split.

### L-F32-21
**Verdict:** PASS
**Homes read:** tools/ch_mcp/quiet.py:38 and :43 ("The user's ruling of 2026-11-07"); rulings/agents-and-workflow.md 2026-11-07 lines (builds only, nothing on MSBuild helpers or the scan cap); cards 2026-11-07-7d216054.md:13 ("the user's four picks").
**Why:** No Confirmed line holds it. The card points to picks but a card never closes anything. Drop-the-claim holds.

### L-F33-02
**Verdict:** REOPEN
**Homes read:** gates/delete_rule.py:14-18 (H\.claude\worktrees and ~\.claude\worktrees); gates/cleanup_sweep.ps1:174-178 (`<repo>\.claude\worktrees`, and `<root>\worktrees` only for the repo the hook lives in, which is ~\.claude).
**Why:** Q11 assumed the sweep also trusts `<root>\worktrees` for Hydra. It does not: for Hydra it trusts only H\.claude\worktrees, the same as the delete rule. So the sweep-against-delete-rule split is not there; both leave out H\worktrees, where task worktrees now live (this audit's worktree is under it). "Trust both" changes only the delete rule, and leaves the sweep not covering H\worktrees. The closure rests on a wrong premise and should go back to the user with the real state.

### L-F33-03
**Verdict:** PASS
**Homes read:** gates/cleanup_sweep.ps1:53-54 (`$ScratchAgeDays = 7`); tools/session_cards/prepass.py:31 (`RUNNING_CUTOFF_SECONDS = 2 * 3600`), used at :391 and prune_list.py:114.
**Why:** Both code, one question answered two ways. Merge-I-pick kind holds.

### L-F33-04
**Verdict:** PASS
**Homes read:** gates/posture.py:16 (`RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2`); rulings folder grep for posture, "60 min", resend: nothing; card 2026-10-31-6777b546.md:33 states the 60 minutes as an outcome only.
**Why:** The comment claims a user answer with no Confirmed line. Drop-the-claim holds.

### L-F33-06
**Verdict:** PASS
**Homes read:** gates/ref_judge.py:417-428 `made_from_existing` (docstring "the same carry rule as review_rules._code_review"; also true if another ref already contains it); gates/review_rules.py:255-265 `_carries` (clean merge of base and a branch commit that passes).
**Why:** Both code, differing for a commit on another ref and for parent order. Merge-I-pick kind holds. Line numbers have moved from the ledger.

### L-F33-08
**Verdict:** PASS
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\check.py:213-226 `default_cards` (published, local, staged); gates/ruling_rules.py:214-215 (`cards_roots = [cards.CARDS_ROOT, cards.local_cards_dir(cards.DATA_ROOT)]`, no staging).
**Why:** Both code. Kind holds.

### L-F33-10
**Verdict:** GONE
**Homes read:** docs/adr/0018-stored-data-has-one-set-of-version-stamps.md:36-40 and :59-68 ("Check it at every release: if anything under src/search, the scoring in src/core, the readers in src/parse or the record contents changed since the last bump, bump it"); src/store/stored_versions.h:42-54 (the bump rule in the stamp's own comment).
**Why:** The ledger said the rule lives only in a memory note, after searching docs/agents and .claude only. ADR 0018 and the header comment already hold it. The note repeats a rule that has a home. (Nothing checks it by hook or test, but that is not what the closure says. Also stored_versions.h:51 cites "the user's ruling, 2026-10-31", a claim to check under theme 1.)

### L-F33-12
**Verdict:** PASS
**Homes read:** gates/percall.py:135 (saves only when tool_name == "SubagentHandback"); gates/progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`, :119 exempt; percall.py:145 and :207 use progress.REPORT_TOOLS.
**Why:** Both code, two lists for "report tool". Merge-I-pick kind holds.

Ids not reached: none.
