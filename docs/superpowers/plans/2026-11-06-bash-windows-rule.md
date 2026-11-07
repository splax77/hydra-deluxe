# Plan: a hook rule that refuses Bash commands that fail on Windows

Date: 2026-11-06. Planner: a59fd5a52518d5b92 (Fable). Task: bash-windows-rule-plan. Status: waiting for the user's yes. Nothing here is built.

## What the user asked for

The user wants a hook that stops Claude attempting Bash tool commands that will fail on Windows. Their ruling, typed word for word, was "length over 1000 and backslashes". When asked whether three more shapes should go in the hook as well, they typed "yes block those too". The main session reads those two answers as the five parts below. This plan states that reading plainly so the user can correct it at approval.

Only the Bash tool is read. The PowerShell tool has none of these problems (backslashes are plain characters there, and `cmd /c` runs as typed), so the rule never looks at a PowerShell call.

## The rule in one paragraph

A new module, `gates/bash_windows_rule.py`, has one function `check(data)` that returns a refusal reason or `None`. `percall.py` calls it for every Bash call, in the main session and in every subagent, the same way it already calls `delete_rule.check`. There is no second `*` registration (the README's "How a hook is registered" says a new per-call rule becomes a module `percall.py` calls). The module runs with the other refusals, so a crash in it denies the call with the error in the reason: that is the README's `closed` mode, and `common.run` already does it for `percall.py`. The refusal always ends with an `Instead:` line that says what to do instead. The five parts:

1. **Length.** `len(command) > MAX_CHARS`, with `MAX_CHARS = 1000` as the first constant in the module. Raw character count, no weighting: 1,000 passes, 1,001 is refused. The user saw the measured 7,600 option and chose 1,000, so nobody swaps in a higher number. Instead: put the content in a file with the Write tool and run that file (`python -I <file>`, `bash <file>`), or split the command. A long `git commit` is refused like anything else; the agent writes the body to a scratch file with Write and commits with `git commit -F <file> --trailer ...` (a file, not stdin; the preamble forbids `-F -`). In the last 14 days no `git commit` call passed 1,000 characters, and only 12 `git` calls of any kind did, so this bites almost never. An agent that is already over budget cannot call Write, but it also cannot have written a 1,000-character commit yet; it shortens the body.

2. **Doubled backslashes.** A `\\` pair whose quoting is single quotes, a heredoc body, or a bare word is refused. In those three places Claude Code halves the pair before bash sees it, so the text is silently changed: `'x\\y'` reaches bash as `x\y`, a heredoc line `b:\\` is written as `b:\`, and bare `a\\b` becomes `ab`. A pair inside double quotes passes (open question 1). Instead: write the text to a file with the Write tool, or use forward slashes in paths.

3. **`cmd` with a slash flag.** A piece of the command whose first word is `cmd` or `cmd.exe` (by its last path part, lower-cased, as `delete_rule._word` reads a command word) and that has a later word starting with `/`, unless that same piece holds `MSYS_NO_PATHCONV=1`. Git Bash rewrites `/c` as a Windows path, `cmd` gets no command to run and opens an interactive prompt that sits until the tool's 120-second timeout. Measured today: `cmd.exe /d /c "echo hi"` printed the cmd banner and never `hi`; with `MSYS_NO_PATHCONV=1` in front it printed `hi`. Instead: run it from the PowerShell tool, or put `MSYS_NO_PATHCONV=1` before it. Only `cmd` is listed; open question 2 asks about other slash-flag tools.

4. **A `<ref/with/slash>:.dotpath` argument.** A bare or quoted word matching `<word>/<...>:.<letter>`, such as `origin/main:.gitignore`. Git Bash rewrites it as a Windows path: measured today, `git show 'origin/main:.gitignore'` failed with `fatal: ambiguous argument 'origin\main;.gitignore'` while `main:.gitignore` worked. Words inside a heredoc body are not read for this part. Instead: name a ref with no slash (`main:.gitignore`), or run the git command from the PowerShell tool.

5. **A Windows path ending in a backslash before a closing double quote.** A double-quoted word that starts with a drive letter and ends `\"`, as in `ls "C:\Users\Patrick\"`. Bash reads `\"` as an escaped quote, so the quote never closes; measured today it failed with `unexpected EOF while looking for matching`. Instead: drop the trailing backslash, or write the path with forward slashes.

Each refusal reason starts with a fixed tag naming its part (`[length]`, `[backslashes]`, `[cmd]`, `[ref]`, `[path quote]`), so the proof step can count refusals per part by that tag and the refusal log keeps it in its first 300 characters.

## How the quoting check stays small

Two rulings bear on this. The user said on 2026-10-31 that "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check." The harness plan of the same day says a hook "never parses free text when a chokepoint exists." The old path gate (session 2026-10-06) matched path-looking text without knowing how it was quoted, and it would have refused 39 ordinary commands.

Here there is no chokepoint: the command string is the only place the fact exists, so the rule reads it. It keeps that reading small in three ways. First, it looks for five fixed shapes and nothing else; the user named each one. Second, parts 2, 4 and 5 all need to know the quoting, and one scanner answers for all three. It is a small state machine (about forty lines) that walks the command once and labels each character as bare, single-quoted, double-quoted or heredoc body, reading `\x` as one escaped character outside single quotes and a heredoc as starting at `<<WORD` and ending at a line that is exactly `WORD`. Part 2 asks it where each `\\` sits, part 4 asks it for the bare and quoted words, part 5 asks it which double-quoted words start with a drive letter and end in `\"`. Nobody writes a second scanner. Third, the scanner has one fixed answer for every input, so there is no "cannot read" case: a quote that never closes means the rest of the command is inside that quote, and a heredoc whose end word never comes means the rest is heredoc body. Bash itself reads an unclosed quote the same way, then refuses the command. A crash anywhere in the module denies the call, per `closed`.

The status helper call is never read: `percall._subagent` already skips every shell rule for a call `progress.is_status_call` recognises, and the new call sits inside that same `if`. The main session makes no status calls.

## Tasks

### T1: the module and its tests

In a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\bash-windows-rule` (a folder `delete_rule.WORKTREE_FOLDERS` allows), on branch `bash-windows-rule`, one executor writes `gates/bash_windows_rule.py` with `MAX_CHARS`, the scanner, the five parts and `check(data)`, and wires it into `percall.py`: in `_subagent`, into the existing `reason = delete_rule.check(data) or ...` chain; in `_main_session`, into the `reason = _library_refusal(...) or _key_refusal(...)` line, which sits outside the `stall` try so a crash denies. Both sides deny; the command would fail anyway, so there is nothing to ask. The module's docstring names this plan and says the scanner is the one quoting reader.

Tests go in `gates/tests/test_bash_windows_rule.py`, run with the README's command and `-p test_bash_windows_rule.py`. Acceptance criteria, each one a test:

- `test_length_boundary`: a 1,000-character command passes, a 1,001-character one is refused with the `[length]` tag; the test builds both from `MAX_CHARS` so it fails if the number moves.
- `test_length_is_raw_len`: a 1,001-character command made of multibyte characters is refused (raw `len`, no weighting).
- `test_long_git_commit_refused_with_file_advice`: a `git commit -m` over 1,000 characters is refused and the reason names `git commit -F` and the Write tool.
- `test_status_call_passes_through_percall`: a status helper line passes `percall._subagent` with the rule wired in.
- `test_powershell_tool_not_read`: a PowerShell call of 1,500 characters holding `\\`, `cmd /c` and a trailing `\"` passes.
- `test_backslashes_single_heredoc_bare_refused`: three commands, one pair in each quoting, each refused with `[backslashes]`.
- `test_backslashes_double_pass`: `"a\\b"` passes (open question 1 may change this test's expected value, and the test says so in its name only).
- `test_unclosed_quote_is_inside`: a pair after an unclosed single quote is refused; a pair after an unclosed double quote passes.
- `test_cmd_slash_refused_and_pathconv_passes`: `cmd.exe /d /c "echo hi"` refused with `[cmd]`; the same with `MSYS_NO_PATHCONV=1` in front passes; `cmd` with no slash word passes; a `/c` word inside a heredoc passes.
- `test_ref_dotpath`: `git show 'origin/main:.gitignore'` refused with `[ref]`; `git show main:.gitignore` passes; the same text inside a heredoc passes.
- `test_trailing_backslash_quote`: `ls "C:\Users\Patrick\"` refused with `[path quote]`; `ls "C:/Users/Patrick/"` and `ls "C:\Users\Patrick"` pass.
- `test_every_refusal_has_instead`: each part's reason holds `Instead:` and its tag.
- `test_crash_denies`: a `tool_input.command` that is not a string makes `percall.body` deny through `common.run` in `closed` mode, in the main session and in a subagent.
- `test_scanner_is_the_one_reader`: the module defines exactly one function that walks quoting, and `check` reaches each part through it (a structural test: parts 2, 4 and 5 are called with the scanner's output, never the raw string).

### T2: the proof over past calls

Same executor, same branch. `gates/tests/replay.py` judges a subagent's shell call through `percall.body` but a main-session call only through `percall._library_refusal`, so today it would miss the new rule in the main session. The executor adds one small block to `replay.py`: for every Bash call, main or sub, call `bash_windows_rule.check` alone and count the refusals by tag, printing one line per part and the number of distinct calls refused by any part. Then it regenerates the extract with `python -I gates/tests/replay_extract.py --out gates/state/replay/calls.jsonl --days 14` (the output stays under the gitignored `gates/state/replay/`, which `tracked_path_refusal` enforces) and runs `replay.py` on the branch's gates folder.

The report in the executor's hand-back gives the five counts and the distinct total, against the figures measured on 2026-11-06 over a 14-day extract of 14,686 Bash calls (3,164 main, 11,522 subagent): length 791 (227 main, 564 sub; 531 hold a heredoc; 3 are exactly 1,000 and pass); backslashes in single quotes, heredocs or bare words 165 (of which 74 are also over 1,000); `cmd` with a slash flag and no `MSYS_NO_PATHCONV` 13; `ref/slash:.dotpath` 2; trailing `\"` 6. A fresh 14 days shifts the window, so the numbers differ; the executor names each shift and says whether the rule or the window caused it. A count more than double or less than half of these is a stop: report it before merging. The run must show zero hook crashes (the replay exits 1 on a crash) and hook code under `LINE_BUDGET`. Nobody runs the full hook test suite; the executor runs `test_bash_windows_rule.py`, `test_percall.py` if it exists, and `test_docs_numbers.py`.

### T3: the probe record

Same executor. `gates/tests/probe_results.md` is the README's record of what live probes of Claude Code found. The executor adds one dated section with the five measurements above as run on Claude Code 2.1.310 (the version `claude --version` printed today): the four halving results, the `"\\$HOME"` and `"\\\\srv"` double-quote results, the `cmd` hang and its `MSYS_NO_PATHCONV=1` cure, the `origin/main:.gitignore` failure, and the trailing `\"` failure. The executor reruns each one in the Bash tool before writing it, with a `timeout 5` in front of the `cmd` call so it cannot hang. This settles rows P9 to P13 of the Premise map, which today rest on this session's runs and have no repo home.

### T4: the docs

The hooks-repo side, same branch: `gates/README.md` gets one paragraph on `bash_windows_rule.py` after the `media_rule.py` paragraph, saying the five parts in plain words, that `MAX_CHARS` in the module is the one home of the number, that one scanner reads quoting for three parts, and that it reads the Bash tool only; the `percall.py` paragraph names it in both lists. `README_NAMES` in `gates/tests/test_docs_numbers.py` gains `bash_windows_rule.py`, so `test_readme_names_new_records` fails if the README forgets it. A new `test_preamble_quotes_the_bash_limit` in the same file reads `MAX_CHARS` from the module and fails unless the preamble's "Shells on Windows" section quotes that number formatted as `1,000` (the test formats it, so the doc never drifts from the code).

The Hydra side, on a branch `claude/bash-windows-rule-docs` in a worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\bash-windows-rule-docs`: the "Shells on Windows" section of `docs/agents/brief-preamble.md` grows one short paragraph saying a hook refuses the five shapes in a Bash call, what to do instead (Write the content to a file and run the file; forward slashes; the PowerShell tool for `cmd` and for git on a slashed ref; `MSYS_NO_PATHCONV=1`), and that `bash_windows_rule.py` in the gates folder owns the rule and the number. It does not restate the scanner. `docs/agents/rulings/review-and-hooks.md` gets two lines in its format, newest first: "yes block those too" (2026-11-06, the three added shapes) and "length over 1000 and backslashes" (2026-11-06, the hook's first two parts), each pointing at this plan. No doc in `docs/agents/` or in `C:\Users\Patrick\.claude\*.md` tells agents to use long heredocs: a grep for `heredoc` found only the preamble's lines 15 and 55, which forbid them for status lines and commit messages. So no other doc changes. The checks that read these docs are `test_docs_numbers.py` (its `DOCS` tuple holds the preamble) and Hydra's `tools/check_pointers.py`, which fails on a hook name in `docs/agents/` that does not exist as a file, so the preamble's `bash_windows_rule.py` pointer is checked once the hooks branch is merged. The merge order is the hooks repo first, then Hydra, so the pointer check never sees a missing file.

### T5: review and merge

One fresh reviewer (Sonnet) is dispatched with the hooks branch tip, named from the Hydra worktree with the hooks repo's root folder as `review_dispatch.judged_folder` allows, and its brief lists the Hydra docs branch as part of the same change. The reviewer reads the module, the tests, the replay block, the probe record and both docs changes, checks the replay counts against T2's figures, fixes what it finds, and signs off once. Then the main session merges as the README's "Making a change live" says: `main` into the branch in the worktree, run `test_bash_windows_rule.py` and `test_docs_numbers.py` there, fast-forward `main`. It never runs the full hook suite, because the change does not touch `common.py`, `lib\`, `settings.json` or `tests\lib\`. The Hydra docs branch merges after, with the docs check `hydra_tests.exe "-tc=docs match code*"`.

## Open questions for the user

1. **Backslash pairs in double quotes.** The rule's real test for part 2 is: refuse when the scanner labels any `\\` as single, heredoc or bare. Double is left out. In double quotes bash itself turns `\\` into `\`, so the usual result is what Claude meant: `"a\\b"` prints `a\b`. But two cases came out wrong in this session's probes: `"\\$HOME"` printed the literal `$HOME`, and `"\\\\srv"` printed `\srv` (bash would print `\\srv`). In 14 days, 104 past calls had pairs only in double quotes, and none of them showed a real failure. Adding double quotes would have refused 269 calls instead of 165. Recommendation: leave double quotes out, as the user was told they are usually harmless, and revisit if a real failure appears in the refusal log or a transcript.

2. **Other slash-flag tools.** Part 3 lists `cmd` only, as the user's words did. Other Windows tools take slash flags too (`reg`, `tasklist`, `taskkill`, `findstr`), and Git Bash rewrites their `/x` words the same way, though they fail with an error rather than hanging. Past calls to them were not counted. Recommendation: keep `cmd` only. A list of tools is the pattern the user called a losing battle, and the preamble already says to run Windows tools from the PowerShell tool.

3. **The PowerShell tool as a second suggestion in the length refusal.** Today's check shows the PowerShell tool starts `pwsh.exe` with the command in an environment variable (`CLAUDE_CODE_SHELL_LAUNCHER_SCRIPT`) and runs it with `Invoke-Expression`, so it does not pass the text on the command line the way the Bash tool does. Its own ceiling was not measured. Recommendation: leave PowerShell out of the length refusal in this change; the refusal names the Write tool only. If wanted, a one-call probe of an 8,500-character PowerShell command can be added to T3.

4. **Citing the GitHub issues.** The reasons for the halving and the long-command cut-off come from users' reports on `anthropics/claude-code` (#85856, #97409, #93618, #85111), none confirmed by a maintainer, and this planner did not open them. Recommendation: the preamble cites the user's ruling and the measured results in `probe_results.md`, not the issues.

## Out of scope

Double-quoted backslash pairs (question 1), a tool list for part 3 (question 2), the PowerShell tool's own length limit (question 3), and the four GitHub issues (question 4) are decided by the user and build nothing here. The retired path gate of 2026-10-06 is not revived; this rule replaces nothing and shares no code with it.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Module `percall.py` calls for Bash calls, main and subagent, never a second `*` registration | The rule in one paragraph | T1 | `test_status_call_passes_through_percall`, `test_crash_denies` (both routes); reviewer reads `settings.json` unchanged |
| Crash denies (`closed`) | The rule in one paragraph | T1 | `test_crash_denies` |
| 1,000 passes, 1,001 refused, raw `len` | Part 1 | T1 | `test_length_boundary`, `test_length_is_raw_len` |
| Nobody raises the number | Part 1 | T1, T4 | `test_length_boundary` builds from `MAX_CHARS`; `test_preamble_quotes_the_bash_limit` ties the doc to it |
| Refusal says what to do instead, Write-then-run | The rule in one paragraph, parts 1 to 5 | T1 | `test_every_refusal_has_instead`, `test_long_git_commit_refused_with_file_advice` |
| Status helper call never caught | How the quoting check stays small | T1 | `test_status_call_passes_through_percall` |
| Long `git commit` refused; agent commits with `-F <file>` | Part 1 | T1 | `test_long_git_commit_refused_with_file_advice` |
| PowerShell tool never read | What the user asked for | T1 | `test_powershell_tool_not_read` |
| Single, heredoc and bare pairs refused; double passes | Part 2 | T1 | `test_backslashes_single_heredoc_bare_refused`, `test_backslashes_double_pass` |
| One scanner for parts 2, 4, 5; no second quoting reader | How the quoting check stays small | T1 | `test_scanner_is_the_one_reader`; reviewer reads the module |
| Unclosed quote means inside; no "cannot read" case | How the quoting check stays small | T1 | `test_unclosed_quote_is_inside` |
| `cmd` slash flag refused unless `MSYS_NO_PATHCONV=1`; heredoc text not read | Part 3 | T1 | `test_cmd_slash_refused_and_pathconv_passes` |
| `ref/slash:.dot` refused; no-slash ref and heredoc text pass | Part 4 | T1 | `test_ref_dotpath` |
| Trailing `\"` on a drive path refused; forward slashes pass | Part 5 | T1 | `test_trailing_backslash_quote` |
| Each reason carries its part tag | The rule in one paragraph | T1 | `test_every_refusal_has_instead` |
| Proof reports past refusals per part, main and sub, against the figures; big shift is a stop | T2 | T2 | the replay block's output lines in the hand-back; reviewer compares to T2's figures |
| Zero crashes and under `LINE_BUDGET` | T2 | T2 | `replay.py` exit code |
| Agents never run the full hook suite | T2, T5 | T2, T5 | the test commands in the hand-back name three files at most |
| Probe record holds the five measurements, rerun first | T3 | T3 | reviewer opens `probe_results.md` and reruns one |
| README names the module and the one home of the number | T4 | T4 | `test_readme_names_new_records` with `bash_windows_rule.py` in `README_NAMES` |
| Preamble quotes 1,000 from the code and points at the module | T4 | T4 | `test_preamble_quotes_the_bash_limit`; `tools/check_pointers.py` |
| Rulings file gets both quotes | T4 | T4 | reviewer reads `docs/agents/rulings/review-and-hooks.md` |
| Hooks merge before Hydra docs | T4 | T5 | `check_pointers.py` at the Hydra merge |
| One executor, one fresh reviewer, worktrees in allowed folders | T1, T5 | T5 | `review_dispatch.py` record; the ref gate refuses an unreviewed move of `main` |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `percall.py` is the only PreToolUse `*` hook and a new per-call rule becomes a module it calls | code | `C:\Users\Patrick\.claude\gates\README.md` line 92 | checked 2026-11-06 |
| P2: `_subagent` skips the shell rules for a status call and chains `delete_rule.check` with the other refusals; `_main_session` computes its refusals outside the `stall` try | code | `C:\Users\Patrick\.claude\gates\percall.py` lines 89-94, 103-105 | checked 2026-11-06 |
| P3: `progress.is_status_call` recognises the helper by its exact shape; `SHELL_TOOLS` is Bash and PowerShell | code | `C:\Users\Patrick\.claude\gates\progress.py` lines 20-34 | checked 2026-11-06 |
| P4: `common.run` in `closed` mode denies on a crash | code | `C:\Users\Patrick\.claude\gates\common.py` lines 48-65 | checked 2026-11-06 |
| P5: `WORKTREE_FOLDERS` names the two allowed worktree folders; `OPERATORS` and `_word` cut a command into pieces and read a command word | code | `C:\Users\Patrick\.claude\gates\delete_rule.py` lines 17-18, 29, 66-69 | checked 2026-11-06 |
| P6: `replay.py` judges a subagent shell call through `percall.body` and a main-session one through `_library_refusal` only; it fails on a crash or over `LINE_BUDGET` | code | `C:\Users\Patrick\.claude\gates\tests\replay.py` lines 39, 243-256, 496 | checked 2026-11-06 |
| P7: the extract is regenerated with `replay_extract.py --out ... --days N` and refuses an output path git would track | code | `C:\Users\Patrick\.claude\gates\tests\replay_extract.py` lines 14, 31-45 | checked 2026-11-06 |
| P8: `test_docs_numbers.py` reads the preamble (`DOCS`) and checks `README_NAMES` against the README | code | `C:\Users\Patrick\.claude\gates\tests\test_docs_numbers.py` lines 21, 49, 131-134 | checked 2026-11-06 |
| P9: `'x\\y'` reaches bash as `x\y`, heredoc `b:\\` as `b:\`, bare `a\\b` as `ab`, on Claude Code 2.1.310 | unchecked | this session's Bash tool runs; no repo file holds them yet | T3 |
| P10: `"a\\b"` gives `a\b`, `"\\\\srv"` gives `\srv`, `"\\$HOME"` gives the literal `$HOME` | unchecked | this session's runs for the first two; the main session's brief for the third | T3, question 1 |
| P11: `cmd.exe /d /c "echo hi"` opens an interactive cmd and never prints `hi`; with `MSYS_NO_PATHCONV=1` it prints `hi` | unchecked | this session's Bash tool run under `timeout 5` | T3 |
| P12: `git show 'origin/main:.gitignore'` fails with `ambiguous argument 'origin\main;.gitignore'`; `main:.gitignore` works | unchecked | this session's Bash tool run in the Hydra checkout | T3 |
| P13: `ls "C:\Users\Patrick\"` fails with `unexpected EOF while looking for matching` | unchecked | this session's Bash tool run; `docs/sessions/2026-09-17-b18a4752.md` line 11 reports the same | T3 |
| P14: the 14-day counts (14,686 Bash calls; 791 over 1,000; 3 exactly 1,000; 165 pairs in single, heredoc or bare; 13 `cmd`; 2 ref; 6 trailing `\"`; 12 `git` over 1,000, no commits) | unchecked | a scratch extract made with `replay_extract.py`, which a committed doc never cites | T2 |
| P15: the user's rulings "length over 1000 and backslashes" and "yes block those too" | ruling | the main session's brief to this planner, quoted in "What the user asked for"; T4 files them in `docs/agents/rulings/review-and-hooks.md` | checked 2026-11-06 |
| P16: "trying to ban specific command patterns is a losing battle because there will always be another pattern nobody thought to check." | ruling | `docs/sessions/2026-10-31-6777b546.md` line 25 | checked 2026-11-06 |
| P17: a hook never parses free text when a chokepoint exists | code | `docs/superpowers/plans/2026-10-31-harness-hooks-and-tools.md` line 9 (the plan's paraphrase, not a quote) | checked 2026-11-06 |
| P18: the old path gate matched text without knowing quoting and would have refused 39 ordinary commands | unchecked | `docs/sessions/2026-10-06-ca4b1758.md` line 12 reports it; the old gate's code is gone, so the plan uses it only as history | T1 |
| P19: the PowerShell tool starts `pwsh.exe` 7.6.9 with the command in `CLAUDE_CODE_SHELL_LAUNCHER_SCRIPT` and `Invoke-Expression`; its length ceiling is unmeasured | unchecked | this session's `Win32_Process` CommandLine read | question 3 |
| P20: the GitHub issues #85856, #97409, #93618, #85111 are users' claims with no maintainer confirmation | unchecked | the main session's brief; not opened | question 4 |
| P21: no doc in `docs/agents/` or `C:\Users\Patrick\.claude\*.md` tells agents to use long heredocs | code | `docs/agents/brief-preamble.md` lines 15, 55 are the only `heredoc` hits in both folders | checked 2026-11-06 |
| P22: Claude Code is 2.1.310 | unchecked | `claude --version` in this session | T3 |
| P23: a reviewer brief may name a commit in the other repo with that repo's root folder | code | `C:\Users\Patrick\.claude\gates\README.md` line 27 (`judged_folder` in `review_dispatch.py`, not opened) | checked 2026-11-06, README only |

This plan has no live steps: nothing here touches Clone Hero, so there is no `## The live steps` section.
