# TOOL-AUDIT-T5-V16 (verifier of F16). Agent a7b3adc10e2c512b8. All claims "by reading"; I ran nothing.
"gates" = C:\Users\Patrick\.claude\gates. "Hydra" = C:\Users\Patrick\Downloads\Hydra\hydra-test.

Rulings read (docs/agents/rulings, `## Confirmed` only):
- review-and-hooks.md:10 (Confirmed) "why did you only show me those two if there are more important patterns? yes block those too". Names one side (block the extra shapes). It does not speak to quote reading.
- review-and-hooks.md:17 (Confirmed) chose "Keep rule, move worktrees (Recommended)". Speaks to where worktrees sit for the delete rule. Does not speak to the git list or main-session asking.
- agents-and-workflow.md:8-9 (Confirmed) "agents are never allowed to start their own builds, only the main session..." and "any tests that require a build are done by the main session, not agents." Backs the build-pass refusal in percall.py and README:15-17.
- review-and-hooks.md:40 "Exempt git commit" is under `## Unconfirmed` (line 37 heading). Confirmed by reading.
- "losing battle": grep of Hydra/docs finds it only in two plans and the card docs/sessions/2026-10-31-6777b546.md:25. Not in any rulings file. Confirmed.
- "key allow" and "Lock it too" (game_tools.py comment): no match in docs/agents/rulings.
- "a pair in double quotes passes (the user's ruling)": bash_windows_rule.py:178 and probe_results.md:96 say so; no ruling id and no matching Confirmed line found in review-and-hooks.md (searched for Confirmed block text). Open: searched only review-and-hooks.md and agents-and-workflow.md, not every rulings file.

## Q1 One reader of quote characters (I9-060, I9-189)
Verdict: CONFIRMED-CHANGED. K6 is real; the K8 is overstated.
- bash_windows_rule.py:5-6 says "_scan is the one reader of shell quoting: every part works from what it returns", and "this module owns ... the five refused shapes". README:27 says "One scanner, `_scan`, reads quoting for all five parts, so nobody writes a second one." Both sentences are scoped to that rule's five parts, not to every hook. So the K8 ("wrong owner claim") is weak: the claim is true for what it covers.
- K6 stands: other code reads quotes separately. media_rule.py:15 `dr._word(token.strip("\"'"))`; media_rule.py:31 counts quote characters (`odd`); media_rule.py:51 strips a matching pair; delete_rule.py:90 `_inside` strips a matching pair and NOT_LITERAL (:26) lists `"` and `'`; shell_output.py:45-52 `_unquoted`. Also bash_windows_rule.py:141 reads `piece[at].raw != piece[at].value` and then calls delete_rule._word, a fourth reading.
- Situation: the first word of a command is `"cmake.exe"` (quoted). delete_rule.py:3-5 and README:19 say a quoted delete is not read. shell_output._unquoted (docstring :46-47: "because that rule leaves a quoted delete unread on purpose") reads it as `cmake` for the exit-code tail. media_rule._word reads `"ffmpeg.exe"` as ffmpeg. bash_windows_rule:141 treats a quoted word as not the name. Four answers to "is a quoted first word the command?".
- SPLIT (F16 found): quoted first word unread by delete_rule, read by shell_output, read by media_rule, unread by bash_windows_rule:141 (the last is my addition).
- Second cutter, K6 confirmed: delete_rule.py:75-77 `pieces` says "the one reader of where a command is cut". media_rule._segments (:18-27) cuts again: at standalone operator words only (an operator glued to a word is not a cut), where delete_rule.OPERATORS.split cuts inside words. bash_windows_rule.py:19-20 OPERATOR_CHARS cuts a third way (quote-aware). Edge: `a;b` unquoted inside a word is split by delete_rule and shell_output, not by media_rule.
- I9-189 (preamble:33 vs code): preamble:33 lists the five shapes and the 1,000 limit. test_docs_numbers.py:192-196 `test_preamble_quotes_the_bash_limit` holds 1,000 equal to MAX_CHARS. Nothing holds the other four shapes. K1/K5 confirmed.
- Ruling: none names both sides.

## Q2 Delete and git-discard list and roots (I9-062 to I9-067)
Verdict: CONFIRMED (K1, K3, K6).
- Owner delete_rule.py:17-35 (WORKTREE_FOLDERS, DELETE_WORDS, GIT_DISCARD_WORDS, GIT_DISCARD_ACTIONS). `_instead` text :60-63 restates both folders.
- README:19 gives the long list and says the three constants "are the one list". It includes `git switch --force/-f/--discard-changes`, `git checkout .`, `git restore .`, `git stash drop` and `clear`, `git worktree remove` and `prune`, `git branch -D`, `git branch -d --force`. Matches code.
- preamble:43 lists "such as `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`", and then adds "no agent task needs them". It names fewer than the code refuses. Because it says "such as" it is an example list, not claiming completeness. README:19 says it spells the full list. SPLIT (docs): two lists, a short one in the preamble and a long one in README; the preamble does not say it is partial except by "such as".
- preamble:43 spells both worktree folders and the strictly-inside rule; derive-once-review.md:197 states both paths again (Hydra, per F16; I did not reopen that line).
- Main session: settings.fragment.json:59-64 `ask` holds exactly four entries (`Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`). replay.py:88 MAIN_ASK is a copy of those four. README:11 says delete_rule runs in a subagent only. So SPLIT (K3): an agent is refused `git worktree prune`, `git stash drop`, `git checkout -f`, `git branch -D`, `git restore .`, `rmdir`, `del` and so on; a main session is asked only for the four shapes and is not asked or refused for the rest. The preamble and README describe only the agent side. No Confirmed ruling names both sides.
- Root folders: new_worktree.ps1:6,29 cuts at `<main>\.claude\worktrees\<TaskId>`. delete_rule roots are `...\hydra-test\.claude\worktrees` and `C:\Users\Patrick\.claude\worktrees`. These agree today. delete_rule.py:100 `len(parts) > 1` keeps the worktree's own folder out.
- F16 side note CONFIRMED: this verifier's cwd is `...\hydra-test\worktrees\tooling-audit-records`, which is under neither WORKTREE_FOLDERS entry; an agent there could not run a recursive delete in it. By reading delete_rule.py:17-18 and :97-102. A situation for the user: the ruling review-and-hooks.md:17 chose "worktrees\" under the repo's .claude; this worktree sits beside it.
- I9-067 crash mode: percall.py:176 `common.run(body, "closed")`, prompt.py:18 `("percall", "closed")`, common.py:53-70 defines the mode. K1: two places state "closed" for percall, and they agree. Whether prompt.py's value matters when percall runs as a prompt hook is a run question (I did not read prompt.py's use of the mode).
- Hook gap: progress.py:119 `command.startswith("git ")` exempts the whole command from the budget refusal; it is checked before :123 (helper-name) and :129 (budget).

## Q3 Which Clone Hero calls press keys (I9-073, I9-095)
Verdict: CONFIRMED (K6 on the string "keys", K1 on the allow text).
- percall.py:16-18 `KEY_TOOLS`, `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`; game_tools.py:70-72 `ROUTE_KIT = "kit"`, `ROUTE_KEYS = "keys"`, `ROUTES`. The hook cannot import the server's constant. Tests: percall tests found in gates/tests/test_percall_key.py and test_progress.py; nothing in tools/ch_mcp reads percall.py (grep of ch_mcp for "percall|gates": only a comment at front.py:493 and unrelated test lines). So nothing holds "keys" equal across the two. The two agree today.
- Values: no drift. README:13 prose agrees with percall.py. No Confirmed ruling for the allow-key rule or "Lock it too" found in docs/agents/rulings (grep).
- Situation: a route-less `ch_play_chart` passes the hook; game_tools.py:1097-1108 treats route None as the kit when set up and refuses with a hint otherwise. These agree.

## Q4 Exit-code tail, cap, trims (I9-076, I9-147 to I9-153)
Verdict: CONFIRMED (K1, K5; K6 for the command cut).
- shell_output.py:14-15 "The one home of these numbers and lists ... Docs point here." CAP 4000. README:23-25 and preamble:35 state it. test_docs_numbers.py:198-202 holds 4,000 equal; nothing holds 30,000, 1,500 or the lists (grep of that file for 30,000 and 1,500 found none).
- delete_rule.word/pieces used by shell_output.py:59-60 (and git_words :132). media_rule uses `dr._word`. Second cutters: media_rule._segments and bash_windows_rule OPERATOR_CHARS (see Q1).
- Splits: none in value. Pointer loop F16 noted (I9-148 comment vs README:25) not re-opened.

## Q5 Status call and budget (I9-087, I9-088, I9-091)
Verdict: CONFIRMED (K1/K5), with the edge split confirmed.
- progress.py:14 `CALL_WARN, CALL_STOP = 100, 150`; :91 increments `record["calls"]` first; :129 refuses when `record["calls"] >= CALL_STOP`. So call 150 is the first refused; 149 pass. preamble:73: "At 150 tool calls or 30 minutes it stops you"; :71 "Commit before your 100th tool call". SPLIT (edge): docs read "at 150 it stops you", code refuses the 150th call itself. Warn: at :92 the call 100 gets the warning and still runs. Docs wording is ambiguous, not contradictory; no ruling.
- :119 exempt = report tools or `command.startswith("git ")`: the hook gap above. preamble:21 says "a command that starts with `git`", matching.
- test_docs_numbers.py:110-127,182-190 hold call/minute numbers and STATUS_PREFIX in docs. Writer brief and other copies not re-opened.

## Q6 gh posting (I9-106)
Verdict: CONFIRMED (K3, K8-like doc split).
- fragment denies (Bash, PowerShell, `gh.exe`, `& gh`, full-path forms): pr create, release create, release edit, issue create, issue comment, pr comment, api. It does not deny `gh issue close`, `gh issue edit`, `gh pr edit`, `gh pr close`. Grep of gates *.py/*.ps1/*.json/*.md for "issue close|issue edit|pr edit|--comment" found no other hook for them.
- issue-tracker.md:5 says text reaches GitHub only through publish.ps1 and "the settings refuse the posting `gh` commands typed by hand". issue-tracker.md:14 `gh issue close <number> --comment "..."` posts text by hand and is not denied. :13,:42,:46 `gh issue edit`; :26 `gh pr comment` (denied by the fragment) and `gh pr edit`, `gh pr close`. SPLIT (docs against settings): :26 tells a session to run a command the settings refuse; :14 tells it to post text outside publish.ps1. publish.ps1:8 has kinds pr, release, release-edit, issue, issue-comment, pr-comment, so :26's `gh pr comment` has a publish kind. No test holds the deny list equal to publish kinds (by reading; test_publish.py not opened).
- Ruling: none Confirmed found naming both sides.

## Q7 Wake, ruling check command (I9-124, I9-145)
- Ruling check: fragment :6 allow rule; test_docs_numbers.py:139 `"PowerShell(%s *)" % ruling_rules.CHECK_COMMAND`. Confirmed a held copy. Wake not opened by me: NOT REACHED.
- I9-177 harness: NOT REACHED.

## Q8, Q9 (I9-070, I9-180, I9-181, I9-183)
- Media guard second cutter and quote handling confirmed as above (media_rule.py:6 MEDIA_ROOT `C:\ytdl`). Hydra docs "ytdl" absence not rechecked. Replay MAIN_ASK equals the fragment ask list today (confirmed, K1). I9-180/181/183 otherwise NOT REACHED.

## Q10 Check tools (I10-003, -008, -013, -016, -023, -027, -028)
- I10-008 / F16's K8 candidate (handoff.md:59 says check_handoff checks lines naming the hooks repo; no matching code): REFUTED. check_handoff.py:136-153 `other_repos` finds absolute paths on each line via doc_tokens.absolute_paths, and checks lines naming another repo's root folder in that repo. That covers the hooks repo's root without the word "hooks". handoff.md:59 says "A line that names the hooks repo's root folder is also checked in that repo" and is true for the repo root; F16's grep for "hooks" missed the generic code. Separate: HOOKS_REPO_WORDS ("hooks repo", check_pointers.py:40,166) is a second way a line picks the hooks repo, used by check_pointers only. The two ways differ: check_handoff picks by a named root folder; check_pointers by the words. Candidate K6 across two tools, not K8. Not tested by me for what each prints.
- I10-016 git pins: CONFIRMED K6/K2. check_handoff.py:25 `_GIT_PINS` (quotePath, showSignature, useReplaceRefs), review_rules.py:61 same, ref_judge.py:34-35 `PINNED` adds `trailer.separators=:`. Env drop lists: check_handoff.py:26 and review_rules.py:62 (same words; check_handoff's second line not opened). effort_test_faults.py:97 pins only quotePath. SPLIT: ref_judge pins one more setting than the other two. No test holds them equal (by grep; not exhaustive).
- I10-003, -013, -023, -027, -028 NOT RE-CHECKED.

## Q11 Build pass (I10-047, -049, -065)
Verdict: CONFIRMED (K6).
- build_slot.ps1:90 `Test-BuildPassGated { $env:CLAUDECODE -eq '1' }`; Directory.Build.targets:9 `Condition="'$(CLAUDECODE)' == '1'"`; development.md:143 "The limit applies only when `CLAUDECODE` is 1". The target's condition is a second copy of the gate; the .targets file cannot import the function.
- percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`; hydra_ui_mcp.py:874 `def hydra_build_pass()`; README:15 names `BUILD_PASS_TOOL`. Tests in gates/tests mentioning it: test_progress.py and test_percall_key.py (grep). Nothing compares to the server's name. Server prefix `mcp__hydra__` depends on the MCP server's registered name (a run question).
- Ruling agents-and-workflow.md:8 names "agents are never allowed to start their own builds, only the main session"; it names the rule, not the tool name.
- -DryRun and check-mode text NOT RE-CHECKED.

## Q12, Q13 (ui MCP text, CH tool values, precheck, mutate)
NOT REACHED: I10-067, I10-068, I10-080, I10-082, I10b-010, I10b-075, I10b-104.

## Pointer drift
- CONFIRMED-CHANGED: docs/superpowers/plans/2026-11-07-ch-run-timing.md:31 and :457 cite "README.md line 15" for the losing-battle quote; the quote is at README:19. But 2026-11-07-build-limit.md:130 cites the README without a line number, so F16 was wrong about it. Only ch-run-timing drifts.

## Splits I add or change
1. Quoted first word: four readers, not three (adds bash_windows_rule.py:141).
2. Command cut: three cutters (delete_rule.OPERATORS, media_rule._segments with standalone-operator rule, bash_windows_rule.OPERATOR_CHARS).
3. issue-tracker.md:14 `gh issue close --comment` posts text by hand and no deny rule covers it.
4. Main session is only asked for four shapes; agents are refused a longer list (F16 split 1, confirmed).

## Candidate tally
Checked: Q1 (2 rows), Q2 (5), Q3 (2), Q4 (6 incl. media cut), Q5 (3), Q6 (1), Q7 ruling check (1), Q10 (3: -008, -016 and handoff claim), Q11 (3), drift (1).
