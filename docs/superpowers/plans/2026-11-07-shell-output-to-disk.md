# Shell output to disk: shorter shell results, and failed builds that keep their end

Task SO-PLAN, session 6758ee5e, planned 2026-11-07. The change lives in the hooks repo, `C:\Users\Patrick\.claude`.

User decisions: on 2026-11-07 the user approved this plan and answered all ten questions with "go with recs". So every recommended value under `## Questions` is the user's decision. Question 7's display change is accepted, and for question 8, part 2 stops and part 1 goes ahead if auto mode denies the rewrite.

What SO-0 changed. Its results are in `gates\tests\probe_results.md`, "Shell output probes (2026-11-07)". LS-S3 saw no denial, but it does not prove part 2 is safe: nothing shows a review took place in those `claude -p` runs, and Claude Code 2.1.310 holds a denial for a hook-changed input (`server_input_rewritten`). So part 2 goes live only after one failing test command passes through the rewrite in a real desktop session in auto mode. Part 1 does not wait for that. A passing result above the 30,000-character ceiling keeps Claude Code's own notice, because that notice is shorter than a trim.

## The plain version

Today a long shell result goes into the conversation whole, up to about 30,000 characters, and every one of those characters costs usage on every later turn. A failed build or test is worse: nothing is saved to disk, and the model gets an excerpt of about 10,000 characters. A failure of 10,000 to 30,000 characters loses its middle, and one over 30,000 also loses its real end, so the line that says why it failed can be the line we never see. The user's rule for this work, in their words: "so instead of saving you want to waste even more usage? no. unacceptable." So no limit goes up, and no new tool or wrapper program is added. The user picked the design "Hook and build rewrite".

The change has two parts, both hooks in the gates folder. Part 1 is a new PostToolUse hook on every Bash and PowerShell call. When a passing command's output is longer than a cap (4,000 characters is the recommendation, question 1), the hook saves the full text to a file and shows the model only the start, the lines that look like errors, the end, and the file's path. Above Claude Code's own 30,000-character ceiling the file already exists, because Claude Code saved it; the hook reads that file and uses it. Part 2 is a rewrite, inside the existing `percall.py` hook, of build and test commands only. It appends one line that prints the command's real exit code, so the command itself exits 0, reaches Claude Code's save step, and the full output survives. The PostToolUse hook then reads that exit code and puts `COMMAND FAILED, exit code N` as the first line the model sees. Nothing is auto-approved: the rewrite never sets `permissionDecision`, so the user's permission rules and auto mode judge the command as they do today. What the user will see differently: a failed build or test no longer shows as a red error in the tool display, because the command now exits 0; its result starts with `COMMAND FAILED, exit code N` instead (question 7). A long passing result arrives trimmed (question 1); whether the user's own display shows the trim or only the model does is not known yet (P40).

Measured today over the last 40 main-session transcripts and the last 300 agent transcripts (64.2 million characters of tool results), a 4,000 cap would have removed 5.5 million characters, 8.6% of everything every tool returned. That is the saving the change buys, before read-backs. A check after the change counts how much of it the model reads back from the saved files, with a stop rule, so we know it saved usage rather than moved it.

One thing the probe must settle before any code is written: whether a rewritten command passes in auto mode, the real default on this machine. Claude Code's docs say a settings command hook may change a Bash or PowerShell call without the auto-mode denial. If the live probe shows otherwise, part 2 stops there and is reported; part 1 goes ahead alone, because it never touches a command's input.

## Rulings that bind this plan

- "so instead of saving you want to waste even more usage? no. unacceptable." (the user, choosing against raising `bashOutputMaxChars` or any limit; quoted by the main session in the SO-PLAN brief). So: no limit goes up, no new MCP tool, no wrapper program. The change must reduce usage, and the check in SO-4 is how we know.
- "Hook and build rewrite" (the user, choosing this design; quoted in the same brief). So: a PostToolUse hook that trims, and a PreToolUse rewrite for build and test commands, and nothing from squeez beyond the idea.
- Permanent docs never cite scratch (`tests/test_docs_match_code.cpp`, the scratch-path check whose fixtures sit near line 1290). So the throwaway spike's folder is named nowhere in this plan; every fact that came from it is `unchecked` below, and SO-0 reruns it into `gates\tests\probe_results.md`.
- A new number needs a user decision (`docs/agents/brief-preamble.md`, "If you write code"). Every number in this plan is a question under `## Questions`, with a recommended value.

## What was checked in the code, and how

How a per-call hook answers today. `percall.py` is the only PreToolUse `*` hook (`gates\README.md`, "How a hook is registered", and the `PreToolUse` block of `gates\settings.fragment.json`). Its `body` routes by `agent_id`; the subagent side runs `progress.check`, refuses `hydra_build_pass`, then for a shell call runs `delete_rule.check`, the whole-library check, `bash_windows_rule.check` and `media_rule.check`, and returns `common.deny(reason)` for the first that refuses, else the progress notice (`percall.py` lines 98 to 122). The main-session side does the same refusals and then `stall.run`. Every answer is one dict with a `hookSpecificOutput` key: `common.deny` sets `permissionDecision: deny`, `common.context` sets `additionalContext` (`common.py` lines 17 to 28). `common.run` wraps the body in one `try`; in `closed` mode a crash denies, in `loud` mode a crash prints a `systemMessage` (`common.py` lines 48 to 71). So `updatedInput` can ride in the same `hookSpecificOutput` dict as a progress notice, and never beside a deny, because percall returns the deny and stops.

How the refusing rules see the command. `delete_rule.check` reads `data["tool_input"]["command"]` as the raw string, cuts it at `OPERATORS` (`&&`, `||`, `;`, `|`, `&`, newline) and reads the first word of each piece with `_word` (`delete_rule.py` lines 29, 66 to 69, 152 to 162). `bash_windows_rule.check` reads the same raw string (`bash_windows_rule.py` lines 237 to 244). Both run before anything in percall composes an answer, so a rewrite that runs last, on the string they already judged, cannot hide a command from them: what they read is what Claude wrote, and the only change is a tail appended after their verdict. The user's `permissions.deny` rules in `settings.json` today are all prefix rules on `gh` (`Bash(gh pr create*)` and its PowerShell and `gh.exe` forms), and the tail is added at the end, so a command's start is unchanged. Whether Claude Code judges those rules on the original or the rewritten command is not read from any source; it is P39, and LS-S4 runs a rewritten command that starts with a denied prefix.

What percall skips. `progress.is_status_call` names the one shell call that is never counted or refused (`progress.py` lines 30 to 34). The rewrite skips it too, so an allow rule that matches the status helper by prefix keeps matching.

Settings. `settings.json` and `gates\settings.fragment.json` hold the same `hooks` block today (checked with a JSON compare on 2026-11-07, and held by `test_fragment_matches_settings` in `gates\tests\test_docs_numbers.py`). There is no PostToolUse registration in either. `settings.json` sets `permissions.defaultMode` to `auto`. The fragment's allow rules are two PowerShell prefix rules, for `status_append.ps1` and `publish.ps1`; neither names a build or test command.

The test rule for a hooks merge. Rule 1 in `docs/agents/agent-rules.md` says the main session runs the whole hooks suite only when a change touches `gates\common.py`, `gates\lib`, `settings.json` or `gates\tests\lib`. This change touches `settings.json`, so the merge runs every hook test. `gates\README.md`, "Running the tests", gives the command.

The sweep. `cleanup_sweep.ps1` removes idle worktrees, old scratch folders under `Temp\claude` and old records under `gates\state`; `$ScratchAgeDays = 7` is how old a scratch entry must be, and a running session's scratch folder is never removed (its header and lines 48 to 58). It does not touch Claude Code's `tool-results` folders.

Where Claude Code saves big output. This session's project folder holds a `<session id>.jsonl` transcript and a `<session id>\tool-results\` folder beside it; across the project there are 111 such folders holding 838 files and 169 MB (listed 2026-11-07). The oldest file there is dated 2026-10-14, the same day as the oldest transcript, which fits a 30-day cleanup but proves nothing about who deletes them. That is an `unchecked` row (P31).

What the docs say about output limits. Claude Code's tools reference, "Output limits", says a valid result is inline up to roughly 30,000 characters by default, and past that the model gets "the path of a file saved to the session directory" plus a preview of the first 2,000 characters; a failure is inline up to roughly 10,000 characters, past that "a head-and-tail excerpt of that size cut from the read-back window, with no file path". It also says exit 1 counts as valid for the Bash tool only for `grep`, `rg`, `egrep`, `fgrep`, `find`, `diff`, `test`, `[`, `git diff` and `git grep`. Those are the two limits this plan works around, and SO-0 confirms them on this machine's Claude Code.

What the docs say about rewriting input in auto mode. The hooks page, "PreToolUse `updatedInput` in auto mode", says a hook that returns `updatedInput` with no `permissionDecision` can have its call denied with "a hook changed this call's input after auto mode reviewed it", and then: "In the sessions that show this text, a `command` hook in settings or a plugin's hooks file can change Bash, PowerShell, Monitor, Edit, Write, and NotebookEdit calls without this denial." That is the sentence part 2 rests on, and LS-S3 tests it live.

What else reads a failed tool result. In the gates folder only `tests\replay_extract.py` reads `is_error`, and it reads old transcripts for the replay proof, not live calls. `stall.py` reads the transcript's `usage` block for its context warning, not tool results. The session-card scripts in `C:\Users\Patrick\.claude\tools\session_cards\common.py` read a tool result's text (`tool_result_text`, line 334) and never its `is_error` flag; only their fixtures carry one. The Hydra repo's `tools\` folder reads `is_error` only in the Clone Hero MCP server's own replies. So the one thing that changes when a failed build stops being an error is what the user sees in the tool display, and that is question 7.

## Prior art, read on 2026-11-07

squeez (https://github.com/claudioemmanuel/squeez) rewrites Bash calls to run through its own `squeez wrap` program, keeps the original in a content-addressed store and lets the model fetch it back with an MCP tool. It reports 91% token reduction on its own benchmark. This plan takes only its idea (trim what the model sees, keep the full text on disk) and none of its mechanism: the user ruled out a wrapper program and a new MCP tool.

qdf-hook, also called terse (https://pkg.go.dev/github.com/alex60217101990/terse), is a PostToolUse hook on every tool that summarises structured output, replaces repeats with a reference and keeps blobs in a cache with a 30-day life and a 128 MiB cap. It reports 88% of tool-output tokens removed over 18,000 calls, self-measured. Its one lesson for this plan is the garbage collector: saved files need an owner that deletes them, which is question 6.

Claude Code's own docs (https://code.claude.com/docs/en/hooks, "PostToolUse decision control") say `updatedToolOutput` "replaces the tool's output with the provided value before it is sent to Claude" and must match the tool's output shape, or it is ignored and the original is used. The Bash example there has `stdout`, `stderr`, `interrupted` and `isImage`.

## The numbers, measured on 2026-11-07

Two read-only scripts counted the sizes of tool results in the transcripts under `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test`, the last 40 main-session files and the last 300 agent files by modification time. They read sizes only. SO-0 commits them as `gates\tests\shell_output_sizes.py` so the numbers can be rerun and the SO-4 check reuses the reader.

| Set | Tool results | Characters | Shell calls | Over 4,000 | Removed at a 4,000 cap | Removed at 8,000 |
|---|---|---|---|---|---|---|
| 40 main transcripts | 8,888 | 16,054,731 | 2,972 | 389 | 1,615,290 (10.1%) | 658,721 (4.1%) |
| 300 agent transcripts | 16,617 | 48,192,451 | 3,559 | 578 | 3,900,877 (8.1%) | 2,169,779 (4.5%) |
| Both | 25,505 | 64,247,182 | 6,531 | 967 | 5,516,167 (8.6%) | 2,828,500 (4.4%) |

"Removed" counts the characters past the cap, as if a trimmed result kept all 4,000. The hook keeps about 3,300 (question 2), so at a 4,000 cap the real saving is about 700 characters per trimmed call larger than the table shows, before read-backs. Shell results are 35.5% of main-session tool-result characters and 18.1% of agents'. Read results are the biggest share (44.5% and 63.8%) and are out of scope here.

Failed shell calls: 203 in the main set and 473 in the agent set, 676 in all. 22 of them were over 4,000 characters, and 7 sat at the 10,000 ceiling, which means their middle was lost. Transcripts cannot show how many of those 7 also lost their real end, which needs output over 30,000 characters. Those 7 are the calls part 2 exists for.

Python start cost, measured 5 times each on this machine: `python.exe -I -S -c pass` takes 15 to 27 ms; the whole `percall.py` on a main-session Read call takes 36 to 65 ms. A PostToolUse hook adds one such start to every shell call, so about 40 ms per call.

## The design, point by point

### 1. One new module owns every rule and number

`gates\shell_output.py` is the one home. Its first lines hold the numbers: `CAP` (question 1), `HEAD_CHARS` and `TAIL_CHARS` (question 2), `ERROR_LINES` and `ERROR_WORDS` (question 3), `BUILD_TEST_COMMANDS` and `PYTHON_TEST_FLAGS` (question 4), `NO_TRIM_COMMANDS` (question 5). It has three public functions. `rewrite(data)` returns the rewritten `tool_input` for a build or test command, else `None`. `trim(data)` returns the `updatedToolOutput` dict for a PostToolUse call, else `None`. `saved_path(data, text)` names the file the full text goes to (question 6). Docs point at the module and never restate the lists.

### 2. Part 2 inside percall: the exit-code tail

`percall.body` calls `shell_output.rewrite` last, after every refusal, on both the subagent and the main-session side, only when the answer so far sets no `permissionDecision`: the main-session side can answer `ask` for a media-rule refusal (`percall.py`, `_main_session`), and an `ask` is never rewritten either. The returned `tool_input` is placed as `updatedInput` inside the same `hookSpecificOutput` dict as any progress notice, and `hookEventName` stays `PreToolUse`; when the answer so far is `None` or holds only a `systemMessage` (stall's crash notice), a new `hookSpecificOutput` is added beside it. `permissionDecision` is never set (the brief's rule: auto-approving would skip the user's permission checks). The call to `rewrite` sits in its own `try`: a crash there returns the un-rewritten answer plus a `systemMessage` naming the crash, so a bug in the rewrite never refuses a call and never silently changes one.

`rewrite` returns `None` for: a call whose `tool_input.run_in_background` is true; the status call (`progress.is_status_call`); a command that already ends with the tail; and any command with no build or test piece. A piece is one part of the command cut at `delete_rule.OPERATORS`. A piece counts when its first word, read with the same `_word` rule `delete_rule` uses, is in `BUILD_TEST_COMMANDS`, or when its first word is a Python launcher (`py`, `python`, `python3`) and the piece holds `-m unittest` or `-m pytest`. SO-2 makes the splitter and the word rule public in `delete_rule.py` (`pieces` and `word`) so one reader serves both; no second splitter is written.

The tail for Bash is a newline then `echo "[real exit code $?]"`. The tail for PowerShell is a newline then `$__ok=$?; $__rc=$LASTEXITCODE; if($null -eq $__rc){$__rc=0}; if(-not $__ok -and $__rc -eq 0){$__rc=1}; "[real exit code $__rc]"; $global:LASTEXITCODE=0`. Both come from the spike and are `unchecked` until LS-S2 runs them (P20, P21). The PowerShell one is longer because the tool reads `$LASTEXITCODE` to decide failure (P21), and a cmdlet error such as `Get-Item` on a missing file sets `$?` false with no exit code, which the tail turns into 1.

Chained commands. The tail is appended once, after the whole command, so it reports what the shell leaves in `$?` when the command ends: for `a && b` the failing piece's code, for `a; b` the last piece's code. That is the same status the tool reads today, so the `COMMAND FAILED` line says what the tool would have said. A build word in any piece rewrites the whole command. A command that ends in its own `exit N` never reaches the tail, so it stays a plain failure, cut at 10,000 characters as today; the preamble tells agents not to write one in a build or test command (SO-3).

The bash length rule. `bash_windows_rule.MAX_CHARS` counts the command Claude wrote, before the tail, because the rule runs first. The tail adds 29 characters in Bash and 157 in PowerShell; Claude Code has no command-length limit of its own that this reaches.

### 3. Part 1, the PostToolUse hook

A new registration, matcher `Bash|PowerShell`, runs `gates\shell_output.py` as its own script, in `common.run(body, "loud")`, with no `onFailure`. A crash prints a `systemMessage` and returns no `updatedToolOutput`, so Claude Code keeps today's output. That is the only hook on PostToolUse, so a later PostToolUse job becomes a part this script runs, the way `percall.py` and `prompt.py` work.

`trim(data)` reads `tool_response`. When `tool_response.persistedOutputPath` is present (P22), it reads that file as the full text; else the full text is `stdout` followed by `stderr`. It then finds the last `[real exit code N]` marker line, removes every marker line from the text, and remembers N. Four outcomes:

- No marker and the text is at most `CAP` characters: return `None`. Nothing changes, and this is most calls.
- `persistedOutputPath` present and no failing marker: return `None`. Claude Code's own notice is 2,292 to 2,433 characters (`gates\tests\probe_results.md`, "Shell output probes"), so a 3,300-character trim would cost more than today, which the user's first ruling above forbids.
- Marker with N not 0, text at most `CAP`: return the response with `stdout` set to `COMMAND FAILED, exit code N` then the text.
- Text over `CAP`, unless the command's first piece is in `NO_TRIM_COMMANDS` (question 5): save the full text (with the `COMMAND FAILED` line first when N is not 0) at `saved_path`, unless `persistedOutputPath` already names a file, and return the response with `stdout` set to: the first `HEAD_CHARS` characters, one line `[trimmed: N characters, full text at <path>]`, up to `ERROR_LINES` lines holding an `ERROR_WORDS` word, then the last `TAIL_CHARS` characters; `stderr` empty; every key starting `persistedOutput` dropped (P23).
- Marker with N 0 and text at most `CAP`: return the response with the marker lines removed.

The returned dict keeps every other key of `tool_response` as it came, so it matches the tool's output shape (the docs' rule for `updatedToolOutput`).

### 4. Where the saved file goes

`saved_path` writes under the session's scratch folder, in a `shell-output` subfolder, named by the first 12 hex digits of the text's SHA-1 so a repeated output is one file. The scratch folder is the hook input's scratchpad_dir field, present in every desktop-hosted session (`gates\README.md`, "What the probes found"). That folder is already swept by `cleanup_sweep.ps1` after `$ScratchAgeDays` days once the session has ended, so no new deletion code is written (question 6). When the field is absent, which the probes saw only in plain `claude -p` sessions, the file goes to the `tool-results` folder beside the transcript named in `transcript_path`, where Claude Code puts its own; LS-S1 runs under `claude -p`, so that is the branch it exercises, and `test_shell_output.py` pins both.

### 5. What the model sees, and what it should do

A trimmed result tells the model where the full text is. The preamble gains one paragraph (SO-3): read the saved file with the Read tool and `offset`/`limit`, or Grep it, and never read it whole, because a whole read puts back what the trim took out. The SO-4 check counts exactly that.

### 6. The visible change

A build or test command that fails now exits 0 as far as Claude Code can tell. The tool display no longer marks it as an error; the model, and the user reading the transcript, see `COMMAND FAILED, exit code N` as the first line instead. Nothing in the gates, the session-card scripts or Hydra's tools reads the error flag of a live call (checked above). This is question 7, and no code is written until the user answers it.

### 7. Auto mode

The design never sets `permissionDecision`, so the question is whether a settings command hook's `updatedInput` on Bash and PowerShell passes under `defaultMode: auto`, as the docs say. LS-S3 runs a rewritten build-like command under `--permission-mode auto`. If it is denied, the plan stops part 2: SO-2 does not add the percall call, the `COMMAND FAILED` path in `trim` stays as dead code, and the report to the user says the docs sentence did not hold on this machine. There is no workaround step: an `allow` from the hook would skip the user's checks, and that is the one thing the brief forbids.

The spike also saw a narrow allow rule such as `Bash(python gen.py:*)` deny the rewritten command (P27, unchecked). No rule in `settings.json` today names a build or test command, so nothing on this machine meets that case; LS-S4 records it so the README can say it.

### 8. How we know it saved usage

SO-4 adds `gates\tests\shell_output_usage.py`, which reads the transcripts written since the merge date and counts, per session: shell results carrying the `[trimmed:` line and the characters each one removed (the number in that line less the characters shown); Read and Grep calls whose `file_path` or `path` lies under a saved-file folder, and the size of each such result; and `COMMAND FAILED` lines, with how many of those results end in the real last line (the one before the marker). The saving is removed characters minus read-back characters. The check runs once, after 7 days or 20 main-session transcripts, whichever comes first (the stop condition). If read-backs are more than half of the removed characters, or the saving is negative, SO-4 stops and reports to the user with the numbers, and the user decides between a higher cap and a revert; nothing is changed on their behalf (question 9).

### 9. Rollback

To undo the whole change: `git -C C:\Users\Patrick\.claude revert --no-commit --no-merges <old main>..HEAD`, then one commit with trailers, as `gates\README.md`, "Rolling back", says. To undo only part 1, remove the PostToolUse block from `settings.json` and the fragment together and commit; the next shell call gets today's output. To undo only part 2, `git -C C:\Users\Patrick\.claude checkout <last good> -- gates/percall.py` and commit. `test_docs_numbers.test_fragment_matches_settings` fails if the two files drift during either.

### 10. What stops being true in the docs

`gates\README.md` says the hooks "do five jobs and nothing else"; SO-2 adds the sixth in that sentence (they keep long shell output out of the conversation) and a paragraph for `shell_output.py` after `media_rule.py`'s, says PostToolUse has one registration, and adds the rewrite to the `percall.py` paragraph. "percall.py is the only hook on PreToolUse `*`" stays true. `README_NAMES` in `test_docs_numbers.py` gains `shell_output.py`, and a new test there checks that the preamble quotes `shell_output.CAP` the way `test_preamble_quotes_the_bash_limit` checks `MAX_CHARS`. In the Hydra repo, `docs/agents/brief-preamble.md` line 5 lists what hooks enforce and gains the trim; its "Shells on Windows" section gains the paragraph from point 5 and the `exit N` note from point 2. `docs/agents/agent-rules.md` line 22 ("Hooks check four of these") stays as it is, because the new hook checks no rule. `tests/test_docs_match_code.cpp` changes only in that the finished plan is removed in SO-4's merge and any line that names it says how to read it now.

## Questions

Each question names the side that owns the answer: the hooks repo (Python, `shell_output.py`) unless it says otherwise.

1. **The cap.** Above how many characters does a passing shell result get trimmed? At 4,000 (recommended): in the last 40 main sessions, 389 of 2,972 shell results would have been trimmed and 10.1% of all tool-result characters removed; a typical trimmed result shows about 3,300 characters instead of, say, 7,500. At 8,000: 145 results trimmed, 4.1% removed; a 7,500-character `git status` after a big merge shows whole. At 2,000: 816 trimmed, 17.0% removed, but a 2,500-character test summary loses its middle. What you see in the tool display for a trimmed result is not known: the probes can only read what the model gets, so the display may show the trim or the full text (P40). The counts at 8,000 and 2,000 and the 17.0% come from the same two scripts as the table and are rechecked by SO-0 (P42). Owner: `CAP`.

2. **How much of the start and the end.** Recommended 1,500 characters each, so a trimmed result is about 3,300 characters with the path line and error lines. At 1,500 you see a build's first configure lines and its last 20 or so lines of link output. At 1,000 each a trimmed result is about 2,300 characters, and a doctest summary's last lines may be cut mid-table. At 2,500 each the result is about 5,300 characters and most 6,000-character outputs save almost nothing. Owner: `HEAD_CHARS`, `TAIL_CHARS`.

3. **Error lines.** How many lines holding an error word are kept from the middle, and which words. Recommended: up to 10 lines holding any of `error`, `failed`, `exception`, `traceback`, `fatal`, `assert`, case-insensitive, in order. With 10, a `hydra_tests.exe` run with 3 failing checks shows all 3 `ERROR:` lines; a compile with 40 errors shows the first 10 and the path line says where the rest are. With 0 you see only the start, the end and the path. Owner: `ERROR_LINES`, `ERROR_WORDS`.

4. **Which commands get the exit-code tail.** Recommended list, taken from `build_cpp.ps1`'s header, `docs/development.md` "Building under Claude Code" and "Running the tests" in `gates\README.md`: `build_cpp.ps1`, `build_slot.ps1`, `build_installer.ps1`, `mutation_probe.ps1`, `cmake`, `ctest`, `msbuild`, `hydra_tests`, `hydra_uitest`, `hydra_replay`, `hydra_precheck`, `pytest`, plus any Python launcher piece holding `-m unittest` or `-m pytest`. With this list, `.\build_cpp.ps1 -Target hydra_tests` that fails shows `COMMAND FAILED, exit code 1` first and keeps its last lines; `git push` that fails stays a plain error as today. Should `git` commands be added? The recommendation is no: their failures are short. Owner: `BUILD_TEST_COMMANDS`, `PYTHON_TEST_FLAGS`.

5. **Commands never trimmed.** Which commands' output is wanted whole even when long. Recommended: `git diff` and `git show` (the model is usually reviewing the whole change), judged by the first piece's first two words. With them on the list, a 9,000-character `git diff` arrives whole; without them, it arrives as 1,500 + 1,500 characters and the model has to Read the saved file, which costs more than the trim saved. `git log` is not recommended: its long form is paged by `-n` and its output rarely needs the middle. Owner: `NO_TRIM_COMMANDS`.

6. **Where saved files live and how long.** Recommended: the session's scratch folder, `shell-output` subfolder, which `cleanup_sweep.ps1` already removes 7 days after the session ends (`$ScratchAgeDays`). The other choice is Claude Code's own `tool-results` folder beside the transcript, where the 30,000-plus outputs already go; who deletes those is unchecked (P31), and 169 MB sit there today across 111 sessions. Owner: `saved_path`, and `cleanup_sweep.ps1` for the life.

7. **A failed build no longer shows as an error.** With part 2, a failing build or test command exits 0 for Claude Code. In the tool display it is no longer marked red; the first line of its result reads `COMMAND FAILED, exit code 1`, and the real last lines are there. Without part 2, the display stays red and the output is cut at 10,000 characters with the end lost, as today (7 of 676 failed calls in the measured set). Do you accept the display change? Owner: Claude Code's display; the hooks repo decides whether to rewrite.

8. **If auto mode denies the rewritten command.** LS-S3 decides. If it is denied, the plan's rule is: part 2 stops, part 1 goes ahead, and the report says so. The other reading is that both stop until the docs sentence is explained. Which do you want? Owner: the user.

9. **The usage check's stop rule.** After 7 days or 20 main sessions, if the characters read back from saved files are more than half of the characters the trim removed, SO-4 stops and asks. The other choice is a lower bar (a quarter) or no bar, which would let a read-back habit eat the saving unseen. Owner: `shell_output_usage.py`.

10. **Three small constants.** The window in question 9 (7 days or 20 main-session transcripts); the PostToolUse hook's `timeout` of 10 seconds (the hook costs about 40 ms, so 10 s only matters if it hangs, and a timeout leaves today's output); and 12 hex digits in a saved file's name (a repeated output is one file; 12 digits is about 280 trillion names, so a clash is not a worry). Recommended as written. A shorter window gives an earlier verdict on less data; a longer `timeout` lets a hung hook stall a shell result longer. Owner: `shell_output_usage.py`, `settings.json`, `saved_path`.

## Tasks

Each task is one executor on Opus (agent rule 2), in a worktree under `C:\Users\Patrick\.claude\worktrees\` for hooks work or `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\` for Hydra docs, one branch each, then one fresh reviewer, then the main session merges `main` into the branch, runs the merge's tests and fast-forwards `main` (`gates\README.md`, "Making a change live"). Each task fits in 100 tool calls and 20 minutes. SO-0 goes first and alone, because every later task reads its results. SO-1 and SO-3 can run together. SO-2 follows SO-1 on the same branch. SO-4 runs after the merge has been live for its window.

### SO-0: the probe (live, Opus, after the user's yes)

Owns `gates\tests\probe_results.md` (a new section, "Shell output probes (2026-11-xx)") and `gates\tests\shell_output_sizes.py` (the two size scripts, combined, read-only). Runs the live steps below with a throwaway hook and runner in the session scratch folder, which are never committed. Records for each case the Claude Code version, the command, the shell, the result's length, whether the first line, an error line and the last line survived, whether the result was marked an error, and the fields seen in `tool_response`. Reruns the size scripts and records the table from "The numbers" with the date. Acceptance: every `unchecked` row that names SO-0 in the Premise map has a line in the new section that confirms or contradicts it; `shell_output_sizes.py` runs on the project transcripts folder and prints the three-row table; no scratch path appears in the committed text.

### SO-1: the module and its tests (Opus)

Owns `gates\shell_output.py` and `gates\tests\test_shell_output.py`. Writes `rewrite`, `trim` and `saved_path` with the numbers the user chose. Tests, each pinning a literal from one run: a 200-character passing result returns `None`; a `CAP + 1`-character passing result returns the head, the path line, the error lines and the tail and drops `persistedOutputPath` and `persistedOutputSize` while keeping `interrupted`; a result with `persistedOutputPath` reads that file and saves nothing new; a result with a marker `[real exit code 3]` and 200 characters returns `COMMAND FAILED, exit code 3` first and no marker line; a marker with code 0 returns the text with the marker gone; a long failed result saves the file with the `COMMAND FAILED` line first; a `git diff` result over the cap returns `None`; `rewrite` returns `None` for `run_in_background`, for the status call, for a command already ending in the tail and for `git status`; `rewrite` appends the Bash tail to `./build_cpp.ps1 -Target hydra_tests`, to `py -I -m unittest discover ...` and to `git fetch && .\build_cpp.ps1`; the PowerShell tail is the one in point 2 of the design; `saved_path` uses the scratch field when present and the `tool-results` folder beside `transcript_path` when not; a crash inside `trim` run through `common.run(..., "loud")` prints a `systemMessage` and no `updatedToolOutput`. Acceptance: `python.exe -I -m unittest discover -t . -s gates/tests -p test_shell_output.py` passes; no number appears anywhere but the module's first lines.

### SO-2: wiring, settings and the gates docs (Opus, on SO-1's branch)

Owns `gates\percall.py`, `gates\delete_rule.py` (make `pieces` and `word` public, no rule change), `settings.json`, `gates\settings.fragment.json`, `gates\README.md`, `gates\tests\test_docs_numbers.py`, `gates\tests\test_percall_key.py` (or a new `test_percall_rewrite.py`). Adds the PostToolUse registration to both settings files in exec form with `timeout` 10 and no `onFailure`. Adds the `rewrite` call to `percall.body` in its own `try`, merging `updatedInput` into the answer's `hookSpecificOutput`. Tests: a subagent Bash call to `.\build_cpp.ps1` with a fresh agent record returns `additionalContext` (the first-call notice) and `updatedInput` in one `hookSpecificOutput`, and `permissionDecision` is absent; a denied call (an `rm -rf C:\` in a subagent) returns the deny and no `updatedInput`; a crash forced in `rewrite` returns the plain answer plus a `systemMessage`; a Bash call to `git status` returns no `updatedInput`; a main-session call that the media rule answers `ask` returns the `ask` and no `updatedInput`; `rm -rf C:\ && .\build_cpp.ps1` is denied with no `updatedInput`; `settings.json` holds no `bashOutputMaxChars` key; the fragment equals `settings.json`; `settings.json` has exactly one PostToolUse hook and it is `shell_output.py`; the README names `shell_output.py`; the preamble quotes `CAP`. Docs: the README changes in design point 10. Acceptance: `test_percall_key.py` (or the new file), `test_delete_rule.py` and `test_docs_numbers.py` pass with the commands in "Running the tests"; `test_docs_numbers.py` is run with `GATES_HYDRA_ROOT` pointing at SO-3's worktree, because the preamble check needs SO-3's text. If LS-S3 denied the rewrite, this task adds the registration and the README text for part 1 only, and the percall change is left out with one README sentence saying why.

### SO-3: the Hydra docs (Opus, Hydra repo, parallel with SO-1)

Owns `docs/agents/brief-preamble.md`. Line 5 gains the trim in its list of hook-enforced things. "Shells on Windows" gains one paragraph: a shell result over the cap arrives as its start, its error lines, its end and a path; read the file with Read and `offset`/`limit` or Grep it, never whole; a build or test command gets an exit-code line appended, so never end one with your own `exit N`, and read the `COMMAND FAILED` line as the failure. The number is written as `shell_output.CAP` is, with a thousands separator. Acceptance: `hydra_tests.exe "-tc=docs match code*"` passes on the branch (the main session runs it, rule 14); `test_docs_numbers.py` with `GATES_HYDRA_ROOT` at this worktree passes once SO-2's test exists.

### SO-4: the usage check, then the plan comes out (Opus, after the window)

Owns `gates\tests\shell_output_usage.py`, a new section in `gates\tests\probe_results.md`, and the removal of this plan from the Hydra repo. Writes the reader described in design point 8 on top of `shell_output_sizes.py`'s transcript reader, with tests on two fixture transcripts (one trimmed result with no read-back, one with a whole-file Read of the saved path) that pin the removed, read-back and net numbers. Runs it once when the window closes and records the numbers with the date and inputs. If the stop rule fires, the task ends with the report and the user's question 9 decision, and the plan stays until that is settled. Otherwise it removes `docs/superpowers/plans/2026-11-07-shell-output-to-disk.md`, and any line in the repo that names it gains `(removed; git show <commit>:<path>)`. Acceptance: the usage test passes; `hydra_tests.exe "-tc=docs match code*"` passes on the Hydra branch; the probe results section holds the three numbers and the verdict line.

## The live steps

All steps run `claude -p` on this machine's Claude Code with `--settings <a file in the session scratch folder>` and `--setting-sources project`, so the user's live hooks stay out, the way the T0 probes ran (`gates\tests\probe_results.md`, "How the live probes ran"; those ran under `--permission-mode bypassPermissions`, so `--permission-mode auto` and `--allowedTools` in LS-S3 and LS-S4 are new, P41). A throwaway hook script logs its raw input and emits what the step says. Each step has a cap of 2 runs and 10 minutes; past that it reports what it saw and stops. No step builds anything.

### LS-S0: today's behaviour (2 runs, 10 minutes)

No hook. A generator prints a first-line marker, a middle `ERROR` line and a last-line marker, 40,000 characters, exiting 0 then 1, in Bash and in PowerShell. Records the four results' lengths, whether the last line survived, whether a `persisted-output` path appeared, and whether the failed ones were marked errors. Settles P24, P25 and P26.

Calls: `Write` of the runner (seen: SO-PLAN's writes on this machine), `PowerShell` running `py -I <runner> baseline` (never tried), `Read` of its output (seen: SO-PLAN's reads).

### LS-S1: the trim alone (2 runs, 10 minutes)

A PostToolUse hook on `Bash|PowerShell` that trims as design point 3 says and saves below the ceiling. Passing 40,000-character and 6,000-character outputs in both shells. One of the 2 runs keeps `CLAUDE_CODE_ENTRYPOINT=claude-desktop` in the child's environment, the way probe run D did, so the PostToolUse input's `scratchpad_dir` and `transcript_path` are seen (settles P43); the other runs without it, which is the branch design point 4 says LS-S1 exercises. Records the `tool_response` keys seen (settles P22), that dropping the `persistedOutput` keys shows the hook's text rather than Claude Code's preview (P23), the shown length, and that the saved file holds the last line.

Calls: `Write` of the hook (seen: SO-PLAN's writes), `PowerShell` running `py -I <runner> trim` (never tried), `Read` of its output (seen: SO-PLAN's reads).

### LS-S2: the rewrite on a failing command (2 runs, 10 minutes)

The PreToolUse hook appends the tails from design point 2 with no `permissionDecision`; the PostToolUse hook from LS-S1 adds the `COMMAND FAILED` line. Failing 40,000-character output in both shells. Records whether the result was marked an error, its length, whether the first line, the error line and the last line survived, and the exit code shown. Settles P20 and P21.

Calls: `Write` of the hook change (seen: SO-PLAN's writes), `PowerShell` running `py -I <runner> rewrite` (never tried), `Read` of its output (seen: SO-PLAN's reads).

### LS-S3: the rewrite in auto mode (2 runs, 10 minutes)

The LS-S2 hooks, with `--permission-mode auto` on the `claude -p` call, and one failing build-like command (a Python launcher with `-m unittest` on a failing test file) in each shell. Records whether the call ran, and the denial text if it did not. Settles P28. A denial here stops part 2 (design point 7, question 8).

Calls: `PowerShell` running `py -I <runner> auto` (never tried), `Read` of its output (seen: SO-PLAN's reads).

### LS-S4: awkward shapes (2 runs, 10 minutes)

The LS-S2 hooks. One run each of: a command with a trailing `# comment`; `exit 3`; `Get-Item` on a missing file in PowerShell; `a && b` where `a` fails; a `claude -p` with `--allowedTools "Bash(python gen.py:*)"` and a rewritten `python gen.py`; and a hook output that carries `additionalContext` and `updatedInput` in one `hookSpecificOutput`; and a `claude -p` with a throwaway `permissions.deny` rule `Bash(python blocked.py*)` in the probe's settings file and a rewritten `python blocked.py` (a file that does nothing), the same prefix shape as the user's `gh` rules but running nothing real. Records what each showed. Settles P27, P29, P30 and P39.

Calls: `Write` of the hook change (seen: SO-PLAN's writes), `PowerShell` running `py -I <runner> shapes` (never tried), `Read` of its output (seen: SO-PLAN's reads).

## Out of scope (needs the user's yes to leave out)

- Trimming Read, Grep or MCP results. Read is the biggest share of tool-result characters (44.5% main, 63.8% agents), but the user's words chose a hook on shell output; a Read trim is its own plan.
- A way for the model to fetch a saved file back other than Read and Grep. A fetch tool is what the user ruled out.
- Deleting Claude Code's own `tool-results` files. Who deletes them is P31; if question 6 picks that folder, the sweep's change becomes a task.
- Changing how a failing command's output is capped in Claude Code itself. That is Claude Code's code, not ours.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| No limit goes up; no new MCP tool; no wrapper program | Rulings; The plain version | SO-2 | `test_docs_numbers.py`: `settings.json` holds no `bashOutputMaxChars` key and exactly one PostToolUse hook, `shell_output.py`; the reviewer reads the diff for any new tool or wrapper |
| The rewrite never sets `permissionDecision` | Design 2, 7 | SO-2 | percall test: the build-command answer has `updatedInput` and no `permissionDecision` key |
| A deny from any rule never carries `updatedInput` | Design 2 | SO-2 | percall test: a refused `rm -rf C:\` returns the deny alone |
| Rewriting cannot hide a command from the refusing rules | What was checked; Design 2 | SO-2 | percall test: `rm -rf C:\ && .\build_cpp.ps1` is denied by the delete rule, with no rewrite; the reviewer confirms `rewrite` is called after every refusal |
| `run_in_background` calls are never rewritten | Design 2 | SO-1 | `test_shell_output.py`: `rewrite` returns `None` when the flag is true |
| The status call is never rewritten | Design 2 | SO-1 | `test_shell_output.py`: the exact status call returns `None` |
| Only build and test commands are rewritten; one list owns which | Design 1, 2; question 4 | SO-1 | `test_shell_output.py`: `git status` returns `None`, `.\build_cpp.ps1` and `py -I -m unittest` and `git fetch && .\build_cpp.ps1` get the tail; grep of the repo finds the list once |
| A crash in the rewrite never refuses or changes a call | Design 2 | SO-2 | percall test: a forced crash in `rewrite` returns the plain answer plus a `systemMessage` |
| A crash in the PostToolUse hook leaves today's output in place | Design 3 | SO-1 | `test_shell_output.py`: a forced crash through `common.run(..., "loud")` prints a `systemMessage` and no `updatedToolOutput` |
| A passing result at or under the cap is untouched | Design 3 | SO-1 | `test_shell_output.py`: 200-character result returns `None` |
| A long result shows start, error lines, end and the path | Design 3 | SO-1 | `test_shell_output.py`: literal expected text for a `CAP + 1` input |
| The `persistedOutput*` keys are dropped from the hook's answer | Design 3 | SO-1 | `test_shell_output.py`: both keys absent, `interrupted` kept |
| Above the ceiling the hook reads Claude Code's file and saves nothing new | Design 3 | SO-1 | `test_shell_output.py`: with `persistedOutputPath` set, no file is written under the scratch folder |
| A failed rewritten command shows `COMMAND FAILED, exit code N` first | Design 3 | SO-1 | `test_shell_output.py`: literal first line for marker code 3 |
| Marker lines never reach the model | Design 3 | SO-1 | `test_shell_output.py`: the code-0 and code-3 cases hold no `[real exit code` |
| `NO_TRIM_COMMANDS` output arrives whole | Design 3; question 5 | SO-1 | `test_shell_output.py`: a long `git diff` returns `None` |
| Saved files have one home and an existing sweep deletes them | Design 4; question 6 | SO-1, SO-0 | `test_shell_output.py`: `saved_path` under the scratch field, else beside the transcript; SO-0 records which sweep covers the chosen folder (P31) |
| `settings.json` and the fragment carry the same hooks block | Design 9, 10 | SO-2 | `test_fragment_matches_settings` |
| PostToolUse has one registration | Design 3 | SO-2 | new test in `test_docs_numbers.py` modelled on `test_one_process_per_prompt` |
| Every number lives in the module's first lines and the docs quote it | Design 1, 10 | SO-1, SO-2, SO-3 | new preamble test for `CAP`; reviewer greps for the literals outside the module |
| The README and the preamble say what the hooks now do | Design 10 | SO-2, SO-3 | `README_NAMES` holds `shell_output.py`; the preamble test; `hydra_tests.exe "-tc=docs match code*"` |
| If auto mode denies the rewrite, part 2 stops, no workaround | Design 7; question 8 | SO-0, SO-2 | LS-S3's recorded line; SO-2's acceptance names the part-1-only path |
| The check after the change counts read-backs against removed characters, with a stop rule | Design 8; question 9 | SO-4 | fixture tests pin removed, read-back and net; the probe-results section holds the verdict |
| The finished plan leaves the repo in the merge that finishes the work | Design 10 | SO-4 | `hydra_tests.exe "-tc=docs match code*"` |
| Rollback is one revert, or one file checkout per part | Design 9 | SO-2 | the README's "Rolling back" section names both; the reviewer runs `git revert --no-commit` on a scratch clone and `git diff --stat` shows only the task's files |
| Each executor task fits 100 calls and 20 minutes | Tasks | all | `progress.py`'s warning line in each agent's report; a task that trips it is split by the main session |
| No scratch path in any committed text | Rulings | SO-0, SO-4 | the scratch-path check in `tests/test_docs_match_code.cpp`; the hooks-repo reviewer greps `probe_results.md` for `scratchpad` |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: `percall.py` is the only PreToolUse `*` hook, and a new per-call rule becomes a module it calls | code | `gates\README.md` "How a hook is registered"; `gates\settings.fragment.json` PreToolUse block | checked 2026-11-07 |
| P2: percall's subagent side runs progress, the build-pass refusal, then the shell rules, and returns the first deny | code | `gates\percall.py` lines 98 to 122 | checked 2026-11-07 |
| P3: every hook answer is one dict under `hookSpecificOutput`; `deny` sets `permissionDecision`, `context` sets `additionalContext` | code | `gates\common.py` lines 17 to 28 | checked 2026-11-07 |
| P4: `common.run` has two modes, `closed` (a crash denies) and `loud` (a crash prints a `systemMessage`), and always exits 0 | code | `gates\common.py` lines 48 to 71 | checked 2026-11-07 |
| P5: `delete_rule.check` reads `tool_input.command` raw, splits at `OPERATORS`, reads each piece's first word with `_word` | code | `gates\delete_rule.py` lines 29, 66 to 69, 152 to 162 | checked 2026-11-07 |
| P6: `bash_windows_rule.check` reads `tool_input.command` raw; `MAX_CHARS` is 1000 | code | `gates\bash_windows_rule.py` lines 16, 237 to 244 | checked 2026-11-07 |
| P7: `progress.SHELL_TOOLS` is Bash and PowerShell; `is_status_call` names the one uncounted shell call | code | `gates\progress.py` lines 20 to 34 | checked 2026-11-07 |
| P8: `settings.json` and the fragment hold the same `hooks` block, and no PostToolUse registration | code | `gates\settings.fragment.json` lines 65 to 206; `gates\tests\test_docs_numbers.py` `test_fragment_matches_settings` | checked 2026-11-07 |
| P9: `settings.json` sets `permissions.defaultMode` to `auto` | code | `C:\Users\Patrick\.claude\settings.json` line 14 | checked 2026-11-07 |
| P10: the fragment's allow rules are two PowerShell prefix rules for `status_append.ps1` and `publish.ps1` | code | `gates\settings.fragment.json` lines 3 to 6 | checked 2026-11-07 |
| P11: a hooks merge runs the whole suite when it touches `settings.json` | code | `docs/agents/agent-rules.md` rule 1 | checked 2026-11-07 |
| P12: the sweep removes scratch folders of ended sessions after `$ScratchAgeDays` (7) and never a running session's | code | `gates\cleanup_sweep.ps1` header and lines 48 to 58 | checked 2026-11-07 |
| P13: the sweep does not touch Claude Code's `tool-results` folders | code | `gates\cleanup_sweep.ps1` header, "What it deliberately leaves alone" and `Invoke-ScratchSweep` | checked 2026-11-07 |
| P14: hooks are registered in exec form with `python.exe -I -S <script>` | code | `gates\README.md` "How a hook is registered"; `gates\settings.fragment.json` | checked 2026-11-07 |
| P15: the merge recipe is merge `main` into the branch, run the merge's tests, fast-forward; rollback is `revert --no-commit --no-merges` or a file checkout | code | `gates\README.md` "Making a change live", "Rolling back" | checked 2026-11-07 |
| P16: the hook tests run with `python.exe -I -m unittest discover -t . -s gates/tests -p <file>` | code | `gates\README.md` "Running the tests" | checked 2026-11-07 |
| P17: `test_docs_numbers.py` checks the preamble quotes `MAX_CHARS`, the README names `README_NAMES`, and one prompt hook | code | `gates\tests\test_docs_numbers.py` lines 50, 120 to 152 | checked 2026-11-07 |
| P18: the build command is `$env:HYDRA_BUILD_PASS='<pass>'; .\build_cpp.ps1 -Target ...`, and `build_slot.ps1`, `build_installer.ps1`, `mutation_probe.ps1` build too | code | `docs/development.md` lines 112 to 136; `build_cpp.ps1` lines 1 to 10 | checked 2026-11-07 |
| P19: the Hydra test tools are `hydra_tests.exe`, `hydra_uitest.exe`, `hydra_replay.exe`, `hydra_precheck.exe`, and `py -m pytest` for the Python tools | code | `docs/agents/agent-rules.md` rules 1 and 14; `docs/development.md` line 292 | checked 2026-11-07 |
| P20: the Bash tail `echo "[real exit code $?]"` makes a failing command exit 0 and the tool save its output | unchecked | the `SO-PLAN` brief reports it from a throwaway run | SO-0, step LS-S2 |
| P21: the PowerShell tool reads `$LASTEXITCODE` to decide failure, so the tail must set it to 0 and turn `$?` false into 1 | unchecked | the `SO-PLAN` brief reports it from a throwaway run | SO-0, step LS-S2 |
| P22: a PostToolUse `tool_response` for a result over the ceiling carries `persistedOutputPath` naming the saved file | unchecked | the `SO-PLAN` brief; the hooks page `https://code.claude.com/docs/en/hooks` names no such field | SO-0, step LS-S1 |
| P23: an `updatedToolOutput` that keeps `persistedOutput*` keys makes Claude Code show its own preview; dropping them shows the hook's text | unchecked | the `SO-PLAN` brief reports it from a throwaway run | SO-0, step LS-S1 |
| P24: a passing result is inline up to about 30,000 characters, then a saved file plus a 2,000-character preview | unchecked | `https://code.claude.com/docs/en/tools-reference`, "Output limits" (a vendor page, not one of the six kinds) | SO-0, step LS-S0 |
| P25: a failed result is inline up to about 10,000 characters, then a head-and-tail excerpt with no file | unchecked | `https://code.claude.com/docs/en/tools-reference`, "Output limits" | SO-0, step LS-S0 |
| P26: a PostToolUseFailure hook gets only the excerpt in `error`, and its `updatedToolOutput` has no effect | unchecked | the `SO-PLAN` brief; the hooks page lists only `additionalContext` for that event | SO-0, step LS-S0 |
| P27: a narrow allow rule such as `Bash(python gen.py:*)` denies the rewritten command | unchecked | the `SO-PLAN` brief reports two denials | SO-0, step LS-S4 |
| P28: a settings command hook may change a Bash or PowerShell call's input in auto mode without the "a hook changed this call's input" denial | unchecked | `https://code.claude.com/docs/en/hooks`, "PreToolUse updatedInput in auto mode" | SO-0, step LS-S3 |
| P29: a trailing `# comment` keeps the tail working; `exit 3` skips it; `Get-Item` on a missing file reports exit 1 | unchecked | the `SO-PLAN` brief reports it from a throwaway run | SO-0, step LS-S4 |
| P30: `additionalContext` and `updatedInput` work together in one `hookSpecificOutput` | unchecked | the hooks page `https://code.claude.com/docs/en/hooks` describes each alone; nothing shows both | SO-0, step LS-S4 |
| P31: who deletes Claude Code's `tool-results` files, and after how long | unchecked | `C:\Users\Patrick\.claude\settings.json` sets no `cleanupPeriodDays`; the oldest file in the project's folders is dated 2026-10-14 | question 6 |
| P32: the scratchpad_dir field is present in every desktop-hosted session and names the session's scratch folder; absent in plain `claude -p` | code | `gates\README.md` "What the probes found" | checked 2026-11-07 |
| P33: a 4,000 cap would have removed 8.6% of tool-result characters (5,516,167 of 64,247,182) over the last 40 main and 300 agent transcripts; 7 of 676 failed shell results sat at the 10,000 ceiling | unchecked | the two size scripts run by SO-PLAN on 2026-11-07 against `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test` | SO-0 |
| P34: a Python hook start costs 15 to 27 ms bare and 36 to 65 ms for `percall.py` on this machine | unchecked | `Measure-Command` runs by SO-PLAN on 2026-11-07 | SO-0 |
| P35: nothing in the gates reads a live call's `is_error`; the session-card scripts read tool-result text only | code | `gates\tests\replay_extract.py` line 120; `gates\stall.py` lines 167 to 175; `C:\Users\Patrick\.claude\tools\session_cards\common.py` lines 334 to 370 | checked 2026-11-07 |
| P36: a plan dated 2026-11-01 or later needs a Premise map, `## Questions` numbering and `Calls:` lines, and may cite no scratch path | code | `tests/test_docs_match_code.cpp` lines 1290, 1515 to 1520 | checked 2026-11-07 |
| P37: `updatedToolOutput` must match the tool's output shape or it is ignored; the Bash shape is `stdout`, `stderr`, `interrupted`, `isImage` | unchecked | `https://code.claude.com/docs/en/hooks`, "PostToolUse decision control" | SO-0, step LS-S1 |
| P39: Claude Code's `permissions.deny` prefix rules still refuse a command whose input a PreToolUse hook rewrote by appending a tail | unchecked | `C:\Users\Patrick\.claude\settings.json` `permissions.deny` holds only prefix rules on `gh`; no source says which input they judge | SO-0, step LS-S4 |
| P40: a PostToolUse `updatedToolOutput` changes what the model gets; whether the user's tool display shows the hook's text or the original is unknown | unchecked | the hooks page `https://code.claude.com/docs/en/hooks`, "PostToolUse decision control", says it replaces the output "sent to Claude" and names no display | question 1 |
| P41: `claude -p` accepts `--permission-mode auto` and `--allowedTools` together with `--settings`, and `auto` there judges calls as the desktop session does | unchecked | `gates\tests\probe_results.md` "How the live probes ran" used `bypassPermissions` only | SO-0, step LS-S3 |
| P42: at an 8,000 cap 145 of 2,972 main-session shell results are trimmed and 4.1% removed; at 2,000, 816 and 17.0%; shell results are 35.5% and 18.1% of tool-result characters, Read 44.5% and 63.8%; 111 `tool-results` folders hold 838 files and 169 MB | unchecked | the two size scripts and a listing of `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test`, run by SO-PLAN on 2026-11-07, never committed | SO-0 |
| P43: a PostToolUse hook's input carries `transcript_path` and, in a desktop-hosted session, `scratchpad_dir`, like PreToolUse | unchecked | `gates\tests\probe_results.md` probe 3 saw `scratchpad_dir` on PreToolUse, SubagentStart, SubagentStop and Stop only; no PostToolUse record was captured | SO-0, step LS-S1 |
| P38: the user chose "Hook and build rewrite" and ruled "so instead of saving you want to waste even more usage? no. unacceptable." | ruling | the `SO-PLAN` brief, as the main session quotes the user; the main session confirms at review | checked 2026-11-07 |
