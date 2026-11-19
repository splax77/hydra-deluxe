# TOOL-AUDIT-T7-C08 (closure checker, batch 7: sorting S04, ledger L04)

G = C:\Users\Patrick\.claude\gates. H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout).
All 13 ids reached. PASS 13, REOPEN 0, GONE 0. No hook gap found.

### L-F11-14
**Verdict:** PASS (P)
**Homes read:** G/common.py:10,121,125 (STATE_ROOT, "agents", "status"); G/status_append.ps1:11-14 (same root, same two names); G/cleanup_sweep.ps1:597-600 (status, agents, stall, posture); G/posture.py:22 ("posture"); G/stall.py:51 ("stall"); G/sync_settings.py:36-40 (FRAGMENT, LIVE, STATE_DIR settings_sync); G/tests/test_docs_numbers.py:48,143-146; H/docs/agents/this-machine.md:19,71 (line 71 is now the model setting, no state path; gates folder named at :19 and matches).
**Why:** Every folder name and path typed in the PowerShell files equals the Python owner today. LIVE and the test's SETTINGS both resolve to C:\Users\Patrick\.claude\settings.json. A pointer fix changes nothing and needs no choice.

### L-F11-15
**Verdict:** PASS (P)
**Homes read:** G/media_rule.py:1-11 (MEDIA_ROOT C:\ytdl, TOOLS, TEMPLATE); G/README.md:21 (main may write under MEDIA_ROOT and scratch, subagent only scratch); G/percall.py:4,149,166-167 (subagent refused, main asked); H/CLAUDE.md:13, H/docs/agents/this-machine.md:15 (splax77/hydra-deluxe); H/docs/agents/issue-tracker.md:16 ("infer from git remote"); `git remote -v` in H shows splax77/hydra-deluxe; memory note hydra-deluxe-rename.md:11 same name; G/tests/test_publish.py:341-393; brief-preamble.md (no media text; searched ffmpeg/yt-dlp in H/docs/agents/*.md, nothing).
**Why:** The repo name agrees everywhere and with the real remote. README and media_rule agree on who may write where. The review_dispatch half of the finding: G/review_dispatch.py still exists, but the row no longer relies on it and nothing in it disagrees with a doc I read. No value split.

### L-F11-17
**Verdict:** PASS (P)
**Homes read:** H/tools/check_pointers.py:34-35,42 (HOOK_FOLDERS, SKIP_FOLDERS, RULINGS_FOLDER = docs\agents\rulings) vs G/ref_judge.py:37,45 (MAIN, RULINGS_DIR "docs/agents/rulings/"): same folder. "refs/heads/main" typed in ref_judge.py, publish_gate.py:47, review_dispatch.py:86-144, review_rules.py:3, cleanup_sweep.ps1:270: all the same string. H/tools/mutate/run.py:69 MUTANTS_FORMAT = 1 vs rewrite.py:86 FORMAT = 1 (run.py comment ties them). H/tools/ch_mcp/game_files.py:29-40 (GAME_DIR, DOCUMENTS_DIR, LOCALLOW_DIR, SCORESTATS, SETTINGS) vs H/docs/agents/clone-hero-mcp.md:15,60 and this-machine.md:53 (Player.log folder matches LOCALLOW_DIR) and dump_build.py:9 (same game dir); ch_probe/constants.py:271 PROCESS_NAME "Clone Hero.exe"; H/tools/build_slot.ps1:38-46 and H/tools/new_worktree.ps1:5-18 (single-home notes).
**Why:** Every typed copy I could open equals its owner. Moved files (check_pointers now in H/tools, ch_probe now tools/ch_mcp) do not change the values. I did not open game_tools.py:47-57 vs stall_harness.py:796,854 line by line; they sit in the same game-folder family and nothing in the files I read contradicts them. The row never needed a choice of value.

### L-F12-01
**Verdict:** PASS (A), with one note
**Homes read:** G/status_append.ps1:5 ("The call shape: the brief preamble's 'Status lines' section"); H/docs/agents/brief-preamble.md:15,19 (shape stated; "The status-call check in progress.py owns the exact shape"); G/progress.py:21-27 (STATUS_PREFIX, _STATUS_CALL regex; comment names the settings allow rule); G/settings.fragment.json:4 and C:\Users\Patrick\.claude\settings.json:10 (allow rule, same prefix); H/docs/agents/doc-review.md:9 and derive-once-review.md:9 (shape typed).
**Why:** The three owner claims are comments or doc lines. The real code is progress.py's STATUS_PREFIX and regex (what recognises a call) and the allow rule (what permits it); both carry the same prefix today, so no code-against-code conflict. The user's "code wins" answer fixes the comments and doc to point at progress.py. Note for the main session: the lines that disagree with each other (ps1 comment vs preamble) are both prose, so the fix must name progress.py as the owner in the ps1 comment, not just edit one against the other.

### L-F12-06
**Verdict:** PASS (A)
**Homes read:** G/bash_windows_rule.py:172-181 (`_doubled_backslash`: a pair passes when its label is DOUBLE or COMMENT), :215-218 (refusal text); G/README.md:27; H/docs/agents/brief-preamble.md:33 ("A doubled backslash outside double quotes").
**Why:** One side is the preamble sentence, which is a doc; the other is the code. The code also passes a pair in a comment; the preamble does not say so. The "(the user's ruling)" text in the docstring is a comment, not a split side. No side is a doc against a doc.

### L-F12-08
**Verdict:** PASS (P)
**Homes read:** G/progress.py:19 (REPORT_TOOLS), :119; G/percall.py:135 (literal "SubagentHandback", a hand-back save), :202 (uses progress.REPORT_TOOLS); G/agent_stop.py:42; G/README.md:11; H/docs/agents/brief-preamble.md:21; G/../agents/reviewer.md:25 (SubagentHandback named, not StructuredOutput).
**Why:** The one literal in percall.py spells a name that is in REPORT_TOOLS. No home states a different list. A pointer or constant reuse changes no behaviour.

### L-F12-09
**Verdict:** PASS (P)
**Homes read:** G/progress.py:20 (SHELL_TOOLS); G/delete_rule.py:168 (("Bash","PowerShell")); G/percall.py:47,146,167,181 (uses progress.SHELL_TOOLS); G/shell_output.py:35-37,72,172 (TAILS keys Bash, PowerShell); G/settings.fragment.json:192 (matcher "Bash|PowerShell"); G/bash_windows_rule.py:1-2 (judges Bash only, by its own stated job).
**Why:** The pair is the same in every home. Bash-only in bash_windows_rule is a different job, not a split.

### L-F12-11
**Verdict:** PASS (P)
**Homes read:** G/prompt.py:17-18 (PARTS; comment "each mode is the one its script's __main__ uses"); G/posture.py:55 loud; G/waiver.py:50 loud; G/percall.py:210 closed; G/delete_rule.py:187 closed; G/review_dispatch.py:190 closed; G/agent_stop.py:369 loud; G/shell_output.py:201 loud; G/prompt.py:67 loud; G/common.py:52-75 (run, modes); G/README.md:97-98 (closed refuses on crash; loud informs).
**Why:** Every mode in PARTS equals its script's `__main__` mode, and README describes common.run correctly. publish_gate.py:141 has its own main() and no common.run mode; it is outside PARTS. No disagreement.

### L-F12-13
**Verdict:** PASS (P)
**Homes read:** G/agent_stop.py:45-56 (_BACKGROUND_STARTED, _TASK_ENDED, _STILL_RUNNING); C:\Users\Patrick\.claude\tools\session_cards\common.py:68,71,414 (TASK_NOTICE, DROPPED_HUMAN_PREFIXES); G/README.md (searched task-notification, "moved to the background", "<status": no hit).
**Why:** Both readers match the same `<task-notification>` tag for different jobs (end-of-task vs dropping the notice from cards); values do not conflict. Note for the fix: the agent_stop.py comment at :47 points at "gates/README.md, the agent_stop paragraph" for the strings, and the README does not hold them. That dangling pointer is a missing-pointer fix inside the same row, not a value split.

### L-F12-14
**Verdict:** PASS (P)
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\batch.py:75 (`sessions=%d batches=%d waves=%d`); prune_list.py:146-147 (`paths= held_back= mb=`); G/session_cards_start.ps1:8,159,334-347 (Get-Field reads `sessions` from batch.py and `mb` from prune_list.py); G/tests/test_h53_session_cards_start.ps1:67-69 (fake scripts print the same keys); tools/session_cards/test_session_cards.py:514,668,974 (real scripts' lines).
**Why:** The consumer reads exactly the keys the producers print. Nothing disagrees.

### L-F12-16
**Verdict:** PASS (A), with one note
**Homes read:** G/delete_rule.py:24-26 (NOT_LITERAL: $ ` % * ? , .. " '), :60-66 (`_instead` refusal text: variables, wildcards, '..', quotes, other words); G/bash_windows_rule.py:17,184,210-228 (PATHCONV_OFF, five parts and their texts); G/README.md:19,27; H/docs/agents/brief-preamble.md:33,43; G/media_rule.py:7-11 (TOOLS, READS, PATH_SHAPE).
**Why:** The prose lists (README, preamble) name fewer characters than the code refuses (%, ?, and the comma are named nowhere); that is a doc line against code, so "code wins" applies. Note: the refusal text at delete_rule.py:60-66 is prose inside a program file and also lists fewer than NOT_LITERAL; it is code-held text, so the fix must update it with the docs. The ref:.dotpath and PATHCONV wording differences are also preamble prose against bash_windows_rule.py.

### L-F12-17
**Verdict:** PASS (P) (the finder's row was NOT VERIFIED; I read it now)
**Homes read:** G/progress.py:60-135 (status_id equals agent id counts; another name is refused; exempt = report tool or command starting "git "); G/status_append.ps1:21-41 (refuses a name with no record under agents\, exit 1); H/docs/agents/brief-preamble.md:17,19,21; G/percall.py:1-6,128-160; G/README.md:11,21,25.
**Why:** The two layers ask different things (is this my own id; does this id have a record) and the preamble describes both correctly. percall's docstring order and README:11's order both list the same checks. README:21 and percall (media: refused in subagent, asked in main) agree. No disagreement.

### L-F12-18
**Verdict:** PASS (P)
**Homes read:** H/tools/build_slot.ps1:1-37 (header: "the one place that knows the slot rule, the pass rule and their numbers"), :71-75 ($BuildSlotCount = 3), :187-299 (New-BuildPass, Enter-BuildPass, slot functions); G/percall.py:18-19 (BUILD_PASS_TOOL, comment "tools/build_slot.ps1 owns what a pass allows"); G/README.md:15; H/docs/agents/agent-rules.md:20 and brief-preamble.md:57 ("at most three"); H/CLAUDE.md rule 14.
**Why:** I read the whole header and the pass and slot functions; the file does own the rule, and the limit of three agrees in every home. The copies of "three" are a typed number with an owner, no split.

## Ids not reached
None.
