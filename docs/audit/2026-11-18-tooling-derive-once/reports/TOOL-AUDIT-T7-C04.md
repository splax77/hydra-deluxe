# TOOL-AUDIT-T7-C04 closure check

Batch: item 3 of t7.json (sorting S02, ledger L02). 14 ids: PASS 13, REOPEN 1, GONE 0. Not reached: none.
Abbreviations: G = C:\Users\Patrick\.claude\gates, MP = C:\Users\Patrick\.claude\model-postures.md, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, PRE = H\docs\agents\brief-preamble.md, MEM = the project memory folder.

### L-F05-19
**Verdict:** PASS (category P holds)
**Homes read:** H\tools\build_slot.ps1:75 `$BuildSlotCount = 3` (and :152,:197,:199 use it); G\percall.py:20,142-145 (BUILD_PASS_TOOL refused to every agent); G\README.md:15 (says percall refuses the pass tool, build_slot.ps1 owns the limits); PRE:57 "At most three builds run at once ... A hook refuses that tool to every agent"; H\CLAUDE.md rule 14; H\docs\agents\agent-rules.md:20 (rule 14, "At most three builds"); H\docs\development.md:114 "at most three builds run at once"; ruling agents-and-workflow.md:8 (Confirmed section).
**Why:** Every prose home says three and one-at-a-time per main session, the code constant is 3, and the refusal text matches. Searched docs\agents and gates README for other build counts ("three build", "3 build", "builds at once", "at most N"): no different number. A pointer in place of the copies changes no behaviour.

### L-F06-01
**Verdict:** PASS (P)
**Homes read:** G\percall.py:16 `KEY_TOOLS = ("mcp__ch__ch_key", "mcp__ch__ch_press")`, :18 `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`; H\tools\ch_mcp\game_tools.py:71 `ROUTE_KEYS = "keys"`, :984 ch_press, :1013 ch_key; H\tools\ch_mcp\server.py:146-147,389-390,463-479 (same two key tools, play_chart on route keys); H\docs\agents\clone-hero-mcp.md:29; G\README.md:13,37; G\review_rules.py:54 `KEY_ALLOW_TEXT = "allow key run"`; MEM ch-runs-must-not-need-focus.md:17.
**Why:** All homes name ch_key and ch_press as key tools and ch_play_chart with route "keys" as the conditional one. No other tool in server.py takes a route argument (grep "route" in server.py). No disagreement.

### L-F06-02
**Verdict:** PASS (P)
**Homes read:** G\progress.py:20 `SHELL_TOOLS = ("Bash", "PowerShell")`; G\delete_rule.py:168 own literal, same two; G\tests\shell_output_sizes.py:20 same; G\shell_output.py:35-36 TAILS keys Bash and PowerShell; G\settings.fragment.json:192 matcher "Bash|PowerShell"; G\bash_windows_rule.py:235 judges Bash only (it is a Bash-syntax rule, and PRE:33 says "in a Bash call"; not a different shell-tool list).
**Why:** Same two names everywhere; replacing the retyped literals with progress.SHELL_TOOLS changes nothing.

### L-F06-03
**Verdict:** PASS (P)
**Homes read:** G\percall.py:1-6 docstring and :132-173 code. Subagent order in code: hand-back save, progress, build-pass refusal, then for shell calls delete_rule, library, bash_windows, media(main=False), then key check. Main order in code (:163-171): library, key, bash_windows, media (ask), stall. G\README.md:11 states exactly these two orders (subagent: ...delete rule, whole-library, bash_windows_rule, media_rule, then key-route; main: whole-library, key-route, bash_windows_rule, media_rule, stall).
**Why:** The README sentence equals the code today. The two sides differ from each other, but each page statement is right for its side. Change to a pointer is behaviour-free.

### L-F06-04
**Verdict:** PASS (A holds)
**Homes read:** G\delete_rule.py:15-18 `WORKTREE_FOLDERS` two exact folders (comment says any worktree directly under those two) and :48-55 `_roots`; MP:5 "each in its own worktree under a `worktrees` folder (the delete rule trusts only those)"; G\README.md:136 "Keep it under a `worktrees` folder: the delete rule trusts only those"; PRE:43 and H\docs\agents\derive-once-review.md:200 (step 1 text at line ~197 in the findings) both name the two exact folders and agree with the code.
**Why:** The split is MP:5 and README:136 (docs reading as any folder called worktrees) against delete_rule.py (code, two exact paths). Docs against a program file, nothing else on a side. The ruling review-and-hooks.md:17 only names "a `worktrees\` folder" and is not a side. Note for the main session: this audit's own worktree is at H\worktrees\tooling-audit-records, which is in neither trusted folder; it shows the split is real, and that "code wins" means the doc lines get the two exact paths.

### L-F06-05
**Verdict:** PASS (B holds)
**Homes read:** MP:11 "A brief names no status file and no id; it carries only the preamble's status rule."; MEM every-brief-carries-status-rule.md:3 and body ("the brief must carry the status-file sentence", "the dispatch gate now denies briefs that lack it"); MEM MEMORY.md index line; G\progress.py:88-90 (the hook tells the agent its own id and status file on its first call); G\review_dispatch.py (searched "status": no match) and G\review_rules.py (no match); G\settings.fragment.json:158 (only Agent matcher goes to review_dispatch); PRE:3,9,13-19; reviewer pages derive-once-review.md:9 and doc-review.md:9 (these tell the reviewer itself to write a status line; they do not say what a brief carries).
**Why:** Both sides are the memory note: against MP:11 (doc) and against code (no gate reads a brief for a status sentence). No doc or code home says a brief must carry the status-file sentence. Memory note only.

### L-F06-07
**Verdict:** PASS (A holds)
**Homes read:** PRE:65 names only "a merge commit (two or more parents)" as exempt; G\ref_judge.py:242-253 `missing_trailers` (skips merges), :223-239 `new_commits` (git log ... `--not --branches *known`, so a commit on any local branch or an origin tip is not judged), :317-342 `trailer_reason` text "Merge commits, and commits origin already has on one of its branches, are exempt"; G\README.md:5,53 (point at PRE; the judge owns the trailer rule); derive-once-review.md:100, doc-review.md:95, integrate.md:27 (use "the preamble's trailers", no exemption claim).
**Why:** One doc line (PRE:65) against a program file that exempts more (local-branch commits too). Docs against code. The side question about the Co-Authored-By model line is not a split of two homes: PRE:69 says use the brief's model line.

### L-F06-10
**Verdict:** PASS (A holds, but part of the finding is already fixed)
**Homes read:** G\review_dispatch.py:152-186 `body` (a brief with no key, no label and an ungated session folder passes at :169-170; any other brief is refused), :140-145 `_gated`, :15 `REVIEWER_TYPE`; G\agent_stop.py:20 (REVIEWER_TYPE declared separately; both "reviewer"); G\README.md:39 (now says a brief with no commit and neither line passes "but only from a session folder whose repo has no review marker on main"); H\docs\agents\derive-once-review.md:17 "A hook refuses a reviewer dispatch whose brief lacks either line, or holds a line not in its exact form, a key already on main, a key git does not know or more than one commit"; derive-once-review.md:177,200; doc-review.md:29; G\agents\reviewer.md:9,33.
**Why:** README:39 now states the condition, so that side of S5 is gone. derive-once-review.md:17 still reads as if lacking lines is always refused, while the code lets a brief with neither line and no key through from an ungated repo: a doc against a program file, so A still holds. No doc-vs-doc or ruling side remains. The finder's `judged_folder` and the "more than one commit" wording no longer exist in the code (the code counts distinct 40-hex strings, and the page at :177 now says so).

### L-F06-11
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude\agents\card-writer.md:3-9 (`tools: Read, Glob, Grep, Write, PowerShell`, `disallowedTools: Agent`); C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md (the tool sentence "Use only Read, Glob, Grep and Write, plus the PowerShell status-line command", and "How to work" step 4 "At 20 minutes or 100 tool calls"); G\progress.py:14-15 (CALL_WARN 100, MINUTES_WARN 20).
**Why:** The tool list in the agent file, the brief and the numbers in progress.py all agree today. The brief's PowerShell limit is prose only and nothing disagrees with it. Pointer fix changes no behaviour.

### L-F06-14
**Verdict:** PASS (B holds)
**Homes read:** MEM lean-agent-fanout.md:3,21 ("3-4 agents per discovery round"; "use 3-4 agents ... More agents per round makes 'stop when a round finds nothing' harder"); MEM MEMORY.md:39 "3-4 agents per round"; H\docs\agents\rulings\agents-and-workflow.md:21 (under `## Confirmed`, 2026-10-03) "you dont need to stay limited to 10. run as many as you can."; H\CLAUDE.md rule 6 and H\docs\agents\agent-rules.md:12 ("Waves of parallel agents are fine", no cap); MP:5.
**Why:** The only home with a cap is the memory note and its index line. Every doc says no cap. One caveat for the main session: the Confirmed line is about "10", and the note's cap is for discovery rounds, so a reader could say the scopes differ; the user already chose "Note is wrong", and the note is the only side that differs.

### L-F06-16
**Verdict:** REOPEN (should be a question, or at least not category A)
**Homes read:** MP:11 "nudge it once with SendMessage ... If it does not recover, stop it with TaskStop and re-dispatch"; G\stall.py:124-125 (stuck notice: "If an agent is stuck, stop it with TaskStop and re-dispatch"), :143-144 (past-limit notice, same wording), :458-461 `CHECK_IN_HEADER` ("open its transcript, nudge it or stop it"); G\wake.py:6 (names the nudge probes); G\tests\probe_results.md:238-248 ("The nudge step goes into the posture (T4)"); H\docs\agents\brief-preamble.md:23 and agent pages (searched TaskStop, nudge, SendMessage: no further home); C:\Users\Patrick\.claude\agents\*.md (no match).
**Why:** "The code wins" does not pick one value here. Code against code: two hook messages (stall.py:124 and :143) say stop and re-dispatch, with no nudge, but the hook's own check-in text (stall.py:460) says "nudge it or stop it", and the hook's probe record says the nudge step belongs in the posture. Deleting the nudge from MP:11 to match the stuck notice would contradict stall.py:460; adding the nudge to the stuck notices would change the hook text. That is a choice of which step order the user wants, so it needs a question. (Not a clean A, since both sides include code.)

### L-F06-17
**Verdict:** PASS (A holds; the `judged_folder` part is GONE, the rest stands)
**Homes read:** MP:5 "A brief may name a commit in the other repo if it writes that repo's root folder."; G\review_dispatch.py:18-26,99-133 (the `Review repo:` line must be the top folder of the worktree with the branch under review checked out; a brief naming a second commit is refused at :175-178; `judged_folder` no longer exists: grep of gates and the agent files finds no match); G\README.md:39 and H\docs\agents\derive-once-review.md:207 (both say the `Review repo:` line names a worktree of that repo, "in this repo or the other one", agreeing with code).
**Why:** The one remaining split is MP:5 (doc: "that repo's root folder", "may name a commit") against review_dispatch.py (code: the worktree's top folder on a labelled line, and only the one key). Doc against code. Pages agree with code already; only MP:5 differs.

### L-F06-18
**Verdict:** PASS (P)
**Homes read:** G\agents\reviewer.md:9 and executor.md:9; H\docs\agents\derive-once-review.md:17,53,177,200; doc-review.md:29; MP:5,13; PRE:95-97; H\CLAUDE.md:71 (rule 9); H\docs\agents\agent-rules.md:15; G\review_dispatch.py:21,23-26 (range form `<start>...<key>`, start a branch name, usually main).
**Why:** The handoff sentence is word for word the same in agent-rules.md:15, PRE:95, CLAUDE.md:71 and MP:13 (four copies, found by grep). The range wording is the same on reviewer.md and the review pages ("usually main", `Review range:` line). No disagreement.

### L-F06-19
**Verdict:** PASS (P)
**Homes read:** PRE:33 and :43; G\bash_windows_rule.py:16 `MAX_CHARS = 1000`; G\shell_output.py:15 `CAP = 4000`; G\delete_rule.py:15-18 (roots), :32-35 (`GIT_DISCARD_WORDS`, `GIT_DISCARD_ACTIONS`: reset --hard/--merge, checkout -f, worktree prune, stash drop, clean at :121); the 30,000 limit is Claude Code's own and PRE:35 says so.
**Why:** The prose that spells out the hook rules matches the code today in every number and command I checked, so there is no split. Replacing the spelled-out text with a pointer to the hook files changes no behaviour. (I did not re-derive the "five Bash shapes" list; it was not part of any disagreement.)

## Ids not reached
None.
