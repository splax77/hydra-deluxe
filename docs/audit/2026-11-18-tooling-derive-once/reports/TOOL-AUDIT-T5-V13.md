# TOOL-AUDIT-T5-V13 (verifier V13, agent a3f2b8e3879a072e7), checks finder F13

All claims below are "by reading" unless a run is named. GATES = C:\Users\Patrick\.claude\gates, HY = C:\Users\Patrick\Downloads\Hydra\hydra-test, BP = HY\docs\agents\brief-preamble.md. The finder report exists and was read.

## 1. Git pins and env-drop lists in ref_judge.py, review_rules.py, check_handoff.py: CONFIRMED
- GATES\ref_judge.py:34-35 PINNED has four settings (quotePath, showSignature, trailer.separators=:, useReplaceRefs). Its git() drops only GIT_CONFIG_PARAMETERS and GIT_CONFIG_COUNT and sets GIT_NO_REPLACE_OBJECTS=1 and GIT_GRAFT_FILE.
- GATES\review_rules.py:60-62 `_GIT_PINS` has three settings (no trailer.separators). `_GIT_ENV_DROP` is GIT_DIR, GIT_WORK_TREE, GIT_INDEX_FILE, GIT_COMMON_DIR, GIT_PREFIX, GIT_REPLACE_REF_BASE. Comment: "Inputs that could make git answer for another repo or lie about history."
- HY\tools\check_handoff.py:24-26 holds the same three pins and the same six drop names, word for word, and its comment at 37-38 says "so they cannot point it at another repo". It names no owner.
- Also found: GATES\publish_gate.py:51-52 refuses when GIT_DIR or GIT_WORK_TREE is set (refuse, not drop). HY\tools\effort_test_faults.py:97 passes only `-c core.quotePath=false`.
- Splits shown: (a) trailer.separators pinned in one file, not in the other two; (b) GIT_CONFIG_* dropped only in the judge, GIT_DIR and the other five only in the other two; (c) NO_REPLACE_OBJECTS and grafts file set only in the judge; (d) drop vs refuse for GIT_DIR/GIT_WORK_TREE (publish_gate).
- Situation where they differ: git run from inside a hook (git sets GIT_DIR itself; the judge keeps it, the others drop it), and a commit with replaced objects or grafts.
- Ruling: no `## Confirmed` line covers it (I grepped HY\docs\agents\rulings for python/amend/rebase/hand-typed/gh issue; nothing on git pins).

## 2. Status-call shape has two claimed owners: CONFIRMED
- GATES\status_append.ps1:1 "The call shape: the brief preamble's "Status lines" section." BP:19 "The status-call check in progress.py owns the exact shape." GATES\progress.py:22-27 holds STATUS_PREFIX and _STATUS_CALL; its comment calls STATUS_PREFIX "spelled as the allow rule in settings.fragment.json names it".
- Homes of the call text: GATES\settings.fragment.json:4 (`PowerShell(& "C:/Users/Patrick/.claude/gates/status_append.ps1"*)`), progress.py:22, BP:15, C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:100 (`... <your agent id> "HH:MM done <n> of <m> cards | next: <session>"`, "(not Bash)"). Note: the finder wrote the writer-brief path as under GATES\..\tools; it is C:\Users\Patrick\.claude\tools\session_cards\, not in the Hydra repo. Same file, so no change to the finding.
- Splits: (1) status_append.ps1 says BP owns the shape; BP says progress.py owns it (circular). (2) writer-brief.md:100 carries no copy of BP:19's no-`$`/backtick/double-quote rule; the card writer skips the preamble, so only the hook enforces it for that agent. (3) The allow rule (settings.fragment.json:4) and STATUS_PREFIX are two code-side copies; the finder says no test ties them; I did not grep the tests, so that part is "unchecked".
- Ruling: no Confirmed line on call shape. The status-lines demand rulings (review-and-hooks.md, agents-and-workflow.md) were not re-read by me.

## 3. The 30,000 figure in BP:35 has no code owner: CONFIRMED
- BP:35 states "Above 30,000 characters it arrives as Claude Code's own notice" and "shell_output.py ... owns the number and the lists". I grepped GATES\*.py and GATES\README.md for 30,000 and 30000; the only hits are tests sending a 30000-character string. shell_output.py holds CAP = 4000 only. The number sits in BP and in a probe note the finder cites (probe_results.md, not reopened by me).
- Split: BP's sentence "shell_output.py owns the number" is true for 4,000 and not for 30,000 (a Claude Code limit). Situation: output between 4,000 and 30,000 vs above.

## 4. BP:43 delete-rule git list is shorter than delete_rule.py: CONFIRMED, plus one new split
- BP:43 names git clean, reset --hard, reset --merge, checkout -f, worktree prune, stash drop, introduced by "such as". GATES\delete_rule.py:32-35 also refuses `switch --force/-f/--discard-changes`, `checkout .`, `restore .`, `branch -D`, `stash clear`, `worktree remove`. BP closes with "`delete_rule.py` ... is the one home", so the shorter list is an example, not a copy of the rule.
- NEW (finder left it unchecked): delete_rule.py:17-18 WORKTREE_FOLDERS is `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`, matching BP:43. This session's worktree is `...\hydra-test\worktrees\tooling-audit-records`, which is not under either root, so by reading a recursive delete there is refused (BP:43 itself says a worktree cut elsewhere is not a root). Not a conflict between BP and code; a conflict between where worktrees are actually cut and the allowed roots. A run of a delete would settle it; I ran none.
- Also: GATES\settings.fragment.json ask rules (rm, git clean, git reset --hard, Remove-Item, per the finder) are shorter than the agent refuse list; the main session gets the longer list unguarded. Not re-opened by me beyond the finder's reading.

## 5. Python path in many places; bare python.exe in the allow rule and ruling_rules.py: CONFIRMED
- GATES\settings.fragment.json:6 `PowerShell(python.exe -I C:/Users/Patrick/.claude/gates/ruling_rules.py --check *)`; GATES\ruling_rules.py:108 `CHECK_COMMAND = "python.exe -I C:/Users/Patrick/.claude/gates/ruling_rules.py --check"` (comment: the allow rule matches its start).
- HY\docs\agents\this-machine.md:4-8 says run `py` (pythoncore 3.14.5) and that a bare `python` runs C:\Python314\python.exe 3.14.7, "Use `py`, so every script runs on the same Python". GATES\README.md:110, 114, 145 use the full pythoncore path. BP:33 says `python -I <file>`; BP:97 says `py -I`.
- Splits: BP:33 `python -I` vs this-machine.md "Use `py`"; ruling_rules.py:108 and settings.fragment.json:6 bare `python.exe` vs README:114 (hooks use the full path). Open: which install a bare `python.exe` reaches inside a PowerShell tool call needs a run (`Get-Command python.exe`); this-machine.md says bare reaches the other install in both shells, which is a page claim, not checked by me.
- No test holds the path equal across homes (finder's claim; not re-grepped).

## 6. BP:71 "never ... reset or move a branch" is wider than the ref gate: CONFIRMED
- GATES\ref_judge.py:135-137: `shared = ref == MAIN or origin has refs/remotes/origin/<name>`; only a shared branch that moves to a non-descendant is refused (message at 100-103). BP:71 says never amend, rebase, reset or move a branch, then in its last sentence says the gate refuses only a main or origin-shared branch. HY\docs\agents\integrate.md:31 "Never rebase, amend, reset or force-move a branch."
- Situation: a local branch origin lacks (an agent's own task branch): instruction forbids, gate allows. Also BP:43 delete rule refuses `git reset --hard`/`--merge` for subagents but nothing refuses `reset --soft` or `commit --amend` on such a branch.
- Kind: instruction with a hook behind only part of it. Not a split in numbers; a split in scope. Ruling: none found Confirmed that covers it.

## 7. issue-tracker.md:14 and :46 tell agents to run hand-typed gh: CONFIRMED-CHANGED
- HY\docs\agents\issue-tracker.md:14 `gh issue close <number> --comment "..."` posts text. :13 `gh issue edit ... --add-label` and :46 `gh issue edit <n> --add-assignee @me` change state and carry no body text. HY\docs\agents\this-machine.md:15 "Text never goes to GitHub through a hand-typed `gh` command."
- GATES\settings.fragment.json deny list: I grepped for "issue close" and "issue edit": count 0; the issue lines present are `issue create` and `issue comment`. So `gh issue close --comment` is not denied.
- Change from the finder: the clash with this-machine.md:15 and doc-review.md:19 is real for :14 (the --comment is text). For :13 and :46 the finder's reading overreaches: they post no text, so "text never goes through hand-typed gh" is not contradicted; they are only gaps against the claim that hooks refuse "posting gh commands typed by hand" if that sentence is read as all gh writes (doc-review.md:19 wording is longer than I quoted; I read it as ending in "refuse the posting gh commands typed by hand").
- Split: issue-tracker.md:14 vs this-machine.md:15 (and doc-review.md:19 as the finder reads it). Hook gap (general words): a gh path that posts a comment exists outside the deny list.
- Ruling: none Confirmed.

## 8. integrate.md:27 "hold the last merge uncommitted" vs BP:71 "commit ... after each finished step": CONFIRMED
- integrate.md:27 "Hold the last task's merge uncommitted until the precheck in step 4 is clean". BP:71 "Commit before your 100th tool call, and after each finished step." Situation: an integrator whose precheck step runs past 100 calls; BP's 100-call warning is by the progress hook, so the warning fires with the merge uncommitted. Which wins is not stated in either page; BP:3 says where the brief and the preamble disagree, stop and report.
- Second split the finder listed, integrate.md:31 "force-move" vs BP:71 "move a branch": same family as item 6; same text, a little narrower wording on integrate's side.

## 9. Memory note contradicts itself: CONFIRMED
- C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\every-brief-carries-status-rule.md: front-matter description "...the dispatch gate now denies briefs that lack it" vs body "No gate checks a brief for this sentence today". My grep of GATES\review_dispatch.py for "status" (and "yet") returned nothing. A memory note is never the user's words, so this is a note error, not a ruling issue.

## Splits added by me
- New: delete_rule.py WORKTREE_FOLDERS vs the actual worktree location used in this session (item 4).
- Narrowed: issue-tracker.md :13 and :46 are not text posts (item 7).
- Same as finder: all others above.

## Not reached
I6-197 and I6-198 (finder's own gaps) and the finder's group F/G rows I did not pick as candidates. I did not open percall.py, tests, or rulings beyond the grep above. I ran nothing.
