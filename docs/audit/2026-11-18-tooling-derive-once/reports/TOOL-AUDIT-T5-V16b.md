# TOOL-AUDIT-T5-V16b (verifier of F16b). All claims "by reading". Gates = C:\Users\Patrick\.claude\gates; Hydra = ...\Downloads\Hydra\hydra-test.
Candidates reached: the four splits, the K6 pairs, the main K1 copies. Rows not reopened (finder's words, unverified by me, no claim either way): I9-177, I9-181, I10-080, I10b-013, I10b-029, I10b-045, I10b-068, I10b-073, I10b-010, I10b-075, I10b-104.

## I10b-099 squeeze window: CONFIRMED-CHANGED
- Owner: Hydra src/core/model.h:121 `constexpr double kSqueezeWindowMs = 500.0;`, within_squeeze_window at :133. graph.h:216 only mentions it in a comment.
- Split A: the skill is not in the Hydra repo; the live one is C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10: "within 500 ms of a deactivation node (`kSqueezeWindowMs`, src/search/graph.h)". Wrong owner path (K8). CONFIRMED. (Finder wrote "skills/analyzing-sp-squeezes" without saying it lives in ~/.claude; worktree copies exist too.)
- Split B: Hydra docs/clone-hero/05-star-power.md cites "src/core/model.h line 111" (:417, :419) and "line 123" for within_squeeze_window (:399, :417). Code has 121 and 133. Stale line numbers (K2). CONFIRMED. Finder missed that the cite appears at lines 399, 417 and 419.
- Situation: any reader who follows the pointer. Same for all agent types.
- Ruling: none found naming either side (grepped rulings for squeeze: nothing).

## I10b-067 capture start wait: CONFIRMED
- tools/capture_window.ps1:56 `function Start-Hydra([string]$Dir, [int]$Wait = 12)`; tools/capture/shot.ps1:2 `[int]$Wait = 10` and :4 `Start-Hydra $Dir $Wait`; steps.ps1:8 and report_shot.ps1:10 `Start-Hydra $Dir 12`.
- Split: shot.ps1 waits 10 s, the others 12. Situation: a slow-starting Hydra launched through shot.ps1 with no -Wait. Open run: a capture on a slow machine.

## I9-183 status-call reader: CONFIRMED
- gates/progress.py:22 STATUS_PREFIX names `...gates/status_append.ps1`; progress.py:41 is_status_call is the owner reader. gates/tests/replay.py:279-280 `_status_call` regex `(hooks|gates)/status_append\.ps1`: a second reader that also accepts `hooks`.
- Split: replay accepts a `hooks` folder, progress does not. Replay reads recorded calls, so the extra word may cover old recordings; that is a claim, not read. replay.py:221 itself calls progress.is_status_call, so both readers run in one file.
- Status path as copy: settings.fragment.json:4, progress.py:22, replay.py:280 agree today (live settings.json:10 not reopened by me).

## I9-180 ask rules: CONFIRMED (K1 + K6), no split today
- Fragment gates/settings.fragment.json:59-64 ask: `Bash(rm *)`, `Bash(git clean *)`, `Bash(git reset --hard*)`, `PowerShell(Remove-Item *)`. replay.py:87-88 `MAIN_ASK = {"Bash": ("rm ", "git clean", "git reset --hard"), "PowerShell": ("remove-item ",)}`, comment "as command prefixes". Live settings not reopened by me.
- Prefix shapes differ (`git clean` bare vs `git clean *`): `git cleanfoo` would match replay but not the rule. Edge only.
- Holding equal (finder, not opened): fragment vs live by test_docs_numbers and sync_settings. Nothing ties MAIN_ASK to either.
- K6 pair: replay's prefix reader vs delete_rule's discard sets (discard sets not opened by me).

## I9-070 media guard cutter: CONFIRMED (K6)
- Owner: gates/delete_rule.py:75 `pieces`, docstring "the one reader of where a command is cut": splits with OPERATORS.split anywhere in the string.
- media_rule.py:18-27 `_segments`: its own loop; cuts only where an operator stands alone as a word (OPERATORS.fullmatch(token)) and turns newlines into " ; ". Its comment says an operator inside a quoted filter ("a;b") must not split. So on `a;b` with no spaces, pieces cuts and _segments does not. Real behavioural difference by reading. A run would be needed to show a missed write: give both `ffmpeg -i x -vf "a;b" out.mp4`.
- pieces' own docstring claim ("the one reader") is contradicted by _segments.

## K1 copies checked
- Game folder: owner tools/ch_mcp/game_files.py:31; copies README.md:12, :58, dump_build.py:9 docstring. Agree today.
- 12,000 cap: hydra_ui_mcp.py:44 `REPLY_CAP_CHARS = 12_000`; test_hydra_ui_mcp.py:293 literal "[cut at 12,000 characters: {1000 - kept} of 1000 lines dropped;". Agree today.
- 500 ms: skill, memory note, SP-38 (above).

## Not checked
Whether a hook refuses agents running hydra_replay (percall.py not read); the docs check; every ruling file in full (I grepped for squeeze, status, capture, replay only; no Confirmed line found on these).
