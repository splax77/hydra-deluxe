# TOOL-AUDIT-T5-F12 (finder F12, agent a54f3eb5abec42b21)

Scope searched: gates (py, ps1, README, settings.fragment.json, tests), tools/session_cards, agents, model-postures, ~/.claude CLAUDE.md, live settings.json, Hydra CLAUDE.md, docs/agents (with rulings), tools/ch_mcp, memory notes (no archive), hydra plan 2026-11-07-shell-output-to-disk.md. Everything below was read in the files named; nothing is graded.

Rulings read first: plan hooks-from-scratch line 108 (numbers live at the top of progress.py; docs quote them; a test greps the docs); Confirmed lines of review-and-hooks.md (lines 8-34) and agents-and-workflow.md (lines 8-29). The only Confirmed lines that touch my rows are named under the rows. The three claimed "user rulings" that the code and README quote are NOT under Confirmed (see S-A below).

Tests that hold homes equal (found, by name): test_docs_numbers.py (gates/tests) holds progress.STATUS_PREFIX equal to the Hydra preamble (line 189), bash_windows_rule.MAX_CHARS and shell_output.CAP to the preamble's "Shells on Windows" section (lines 192-202), live settings.json to settings.fragment.json (line 142). No test holds anything else below equal; where a row has no test I say so.

---------------------------------------------------------------
## Group 1. The status call (I1-019, I1-023, I1-024, I1-094) and the report tools (I1-017)

### I1-019 The exact status-helper call shape
Homes:
- progress.py:21-27 `STATUS_PREFIX = '& "C:/Users/Patrick/.claude/gates/status_append.ps1"'` plus the `_STATUS_CALL` regex; progress.py:58 `_helper_line` builds the text the hook prints back (`... "HH:MM done ... | next: ..."`).
- settings.fragment.json:4 and live settings.json:10 `PowerShell(& "C:/Users/Patrick/.claude/gates/status_append.ps1"*)` (the allow rule). progress.py:21 says its prefix is "spelled as the allow rule in settings.fragment.json names it".
- Hydra docs/agents/brief-preamble.md:15 the same prefix plus `<your agent id> "<line>"`; preamble:19 says "The status-call check in progress.py owns the exact shape" (and also states the shape at :15, so it is a home, not a pointer).
- docs/agents/doc-review.md:9 and docs/agents/derive-once-review.md:9: `Use & "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>" ...`.
- tools/session_cards/writer-brief.md:100: `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "HH:MM done <n> of <m> cards | next: <session>"` and "(not Bash)".
- gates/README.md:65 (what the helper does, not the shape).
- memory every-brief-carries-status-rule.md:15 (long line; names status_append.ps1).
- status_append.ps1:3-5 (comment, see I1-094).
- tests/replay.py:280 has its own regex of the call shape for old hooks too (`\s*&\s*"C:/Users/Patrick/\.claude/(hooks|gates)/status_append\.ps1"`), a test-side copy of the prefix.
What each home tells its reader:
| home | reader | tells |
|---|---|---|
| progress.py | hook | exact text only; quote or bare id; line bars `"`, `$`, backtick, CR/LF, curly quotes |
| settings allow | permission engine | any text after the prefix is allowed (`*`), so the hook is the narrower check |
| preamble:15, doc-review:9, derive-once-review:9 | every agent with the preamble | `<your agent id> "<line>"`; preamble adds "alone in its own call" and Windows/PowerShell tool |
| writer-brief.md:100 | card writers (agent type that skips the preamble; the writer brief is what they read) | same call, own line content |
| main session vs agent | main session has no status-call rule (progress.check runs for agents only) | the helper is allowed for both by the settings rule |
| worktree vs main checkout | the prefix is a fixed path under the main ~/.claude/gates; test_progress.py:791 expects a worktree copy of the helper path is NOT a status call | agents in a worktree must still use the main path |
Kinds: K1 (value copied to 6+ places), K6 (replay.py:280 second regex; settings allow rule is a second reader of the same prefix). Held equal by test: preamble (test_docs_numbers.py:189). NOT held equal by any test: settings.fragment.json allow rule vs STATUS_PREFIX; doc-review.md:9, derive-once-review.md:9, writer-brief.md:100.
Splits: none in value today. Shape split: the preamble (:15) tells agents to put the id bare as `<your agent id>`; progress.py accepts bare or quoted; the hook's own printed line quotes the id (`"%s"`). Not a disagreement, two spellings.
Confirmed rulings: agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (2026-09-05), :26 "the status updates should be brief but relayed in plain english". They say status is required, not the call shape.

### I1-023 A status call is never counted; another agent's id is refused and not counted
Homes: progress.py:80-86, 111-115 (the text "This status call names %s, but your agent id is %s, so it is refused and not counted"); preamble:17 ("An exact call under any other id is refused by the progress hook, and the refused call is not counted"); status_append.ps1:38 (refuses a name with no agent record, exit 1); gates/README.md:65 (same refusal, in README words). Two code places decide "is this name a valid agent id": progress.py (compares to the caller's own id) and status_append.ps1:38 (looks for `<name>.json` under state\agents). Preamble:17 states both layers.
Table: hook text tells the agent to write its own line with its own id; preamble tells the same; the helper tells "Use the agent id the progress hook gave you at your first call". Main session: no status call exists for it (progress.check is agent-only), so a main session calling the helper would pass the allow rule and fail only at the helper (no agent record). Card writers: writer-brief.md:100 says "The progress hook gives you your agent id on your first call". Worktree vs main: n/a.
Kinds: K6 (id validity decided in progress.py and in status_append.ps1), K1 (preamble repeats the refusal). Test: test_progress.py and test_status_append.ps1 each test one side; none holds the two equal.

### I1-024 Any other shell call naming the helper is refused and counted
Homes: progress.py:123-128 text "This call names the status helper but is not a status call ... Instead: write the status line alone in its own call, exactly: ..."; preamble:19 (the same rule, plus "unless the command starts with `git`" and the `powershell -File` example). The hook exempts commands starting `git ` (progress.py:119); preamble:19 says "unless the command starts with `git`". Agree. Kind: K1/K5 (preamble spells a hook rule out and also says progress.py owns it).

### I1-094 status_append.ps1: why a script, call shape home
Homes: status_append.ps1:3-5 "one allow rule covers running this script. The call shape: the brief preamble's 'Status lines' section." That says the preamble owns the shape; progress.py:21-27 says progress.py owns the text it reads; preamble:19 says progress.py owns it. Three owner claims for one shape.
Split: status_append.ps1:5 says the home is the preamble; preamble:19 and progress.py:21 say the home is progress.py. Kind: K8 (conflicting owner claims) and K1.

### I1-017 Tools that always pass so a report can be handed back
Homes: progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; percall.py:102 literal `"SubagentHandback"` (the save of the hand-back text; second code spelling of one of the two names) while percall.py:168 and the pass in progress.py:119 use REPORT_TOOLS; gates/README.md:11 (long line: "In a subagent it first saves a `SubagentHandback` call's report"); agents/reviewer.md:25 "Hand it back with the `SubagentHandback` tool when you have it, or as your last message when you do not"; agent_stop.py:40 docstring; the progress.py:129-133 refusal text says "Only your final report, git commands and your status line get through now"; preamble:21 "Two kinds of call still pass then: a command that starts with `git`, and your final report."
Table: at the limit (calls >= 150 or minutes >= 30): hook lets report tools and `git ...` shell commands through; preamble says the same; reviewer.md says hand back with the tool or as last message (an agent without the tool). Main session: no budget, no report-tool rule. Card writers: writer-brief (not read in full by me) handled status only.
Kinds: K6 (percall.py:102 literal name vs progress.REPORT_TOOLS), K1. StructuredOutput appears only in progress.py. Test: none holds the literal at percall.py:102 equal to REPORT_TOOLS.

---------------------------------------------------------------
## Group 2. Whole-library flag (I1-004)

Question: which command text marks a whole-library run.
Homes: percall.py:14 `LIBRARY_FLAG = "--whole-library"` (checked at :48); Hydra src/cli/batch_scope.h:10 `kWholeLibraryFlag = "--whole-library"` (the app's own owner, outside the scope but it is the real one); tools/compare_db.py:29 comment "gets --whole-library (src/cli/batch_scope.h owns that rule)" (pointer); gates/README.md:13 ("holds `--whole-library`"); docs/agents/agent-rules.md:14 (names `hydra_batch --whole-library` as an example); brief-preamble.md:49 (`hydra_batch` given no chart folder refuses to run unless it also gets `--whole-library`; "A hook then refuses any shell command that holds `--whole-library`"); docs/agents/this-machine.md:91 (whole-library `--redo` needs `allow library run`); memory no-full-library-runs.md:11 (describes "whole-library tools/compare_db.py check", `hydra_batch` with no arguments).
Table: percall tells main and every agent: refuse unless a typed `allow library run`. Preamble: same plus the app guard. At the edge: a library run that does NOT use the flag (a compare or timing run over the whole library, `hydra_batch` with no folder) is covered by the app's own guard (batch_scope.h) but not by the hook; agent-rules.md:14 and the memory note define the rule by what it covers ("any run over the user's whole song library"), the hook defines it by one flag word. Main vs agent: same check in both. Worktree vs main: n/a.
Kinds: K1 (flag spelled in hook and app), K3 (hook covers one spelling; the rule text covers any whole-library run; memory note lists runs the hook cannot see). Held equal by test: none (test_percall_rewrite.py:73,135 and test_progress.py:425 use the flag as input only; tests/replay.py:297 has a third copy of the rule's words).
Confirmed rulings: review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (2026-10-25).

---------------------------------------------------------------
## Group 3. ch_call, hook crash modes, media ask, rewrite-last (I1-007, I1-009, I1-010, I1-011, I1-049, I1-056, I2-075)

### I1-007 ch_call is judged as the call it forwards; ch_call to ch_call counts as a key call; unreadable args count
Homes: percall.py:58-82 (the code and docstring); gates/README.md:13 (full restatement incl. "a `ch_call` to `ch_call` is refused as one the check does not unwrap"); Hydra tools/ch_mcp/front.py:491-504 `call_shape_refusal`: the Clone Hero server itself refuses ch_call whose `tool` is not a string, whose `args` is not an object, or whose `tool` is `ch_call` or `ch_reload`, with its docstring "The hook in percall.py reads the same two fields, so nothing is coerced here"; clone-hero-mcp.md:11 and :66; server.py:147-148 and the ch MCP instruction text ("a hook refuses them unless the user has typed allow key run").
Table: percall: ch_call to ch_call needs a typed `allow key run` (counted as a key call). front.py: the same call never reaches a worker (refused outright). So a user who types the allow gets it consumed on a call the server would refuse anyway. Main vs agent: same. ch_call with a missing/non-object `args` for a non-key tool: percall passes it (tool not in key sets); front refuses it.
Kinds: K6 (two code places decide what a bad ch_call is: percall._key_call and front.call_shape_refusal), K3 (the hook uses "counts as a key call" where the server uses "refused"). Test: tests/test_percall_key.py:81 pins the hook side only; none holds front.py equal.
Split (own line): percall.py:69-70 treats `ch_call` to `ch_call` as a key call needing an allow; front.py:502-503 refuses the same call outright without any allow.

### I1-009 Media rule in the main session is "ask", not deny
Homes: percall.py:133-134; README:21 "The main session may write under the folder `MEDIA_ROOT` names, and under its scratch folder, with no question; anywhere else, the user is asked. A subagent may write only under its scratch folder, and anything else is refused". Plan docs/superpowers/plans/2026-11-07-shell-output-to-disk.md:80 repeats that an `ask` is never rewritten. Memory ytdl-folders-not-a-project-matter.md and this-machine.md name the media folder (not re-read). Agree. Kind: K1. Test: test_media_rule.py tests the ask path; none holds README equal.

### I1-010 The shell-output rewrite runs last and never sets a permissionDecision
Homes: percall.py:1-6 and 142-158; README:11 and :25 ("the rewrite never sets `permissionDecision`"); plan shell-output-to-disk.md:80 (and "Nothing is approved by this"); shell_output.py:1-6. Agree. Kind: K1. No test holds them equal (test_percall_rewrite.py tests the behaviour).

### I1-011 A crash in the subagent path denies, except for report tools, which pass with a message
Homes: percall.py:165-171,176; README:13 ("A crash in either check refuses the call"), :27, :79, :97; progress.py REPORT_TOOLS. Main session: percall `_main_session` is inside the same closed `common.run`, so a crash there denies; the one carve-out in main is stall (percall.py:135-139, "stall.py crashed and did nothing" message). The report-tool exemption exists only for agents. Kinds: K1. Test: none holds the README sentence equal.

### I1-049 delete_rule: a crash denies
Homes: delete_rule.py:187 `common.run(body, "closed")`; README:97 "A crash denies the call, with the error text in the reason."; the same word is passed at percall.py:176, delete_rule.py:187, publish_gate.py:135, review_dispatch.py:178.

### I1-056 Crash modes: "closed" denies, "loud" prints a systemMessage; one JSON line, exit 0
Homes: common.py:52-75 (the owner); README:97-100 (restates: "A crash denies the call ... A crash prints a `systemMessage` saying the hook did nothing", and non-zero exit being non-blocking); every `common.run(body, "<mode>")` call site (agent_stop.py:297 loud, posture.py:55 loud, waiver.py:50 loud, shell_output.py:201 loud, prompt.py:67 loud, percall/delete_rule/review_dispatch closed); common.py's own refusal text.
Kinds: K1. Test: tests/test_common.py checks the modes in common; no test checks that each script uses the mode the README says.

### I2-075 Which scripts the one prompt hook runs, in order, and each failure mode
Homes: prompt.py:17-18 `PARTS = (("posture", "loud"), ("percall", "closed"), ("waiver", "loud"))` with the comment "each mode is the one its script's __main__ uses"; the real modes are at posture.py:55 `loud`, percall.py:176 `closed`, waiver.py:50 `loud` (they agree today); settings.fragment.json `UserPromptSubmit` registers prompt.py once; README (prompt hook paragraph, restates the three parts and order, line 33 "each part keeps its own crash reply"); tests/test_posture.py:140 docstring "prompt.py runs posture.py, percall.py and waiver.py in one process".
Kinds: K1 now (modes agree), K6 (prompt.py states the mode each script's `__main__` uses instead of reading it). Tests: test_docs_numbers.py:148 holds that one process runs; test_posture.py:251 patches PARTS; NO test holds PARTS' modes equal to each script's `__main__` mode.

---------------------------------------------------------------
## Group 4. The delete rule (I1-031, 034, 035, 036, 037, 038, 040, 041, 042, 043, 044, 045, 046, 047)

### I1-031 Which tools the delete rule judges
Homes: delete_rule.py:168 `("Bash","PowerShell")`; progress.py:20 `SHELL_TOOLS = ("Bash","PowerShell")` (the same pair, a second code home); shell_output.py:35 `TAILS` keys `Bash`/`PowerShell`; media_rule via percall (uses progress.SHELL_TOOLS); bash_windows_rule judges Bash only (:235); README:19 "a subagent's recursive or forced delete"; preamble:43 "A hook checks every recursive or forced delete you run from a shell".
Kinds: K6 (the pair is spelled in delete_rule.py and progress.py, percall.py:112 already passes through progress.SHELL_TOOLS before calling delete_rule.check, so the second spelling inside check is a repeat). Main session: delete rule is agent-only (README:19 says "a subagent's"); settings ask rules cover main (see I1-044).

### I1-034 Which command words count as deletes
Homes: delete_rule.py:19 `{"rm","rmdir","rd","del","erase","remove-item","ri"}`; settings.fragment.json:60,63 and live settings.json "ask" for `Bash(rm *)` and `PowerShell(Remove-Item *)` (main session, only two of the seven words); README:19; preamble:43 ("`rm -rf`, `Remove-Item -Recurse -Force` or `find <paths> -delete`" and "delete_rule.py ... is the one home of which commands and flags count"); media/bash don't read it. test_delete_rule.py pins behaviour.
Table: delete_rule (agent): refuse unless literal absolute path in scratch/worktree. settings ask (main): ask for `rm *` and `Remove-Item *` only; `del`, `rd`, `rmdir`, `erase`, `ri` in main are not asked. Preamble: tells agents the hook is the home. At the limit: a plain `rm` with no force flag passes the hook (only recursive/forced are judged, delete_rule.py:161) while the preamble's headline reads "every recursive or forced delete".
Kinds: K6 (two lists of delete words: delete_rule.DELETE_WORDS vs settings ask list), K3 (the settings list is narrower and has a different actor: ask for main vs refuse for agents). No test holds the settings ask list equal to DELETE_WORDS.

### I1-035 cmd-style delete words and flags; I1-036 rm short options; I1-037 long force flags
Homes: delete_rule.py:20-23, 80-86 only. README:19 describes in words ("recursive or forced"), preamble:43 same. No second code copy found (searched gates/*.py for "/s", "/q", "RM_SHORT", "-recurse"). One code home plus prose pointers that state no list. test_delete_rule.py pins behaviour. Kind: none beyond K1 for the prose "recursive or forced".

### I1-038 What is "not a literal path"
Homes: delete_rule.py:24-26 `NOT_LITERAL`; delete_rule._instead text (:60-63, "no variables, wildcards, '..', quotes or other words after them"); README:19 ("a variable, a wildcard or a `..`"); README:21 ("the same literal-path test delete_rule.py uses"); preamble:43 ("a wildcard or a `..`, a path with a quote left inside it, and a path with a space in it"); media_rule.py:52 (calls dr._inside, so reuses); test_media_rule.py:61.
Table: code refuses on the nine character classes plus space (space is refused only through word splitting, not through NOT_LITERAL); the refusal text lists five; README lists three; preamble lists variables, wildcard, `..`, quote, space. Edge: `,` `%` and `?` are refused by code but named nowhere in prose; a path with a space is named by the preamble only.
Kinds: K1 (prose lists copy part of the list), K3 (space handled by split not by the list; prose treats it as one more class). No test holds the prose lists equal.

### I1-040 find: which tokens end its starting paths, which run a command
Homes: delete_rule.py:27-28, 132-140 only; README:19 and preamble:43 say "a `find` that deletes" without the token lists. Single code home, prose pointers. Search: grep `-exec`, `-delete`, `-execdir` across the scope found only delete_rule.py, bash_windows_rule.py:7 (the phrase "find -exec" as an example of a wrapper) and README:27.

### I1-041 Where a command is cut into pieces
Homes: delete_rule.py:29 `OPERATORS`, :75-77 `pieces()` ("the one reader of where a command is cut"); bash_windows_rule.py:19 `OPERATOR_CHARS = ";|&\r\n"` and its own `_scan` that cuts quote-aware (:5-6 "_scan is the one reader of shell quoting"); media_rule.py:18-27 `_segments` (cuts on newlines and operators standing alone as a word, its own loop, uses dr.OPERATORS.fullmatch); shell_output.py:55-60 (uses delete_rule.pieces); README:19 and preamble ("cuts the command at its shell operators").
Table: delete_rule cuts at operators anywhere, ignoring quotes; bash_windows_rule cuts quote-aware; media_rule cuts at an operator only when it stands alone as a word (so `a;b` inside a filter does not split, while `ffmpeg x;rm y` does not split either); shell_output reuses delete_rule. Edge: a command like `echo "a;b" && cmake x` is cut inside the quotes by delete_rule/shell_output and not by the other two.
Kinds: K6 (three cutters; delete_rule's own docstring calls pieces the one reader), K3. Split: delete_rule.py:76 says `pieces()` is the one reader of where a command is cut; bash_windows_rule.py:19-20,5-6 and media_rule.py:18-27 each cut commands their own way. Test: none holds them equal (test_media_rule.py:164 and test_shell_output.py test each separately).

### I1-042 Which command word a token names
Homes: delete_rule.py:66-72 `word()` and alias `_word`; media_rule.py:14-15 `_word` (strips quotes first, then dr._word); shell_output.py:45-60 `_unquoted` + `delete_rule.word` (strips a quoted first word, rejoins a space-split quoted word; the comment says "This lives here and not in delete_rule.word, because that rule leaves a quoted delete unread on purpose"); bash_windows_rule.py:137-143 `_command_at` (calls `delete_rule._word` but refuses a first word that carries a quote); progress.py: `command.startswith("git ")` (a fifth reader of "is this a git command"); test_shell_output.py:99-100 pins that delete_rule.word('"C:/tools/cmake.exe"') returns 'cmake.exe"'.
Table: for a quoted command word like `"C:/tools/cmake.exe"`: delete_rule does NOT read it as a command (leaves it); shell_output reads it as `cmake` and adds the exit-code tail; media_rule reads it (strips quotes) and guards ffmpeg; bash_windows_rule does not read it. One question ("what command is this piece"), four answers on a quoted word.
Kinds: K6, K3. Split: delete_rule.py:2-6 and shell_output.py:47 say a quoted delete word is left unread on purpose; media_rule.py:14 reads a quoted tool path as the tool. Tests: test_shell_output.py:99-100 holds delete_rule's side only.

### I1-043 git options that take a value
Homes: delete_rule.py:30 `GIT_VALUE_OPTIONS = {"-c","-C","--git-dir","--work-tree","--namespace","--config-env","--attr-source"}` with `git_words` "the one reader of those" (:105-111); bash_windows_rule.py:25 `GIT_OPTIONS_WITH_VALUE = ("-C","-c")` read by `_is_git_commit` (:146-153); shell_output.py:130-132 reuses delete_rule.git_words; README:19 mentions reading "past git's own options" for NO_TRIM_COMMANDS (README:23).
Split (own line): delete_rule.py:30 lists seven value-taking git options; bash_windows_rule.py:25 lists two. `git --git-dir x commit -m <long text>` is a commit to delete_rule.git_words but not to bash_windows_rule._is_git_commit, so its text counts toward the 1,000-character limit there. Kinds: K6, K2. Test: none holds the two lists equal.

### I1-044 Which git commands are refused to agents
Homes: delete_rule.py:31-35 `GIT_DISCARD_WORDS`, `GIT_DISCARD_ACTIONS`, `git clean` always, `branch -d` with `-f` (:114-129); README:19 (a plain-words list, and "`WORKTREE_FOLDERS`, `GIT_DISCARD_WORDS` and `GIT_DISCARD_ACTIONS` in that file are the one list"); preamble:43 ("such as `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune` and `git stash drop`"); settings.fragment.json:61-62 and live settings.json:71-72 "ask" for `Bash(git clean *)` and `Bash(git reset --hard*)` (main session); delete_rule._instead text :62; cleanup_sweep.ps1:25 ("never runs `git worktree prune`"); memory git-traps-in-shared-checkout.md (title only; not read in full).
Table: agent: refused outright (git clean, reset --hard/--merge, checkout -f/--force/., restore ., switch -f/--discard-changes, branch -D, stash drop/clear, worktree remove/prune, branch -d -f). Main session: only `git clean *` and `git reset --hard*` ask. At the limit: `git branch -d` alone passes; `git branch -d -f` refused (README adds it, the preamble does not name it).
Kinds: K6 (two code-side lists: delete_rule and the settings ask rules, for different actors), K1 (README and preamble list part of it). Test: none holds settings ask vs GIT_DISCARD_*; test_delete_rule.py:266 pins one case.

### I1-045 Refusal text for git discards; I1-046 Text telling the agent how to delete
Homes: delete_rule.py:157, 58-63. The preamble:43 repeats the same advice ("delete only what you made, by literal absolute paths" and names the scratch folder and the two `worktrees` folders); the hook's text names the worktree folders from WORKTREE_FOLDERS (delete_rule.py:17-18) while the preamble and README:19 and derive-once-review.md:197 each write the two folder paths out.
Kinds: K1 (WORKTREE_FOLDERS spelled in delete_rule.py, README:19 (by name only), preamble:43, derive-once-review.md:197). Preamble:43 states the "worktree" rule as "any worktree directly inside a `worktrees` folder of either repo (`...hydra-test\.claude\worktrees\` or `...\.claude\worktrees\`)". See side note SN-1 on this session's own worktree path.

### I1-047 Delete judged only when the delete word is the first word of a piece; quoted or wrapped not read
Homes: delete_rule.py:1-7 ("dumb and strict on purpose"); README:19 (same, with a quote attributed to the user: "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check."); preamble:43 (same: "A delete inside quotes, behind `bash -c`, `xargs` or `sudo`, or spelled any other way is not read"); bash_windows_rule.py:7 and README:27 ("not read, on purpose, as in delete_rule.py"). Kind: K1/K7. A way past this rule is stated in the docstring itself (quoted and wrapped forms are not read).
Confirmed: none found for the quoted words. The "losing battle" words are in docs/sessions/2026-10-31-6777b546.md:25 and two plans (2026-11-07-ch-run-timing.md:31, 2026-11-07-build-limit.md:130); the 2026-10-31 Confirmed line in review-and-hooks.md:11 reads differently ("im not too concerned about the exact number as much as efficiency and functionality ..."). See S-A.

---------------------------------------------------------------
## Group 5. shell_output (I1-062, 063, 064, 065, 069, 070, 071, 071a)

Owner statement (plan shell-output-to-disk.md:76): "`gates\shell_output.py` is the one home ... Docs point at the module and never restate the lists." README:23-25 and preamble:35 also point there.

### I1-062 Error lines kept from the middle, their room; error words
Homes: shell_output.py:17-19; plan shell-output-to-disk.md:142 ("up to 10 lines holding any of `error`, `failed`, `exception`, `traceback`, `fatal`, `assert`, case-insensitive") and :76 (owner names); README:23 (says "its error lines", no values). The plan states the whole list, so it is a home. Kind: K1. No test holds the plan equal (test_shell_output.py:219 pins each word).

### I1-063 Which commands get the exit-code tail
Homes: shell_output.py:20-22 `BUILD_TEST_COMMANDS`; plan :144 (full list, also "taken from `build_cpp.ps1`'s header, `docs/development.md` ... and `gates\README.md`"); README:25 (names the constants only); preamble:35 ("A build or test command gets one line appended"; names `hydra_tests` etc. nowhere in that line); preamble:47 and CLAUDE.md rule 1/14 and agent-rules.md rule 14, README:17, model-postures.md:9, agents/reviewer.md:21, agents/executor.md:11 all name the test programs `hydra_tests.exe`, `hydra_uitest.exe`, `hydra_replay.exe`, `hydra_precheck.exe` as the programs agents may not run (precheck allowed from main). The Hydra root build_cpp.ps1:4-9 header names the same programs; tools/build_slot.ps1 exists.
Table: code adds the tail to all twelve names, including `hydra_precheck` (an agent may run it) and `mutation_probe.ps1`. Rule texts split the same programs into "agents may not run" (tests, uitest, replay) and "agents may run" (precheck).
Finding inside this row: `mutation_probe.ps1` is in the list but tools/mutation_probe.ps1 does not exist on the main checkout (it is found only in old worktrees under hydra-test\worktrees; the plan at :144 says "(removed; `git show ed0aa857:tools/mutation_probe.ps1`)"). Kind: K8-like stale entry, K1.
Test: none holds the list equal to any doc.

### I1-064 Python launchers and test flags; I1-065 Commands never trimmed
Homes: shell_output.py:23-25; plan :144 (launchers, flags), :146 (`git diff`, `git show`, "judged by the first piece's first two words"); README:23 ("`NO_TRIM_COMMANDS` names the commands whose output arrives whole, read past git's own options") and :25 ("`BUILD_TEST_COMMANDS` and `PYTHON_TEST_FLAGS` say which commands count"); preamble:35 ("`shell_output.py` owns the number and the lists of which commands get that line or are never trimmed"). The preamble says "A build or test command gets one line appended" but gives no list. Kind: K1 (plan holds the values). No test.

### I1-069 The tail text per shell tool
Homes: shell_output.py:33-38; plan (design text, not re-read); README:25 describes in words ("prints its real exit code, and the command itself then exits 0"); preamble:35 ("A build or test command gets one line appended that prints its real exit code ... never end one with your own `exit N`") and tells readers the rewritten line `[real exit code N]` is not theirs to write. Kind: K1. Test: test_shell_output.py exercises both tails.

### I1-070 No rewrite for a background call, a status call, or a command already ending with the tail
Homes: shell_output.py:75-79; plan :82 (same three cases, in the plan's words). Kind: K1. Status call: progress.is_status_call (a second code place, called, not copied). No test holds the plan equal.

### I1-071 A result Claude Code already saved is not trimmed unless the command failed; a trim that does not shorten is dropped
Homes: shell_output.py:179-189; README:23 ("A trim that would come out no shorter than the text is dropped"; "A result already past Claude Code's ceiling is left alone"). Kind: K1.

### I1-071a Trim never reads a quoted-first-word delete as a command; unquoting lives in shell_output not in delete_rule.word
Homes: shell_output.py:45-52; delete_rule.py:2-6 (quoted delete not read); media_rule.py:14-15; test_shell_output.py:99-100. The comment names where the code lives. See I1-042 for the four readers. Kind: K6 (as I1-042).

---------------------------------------------------------------
## Group 6. common.py (see I1-056 above)

---------------------------------------------------------------
## Group 7. bash_windows_rule (I1-073, 074, 075, 077, 078, 079, 080, 081, 083, 084)

### I1-073 git options with a value (for finding `git commit`)
See I1-043; split with delete_rule.GIT_VALUE_OPTIONS recorded there.

### I1-074 Word that stops Git Bash rewriting paths
Homes: bash_windows_rule.py:17 `PATHCONV_OFF = "MSYS_NO_PATHCONV=1"` (used in the refusal text :221 so the text follows the constant); README:27 ("`MSYS_NO_PATHCONV=1` in the same part of the command lets the third and fourth through"); preamble:33 ("put `MSYS_NO_PATHCONV=1` before it" for cmd); the preamble does not say the word also lets a `ref:.dotpath` argument through, while README and code do. Kinds: K1; K3 (the preamble gives the word only as the fix for cmd, the code accepts it for two shapes). Test: none holds preamble equal (test_docs_numbers.py:192-196 holds only the 1,000).

### I1-075 The five refused shapes and their tags
Homes: bash_windows_rule.py:209-230; README:27 ("five parts", "Each refusal starts with a tag naming its part"); preamble:33 ("A hook refuses five shapes in a Bash call" and each shape in words); tests/replay.py:293,431 read `TAGS` (reads the owner, not a copy). Preamble's five and the code's five agree today. Kinds: K1. No test holds the preamble's list to PARTS.

### I1-077 A doubled backslash passes in double quotes and in a comment; refused in bare, single-quoted, heredoc
Homes: bash_windows_rule.py:26, 177-180 (docstring says "(the user's ruling)"); README:27 ("a pair in double quotes passes"); preamble:33 ("A doubled backslash outside double quotes: put the text in a file ... or use forward slashes"); the refusal text :216-218 ("in single quotes, a heredoc or a bare word"). A pair in a comment also passes in code but is named by neither README nor preamble.
Split: preamble:33 says a doubled backslash "outside double quotes" is refused, which would include a comment; the code passes a comment (bash_windows_rule.py:177-180). Kind: K2 (small), K8 (the "user's ruling" cited in the docstring is not under Confirmed; see S-A).
Confirmed: review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too" (2026-11-06) covers the cmd, ref and path-quote shapes. Nothing Confirmed speaks to the pass in double quotes or in a comment.

### I1-078 `cmd` with a `/x` flag refused unless the piece holds PATHCONV_OFF; `//x` and a redirect target pass
Homes: bash_windows_rule.py:188-198; README:27 (third part); preamble:33 ("`cmd` with a slash flag: run it from the PowerShell tool, or put `MSYS_NO_PATHCONV=1` before it"); tests/probe_results.md (measurement). The `//x` and redirect passes are in code and the code docstring only. Kind: K1. Confirmed: review-and-hooks.md:10 (above).

### I1-079 A `ref:.dotpath` argument is refused: pattern
Homes: bash_windows_rule.py:27, 201-202 `REF_DOTPATH`; refusal text :222-224 (`origin/main:.gitignore` example); README:27 ("a whole word shaped like `origin/main:.gitignore`"); preamble:33 ("A ref with a slash before `:.` and a dotted path, such as `origin/main:.gitignore`"). The pattern needs a letter right after `:.`; the preamble says "a dotted path". Kinds: K1, K3 (small: preamble's words are looser). Confirmed: review-and-hooks.md:10.

### I1-080 An open double quote starting a Windows path that ends in a backslash is refused
Homes: bash_windows_rule.py:28, 205-206 `PATH_QUOTE`; refusal text :226-228; README:27 (fifth part); preamble:33 ("A Windows path ending in a backslash just before its closing double quote"). Kind: K1. Confirmed: review-and-hooks.md:10.

### I1-081 Judges the Bash tool only; PowerShell never read; shapes behind wrappers not read on purpose
Homes: bash_windows_rule.py:1-8, 235; README:27 ("It reads the Bash tool only"; wrappers "not read, on purpose"); preamble:31-33 ("Run commands with the PowerShell tool" and a hook "refuses five shapes in a Bash call"). A way past this rule exists in general words: wrapped or longer-quoted forms are not read (stated in the docs themselves).

### I1-083 Refusal texts (the fixes)
Homes: bash_windows_rule.py:211-228; preamble:33 (the same five fixes in the same words: Write tool and run the file; forward slashes; PowerShell tool or `MSYS_NO_PATHCONV=1`; ref with no slash; drop the trailing backslash). Preamble:33 also covers the length rule's other fix ("split the command" is only in the hook's text). Kind: K1/K5 (preamble spells out what the hook prints each time).

### I1-084 Heredoc and assignment patterns
Homes: bash_windows_rule.py:23-24 only (searched gates for `<<` and `ASSIGNMENT`; shell_output and delete_rule have no heredoc reader). One home; delete_rule.pieces and media_rule._segments split heredoc bodies on operators without knowing heredocs. Kind: K6 via I1-041.

### I1 length rule (not my row; recorded because I1-075 depends on it)
`MAX_CHARS = 1000`: bash_windows_rule.py:16; preamble:33 (1,000, held by test_docs_numbers.py:192-196); README:27 (names the constant, no number); Confirmed? Not under Confirmed: the "length over 1000 and backslashes" line is Unconfirmed (review-and-hooks.md:43).

---------------------------------------------------------------
## Group 8. media_rule (I1-086, 087, 088, 089)

Homes: media_rule.py:7-11 (tools, reads, output options, path shape); README:21 ("Which words count as a write is decided in that file alone"). Search for ffmpeg, yt-dlp, yt_dlp, ytdl across gates, agents, model-postures, CLAUDE.md, tools/session_cards, settings, Hydra docs/agents found only media_rule.py and README:21. Hydra tools such as video scripts use ffmpeg but do not state the guard's lists (not opened). Single home plus a pointer. Hydra docs/agents/this-machine.md and the memory note about the media folder name `C:\ytdl` (MEDIA_ROOT is not my row).
Kind: single home. Test: test_media_rule.py pins behaviour.
A hook gap noted in general words: quoted or wrapped media commands are not read (stated in media_rule.py:1-2).

---------------------------------------------------------------
## Group 9. I2-046 and I2-109

### I2-046 How a background task start and end are recognised
Homes: agent_stop.py:44-53 (`_BACKGROUND_STARTED`, `_TASK_ENDED`, `_STILL_RUNNING`), comment "as read from real agent transcripts (gates/README.md, the agent_stop paragraph)"; README:41 describes the idea (not the strings); tools/session_cards/common.py:66-71 (`TASK_NOTICE`, `NOTICE_TOOL_USE`, `NOTICE_AGENT_COUNT`, `DROPPED_HUMAN_PREFIXES = ("<task-notification>", ...)`), which reads the same `<task-notification>` shape for a different job (stripping notices from cards); tests/test_agent_stop.py:121 builds a task-notification record (origin kind "task-notification"); tools/session_cards/test_session_cards.py:699-702 builds another; review_rules.py:95-103 decides "is this a human prompt" by `"human"` origin (a third way to tell a notice from typed text); the comment at agent_stop.py:46 says the home of the strings is the README, but the README does not hold them.
Kinds: K6 (two regexes for the same `<task-notification>` block in agent_stop.py and session_cards/common.py; a third rule in review_rules), K8 (agent_stop.py:46 names README as the source and README holds no strings). Test: none holds the two regexes equal.
No Confirmed ruling found.

### I2-109 What the session-card start hook runs and in what order
Homes: session_cards_start.ps1:10-17 and :327-353 (prepass, then batch [field `sessions=`], then prune_list [field `mb=`], each `py -I -X utf8 <script>`; `--out`/`--cards-root` from -DataDir/-CardsDir); its header :7-8 "Its module docstrings are the spec of each script; this hook only runs them and reads the one summary line each prints"; batch.py:75 prints `sessions=%d batches=%d waves=%d`; prune_list.py:147 prints `paths=%d held_back=%d mb=%.1f`; tests/test_h53_session_cards_start.ps1:57-69 builds FAKE scripts that print `sessions=%d batches=%d waves=1` and `paths=2 held_back=0 mb=...` and :121 asserts the order `prepass,batch,prune_list`; tools/session_cards/test_session_cards.py:668 asserts the real batch.py prints `sessions=0 batches=0`; Hydra CLAUDE.md "Past sessions" ("A start-of-session hook asks the main session to write cards for finished sessions that have none").
Also in the hook's text (Get-CardNote, :269-290): the commit steps are spelled out (stage by name, never `git add -A`, trailers Task/Agent/Session, "A commit that changes only docs/sessions needs no reviewer"), the docs-check command `hydra_tests.exe "-tc=docs match code*"` (also in Hydra CLAUDE.md), check.py, index.py, link_memory.py order, and the comment :267-268 says the commit step "relies on gates\review_rules.py explain_main_move". The hook text tells the main session to "Run them in the background", while the preamble tells agents never to use run_in_background (different actor: main).
Table: hook (main session at SessionStart): run the three scripts; if sessions wait, claim and print writer instructions; prune reminder. Main vs agent: only a main session gets the instruction; card writers (agent type that skips the preamble) read writer-brief.md and are told status shape separately (writer-brief.md:100). Worktree vs main: Get-CardNote commits "on main in the main checkout"; Hydra CLAUDE.md rule on code-on-a-branch does not apply because docs/sessions only commits need no reviewer; the "Derive-once review" paragraph in CLAUDE.md says merges and commits on main that touch code need review.
Kinds: K6 (summary-line format `sessions=`/`mb=` is spelled in each script and in the fake scripts of the hook test; no test runs the real scripts through the hook), K5 (the hook text spells out commit rules that the ref gate enforces: trailers), K1 (docs-check command in two places). Tests: test_h53 holds hook vs its own fake format; test_session_cards.py:668 holds real batch.py's `sessions=`; nothing holds the two formats equal.

---------------------------------------------------------------
## Splits (each on its own line)
- SP1 I1-043/073: delete_rule.py:30 lists seven value-taking git options; bash_windows_rule.py:25 lists two.
- SP2 I1-041: delete_rule.py:76 calls `pieces()` "the one reader of where a command is cut"; bash_windows_rule.py:19-20 (`OPERATOR_CHARS`, quote-aware `_scan`) and media_rule.py:18-27 (`_segments`) each cut commands their own way.
- SP3 I1-042/071a: delete_rule.py:2-6 and shell_output.py:47 leave a quoted command word unread on purpose; shell_output.py:45-60 reads it for the exit-code tail and media_rule.py:14-15 reads it for the media guard.
- SP4 I1-094/I1-019: status_append.ps1:5 names the preamble as home of the call shape; progress.py:21 and preamble:19 name progress.py.
- SP5 I1-007: percall.py:69-70 counts a `ch_call` to `ch_call` as a key call that needs an allow; tools/ch_mcp/front.py:502-503 refuses it outright.
- SP6 I1-077: preamble:33 "outside double quotes" vs code passing a doubled backslash in a comment (bash_windows_rule.py:177-180).
- SP7 I1-034/044: settings ask rules for main session cover only `rm *`, `Remove-Item *`, `git clean *`, `git reset --hard*`; delete_rule refuses a wider list to agents. Different actors; the same question ("which deletes and discards need a person's yes") answered from two lists.

## Confirmed ruling lines found that speak to my rows
- review-and-hooks.md:10 "why did you only show me those two if there are more important patterns? yes block those too" (2026-11-06, session 95a8b349), speaks to I1-078/079/080.
- review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (2026-10-25), speaks to I1-004.
- review-and-hooks.md:17 chose "Keep rule, move worktrees (Recommended)" to a question on the delete rule's roots (2026-10-25, session 967c59de), speaks to WORKTREE_FOLDERS and I1-046.
- agents-and-workflow.md:25,26 status updates (2026-09-05), speaks to I1-019/023/024.
- agents-and-workflow.md:8,9 builds only by main session (2026-11-07), speaks to the test-program lists in I1-063.
- review-and-hooks.md:11 "... if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized" (2026-10-31), speaks generally to the delete/shell lists.

## S-A. Quoted "user rulings" that are not under Confirmed
- bash_windows_rule.py:173 "A commit's own text does not count (the user's ruling)", :178 "(the user's ruling)", README:27 "the user ruled on 2026-11-06 'Exempt git commit'": that text is under `## Unconfirmed` (review-and-hooks.md:40,43).
- README.md:19 attributes "trying to ban specific command patterns is a losing battle ..." to a user ruling on 2026-10-31: found only in docs/sessions/2026-10-31-6777b546.md:25 and plans, not under Confirmed.
- README.md:21 says the media guard is "D189, brought back by D273" (decision log; not opened by me).

## Side notes
- SN-1 delete_rule.py:17-18 and preamble:43 name `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees` as the worktree root. This session's worktree is `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records` (and the glob above found dozens of worktrees under `hydra-test\worktrees\`), which is neither listed root, so an agent working there cannot delete inside it under the rule as written. Same path claim in derive-once-review.md:197 and README:19.
- SN-2 progress.py:21 comment points at settings.fragment.json for the prefix; test_docs_numbers.py:142 holds live settings equal to the fragment but nothing holds the fragment's allow rule equal to STATUS_PREFIX.
- SN-3 prompt.py runs `percall` at UserPromptSubmit in "closed" mode with `body(data, now, root)`; the README prompt paragraph (line 33) is the only prose home I read for it.
- SN-4 settings.json "ask" rules for `rm *`, `git clean *`, `git reset --hard*`, `Remove-Item *` mean the main session is asked on these, but the user-facing text of the preamble names only the agent side.
- SN-5 percall.py:19 comment "Hydra's tools/build_slot.ps1 owns what a pass allows" and README:15 "Hydra's `tools/build_slot.ps1` owns the limits on it": tools/build_slot.ps1 exists on main (checked by glob); I did not read it, so whether it owns the pass rules is unchecked.

## Rows not reached
None skipped. Depth is thin for I1-035/036/037/040/084/086-089 (single code home confirmed by search, no second home found) and for I1-069/071 where only README, plan and preamble were checked, not tests line by line.
