# TOOL-AUDIT-T5-V11b (checks F11b). All claims "by reading"; nothing run.

## I8-255: CONFIRMED-CHANGED (counts only)
M top-level notes hold 66 [[..]] occurrences, 39 distinct; none missing from M or M\archive (the finder's "66 links" are occurrences). "(archived)" tags: 14 distinct targets (finder said 19, likely occurrences). skills/analyzing-sp-squeezes/SKILL.md:34: "see memory `sp-squeeze-mechanics`, `gameplay-video-score-analysis`". sp-squeeze-mechanics.md is live; gameplay-video-score-analysis.md exists only in M\archive. Kind K8-ish. No value split.

## I9-178 and I10-056: UNCHECKED beyond one line
harness.ps1:5 read: "This file is the one home of how a test runs a hook...". I did not reopen the Python tests that copy hooks or the build-cpp\Release copies; both finder claims stand unverified by me. Not reached.

## I10-057: CONFIRMED (partly reread)
cleanup_sweep.ps1:54 `$ScratchAgeDays = 7`. hydra_ui_mcp.py:67 SCRATCH_SUBFOLDER = "claude"; :306 gettempdir()/SCRATCH_SUBFOLDER; :600 comment "gone a week with no writes; $ScratchAgeDays in cleanup_sweep.ps1 owns"; :770 "after a week". game_tools.py:47 SCRATCH_PREFIX = "ch_live_"; :599 scratch root = tempfile.gettempdir() (Temp itself, not Temp\claude); :1402 ch_live_<pid>_<n>.
Splits: (a) hydra_live_ under Temp\claude vs ch_live_ directly under Temp (different roots; whether anything removes ch_live_ I did not search). (b) "a week" prose vs 7 in code (agree today).
Not reopened: Invoke-ScratchSweep's age rule, ui-testing.md:213.

## I10-073: CONFIRMED
delete_rule.py:17-18 WORKTREE_FOLDERS = (H\.claude\worktrees, C:\Users\Patrick\.claude\worktrees). cleanup_sweep.ps1:174-178: `<root>\.claude\worktrees` for any repo, plus `<root>\worktrees` only when the repo root equals the hooks repo. README.md:136: "under `C:\Users\Patrick\.claude\worktrees\<task>`... Keep it under a `worktrees` folder".
Splits: S1a this audit's worktree path is H\worktrees\tooling-audit-records, which neither delete_rule.py's Hydra entry nor the sweep's Hydra prefix names. S1b README.md:136 names only the hooks-repo folder. S1c the hooks-repo folder is typed twice (sweep, delete_rule) with no test ties found (test_delete_rule.py not reopened by me).
Ruling rulings/review-and-hooks.md:17 not reopened; the finder's quote is unchecked and per the finder names neither H\worktrees nor claude/<TaskId>, so it does not name both sides.

## I10b-015: CONFIRMED
Grep of tools/ch_mcp .py/.ts for `"dump"` found only dump_facts.py:1054 (a diff label), no reader of the JSON "dump" key. kit.ts:19 and :63-64 only say kit_methods.json lists each method's dump line; no access of a "dump" field in kit.ts. The finder's unchecked item resolves: not read there (by grep, not a full read). The JSON "dump" paths are prose in data files. No split.

## Also
tests/test_h39_harness.ps1, named by harness.ps1, is absent from G\tests (confirmed by a missing-file error).
