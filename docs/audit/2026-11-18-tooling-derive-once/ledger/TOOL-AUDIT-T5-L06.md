# TOOL-AUDIT-T5-L06 ledger (finders F16, F17, F18; compiled from the F/V reports and their re-sends only)

Paths: gates = C:\Users\Patrick\.claude\gates. Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test. Report sources: TOOL-AUDIT-T5-F16/V16/F16b/V16b, F17/V17/F17b/V17b, F18/V18/F18b/V18b. All claims are "by reading"; no verifier ran anything. Where a verifier and its finder disagree, the verifier's reading is used and said so. Each split sits on its own line starting "SPLIT:". Cross-finder overlaps (the allow phrase, the sign-off line, the marker files) have a row under each finder because the finders are different.

Rulings under `## Confirmed` that the reports found (quoted by the reports; none of them names both sides of any split below unless a row says so):
- Hydra docs/agents/rulings/review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too".
- review-and-hooks.md:16 (and tests-and-runs.md:14) "i want to be able to give explicit approval for a full library run."
- review-and-hooks.md:17 chose "Keep rule, move worktrees (Recommended)".
- review-and-hooks.md:19 chose "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session."
- review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done."
- review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire..."
- review-and-hooks.md:24 "Agents have a soft cap of 20 minutes ... hard cap of 30 minutes real time."
- review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues."
- agents-and-workflow.md:8-9 "agents are never allowed to start their own builds, only the main session..." and "any tests that require a build are done by the main session, not agents."
- agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable".
- tests-and-runs.md:15, :21 and questions-and-audits.md:18-19 (quoted in rows below where they matter).
Not Confirmed (the reports say so): review-and-hooks.md:39 "alright approve b" and :40-43 "Exempt git commit", "length over 1000 and backslashes", all under `## Unconfirmed` (heading at :37).

---------------------------------------------------------------- F16 rows

### L-F16-01: Is there one reader of quote characters in a shell command, and is a quoted first word a command?
- **Source:** I9-060, I9-189; F16 Q1, V16 Q1.
- **Kind:** K6, K8 (weak), K1/K5 (for I9-189).
- **Verdict:** CONFIRMED-CHANGED. K6 is real. The verifier found the K8 overstated: the "one reader" claim is scoped to bash_windows_rule's five parts. The verifier also added a fourth reader.
- **Homes:** gates/bash_windows_rule.py:5-6 "_scan is the one reader of shell quoting: every part works from what it returns."; gates/README.md:27 "One scanner, `_scan`, reads quoting for all five parts, so nobody writes a second one." Other readers: gates/media_rule.py:15 `dr._word(token.strip("\"'"))`, :31 counts quote characters (`odd`), :51 strips a matching pair; gates/delete_rule.py:90 `_inside` strips a matching pair, NOT_LITERAL :26 lists `"` and `'`; gates/shell_output.py:45-52 `_unquoted` (:46-47 "because that rule leaves a quoted delete unread on purpose"); gates/bash_windows_rule.py:141 `piece[at].raw != piece[at].value` then delete_rule._word (verifier's addition). For I9-189: gates/tests/probe_results.md:85-102, bash_windows_rule.py:216,226, Hydra docs/agents/brief-preamble.md:33 (five shapes in prose).
- **Splits:**
  - SPLIT: a first word `"cmake.exe"` (quoted). delete_rule (delete_rule.py:3-5, README:19) leaves a quoted delete unread; shell_output._unquoted reads it as `cmake` for the exit-code tail; media_rule._word reads `"ffmpeg.exe"` as ffmpeg; bash_windows_rule.py:141 treats a quoted word as not the name. Four answers (finder found three, verifier four). test_shell_output.py:99-100 pins the delete_rule side.
- **Ruling quotes:** None found naming quote handling. bash_windows_rule.py:178 and gates/tests/probe_results.md:96 both say a pair in double quotes passes "by the user's ruling" with no ruling id; no matching Confirmed line was found (verifier grepped only review-and-hooks.md and agents-and-workflow.md).
- **Tests:** test_bash_windows_rule.py:313-327 holds it only inside that file. test_docs_numbers.py:192-196 holds the preamble's 1,000 equal to MAX_CHARS; nothing holds the other four shapes in the preamble.
- **Open:** whether the four readers differ on real commands is a run question.
- **Hook gap:** none stated.

### L-F16-02: Who decides where a shell command is cut into pieces?
- **Source:** I9-149, I9-070, I9-076 (Q4 tail), F16 Q1/Q4/Q8, V16 Q1/Q4, V16b I9-070.
- **Kind:** K6, K8 (delete_rule's "the one reader" claim).
- **Verdict:** CONFIRMED.
- **Homes:** gates/delete_rule.py:75-77 `pieces` "the one reader of where a command is cut" (cuts at OPERATORS anywhere in the string; delete_rule.py:66-72 owns `word`); gates/media_rule.py:18-27 `_segments` (own loop, cuts only where an operator stands alone as a word, turns newlines into " ; "; its comment says an operator inside a quoted filter "a;b" must not split); gates/bash_windows_rule.py:19-20 OPERATOR_CHARS (quote-aware, a third cut); shell_output.py:56-57 "delete_rule owns the cut and the word rule" and uses delete_rule.pieces (:59-60, git_words :132). MEDIA_ROOT `C:\ytdl` is at media_rule.py:6 ("D189"), gates/README.md:21 names it without the value; no Hydra doc states "ytdl" (grep).
- **Splits:**
  - SPLIT: `a;b` with no spaces. delete_rule and shell_output cut inside the word; media_rule does not; bash_windows_rule cuts quote-aware. A missed write is possible for `ffmpeg -i x -vf "a;b" out.mp4` (verifier's example; not run).
- **Ruling quotes:** None found.
- **Tests:** none holding the three cutters equal.
- **Open:** a run would be needed to show the missed write.
- **Hook gap:** none stated.

### L-F16-03: What is a subagent refused when deleting or discarding, which roots are allowed, and what is the main session asked?
- **Source:** I9-062, I9-063, I9-064, I9-065, I9-067 (partly), I9-180; F16 Q2/Q9, V16 Q2, F16b I9-180, V16b I9-180.
- **Kind:** K1, K3, K6.
- **Verdict:** CONFIRMED.
- **Homes:** gates/delete_rule.py:17-35 WORKTREE_FOLDERS, DELETE_WORDS, GIT_DISCARD_WORDS, GIT_DISCARD_ACTIONS; :58-63 the Instead text; gates/README.md:19 "`WORKTREE_FOLDERS`, `GIT_DISCARD_WORDS` and `GIT_DISCARD_ACTIONS` in that file are the one list" (long list incl. `git switch --force`, `git checkout .`, `git restore .`, `git stash drop`/`clear`, `git worktree remove`/`prune`, `git branch -D`, `git branch -d --force`); README:136; Hydra docs/agents/brief-preamble.md:43 "such as `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`"; Hydra docs/agents/derive-once-review.md:197 (both worktree paths again, finder only); gates/settings.fragment.json:59-64 `ask`: `Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)` (live settings.json:70-73 same four); gates/tests/replay.py:87-88 MAIN_ASK `{"Bash": ("rm ", "git clean", "git reset --hard"), "PowerShell": ("remove-item ",)}`; Hydra tools/new_worktree.ps1:6,29 cuts worktrees at `<main>\.claude\worktrees\<TaskId>`; delete_rule.py:100 `len(parts) > 1` keeps a worktree's own folder out.
- **Splits:**
  - SPLIT: the main session is only asked for four shapes (fragment ask list, replay MAIN_ASK) and is not refused or asked for `git worktree prune`, `git stash drop`, `git checkout -f`, `git branch -D`, `git restore .`; an agent is refused all of them. README:19 and preamble:43 describe only the agent side. Confirmed by the verifier.
  - SPLIT (docs): preamble:43 gives a short "such as" list; README:19 gives the long list and calls its three constants "the one list".
  - Edge (not a split): replay matches `git clean` bare and the rule matches `git clean *`; `git cleanfoo` would match replay only.
- **Ruling quotes:** review-and-hooks.md:17 "Keep rule, move worktrees (Recommended)" speaks to where worktrees sit; it does not speak to the git list or main-session asking.
- **Tests:** none holds README:19, preamble:43, the fragment ask list or replay.MAIN_ASK equal to delete_rule. test_docs_numbers.py `test_live_settings_carry_the_fragment` (~:142) and test_sync_settings.py hold fragment and live equal (finder, V16b did not reopen). 
- **Open:** side note, confirmed by the verifier: the verifier's own cwd `...\hydra-test\worktrees\tooling-audit-records` is under neither WORKTREE_FOLDERS entry (delete_rule.py:17-18, :97-102), so an agent cut there could not run a recursive delete in it; the ruling at :17 chose the `.claude\worktrees` shape. Also delete_rule's discard set vs replay's `startswith` prefix reader are two readers (K6), no split today.
- **Hook gap:** none for this row.

### L-F16-04: Which Clone Hero server calls count as pressing keys?
- **Source:** I9-073, I9-095; F16 Q3, V16 Q3.
- **Kind:** K6, K1, K5.
- **Verdict:** CONFIRMED.
- **Homes:** gates/percall.py:15-18 `KEY_TOOLS`, `KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}`, `_key_call` :77-81; gates/README.md:13 (same set in prose, "`_key_call` in `percall.py` owns which calls count"); gates/review_rules.py:54 `KEY_ALLOW_TEXT = "allow key run"`; Hydra tools/ch_mcp/server.py:147-148, :466, :477; Hydra docs/agents/clone-hero-mcp.md:29; Hydra tools/ch_mcp/game_tools.py:70-72 `ROUTE_KIT = "kit"`, `ROUTE_KEYS = "keys"`, `ROUTES` (comment cites "the user's answer to the plan's question 6, 2026-10-30, 'Lock it too'"); game_tools.py:1097-1108 treats route None as the kit once set up.
- **Splits:** None found in values. They agree today.
- **Ruling quotes:** None found: no `## Confirmed` line for the allow-key rule or "Lock it too" (grep of rulings).
- **Tests:** test_percall_key.py and test_progress.py:611-696 pin the percall side; nothing compares percall's "keys" to game_tools.ROUTE_KEYS (verifier grepped ch_mcp for percall: only a comment at front.py:493).
- **Open:** none.
- **Hook gap:** none stated.

### L-F16-05: When does a build or test command get the exit-code line, and what is trimmed?
- **Source:** I9-076, I9-147, I9-148, I9-149 (partly), I9-152, I9-153; F16 Q4, V16 Q4.
- **Kind:** K1, K5, K6.
- **Verdict:** CONFIRMED.
- **Homes:** gates/shell_output.py:14 "The one home of these numbers and lists... Docs point here." with CAP 4000 :15, HEAD/TAIL 1500 :16, BUILD_TEST_COMMANDS :21, PYTHON_TEST_FLAGS :24, NO_TRIM_COMMANDS :25, TAILS :35-38; Hydra docs/agents/brief-preamble.md:35; gates/README.md:23-25; gates/tests/probe_results.md:133, :206-212; gates/tests/shell_output_sizes.py:18-19; gates/tests/replay_extract.py:28 `REFUSAL_TEXT_CAP = 4000`; test copies of the tail text in test_percall_rewrite.py `BASH_TAIL` and test_shell_output.py:16-18.
- **Splits:** None in value. Finder's pointer loop (I9-148 comment points at README "Running the tests" or docs/development.md; README:25 says the lists are in the code) was not reopened by the verifier.
- **Ruling quotes:** None found.
- **Tests:** gates/tests/test_docs_numbers.py:198-202 holds "4,000" equal to shell_output.CAP. Nothing holds 30,000, 1,500 or the command lists.
- **Open:** none.
- **Hook gap:** none.

### L-F16-06: What counts as a status call, and where does the call budget stop?
- **Source:** I9-087, I9-088, I9-091; F16 Q5, V16 Q5.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED (edge split confirmed).
- **Homes:** gates/progress.py:13-16 numbers (`CALL_WARN, CALL_STOP = 100, 150`), :22 STATUS_PREFIX, :26 shape, :91 increments before the check, :119 exempt = report tools or `command.startswith("git ")`, :129 refuses at `record["calls"] >= CALL_STOP`; Hydra docs/agents/brief-preamble.md:13-21,:71,:73 ("At 150 tool calls or 30 minutes it stops you"; "Commit before your 100th tool call"); CLAUDE.md rule 7; agent-rules.md:13 ("150 tool calls is a hard stop"); doc-review.md:9; derive-once-review.md:9; tools/effort_test_scorer_brief.md:1; hooks tools/session_cards/writer-brief.md:100 (card writers' own copy of the call); gates/settings.fragment.json:4; memory every-brief-carries-status-rule.md:15.
- **Splits:**
  - SPLIT (edge): docs read "at 150 calls it stops you"; code refuses the 150th call itself (149 pass); call 100 gets a warning and still runs. Docs wording ambiguous, not contradictory.
- **Ruling quotes:** None found for the edge. review-and-hooks.md:24 covers the 20/30 minute caps.
- **Tests:** test_docs_numbers.py:110-127, :182-190 hold call counts, "every N minutes" and STATUS_PREFIX in the docs equal to progress.py. writer-brief copy not reopened.
- **Open:** I9-091 crash note (README:13/:97, percall.py:127-139,:170, prompt.py:18, common.py:53-70) was confirmed only as part of row 12.
- **Hook gap:** hook gap: yes, gates/progress.py (an exemption: a command starting with `git ` is exempt from the spent-budget refusal, checked before the helper-name and budget checks).

### L-F16-07: Do progress.py and the replay tool read "is this a status call" the same way?
- **Source:** I9-183 (also I9-181 context); F16b I9-183, V16b I9-183.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED.
- **Homes:** gates/progress.py:22 STATUS_PREFIX `& "C:/Users/Patrick/.claude/gates/status_append.ps1"`, :41 `is_status_call` (owner reader); gates/tests/replay.py:279-280 `_status_call` regex `(hooks|gates)/status_append\.ps1` (second reader; replay.py:221 also calls progress.is_status_call); settings.fragment.json:4 and live settings.json:10 (the allow rule); replay.py:290-291 comment recording a bug: percall runs delete_rule on a status-helper call although progress.py says a status line is never refused (comment only read; finder).
- **Splits:**
  - SPLIT: replay accepts a `hooks` or `gates` folder name in the status path; progress.py:22 accepts only `gates`. The verifier notes replay reads recorded calls so `hooks` may cover old recordings; that is a claim, not read.
- **Ruling quotes:** None found.
- **Tests:** none holds these equal; no test imports replay.py (finder grep).
- **Open:** whether the recorded delete-word bug is fixed was not checked. Live settings.json:10 not reopened by the verifier.
- **Hook gap:** none stated.

### L-F16-08: Which `gh` commands may carry text, and who may post?
- **Source:** I9-106; F16 Q6, V16 Q6.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED.
- **Homes:** gates/publish.ps1:8 kinds (pr, release, release-edit, issue, issue-comment, pr-comment), :18-38 `$TextLetters`/`$ValueLetters`; gates/settings.fragment.json:8-57 deny list (pr create, release create, release edit, issue create, issue comment, pr comment, api; several spellings each; it does not deny `gh issue close`, `gh issue edit`, `gh pr edit`, `gh pr close`); Hydra CLAUDE.md "Doc review" ("Text goes to GitHub only through publish.ps1"); docs/agents/doc-review.md:19,:102; docs/agents/issue-tracker.md:5 "the settings refuse the posting `gh` commands typed by hand", :14 `gh issue close <number> --comment "..."`, :13,:42,:46 `gh issue edit`, :26 `gh pr comment` and `gh pr edit`.
- **Splits:**
  - SPLIT (docs against settings): issue-tracker.md:14 tells a session to post text with `gh issue close --comment`, outside publish.ps1, and no deny rule covers it (verifier grepped gates *.py/*.ps1/*.json/*.md for issue close|issue edit|pr edit|--comment).
  - SPLIT (docs against settings): issue-tracker.md:26 tells a session to run `gh pr comment`, which the fragment denies when typed by hand; publish.ps1 has a `pr-comment` kind.
- **Ruling quotes:** None found naming both sides.
- **Tests:** none holds the deny list equal to publish.ps1's kinds (test_publish.py pins publish.ps1 only; not opened by the verifier).
- **Open:** none.
- **Hook gap:** hook gap: yes, gates/settings.fragment.json (deny list does not cover every text-carrying `gh` command).

### L-F16-09: Is the ruling-check command written once?
- **Source:** I9-145; F16 Q7, V16 Q7.
- **Kind:** K1.
- **Verdict:** CONFIRMED (a held copy).
- **Homes:** gates/ruling_rules.py:108 `CHECK_COMMAND` (owner); gates/settings.fragment.json:6 allow rule; Hydra docs/agents/rulings/README.md:40 ("`ruling_rules.py --check <file>` (in the hooks repo's `gates` folder)", prose).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** gates/tests/test_docs_numbers.py:137-140 (fragment allows `PowerShell(%s *)` % CHECK_COMMAND).
- **Open:** none.
- **Hook gap:** none.

### L-F16-10: What does a hook crash in the wake hook do, and how long may a test harness wait for a hook?
- **Source:** I9-124, I9-177; F16 Q7, F16b I9-177.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED. V16 said wake not opened; V16b listed I9-177 as not reopened.
- **Homes:** (finder only) gates/wake.py:69-114 and gates/README.md:31 ("A crash wakes the session once with the error ... the same crash again exits 1"), values agree. Harness: gates/tests/lib/harness.ps1:20-23,:50-113 (default -TimeoutSec 120, HarnessDrainMs 5000); callers gates/tests/test_h52_cleanup_sweep.ps1:283 (-TimeoutSec 60), :409/:463 (120), :411 `WaitForExit(120000)`; test_h53_session_cards_start.ps1:50 `time.sleep(120)`; live hook timeouts in settings.fragment.json lines 81,93,104,119,135,152,168,184,202,219,235 (a different question: what Claude Code enforces).
- **Splits:** None found by the finder.
- **Ruling quotes:** None found.
- **Tests:** none; test_h39_harness.ps1 only proves failure paths.
- **Open:** no doc states the harness defaults.
- **Hook gap:** none.

### L-F16-11: Do the replay tool's old refusal texts and list-3 labels still match the live refusal texts?
- **Source:** I9-181, I9-183 (labels); F16 Q9, F16b I9-181/I9-183, V16b (I9-181 not reopened).
- **Kind:** K1, K3 (mild).
- **Verdict:** NOT VERIFIED for I9-181 (V16b "not reopened"). The I9-183 status-helper part is row 07.
- **Homes:** (finder only) gates/tests/replay.py:44-85,:118 regexes (comment :44-45 "Class obj1/obj2 are the objectives the new set must still enforce (list 1); obj3 is the budget, kept by progress.py"); replay.py:283-344 `tightening()` substrings each matching a live text: progress.py:125, percall.py:52 "A run over the whole song library needs the user's permission", delete_rule.py:157, :148, review_dispatch.py:158/:113/:107, publish_gate.py:103-104; replay.py regex `Budget of \d+ tool calls reached` is the old wording vs live progress.py:130 "Your budget is spent (%d calls of %d ...)".
- **Splits:** None found.
- **Ruling quotes:** None found.
- **Tests:** none holds replay's strings equal to the hook texts.
- **Open:** the objectives' owner (plan 2026-10-25-hooks-from-scratch.md) was not read.
- **Hook gap:** none.

### L-F16-12: Does a crash in percall and in the other gates fail closed, and is that mode written once?
- **Source:** I9-067, I9-091; F16 Q2, V16 Q2.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (the two "closed" words agree).
- **Homes:** gates/delete_rule.py:187 `common.run(body, "closed")`; gates/percall.py:176 "closed"; gates/prompt.py:18 `("percall", "closed")`; gates/common.py:53-70 defines the mode; gates/README.md:97 "A crash denies the call".
- **Splits:** None. Whether prompt.py's mode matters when percall runs as a prompt part is a run question (see L-F17-13).
- **Ruling quotes:** None found.
- **Tests:** none compares README to code.
- **Open:** verifier did not read prompt.py's use of the mode.
- **Hook gap:** none.

### L-F16-13: Does check_handoff check lines naming the hooks repo, as handoff.md says?
- **Source:** I10-008; F16 Q10, V16 Q10.
- **Kind:** K8 (finder's candidate), K6 (verifier's replacement).
- **Verdict:** REFUTED for the K8 claim. check_handoff.py:136-153 `other_repos` finds absolute paths on each line via doc_tokens.absolute_paths and checks lines that name another repo's root folder in that repo, so handoff.md:59 "A line that names the hooks repo's root folder is also checked in that repo" is true; the finder's grep for "hooks" missed it.
- **Homes:** Hydra tools/check_pointers.py:40,:166 `HOOKS_REPO_WORDS = "hooks repo"` (check_pointers picks the hooks repo by those words); tools/check_handoff.py:136-153 (picks by a named root folder); docs: premise-map.md:41, handoff.md:47,:59, agent-rules.md:33.
- **Splits:**
  - SPLIT (verifier's reading, not tested): check_handoff picks the hooks repo by a named root folder; check_pointers by the words "hooks repo". Two ways for one question; not tested by the verifier for what each prints.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** what each tool prints for the same line.
- **Hook gap:** none.

### L-F16-14: Are the git settings pinned the same everywhere git is called?
- **Source:** I10-016; F16 Q10, V16 Q10.
- **Kind:** K6, K2.
- **Verdict:** CONFIRMED.
- **Homes:** Hydra tools/check_handoff.py:25-26 `_GIT_PINS` (quotePath, showSignature, useReplaceRefs), `_GIT_ENV_DROP`; gates/review_rules.py:61-62 (same pins and env drop); gates/ref_judge.py:34-35 `PINNED` (adds `trailer.separators=:`); Hydra tools/effort_test_faults.py:97 (quotePath only); tools/check_pointers.py:28 says "its git() and Repo are the one home of the git calls".
- **Splits:**
  - SPLIT: ref_judge pins one more setting (`trailer.separators=:`) than check_handoff and review_rules; effort_test_faults pins only quotePath. Env drop lists: check_handoff.py:26 and review_rules.py:62 same words (check_handoff's second line not opened).
- **Ruling quotes:** None found.
- **Tests:** none holds them equal (grep, not exhaustive).
- **Open:** none.
- **Hook gap:** none.

### L-F16-15: Check-tool constants not reopened by the verifier (extension lists, exit codes, branch prefixes)
- **Source:** I10-003, I10-013, I10-023, I10-027, I10-028; F16 Q10, V16 Q10 ("NOT RE-CHECKED").
- **Kind:** K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) tools/check_pointers.py:32 `HOOK_EXTENSIONS = (".py", ".ps1", ".sh")` vs tools/doc_tokens.py:30-35 `KNOWN_EXTENSIONS` (same words, different lists) and docs/agents/handoff.md:59 "A token with a path separator or a known extension is a path"; check_pointers.py header exit codes and gates/README.md:71 ("it always exits 0, so its registration has no `onFailure`") with settings.fragment.json:101-104 timeout 15; check_handoff exit codes 0/1/2 repeated in handoff.md:57,:59 and pointed at from brief-preamble.md:97; tools/doc_tokens.py:37 `BRANCH_PREFIXES = ("claude/", "hx-")` vs tools/new_worktree.ps1:4,7 and path text in several docs; "hx-" appears nowhere else.
- **Splits:** None found.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers does not check the exit-code claim.
- **Open:** all unreached by the verifier.
- **Hook gap:** none.

### L-F16-16: Where is the build-pass gate decided, and is the tool name written once?
- **Source:** I10-047, I10-049, I10-065; F16 Q11, V16 Q11.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED (the -DryRun and check-mode text rows were not rechecked: NOT VERIFIED for those).
- **Homes:** Hydra tools/build_slot.ps1:90 `Test-BuildPassGated { $env:CLAUDECODE -eq '1' }` (owner); Hydra Directory.Build.targets:9 `Condition="'$(CLAUDECODE)' == '1'"` (copy of the condition; comment "tools\build_slot.ps1 owns the rule; this file only asks it"); docs/development.md:143 "The limit applies only when `CLAUDECODE` is 1"; build_slot.ps1:324-328 "no claimed build pass for this build." vs Directory.Build.targets:13 "This build has no claimed build pass; see the line above"; gates/percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"`; Hydra tools/hydra_ui_mcp.py:22-25,:874 `def hydra_build_pass()`; gates/README.md:15; CLAUDE.md rule 14; development.md:120; -DryRun only at build_slot.ps1:329-330 and its test (finder).
- **Splits:** None today.
- **Ruling quotes:** agents-and-workflow.md:8-9 "agents are never allowed to start their own builds, only the main session..." names the rule, not the tool name.
- **Tests:** test_progress.py and test_percall_key.py mention the tool name; nothing compares it to the server's registered name.
- **Open:** the `mcp__hydra__` prefix depends on the MCP server's registered name (a run question).
- **Hook gap:** none.

### L-F16-17: ui MCP and doc_shots text rows not reopened by the verifier
- **Source:** I10-067, I10-068, I10-080; F16 Q12, F16b I10-080, V16 and V16b ("NOT REACHED"/"not reopened").
- **Kind:** K1, K5, K7 (mild).
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) "stopped at line N of M": Hydra tools/hydra_ui_mcp.py:689-710, docs/agents/ui-testing.md:229, tools/test_hydra_ui_mcp.py:773,949,956; "one command is one line": hydra_ui_mcp.py:629-632, test_hydra_ui_mcp.py:228 (ui-testing.md: not found); doc_shots.ps1:8 exe path `build-cpp\Release\hydra_uitest.exe` (also ui-testing.md:20,:24), :13-15 "ask the main session to build the hydra_uitest target" (restates rule 14, K5), rect-before-screenshot rule stated in tools/doc_shots/shots.txt top comment, docs/agents/doc-screenshots.md:15-25 and coded in doc_shots.ps1:31-34 (K7 mild).
- **Splits:** None found.
- **Ruling quotes:** None found.
- **Tests:** none holding equal.
- **Open:** all unreached by the verifier.
- **Hook gap:** none.

### L-F16-18: Is the stand-in's reply cap the same as the real tool's, and are its commands described once?
- **Source:** I10-082; F16 Q12, F16b I10-082, V16b K1 copies.
- **Kind:** K1, K2 (mild).
- **Verdict:** CONFIRMED for the 12,000 cap (agree today); the rest of the row is finder only.
- **Homes:** Hydra tools/hydra_ui_mcp.py:44 `REPLY_CAP_CHARS = 12_000`; Hydra tools/test_hydra_ui_mcp.py:288-296 (literal "[cut at 12,000 characters: {1000 - kept} of 1000 lines dropped;" at :293); tools/fake_hydra_live.py:9-17 docstring ("fake-big 50,000 characters") against `BIG_LINE = "x"*45+" row"` (about 49,000 in 1000 lines; a rounded copy, finder); fake_hydra_live.py:59 SESSION_TIMEOUT_S comment; hydra_ui_mcp.py:42-43 REPLY_MARGIN_S 15.0, HELLO_TIMEOUT_S 60.0; docs/agents/ui-testing.md:265,275,280, :288, :336 and docs/adr/0030 (finder only).
- **Splits:** None in behaviour.
- **Ruling quotes:** None found.
- **Tests:** test_hydra_ui_mcp.py:288-296 holds 12,000 and 1000 as literals; `assertLessEqual(len(out), hm.REPLY_CAP_CHARS)` reads the constant.
- **Open:** none.
- **Hook gap:** none.

### L-F16-19: Game folder and dump paths, build number, tool values in tools/ch_mcp
- **Source:** I10b-057 (verified), I10b-010, I10b-013, I10b-029, I10b-045 (finder only); F16b, V16b.
- **Kind:** K1, K4 (mild), K6 (mild), K7 (mild).
- **Verdict:** CONFIRMED for the game folder (I10b-057); NOT VERIFIED for I10b-010, -013, -029, -045 (V16b: "not reopened").
- **Homes:** I10b-057: owner Hydra tools/ch_mcp/game_files.py:31 `GAME_DIR = r"C:\Clone Hero\1.1 june\Clone Hero"`; copies tools/ch_mcp/README.md:12, :58 and tools/ch_mcp/dump_build.py:9 docstring (agree today); other finder-only copies: README :20/:27 `--version`, dump_facts.py:27, build_facts.py:28 DUMPS_ROOT, kit_methods.json:3, launch_patch.json:4, start_candidates.json:3; README's `--version` is typed by a person while build_facts.py:69 reads it from the game folder (K4 mild). I10b-010: tools/ch_mcp/bot_run.py:63 `DEFAULT_PLAYER = "Drums,Expert"`, copy in the module docstring :12 (finder). I10b-013: bot_run.py:110 HUMAN_NEEDS_SONG, game_tools.py:731-733, docs/agents/clone-hero-mcp.md:31,:45, server.py:421 (reason stated only at bot_run.py:110-112). I10b-029: tools/ch_mcp/frida_tools.py:56-61 (NO_GAME_MANAGER, TRACE_RUNNING, NOT_ATTACHED) imported by kit_tools.py:44,1072, method_watch.py:428; own-spelled siblings game_tools.py:79, method_watch.py:426, sp_tools.py:66, and the phrase "not attached" at frida_tools.py:97, frida_attach.py:439, kit_tools.py:1268. I10b-045: tools/ch_mcp/menu_steps.json ("verified": false), menu_tools.py:43,:112-124, docs/agents/clone-hero-mcp.md:54,:70, docs/clone-hero/07-internals.md:878,:886, tools/ch_probe/input_driver.py:92-105 MENU_KEYS (same wiki URL as menu_steps.json).
- **Splits:** None found.
- **Ruling quotes:** None found. clone-hero-mcp.md:70 cites "The user chose on 2026-10-31 (question 4 of the direct-start plan) to keep ch_open_song, menu_steps.json" (finder did not open that answer).
- **Tests:** none holding the copies equal.
- **Open:** none beyond the unreached rows.
- **Hook gap:** none.

### L-F16-20: Capture start wait: 10 s or 12 s?
- **Source:** I10b-067; F16b, V16b.
- **Kind:** K2, K1.
- **Verdict:** CONFIRMED.
- **Homes:** Hydra tools/capture_window.ps1:56 `function Start-Hydra([string]$Dir, [int]$Wait = 12)` (owner); tools/capture/shot.ps1:2 `[int]$Wait = 10`, :4 `Start-Hydra $Dir $Wait`; tools/capture/steps.ps1:8 and tools/capture/report_shot.ps1:10 `Start-Hydra $Dir 12`; docs/agents/ui-testing.md:352,:363 state no number.
- **Splits:**
  - SPLIT: shot.ps1 waits 10 s by default; Start-Hydra's own default and the two sibling scripts use 12 s. They differ when a slow-starting Hydra is launched through shot.ps1 with no -Wait.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** a capture on a slow machine (a run).
- **Hook gap:** none.

### L-F16-21: Where does the squeeze window live, and do the pointers to it match?
- **Source:** I10b-099; F16b, V16b.
- **Kind:** K1, K2, K8, K5.
- **Verdict:** CONFIRMED-CHANGED. Verifier: the skill is not in the Hydra repo (live copy C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10, worktree copies exist); the stale cite appears at three lines, not one.
- **Homes:** Hydra src/core/model.h:121 `constexpr double kSqueezeWindowMs = 500.0;`, `within_squeeze_window` :133 (graph.h:216 only mentions it in a comment); SKILL.md:10 "within 500 ms of a deactivation node (`kSqueezeWindowMs`, src/search/graph.h)"; Hydra docs/clone-hero/05-star-power.md cites "src/core/model.h line 111" (:417, :419) and "line 123" for within_squeeze_window (:399, :417); memory sp-squeeze-mechanics.md:3,13,17,23 (finder); tools/replay.cpp:97-135 usage text formats the constant (derived, finder).
- **Splits:**
  - SPLIT (K8): the skill says the constant lives in src/search/graph.h; it is defined in src/core/model.h:121.
  - SPLIT (K2): 05-star-power.md cites model.h line 111 and line 123; the code has them at 121 and 133.
- **Ruling quotes:** None found (verifier grepped rulings for squeeze).
- **Tests:** none holding the 500 equal.
- **Open:** finder's K5 (agent-rules/executor/reviewer say agents may not run hydra_replay; whether a hook refuses it was not checked; percall.py not read).
- **Hook gap:** none.

### L-F16-22: Other single-source rows not reopened (doc_shots start state, replay options, precheck defaults, longest command line)
- **Source:** I10b-068, I10b-073, I10b-075, I10b-104; F16b, V16b ("not reopened").
- **Kind:** K1, K5 (mild).
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) tools/capture/steps.ps1:3-7 step names and `$codes = @{ min = 6; restore = 9; max = 3 }` (one home, K1 inside one file); Hydra tools/doc_shots/shots.txt:12-20 (puts every panel setting back; expects "Analysis settings: defaults") copied in docs/agents/doc-screenshots.md:25 and in tests/ui/uitest_library.cpp:254,273,322, owner src/ui/library_dialogs.cpp:137; tools/precheck/main.cpp:8-12,:41 (`--range` defaults to `main...HEAD`) against docs/agents/derive-once-review.md:21,24 (`--range main...<key>`) and "agents may run hydra_precheck.exe" stated in five prose places; tools/mutate/run.py:77-79 `MAX_COMMAND_LINE = 32767` against gates/bash_windows_rule.py:16 `MAX_CHARS = 1000` (two different limits).
- **Splits:** None found.
- **Ruling quotes:** None found. review-and-hooks.md:43 "length over 1000 and backslashes" is Unconfirmed.
- **Tests:** none.
- **Open:** none beyond unreached.
- **Hook gap:** none.

### L-F16-23: Two quotes the gates README gives as the user's rulings, and pointer drift to them
- **Source:** side notes; F16 Q2/Side notes, V16 pointer drift.
- **Kind:** K8 (a rule with no home), K2.
- **Verdict:** CONFIRMED-CHANGED. Verifier: the quote is not Confirmed (card docs/sessions/2026-10-31-6777b546.md:25 and two plans only). Pointer drift: only 2026-11-07-ch-run-timing.md drifts; build-limit.md:130 cites the README without a line number, so the finder was wrong there.
- **Homes:** gates/README.md:19 quotes the user ("trying to ban specific command patterns is a losing battle...", 2026-10-31) and :27 quotes "Exempt git commit"; "Exempt git commit" is only at review-and-hooks.md:40 under Unconfirmed; Hydra docs/superpowers/plans/2026-11-07-ch-run-timing.md:31,:457 cite "README.md line 15" for the quote, now at README:19; bash_windows_rule.py:178 and probe_results.md:96 say a pair in double quotes passes by "the user's ruling" with no id.
- **Splits:** None.
- **Ruling quotes:** None found under `## Confirmed` for any of the three.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

---------------------------------------------------------------- F17 rows

### L-F17-01: What exact words let one whole-library run through, and what does the hook actually see?
- **Source:** I1-005, I3-032, I2-081, I2-082; F17 Q1, V17 candidates 1-3, 11.
- **Kind:** K1, K2, K5, K6, K7.
- **Verdict:** CONFIRMED (the Hydra batch_scope.h side was not opened).
- **Homes:** gates/review_rules.py:50-52 `LIBRARY_ALLOW_TEXT = "allow library run"`, `_LIBRARY_ALLOW = re.compile(re.escape(...) + r"[.!]?", re.IGNORECASE)`, `fullmatch(text.strip())` (:49-51 comment "a trailing . or ! is allowed"); gates/percall.py:45-55 refusal text ("sending exactly: allow library run"), :14 `LIBRARY_FLAG = "--whole-library"`, :47-48; gates/README.md:13, :37; Hydra docs/agents/brief-preamble.md:49 "unless the user has sent exactly `allow library run` in this session; each such message lets one command through"; docs/agents/agent-rules.md:14 "sending exactly `allow library run`"; audit-method.md:125; this-machine.md:91; rulings/review-and-hooks.md:47 (Unconfirmed) "The user kept the typed allow library run for whole-library runs."; CLAUDE.md rule 8 (no phrase); model-postures.md:13; Hydra src/cli/batch_scope.h:10 `kWholeLibraryFlag = "--whole-library"` (finder only), src/cli/batch.cpp:3, tools/compare_db.py:29.
- **Splits:**
  - SPLIT: docs say "exactly `allow library run`"; the hook accepts any case, surrounding space, and one trailing `.` or `!` (`Allow Library Run!`).
  - SPLIT: rule 8 / agent-rules.md:14 cover any whole-library compare or timing run; percall.py:14,:47-48 judges only a shell command that holds `--whole-library`.
  - Two spellings of one flag in two repos (percall.py:14 vs batch_scope.h:10); nothing ties them (finder; the verifier did not open batch_scope.h).
- **Ruling quotes:** review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." Does not name the phrase or its case rules; names neither side of either split.
- **Tests:** test_review_rules.py:839-866, test_progress.py:427-455, test_review_dispatch.py:309-336 test the code; nothing compares the docs' words with the code.
- **Open:** whether a compare run without the flag in its command text exists in practice.
- **Hook gap:** hook gap: yes, gates/percall.py (the check reads the command text for the flag only).

### L-F17-02: What words let one key-route Clone Hero call through, and which calls are key-route?
- **Source:** I1-006, I3-033, I2-081; F17 Q2, V17 candidate 15 (partly).
- **Kind:** K1, K5, K6.
- **Verdict:** CONFIRMED as a pattern; clone-hero-mcp.md:29 and server.py:146-148 were not opened by the verifier (the session's ch server text carries the same sentence). See also L-F16-04 for the verified percall/game_tools pair.
- **Homes:** gates/review_rules.py:53-55 `KEY_ALLOW_TEXT = "allow key run"`; gates/percall.py:15-18, :91-96; Hydra tools/ch_mcp/server.py:146-148, :466, :477; tools/ch_probe/experiments/play_chart.py:1281; docs/agents/clone-hero-mcp.md:29 ("unless the user has typed exactly `allow key run` ... each such message lets one call through"); gates/README.md:13, :37; memory ch-runs-must-not-need-focus.md:17.
- **Splits:** None in values (same "exactly" vs tolerant-match edge as L-F17-01).
- **Ruling quotes:** None found.
- **Tests:** test_progress.py:612-637 (text names the tools); test_review_rules.py:852 pins the phrase.
- **Open:** none.
- **Hook gap:** none.

### L-F17-03: What does a reviewer's last line mean, and what if it is missing?
- **Source:** I2-041, I2-042; F17 Q3, V17 candidate 4.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED (verifier saw reviewer.md:28-33 and doc-review.md:120-125 directly; README:43 and derive-once-review.md:149-151 by quote plus a grep hit).
- **Homes:** gates/agent_stop.py:21-23 `_BLOCKED, _CLEAN`, :102-109 verdict_word (neither -> CLEAN with a problem); C:\Users\Patrick\.claude\agents\reviewer.md:25-33; gates/README.md:43; Hydra docs/agents/doc-review.md:117-125; Hydra docs/agents/derive-once-review.md:143-151, :86, :106. The sentence "A report with no sign-off line is recorded CLEAN, because a reviewer that finished has signed off what it fixed, and the main session is told the line was missing" is word for word in reviewer.md:33, README:43, doc-review.md:125, derive-once-review.md:151.
- **Splits:** None on the line words.
- **Ruling quotes:** review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." Covers the rule, not the four copies.
- **Tests:** test_agent_stop.py holds the hook's behaviour; no test holds the four prose copies to agent_stop.py.
- **Open:** none.
- **Hook gap:** none.

### L-F17-04: Is the main session told the right thing when a verdict has a problem?
- **Source:** I2-031, I2-045; F17 Q4, V17 candidate 6.
- **Kind:** K3, K1.
- **Verdict:** CONFIRMED.
- **Homes:** gates/stall.py:268-281 (every relayed problem ends "A move of main to a tip with no verdict is refused."); gates/agent_stop.py:242-243 `problem = "Reviewer %s's verdict: ..."` set when a verdict WAS recorded (:243), :248-251 "No verdict was recorded for reviewer %s: %s. The merge stays refused until a reviewer's verdict is recorded."; gates/review_rules.py:222, :234-253 (carry/waiver routes); Hydra derive-once-review.md:189-191, :202; gates/README.md:51.
- **Splits:**
  - SPLIT: stall.py:280-281 says a move to a tip with no verdict is refused for every verdict problem, but agent_stop.py:243 raises a problem also when a verdict was recorded (missing sign-off line, fallback to the handed tip, a file skipped). Also both sentences leave out the waiver and carry routes of review_rules.py:234-253 (finder).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F17-05: How is the brief hashed, and how is a reviewer tied to its handover?
- **Source:** I2-044, I2-049; F17 Q5, V17 candidate 8.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED (whether the two strings are byte-equal is a run question).
- **Homes:** gates/review_dispatch.py:146-173 (:172 `hashlib.sha256(prompt.encode("utf-8"))`); gates/agent_stop.py:26-36, :199-201 (:199 `hashlib.sha256(first.encode("utf-8"))` over the transcript's first user message); agent_stop.py:127-158 picks the commit; gates/README.md:39,:41,:45 ("`signed_tip` owns"); Hydra derive-once-review.md:200; model-postures.md:5; reviewer.md:33.
- **Splits:** None stated on the hash itself.
- **Ruling quotes:** None found.
- **Tests:** test_agent_stop.py and review_dispatch tests cover each side; nothing compares them.
- **Open:** a dispatch plus stop drill.
- **Hook gap:** none.

### L-F17-06: Why is a reviewer dispatch refused: four reasons or five?
- **Source:** I2-049 area; F17 Q5 / Split 7, V17 split 7.
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED. The finder cited derive-once-review.md:200; the list sits in text beginning at :200.
- **Homes:** Hydra docs/agents/derive-once-review.md:200 "no key, a key already on main, a key git does not know or more than one commit"; gates/README.md:39 adds "a commit in a repo that has no local `main` branch"; code gates/review_dispatch.py:128 (no local main -> not gated).
- **Splits:**
  - SPLIT: derive-once-review.md lists four refusal causes; README.md:39 lists five (adds no local `main` branch).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F17-07: Which files count as documents for a verdict: markdown only?
- **Source:** I2-050, I2-051, I2-052; F17 Q6, V17 candidate 5.
- **Kind:** K2, K1.
- **Verdict:** CONFIRMED.
- **Homes:** gates/agent_stop.py:161-172 `DOC_SUFFIXES = (".md", ".html", ".htm")`, :175-178 asks_doc_review, :224-226; gates/README.md:49 "a markdown file or an HTML page ... `DOC_SUFFIXES` in `agent_stop.py` owns the list"; reviewer.md:33 "fingerprints every markdown file the range changes"; derive-once-review.md:151 same; :202 "every markdown file in the range"; Hydra doc-review.md:21, :29.
- **Splits:**
  - SPLIT: three texts say "markdown" only; the hook also records `.html` and `.htm`. Differs when an HTML page is in the range.
- **Ruling quotes:** None found.
- **Tests:** none compares the docs' "markdown" with DOC_SUFFIXES.
- **Open:** none.
- **Hook gap:** none.

### L-F17-08: Is "same document" and the "later edits pass" rule decided once?
- **Source:** I3-036, I3-038, I3-039, I3-040, I3-041, I3-042, I3-047, I3-048; F17 Q7.
- **Kind:** K1, K5, K6.
- **Verdict:** NOT VERIFIED. V17 checked only the waiver-length part (row 09); the path comparison, the carry-rule copies and the "later edits pass" claim were not opened.
- **Homes:** (finder only) gates/review_rules.py:65-74 doc_key, :188-196 `_clean_keys`, :234-259 carry/verdict/waiver/doc rules, :256-259 compares the path of a recorded verdict; gates/publish_gate.py:68-71 `document_path_id` ("the one place in this file that compares a document's path with a recorded one"), :74-88 `_signed_off` (:75 "The user's rule (Hydra CLAUDE.md, "Doc review")"); carry rule also restated as a comment in gates/ref_judge.py:305-316 and in Hydra derive-once-review.md:189-191; Hydra doc-review.md:17, :102; CLAUDE.md "Doc review" says "each document gets it once".
- **Splits:** None value-for-value (finder). Finder notes no Confirmed line says "later edits pass".
- **Ruling quotes:** review-and-hooks.md:27 (every publish needs a review); not the later-edits rule.
- **Tests:** none.
- **Open:** all of it.
- **Hook gap:** none.

### L-F17-09: Waiver key: docs say `<key>`, the code takes 7 to 40 or 64 hex
- **Source:** I3-027, I3-038, I2-082; F17 Q8, V17 candidate 13.
- **Kind:** K1, K4.
- **Verdict:** CONFIRMED.
- **Homes:** gates/review_rules.py:25-28 WAIVER_PHRASE (7-40 hex for derive-once and real-app, 7-64 for doc-review), :77-78 waiver_phrase() (refusal tells the user to type the first 10); gates/waiver.py:3,:19-22; gates/README.md:37,:55; Hydra doc-review.md:21 "`waive doc-review <key>`"; derive-once-review.md:189 "`waive derive-once <key>`"; no Hydra doc names `waive real-app` (finder).
- **Splits:** None value-for-value; docs say `<key>`, code accepts a prefix and the refusal gives 10 characters. Differs when the user types 7 hex, a full 40 or the first 10.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F17-10: Which pages are what outsiders read?
- **Source:** I3-026; F17 Q9, V17 candidate 12.
- **Kind:** K1.
- **Verdict:** CONFIRMED on the code home and CLAUDE.md; doc-review.md:17 and public-doc-style.md:3 not opened by the verifier.
- **Homes:** gates/review_rules.py:23-24 PUBLIC_DOCS (four pages; comment "(the user's call, 2026-10-10)"); Hydra CLAUDE.md "the four public pages"; docs/agents/doc-review.md:17 (four paths: `README.md`, `docs/README.md`, `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`); docs/agents/public-doc-style.md:3 (same four plus release notes).
- **Splits:** None (public-doc-style adds release notes, gated by the publish route).
- **Ruling quotes:** None found: no `## Confirmed` line names the pages (checked review-and-hooks.md).
- **Tests:** none ties doc-review.md's four pages to PUBLIC_DOCS.
- **Open:** none.
- **Hook gap:** none.

### L-F17-11: Do the Hydra gate marker files name the real owner of the code-path and doc-page lists?
- **Source:** I3-025, I3-028, I3-029, I3-043, I3-044, I3-045, I3-046 (marker part); F17 Q10, V17 candidate 14, F18 Family 5.
- **Kind:** K8, K1, K6.
- **Verdict:** CONFIRMED (marker claims); the record-fields K1 was not separately verified here (see L-F18-22).
- **Homes:** Hydra `.derive-once-gate` lines 2-4 "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file."; Hydra `.doc-review-gate` lines 2-3 "...decided by the user-scope hooks (hooks\lib\doc_review_rules.ps1), not by this file." Verifier: C:\Users\Patrick\.claude\hooks does not exist; gates\lib holds only hook_input.ps1; no derive_once_rules.ps1 outside worktrees (find to depth 3). Live owners: gates/review_rules.py:21 BUILTIN_CODE, :22, :24 PUBLIC_DOCS, :180-185 `_is_code`; gates/README.md:138 repeats the ~/.claude marker's three `gate:` lines.
- **Splits:**
  - SPLIT: the two markers say the path rules live in hooks\lib\*.ps1; those files are not in the live gates folder; review_rules.py:21-24 holds the lists.
- **Ruling quotes:** None found.
- **Tests:** none (the RETIRED list in test_docs_numbers.py does not check these two marker files, finder F18).
- **Open:** verifier searched .claude to depth 3 excluding worktrees.
- **Hook gap:** none.

### L-F17-12: Which ref is main, which repo is gated, and do the shim strings match the judge?
- **Source:** I3-001, I3-002, I3-003, I3-014, I3-015, I3-016, I3-017, I3-057, I3-059, I3-064; F17 Q11, V17 candidate 10.
- **Kind:** K1, K6, K3 (light).
- **Verdict:** CONFIRMED for "which ref is main" and "gated repo"; the shim-string copies and the judged-move prose were not separately verified (NOT VERIFIED part).
- **Homes:** "main ref": gates/ref_judge.py:33 MAIN; gates/agent_stop.py:170; gates/publish_gate.py:47; gates/review_dispatch.py:111,:123,:128,:136; gates/cleanup_sweep.ps1:270; more uses in review_rules.py. "Is this repo gated": gates/publish_gate.py:42-60 `gated_repo` (marker on main; finder: throws when git unreadable, not re-read by the verifier); gates/review_dispatch.py:128-137 (`_has_main` and either marker; returns not-gated when no local main); gates/review_rules.py:408-410 `_has_marker`. Shim: gates/ref_judge.py:31-32 ALLOW/DENY/SELFTEST_OK; gates/ref_gate.sh:15-16 and gates/install_ref_gate.ps1:12-13 copies (finder only). Prose: gates/README.md:53,:57.
- **Splits:**
  - SPLIT (K3, verifier's note, finder's reading for the throw): publish_gate raises on unreadable git while review_dispatch returns not-gated on a repo with no local main; the verifier read only review_dispatch.
- **Ruling quotes:** review-and-hooks.md:30 chose "Git-side hook (Recommended)"; speaks to the design only (finder).
- **Tests:** test_ref_judge.py:396-416 runs the shim with a stub (partial); :614,:624 pin strings.
- **Open:** none beyond above.
- **Hook gap:** none.

### L-F17-13: Does a crash in a hook refuse or only inform, and is that stated once?
- **Source:** I2-074, I2-086, I3-060, I3-083, I3-065 (matcher side); F17 Q12, V17 candidate 7, F17b I3-060, V17b I3-060.
- **Kind:** K1, K3, K6.
- **Verdict:** CONFIRMED-CHANGED. Verifier: the K3 reading is overstated; the module's docstring says a deny becomes a systemMessage, so the real two-home fact is K1/K6 (percall's mode string written in percall.py:176 and in prompt.py PARTS). Whether a crash inside the part informs rather than refuses is a run question. F17b: code modes and `onFailure` agree today.
- **Homes:** gates/common.py:53-70 `run(body, mode)` ("closed" denies "Gate hook crashed, so the call is refused"); "closed": publish_gate.py:135, delete_rule.py:187, percall.py:176, review_dispatch.py:178; "loud": agent_stop.py:297, posture.py:55, prompt.py:67, shell_output.py:201, waiver.py:50; prompt.py:17 `PARTS = (("posture","loud"),("percall","closed"),("waiver","loud"))`, :30 passes a variable `mode`, :6-7 "A prompt cannot be refused here, so a part's deny or ask is said in the systemMessage instead"; registration `onFailure: block` on percall (`*`), review_dispatch (`Agent`), publish_gate (`Artifact|ExitPlanMode`) in settings.fragment.json:153,169,185 and live settings.json:163,179,195; gates/README.md:95-100, :190; ruling_rules.py:43,:394 "a crash refuses, like every refusing gate".
- **Splits:**
  - SPLIT (verifier's addition): percall is "closed" when run alone and informing when run through prompt.py.
- **Ruling quotes:** agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (about agents; not on this question).
- **Tests:** gates/tests/test_docs_numbers.py:163 asserts the shell_output registration has no `onFailure`; nothing ties the code-mode list to the `onFailure` list or PARTS to the scripts; test_sync_settings.py compares temp copies only.
- **Open:** a crash drill of prompt.py (not run). Verifier did not open delete_rule's import by percall.
- **Hook gap:** none.

### L-F17-14: Two readers of "did the user type this"
- **Source:** I3-037, I2-084, I2-085, I2-086; F17 Q13, V17 candidate 9.
- **Kind:** K6, K2, K3.
- **Verdict:** CONFIRMED (which transcripts carry which field is a data question, not checked).
- **Homes:** gates/review_rules.py:89-113 `human_prompt_text` (reads `turnOrigin`, rejects non-text blocks, no toolUseResult, not isMeta/isSidechain; :103); C:\Users\Patrick\.claude\tools\session_cards\common.py:372-419 classify_user/human_text, :397, :422-435 (:433 reads `origin.kind` and requires `commandMode == "prompt"`; strips `<system-reminder>` blocks; accepts slash-command arguments and queued prompts); gates/ruling_rules.py:16,:270 calls session_cards is_users_words ("the one home of that test", README:59, check.py:164); allows and waivers use review_rules.
- **Splits:**
  - SPLIT: review_rules reads `turnOrigin`, rejects non-text blocks, does not strip reminders, ignores queued prompts and slash commands; the card tool reads `origin.kind`, strips reminders, accepts slash-command arguments and queued prompts.
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:118,:721 hold the card tool's reader; test_review_rules covers review_rules; nothing compares them.
- **Open:** which transcripts carry which field.
- **Hook gap:** none.

### L-F17-15: Ruling approval labels and question text copied into docs
- **Source:** I3-073, I3-075, I3-076, I3-077, I3-078, I3-079; F17 Q14, V17 (not reopened), V18 candidate 10 ("File it" CONFIRMED, F18 Family 7).
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED in F17's own verifier; see L-F18-11 where V18 confirmed the labels and question text copies for the same code.
- **Homes:** (finder) gates/ruling_rules.py:95-101 CONFIRMED/UNCONFIRMED headings, INSTALL_DATE "2026-11-14", APPROVE/DECLINE "File it"/"Do not file", QUESTION "File this ruling in {path}?"; Hydra CLAUDE.md "Rulings and this machine"; docs/agents/agent-rules.md:33; docs/agents/rulings/README.md:3,:41-43,:51; gates/README.md:57-61.
- **Splits:** None here (see next row for README:20).
- **Ruling quotes:** None found for "File it".
- **Tests:** test_docs_numbers.py:129-135 holds only the heading and an old phrase.
- **Open:** all.
- **Hook gap:** none.

### L-F17-16: Does rulings/README.md describe what the Unconfirmed section holds?
- **Source:** I3-073 area; F17 Q14 / Split 8, V17 split 8.
- **Kind:** K2.
- **Verdict:** CONFIRMED, with a conflict on the line numbers: V17 says "review-and-hooks.md:10 is a 2026-11-06 line (I did not check that line 10 sits under `## Unconfirmed`)"; F16 and V16 both read line 10 as Confirmed (the Confirmed block runs lines 7-35; `## Unconfirmed` at :37).
- **Homes:** Hydra docs/agents/rulings/README.md:20 "copied from memory notes on 2026-10-31"; docs/agents/rulings/review-and-hooks.md:40-43 (Unconfirmed, 2026-11-06 lines).
- **Splits:**
  - SPLIT: README.md:20 says the Unconfirmed section holds lines copied from memory on 2026-10-31; the section also holds lines dated 2026-11-06 (lines 40-43).
- **Ruling quotes:** None (these are Unconfirmed).
- **Tests:** none.
- **Open:** the verifier's line-10 uncertainty.
- **Hook gap:** none.

### L-F17-17: Where is the small-text limit, and how is size counted?
- **Source:** I3-062, I3-063, I3-067; F17 Q15, V17 candidate 16, F17b I3-063, V17b I3-063.
- **Kind:** K1, K7 (the 2000 half has no ruling).
- **Verdict:** CONFIRMED.
- **Homes:** gates/publish_gate.py:13-14 `SMALL_LINES, SMALL_CHARS = 20, 2000` (comment "The user's decisions of 2026-10-10"), :5 "the small-text rule has this file as its one home", :29-33 counting (CRLF/CR folded, UTF-16 units over 2000 is large, non-blank lines over 20, latin-1 fallback; comment "as the old PowerShell gate counted them"), :103-107 reads the constants; gates/README.md:63 and Hydra doc-review.md:17 point with no number; gates/publish.ps1:3.
- **Splits:** None found.
- **Ruling quotes:** review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire..." names the 20-line side only. A grep of the rulings folder for "2000"/"2,000" found only an unrelated clone-hero line. The code comment calls the numbers the user's decision, which is a claim, not evidence.
- **Tests:** gates/tests/test_publish.py:107-109 `test_20_lines_and_2000_characters_pass` states both numbers itself (a second statement).
- **Open:** the "old PowerShell gate" is not in the gates folder, so the counting copy cannot be checked.
- **Hook gap:** none.

### L-F17-18: Which tool calls does publish_gate gate, and does the matcher agree?
- **Source:** I3-065; F17 Q15, F17b I3-065, V17b I3-065.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED.
- **Homes:** gates/publish_gate.py:112 (`ExitPlanMode` with plan), :117-119 (`Artifact` with action None/""/"publish", a file_path, asset not True); settings.fragment.json:174 and live settings.json:184 matcher `Artifact|ExitPlanMode` (equal); gates/README.md:63, :49, :190 (field names `plan`, `planFilePath`); Hydra doc-review.md:17, :21.
- **Splits:** None today.
- **Ruling quotes:** review-and-hooks.md:27 (publish needs fresh review).
- **Tests:** test_publish.py checks each tool's behaviour; none ties the matcher to publish_gate.
- **Open:** none.
- **Hook gap:** hook gap: yes, gates/publish_gate.py (a gated action reachable under a tool name outside the matcher is not seen; finder note, not examined).

### L-F17-19: When is a change "comments and blank lines only"?
- **Source:** I3-045; F17b, V17b.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED (small): the verifier gives review_rules.py:345 for `change_is_comments_only` (finder wrote :346) and other callers at test_review_rules.py:559, :700, :791; no change in verdict.
- **Homes:** gates/review_rules.py:345-346 `change_is_comments_only`, :262-363 algorithm and `_c_code`; gates/README.md:55 (restates the cases in words and names owners).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_review_rules.py:702, :706, :719 run the real function.
- **Open:** none.
- **Hook gap:** none.

### L-F17-20: Is the live settings.json equal to the fragment?
- **Source:** F17b side note; V17b.
- **Kind:** K1.
- **Verdict:** CONFIRMED. The two `hooks` blocks are equal as parsed data (11 entries; only three PreToolUse entries carry `onFailure: block`).
- **Homes:** gates/settings.fragment.json and C:\Users\Patrick\.claude\settings.json.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** no test compares the real files (test_sync_settings.py uses temp copies); gates/sync_settings.py reports a difference when run.
- **Open:** files must be read as UTF-8.
- **Hook gap:** none.

---------------------------------------------------------------- F18 rows

### L-F18-01: What exact words let one whole-library run through (second finder's view)?
- **Source:** I5-044, I5-077, I5-163 (library part); F18 Family 1, V18 candidate 1.
- **Kind:** K1, K2, K3 (K8 for the memory note).
- **Verdict:** CONFIRMED (the Hydra batch_scope.h flag spelling was not opened). Overlaps L-F17-01.
- **Homes:** Hydra docs/agents/agent-rules.md:14 and brief-preamble.md:49 "sending exactly `allow library run`; each such message lets one command through"; gates/review_rules.py:51-52, :81-82; gates/percall.py:45-55 ("they allow one run by sending exactly: allow library run"); memory no-full-library-runs.md:15 "No hook enforces it yet." (:11-15 also "about 115 charts"); this-machine.md:91; audit-method.md:125.
- **Splits:**
  - SPLIT: docs "exactly" against review_rules.py:52 (case ignored, one trailing `.` or `!`, trimmed). Differs for `Allow Library Run!`.
  - SPLIT: the memory note "No hook enforces it yet" against percall.py:45-55, where the hook refuses. By reading the note is stale; it is not the user's words.
- **Ruling quotes:** tests-and-runs.md:14 and review-and-hooks.md:16 (both Confirmed) "i want to be able to give explicit approval for a full library run." Names the approval, not the spelling; the memory-note side is not a ruling.
- **Tests:** test_review_rules.py:839-845, test_progress.py:427-455, test_review_dispatch.py:309-336 test the code's match; nothing compares the docs.
- **Open:** "about 115 charts" has no check found (finder).
- **Hook gap:** none stated here.

### L-F18-02: Does a reviewer's unfixed finding hold the merge, and by what signal?
- **Source:** I5-041, I5-108, I5-089, I6-006 (context), I5-131 (budget part); F18 Family 2, V18 candidate 4.
- **Kind:** K1, K3, K2.
- **Verdict:** CONFIRMED-CHANGED. Verifier added a split the finder missed (derive-once-review.md:151 against :106 and :136). "No hook reads the Holds the merge prefix" is not fully proven (verifier searched agent_stop.py only).
- **Homes:** Hydra docs/agents/derive-once-review.md:106 Notes do not hold the merge "with three exceptions" (visible change marked "not asked", unfixable kind H, unfixable kind D; the last two start "Holds the merge:"; "You still sign off CLEAN ... the orchestrator holds the merge, not your sign-off line."), :136 ("is the one kind that does wait"), :149 and C:\Users\Patrick\.claude\agents\reviewer.md:31 "BLOCKED when a finding stays that you could not fix and that must hold the merge", :151 "Only `SIGN-OFF: BLOCKED` holds the merge."; doc-review.md:125 "Only SIGN-OFF: BLOCKED holds the [publish]"; doc-review.md:98 "It does not hold the publish for them"; agent-rules.md:11 "the merge does not wait for them"; memory one-reviewer-rule-d107.md:11 "pass them on, don't hold the merge"; gates/agent_stop.py:23, :105-109 reads only the two sign-off strings.
- **Splits:**
  - SPLIT: derive-once-review.md:106 (sign off CLEAN, hold by Note prefix) against :149 and reviewer.md:31 (write BLOCKED when something unfixed must hold the merge).
  - SPLIT: derive-once-review.md:151 and doc-review.md:125 "Only SIGN-OFF: BLOCKED holds the merge" against :106 and :136 (a Note holds it too). Verifier's addition.
  - SPLIT: agent-rules.md:11 "the merge does not wait" and "nothing goes back to the user" against derive-once-review.md:106 (three held cases; questions go to the user).
  - SPLIT: memory one-reviewer-rule-d107.md:11 (no exceptions, no D314) against derive-once-review.md:106.
  - SPLIT: doc-review.md:98 (no exceptions) against derive-once-review.md:106 when one reviewer covers code and docs.
- **Ruling quotes:** review-and-hooks.md:20 (Confirmed) "i asked for the review agent to fix its findings directly, then we're done."; :21 "notes are fine, and yes do that for running out"; :9 is a question, not a rule. "alright approve b" is Unconfirmed (see next row). None names both sides of any split above.
- **Tests:** none holds the docs equal; agent_stop tests cover the verdict word only.
- **Open:** whether any hook reads the "Holds the merge:" prefix (verifier searched agent_stop.py only).
- **Hook gap:** hook gap: yes, gates/agent_stop.py (finder side note: the "Holds the merge:" Note prefix holds a merge only by the main session's reading; no hook reads it).

### L-F18-03: Does derive-once-review.md cite an Unconfirmed line as the user's ruling?
- **Source:** I5-108 area; F18 Family 2 (could not find it), V18 candidate 5 (new finding), V18b I6-026.
- **Kind:** K8.
- **Verdict:** CONFIRMED-CHANGED: the finder could not find the line; the verifier found it.
- **Homes:** Hydra docs/agents/derive-once-review.md:106 "The user ruled this on 2026-11-12 ("alright approve b")"; the words are at docs/agents/rulings/review-and-hooks.md:39, under `## Unconfirmed` (heading :37; `## Confirmed` at :7); session card docs/sessions/2026-11-13-bed936f0.md:23 quotes it as "approves option b: a finding a reviewer cannot fix holds the merge".
- **Splits:** None (a citation problem).
- **Ruling quotes:** None under Confirmed. The words do not say CLEAN versus BLOCKED, so even on the card they do not name both sides of the splits in L-F18-02.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F18-04: "Nothing goes back to the user" against "nothing goes back to the executor"
- **Source:** I5-041, I5-108; F18 Family 2, V18 candidate 6.
- **Kind:** K3 (wording).
- **Verdict:** CONFIRMED.
- **Homes:** C:\Users\Patrick\.claude\model-postures.md:5 "Nothing goes back to the executor and no second round (D107), except that a reviewer's fix that fails your build goes to a fresh executor"; Hydra agent-rules.md:11 and derive-once-review.md:90 "nothing goes back to the user"; derive-once-review.md:106 sends held questions to the user.
- **Splits:**
  - SPLIT (wording): the executor half is about the code author, the user half about findings. They differ when a Note holds a question for the user.
- **Ruling quotes:** review-and-hooks.md:20 (see L-F18-02).
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F18-05: Sign-off line, DOC_SUFFIXES and the rule 5 citation
- **Source:** I5-138, I5-139, I5-166, I5-167, I5-168, I5-165; F18 Family 3, V18 candidate 3.
- **Kind:** K2, K8 (minor).
- **Verdict:** CONFIRMED for "markdown" against DOC_SUFFIXES. NOT VERIFIED for the agent_stop.py:22 citation of "Hydra CLAUDE.md rule 5" (finder: rule 5 says that only for a reviewer out of budget).
- **Homes:** gates/agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`, used :172, :226; reviewer.md:33 "fingerprints every markdown file the range changes"; derive-once-review.md:151; doc-review.md:29 "documents are every markdown file the range changes"; gates/README.md:49 "a markdown file or an HTML page" (verifier did not open).
- **Splits:**
  - SPLIT: three docs say markdown only; the hook also takes html. (Same as L-F17-07.)
- **Ruling quotes:** review-and-hooks.md:19.
- **Tests:** test_docs_numbers.py `test_review_docs_name_no_typed_key` scans for typed-key words only.
- **Open:** the rule 5 citation.
- **Hook gap:** none.

### L-F18-06: How long is a review key, and what do the docs say?
- **Source:** I5-106, I5-164, I6-007, I6-008, I6-010; F18 Family 4 (not reached by V18).
- **Kind:** K1, K3 (minor).
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) gates/review_dispatch.py:17-21 `_KEY` (40 hex alone); gates/review_rules.py:56 `_HEX` (40 or 64), :26-28 (7-40 / 7-64); gates/README.md:39; derive-once-review.md:17,:189,:200-202; doc-review.md:29; model-postures.md:5; reviewer.md:9; Hydra docs/agents/handoff.md:19,:35,:51; .claude/commands/handoff.md:15.
- **Splits:** None changing a dispatch answer.
- **Ruling quotes:** None found.
- **Tests:** test_review_dispatch.py covers the 40-hex rule.
- **Open:** all.
- **Hook gap:** none.

### L-F18-07: Retired .ps1 hook files named by the Hydra marker files (second finder's view)
- **Source:** I3-101; F18 Family 5, V18 candidate 2.
- **Kind:** K8, K1.
- **Verdict:** CONFIRMED. Overlaps L-F17-11. Verifier: Glob of C:\Users\Patrick\.claude\hooks\** finds only state\build_slots\slot1.lock.
- **Homes:** Hydra `.derive-once-gate` line 3, `.doc-review-gate` line 3 (quoted in L-F17-11); gates/review_rules.py:22 BUILTIN_CODE, :24 PUBLIC_DOCS (:184, :448); gates/README.md:138 repeats the three `gate:` lines (finder); docs/agents/doc-review.md:17 repeats the four page names (finder).
- **Splits:**
  - SPLIT: marker comment against code owner (S3 in V18).
- **Ruling quotes:** None found.
- **Tests:** none compares doc-review.md:17's pages with PUBLIC_DOCS.
- **Open:** the README:138 and doc-review.md:17 copies were not opened by the verifier.
- **Hook gap:** none.

### L-F18-08: Text-to-GitHub kinds and the "small-text" naming
- **Source:** I5-027, I5-026, I5-173, I5-174, I3-089; F18 Family 6, V18 candidates 8-10 (in part).
- **Kind:** K1.
- **Verdict:** CONFIRMED in part (publish.ps1:8 six kinds; publish_gate.py:14 owns 20/2000 and :5 "the one home"; publish.ps1:3 "small-text rule"). README.md:63/65 and doc-review.md:17-21 copies not opened by the verifier.
- **Homes:** gates/publish.ps1:1-6,:8 ValidateSet of six kinds; gates/publish_gate.py:5, :13-14; gates/README.md:63, :65 ("short-text" naming); Hydra doc-review.md:17 ("A short text"), :19, :21; CLAUDE.md:53.
- **Splits:** None. Name drift: "small-text" (publish.ps1, publish_gate.py, review_rules.py:67) against "short-text" (README:63) against "A short text" (doc-review.md:17).
- **Ruling quotes:** review-and-hooks.md:27 (does not speak to the size limit).
- **Tests:** none holds publish.ps1's kinds equal to README/doc-review.
- **Open:** none beyond unopened copies.
- **Hook gap:** none.

### L-F18-09: "File it" question text, labels and install date copied into the rulings README
- **Source:** I5-055, I5-172, I3-083, I3-084; F18 Family 7, V18 candidate 8-10.
- **Kind:** K1.
- **Verdict:** CONFIRMED for the labels and question; README.md:51's date copy not read by the verifier.
- **Homes:** gates/ruling_rules.py:99-101 `INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`, QUESTION; :394 comment "a crash refuses, like every refusing gate" (a claim about other gates, F18: common.py:53-70 owns the mode); Hydra docs/agents/rulings/README.md:3, :40-43 (question and both labels written out), :51 ("INSTALL_DATE in ruling_rules.py, 2026-11-14"); agent-rules.md:33; gates/README.md:57-61.
- **Splits:** None.
- **Ruling quotes:** None found under Confirmed for "File it" itself; the rule rests on agent-rules.md:33's quote (not exhaustively searched).
- **Tests:** test_docs_numbers.py:129-136 checks the README holds the approval heading; no test compares README:41's question text with ruling_rules.py:101.
- **Open:** none.
- **Hook gap:** none.

### L-F18-10: What counts as the user's words, and in which pages?
- **Source:** I5-057, I5-059, I5-094; F18 Family 8; V18: not reached.
- **Kind:** K1, K3.
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) Hydra agent-rules.md:35-41 (a `## Confirmed` line; a decision entry only if it quotes Confirmed or a card), derive-once-review.md:45 (same condition), :52 ("a D-record, a plan's User decisions, a Confirmed line, a session card, or the words your brief quotes", no quote condition), premise-map.md:45 (adds "and confirms"), doc-review.md:63, rulings/README.md:18-22,:34, writer-brief.md:67.
- **Splits (finder only):**
  - SPLIT: derive-once-review.md:45 against :52 (D-record or plan decision stands as a place for step 9 without the quote condition).
  - SPLIT: the brief-quote rule worded three ways (derive-once-review.md:52, premise-map.md:45, doc-review.md:63; agent-rules.md does not list it).
- **Ruling quotes:** None found that say which places count.
- **Tests:** none.
- **Open:** all.
- **Hook gap:** none.

### L-F18-11: Which tests does a merge run, and where is the hooks list?
- **Source:** I5-035, I5-036, I5-113, I5-114, I5-190; F18 Family 9; V18: not reached.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) Hydra tools/precheck/test_pick.h:90-95 `shared_code` ("The one place shared code is decided"), test_pick.cpp:789-791 (three `always:` lines); agent-rules.md:7 (writes the three commands); model-postures.md:11 and gates/README.md:156 (hooks shared-file list: common.py, lib, settings.fragment.json, tests\lib; no code owner; K7); memory merge-tests-touch-changed-files.md:11-15; docs/development.md:542.
- **Splits:** None (same list in three homes today).
- **Ruling quotes:** tests-and-runs.md:15 (Confirmed) "Fix the doc line, and also add the repo-wide tests (docs-vs-code, single-owner scan, precheck self-test) to every merge run."; :17 "all the parts you didnt change will remain the same" (inside the "why do we need to run the full suite every time ..." line). The hooks half ("whole suite only when shared code") rests on tests-and-runs.md:34 (Unconfirmed); agent-rules.md:7 cites "the user's ruling, 2026-10-14".
- **Tests:** none compares agent-rules.md:7 with test_pick.cpp:789-791.
- **Open:** all.
- **Hook gap:** none.

### L-F18-12: Owner map and premise map rules restated in reviewer and planner pages; kind F "mechanically" or "candidates"
- **Source:** I5-132, I5-133, I5-134, I6-006, I6-011, I6-014; F18 Family 10, V18 candidate 7, V18b I6-013.
- **Kind:** K1, K8, K3, K2.
- **Verdict:** NOT VERIFIED for the owner-map/premise-map restatements. For the precheck wording: V18 candidate 7 CONFIRMED-CHANGED (partly unchecked: DOR:38 was read only truncated); V18b read DOR:38 "The precheck's lines cover kinds B, C and E, kind D's new numbers, and kind F's candidates." and did not reopen the DOR:21 contrast.
- **Homes:** (finder) Hydra owner-map.md:3 and premise-map.md:5 each claim to be "the one home ... other pages point here and do not copy"; agent-rules.md:23-25; reviewer.md:13,:15,:17; planner.md:15-19; doc-review.md:63; owner-map.md:39,:43-47; premise-map.md:74; derive-once-review.md:21 "It finds kinds B, C, E and F mechanically, and kind D's new numbers." against :38; Hydra tools/precheck/checks.cpp:8-12 labels (C, B, D, E, F); items.h:50 `kLargeRangeLines = 400`; clones.cpp:90 `kCloneWindowLines = 8`; ADR 0025:38.
- **Splits (finder only except the last):**
  - SPLIT: owner-map.md:39,:47 and a plan reviewer "signs off BLOCKED" on a contradiction; derive-once-review.md:106 says a code reviewer with a held finding signs CLEAN (different jobs; one reviewer covering both reads two instructions).
  - SPLIT (K2): derive-once-review.md:21 says kind F is found mechanically; :38 says kind F's lines are candidates.
- **Ruling quotes:** None found for these pages; owner-map.md:7 quotes the user ("Yes, write them"; agent-rules.md:25 same); the finder did not open code-and-engine.md.
- **Tests:** tests/test_docs_match_code.cpp checks plan shapes (premise-map.md:76); nothing checks reviewer.md or planner.md against the maps.
- **Open:** all.
- **Hook gap:** none.

### L-F18-13: Early start of a dependent task, reviewer-type constants, cards, pointer rows
- **Source:** I5-109, I5-106, I4-002, I5-019, I5-020, I5-024, I5-025; F18 Families 11, 12 and pointer rows; V18: not reached.
- **Kind:** K7, K6, K2, K5.
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) model-postures.md:7 earlyStart rule; gates/agent_stop.py:135-136 comment mentions early starts; Hydra docs/superpowers/plans/2026-11-08-player-audit-fixes.md:140 `earlyStart: F1`; no schema or checker found in the searched scope (K7). Reviewer type: gates/review_dispatch.py:16 and gates/agent_stop.py:20 `REVIEWER_TYPE` (two constants, same value), model-postures.md:5, derive-once-review.md:200, README:39. Cards: Hydra CLAUDE.md:25 "Every past session of this project has a short card ... in the repo at docs/sessions/" against writer-brief.md:18 (`local: yes`) and tools/session_cards/check.py:10,:216-226,:245-259,:281 (a local card never lands in the published folder; test_session_cards.py:254-294,:556-581 test that). Pointer rows: audit-method.md "four exits" (:19,:34,:121,:186; CLAUDE.md:33; test-rubric.md:189) not compared; CLAUDE.md:45 states the public-doc-style rule; "staged on main"/"worktree" rule (derive-once-review.md:189-197, CLAUDE.md:49, :17, model-postures.md:5) has no check before the ref gate (K5/K7).
- **Splits (finder only):**
  - SPLIT (K2 minor): CLAUDE.md:25 says every session has a card in the repo; writer-brief.md:18 and check.py say a local-only session's card is never in the repo.
- **Ruling quotes:** None found.
- **Tests:** none for these.
- **Open:** all.
- **Hook gap:** none.

### L-F18-14: Budget numbers (100/150 calls, 20/30 minutes) across prose homes
- **Source:** I5-131; F18 (numbers only), F18b, V18b.
- **Kind:** K1, K3.
- **Verdict:** CONFIRMED.
- **Homes:** gates/progress.py:13-15 "The one home of these numbers ... Docs quote them; code reads them here" (100/150, 20/30; :92, :97, :129); Hydra agent-rules.md:13; brief-preamble.md:71, :73; derive-once-review.md:90, :110; doc-review.md:91, :100; reviewer.md:11 "at 100 calls or 20 minutes you stop fixing"; model-postures.md:5 "inside 100 tool calls and 20 minutes" (task sizing); hooks tools/session_cards/writer-brief.md:101 "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report"; gates/README.md:29 (pointer); gates/stall.py:23,:140 reads MINUTES_STOP; agents-and-workflow.md:43 (Unconfirmed) "soft cap 100 calls (hand off), hard cap 150".
- **Splits:**
  - SPLIT (wording): reviewer.md says "100 calls", the rules and preamble "100 tool calls".
  - SPLIT: writer-brief.md "stop starting new cards, finish and report" against the hook's warn-then-refuse; finder also notes reviewer.md asks for "the plan line each finding breaks" which derive-once-review.md:110 and agent-rules.md:13 do not (finder only).
- **Ruling quotes:** review-and-hooks.md:24 (Confirmed) "Agents have a soft cap of 20 minutes ... hard cap of 30 minutes real time." (not re-read by the verifier).
- **Tests:** gates/tests/test_docs_numbers.py:44 CALL_COUNT regex `(\d+)(?:st|nd|rd|th)? tool calls?` over DOCS (:25-26: CLAUDE.md, agent-rules, preamble, derive-once-review, doc-review), :110-127 vs progress.CALL_WARN/CALL_STOP, :122-127 rule 7 quotes five numbers; :40 REVIEW_DOCS_HERE lists reviewer.md for the typed-key check only. reviewer.md, model-postures.md and writer-brief.md are not read for the numbers.
- **Open:** none.
- **Hook gap:** none.

### L-F18-15: Does the reviewer fix on the same branch, and how does the hook sign a tip?
- **Source:** I5-135; F18b, V18b (reviewer.md only).
- **Kind:** K1, K3, K5, K7.
- **Verdict:** CONFIRMED-CHANGED, partly unchecked. The verifier read reviewer.md only; the DOR/DR/agent_stop.py/README homes were not reopened (the verifier lists this row as "open").
- **Homes:** reviewer.md:19, :33; Hydra derive-once-review.md:112, :167, :200, :202; doc-review.md:95; gates/README.md:45 ("`signed_tip` in `agent_stop.py` owns the rule"); gates/agent_stop.py:132-155 `signed_tip`; ref_judge.py:184-200 `missing_trailers` skips merge commits; brief-preamble.md:65; integrate.md:27 (finder only).
- **Splits (finder only):**
  - SPLIT (K3): reviewer.md/derive-once-review.md say the hook "checks that the new tip descends from the tip you were handed"; README:45 says the hook signs the reviewer's own newest commit found by the `Agent:` trailer among branches that descend, falling back to the handed tip.
- **Ruling quotes:** review-and-hooks.md:20.
- **Tests:** none holding the prose copies equal.
- **Open:** code side not reopened.
- **Hook gap:** none stated.

### L-F18-16: Cross-repo review: does a ruling back the one exception?
- **Source:** I5-137; F18b, V18b.
- **Kind:** K8, K1, K5.
- **Verdict:** CONFIRMED-CHANGED: the finder's "no approval found" held up under the verifier's grep of the whole rulings folder for "2026-10-24" (hits: review-and-hooks.md:48 "just move everything to c++"; tests-and-runs.md:15,:16,:33; ui-and-display.md:47-51, all other subjects). Not searched: session cards, the decision log.
- **Homes:** gates/review_dispatch.py:68 docstring "the one exception the user approved on 2026-10-24"; review_dispatch.py:65-99 `judged_folder`; reviewer.md:23; derive-once-review.md:200; gates/README.md:39; model-postures.md:5.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** session cards and the decision log.
- **Hook gap:** none.

### L-F18-17: The ref gate "owns" claims and the trailer rule; making a hooks change live
- **Source:** I5-170, I5-171 (owner claims), I5-191; F18b; V18b: "NOT RE-VERIFIED".
- **Kind:** K1, K5, K7.
- **Verdict:** NOT VERIFIED.
- **Homes:** (finder only) gates/README.md:53-55 owner claims (missing_trailers ref_judge.py:182, judge_push_lines :143, restore_checkout :319, _code_cause review_rules.py:210, explain_main_move :398: the functions exist); trailer rule also at brief-preamble.md:65, ref_judge.py:184 comment "the user's ruling named an agent's own commits" (no Confirmed line found; review-and-hooks.md:57 Unconfirmed records "subagent Agent: trailer must be its own id"), README:160, reviewer.md:33. I5-191: README:160 "`git -C C:\Users\Patrick\.claude merge --ff-only <branch>`", README:110, :112, :166-168; model-postures.md:11, agent-rules.md:7; a finder negative grep found no hook refusing a non-fast-forward merge in ~/.claude (prose with no hook; K5/K7).
- **Splits:** None found.
- **Ruling quotes:** None found. agent-rules.md:7 cites "the user's ruling, 2026-10-14" (not verified).
- **Tests:** not opened.
- **Open:** all.
- **Hook gap:** none stated.

### L-F18-18: The real-app record: is its file name and field names written once?
- **Source:** I5-171, I3-044 (F17 Q10 K1); F18b, V18b.
- **Kind:** K6, K8 (comment), K3.
- **Verdict:** CONFIRMED.
- **Homes:** gates/review_rules.py:47-49 (comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them." then :49 `REALAPP_FILE = "realapp.jsonl"`), :384-385 reads `kind`, `dirty`, `commands`, `commit` by string, :371 `realapp_checked`; Hydra tools/hydra_ui_mcp.py:78-81 (`REALAPP_FILE = "realapp.jsonl"`), :194-195 the record literal `{"kind": "realapp", "commit", "dirty", "commands", "shots"...}`; gates/README.md:55; Hydra docs/agents/ui-testing.md:233-251 (seven fields; ":235 The field names below are that gate's contract"; :5-13 a layout change needs the real app, case 2) against the gate firing on any change under `src/ui/` (README:55).
- **Splits:**
  - SPLIT: the name and fields are string literals in both repos; the reader keeps its own copy although its comment says the tool owns them. A third copy is the hand-written record at test_review_rules.py:404.
  - SPLIT (K3, finder only): ui-testing.md's trigger (what the change does) against the hook's trigger (where the file is).
- **Ruling quotes:** rulings/ui-and-display.md:115 is an Unconfirmed note ("ugly as hell"); nothing under Confirmed read on the gate.
- **Tests:** none compares the two constants; tools/test_hydra_ui_mcp.py:104 reads `hm.REALAPP_FILE`; test_review_rules.py:410 uses `review_rules.REALAPP_FILE`; test_docs_numbers.py:54 only needs the README to name "realapp.jsonl".
- **Open:** one side renames and each side's tests still pass (stated in general words).
- **Hook gap:** none.

### L-F18-19: The kinds of copy and their letters
- **Source:** I6-013; F18b, V18b.
- **Kind:** K1, K2.
- **Verdict:** CONFIRMED (the DOR:21 contrast was not reopened; see L-F18-12).
- **Homes:** Hydra derive-once-review.md:28 "Seven are listed here; kind G is under ...", :30-36 (A, B, C, D, E, F, H), :38; tools/precheck/checks.cpp:8-12 labels (C, B, D, E, F); test-rubric.md:39,:148 (kind A); integrate.md:9.
- **Splits:** None (the finder's contrast is L-F18-12).
- **Ruling quotes:** review-and-hooks.md:39 (Unconfirmed) names kind H and kind D.
- **Tests:** none found.
- **Open:** none.
- **Hook gap:** none.

### L-F18-20: Is adding an entry to `known_clones` a finding or an allowed parked step?
- **Source:** I6-015; F18b, V18b.
- **Kind:** K2, K3.
- **Verdict:** CONFIRMED.
- **Homes:** Hydra derive-once-review.md:38 "A change that adds an entry to `known_clones` is a finding."; Hydra docs/adr/0029-the-single-owner-rules-are-data.md:130-142 ("a copy may be parked, but never kept"; "an agent that can't edit a file another task owns still land its change"; :140 "The user chose 14 days in the plan's question 5"); docs/adr/0025:40-53; tests/single_owner/clones.cpp:8-9; engine.h:35 `kBaselineMaxDays = 14`; scan.cpp:571-572 `baseline_due`; baseline.json:2-3 empty today.
- **Splits:**
  - SPLIT: derive-once-review.md calls any add a finding; ADR 0029 allows a dated, time-limited entry. The code accepts a dated entry and fails only after `remove_by`. Which one a reviewer follows is stated nowhere.
- **Ruling quotes:** none found; ADR 0029:140 quotes a choice (an ADR line, not a Confirmed line).
- **Tests:** scan.cpp:571-589 tests the date rule, not a rule that nothing new may be added.
- **Open:** none.
- **Hook gap:** none.

### L-F18-21: Review steps cited by number, and the D13 example
- **Source:** I6-016, I6-024; F18b, V18b.
- **Kind:** K1, K5, K7, K2, K8.
- **Verdict:** I6-016 PARTLY CONFIRMED (DOR:42-53 numbered 1-10; citations from agent-rules.md:18, :41, check_5.cpp:32, changed_names.h:3 not opened). I6-024 CONFIRMED.
- **Homes:** Hydra derive-once-review.md:42-55 (ten steps), :106, :134, :80, :86, :108, :110, :128 (cite steps by number); agent-rules.md:18, :21, :41; brief-preamble.md:115; tools/precheck/check_5.cpp:32; changed_names.h:3. I6-024: derive-once-review.md:52 "D13 approved "free", not the sentence "no more than 0.0 ms late""; docs/audit/2026-10-03-fix-decisions.md:33 (D13) and :1680-1686 (D302 2026-10-31 "This reverses D13"; user's words :1682 "who came up with this rule and when was it approved by me?"); questions-and-audits.md:18 (Confirmed, 2026-10-31) chose "Yes, add it (Recommended)" to "Proposal 4. Should the reviewer check 'was this asked for?' ...", cited again at agent-rules.md:19 for rule 13 (questions-and-audits.md:19, Confirmed, "Proposal 5").
- **Splits:**
  - SPLIT (K2/K8): derive-once-review.md:52 names D13 as the example; D302 reversed D13 on the same date as the ruling it cites. Whether "free" is the user's own words: not checked (finder's grep of rulings for the phrase found nothing).
- **Ruling quotes:** questions-and-audits.md:18 (Confirmed), as above; it backs rule 12's "was this asked for?" check, not the D13 wording.
- **Tests:** none holds the step numbers.
- **Open:** renumbering would break the number citations silently (K7, finder).
- **Hook gap:** none.

### L-F18-22: Step 10's example numbers against the Clone Hero page
- **Source:** I6-026 (and I6-013/015/016/024 DOR side); F18b, V18b.
- **Kind:** K2, K1.
- **Verdict:** CONFIRMED.
- **Homes:** Hydra derive-once-review.md:55, :57-64 (chord 466 on Pitch Dark: "ratio 1.1067 ... planned 158,254.77 ms. ... The game stored 158,268.99 ms, which is 14.21 ms away"), :66-72, :86, :106, :108 ("15 ms"; code tools/replay_schedule.cpp:40 `kSpEndLeadMs = 15.0`); Hydra docs/clone-hero/05-star-power.md:437 (chord 466: ratio 1.3926, planned 158,262.53 ms, game read 158,268.4 ms, 5.8 ms late; "Chord 466 collects the phrase that clamps activation 2's end"; the verifier found 1.1067 does not occur on that page); finder: tools/replay_schedule.cpp:436-437 still reads `q.transfer` / `act.transfer_post` and :249 points to a removed plan, while derive-once-review.md:62-64 speaks in present tense of the tool taking the ratio from `SPSqueeze::transfer`.
- **Splits:**
  - SPLIT: one chord, two pages: different ratio, plan, game read and activation. Which run each used was not checked.
- **Ruling quotes:** review-and-hooks.md:9 is a question, not a rule (finder). "alright approve b" is Unconfirmed (L-F18-03).
- **Tests:** none.
- **Open:** whether the tool example is still the flaw or was fixed (finder; present tense).
- **Hook gap:** none.

---------------------------------------------------------------- Summary lists

Candidates the verifier REFUTED: L-F16-13 (the claim that check_handoff has no code for hooks-repo lines).

Rows no verifier reached (NOT VERIFIED): L-F16-10, L-F16-11, L-F16-15, L-F16-17, L-F16-22, L-F17-08, L-F17-15, L-F18-06, L-F18-10, L-F18-11, L-F18-12 (owner-map half; the precheck wording half was reached), L-F18-13, L-F18-17. Parts of verified rows not reached: L-F16-16 (-DryRun, check-mode text), L-F16-19 (I10b-010, -013, -029, -045), L-F17-12 (shim strings, judged-move prose), L-F18-05 (rule 5 citation), L-F18-15 (code side), L-F18-21 (step citations).
