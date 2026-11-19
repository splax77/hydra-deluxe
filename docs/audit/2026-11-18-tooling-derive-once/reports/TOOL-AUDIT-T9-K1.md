# TOOL-AUDIT-T9-K1: closure check, 18 ids

Summary: 17 PASS, 1 REOPEN (L-F02-08), 0 GONE. Three PASS rows carry a note for the fix plan (L-F01-01, L-F02-05, L-F01-26). No row left unreached.

Paths: R = C:\Users\Patrick\Downloads\Hydra\hydra-test; G = C:\Users\Patrick\.claude\gates; MEM = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory.

### L-F01-01
**Verdict:** PASS (with a note)
**Homes read:** R\docs\agents\agent-rules.md:13 (rule 7: "only time counts, not calls" and then "At 100 tool calls an agent starts wrapping up"); G\progress.py:14-16 (CALL_WARN/STOP 100/150, MINUTES 20/30, status 5/10); C:\Users\Patrick\.claude\model-postures.md:5 ("inside 100 tool calls and 20 minutes"); R\docs\agents\rulings\review-and-hooks.md:22, :33, :34 (all under ## Confirmed, which starts at :7).
**Why:** The split (rule 7 "not calls" against the call caps in the hook, MP:5 and AR:13) is still there. Q8's answer, "Minutes (line 34)... Matches rule 7", picks the minutes side, and :22 "minutes only" and :34 are Confirmed. So the closure stands. Note for the fix plan: the answer says nothing about what happens to the 100/150 call caps. No Confirmed line states them; they exist only in the hook, the preamble (:71-73), AR:13 and MP:5. The fix has to drop them or reword rule 7, and that is a hook change.

### L-F01-02
**Verdict:** PASS
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:101 ("stop starting new cards: finish the one in hand and report"); G\progress.py:94-100 (wrap up, commit, report); C:\Users\Patrick\.claude\agents\reviewer.md:11 ("stop fixing, sign off what you fixed, and list every unfixed finding under a Notes heading"); R\docs\agents\brief-preamble.md:73 ("so a fresh agent can take the rest"); agent-rules.md:13; derive-once-review.md:110 ("No fresh agent picks it up").
**Why:** The three-way split and the "fresh agent takes the rest" against "reviewer hands nothing on" split both exist today. The answer ("finish the item in hand, commit or sign off what passes, list what's left; main session decides about a fresh agent; reviewers never hand on") covers both. review-and-hooks.md:24 ("dispatch remaining work to a new agent") is consistent with the main session deciding.

### L-F01-03
**Verdict:** PASS
**Homes read:** G\delete_rule.py:29 (OPERATORS regex) and :75-77 (pieces splits raw text); G\bash_windows_rule.py:19, :40-61 (_scan cuts only in state BARE); G\media_rule.py:20-23 (reads delete_rule.OPERATORS).
**Why:** Both sides are code, one cut quote-blind and one quote-aware, answering "where does a command end a part". "Merge, I pick" fits.

### L-F01-04
**Verdict:** PASS
**Homes read:** G\stall.py:36-37 (CTX_TEXT), stall.py:1-12 (output is additionalContext only, never a decision); C:\Users\Patrick\.claude\model-postures.md:15.
**Why:** Both are instruction text; the stall notice enforces nothing. That is doc against doc, so "Merge, I pick" is the right kind. Late-folds.md:7 agrees.

### L-F01-05
**Verdict:** PASS
**Homes read:** G\cleanup_sweep.ps1:58 (`$GitTimeoutSec = 600`), :125 (WaitForExit); G\settings.fragment.json:79-81 (registered with `"timeout": 30`).
**Why:** Both sides are code or settings that Claude Code reads, answering how long the sweep may run. The registration is config, not prose, so "Merge, I pick" (code against code) fits. Still open as the ledger said: only a runtime fact shows whether Claude Code kills the hook at 30 s.

### L-F01-06
**Verdict:** PASS
**Homes read:** G\bash_windows_rule.py:16 ("the user's number"), :173 and :178 ("the user's ruling"); G\README.md:19, :27; R\docs\agents\rulings\review-and-hooks.md:40 and :43 (both under ## Unconfirmed, which starts at :37); grep of R\docs\agents\rulings for "Exempt git commit", "chose 1,000", "1,000" and "1000 character".
**Why:** The code and README claim a ruling. The only matching lines are under Unconfirmed. The Confirmed line at :10 ("yes block those too") is about three other shapes, not the length limit or the commit exemption. "Drop the claim" holds. Note: the :178 comment ("A pair in double quotes passes ... the user's ruling") is a second claimed ruling in the same file; its backing is :41, also Unconfirmed. The fix should drop that one too.

### L-F01-08
**Verdict:** PASS
**Homes read:** R\docs\agents\rulings\review-and-hooks.md:33 and :34 (both Confirmed), :22; G\progress.py:16 and :135-143 (status age in minutes, nag 5, deny 10; no calls-based status rule).
**Why:** Both lines are still there, they still disagree, and the hook follows :34. "Minutes (line 34)" covers the split. Line 33 is still under Confirmed today; its retirement is on the fix list.

### L-F01-09
**Verdict:** PASS
**Homes read:** R\docs\agents\brief-preamble.md:21 ("a command that starts with `git`"); G\progress.py:119 (`command.startswith("git ")`), :121-122.
**Why:** The doc and the code give the same rule, so exit 4 is right. A chained command such as `git status && ...` starts with git and passes; that is a hook gap, recorded.

### L-F01-15
**Verdict:** PASS
**Homes read:** G\stall.py:8-9 ("the empty-fleet notice is here on the user's ruling of 2026-10-25"); grep of R\docs\agents\rulings and R\docs\sessions for "empty fleet", "empty-fleet", "fleet".
**Why:** No line under ## Confirmed names an empty-fleet notice. The 2026-10-25 Confirmed lines (review-and-hooks.md:13-19) cover the cap, the new plan, library runs, the worktree choice and the review key. "Fleet" hits are only unconfirmed notes or unrelated cards. "Drop the claim" holds.

### L-F01-26
**Verdict:** PASS (with a note)
**Homes read:** G\progress.py:23-27 (_STATUS_CALL accepts any text without `"`, `$`, backtick or newline); R\docs\agents\brief-preamble.md:13 and :15-19; C:\Users\Patrick\.claude\model-postures.md:11 (the evidence line is checked against the claim).
**Why:** The rule on what a line should say is already in the preamble, so it has a home. Only the check is missing. Q7's answer adds a hook "only where a check is cheap and exact", and free text is not exact. So the instruction stays in the preamble, which agrees with the answer. Note: this is not a rule with no home, so "Give each a home" is a loose label; the real outcome is "instruction only, no check".

### L-F02-05
**Verdict:** PASS (with a note)
**Homes read:** R\docs\agents\clone-hero-mcp.md:39 ("Nothing runs over the user's song library. Each tool takes one song folder or one chart folder."); grep of R\tools\ch_mcp\*.py and clone-hero-mcp.md for "library", "whole-library" and "library root" (no refusal of a library root in any tool); G\percall.py LIBRARY_FLAG (reads shell text only).
**Why:** The rule is in a doc, with no check in the tools. It is a doc claim with no code behind it, not a homeless rule. The answer ("add a hook only where a check is cheap and exact") leaves it to the fix plan whether to add a refusal in the ch_mcp tools or soften the doc line. Closure stands only as "decide in the fix plan". The main session should not count the sentence in line 39 as enforced.

### L-F02-08
**Verdict:** REOPEN
**Homes read:** R\tools\ch_mcp\sp_tools.py:317, :330-331 (refuses without `resolution`, asks the agent for it); R\tools\ch_mcp\server.py:551-565 (same argument); R\tools\ch_mcp\game_tools.py:470 (`schedule_chart_folder`, returns the chart's folder only); grep of R\tools\ch_mcp\*.py for "resolution" and "schedule_chart_folder".
**Why:** "Merge, I pick" needs two answers to one question. Here only one place answers "what is the chart's Resolution", and it is the agent typing it in. `schedule_chart_folder` answers "which folder is the chart in", a different question. Nothing in ch_mcp reads Resolution from the chart. So there are no two answers to merge. It is a value the display-side tool asks the agent for although the engine or the chart file could supply it (agent rule 15: a value the engine can work out is carried out, not asked for). It should be re-sorted as "no owner / carry out" or exit 4 (different questions), and sent to the user as a question if the user's words do not cover it. A .mid chart keeps Resolution in its header, which the refusal text does not mention (ledger).

### L-F02-10
**Verdict:** PASS
**Homes read:** R\tools\replay_schedule.cpp:36-40 ("The user set it: question 3 of the optimal-score run plan, which has left the tree"); grep of R\docs\agents\rulings for "15 ms", "kSpEndLead", "SP end lead", "freak" (no match under ## Confirmed).
**Why:** The comment claims the user set 15 ms, and no Confirmed line says so. "Drop the claim" holds. Note that :45 (kSpOutLagMs, also 15) and :49, :55-56 carry the same kind of claim and are not in this row.

### L-F02-11
**Verdict:** PASS
**Homes read:** R\tools\replay_schedule.cpp:22-31 ("The user set it: question 1 of ...player-freak-fixes.md"; "the user chose 'One frame', D319 item 3"); R\docs\agents\rulings\code-and-engine.md:12-13 (Confirmed, about the 3 ms leeway only); R\docs\agents\rulings\clone-hero.md:20 (Confirmed, "Accept all four", about the LS-G1 probe numbers, not kFrameMs).
**Why:** No Confirmed line names "One frame" or the 3.0 ms frame. D319 is a decision-log line, not a ruling. The two Confirmed lines that mention 3 ms or frames speak to other numbers. "Drop the claim" holds.

### L-F03-04
**Verdict:** PASS
**Homes read:** MEM\lean-agent-fanout.md:3, :20, :21 ("about 8 per agent", "3-4 agents"); MEM\MEMORY.md (lean-agent-fanout line); grep of R\docs\agents, G\*.py, G\*.md, model-postures.md, CLAUDE.md, agents\*.md, commands\*.md for "per agent", "per round", "3-4 agents", "jobs per".
**Why:** The numbers live only in the note. The only nearby text is agents-and-workflow.md:48 under Unconfirmed, which does not carry the numbers. No doc or hook contradicts them. "Give each a home" holds.

### L-F03-12
**Verdict:** PASS
**Homes read:** MEM\question-the-design-before-patching.md; MEM\MEMORY.md; grep of R\CLAUDE.md, R\docs\agents\*.md, G, model-postures.md for "same goal", "failed twice", "fails twice", "failed runs", "design itself" (nothing).
**Why:** The rule is only in the note. brief-preamble.md:85 is about a stuck agent and a different trigger. Nothing contradicts it.

### L-F03-14
**Verdict:** PASS
**Homes read:** MEM\approved-work-is-not-reasked.md; MEM\MEMORY.md; R\docs\agents\agent-rules.md:9 (rule 3, work waits for a yes); grep of R\docs\agents and C:\Users\Patrick\.claude gates and agents for "already approved", "re-ask", "do not re-ask".
**Why:** No doc or hook holds the rule; rule 3 points the other way (work starts after a yes) and does not contradict it. The user's words are in rulings (code-and-engine.md:8, ui-and-display.md:9 per the ledger), but those are words, not a rule page. "Give each a home" holds.

### L-F03-17
**Verdict:** PASS
**Homes read:** MEM\git-traps-in-shared-checkout.md:13 and :16 (361 commits; cut the worktree from main by hand; the live note now also gives the cause, "cuts its worktree from `origin/main`, not local `main`"); R\docs\agents\derive-once-review.md:197 and :200 (reviewer isolation only); G\review_dispatch.py:32-34, :172-173, :216, :243 (reviewer isolation only); C:\Users\Patrick\.claude\model-postures.md:5-9; grep of R\docs\agents and the gates for "base commit", "from main", "isolation", "361".
**Why:** Only reviewer dispatch with isolation has a doc and a hook. The executor-base rule is in the note and nothing else. model-postures.md:9 "from main" is about running precheck from main. No other home contradicts it, so "Give each a home" holds. Small change from the ledger: the live note now gives the cause as well as the figure.

## Ids not reached
None.
