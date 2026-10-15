# Handoff: Hydra's MCP server (Claude drives the GUI as text)

Written 2026-10-14 by session 719dc0ea-0e1c-49fb-871e-a62dab4d6c95, when the user said "stop after this review and create a handoff". The plan is `docs/superpowers/plans/2026-10-14-hydra-mcp-server.md`. The plan wins where they differ. Its "T8: one owner for every command rule" section is the current spec for the protocol, and this file adds what happened around it.

## What this is

Claude drives Hydra's GUI through an MCP server, by widget label and as text, instead of taking screenshots. It has two halves:

- **The C++ side.** This is `hydra_uitest --live` for the headless mode, and a dev build's `Hydra.exe --live` for the real window. Each takes one command line at a time over a named pipe and answers with one JSON line. Both are built on the test engine's command interpreter in `tests/ui/uitest_commands.cpp`.
- **The Python helper.** `tools/hydra_ui_mcp.py` (on `mcp` 2.3.0) is registered in the project's `.mcp.json`. It launches the right exe, carries messages, and in window mode copies the user's library to a temp snapshot first.

The real `hydra.db` is never opened. A temp-folder guard in the live test refuses any seed outside temp.

## Where things stand

Main is at ff2c03a3. Other sessions keep committing there; the last MCP merge is 56aeeca1 (T5), and the T8 spec is 771d3313. Nothing is pushed. No agent is running.

**T8a is reviewed but not merged.** The user stopped the session right after its review. The branch is `claude/mcp-t8a`, worktree `.claude/worktrees/mcp-t8a`, with its signed-off tip at **06079ec22ada4a62876942ed0787f117934691b4** (Verdict CLEAN, gate submit accepted). It was not merged on purpose. Merging it alone breaks the helper's real-exe tests on main, because the helper still speaks the old protocol, so merge it and start T8b in the same sitting.

The reviewer fixed three things above the author's 441ff939:

- `parse_seconds` now reads through `hydra::parse_finite_number`, the existing strict number reader. One behaviour change: `+5` is now accepted. A scan row was added in `concept-F3.json`.
- One `uitest::pipe_problem` decides whether `--live` and `--pipe` fit together, and both mains call it. A scan row was added in `tests.json`.
- A comment in `smoke.txt` now names the verb instead of restating the rule.

The reviewer also confirmed the hidden panel against its real title, "Dear ImGui Test Engine" (`third_party/imgui_test_engine/imgui_te_ui.cpp`). Nothing matches it under `--live`, and its Stop and Break items show under `--uitest`.

Its tests: `test_live_protocol` 10 of 10, single-owner 17 of 17, smoke PASS, and live runs on both exes. Run `main_move_check.ps1 claude/mcp-t8a` before the merge, since main has moved. After the merge, build and run the full suite once.

**Two questions for the user from the T8a review. Ask them before T8b.**

1. **Should `type <ref> |` with nothing after the bar clear the field, as the old interpreter did?** Today, after T8a, only `type <ref> | ` with a trailing blank clears; the bare form is an error. Recommended: yes, the bare form clears too. It is one case in `split_bar`. The reason is that a client which trims trailing blanks would otherwise lose the only way to clear a field.
2. **Line ends disagree.** Unfiltered `text` keeps the screen text's `\r\n`, while `text | x` strips the `\r`. That is the same question answered two ways, so by the user's rule it gets fixed rather than documented. Recommended: strip the `\r` in one place, where the screen text becomes lines, so every `text` reply uses `\n`. This is a small C++ change. Do it as its own reviewed fix before T8b, or fold it into T8b with `uitest_harness.cpp` added to T8b's owned files.

Other notes from the review for T8b:

- `crash_dump` is an empty string for `Hydra.exe`, and `log` is an empty string for `hydra_uitest` (stdout). The helper must treat empty as "none".
- The helper must send the trailing blank in `type <ref> | ` when clearing.
- The new rules (strict numbers, the wait cap, the filters, `rect` failing) are pinned in C++ only on the wire format. T8b's real-exe tests should pin their output as literals.
- A filtered `dump` matches the whole item line, so `dump | rect` keeps every item. That is as specified.

## Merged to main this session, in order

Every merge below had one fresh Sonnet reviewer and passed the derive-once gate.

- **T1** at 5343a673. The interpreter was split into `run_command`. It added the verbs `key`, `rect`, `reset` and `quit`, the `--ini` seed, and `is_under_temp`.
- **T3** at 15035589. The Python helper, the stand-in and `.mcp.json`.
- **T2** at 40be9d49. The live pipe channel, which is overlapped so reads and writes never block each other. The hello line carries the timeout, the engine watchdog is off for live, and a seed guard was added.
- **T6** at 7e4506ee. The helper was run against the real exe, `realpath` was added to the running-Hydra check, and a stale crash log is never quoted.
- **T5** at 56aeeca1. `Hydra.exe --live` drives the real window, closes when the client leaves, and deletes its scratch folder. It also fixed the `LivePipe` shutdown race with a stop event and added `uitest::seed_problem`.

The plan commits are f38d07e9, 283273be, fe919b15, bcf2d2f0, 607806a6 and 771d3313.

**Full suite on main at 771d3313** (the T5 merge, plus another session's D115 A1-6a/b merges):

- hydra_tests: 1397 of 1397 test cases. That is 12 fewer than after T2, because D115 deleted the old upgrade tests.
- hydra_uitest --all: 99 of 99.
- `py -I tools/test_hydra_ui_mcp.py` with `HYDRA_LIVE_WINDOW_TEST=1`: 38 of 38.

The ship preset's `Hydra.exe` holds no `uitest` or `--live` text. I checked it myself, in ASCII and in UTF-16. A reviewer's claim that it did came from a stale exe.

## Why T8 exists (read this before touching the protocol)

The first build split C++ and Python without saying which side owned what. The helper then copied Hydra's rules: it parsed `timeout` and `wait` with `float()` where Hydra uses `atof`, skipped blank lines, numbered lines, filtered dumps, defaulted `dump` to "Hydra" and labelled failures. The stand-in copied the reply wording a third time.

The user pushed back twice. First: "you think giving claude conflicting numbers is a good idea? that is just going to confuse the model". Then: "why are you still letting different areas do their own thing? this is why everything needs to be derived once".

A scout's boundary audit listed 12 copies. The plan's T8 section now holds the ownership table and the new protocol. Hydra owns every command rule. The helper is transport only, and the stand-in owns only hang, exit, close and oversized replies.

The precheck compares within one language only, so cross-language copies pass it. Check the T8 table by hand in every review.

## What is left, in order

1. **Ask the user the two T8a questions, then merge T8a.** Its reviewed tip is 06079ec2. Merging it breaks the helper's real-exe tests on main until T8b lands, so start T8b straight after. Fix the line-end question first, or in T8b, as the user decides.
2. **T8b (Python), forked from the T8a merge.** Spec: the plan's T8 section, task T8b. The helper passes the full pipe path with `--pipe` (Hydra never adds the `\\.\pipe\` prefix), and reads `text` and `timeout` from every reply and `log` and `crash_dump` from the hello line. It stops parsing verbs and arguments, labelling, filtering and defaulting. One-off timeouts restore the reported old value. The stand-in shrinks to transport-only behaviours. Every test the audit listed as movable runs on the real headless exe. A new test runs one command before a `hydra_run` and checks the reply carries exactly one line number, the helper's.
3. **T7: docs and the ADR.** It covers `docs/agents/ui-testing.md` (the live mode section, the tool table, the new verbs and flags, the T8 protocol), one sentence in CLAUDE.md's GUI testing section, and ADR 0030. The ADR must record the ownership boundary as the main decision, besides the pipe, Python and the two-copies-with-a-guard rule. The plan's line 129 still shows the old `.mcp.json` env block; T6 dropped it, so fix that text too.
4. **The real-app check, with the user watching.** Close the installed Hydra, then run `hydra_start("window")` from a Claude Code session on `C:\Program Files\Hydra`. Open a song, flip 2x Bass and back, narrow the library with a search to a few charts and run `Analyze shown (N)...`, then `hydra_stop`. Show that `C:\Program Files\Hydra\hydra.db` keeps its size (17,993,728 bytes at 18:04 on 2026-10-14 by `ls`; check again before) and its modification time. Never analyze the whole library; that is a library run and needs the user's yes (CLAUDE.md rule 8).
5. **Push.** Only when the user says so.

## User decisions made this session

These are all recorded in the plan's "Answered" paragraphs:

- **The build's scope:** both modes; full control on a copy of the library; dev builds only; a project `.mcp.json`.
- **The plan's ten open questions** were answered as recommended. That includes installing `mcp`, one session at a time, refusing to snapshot while Hydra runs, a 15 s margin, a 60 s hello cap and a 12,000-character reply cap.
- **Timeouts and caps:** a one-off timeout on a wait lasts for that call only. The 15 s margin is also the exit wait after `quit`. `LOG_TAIL_LINES = 20`, `CONNECT_RETRY_S = 0.05` and the stand-in's `HANG_CAP_S = 600` stay.
- **Line numbers:** one owner per line number. Live replies carry none, and the helper numbers `hydra_run` scripts.
- **The engine panel** is hidden in live mode.
- **Numbers belong to Hydra:** every reply carries the timeout in force. Numbers parse strictly in both modes. In live mode, a `wait` longer than the timeout is refused.
- **The ownership boundary** in the plan's T8 table.

## Machine state

- `mcp` 2.3.0 is installed into the system site-packages. A second copy of `cryptography` (50.0.1, with cffi 2.1.1 and pycparser 3.0) is there as well, because `py -I` can't see the user site-packages.
- Scratch folders were left by `--keep-temp` and `keep=True` runs: `%TEMP%\hydra_test_uitest_55088`, `%TEMP%\hydra_live_61820_1` and `%TEMP%\hydra_live_48300_1`. The 7-day cleanup sweep removes them.
- Review files are in this session's scratchpad `reviews\`. They are temporary; their findings are summarised above.

## Notes from reviews that are not done

- A `--uitest` run (not `--live`) with a bad seed leaves an empty window open and exits 0. That matches the existing "no test or script file" case.
- Three older uitest registrars still repeat the registration lines and could call `register_test`: `uitest_tests.cpp`, `uitest_paths.cpp` and `uitest_report_windows.cpp`.
- A stalled reply write (`kWaitCap`) ends the live session without telling the helper why, so the helper only sees a closed pipe.
- Two connect waits nobody reconciles: Hydra waits `kUitestProcessCap` (5 minutes) for a client, while the helper gives up at 60 s. The helper owns the connect question as transport.
- CI does not run `tools/test_hydra_ui_mcp.py`; it would need `pip install mcp` first.

## Journal state at handoff

The journal hook printed these lines when the user asked for the handoff:

```
Subagents active in the last 30 minutes and not finished:
  agent-a060443c6d57a1353: unfinished, last tool call SubagentHandback, touched 21:37
  agent-a2d15f0c2f34134ef: unfinished, last tool call PowerShell, touched 21:43
  agent-ab9af276bb4e73601: unfinished, last tool call SubagentHandback, touched 21:19
  agent-ae850a30dadc0b2a5: unfinished, last tool call SubagentHandback, touched 21:19

What those are: a060443c is the T8a executor, ab9af276 the boundary audit scout, ae850a30 the T5 reviewer, and a2d15f0c the T8a reviewer. All four have handed back their reports, so none is still running. The "unfinished" mark is only the journal's view.
```
