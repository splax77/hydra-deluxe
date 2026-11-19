# TOOL-AUDIT-T9-K3 closure check (18 ids)

Summary: 17 PASS, 1 REOPEN (L-F09-06), 0 GONE. Not reached: none.

### L-F08-15
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** delete_rule.py:75-77 pieces() cuts at OPERATORS, reads the first word of each piece; media_rule.py:14-15,18-27,30-44 has its own segment loop and finds the tool word anywhere in a segment, skipping odd-quote prefixes.
**Why:** both sides are code answering "what command does this text name" differently. Kind matches.

### L-F09-06
**Verdict:** REOPEN.
**Homes read:** C:\Users\Patrick\.claude\tools\session_cards\common.py:266 ("The user ruled on 2026-10-31 ..."), :278-289 (marked_local, is_local_only "Every script asks here, never by its own test."); check.py:234-239 keeps_local combines both; check.py:316 and prepass.py:461 each build `[common.HYDRA_REPO, common.CLAUDE_REPO]`; writer-brief.md:3,16 "The user ruled on four kinds of content on 2026-10-31". docs\agents\rulings\ grepped for "local card", "locally", "2026-10-31", "not about the project": no Confirmed line holds it. docs\sessions\ not needed since only Confirmed lines count.
**Why:** the claim split is covered by "Drop the claim" (no Confirmed line; claim still in common.py:266 and writer-brief.md:16). But the row carries more splits that answer does not cover: the default repo list is typed in two scripts (check.py:316, prepass.py:461, code against code), and keeps_local ORs two local tests while common.py:286 says every script asks one place (docstring against code). The row should leave by "Drop the claim" for the claim plus "Merge, I pick" (repo list) and "The code wins" (docstring), not by "Drop the claim" alone.

### L-F09-09
**Verdict:** PASS (exit 3 "Give each a home").
**Homes read:** C:\Users\Patrick\.claude\agents\scout.md:9 ("Change no files except your status line and files in your own scratch folder"); gates\*.py grepped for scout, agent_type, subagent_type, read-only, write scope: only the reviewer type is judged (percall.py:104, review_dispatch.py:167,274, agent_stop.py), scout appears only in tests/test_agent_stop.py. brief-preamble.md:3,41 agree.
**Why:** the rule lives in prose only, no hook, and nothing contradicts it (granting Edit/Write is needed for the status line and scratch).

### L-F10-06
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** squeeze-tools\squeeze_scout.py:38,153,974 (`--replay`); video-tools\fcvideo.py:2906 and replay.py:30,47, video-tools\README.md:78 (`--hydra-replay`). skills\analyzing-sp-squeezes\SKILL.md:18 names squeeze_scout's flag.
**Why:** two tools (code) use two flag names and each types the default path. Both sides code.

### L-F10-08
**Verdict:** PASS (exit 3 "Drop the claim").
**Homes read:** memory\windows-installer.md:15 "That is the user's design"; installer permissions at hydra.iss. Grepped docs\ (rulings, sessions, decision log) for "users-modify": no match; the rulings folder has no installer-permission line.
**Why:** a note says the user decided it; no Confirmed line (or card) holds it.

### L-F10-09
**Verdict:** PASS (exit 3 "Give each a home").
**Homes read:** docs\agents\this-machine.md (grep dumpbin|ffmpeg: none); memory\mimalloc-link-order.md:15 holds the dumpbin path; ffmpeg path only in notes (archive gameplay-video-score-analysis.md:17 says `C:\ytdl`); video-decode-ceilings.md, stitched-chart-auto-fills.md mention ffmpeg.
**Why:** the paths live only in memory notes; this-machine.md says nothing different, so it is homeless, not split.

### L-F10-11
**Verdict:** PASS (exit 3 "Trust both").
**Homes read:** delete_rule.py:14-18 (two folders, `.claude\worktrees` of Hydra and of ~/.claude); cleanup_sweep.ps1:174-178 (`<root>\.claude\worktrees`, plus `<root>\worktrees` in the hooks repo); rulings\review-and-hooks.md:17 (Confirmed 2026-10-25, "Keep rule, move worktrees": the rule lets an agent delete inside a worktree that sits in a `worktrees\` folder).
**Why:** the split is which folders the delete rule accepts; "both" covers it and the Confirmed line does not name a narrower set. Today delete_rule.py still accepts only the two folders, so H\worktrees\<task> is refused until the fix is made.

### L-F10-22
**Verdict:** PASS (exit 3 "Give each a home").
**Homes read:** skills\reverse-engineer-anything\SKILL.md:147-154,183 (sigrok, WABT via REA_WABT_BIN_DIRECTORY, Node 24.x); docs\agents\this-machine.md:25 holds only the rea-agents@6.5.0 start line, none of those tool paths.
**Why:** the skill needs tool paths/versions that no doc or hook holds, and nothing contradicts it.

### L-F11-01
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** tools\build_slot.ps1:10,78 (`C:\Users\Patrick\.claude\hooks\state\build_slots`); gates\common.py:10 `STATE_ROOT = <gates>\state`. Grep of gates for build_slots: none.
**Why:** two scripts (code) hold two state folders.

### L-F11-02
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** tools\find_cmake.ps1:11,17 (-latest, then -all -prerelease); build_cpp.ps1:56-60 (-all -prerelease); .github\workflows\ci.yml:326 (-latest -all).
**Why:** three code files choose the Visual Studio install differently.

### L-F11-03
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** tools\mutate\hydra_mutant.h:41-53 (unset is -1, bad text aborts); tools\mutate\py_mutate.py:63 `int(os.environ.get('HYDRA_MUTANT') or 0)`.
**Why:** C++ and Python read the same variable differently. Both code.

### L-F11-11
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** tools\hydra_ui_mcp.py:301-318,344,599-600 (hydra_live_ under a swept temp root; cleanup_sweep owns age); tools\ch_mcp\game_tools.py:47,1402 (ch_live_ prefix; only test_game_tools.py:386 references it; no removal found by grep of tools\ch_mcp for rmtree).
**Why:** two tools (code) place and clean scratch folders differently.

### L-F11-16
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** gates\tests\test_ref_judge.py:24 `GATE_FILES = ("ref_gate.sh", "install_ref_gate.ps1", "ref_judge.py", "common.py")`, :719-721,874 later copy review_rules.py and ruling_rules.py per test class.
**Why:** a test's file list against what ref_judge loads at run time; both code.

### L-F12-02
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** delete_rule.py:30 GIT_VALUE_OPTIONS (seven), :105-111 git_words; bash_windows_rule.py:25 GIT_OPTIONS_WITH_VALUE = ("-C", "-c"), :146-153.
**Why:** two code lists of git options that take a value still differ. shell_output.py:130-132 already uses delete_rule.git_words, so bash_windows_rule is the one left.

### L-F12-03
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** delete_rule.py:29,75-77 (quote-blind pieces); bash_windows_rule.py:40 _scan (quote-aware); media_rule.py:18-27 (own loop using dr.OPERATORS).
**Why:** three code cutters still exist.

### L-F12-04
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** delete_rule.py:66-69; shell_output.py:45-52 _unquoted ("lives here and not in delete_rule.word, because that rule leaves a quoted delete unread on purpose"); media_rule.py:14-15 strip quotes; bash_windows_rule.py:137-143 _command_at refuses a quoted first word.
**Why:** four code readers of a quoted command word disagree.

### L-F12-05
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** gates\percall.py:65-70 (ch_call to ch_call counted as a key call); tools\ch_mcp\front.py:502-503 (refused: does not go through ch_call).
**Why:** hook and server (both code) answer it two ways.

### L-F12-07
**Verdict:** PASS (exit 3 "Merge, I pick").
**Homes read:** gates\settings.fragment.json:59-63 ask list (rm, git clean, git reset --hard, Remove-Item); delete_rule.py:19,31-35 wider refusal list.
**Why:** two enforcing code/config lists for which deletes need a yes. Both code.

Ids not reached: none.
