# TOOL-AUDIT-T9-K2 closure check (18 ids)
Summary: REOPEN L-F08-11 (a second split is not covered). GONE L-F08-06 (the comment no longer exists). The other 16 PASS.
Rulings searched: every `## Confirmed` section in docs/agents/rulings/*.md by keyword (120 s, 2 minutes, unclaimed, scan, cap, frames, posture, resend, 60 minutes, losing battle, exception, compact, dates 2026-10-28 to 11-07); session cards grepped for "losing battle".

### L-F04-05
**Verdict:** PASS (The code wins)
**Homes read:** gates\stall.py:36 CTX_TEXT "Auto-compact should have fired at 500K (50% of the window)"; ~\.claude\settings.json and gates\settings.fragment.json (grep for compact / PCT_OVERRIDE / 500 finds only the SessionStart "compact" matcher, settings.json:119, fragment:109).
**Why:** the settings that decide auto-compact set no threshold; the text is printed advice that enforces nothing, so matching it opens no gate.

### L-F04-06
**Verdict:** PASS (Drop the claim)
**Homes read:** gates\posture.py:16 "the user's answer to Q8 of the hooks plan HX-P2"; docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:306 ("T2 picks 60 minutes"), :83 ("starts after Q8 is answered"). No Confirmed line mentions posture or a resend interval.
**Why:** a claim of a user answer that no ruling holds.

### L-F04-10
**Verdict:** PASS (Drop the claim)
**Homes read:** tools\build_slot.ps1:79-80 "The user's answer in the same rulings page" (120 s); rulings\agents-and-workflow.md:35 is under `## Unconfirmed` (heading :32); Confirmed :8-9 cover only the 3-build limit and build tests.
**Why:** only an Unconfirmed line holds it.

### L-F04-14
**Verdict:** PASS (Merge, I pick)
**Homes read:** .claude\commands\handoff.md step 4 ("If a row is still `missing` after that second run, leave it, and say which row and why"); docs\agents\handoff.md ("fixes every `missing` row it finds", now line 64).
**Why:** doc against doc, still worded differently; kind is right.

### L-F05-03
**Verdict:** PASS (Drop the claim)
**Homes read:** tools\ch_mcp\quiet.py:43 "The user's ruling of 2026-11-07" for SCAN_CAP_S; Confirmed 2026-11-07 lines (agents-and-workflow.md:8-9) cover builds only; "four tool numbers stand" is Unconfirmed (:34).
**Why:** no Confirmed line holds the cap.

### L-F05-04
**Verdict:** PASS (Drop the claim)
**Homes read:** tools\ch_mcp\bot_run.py:62-91 ("The user's caps ... question 5", "answered by the user on 2026-10-31"); clone-hero.md Confirmed searched for cap/launch/attach/poll.
**Why:** no Confirmed line holds these caps.

### L-F05-05
**Verdict:** PASS (Drop the claim)
**Homes read:** tools\ch_mcp\kit_tools.py:94-123 ("user's answer ... 2026-10-30", "Cap at 60 s (Recommended)"); frida_tools.py:39-49 ("approved by the user on 2026-10-29"); trace_hits.py:67-74 ("approved ... 2026-10-28").
**Why:** the only caps in Confirmed (clone-hero.md, 2026-11-11/12 stall plan) are different numbers.

### L-F05-13
**Verdict:** PASS (Merge, I pick)
**Homes read:** tools\capture_window.ps1:56 `$Wait = 12`; tools\capture\shot.ps1:2 `$Wait = 10` (moved from tools\shot.ps1).
**Why:** code against code, still differs.

### L-F06-06
**Verdict:** PASS (Merge, I pick)
**Homes read:** gates\progress.py:24-26 regex [A-Za-z0-9_-]+; gates\common.py:113-116 check_id; gates\status_append.ps1:25-39.
**Why:** three code checks for a valid agent id, still different.

### L-F06-08
**Verdict:** PASS (Merge, I pick)
**Homes read:** gates\percall.py:202 `if not data.get("agent_id")`; stall.py:435,447,483 and wake.py:97 `agent_id or not session_id`.
**Why:** code against code, still differs.

### L-F06-12
**Verdict:** PASS (Merge, I pick)
**Homes read:** ~\.claude\tools\session_cards\prepass.py:143-170 (counts agent-*.jsonl in one workflow run folder); Hydra tools\agent_cost.py:160 rglob under subagents; gates\stall.py:175-188 own layout.
**Why:** three code readers, different layouts.

### L-F07-06
**Verdict:** PASS (Merge, I pick)
**Homes read:** common.py:113-116; progress.py:26; status_append.ps1:25-39; brief-preamble.md "Your agent id".
**Why:** the id shape is still answered differently in code.

### L-F07-08
**Verdict:** PASS (Minutes, line 34)
**Homes read:** rulings\review-and-hooks.md:33 (10 calls / 15) and :34 (5 / 10 minutes), both under `## Confirmed` (:7); gates\progress.py:15-16 STATUS_NAG_MINUTES 5, STATUS_DENY_MINUTES 10, no by-calls status constant.
**Why:** split exists; the answer covers it and the code already runs by minutes.

### L-F07-11
**Verdict:** PASS (Trust both)
**Homes read:** gates\delete_rule.py:14-18 (two folders); gates\cleanup_sweep.ps1:174-178 (also <root>\worktrees); questions.md Q11 "The delete rule accepts both folders."
**Why:** split still there; the answer decides it.

### L-F08-06
**Verdict:** GONE
**Homes read:** gates\review_dispatch.py (whole file, grep "exception", "approved", "2026-10-24"); git grep "2026-10-24" in ~\.claude; gates\README.md.
**Why:** the comment "one exception the user approved on 2026-10-24" is no longer in review_dispatch.py (lines 62-74 are the brief-line parser and _same_path). Nothing in the hooks repo carries it. (Hydra's 2026-10-24 exception, tests-and-runs.md:33, is a different, Unconfirmed test-run rule.)

### L-F08-10
**Verdict:** PASS (Merge, I pick)
**Homes read:** stall.py:175-188 (_agent_transcript from layout); percall.py:100-112 (calls it); agent_stop.py:147,306,370 (`agent_transcript_path` from hook input); cleanup_sweep.ps1:596-600.
**Why:** both code; two ways to find an agent transcript remain.

### L-F08-11
**Verdict:** REOPEN
**Homes read:** posture.py:16 (claim, fine under Drop the claim); but also docs\agents\agent-rules.md:5 "the model-postures file (what the main session gets each prompt)" against gates\README.md:35 and posture.py:1-8 (first prompt, then every RESEND_MINUTES, after compaction).
**Why:** the row has a second split, doc against code, about when the posture is sent. "Drop the claim" does not cover it. It needs its own exit; "The code wins" would fit (fix agent-rules.md:5 to the code), nothing gated.

### L-F08-13
**Verdict:** PASS (Drop the claim)
**Homes read:** gates\README.md:19 "The user ruled on 2026-10-31 that ... losing battle"; grep "losing" in docs\agents\rulings finds nothing; only card docs\sessions\2026-10-31-6777b546.md:25.
**Why:** a card never closes anything.

Not reached: none.
Note: I started a `find /` by mistake early on (in the background); I had no tool to stop it. It only reads.
