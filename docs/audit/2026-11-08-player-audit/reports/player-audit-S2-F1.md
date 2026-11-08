# player-audit-S2-F1: built is not source (family 1), tests that test fakes (family 7)

Read-only. About 40 tool calls. Main checkout at fd93c684.

## C1. The built kit.js is older than kit.ts and cannot do split chords (proven)

Homes: `tools/ch_mcp/agent/kit.js` (main checkout, git-ignored, written 2026-11-06 21:31) and `tools/ch_mcp/agent/kit.ts` (commit 2709008f, "Player limits T3", 2026-11-07 23:11; file time 11-07 23:20).
kit.ts has `groupsOf`, `then`, `after_ms` and a per-group `groups` reply. kit.js has none of them: counts in kit.js are 0 for `after_ms`, `groupsOf` and `"then"`. By reading (grep of both files).

What each does: kit_tools.press_message (kit_tools.py:326-337) puts `then:[{elements, after_ms}]` in the message. play_chart.play_chords (play_chart.py:1120-1122) sends a split chord as one message with `then=`. The old kit.js ignores `then`, presses only the first group, and replies with no `groups`.

Proven by a run: `ls-m2-run1/press-2.csv` (written 12:29 on 11-08, after T3 landed) has 13 chord indexes that appear twice (e.g. 10 and 1064). The second row of each pair has no down_frame, up_frame, down_ms or up_ms. play_chart.py:1166 only uses `groups` when the reply has it, so the stale reply is accepted without a word. Whether the second group was pressed in the game is not shown by the CSV. By reading kit.js it was not.

| input | answer |
|---|---|
| kit.js older than kit.ts | nothing refuses; no log line says which kit.js ran |
| `then` sent to old kit.js | ignored, reply has no `groups`, rows for later groups blank |
| `then` empty | works in either build |
| agent file missing | refused loudly (bot_run.py:631, trace_hits.py:299, method_watch.py:437) |

Nothing compares the .js with the .ts. Searched kit_tools.py, frida_attach.py, frida_tools.py, bot_run.py, trace_hits.py, method_watch.py for hash, mtime, sha, version, stale: only the game build number (build_facts) and "agent missing" exist. The ch_reload reply (front.py) lists pids, times and tool-list differences, not the agent files. Open: whether the LS-M runs' misses at split chords came from this. A run with the new kit.js would settle it.

## C2. The tests for kit.ts run the .ts, never the built .js

Home: `tools/ch_mcp/tests/test_kit_tools.py:1247-1271`. It reads `kit.ts` (types erased by Node) and runs it against a fake il2cpp. So the T3 tests pass while the .js that actually loads is stale (C1). The test is also `skipIf(node is None)`, so a machine without node skips them quietly. The frida-compile step (bundling, `import "frida-il2cpp-bridge"`) is not covered. By reading.

## C3. package.json has one build script per agent and no build-all, no freshness check

Home: `tools/ch_mcp/agent/package.json` (build:clock, trace, state, menuhooks, kit, launch). Nothing builds after a merge, and nothing lists which .js are older than their .ts or than an imported .ts (gamemanager.ts, hooks_common.ts, menu.ts, launch_core.ts are bundled into several agents). By reading.

Current state of each built file against its sources (file times and `git diff a41101fc HEAD`):

| .js | built | newer source? |
|---|---|---|
| kit.js | 11-06 21:31 | yes, kit.ts T3 (C1) |
| launch.js | 10-31 13:17 | launch.ts and launch_core.ts changed 11-07; the diff is comments only |
| menuhooks.js | 10-31 10:14 | menuhooks.ts, menu.ts 11-07: comments only |
| state.js | 10-31 11:06 | state.ts 10-31 10:11, menu.ts comment: current |
| trace.js | 11-07 17:11 | has `round_start_double`, not `hit_time_double`: current for the D317 rename |
| clock.js | 10-30 21:08 | clock.ts, hooks_common.ts older: current |

## C4. The hydra_replay exe is not tied to the source it wrote JSON from

- `tools/ch_probe/prep_inputs.py:45` defaults to `<repo root>/build-cpp/Release/hydra_replay.exe` and prints the exe's file date (line 85). It does not compare it with source. That is the one place a run logs which build ran.
- Main checkout `build-cpp\Release\hydra_replay.exe` is dated 11-08 11:14. `tools/replay.cpp` (commit 3b23d8f1, 11:32) and `tools/replay_schedule.cpp` (12:26) are newer. The kick-prep JSONs (11:16) came from the 11:14 exe. The exe has no `schedule` command if it predates the schedule code (by file dates only; not run).
- A worktree has its own build-cpp, so `--exe` default and `_REPO_ROOT` change with where the script is run from. `.mcp.json` runs `tools/ch_mcp/front.py` by a relative path, so the worker comes from whichever checkout the session started in; a worktree has no built .js at all (git-ignored), which fails loudly.
- The score/dump JSON carries `path_source` (file, index) but no exe stamp or version, so a run folder cannot show which exe wrote it.

## C5. A second home for press planning: `hydra_replay schedule` is not read by the player

Home: `tools/replay_schedule.cpp:962-965` writes `decline`, `chords`, `presses`. `tools/ch_probe/experiments/play_chart.py` reads only the `score` JSON (play_chart.py:700-930) and plans presses itself (`press_plan`, `_order_groups`, `_chord_presses`). Searched play_chart.py, prep_inputs.py, game_tools.py for "schedule" and "presses": no reader of the schedule JSON. So the two plans can differ and nothing compares them. This is a split (two homes for one question), by reading. It may be a planned later step (plan 2026-11-08-replay-press-schedule.md); I did not check the plan.

## C6. Fake shapes against the real thing (family 7)

| fake | real | difference |
|---|---|---|
| `_SplitReplyPress` (test_play_chart.py:1482-1496) | kit.ts reply (kit.ts:647-656) | the fake's group entries lack `after_ms`; the fake reply lacks `rewrite` and `on_main`. Fake `down_ms` is 9000+, real is epoch ms. No test sends `then` and gets back a reply without `groups` (the stale-kit case in C1). |
| `PressReplyRowTest` fake reply (test_play_chart.py:675-678) | same | adds `rewrite` and a `hit` key to test filtering; fine, but no `groups`. |
| score fixtures `*_score.json` in tools/ch_probe/tests | `hydra_replay score` (replay.cpp:427-485) | hand-trimmed: top level has only path_source, activations, chords; the real one also has chart, chartmode, final, sections, warnings. Chords drop is_fill, is_solo, measure, combo_before, points, cum. play_chart reads none of the dropped ones (by grep of `["..."]` in play_chart.py), so no break today. Activations carry `sqout_tick` only when set (replay.cpp:432); the fixtures match that. |
| dump fixtures `*_dump.json` | replay_json.cpp:156-167 | act fields match what play_chart reads (act_tick, deact_tick, nominal_deact_tick, sqout_tick, sqin_ticks, sqinouts, end_anchor_tick, steady_ms_*). |
| `senri_end_chords.json` | score JSON | dated 11-06 22:04, earlier than the other fixtures; it has only activations and chords, no path_source. Not checked whether any test needs path_source. |
| real run `launch-*.jsonl` | launch.ts | not compared with the fakes in test_bot_run.py (not reached). |

Edges with tests, checked: the late-press drop at exactly 0.01 ms past the edge (test_play_chart.py:2264-2270) has one. Edges with none that I found: a `then` send answered by a reply with no `groups`; a `groups` list shorter than 1+len(later) (play_chart.py:1186 silently leaves the row blank); a missing kit.js at the ch_play_chart level (covered in bot_run only for launch.js).

## Not reached

test_bot_run.py, test_frida_attach.py, test_trace_hits.py and test_server.py fakes against launch.ts, trace.ts and server replies; test_sp_tools.py, test_game_tools.py, test_menu_tools.py fakes; machine_recorder/report and compare_scorestats fakes; the C++ schedule tests; data files kit_methods.json, menu_fields.json and friends against the agents. sys.path tricks: every script inserts its own repo root (`_REPO_ROOT` from `__file__`), so a script run from a worktree loads that worktree's modules; not found loading a different checkout. Python modules cached by the long-running worker: ch_reload starts a new worker process (front.py:180 Popen), so server-side modules refresh; front.py itself and the tool list text refresh only at reconnect (front.py:78 notes this). I did not read the reload code line by line.
