# TOOL-AUDIT-T5-F16 (finder F16, 51 rows). Agent a0013ab9b7ef3c01c, session f6a384b5-9eed-4607-95cb-681ec635d569

Read-only. I stopped at the 100-call warning. Rows are grouped by question because many share homes. A "gates" path means C:\Users\Patrick\.claude\gates; "Hydra" means C:\Users\Patrick\Downloads\Hydra\hydra-test. Row text was read from the T3 reports (copied to scratchpad rows.txt).

Rows not reached in depth (searched once or not at all, so "no home found" here proves little): I9-177, I9-180, I9-181, I9-183, I9-070 (media: only code read, docs searched once), I10-080, I10-082, I10b-013, I10b-029, I10b-045, I10b-057, I10b-067, I10b-068, I10b-073, I10b-099. Rows partly done: I10b-010, I10b-075, I10b-104.

Rulings found (## Confirmed, quoted exactly)
- Hydra docs/agents/rulings/review-and-hooks.md:17 (2026-10-25) chose "Keep rule, move worktrees (Recommended)" to a question about the delete rule letting an agent delete only inside its scratch folder or a worktree in a `worktrees\` folder.
- review-and-hooks.md:10 (2026-11-06) "why did you only show me those two if there are more important patterns? yes block those too" (the extra Bash Windows shapes).
- review-and-hooks.md:40 (2026-11-06) "Exempt git commit" and :43 "length over 1000 and backslashes" (these two sit under Unconfirmed, I read them as lines 40-43 after `## Unconfirmed` at line 37; they are NOT Confirmed). The Confirmed block runs lines 7-35.
- agents-and-workflow.md:8-9 (2026-11-07) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." and "any tests that require a build are done by the main session, not agents."
- Not found under Confirmed: the user words quoted by gates/README.md:19 ("trying to ban specific command patterns is a losing battle...", 2026-10-31); only in session card docs/sessions/2026-10-31-6777b546.md:25 and two plans. Not found: any Confirmed line for "a pair in double quotes passes", though bash_windows_rule.py:178 and probe_results.md:96 both say "the user's ruling".

## Q1. Is there only one reader of quote characters in a shell command? (I9-060, I9-189)
Homes:
- gates/bash_windows_rule.py:5-6 "_scan is the one reader of shell quoting: every part works from what it returns." README.md:27 "One scanner, `_scan`, reads quoting for all five parts, so nobody writes a second one."
- Other quote readers elsewhere in the same hook set: gates/media_rule.py:15 `token.strip("\"'")`; media_rule.py:31 `odd = ... count('"') % 2 or ... count("'") % 2` ("a quote is open"); media_rule.py:51 strips a matching pair of quotes; gates/delete_rule.py:90 `_inside` strips a matching pair, NOT_LITERAL :26 lists `"` and `'`; gates/shell_output.py:45-52 `_unquoted` ("words with a quoted first word made plain"); gates/progress.py:26 `_STATUS_CALL` regex bars quote shapes.
- The test (test_bash_windows_rule.py:313-327) holds it only inside bash_windows_rule.py.
Kinds: K6 (several code places read quoting; the "one reader" claim covers one file), K8 for the "nobody writes a second one" line in README:27 and the module docstring.
Table: bash_windows_rule tells every caller (main session and every agent) to treat quoting via _scan; media_rule, delete_rule, shell_output each keep a private cut. delete_rule.py:3-5 says a quoted delete is "not read"; shell_output.py:47 says it reads a quoted word "because that rule leaves a quoted delete unread on purpose"; media_rule reads it. Three different answers to "is a quoted first word a command?".
Split: delete_rule.word treats `"cmake.exe"` as unread (test_shell_output.py:99-100 pins this), shell_output._unquoted treats it as `cmake`, media_rule._word treats `"ffmpeg.exe"` as ffmpeg.
Ruling: none Confirmed for double quotes (see above).
Test holding homes equal: none across files.
I9-189 homes: gates/tests/probe_results.md:85-102 (the measurements), bash_windows_rule.py:216,226 (refusal texts restate the measured facts), brief-preamble.md:33 (states the five shapes, 1,000 limit, backslash/`cmd`/ref shapes in prose). test_docs_numbers.py:192-196 holds the 1,000 in the preamble equal to MAX_CHARS; nothing holds the other four shapes in the preamble equal to the code. K1/K5.
Edge: main session against agent: README:27 says the rule refuses in both; preamble line 33 speaks only to agents. Card writers skip the preamble but the hook still refuses them (percall applies to every agent).

## Q2. What is a subagent delete / git discard refused for, and where are the roots? (I9-062, I9-063, I9-064, I9-065, I9-067)
Homes:
- Owner: gates/delete_rule.py:17-35 `WORKTREE_FOLDERS`, `DELETE_WORDS`, `GIT_DISCARD_WORDS`, `GIT_DISCARD_ACTIONS`; :58-63 the `Instead:` text restates the two worktree folders and says "Never run git's discarding commands in an agent."
- gates/README.md:19 spells the git list in plain words ("`git clean`, `git reset --hard` and `--merge`, `git checkout` or `switch` with `--force` or `-f`, ..., `git branch -D`, and `git branch -d` with `--force` or `-f`") and says "`WORKTREE_FOLDERS`, `GIT_DISCARD_WORDS` and `GIT_DISCARD_ACTIONS` in that file are the one list." README:136 repeats the worktree folder rule.
- Hydra docs/agents/brief-preamble.md:43 spells the delete rule: both worktree paths, the git refusals ("`git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`") and the "not read" shapes.
- Hydra docs/agents/derive-once-review.md:197 states both worktree paths again.
- gates/settings.fragment.json:59-63 (and live settings.json:69-73) `ask`: `Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`, a third list of the same words, for the main session.
- gates/tests/replay.py:88 `MAIN_ASK = {"Bash": ("rm ", "git clean", "git reset --hard"), "PowerShell": ("remove-item ",)}` a copy of that ask list ("The main-session ask rules (plan Q4, settings.fragment.json)").
- Hydra tools/new_worktree.ps1:6,29 creates worktrees at `<main checkout>\.claude\worktrees\<TaskId>`; tools/effort_test_guard.ps1:17,105 names the same folder; gates/tests/test_delete_rule.py:29 and test_media_rule.py:28 copy the Hydra path.
Kinds: K1 (README list, preamble list, replay MAIN_ASK, fragment ask), K6 (new_worktree.ps1 decides where worktrees live, delete_rule decides which count), K3 (main session is only asked for `rm`/`git clean`/`git reset --hard`/`Remove-Item`; an agent is refused a larger list; the preamble says only the agent half).
Table of what each home says:
| home | agent | main session | at the limit/edge |
| delete_rule.py | refuses delete outside roots and the git list | not run for main (percall: delete_rule is subagent only; README:11) | a worktree's own folder never passes (`len(parts) > 1`) |
| preamble:43 | same, adds "any worktree directly inside a `worktrees` folder" | silent | says paths "strictly inside" |
| settings ask | n/a | asks for four command shapes only (not `git worktree prune`, `git stash drop`, `git checkout -f`, `git branch -D`) | |
| new_worktree.ps1 | cuts at .claude\worktrees | same | |
Worktree against main checkout: a worktree under `...\hydra-test\.claude\worktrees\` or `C:\Users\Patrick\.claude\worktrees\` passes; the main checkout itself is never a root. Side note: this very scout's cwd is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records`, which is under neither WORKTREE_FOLDERS entry, so an agent cut there could not delete its own files.
Splits (own lines):
- SPLIT: the main session is not refused `git worktree prune`, `git stash drop`, `git checkout -f`, `git branch -D` by any rule (only asked for four shapes), while agents are refused all of them; README:19 and preamble:43 only describe the agent side. Whether that is intended is not stated in a Confirmed ruling.
- SPLIT (docs): preamble:43 lists "git clean, git reset --hard, git reset --merge, git checkout -f, git worktree prune and git stash drop" as "such as"; README:19 lists a longer set (adds switch, restore ., branch -D, -d --force, checkout .). Both say they are the list.
- Pointer drift: docs/superpowers/plans/2026-11-07-ch-run-timing.md:31,457 and 2026-11-07-build-limit.md:130 cite "gates\README.md line 15" for the "losing battle" quote; it is now at line 19.
Ruling: review-and-hooks.md:17 (above) speaks to the worktrees folder rule (move worktrees under `worktrees\`).
Test holding homes equal: none. test_delete_rule.py pins the code; no test compares README:19, preamble:43, fragment ask or replay.MAIN_ASK to delete_rule.py.
I9-067 (crash/entry): common.run(body, "closed") at delete_rule.py:187; the mode word is also written in prompt.py:18 (`("percall", "closed")`) and percall.py:176; README:97 "A crash denies the call". K1/K6 across three files for the mode of percall.
Hook gap (general words): progress.py treats any command starting with `git ` as exempt from the spent-budget refusal, whatever follows it on the line.

## Q3. Which Clone Hero server calls count as pressing keys? (I9-073, I9-095)
Homes:
- Owner: gates/percall.py:15-18 `KEY_TOOLS`, `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`; `_key_call` :77-81.
- gates/README.md:13 lists the same set in prose, including the ch_call forms, and says "`_key_call` in `percall.py` owns which calls count."
- gates/review_rules.py:54 `KEY_ALLOW_TEXT = "allow key run"`; also written in Hydra tools/ch_mcp/server.py:147-148 (the server's own description) and :466,:477 (docstrings of ch_press/ch_key), docs/agents/clone-hero-mcp.md:29, README:13/37.
- Hydra tools/ch_mcp/game_tools.py:70-72 `ROUTE_KIT = "kit"`, `ROUTE_KEYS = "keys"`, `ROUTES`: the route values are decided here too, with the comment citing "the user's answer to the plan's question 6, 2026-10-30, "Lock it too"" (not in Confirmed rulings by my grep).
Kinds: K6 (the string "keys" is the route name in game_tools.py and in percall.py; the hook cannot import it), K1 (allow text in four files; server.py tells agents the rule in prose, K5).
Table: percall refuses a counted call for main session and every agent unless an allow was typed (README:13). server.py descriptions tell the model "a hook refuses them unless the user has typed allow key run". game_tools: route None with the kit not set up is refused, route None with the kit set up uses the kit, so the hook passing route-less calls and the tool agree.
Split: none found in values. Test holding equal: test_percall_key.py and test_progress.py:611-696 pin the percall side; nothing compares percall's "keys" to game_tools.ROUTE_KEYS.

## Q4. When and how does a build/test command get the exit-code tail; what is trimmed; what does the marker do? (I9-076, I9-147, I9-148, I9-149, I9-152, I9-153)
Homes:
- Owner: gates/shell_output.py:14 "The one home of these numbers and lists... Docs point here." CAP 4000 :15, HEAD/TAIL 1500 :16, ERROR_LINES :17, BUILD_TEST_COMMANDS :21, PYTHON_TEST_FLAGS :24, NO_TRIM_COMMANDS :25, TAILS :35-38, MARKER/FAILED_LINE :30-31.
- Hydra brief-preamble.md:35 states 4,000 characters, 30,000, "a build or test command gets one line appended that prints its real exit code", "never end one with your own `exit N`", and says shell_output.py owns the number and lists. gates/README.md:25 describes the same (names BUILD_TEST_COMMANDS and PYTHON_TEST_FLAGS as the lists, repeats the `exit N` caveat).
- gates/tests/probe_results.md:133 "a 4,000 cap and 1,500 characters at each edge"; :206-212 tables at 4,000/8,000; gates/tests/shell_output_sizes.py:18-19 CAPS/TABLE_CAPS and `AT_FAILURE_CEILING = 9900`; gates/tests/replay_extract.py:28 `REFUSAL_TEXT_CAP = 4000` (an unrelated 4000 that equals CAP).
- gates/tests/test_percall_rewrite.py `BASH_TAIL` and test_shell_output.py:16-18 each write the tail text literally (two test copies of TAILS).
- Hydra docs: no other home of the cap found (CLAUDE.md, docs/agents/*, docs/development.md searched for "real exit code", "COMMAND FAILED", "4,000 characters").
Kinds: K1 (preamble:35, README:25), K5 (preamble tells agents what the hook does and what not to type), K6 (cap numbers repeated in shell_output_sizes.py and replay_extract.py as separate numbers).
Table: shell_output rewrites for both sides (percall runs it in main session and agents, README:11); preamble speaks to agents only. Command lists: preamble names no list, README names the two constants, the code is the list. Card writers skip the preamble but their shell output is still trimmed and tailed. Exactly at the limit: trim applies to results "longer than" CAP (strictly over 4,000); preamble says "A result over 4,000 characters", agree.
Split: none in value. Note I9-148's comment says shapes come "from gates/README.md 'Running the tests' or Hydra's docs/development.md"; README:25 says the lists are in the code; both homes point at each other (a pointer loop with no value stated in the test cases' source).
Test holding homes equal: gates/tests/test_docs_numbers.py:198-202 `test_preamble_quotes_the_output_cap` holds "4,000" in the preamble's "Shells on Windows" section equal to shell_output.CAP (it looks in the Shells on Windows section; the figure sits at preamble:35 inside that section). Nothing holds the 30,000, the 1,500, or the command list.
I9-149: `delete_rule.word` is read by shell_output (`_read_pieces`), bash_windows_rule and media_rule (`dr._word`); owner delete_rule.py:66-72; shell_output.py:56-57 says "delete_rule owns the cut and the word rule". delete_rule.py:75-77 `pieces` "the one reader of where a command is cut", yet media_rule._segments (:18-27) and bash_windows_rule (:19-20 OPERATOR_CHARS) each cut commands again. K6.

## Q5. What counts as a status call, and what does the budget refuse? (I9-087, I9-088, I9-091)
Homes:
- Owner: gates/progress.py:13-16 numbers; :22 STATUS_PREFIX; :26 the whole shape; :119 exempt = report tools or `command.startswith("git ")`; :129 CALL_STOP check.
- Hydra brief-preamble.md:13-21,73 (5 and 10 minutes, 100/150 calls, 20/30 minutes, the exact call shape), CLAUDE.md rule 7, agent-rules.md:13, doc-review.md:9, derive-once-review.md:9 (status form), tools/effort_test_scorer_brief.md:1; C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:100 (card writers' own copy of the call, with `<your agent id>` bare); gates/settings.fragment.json:4 (the allow rule spells the helper path); memory every-brief-carries-status-rule.md:15 (says the form is owned by the preamble); the progress hook's own first-call message (progress.py:88-90).
- test_docs_numbers.py:110-127,182-190 holds call counts, "every N minutes", and STATUS_PREFIX in the docs equal to progress.py.
Kinds: K1 (numbers and helper path in many docs), K5 (preamble spells the hook rule), K7 not found.
Table:
| home | agent | main session | limit |
| progress.py | counts, warns at call 100 / minute 20, refuses from call 150 / minute 30 (`record["calls"] >= CALL_STOP` after incrementing, so the 150th call is the first refused) | not run (README:11: progress only in a subagent) | git commands starting `git ` and the report tools pass |
| preamble:73 | "At 150 tool calls or 30 minutes it stops you" | silent | |
| writer-brief.md:100 | card writer given its own status call text | | skips the preamble |
Edge split: at exactly 150 the docs read as "150 calls are allowed then stops"; the code refuses the 150th call itself (149 pass). Docs wording is ambiguous; I could not find a Confirmed ruling on the edge. Agent-rules.md:13 says "150 tool calls is a hard stop".
I9-091 (percall crash): README:13/:97 plus percall.py:127-139,170 (a main-session crash says "did nothing", an agent's denies) and prompt.py:18, common.py:53-70; no test compares README to code.

## Q6. Which gh flags may not carry text, and who may post? (I9-106)
Homes: gates/publish.ps1:18-38 (`$TextLetters = 'bFnfwetT'`, `$ValueLetters = 'aBHlmrR'`, the long-flag regex), comment "Letters from `gh <cmd> --help`"; gates/settings.fragment.json:8-57 deny list of gh commands (pr create, release create/edit, issue create/comment, pr comment, api, in each of 4-5 spellings); Hydra CLAUDE.md:53, docs/agents/doc-review.md:19,102, issue-tracker.md:5,9,12,42,47, this-machine.md:7,15 (posting only through publish.ps1).
Kinds: K1 (list of posting kinds in publish.ps1 line 8/56-62, the fragment deny list and publish_gate), K3/K8 below.
Split 1: docs/agents/issue-tracker.md:5 "the settings refuse the posting `gh` commands typed by hand", yet issue-tracker.md:14 tells a session to run `gh issue close <number> --comment "..."` and :13,:42,:46 `gh issue edit`, :26 `gh pr comment` and `gh pr edit`; the fragment denies `gh pr comment` (:42) but not `gh issue close --comment`, `gh issue edit` or `gh pr edit`. `gh issue close --comment` posts text outside publish.ps1. CLAUDE.md "Doc review" says "Text goes to GitHub only through publish.ps1".
Split 2 (inside issue-tracker.md): :26 says to use `gh pr comment`, which the fragment denies (hand-typed).
Test: none holds the deny list equal to publish.ps1's kinds (test_publish.py pins publish.ps1 only).

## Q7. Wake crash, ruling check command, harness (I9-124, I9-145, I9-177)
- Wake: gates/wake.py:69-114 and gates/README.md:31 ("A crash wakes the session once with the error ... the same crash again exits 1"). K1, values agree, no test links.
- Ruling check command: owner gates/ruling_rules.py:108 `CHECK_COMMAND`. Copies: gates/settings.fragment.json:6 allow rule; Hydra docs/agents/rulings/README.md:40 ("`ruling_rules.py --check <file>` (in the hooks repo's `gates` folder)" says the shape in prose). Test holding equal: gates/tests/test_docs_numbers.py:137-140 (fragment allows `CHECK_COMMAND`). The docstring in test_ruling_rules.py gives its own run command.
- I9-177 harness: gates/tests/lib/harness.ps1 and cleanup_sweep.ps1 and test_h52_cleanup_sweep.ps1 mention TimeoutSec/HOOK TIMED OUT; no docs home found (searched gates/ md/py/ps1 only; not searched thoroughly).

## Q8. I9-070 media guard (partly done)
Owner gates/media_rule.py; MEDIA_ROOT `C:\ytdl` at media_rule.py:6 ("D189"); README:21 names MEDIA_ROOT in prose without the value. Hydra docs/agents/* and CLAUDE.md have no "ytdl" (grep). The command cut here (`_segments` :18-27) is a second cutter beside delete_rule.pieces (see Q4). K6.

## Q9. Replay copies (I9-180, I9-181, I9-183) (not reached in depth)
Seen only: replay.py:88 MAIN_ASK equals fragment ask today (K1, see Q2). Nothing else checked.

## Q10. Exit codes, extension lists, git pins in the check tools (I10-003, I10-008, I10-013, I10-016, I10-023, I10-027, I10-028)
- I10-003 `HOOK_EXTENSIONS = (".py", ".ps1", ".sh")` (tools/check_pointers.py:32) and I10-027 `KNOWN_EXTENSIONS` (tools/doc_tokens.py:30-35) share the same words but different lists; no one cross-checks. Other home of "which extension makes a word a path": Hydra docs/agents/handoff.md:59 ("A token with a path separator or a known extension is a path"), a pointer. No other code home found in tools/*.py|ps1 (grep for extension runs).
- I10-008 `HOOKS_REPO_WORDS = "hooks repo"` (check_pointers.py:40): a line sends a `git show` to the hooks repo only if it contains these words. The phrase is used in docs: docs/agents/premise-map.md:41, handoff.md:47,59, agent-rules.md:33. check_handoff.py has no such word (grep: no "hooks"); handoff.md:59 says "A line that names the hooks repo's root folder is also checked in that repo" for check_handoff, which has no matching code in check_handoff.py I could find (my grep for "hooks|HOOKS|.claude" returned nothing). Candidate K8 (a doc says the checker does this; the checker file does not contain it; doc_tokens.py or the repo class may; not traced).
- I10-013: exit codes: check_pointers.py header (owner), gates/README.md:71 "it always exits 0, so its registration has no `onFailure`" with settings.fragment.json:101-104 timeout 15; test_docs_numbers does not check. K1.
- I10-016 git pins: Hydra tools/check_handoff.py:25-26 `_GIT_PINS`, `_GIT_ENV_DROP` (check_pointers.py:28 says "its git() and Repo are the one home of the git calls"); copies in gates/review_rules.py:61-62 (same pins and same env drop), gates/ref_judge.py:34-35 (same pins plus `trailer.separators=:`), Hydra tools/effort_test_faults.py:97 (quotePath only). K6 across two repos; ref_judge's list differs by one pin (K2 candidate). No test holds them equal.
- I10-023: check_handoff exit codes 0/1/2: Hydra docs/agents/handoff.md:57,59 repeat them; brief-preamble.md:97 points at the doc. K1.
- I10-028 `BRANCH_PREFIXES = ("claude/", "hx-")` (doc_tokens.py:37). Other homes of "what a branch name looks like": tools/new_worktree.ps1:4,7 (`claude/<...>`), brief-preamble.md:15/doc-review.md:9/derive-once-review.md:9 (path text `claude/g...`), handoff.md:3. "hx-" appears nowhere else in my greps (CLAUDE.md, docs/agents, tools). K1 for `claude/`.

## Q11. Build pass and slot (I10-047, I10-049, I10-065)
- Gate condition: tools/build_slot.ps1:90 `Test-BuildPassGated { $env:CLAUDECODE -eq '1' }` (owner); Directory.Build.targets:9 `Condition="'$(CLAUDECODE)' == '1'"` (second copy of the condition, comment says "tools\build_slot.ps1 owns the rule; this file only asks it"); docs/development.md:143 "The limit applies only when `CLAUDECODE` is 1". K6/K1.
- Check mode text: build_slot.ps1:324-328 "no claimed build pass for this build."; Directory.Build.targets:13 repeats the words "This build has no claimed build pass; see the line above". K1 wording.
- Tool name `hydra_build_pass`: tools/hydra_ui_mcp.py:22-25,874 (defines it, "It lives here because a hook can then refuse that one tool to agents by its name"); gates/percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`; gates/README.md:15; CLAUDE.md rule 14; development.md:120. The string with the server prefix is a copy; no test I found compares it to the server's registration name. K6.
- I10-047 -DryRun: only build_slot.ps1 (:329-330) and its test; no doc home found (docs and CLAUDE.md searched for "DryRun").

## Q12. ui MCP and doc_shots text (I10-067, I10-068, I10-080, I10-082)
- I10-068 "stopped at line N of M": hydra_ui_mcp.py:689-710 and docs/agents/ui-testing.md:229 (states it) and tools/test_hydra_ui_mcp.py:773,949,956. K1.
- I10-067 "one command is one line": hydra_ui_mcp.py:629-632, test_hydra_ui_mcp.py:228. ui-testing.md: not found.
- I10-080: doc_shots.ps1:14 "ask the main session to build the hydra_uitest target" restates rule 14 (K5) and the exe path `build-cpp\Release\hydra_uitest.exe` (:8) matches no other home I looked at. Not traced further.
- I10-082: fake_hydra_live.py commands; docs/agents/ui-testing.md:288,336 mention the stand-in. Caps not compared.

## Q13. Clone Hero tool values and precheck defaults (I10b-010, I10b-075, I10b-104)
- I10b-010 default player `Drums,Expert`: tools/ch_mcp/bot_run.py:12 (usage text) and :63 (constant); docs/agents/clone-hero-mcp.md:45 (mentions it); docs/clone-hero/07-internals.md:699,815 (the game's own player string, a different question). K1 (usage text and constant in one file, plus doc).
- I10b-075 precheck default range `main...HEAD`: tools/precheck/main.cpp:10,41 only in my greps; docs/development.md:598 names --code-mask only. gates README:17 and gates/shell_output.py:22 name `hydra_precheck` as a command word (K1 of "an agent may run it", with the preamble:47 and CLAUDE.md rule 14 stating "Agents may run `hydra_precheck.exe` from main").
- I10b-104 `MAX_COMMAND_LINE = 32767` (tools/mutate/run.py:79): single home in tools/docs/CMake searched (grep for 32767/8191); also gates/bash_windows_rule.py MAX_CHARS 1000 is an unrelated limit about a different question (what the Bash hook refuses).

## Side notes
- gates/README.md:19 and gates/README.md:27 each quote the user ("losing battle...", "Exempt git commit") as rulings; neither is under `## Confirmed` in the rulings folder by my grep ("Exempt git commit" is only at review-and-hooks.md:40, under Unconfirmed).
- bash_windows_rule.py:178 and probe_results.md:96 both say "by the user's ruling" for double-quote passing with no ruling id.
- gates/tests/probe_results.md and docs cite plan files (docs/superpowers/plans/...) that other lines mark removed; not traced.
